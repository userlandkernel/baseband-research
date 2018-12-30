/*
 * Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Description:   Architecture independent parts of mutex objects.
 */

#include <l4.h>
#include <kdb/tracepoints.h>
#include <syscalls.h>
#include <tcb.h>
#include <schedule.h>
#include <smallalloc.h>
#include <arch/user_access.h>

DECLARE_TRACEPOINT(SYSCALL_MUTEX);
DECLARE_TRACEPOINT(SYSCALL_MUTEX_CONTROL);

DECLARE_KMEM_GROUP(kmem_mutexids);
DECLARE_KMEM_GROUP(kmem_mutex);

/*
 *  Globals.
 */

#if defined(CONFIG_DEBUG)

/** Global list of present mutex objects. */
mutex_t * global_mutexes_list = NULL;

/** Global mutexes list's lock. */
spinlock_t mutex_list_lock;

#endif

/** Memory allocation pool for mutexes. */
small_alloc_t mutex_pool;

/** Table mapping mutexid_t's to mutex_t pointers */
mutexid_table_t mutexid_table;

/*
 *  Local function prototypes.
 */

/**
 * Return from mutex system call with success result.
 *
 * @prarm continuation  Continuation to return back to userspace.
 */
static INLINE NORETURN void sys_mutex_success(continuation_t continuation);

/**
 * Return from mutex system call with failure result.
 *
 * @param error         The error code explaining the failure reason.
 * @prarm continuation  Continuation to return back to userspace.
 */
static INLINE NORETURN void sys_mutex_failure(word_t error,
        continuation_t continuation);

/**
 * Load the saved continuation 'mutex_continuation' from the TCB, and
 * activate it, returnining success to the caller.
 */
CONTINUATION_FUNCTION(return_success);

/** @todo FIXME: Doxygen header - awiggins. */
CONTINUATION_FUNCTION(return_failure_pagefault);

/** Waking up after blocking on a mutex. */
CONTINUATION_FUNCTION(mutex_wakeup);

/** Waking up after blocking on a hybrid mutex.
 * As for mutex_wakeup but does some extra cleanup in the event
 * that the previous holder has died. */
CONTINUATION_FUNCTION(hybrid_mutex_wakeup);

/**
 * Allocates a new mutex object, initialises it, and returns it.
 *
 * @param mutex_id The mutex identifier being initialised.
 */
static mutex_t * mutex_create(mutexid_t mutex_id);

/**
 * Frees a mutex object.
 *
 * @param mutex Pointer to the mutex object being freed.
 */
static void mutex_delete(mutexid_t mutex_id, mutex_t * mutex);

/*
 *  Mutex system calls.
 */

SYS_MUTEX(mutexid_t mutex_id, word_t flags, word_t * state_p)
{
    continuation_t continuation = ASM_CONTINUATION;
    tcb_t * current = get_current_tcb();
    mutex_t * mutex;
    mutex_flags_t flg;

    flg = flags;

    TRACEPOINT(SYSCALL_MUTEX,
            printf("SYS_MUTEX: current=%t, mutex=%M, flags=%c%c%c, state_p=%p\n",
                    current,
                    mutex_id.get_number(), flg.is_lock() ? 'L' : 'U',
                    flg.is_blocking() ? 'B' : 'N',
                   flg.is_hybrid() ? 'H' : 'K', state_p));

    current->sys_data.set_action(tcb_syscall_data_t::action_mutex);

    /* Check mutex identifier is valid. */
    if (EXPECT_FALSE(!mutexid_table.is_valid(mutex_id))) {
        sys_mutex_failure(EINVALID_MUTEX, continuation);
        NOTREACHED();
    }

    mutex = mutexid_table.lookup(mutex_id);

    /* Check mutex is valid. */
    if (EXPECT_FALSE(mutex == NULL)) {
        sys_mutex_failure(EINVALID_MUTEX, continuation);
        NOTREACHED();
    }

    /* What operation are we perfoming on the mutex object? Hybrid? */
    if (flg.is_hybrid()) {
        mutex_user_t user_mutex;

        user_mutex = state_p;
        /* If operation is lock, try to acquire the hybrid mutex. */
        if (flg.is_lock()) {
            mutex->hybrid_lock(current, user_mutex, continuation);
        }
        /* Otherwise, try to release the hybrid mutex. */
        else {
            mutex->hybrid_unlock(current, user_mutex, continuation);
        }
    }
#if !defined(CONFIG_HYBRID_MUTEXES)
    /* Otherwise, normal kernel-only mutex operation. */
    else {
        /* If operation is lock, try to acquire the mutex. */
        if (flg.is_lock()) {
            mutex->lock(current, flg.is_blocking(), continuation);
        }
        /* Otherwise, try to release the mutex. */
        else {
            mutex->unlock(current, continuation);
        }
    }
#else
    sys_mutex_failure(EINVALID_MUTEX, continuation);
#endif

    NOTREACHED();
}

SYS_MUTEX_CONTROL(mutexid_t mutex_id, word_t control)
{
    continuation_t continuation = ASM_CONTINUATION;
    tcb_t * current = get_current_tcb();
    mutex_t * mutex;

    TRACEPOINT(SYSCALL_MUTEX_CONTROL,
            printf("SYS_MUTEX_CONTROL: current=%t, mutex=%M, control=%s%s\n",
                    current, mutex_id.get_number(),
                    control & MUTEX_CONTROL_CREATE ? "C" : "",
                    control & MUTEX_CONTROL_DELETE ? "D" : ""));

    current->sys_data.set_action(tcb_syscall_data_t::action_mutex);

    /* Check caller is privileged. */
    if (EXPECT_FALSE(!is_privileged_space(get_current_space()))) {
        current->set_error_code(ENO_PRIVILEGE);
        goto error_out;
    }

    /* Check mutex identifier is valid. */
    if (EXPECT_FALSE(!mutexid_table.is_valid(mutex_id))) {
        current->set_error_code(EINVALID_MUTEX);
        goto error_out;
    }

    mutex = mutexid_table.lookup(mutex_id);

    /* Do we need to delete the mutex? If so do it before creating a new one. */
    if (control & MUTEX_CONTROL_DELETE) {

        /* Make sure mutex identifier is allocated. */
        if (EXPECT_FALSE(mutex == NULL)) {
            current->set_error_code(EINVALID_MUTEX);
            goto error_out;
        }

        /* Make sure the mutex isn't held. */
        mutex->mutex_lock.lock();
        tcb_t * holder = mutex->acquire(current);

        if (EXPECT_FALSE(holder != NULL)) {
            current->set_error_code(EMUTEX_BUSY);
            mutex->mutex_lock.unlock();
            goto error_out;
        }

        /* Delete the mutex. */
        mutex_delete(mutex_id, mutex);
        mutex->mutex_lock.unlock();
    }

    /* Do we need to create a new mutex? */
    if (control & MUTEX_CONTROL_CREATE) {

        /* Make sure mutex identifier is free. */
        if (EXPECT_FALSE(mutex != NULL)) {
            current->set_error_code(EINVALID_MUTEX);
            goto error_out;
        }

        /* Create the mutex. */
        mutex = mutex_create(mutex_id);

        /* Make sure creation succeeds. */
        if (EXPECT_FALSE(mutex == NULL)) {
            current->set_error_code(ENO_MEM);
            goto error_out;
        }
    }

    /* Return success. */
    return_mutex_control(1, continuation);

error_out:
    /* Return error, error status already set. */
    return_mutex_control(0, continuation);
}

/*
 *  mutex_t class methods.
 */

void
mutex_t::init(mutexid_t id)
{
    /* Initialise our lock. */
    this->mutex_lock.init();

    /* Mutexes have no initial holder/donatee. */
    this->sync_point.init(NULL);

    /* Add this mutex to the global list of mutexes. */
    this->enqueue_present();

#if defined(CONFIG_SYNCPOINT_NAMES)
    /* Clear the debug name. */
    this->debug_name[0] = '\0';
#endif

#if defined(CONFIG_DEBUG)
    /* Keep a local copy of the mutex's identifier. */
    this->id = id;
#endif
}

void
mutex_t::destroy(void)
{
    tcb_t * holder;
    scheduler_t * scheduler = get_current_scheduler();

    SMT_ASSERT(ALWAYS, this->mutex_lock.is_locked(true));
    ASSERT(ALWAYS, sync_point.get_blocked_head() == NULL);

    scheduler->scheduler_lock();
    this->dequeue_present();
    holder = this->get_holder();
    if (holder != NULL) {
        dequeue_held(holder);
    }
    scheduler->scheduler_unlock();
}

#if !defined(CONFIG_HYBRID_MUTEXES)
NORETURN void
mutex_t::lock(tcb_t * current, bool blocking, continuation_t continuation)
{
    ASSERT(ALWAYS, current != NULL);
    ASSERT(ALWAYS, current->is_grabbed_by_me());

    this->mutex_lock.lock();

    /* Attempt to acquire the mutex. */
    tcb_t * holder = this->acquire(current);

    /* Determine if we got the mutex. */
    if (holder == NULL) {
        /* Mutex successfully acquired by current. */
        this->mutex_lock.unlock();
        sys_mutex_success(continuation);
    }

    /* Are we doing a non-blocking acquire? */
    if (!blocking) {
        /* Current failed to acquire mutex on non-blocking call. */
        this->mutex_lock.unlock();
        sys_mutex_failure(EMUTEX_BUSY, continuation);
    }

    /* Determine if we already hold the mutex. */
    if (holder == current) {
        this->mutex_lock.unlock();
        sys_mutex_failure(EMUTEX_BUSY, continuation);
    }

    /* Current failed to acquire mutex, block. */
    get_current_scheduler()->scheduler_lock();
    this->sync_point.block(current);
    get_current_scheduler()->scheduler_unlock();
    this->mutex_lock.unlock();

    /* Go to sleep. When we next wake up, we should have the mutex
     * or the operation will have failed. */
    TCB_SYSDATA_MUTEX(current)->mutex_continuation = continuation;
    TCB_SYSDATA_MUTEX(current)->mutex = this;
    TCB_SYSDATA_MUTEX(current)->uncontended_hybrid = false;
    get_current_scheduler()->deactivate_sched(current,
            thread_state_t::waiting_mutex, current, mutex_wakeup,
            scheduler_t::sched_default);
    NOTREACHED();
}

NORETURN void
mutex_t::unlock(tcb_t * current, continuation_t continuation)
{
    this->mutex_lock.lock();

    /* If caller already holds the mutex, return an error. */
    if (this->get_holder() != current) {
        this->mutex_lock.unlock();
        sys_mutex_failure(EMUTEX_BUSY, continuation);
        NOTREACHED();
    }

    /* Release the mutex. */
    tcb_t * new_holder;

    new_holder = this->release();
    this->mutex_lock.unlock();

    /* If no thread were blocked on this mutex, simply return success. */
    if (new_holder == NULL) {
        sys_mutex_success(continuation);
        NOTREACHED();
    }

    /* Mutex had blocked threads, activate new holder and do a full schedule. */
    TCB_SYSDATA_MUTEX(current)->mutex_continuation = continuation;
    get_current_scheduler()->activate_sched(new_holder, thread_state_t::running,
                current, return_success, scheduler_t::sched_default);
    NOTREACHED();
}
#endif /* !CONFIG_HYBRID_MUTEXES */

NORETURN void
mutex_t::hybrid_lock(tcb_t * current, mutex_user_t user_mutex,
        continuation_t continuation)
{
    scheduler_t * scheduler = get_current_scheduler();

    ASSERT(ALWAYS, current != NULL);
    ASSERT(ALWAYS, current->is_grabbed_by_me());

    TCB_SYSDATA_MUTEX(current)->mutex_continuation = continuation;
    TCB_SYSDATA_MUTEX(current)->mutex = this;
    TCB_SYSDATA_MUTEX(current)->uncontended_hybrid = false;
    TCB_SYSDATA_MUTEX(current)->hybrid_user_state = user_mutex.get_state_p();

    this->mutex_lock.lock();

    TRACEPOINT(SYSCALL_MUTEX,
               printf("SYS_MUTEX_HYBRID: current=%t, kholder=%t, uholder=%t\n",
                      current, this->get_holder(), lookup_tcb_by_handle_locked(user_mutex.get_holder(current))));

    /* Find out who holds the mutex. */
    tcb_t * holder_kernel = this->get_holder();

    /* If caller already holds the mutex, return an error.
     * Unless user holder is null - correct it and return success. */
    if (holder_kernel == current) {
        this->mutex_lock.unlock();
        sys_mutex_failure(EMUTEX_BUSY, continuation);
        NOTREACHED();
    }

    /* Otherwise, if the kernel object isn't held, check user object.  */
    else if (holder_kernel == NULL) {
        /* Updated the user mutex objects state. */
        do {
            /* If mutex was free and caller locked the mutex, return success. */
            if (user_mutex.atomic_set_holder(current, current->tcb_idx)) {
                this->mutex_lock.unlock();
                sys_mutex_success(continuation);
                NOTREACHED();
            }
        /* The mutex is held, mark as contended, try again if updated failed. */
        } while (!user_mutex.atomic_set_contended(current));

        /* Update kernel mutex object's holder. */
        word_t holder_user = user_mutex.get_holder(current);
        tcb_t * holder_kernel = lookup_tcb_by_handle_locked(holder_user);

        /* User holder has died. */
        if (holder_kernel == NULL) {
            user_mutex.set_holder(current, current->tcb_idx, false);
            this->mutex_lock.unlock();
            sys_mutex_success(continuation);
            NOTREACHED();
        }
        scheduler->scheduler_lock();
        this->set_holder(holder_kernel);
        holder_kernel->unlock_read();
    }
    /* The kernel knows the mutex is held, prepare to block. */
    else {
        /**
         *  @todo FIXME: Validate that the user encoded holder is
         *  correct, if invalid, return an error - awiggins.
         */
        scheduler->scheduler_lock();
    }

    this->sync_point.block(current);
    scheduler->scheduler_unlock();
    this->mutex_lock.unlock();
    scheduler->deactivate_sched(current,
            thread_state_t::waiting_mutex, current, hybrid_mutex_wakeup,
            scheduler_t::sched_default);
    NOTREACHED();
}

NORETURN void
mutex_t::hybrid_unlock(tcb_t * current, mutex_user_t user_mutex,
        continuation_t continuation)
{
    tcb_t * holder_kernel;

    TCB_SYSDATA_MUTEX(current)->mutex_continuation = continuation;
    TCB_SYSDATA_MUTEX(current)->mutex = this;
    TCB_SYSDATA_MUTEX(current)->uncontended_hybrid = false;

    /* Get the current holder of the mutex object. */
    this->mutex_lock.lock();
    holder_kernel = this->get_holder();

    /* If there is no holder, we are just doing a simple user-space unlock. */
    if (holder_kernel == NULL) {
        /* Determine the state of the user mutex object. */
        word_t holder_user = user_mutex.get_holder(current);

        /* If user object has caller as holder, free the user mutex object. */
        if (holder_user == current->tcb_idx) {
            user_mutex.set_free(current);
            this->mutex_lock.unlock();
            sys_mutex_success(continuation);
            NOTREACHED();
        }
        /* Otherwise caller doesn't hold the mutex, return error. */
        else {
            this->mutex_lock.unlock();
            sys_mutex_failure(EMUTEX_BUSY, continuation);
            NOTREACHED();
        }
    }

    /* Mutex is contended. If we aren't the holder, return error */
    if (holder_kernel != current) {
        this->mutex_lock.unlock();
        sys_mutex_failure(EMUTEX_BUSY, continuation);
        NOTREACHED();
    }

    /* Caller holds the mutex, release it. */
    holder_kernel = this->release();

    /* If no threads were blocked on the mutex, simply free it. This
     * may occur if the last thread blocked on the mutex is unwound. */
    if (holder_kernel == NULL) {
        user_mutex.set_free(current);
        this->mutex_lock.unlock();
        sys_mutex_success(continuation);
        NOTREACHED();
    }

    /* Mutex acquired by a new holder, update the user object. */
    word_t holder_user = holder_kernel->tcb_idx;

    /* If mutex is uncontended, stop tracking in kernel. */
    if (!this->is_contended()) {
        (void)this->release();
        TCB_SYSDATA_MUTEX(holder_kernel)->uncontended_hybrid = true;
    }
    user_mutex.set_holder(current, holder_user, this->is_contended());

    this->mutex_lock.unlock();

    /* Perform a full schedule. */
    get_current_scheduler()->activate_sched(holder_kernel,
            thread_state_t::running, current, return_success,
            scheduler_t::sched_default);
    NOTREACHED();
}

void
mutex_t::set_holder(tcb_t * holder)
{
    SMT_ASSERT(ALWAYS, this->mutex_lock.is_locked(true));
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    ASSERT(ALWAYS, holder);
    ASSERT(ALWAYS, this->get_holder() == NULL);
    ASSERT(ALWAYS, this->sync_point.get_blocked_head() == NULL);

    this->enqueue_held(holder);
    this->sync_point.become_donatee(holder);
}

tcb_t *
mutex_t::acquire(tcb_t * candidate)
{
    scheduler_t * scheduler = get_current_scheduler();

    ASSERT(ALWAYS, candidate->get_waiting_for() == NULL);

    /* Check if this mutex is currently held. */
    tcb_t * holder = this->sync_point.get_donatee();

    /* If it is, return the current holder. */
    if (holder != NULL) {
        return holder;
    }

    /* Grab the mutex. */
    ASSERT(ALWAYS, candidate->get_waiting_for() == NULL);
    scheduler->scheduler_lock();
    this->enqueue_held(candidate);
    this->sync_point.become_donatee(candidate);
    scheduler->scheduler_unlock();

    /* This mutex object wasn't held, candidate acquired it. */
    return NULL;
}

tcb_t *
mutex_t::release(void)
{
    int backoff = 1;
    tcb_t * new_holder;
    scheduler_t * scheduler = get_current_scheduler();

restart_release:
    SMT_ASSERT(ALWAYS, this->mutex_lock.is_locked(true));
    ASSERT(ALWAYS, this->get_holder() != NULL);
    ASSERT(ALWAYS, !this->get_holder()->ready_list.is_queued());

    scheduler->scheduler_lock();
    new_holder = this->sync_point.get_blocked_head();
    /* If nobody is blocked on the mutex, simply release donatee. */
    if (new_holder == NULL) {
        dequeue_held(this->get_holder());
        this->sync_point.release_donatee();
        scheduler->scheduler_unlock();
        return NULL;
    }

    /* Try and grab the head blocked thread, if the grab fails,
     * restart release. */
    if (!new_holder->grab()) {
        scheduler->scheduler_unlock();
        mutex_lock.unlock();
        /** @todo FIXME: Replace with relax() call - davidg. */
        for (volatile int i = 0; i < backoff; i++);
        if (backoff < (1 << 16)) {
            backoff *= 2;
        }
        mutex_lock.lock();
        goto restart_release;
    }

    /* Release the mutex. */
    ASSERT(ALWAYS, new_holder->get_state().is_waiting_mutex());
    dequeue_held(this->get_holder());
    this->sync_point.release_donatee();

    /* Hand the mutex over. */
    ASSERT(ALWAYS, new_holder->is_grabbed_by_me());
    enqueue_held(new_holder);
    this->sync_point.unblock(new_holder);
    this->sync_point.become_donatee(new_holder);
    scheduler->scheduler_unlock();
    return new_holder;
}

void
mutex_t::enqueue_present(void)
{
#if defined(CONFIG_DEBUG)

    mutex_list_lock.lock();
    ENQUEUE_LIST_TAIL(mutex_t, global_mutexes_list, this, present_list);
    mutex_list_lock.unlock();

#endif
}

void
mutex_t::dequeue_present(void)
{
#if defined(CONFIG_DEBUG)

    mutex_list_lock.lock();
    DEQUEUE_LIST(mutex_t, global_mutexes_list, this, present_list);
    mutex_list_lock.unlock();

#endif
}

void
mutex_t::enqueue_held(tcb_t * holder)
{
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    ENQUEUE_LIST_TAIL(mutex_t, holder->mutexes_head, this, held_list);
}

void
mutex_t::dequeue_held(tcb_t * holder)
{
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    DEQUEUE_LIST(mutex_t, holder->mutexes_head, this, held_list);
}

/*
 *  mutex_user_t functions.
 */

bool
mutex_user_t::atomic_set_holder(tcb_t * user_thread, word_t holder)
{
    bool result;

    user_state_set_uncontended(holder);
    user_thread->set_user_access(return_failure_pagefault);
    result = user_compare_and_set_word(this->state_p, MUTEX_FREE, holder);
    user_thread->clear_user_access();

    return result;
}

bool
mutex_user_t::atomic_set_contended(tcb_t * user_thread)
{
    bool result;
    word_t user_state;

    user_thread->set_user_access(return_failure_pagefault);
    user_state = user_read_word(state_p);

    /* If already contended, simply return success. */
    if (user_state_is_contended(user_state)) {
        user_thread->clear_user_access();
        return true;
    }
    /* Otherwise, if the holder is zero somethings changed, return failure. */
    if (user_state_is_free(user_state)) {
        user_thread->clear_user_access();
        return false;
    }
    /* The user object is held uncontended, make it contended */
    word_t new_state = user_state;

    user_state_set_contended(new_state);
    result = user_compare_and_set_word(this->state_p, user_state, new_state);
    user_thread->clear_user_access();

    return result;
}

word_t
mutex_user_t::get_holder(tcb_t * user_thread)
{
    word_t user_state;

    user_thread->set_user_access(return_failure_pagefault);
    user_state = user_read_word(this->state_p);
    user_thread->clear_user_access();
    user_state_set_contended(user_state);
    return user_state;
}

void
mutex_user_t::set_holder(tcb_t * user_thread, word_t holder, bool contended)
{
    word_t user_state;

    /* Assert that the holder doesn't have the handle bit set. */
    ASSERT(ALWAYS, !user_state_is_uncontended(holder));

    user_state = holder;
    if (!contended) {
        user_state_set_uncontended(user_state);
    }
    user_thread->set_user_access(return_failure_pagefault);
    user_write_word(this->state_p, user_state);
    user_thread->clear_user_access();
}

void
mutex_user_t::set_free(tcb_t * user_thread)
{
    user_thread->set_user_access(return_failure_pagefault);
    user_write_word(this->state_p, MUTEX_FREE);
    user_thread->clear_user_access();
}

/*
 *  Local functions.
 */

CONTINUATION_FUNCTION(mutex_wakeup)
{
    tcb_t * current = get_current_tcb();
    mutex_t * mutex = TCB_SYSDATA_MUTEX(current)->mutex;
    continuation_t continuation =
            TCB_SYSDATA_MUTEX(current)->mutex_continuation;

    /* If the mutex has been deleted or we have been unwound, return error. */
    if (mutex == NULL) {
        /** @todo FIXME: This error isn't appropriate for unwind - awiggins. */
        sys_mutex_failure(EMUTEX_BUSY, continuation);
        NOTREACHED();
    }

    /* Otherwise, we are the mutex holder. */
    ASSERT(ALWAYS, (mutex->get_holder() == current) ||
            TCB_SYSDATA_MUTEX(current)->uncontended_hybrid);

    sys_mutex_success(continuation);
}

CONTINUATION_FUNCTION(hybrid_mutex_wakeup)
{
    tcb_t * current = get_current_tcb();
    mutex_t * mutex = TCB_SYSDATA_MUTEX(current)->mutex;
    continuation_t continuation =
        TCB_SYSDATA_MUTEX(current)->mutex_continuation;

    /* If the mutex has been deleted or we have been unwound, return error. */
    if (mutex == NULL) {
        /** @todo FIXME: This error isn't appropriate for unwind - awiggins. */
        sys_mutex_failure(EMUTEX_BUSY, continuation);
        NOTREACHED();
    }

    /* Ensure the user holder is consistent */
    mutex->mutex_lock.lock();
    mutex_user_t user_mutex;
    user_mutex = TCB_SYSDATA_MUTEX(current)->hybrid_user_state;
    if (!mutex->is_contended()) {
        TCB_SYSDATA_MUTEX(current)->uncontended_hybrid = true;
    }
    user_mutex.set_holder(current, current->tcb_idx, mutex->is_contended());
    mutex->mutex_lock.unlock();

    /* Otherwise, we are the mutex holder. */
    ASSERT(ALWAYS, (mutex->get_holder() == current) ||
           TCB_SYSDATA_MUTEX(current)->uncontended_hybrid);

    sys_mutex_success(continuation);
}

CONTINUATION_FUNCTION(return_success)
{
    tcb_t * current = get_current_tcb();
    continuation_t continuation;

    continuation = TCB_SYSDATA_MUTEX(current)->mutex_continuation;
    sys_mutex_success(continuation);
}

CONTINUATION_FUNCTION(return_failure_pagefault)
{
    tcb_t * current = get_current_tcb();
    continuation_t continuation;

    continuation = TCB_SYSDATA_MUTEX(current)->mutex_continuation;
    sys_mutex_failure(EINVALID_PARAM, continuation);
}

static INLINE NORETURN void
sys_mutex_success(continuation_t continuation)
{
    /* Return success. */
    return_mutex(1, continuation);
}

static INLINE NORETURN void
sys_mutex_failure(word_t error, continuation_t continuation)
{
    /* Set error code and return error status. */
    tcb_t * current = get_current_tcb();
    current->set_error_code(error);
    return_mutex(0, continuation);
}

static mutex_t *
mutex_create(mutexid_t mutex_id)
{
    mutex_t * mutex = (mutex_t *)mutex_pool.allocate();

    /* Test allocation succeded. */
    if (!mutex) {
        return NULL;
    }

    mutex->init(mutex_id);
    mutexid_table.insert(mutex_id, mutex);

    return mutex;
}

static void
mutex_delete(mutexid_t mutex_id, mutex_t * mutex)
{
    ASSERT(DEBUG, mutex);

    mutexid_table.remove(mutex_id);
    mutex->destroy();
    mutex_pool.free(mutex);
}

/*
 *  Initialisation functions.
 */
void SECTION(SEC_INIT) init_mutex(void)
{
    word_t max_mutexes = get_init_data()->max_mutexes;

    /* kmem.alloc requires allocations to be in multiples of KMEM_CHUNKSIZE. */
    ASSERT(ALWAYS, KMEM_CHUNKSIZE % sizeof(mutex_t *) == 0);
    ASSERT(ALWAYS, max_mutexes * sizeof(mutex_t *) % KMEM_CHUNKSIZE == 0);
    ASSERT(ALWAYS, max_mutexes > 0);

    /* Create the mutexes. */
    TRACE_INIT("Init mutexids for %d mutexs\n", max_mutexes);
    void * new_table =
        kmem.alloc(kmem_mutexids, sizeof(mutex_t *) * max_mutexes, true);
    ASSERT(ALWAYS, new_table);

    /* Initialise the table. */
    mutexid_table.init(new_table, max_mutexes);

    /* Initialise the pool. */
    mutex_pool.init(kmem_mutex, sizeof(mutex_t), max_mutexes);
}
