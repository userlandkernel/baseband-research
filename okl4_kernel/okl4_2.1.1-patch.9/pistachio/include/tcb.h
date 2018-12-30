/*
 * Copyright (c) 2002, 2003-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2005, National ICT Australia (NICTA)
 */
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
 * Description:   TCB
 */
#ifndef __TCB_H__
#define __TCB_H__

#include <kernel/debug.h>
#include <atomic_ops/atomic_ops.h>
#include <kernel/threadstate.h>
#include <kernel/space.h>
#include <kernel/resources.h>
#include <kernel/thread.h>
#include <kernel/preempt.h>
#include <kernel/tcb_syscall_data.h>
#include <kernel/syncpoint.h>
#include <kernel/endpoint.h>
#include <kernel/utcb.h>
#include <kernel/mp.h>
#include <kernel/smallalloc.h>
#include <kernel/mutex.h>
#include <kernel/read_write_lock.h>
#include <kernel/profile.h>

/* implementation specific functions */
#include <kernel/arch/ktcb.h>




class space_t;
class prio_queue_t;
class scheduler_t;
class mutex_t;

/**
 * tcb_t: kernel thread control block
 */
class tcb_t
{
public:
    /* public functions */
    bool activate(void (*startup_func)());

    bool create_kernel_thread(threadid_t dest, utcb_t * utcb);

    /* thread deletion */
    void delete_tcb();
    void cancel_ipcs();
    bool release_mutexes();

    bool migrate_to_domain(cpu_context_t context);

    void unwind(tcb_t *partner);


    /*
     *  Queue manipulations
     */

    void enqueue_present();
    void dequeue_present();

    /* thread id access functions */
    void set_global_id(threadid_t tid);
    threadid_t get_global_id();

    void set_mutex_thread_handle(threadid_t handle);

    bool check_utcb_location (void);
    void set_utcb_location (word_t location);
    word_t get_utcb_location();

    void set_error_code (word_t err);
    word_t get_error_code (void);

#if !defined(CONFIG_MUNITS)
private:
#endif
    /** Set the state of the thread to the given state. Should
     * only be used internally by the class or by the scheduler. */
    void set_state(thread_state_t state);

    /** Get the current state of the thread. */
public:
    thread_state_t get_state();

    /** Initialise a new thread's state to the given state. */
    void initialise_state(thread_state_t state);

    /** Get architecture-specific TCB data. */
    arch_ktcb_t * get_arch();

    void save_state(word_t mrs = MAX_SAVED_MESSAGE_REGISTERS);
    void restore_state(word_t mrs = MAX_SAVED_MESSAGE_REGISTERS);

    /* thread state - global scheduler */
    bool grab();
    void release();
    bool is_grabbed_by_me();
#ifdef CONFIG_MUNITS
    bool is_owned();
#endif

    /**
     * @brief Set the syncpoint that this thread is currently blocked on.
     *
     * @param syncpoint  The synchronisation point this thread is currently
     *                   blocked on.
     */
    INLINE void set_waiting_for(syncpoint_t * syncpoint);

    /**
     * @brief Get the syncpoint that this thread is currently blocked on.
     *
     * @return The syncpoint that this thread is currently blocked on.
     */
    INLINE syncpoint_t * get_waiting_for(void);

    /**
     * @brief If this thread is currently blocked on a syncpoint, unblock it.
     */
    void remove_dependency(void);

    /** Return this thread's end-point. */
    INLINE endpoint_t * get_endpoint(void);

    /**
     *  Mark this thread as reserved, and should not be added back
     *  onto the scheduling queue.
     */
    INLINE void reserve(void);

    /** Unreserve this thread, allowing it to be enqueued once again. */
    INLINE void unreserve(void);

    /** Determine if this thread has been reserved. */
    INLINE bool is_reserved(void);

    /* context - usually set through set_scheduled */
    cpu_context_t get_context();

    /* ipc */
    void set_partner(tcb_t *partner);
    tcb_t * get_partner();
    bool is_partner_valid();

    msg_tag_t get_tag();
    void set_tag(msg_tag_t tag);
    word_t get_mr(word_t index);
    void set_mr(word_t index, word_t value);
    bool copy_mrs(tcb_t * dest, word_t start, word_t count);

    /**
     * Convenience function to perform an ipc to a user thread from
     * within the kernel The message must be in the current threads
     * message registers.
     *
     * This function basically invokes SYS_IPC, and handles the return
     * from this function through the activation of it's continuation
     * function.
     *
     * The tag that used to be returned by this function can now be
     * accessed from the UTCB's MR0
     *
     * @param to_tid the thread id to send the message to
     * @param from_tid the thread id to receive a message from
     * @param continuation the continuation to activate when control is resumed
     */
    void do_ipc(threadid_t to_tid, threadid_t from_tid,
            continuation_t continuation) NORETURN;
    void send_pagefault_ipc(addr_t addr, addr_t ip, space_t::access_e access,
            continuation_t continuation) NORETURN;

    /**
     * Return to user directly from an ipc without using the normal
     * continuations
     *
     * This is used for cancelling IPC's through unwind
     */
    void return_from_ipc(void) NORETURN;
    /**
     * Return to user directly from an exception or irq context without
     * activating the normal continuations
     *
     * This is used for cancelling pagefault/IRQ Ipcs
     */
    void return_from_user_interruption(void) NORETURN;

    /* thread notification */
    void notify(continuation_t continuation);

    /* thread manipulation */
    addr_t get_user_ip();
    addr_t get_user_sp();
    void set_user_ip(addr_t ip);
    void set_user_sp(addr_t sp);
    void copy_user_regs(tcb_t *src);
    word_t set_tls(word_t *mr);
    void copy_mrs_to_regs(tcb_t *dest);
    void copy_regs_to_mrs(tcb_t *src);
    void copy(tcb_t *src);

    /* suspend / resume support */
    bool is_suspended();
    void set_suspended(bool new_state);

    /* Setup a function to be called back prior to this thread next
     * returning to userspace from either an exception or system
     * call. */
    void set_post_syscall_callback(callback_func_t func);
    callback_func_t get_post_syscall_callback();

    /* preemption callback signaling */
    addr_t get_preempted_ip();
    void set_preempted_ip(addr_t ip);
    addr_t get_preempt_callback_ip();

    /* sender space id */
    void set_sender_space(spaceid_t space_id);
    spaceid_t get_sender_space();

    /* space */
    spaceid_t get_space_id() const PURE;
    space_t * get_space() const PURE;
    void set_space(space_t * space);

    /* base/extension for space_switch.
     * only set on switching to the extended space.
     * unset on switching away from the extended space. */
    space_t * get_base_space();
    void set_base_space(space_t * space);
    void clear_base_space(void);
    bool has_base_space(void);
    
    bool is_local_domain();
    bool is_local_unit();

    /* utcb access functions */
    utcb_t * get_utcb() const PURE;
    void set_utcb(utcb_t *new_utcb);

public:
    void set_pager(tcb_t *tcb);
    void set_scheduler(tcb_t *tcb);
    void set_exception_handler(tcb_t *tcb);
    void set_user_handle(const word_t handle);
    void set_user_flags(const word_t flags);
    void set_acceptor(const acceptor_t acceptor);

    tcb_t * get_pager();
    tcb_t * get_scheduler();
    tcb_t * get_exception_handler();

    word_t get_user_handle();
    word_t get_user_flags();
    preempt_flags_t get_preempt_flags();
    void set_preempt_flags(preempt_flags_t flags);
    u8_t get_cop_flags();
    word_t * get_reg_stack_bottom (void);
    acceptor_t get_acceptor();

    bool in_exception_ipc(void);
    void clear_exception_ipc(void);
    bool copy_exception_mrs_from_frame(tcb_t *dest);
    bool copy_exception_mrs_to_frame(tcb_t *dest);

#ifdef CONFIG_SCHEDULE_INHERITANCE
    /** Calculate, but don't modify, a thread's effective priority. */
    prio_t calc_effective_priority();
#endif

    /* asynchronous notification */
    void clear_notify_bits();
    word_t add_notify_bits(const word_t bits);
    word_t sub_notify_bits(const word_t bits);
    void set_notify_mask(const word_t mask);
    /**
     * This function sets the threads state in such a way that
     * preemption is safe and when resumed after preemption the thread
     * will activate the given continuation
     *
     * Note: This functions does not enable preemption, simply makes
     * it safe to enable
     *
     * It is generally not possible to return to user after this
     * function has been called without calling
     * disable_preempt_recover first
     *
     * This is not a control function even though it takes a
     * continuation argument - ie it will return normally, not by
     * activating the continuation
     *
     * @param continuation  Continuation to activate upon resumption
     *                      from preemption.
     */
    void enable_preempt_recover(continuation_t continuation);
    /**
     * This function sets the threads state back to normal, so that
     * preemption is once again unsafe but normal return to user is
     * possible.
     *
     * It is safe to call this function even if enable_preempt_recover
     * has not been called.
     */
    void disable_preempt_recover();
    /**
     * These functions control access to the fault on user address
     * support.
     *
     * Note: These functions are not control functions even if they
     * take a continuation_t as an argument.
     */
    void set_user_access(continuation_t cont);
    void clear_user_access(void);
    bool user_access_enabled(void);
    continuation_t user_access_continuation(void);

    word_t get_notify_bits();
    word_t get_notify_mask();

    /*
     * Thread Locking
     *
     * Write - lock for modify/delete TCB, read lock for IPC etc
     */
    bool try_lock_read() { return thread_lock.try_lock_read(); };
    bool try_lock_write() { return thread_lock.try_lock_write(); };
    void unlock_read() { thread_lock.unlock_read(); };
    void unlock_write() { thread_lock.unlock_write(); };
    bool is_locked() { return thread_lock.is_locked(); };

    /* IPC Control. */
    bool has_ipc_restrictions(void);
    bool cannot_send_ipc(const tcb_t*);

public:
    void init(threadid_t dest);

    /* stack manipulation */
public:
    void init_stack();
private:
    void create_startup_stack(void (*func)());

    friend void make_offsets(); /* generates OFS_TCB_ stuff */

public:


    tcb_t *get_saved_partner (void) { return saved_partner; }
    void set_saved_partner (tcb_t *t) { saved_partner = t; }

    thread_state_t get_saved_state (void)
        { return saved_state; }
    void set_saved_state (thread_state_t s)
        { saved_state = s; }
    void set_saved_state (thread_state_t::thread_state_e s)
        { saved_state = (thread_state_t) s; }

    /* do not delete this STRUCT_START_MARKER */

    /** these have relatively static values here **/
    threadid_t          myself_global;

    /* user location of utcb */
    word_t              utcb_location;
    /* kernel alias of utcb */
    utcb_t *            utcb;

private:
    /* space the thread belongs to */
    space_t *           space;
    /* cache of space's unique identifier. */
    spaceid_t           space_id;

    /* base space that the thread belongs to,
     * if the thread is presently executing in its
     * extended space following a space_switch call */
    space_t *           base_space;

public:
    /* cache of space's page directory */
    pgent_t *           page_directory;

private:
    /* thread's pager */
    cap_t               pager;

    /** thread state and frequently modified data **/
private:
    read_write_lock_t   thread_lock;
    thread_state_t      thread_state;
    tcb_t *             partner;

    /** End-point for this thread's IPC operations. */
    endpoint_t          end_point;

    /** Synchronisation point, if any, this thread is currently waiting on. */
    syncpoint_t *       waiting_for;

    cap_t               exception_handler;

public:
    resource_bits_t     resource_bits;
    continuation_t      cont;

    /* preemption continuation */
    continuation_t      preemption_continuation;

#if defined(CONFIG_MUNITS)
    okl4_atomic_word_t       context;
#endif /* CONFIG_MUNITS */

    arch_ktcb_t         arch;

    /* thread suspention */
    bool                suspended;
    callback_func_t     post_syscall_callback;

    ringlist_t<tcb_t>   ready_list;

    /**
     * List of threads currently blocked on 'waiting_for'
     * synchronisation point.
     */
    ringlist_t<tcb_t>   blocked_list;

    /** List of mutexes currently held by this thread. */
    mutex_t *           mutexes_head;

#if defined(CONFIG_DEBUG)
    ringlist_t<tcb_t>   present_list;
#endif

    /**
     * The priority of the thread, before taking into account interactions with
     * other threads. This value is used as starting point for calculating the
     * effective priority of a thread.
     */
    prio_t              base_prio;

    /**
     * The priority of the thread, after taking into account priority
     * inheritance from other threads. This is the priority that should be used
     * when determining which thread should be running at a particular point in
     * time.
     */
    prio_t              effective_prio;

#if (defined CONFIG_MDOMAINS)
    ringlist_t<tcb_t>   xcpu_list;
    word_t              xcpu_status;

#endif

public:

    /* scheduling */
    word_t              timeslice_length;
    word_t              current_timeslice;

#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    /* Only allow the thread to run on certain hardware units. */
    word_t              context_bitmask;
#endif

#if defined(CONFIG_MUNITS)
    /* Set to a positivbe value 'true' if this thread is reserved to be scheduled /
     * grabbed by another hardware unit. If so, the thread should not be
     * enqueued again. */
    okl4_atomic_word_t       reserved;
#endif
private:
    cap_t               scheduler;

public:
    tcb_t *             saved_partner;
    thread_state_t      saved_state;
    thread_resources_t  resources;
    ringlist_t<tcb_t>   thread_list;
#ifdef CONFIG_THREAD_NAMES
    char                debug_name[MAX_DEBUG_NAME_LENGTH];
#endif
    threadid_t          saved_sent_from;
public:
    tcb_syscall_data_t  sys_data;

#if (defined CONFIG_MDOMAINS) || (defined CONFIG_MUNITS)
    /* Mailbox walking continuation */
    continuation_t      xcpu_continuation;
#endif

    word_t              tcb_idx;
    cap_t *             master_cap;
    threadid_t          sent_from;
    /* saved interrupt stack -> only used from interrupt assembly routine */
    word_t              irq_stack;

#if (defined CONFIG_L4_PROFILING)
    profile_thread_data_t profile_data;
#endif

private:
    /* do not delete this STRUCT_END_MARKER */

    /* class friends */
    friend void dump_tcb(tcb_t *);
    friend void handle_ipc_error (void);
    friend class thread_resources_t;
    friend class scheduler_t;
    friend void switch_to(tcb_t *, tcb_t *);
    friend void switch_from(tcb_t *, continuation_t);
};

/* union to allow allocation of kernel stacks */
union stack_t {
    u8_t pad[STACK_SIZE];
};

#define KTCB_SIZE   ((sizeof(tcb_t) + (KTCB_ALIGN-1)) & (~(KTCB_ALIGN-1)))

/*
 * Thread lookup
 */
#if defined(CONFIG_DEBUG)
/**
 * Translates a pointer within a TCB into a valid TCB pointer,
 * or NULL if the pointer is not valid.
 *
 * @param ptr  Pointer to somewhere in the TCB.
 * @returns    Pointer to the TCB.
 */
tcb_t * get_tcb(void * ptr);
#endif

tcb_t* lookup_tcb_by_handle_locked(word_t threadhandle);
tcb_t* acquire_read_lock_tcb(tcb_t *tcb);

/*
 * TCB allocation and freeing
 */
tcb_t* allocate_tcb(threadid_t tid);
void free_tcb(tcb_t *tcb);

void init_tcb_allocator(void);

/**********************************************************************
 *
 * Thread Context Switching
 *
 **********************************************************************/

/**
 * Mark that the given thread should be considered as no longer running.
 *
 * This function will return, and the kernel will continue to execute on
 * its current stack, but will do so without any concept of
 * 'get_current_thread()'.
 *
 * This function allows a thread to be released by the kernel, more work
 * performed, and then a new thread switched to using the 'switch_to'
 * call. On uniprocessor systems, this is generally unnecessary, but on
 * SMP/SMT systems this is required by the scheduler.
 *
 * The given continuation function will be were the current thread
 * next wakes up.
 *
 * @param current
 *     The currently running TCB that will be switched away from.
 *
 * @param continuation
 *     The continuation that 'current' should wake up at when
 *     it is next scheduled.
 */
void switch_from(tcb_t * current, continuation_t continuation);

/**
 * Perform a context switch to another thread. The kernel must have
 * already called 'switch_from' to disown the currently running thread.
 * Only the scheduler should need to use these functions.
 *
 * This is the most primitive control function. The continuation
 * argument is the point where the current thread will execute upon
 * being resumed.
 *
 * Also passed in is a thread whose schedule we will be running
 * under. If the system is using schedule-inheritance, this means
 * that 'dest' may run using another thread's schedule. This will
 * occur when another thread (with higher priority) is blocked
 * waiting for 'dest' to finish. If 'schedule' is the same as
 * 'dest', then the destination thread runs on its own priority.
 *
 * At the conclusion of this function, execution will pass to the
 * destination's continuation, stored in 'cont'.
 *
 * @param dest
 *     The tcb to switch to.
 *
 * @param schedule
 *     The tcb whose schedule we will start to use. May be NULL to
 *     indicate no change of schedule will take place.
 *
 * @param continuation
 *     The continuation to activate when the thread is resumed.
 */
void switch_to(tcb_t * dest, tcb_t * schedule) NORETURN;

/**********************************************************************
 *
 *                    Generic access functions
 *               Operations on thread ids and settings
 *
 **********************************************************************/
extern spinlock_t state_lock;

INLINE space_t * tcb_t::get_space() const
{
    return space;
}

INLINE spaceid_t tcb_t::get_space_id() const
{
    return space_id;
}

INLINE utcb_t * tcb_t::get_utcb() const
{
    return this->utcb;
}

INLINE void tcb_t::set_utcb(utcb_t *new_utcb)
{
    this->utcb = new_utcb;
}


/**********************************************************************
 *
 *               Operations on thread ids and settings
 *
 **********************************************************************/

INLINE threadid_t tcb_t::get_global_id()
{
    return myself_global;
}

/**
 * Get TCB of a thread's scheduler
 * @return      TCB of scheduler
 */
INLINE tcb_t * tcb_t::get_scheduler()
{
    return this->scheduler.get_tcb();
}

/**********************************************************************
 *
 *                  Access functions
 *
 **********************************************************************/
INLINE bool tcb_t::grab()
{
#ifdef CONFIG_MUNITS
    /* Ensure that we don't attempt to grab a thread in a different domain. */
    ASSERT(ALWAYS, this->get_context().domain == get_current_context().domain);

    /* Ensure that we don't attempt to grab the same thread twice. */
    ASSERT(ALWAYS, this->get_context().unit != get_current_context().unit);

    /* Attempt to set 'context' to our hardware unit. */
    return okl4_atomic_compare_and_set(&this->context, 0x0000ffff,
            get_current_context().unit);
#else
    return true;
#endif
}

INLINE void tcb_t::release()
{
#ifdef CONFIG_MUNITS
    ASSERT(ALWAYS, this->get_context() == get_current_context());
    okl4_atomic_set(&this->context, 0xffff);
#endif
}

INLINE bool tcb_t::is_grabbed_by_me()
{
#ifdef CONFIG_MUNITS
    return this->get_context() == get_current_context();
#else
    return true;
#endif
}

#ifdef CONFIG_MUNITS
INLINE bool tcb_t::is_owned()
{
    return (get_context().unit != 0xFFFF);
}
#endif

INLINE cpu_context_t tcb_t::get_context()
{
#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
    cpu_context_t c;
    c.raw = okl4_atomic_read(&context);
    return c;
#else
    return 0;
#endif
}

INLINE void tcb_t::set_state(thread_state_t state)
{
    this->thread_state.state = state.state;
}

INLINE void tcb_t::initialise_state(thread_state_t state)
{
    this->thread_state.state = state.state;
}

INLINE thread_state_t tcb_t::get_state()
{
    return thread_state;
}

INLINE void tcb_t::set_partner(tcb_t *tcb)
{
    this->partner = tcb;
}

INLINE bool tcb_t::is_partner_valid()
{
    return ((word_t)this->partner != INVALID_RAW);
}

INLINE tcb_t * tcb_t::get_partner()
{
    ASSERT(DEBUG, (word_t)this->partner != INVALID_RAW);
    return this->partner;
}

INLINE endpoint_t *
tcb_t::get_endpoint(void)
{
    return &(this->end_point);
}

INLINE void
tcb_t::set_waiting_for(syncpoint_t * syncpoint)
{
    this->waiting_for = syncpoint;
}

INLINE syncpoint_t *
tcb_t::get_waiting_for(void)
{
    return this->waiting_for;
}

/**
 * Get TCB of a thread's pager
 * @return      TCB of pager
 */
INLINE tcb_t * tcb_t::get_pager()
{
    return this->pager.get_tcb();
}

/**
 * Get TCB of a thread's exception handler
 * @return      TCB of exception handler
 */
INLINE tcb_t * tcb_t::get_exception_handler()
{
    return this->exception_handler.get_tcb();
}

/**
 * Get a thread's user-defined handle
 * @return      user-defined handle
 */
INLINE word_t tcb_t::get_user_handle()
{
    return get_utcb()->user_defined_handle;
}

/**
 * Set user-defined handle for a thread
 * @param handle        new value for user-defined handle
 */
INLINE void tcb_t::set_user_handle(const word_t handle)
{
    get_utcb()->user_defined_handle = handle;
}

/**
 * Set the IPC error code
 * @param err   new IPC error code
 */
INLINE void tcb_t::set_error_code(word_t err)
{
    get_utcb()->error_code = err;
}

/**
 * Get the IPC error code
 * @return      IPC error code
 */
INLINE word_t tcb_t::get_error_code(void)
{
    return get_utcb()->error_code;
}

/**
 * Get a thread's preemption flags
 * @return      preemption flags
 */
INLINE preempt_flags_t tcb_t::get_preempt_flags (void)
{
    preempt_flags_t flags;
    flags.raw = get_utcb()->preempt_flags;
    return flags;
}

/**
 * Set a thread's preemption flags
 * @param flags new preemption flags
 */
INLINE void tcb_t::set_preempt_flags (preempt_flags_t flags)
{
    get_utcb()->preempt_flags = flags.raw;
}

/**
 * Get a thread's coprocessor flags
 * @return      coprocessor flags
 */
INLINE u8_t tcb_t::get_cop_flags (void)
{
    return get_utcb()->cop_flags;
}

/**
 * clear the notify_bits
 */
INLINE void tcb_t::clear_notify_bits()
{
    okl4_atomic_set(&get_utcb()->notify_bits, 0);
}

/**
 * add bits to the notify_word
 */
INLINE word_t tcb_t::add_notify_bits(const word_t bits)
{
    return okl4_atomic_or_return(&get_utcb()->notify_bits, bits);
}

/**
 * clear bits in the notify_word
 */
INLINE word_t tcb_t::sub_notify_bits(const word_t bits)
{
    word_t previous = okl4_atomic_read(&get_utcb()->notify_bits);
    okl4_atomic_and(&get_utcb()->notify_bits, ~(bits));
    return previous;
}

/**
 * read value of the notify_bits
 */
INLINE word_t tcb_t::get_notify_bits(void)
{
    return okl4_atomic_read(&get_utcb()->notify_bits);
}

/**
 * read value of the notify_mask
 */
INLINE word_t tcb_t::get_notify_mask(void)
{
    return get_utcb()->notify_mask;
}

/**
 * set the value of the notify_mask register
 * @param value value to set
 */
INLINE void tcb_t::set_notify_mask(const word_t mask)
{
    get_utcb()->notify_mask = mask;
}

/**
 * Get message tag
 * @returns message tag
 * The message tag will be read from message register 0
 */
INLINE msg_tag_t tcb_t::get_tag (void)
{
    msg_tag_t tag;
    tag.raw = get_mr(0);
    return tag;
}

/**
 * Set the message tag
 * @param tag   new message tag
 * The message tag will be stored in message register 0
 */
INLINE void tcb_t::set_tag (msg_tag_t tag)
{
    set_mr(0, tag.raw);
}

/**
 * set value of sender_space
 */
INLINE void tcb_t::set_sender_space(spaceid_t space_id)
{
    get_utcb()->sender_space = space_id;
}

/**
 * read value of sender_space
 */
INLINE spaceid_t tcb_t::get_sender_space()
{
    return get_utcb()->sender_space;
}

/**
 * enqueue a tcb into the present list
 * the present list primarily exists for debugging reasons, since task
 * manipulations now happen on a per-thread basis
 */
#ifdef CONFIG_DEBUG
extern tcb_t * global_present_list;
extern spinlock_t present_list_lock;
#endif

INLINE void tcb_t::enqueue_present()
{
#ifdef CONFIG_DEBUG
    present_list_lock.lock();
    ENQUEUE_LIST_TAIL(tcb_t, global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE void tcb_t::dequeue_present()
{
#ifdef CONFIG_DEBUG
    present_list_lock.lock();
    DEQUEUE_LIST(tcb_t, global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE void
tcb_t::reserve(void)
{
#if defined(CONFIG_MUNITS)
    okl4_atomic_inc(&reserved);
#endif
}

INLINE void
tcb_t::unreserve(void)
{
#if defined(CONFIG_MUNITS)
    okl4_atomic_dec(&reserved);
#endif
}

INLINE bool
tcb_t::is_reserved(void)
{
#if defined(CONFIG_MUNITS)
    return okl4_atomic_read(&reserved) != 0;
#else
    return false;
#endif
}

INLINE arch_ktcb_t *tcb_t::get_arch()
{
    return &this->arch;
}

/* Migrating domains is meaningless in not MP systems */
#ifdef CONFIG_MDOMAINS
INLINE bool tcb_t::migrate_to_domain(cpu_context_t context)
{
    // update the directory cache on migration between processors
    if (space) {
        this->page_directory = space->pgent(0, context.domain);
    }

#error FIXME: Add stuff to migrate to a new domain here

    return false;
}
#endif

INLINE void init_idle_tcb()
{
    extern tcb_t* __idle_tcb[];
    extern tcb_t* tcb_array;
    extern word_t num_tcbs;
#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
    int index = get_current_context().unit;
#else
    int index = 0;
#endif
    __idle_tcb[index] = (tcb_t *)((word_t)tcb_array + KTCB_SIZE * (num_tcbs - index - 1));
    memset(__idle_tcb[index], 0, KTCB_SIZE);
}

tcb_t * get_idle_tcb(cpu_context_t context) PURE;


INLINE tcb_t * get_idle_tcb(cpu_context_t context)
{
    extern tcb_t* __idle_tcb[];
#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
    return __idle_tcb[context.unit];
#else
    return __idle_tcb[0];
#endif
}

INLINE utcb_t * get_idle_utcb(cpu_context_t context)
{
    extern utcb_t __idle_utcb;
    return (utcb_t*)&__idle_utcb;
}

INLINE tcb_t * get_idle_tcb()
{
    return get_idle_tcb(get_current_context());
}

INLINE utcb_t * get_idle_utcb()
{
    return get_idle_utcb(get_current_context());
}

/*
 * Thread locking.
 */


/*
 * include glue header file
 */
#include <kernel/arch/tcb.h>

/**********************************************************************
 *
 *             global thread management
 *
 **********************************************************************/

INLINE space_t * get_current_space(void) PURE;

INLINE space_t * get_current_space(void)
{
    return get_current_tcb()->get_space();
}

INLINE bool tcb_t::is_local_unit()
{
#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
        /* on non-SMP always evaluates to true */
        return( get_current_context() == get_context());
#else
        return true;
#endif
}

INLINE bool tcb_t::is_local_domain()
{
#ifdef CONFIG_MDOMAINS
    return (cpu_context.scheduler_domain == get_current_context().scheduler_domain);
#else
    return true;
#endif
}

/**
 * Check UTCB location of thread is valid.  It is assumed that the
 * space of the thread is properly initialised.
 *
 * @return true if UTCB location is valid, false otherwise
 */
INLINE bool tcb_t::check_utcb_location ()
{
    return get_space()->check_utcb_location (get_utcb_location ());
}



/**
 * creates a root server thread and a fresh space, if the
 * creation fails the function does not return (assuming that root
 * servers are functional necessary for the system)
 *
 * @param dest_tid id of the thread to be created
 * @param scheduler_tid thread id of the scheduler
 * @param pager_tid thread id of the pager
 * @param except_handler_tid thread id of the exception handler
 * @param utcb_area fpage describing the UTCB area
 * @param kip_area fpage describing the kernel interface page area
 * @return the newly created tcb
 */
tcb_t * create_root_server(threadid_t dest_tid, threadid_t scheduler_tid,
                           threadid_t pager_tid, threadid_t except_handler_tid,
                           fpage_t utcb_area, fpage_t kip_area);

/**
 * initializes the root servers
 */
void init_root_servers();

/**
 * initializes the kernel threads
 */
void init_kernel_threads();

/**********************************************************************
 *
 *                      user access fault functions
 *
 **********************************************************************/

INLINE void
tcb_t::set_user_access(continuation_t cont)
{
    /* Ensure that we don't already user access enabled. It would be
     * ideal if we could ensure that nobody else is using this field at
     * the present time (for instance, visible kernel preemption), but
     * unfortunately this field is left with stale data in it after a
     * (timer-based) preemption continuation takes place.  */
    ASSERT(DEBUG, ((word_t)this->preemption_continuation & 1) == 0);

    /* Set continuation + user_access flag in LSB */
    this->preemption_continuation = (continuation_t)((word_t)cont | 1);
#if defined(ARCH_ENABLE_USER_ACCESS)
    ARCH_ENABLE_USER_ACCESS;
#endif

    okl4_atomic_compiler_barrier();
}

INLINE void
tcb_t::clear_user_access(void)
{
    okl4_atomic_compiler_barrier();

#if defined(ARCH_DISABLE_USER_ACCESS)
    ARCH_DISABLE_USER_ACCESS;
#endif
    ASSERT(DEBUG, (word_t)this->preemption_continuation & 1);
    this->preemption_continuation = NULL;

    okl4_atomic_compiler_barrier();
}

INLINE bool
tcb_t::user_access_enabled(void)
{
    return ((word_t)this->preemption_continuation & 1);
}

INLINE continuation_t
tcb_t::user_access_continuation(void)
{
    ASSERT(DEBUG, (word_t)this->preemption_continuation & 1);

    return (continuation_t)((word_t)this->preemption_continuation & (~1UL));
}

/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

void handle_ipc_error(void);

extern "C" void arm_return_from_notify0(void);
/**
 * register a function to be run next time thread is scheduled
 *
 * This function may not be called on a currently executing thread
 *
 * The function must be a continuation function. The thread must not
 * have a current continuation function set unless the function being
 * set is handle_ipc_error.
 *
 * Arguments to the function must be stored in the TCB
 *
 * This is not a control function - ie it will return normally, not by
 * activating the given continuation
 *
 * @param func notify procedure to invoke upon thread execution
 */
INLINE void
tcb_t::notify(continuation_t func)
{
    cont = (continuation_t)func;
}

INLINE bool
tcb_t::is_suspended()
{
    return suspended;
}

INLINE void
tcb_t::set_suspended(bool new_state)
{
    /* Ensure that the new state is different from the old state. */
    if (suspended) {
        ASSERT(DEBUG, !new_state && suspended);
    } else {
        ASSERT(DEBUG, new_state && !suspended);
    }

    suspended = new_state;
}

INLINE void
tcb_t::set_post_syscall_callback(callback_func_t func)
{
    post_syscall_callback = func;
}

INLINE callback_func_t
tcb_t::get_post_syscall_callback(void)
{
    return post_syscall_callback;
}

/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */

INLINE void generic_space_t::add_tcb(tcb_t * tcb)
{
    thread_count++;
    spaces_list_lock.lock();
    ENQUEUE_LIST_TAIL(tcb_t, thread_list, tcb, thread_list);
    spaces_list_lock.unlock();
    /* Ensure TCB is in space's page directory */
    (void)sync_kernel_space(tcb);
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE void generic_space_t::remove_tcb(tcb_t * tcb)
{
    thread_count--;
    spaces_list_lock.lock();
    DEQUEUE_LIST(tcb_t, thread_list, tcb, thread_list);
    spaces_list_lock.unlock();
}

#endif /* !__TCB_H__ */
