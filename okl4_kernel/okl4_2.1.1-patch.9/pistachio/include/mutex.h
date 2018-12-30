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
/**
 * @file Mutex syncronisation object.
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <kernel/macros.h>
#include <kernel/mutexid.h>

#if defined(CONFIG_DEBUG)

/**
 *  The global list of mutexes present in the kernel.
 *
 *  Note this list is only maintained for use by the OKL4 kernel
 *  debugger.
 */
extern mutex_t * global_mutexes_list;

/** Global mutex preset lists lock. */
extern spinlock_t mutex_list_lock;

#endif

/** Initialise the mutex naming subsystem. */
void SECTION(SEC_INIT) init_mutex(void);

class mutex_flags_t;
class mutex_user_t;

/** Mutex objects. Mutexes are a form of synchronisation point for users. */
class mutex_t
{
public:

    /**
     *  Initialise this mutex object.
     *
     *  @param id Indentifier of this mutex being created.
     */
    void init(mutexid_t id);

    /** Tear down this mutex object. */
    void destroy(void);

    /** Is the mutex currently held by any threads. */
    bool is_held(void);

    /** Is the mutex currently contended, i.e. does it have any blockers? */
    bool is_contended(void);

    /** The current holder of the mutex object. */
    tcb_t * get_holder(void);

    /**
     *  Set the current holder of the mutex.
     *
     *  @param Reference to the thread that is holding the mutex.
     */
    void set_holder(tcb_t * holder);

    /**
     * Try to lock/acquire a mutex object.
     *
     * @param current The TCB of the current thread.
     * @param blocking Is the lock blocking or non-blocking (trylock).
     * @param continuation The continuation to return from the system call.
     */
    NORETURN void lock(tcb_t * current, bool blocking,
            continuation_t continuation);

    /**
     * Unlock/release a mutex object.
     *
     * @param current The TCB of the current thread.
     * @param continuation The continuation to return from the system call.
     */
    NORETURN void unlock(tcb_t * current, continuation_t continuation);

    /**
     *  Try to lock/acquire a hybrid mutex object.
     *
     *  @param current The TCB of the current thread.
     *  @param user_mutex The user-level object tied to the kernel mutex.
     *  @param continuation The continuation to return from the system call.
     */
    NORETURN void hybrid_lock(tcb_t * current, mutex_user_t user_mutex,
            continuation_t continuation);

    /**
     *  Unlock/release a hybrid mutex object.
     *
     *  @param current The TCB of the current thread.
     *  @param user_mutex The user-level object tied to the kernel mutex.
     *  @param continuation The continuation to return from the system call.
     */
    NORETURN void hybrid_unlock(tcb_t * current, mutex_user_t user_mutex,
            continuation_t continuation);

    /**
     * Attempt to acquire this mutex.
     *
     * @param candidate The TCB pointer of the thread attempting to
     * acquire this mutex.
     *
     * @return            The TCB pointer to the thread already holding the
     *                    mutex if such a thread exists.
     * @retval candidate  Condidate recursively acquired this mutex object.
     * @retval NULL       Candidate acqired this mutex object.
     */
    tcb_t * acquire(tcb_t * candidate);

    /**
     *  Release the mutex.
     *
     *  @pre this->is_locked(true);
     *
     *  @post returns NULL or a grabbed thread reference.
     *
     *  @return The new holder of the mutex.
     *  @retval NULL No threads were blocked on the mutex.
     */
    tcb_t * release();

    /**
     * The syncpoint used to represent the scheduling properties of
     * this mutex.
     *
     * @todo FIXME: sync_point should be a private member of the class.
     */
    syncpoint_t sync_point;

    /**
     * Mutex data-structure lock.
     *
     * @todo FIXME: Review use of spinlock, if retained should it be
     * private? - awiggins.
     */
    spinlock_t mutex_lock;

    /**
     * List of mutexes currently held by a given TCB.
     */
    ringlist_t<mutex_t> held_list;

#if defined(CONFIG_MUTEX_NAMES)
    /**
     * Human-readable debugging name for use in KDB.
     */
    char debug_name[MAX_DEBUG_NAME_LENGTH];
#endif

#if defined(CONFIG_DEBUG)

    /** Links for the global list of present mutexes. */
    ringlist_t<mutex_t> present_list;

    /** The ID of this mutex object. */
    mutexid_t id;

#endif

private:

    /**
     * Add this mutex to the list of mutexes held by 'holder'.
     *
     * @param The mutex this thread has just acquired.
     */
    void enqueue_held(tcb_t * holder);

    /**
     * Remove this mutex from the list of mutexes held by 'holder'.
     *
     * @param The mutex this thread has just released.
     */
    void dequeue_held(tcb_t * holder);

    /** Insert this mutex into the global list of mutexes. */
    void enqueue_present(void);

    /** Remove this mutex from the global list of mutexes. */
    void dequeue_present(void);
};

/** Mutex system call flags. */
class mutex_flags_t {
public:
    /**
     * Is the mutex operation a lock operation.
     *
     * @retval true Mutex system call is a lock/trylock operation.
     * @retval false Mutex system call is a unlock operation.
     */
    bool is_lock(void);

    /**
     * Is the mutex a blocking lock operation.
     *
     * @pre is_lock()
     *
     * @retval true Mutex system call is a lock operation.
     * @retval false Mutex system call is a trylock operation.
     */
    bool is_blocking(void);

    /**
     *  Is the mutex a hybrid (kernel/user) mutex operation.
     *
     *  @retval true Mutex system call is a hybrid operation.
     *  @retval false Mutex system call is a kernel-only mutex operation.
     */
    bool is_hybrid(void);

    /**
     * Constructor from assignment.
     *
     * @param value The numeric representation of the new flags.
     */
    void operator = (word_t value);

private:
    union {
        struct {
            BITFIELD4(word_t,
                      lock       : 1,
                      blocking   : 1,
                      hybrid     : 1,
                      __reserved : BITS_WORD - 3);
        } x;
        word_t raw;
    };
};

/** Hybrid mutex user word. */
class mutex_user_t
{
public:

    /**
     *  Constructor from assignment of user virtual address.
     *
     *  @param state_p The user's virtual address of the mutex state.
     */
    void operator = (word_t * state_p);

    /**
     *  Constructor from assignment of another user_mutex.
     *
     *  @param user_mutex The user mutex being copied.
     */
    void operator = (mutex_user_t user_mutex);

    /**
     *  Is this user mutex object valid, not.
     *
     *  @retval true Valid user mutex object.
     *  @retval false Invalid user mutex object.
     */
    bool is_valid(void);

    /** Return this user's virtual address for this mutex object. */
    word_t * get_state_p(void);

    /**
     *  Get the holder of the user mutex object.
     *
     *  @param user_thread Reference to the thread providing mutex_t::state_p.
     *
     *  @return The thread number of the holder of the user mutex object.
     */
    word_t get_holder(tcb_t * user_thread);

    /**
     *  Set the holder of the user mutex object.
     *
     *  @param user_thread Reference to the thread providing mutex_t::state_p.
     *  @param holder Thread number of the holder of the user mutex object.
     *  @param contended If the mutex is currently contended or not.
     */
    void set_holder(tcb_t * user_thread, word_t holder, bool contended);

    /**
     *  Set the user mutex object as free.
     *
     *  @param user_thread Reference to the thread providing mutex_t::user_ptr.
     */
    void set_free(tcb_t * user_thread);

    /** @todo FIXME: Doxygen header - awiggins. */
    bool atomic_set_holder(tcb_t * user_thread, word_t holder);

    /** @todo FIXME: Doxygen header - awiggins. */
    bool atomic_set_contended(tcb_t * user_thread);

private:
    static const word_t MUTEX_FREE = ~(word_t)0;
    static const word_t CONTENTION_BIT = (word_t)1 << (WORD_T_BIT - 1);

    static word_t user_state_is_free(word_t user_state);
    static word_t user_state_is_uncontended(word_t user_state);
    static word_t user_state_is_contended(word_t user_state);
    static void user_state_set_contended(word_t & user_state);
    static void user_state_set_uncontended(word_t & user_state);

    /* The user's virtual address for the mutex object. */
    word_t * state_p;
};


/*
 *  mutex_t class methods.
 */

INLINE bool
mutex_t::is_held(void)
{
    return this->get_holder() != NULL;
}

INLINE bool
mutex_t::is_contended(void)
{
    return this->sync_point.has_blocked();
}

INLINE tcb_t *
mutex_t::get_holder(void)
{
    return this->sync_point.get_donatee();
}

/*
 *  mutex_flags_t class methods.
 */

INLINE bool
mutex_flags_t::is_lock(void)
{
    return this->x.lock;
}

INLINE bool
mutex_flags_t::is_blocking(void)
{
    return this->x.blocking;
}

INLINE bool
mutex_flags_t::is_hybrid(void)
{
    return this->x.hybrid;
}

INLINE void
mutex_flags_t::operator = (word_t value)
{
    this->raw = value;
}

/*
 *  mutex_user_t class methods.
 */
INLINE bool
mutex_user_t::is_valid(void)
{
    return this->state_p != NULL;
}

INLINE void
mutex_user_t::operator = (word_t * ptr)
{
    this->state_p = ptr;
}

INLINE void
mutex_user_t::operator = (mutex_user_t user_mutex)
{
    this->state_p = user_mutex.state_p;
}

INLINE word_t *
mutex_user_t::get_state_p(void)
{
    return state_p;
}

INLINE word_t
mutex_user_t::user_state_is_free(word_t user_state)
{
    return user_state == MUTEX_FREE;
}

INLINE word_t
mutex_user_t::user_state_is_uncontended(word_t user_state)
{
    return user_state & CONTENTION_BIT;
}

INLINE word_t
mutex_user_t::user_state_is_contended(word_t user_state)
{
    return !user_state_is_uncontended(user_state);
}

INLINE void
mutex_user_t::user_state_set_contended(word_t & user_state)
{
    user_state &= ~CONTENTION_BIT;
}

INLINE void
mutex_user_t::user_state_set_uncontended(word_t & user_state)
{
    user_state |= CONTENTION_BIT;
}


#endif /* !__MUTEX_H__ */
