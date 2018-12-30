/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Description:   scheduling declarations
 */

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <types.h>
#include <tcb.h>
#include <arch/schedule.h>
#include <kdb/tracepoints.h>
#include <prioqueue.h>

EXTERN_TRACEPOINT(PREEMPTION_SIGNALED);

class syncpoint_t;

class scheduler_t
{
public:

    /**
     *  Scheduling flags
     *
     *  These flags indicate that a certain scheduling behaviour
     *  should be taken.
     *
     *  Flags to the scheduler inform the scheduler what behaviour
     *  should be taken for the particular invocation of the method.
     *
     *  @todo FIXME: Mothra issue #2066 - awiggins.
     */
    typedef word_t flags_t;

    /** Default scheduling flags */
    static const flags_t sched_default = 0;

    /**
     * Inform the scheduler that a given schedule() call is
     * potentially preempting the current thread. This allows the
     * scheduler to inform the thread if the thread so requires.
     */
    static const flags_t preempting_thread = 1 << 0;

    /*
     * Inform the scheduler that the given schedule() should
     * perform a round-robin if possible. (i.e., the currently
     * running thread should not be re-run, unless no other threads
     * at its priority are ready to go.
     */
    static const flags_t sched_round_robin = 1 << 1;

    /*
     * Initialises the scheduler, must be called before init.
     */
    void init(bool bootcpu = true);

    /**
     * Starts the scheduling, does not return.
     *
     * @param cpu Processor the scheduler starts on.
     */
    NORETURN void start(cpu_context_t context);

    /**
     * Schedule a runnable thread.
     *
     * @param current      The TCB of the currently running thread.
     * @param continuation Continuation of the currently running thread.
     * @param flags        Scheduling flags for the schedule.
     * @retval             true if runnable thread was found, false otherwise.
     */
    NORETURN void schedule(tcb_t * current, continuation_t continuation,
                           flags_t flags);

    /**
     * Donate the current schedule to a targeted thread.
     *
     * @pre dest_tcb->get_state().is_runnable()
     *
     * @param dest_tcb  The TCB of thread the current scheduler is
     *                  being donated to.
     * @param current   The TCB of the currently running thread doing
     *                  the donation.
     * @param continuation  Continuation of the current, donating thread.
     */
    NORETURN void donate(tcb_t * dest_tcb, tcb_t * current,
                continuation_t continuation);

    /**
     * Make the given thread runnable in the scheduler.
     *
     * @pre new_state.is_runnable().
     *
     * @param tcb       The TCB of the thread to be activated in the
     *                  scheduler as runnable.
     * @param new_state Thread's new runnable state.
     * @param flags     Scheduling flags.
     *
     * @todo FIXME: Mothra issue #2061 - awiggins.
     */
    void activate(tcb_t * tcb, thread_state_t new_state,
            flags_t flags = sched_default);

    /**
     * Remove the given thread from the scheduler and mark as blocked.
     *
     * @pre !new_state.is_runnable().
     *
     * @param tcb       The TCB of the thread to be blocked deactivated
     *                  from the scheduler.
     * @param new_state Thread's new blocked state.
     * @param flags     Scheduling flags.
     */
    void deactivate(tcb_t * tcb, thread_state_t new_state,
            flags_t flags = sched_default);

    /**
     * Make the given thread runnable in the scheduler and perform a scheduling
     * operation.
     *
     * @pre new_state.is_runnable().
     *
     * @param tcb           The TCB of the thread to be activated in the
     *                      scheduler as runnable.
     * @param new_state     Thread's new runnable state.
     * @param current       The TCB of the currently running thread.
     * @param continuation  Continuation of the currently running thread.
     * @param flags         Scheduling flags.
     */
    NORETURN void activate_sched(tcb_t * tcb, thread_state_t new_state,
            tcb_t * current, continuation_t continuation, flags_t flags);


    /**
     * Remove the given thread from the scheduler, mark as blocked and perform
     * a scheduling operation.
     *
     * @pre !new_state.is_runnable().
     *
     * @param tcb           The TCB of the thread to be blocked and deactivated
     *                      from the scheduler.
     * @param new_state     Thread's new blocked state.
     * @param current       The TCB of the currently running thread.
     * @param continuation  Continuation of the currently running thread.
     * @param flags         Scheduling flags.
     */
    NORETURN void deactivate_sched(tcb_t * tcb, thread_state_t new_state,
            tcb_t * current, continuation_t continuation, flags_t flags);

    /**
     * Dequeue a TCB from the scheduling queue, enqueue a different TCB to the
     * scheduling queue, and then perform a scheduling operation.
     *
     * @pre !new_inactive_state.is_runnable() &&
     *      new_active_state.is_runnable().
     *
     * @param deactivate_tcb     The TCB of the thread to be blocked and
     *                           deactivated from the scheduler.
     * @param activate_tcb       The TCB of the thread to be activated in the
     *                           scheduler as runnable.
     * @param new_inactive_state Deactivated thread's new blocked state.
     * @param new_active_state   Activated thread's new runnable state.
     * @param current            The TCB of the currently running thread.
     * @param continuation       Continuation of the currently running thread.
     * @param flags              Scheduling flags.
     */
    NORETURN void deactivate_activate_sched(tcb_t * deactivate_tcb,
            tcb_t * activate_tcb, thread_state_t new_inactive_state,
            thread_state_t new_active_state, tcb_t * current,
            continuation_t continuation, flags_t flags);

    /**
     * Update the state of a active (runnable) thread to another active state.
     *
     * @pre tcb.get_state().is_runnable() && new_state.is_runnable().
     *
     * @param tcb        The TCB of the thread to have its runnable state
     *                   updated.
     * @param new_state  Thread's new runnable state.
     */
    void update_active_state(tcb_t * tcb, thread_state_t new_state);

    /**
     * Update the state of a inactivated (blocked) thread to another inactive
     * state.
     *
     * @pre !tcb.get_state().is_runnable() && !new_state.is_runnable().
     *
     * @param tcb        The TCB of the thread to have its blocked state
     *                   updated.
     * @param new_state  Thread's new blocked state.
     */
    void update_inactive_state(tcb_t * tcb, thread_state_t new_state);

    /**
     * Ask the scheduler to perform a context switch to another thread. New
     * code should not use this function, but instead use schedule(),
     * activate_sched(), etc.
     *
     * The destination is switched to using the current thread's schedule.
     *
     * @param current       The currently executing tcb.
     * @param dest          Destination of the context switch.
     * @param continuation  Return continuation.
     */
    NORETURN void context_switch(tcb_t * current, tcb_t * dest,
            thread_state_t new_current_state, thread_state_t new_dest_state,
            continuation_t cont);

    /**
     * Pauses a (possibly) running thread.  If the thread is actually running
     * pause will not return until it has ceased execution.
     *
     * @param tcb  The TCB of the thread to pause
     */
    void pause(tcb_t *tcb);

    /**
     * Unpauses a previously paused thread.
     *
     * @param tcb  The TCB of the thread to unpause
     */
    void unpause(tcb_t *tcb);

private:
    /**
     * Add a thread to the ready queue.
     *
     * @param tcb  The TCB of the thread to be enqueued into the ready queue.
     */
    void enqueue(tcb_t * tcb);

    /**
     * Remove a thread from the ready queue.
     *
     * @param tcb  The TCB of the thread to be removed.
     */
    void dequeue(tcb_t * tcb);

public:

    /**
     * Sets the priority of a thread.
     *
     * It is likely a schedule() call will be required after this call to
     * ensure that the highest priority thread (which may be the thread that
     * just had its priority thread) is running.
     *
     * @param tcb   Thread control block.
     * @param prio  Priority of thread.
     */
    void set_priority(tcb_t * tcb, prio_t prio);

    /**
     * Sets the effective priority of a thread.
     *
     * Updates to effective priority using this function are not propogated
     * to other threads; it is expected that the caller will handle that.
     */
    void set_effective_priority(tcb_t *tcb, prio_t prio);

public:
    /**
     * Initialise the scheduling parameters of a TCB.
     *
     * @param tcb               Pointer to TCB.
     * @param prio              Priority of the thread.
     * @param timeslice_length  Length of time slice.
     */
    void init_tcb(tcb_t * tcb, prio_t prio, word_t timeslice_length,
            word_t context_bitmask);

    /**
     * Handles the timer interrupt event, walks the wait lists and makes a
     * scheduling decision.
     */
    void handle_timer_interrupt(bool wakeup, continuation_t);

#ifdef CONFIG_MUNITS

    /** @todo FIXME: doxygen header - awiggins. */
    NORETURN void do_smt_reschedule(continuation_t continuation);

    NORETURN void hwthread_timer_tick(tcb_t *, continuation_t);
    NORETURN void hwthread_timer_tick_fast(tcb_t *, continuation_t);

private:
    void process_domain_timer_tick();
#endif

public:
#if defined(CONFIG_KEEP_CURRENT_TIME)
    /**
     * Delivers the current time relative to the system startup in
     * microseconds.
     *
     * @return  Absolute time.
     */
    INLINE u64_t get_current_time(void);
#endif

    /**
     * Main part of schedule system call.
     *
     * @return  'True' if a schedule() call is required after this operation,
     *          'false' if the current thread may continue to run.
     */
    bool do_schedule(tcb_t * tcb, word_t ts_len, word_t prio, word_t flags);

#if defined(CONFIG_MUNITS)

    /** Query scheduling info on what is currently running. */
    INLINE tcb_t * get_active_thread(cpu_context_t context);

    /** update scheduling info on what is currently running */
    INLINE void change_active_thread_priority(tcb_t * tcb);

    INLINE void request_reschedule(cpu_context_t context);
    INLINE bool clear_reschedule_request(cpu_context_t context);

    /* Update the 'lowest_unit' data structure componant. Assumes that
     * 'schedule_lock' is already held. */
    void update_lowest_unit();

#endif

    void set_active_thread(tcb_t * tcb, tcb_t * schedule);

    void set_context_bitmask(tcb_t * tcb, word_t x);

    /**
     * Remove the given thread from all scheduling data structures in
     * preparation for the thread to be deleted.
     */
    void delete_tcb(tcb_t * tcb);

    /**
     * Expire the timeslice of the currently running thread and call the
     * scheduler.
     */
    NORETURN void yield(tcb_t * current, tcb_t * active_schedule,
            continuation_t continuation);

    /**
     * FIXME: These functions should only be used from syncpoints, and 
     * should be removed when the syncpoint locking scheme is updated
     */
    void scheduler_lock();
    void scheduler_unlock();
private:
    /**
     * Determine if the given thread could potentially run on another hardware
     * unit and, if so, request that hardware unit to perfrom a full schedule.
     */
    void smt_reschedule(tcb_t * tcb);

    /**
     * Release the lock on the current thread and, if the thread is runnable,
     * enqueue and schedule it to another hardware unit.
     */
    void release_and_reschedule_tcb(tcb_t * tcb);

    /**
     * Mark the given thread as having been preempted, allowing thread thread
     * to take action if it has preemption notification enabled.
     */
    void mark_thread_as_preempted(tcb_t * tcb);

    /** Flags controlling scheduling decisions made by 'fast_schedule'. */
    typedef enum {
        /** 'fast_schedule' should consider scheduling the 'current' tcb. */
        current_tcb_schedulable,

        /** 'fast_schedule' should not schedule the 'current' tcb. */
        current_tcb_unschedulable,
    } fast_schedule_type_e;

    /**
     * Perform a schedule operation, attempting to avoid a full scheduler
     * lookup if possible.
     *
     * It is assumed that none of the provided threads are currently on the
     * queue. All threads (other than the selected thread) will be queued.
     *
     * @param current   Currently running TCB
     * @param type      The type of fast_schedule operation to perform.
     * @param continuation 'current's continuation
     * @param flags     Scheduling flags
     */
    NORETURN void fast_schedule(tcb_t * current, fast_schedule_type_e type,
            continuation_t continuation, flags_t flags);

    /**
     * Perform a schedule operation, attempting to avoid a full scheduler
     * lookup if possible. All TCBs that could possibly be the highest priority
     * must be passed to the function.
     *
     * It is assumed that none of the provided threads are currently on the
     * queue. All threads (other than the selected thread) will be queued.
     *
     * @param current       Currently running TCB
     * @param type          The type of fast_schedule operation to perform.
     * @param tcb           TCB that may be the highest priority thread.
     * @param continuation  Current thread's continuation.
     * @param flags         Scheduling flags.
     */
    NORETURN void fast_schedule(tcb_t * current,
            fast_schedule_type_e type, tcb_t * tcb,
            continuation_t continuation, flags_t flags);

    /** Flags controlling scheduling decisions made by 'fast_schedule'. */
    typedef enum {
        run_current,
        run_new,
        full_schedule
    } run_e;

    /**
     * Determine if the scheduler should schedule the currently running
     * thread, or the newly enqueued thread.
     *
     * @param current
     *     The currently running thread.
     * @param tcb
     *     The newly enqueued thread.
     * @param can_schedule_current
     *     True if 'current' is schedulable.
     * @retval run_new
     *     New thread should be scheduled;
     * @retval run_current
     *     The current thread should be scheduled;
     * @retval full_schedule
     *     A full schedule should be performed.
     */
    run_e should_schedule_thread(tcb_t * current, tcb_t * tcb,
            bool can_schedule_current, flags_t flags);

    /**
     * Get the hardware contexts the given TCB is able to run on.
     */
    word_t get_context_bitmask(tcb_t * tcb);

    /**
     * Marks end of timeslice.
     *
     * @param tcb TCB of thread whose timeslice ends.
     */
    void end_of_timeslice(tcb_t * tcb);

    /**
     * Sets the timeslice length of a thread.
     *
     * @param tcb       Thread control block of the thread.
     * @param timeslice Timeslice length (must be a time period).
     */
    void set_timeslice_length(tcb_t * tcb, word_t timeslice);

    /**
     * Get the priority of the highest priority thread in the scheduler's
     * ready queue.
     *
     * @return  Next thread to be scheduled.
     */
    prio_t get_highest_priority();

#if defined(CONFIG_LOCKFREE_SCHEDULER)
    /**
     * Determine the highest priority thread in the scheduler's ready
     * queue, grab that thread and dequeue it.
     */
    tcb_t *find_next_thread();
#endif

    /**
     *  Remove a bit in the priority queue's bitmap tree.
     */
    void remove_sched_bitmap_bit(word_t level1_index, word_t level2_index);

    /** The scheduler's priority queue. */
    prio_queue_t prio_queue;

    /** Amount of time that has passed since system boot. */
    static volatile u64_t current_time;

public:
#if defined(CONFIG_MUNITS)
    /* Bitmap of units to reschedule */
    okl4_atomic_word_t reschedule_unit_requests;

    /* Lock of ipi and prio queue. */
    fifo_spinlock_t schedule_lock;

    /* The hardware unit with the lowest priority in this domain. */
    word_t lowest_unit;

    /* Threads currently running on each hardware thread. */
    struct {
        tcb_t * tcb;
        tcb_t * schedule;
    } smt_threads[CONFIG_MAX_UNITS_PER_DOMAIN];

    /* The priority that each thread is currently running on. */
    struct {
        /* The unit that the priority refers to. This is the same as the
         * index to the 'priorities' array, but is required as an
         * optimisation in certain fastpaths. */
        u16_t unit;

        /* The priority */
        s16_t prio;

    } priorities[CONFIG_MAX_UNITS_PER_DOMAIN];
#else
public:
    /* Lock of prio queue . */
    spinlock_t schedule_lock;
#endif

    /** Have the scheduler's scheduling invariants been violated? */
    bool scheduling_invariants_violated;

    friend void mkasmsym(void);
    friend void switch_from(tcb_t * current, continuation_t continuation);
    friend void switch_to(tcb_t * dest, tcb_t * schedule);
};

/**********************************************************************
 *
 *             Scheduler configuration helper methods
 *
 **********************************************************************/

INLINE void
scheduler_t::enqueue(tcb_t * tcb)
{
    ASSERT(DEBUG, tcb != get_idle_tcb());

    /* If the thread is reserved by another hardware unit, we don't ever
     * enqueue it again. */
    if (tcb->is_reserved()) {
        return;
    }

    prio_queue.enqueue(tcb);
}

INLINE void
scheduler_t::dequeue(tcb_t * tcb)
{
    ASSERT(DEBUG, tcb != get_idle_tcb());

    prio_queue.dequeue(tcb);
}

INLINE NORETURN void
scheduler_t::fast_schedule(tcb_t * current, fast_schedule_type_e type,
                           continuation_t continuation,
                           flags_t flags = sched_default)
{
    bool can_schedule_current = (type == current_tcb_schedulable);

    /* 'current' is the highest priority thread in the system. */
    if (EXPECT_TRUE(can_schedule_current && !scheduling_invariants_violated)) {
        ACTIVATE_CONTINUATION(continuation);
    }

    /* Otherwise, do a full lookup */
    schedule(current, continuation, flags);
}

INLINE scheduler_t::run_e
scheduler_t::should_schedule_thread(tcb_t * current, tcb_t * tcb,
        bool can_schedule_current, flags_t flags)
{
    /* If the system is in an invalid state, perform a full schedule. */
    if (EXPECT_FALSE(scheduling_invariants_violated)) {
        return full_schedule;
    }

#if defined(CONFIG_MUNITS)
    /* Don't allow reserved threads to be scheduled. */
    if (EXPECT_FALSE(tcb->is_reserved() || current->is_reserved())) {
        return full_schedule;
    }
#endif

#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    /* Ensure that the thread is able to run on this hardware unit. */
    if (EXPECT_FALSE((tcb->context_bitmask
                    & (1UL << get_current_context().unit)) == 0)) {
        if (EXPECT_TRUE(can_schedule_current)) {
            return run_current;
        }
        return full_schedule;
    }
#endif

    /* If the current thread has a higher priority and can be run,
     * schedule it. */
    if (EXPECT_TRUE(can_schedule_current
                && current->effective_prio >= tcb->effective_prio)) {
        return run_current;
    }

    /* If the new thread has a higher priority than the current,
     * schedule it. */
    if (tcb->effective_prio >= current->effective_prio) {
        return run_new;
    }

#if !defined(CONFIG_CONTEXT_BITMASKS)
    /* Search the queue. */
    schedule_lock.lock();
    prio_t next_prio = get_highest_priority();
    schedule_lock.unlock();
    if (tcb->effective_prio >= next_prio) {
        return run_new;
    }
#endif

    /* Still don't know. Full schedule required. */
    return full_schedule;
}

INLINE word_t
scheduler_t::get_context_bitmask(tcb_t * tcb)
{
#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    ASSERT(DEBUG, tcb);
    return tcb->context_bitmask;
#else
    return (word_t)-1;
#endif
}

INLINE void
scheduler_t::set_timeslice_length(tcb_t * tcb, word_t timeslice)
{
    ASSERT(DEBUG, tcb);
    tcb->current_timeslice = tcb->timeslice_length = timeslice;
}

/**********************************************************************
 *
 *                   Public Scheduler Methods
 *
 **********************************************************************/

INLINE void
scheduler_t::activate(tcb_t * tcb, thread_state_t new_state, flags_t flags)
{
    ASSERT(DEBUG, new_state.is_runnable());
    ASSERT(DEBUG, !tcb->get_state().is_runnable());
    ASSERT(DEBUG, tcb != get_idle_tcb());

    schedule_lock.lock();
    tcb->set_state(new_state);
    release_and_reschedule_tcb(tcb);
    schedule_lock.unlock();
}

INLINE void
scheduler_t::deactivate(tcb_t * tcb, thread_state_t new_state, flags_t flags)
{
    ASSERT(DEBUG, !new_state.is_runnable());
    ASSERT(DEBUG, tcb->get_state().is_runnable());
    ASSERT(DEBUG, tcb != get_idle_tcb());

    tcb->set_state(new_state);
    schedule_lock.lock();
    if (tcb->ready_list.is_queued()) {
        dequeue(tcb);
    }
    schedule_lock.unlock();
}

INLINE NORETURN void
scheduler_t::donate(tcb_t * dest_tcb, tcb_t * current,
                    continuation_t continuation)
{
    ASSERT(ALWAYS, current->get_state().is_runnable());

    schedule_lock.lock();

    /* Ensure that the destination is runnable and can be grabbed. */
    if (!dest_tcb->get_state().is_runnable()
            || !dest_tcb->ready_list.is_queued() || !dest_tcb->grab()) {
        /* Can't do a donation, so yield instead. */
        schedule_lock.unlock();
        yield(current, get_active_schedule(), continuation);
        NOTREACHED();
    }

    /* Remove it from the scheduling queue. */
    dequeue(dest_tcb);

    /* Switch to the destination. */
    switch_from(current, continuation);
    current->release();
    enqueue(current);
    schedule_lock.unlock();
    switch_to(dest_tcb, get_active_schedule());
}

INLINE void
scheduler_t::activate_sched(tcb_t * tcb, thread_state_t new_state,
                            tcb_t * current, continuation_t continuation,
                            flags_t flags)
{
    ASSERT(DEBUG, new_state.is_runnable());
    ASSERT(DEBUG, !tcb->get_state().is_runnable());
    ASSERT(ALWAYS, tcb != get_idle_tcb());
    ASSERT(ALWAYS, tcb != current);

    tcb->set_state(new_state);
    fast_schedule(current, current_tcb_schedulable, tcb, continuation, flags);
}

INLINE void
scheduler_t::deactivate_sched(tcb_t * tcb, thread_state_t new_state,
                              tcb_t * current, continuation_t continuation,
                              flags_t flags)
{
    ASSERT(DEBUG, !new_state.is_runnable());
    ASSERT(DEBUG, tcb->get_state().is_runnable());
    ASSERT(ALWAYS, tcb != get_idle_tcb());

    /* Update the deactivated thread's state. */
    tcb->set_state(new_state);

    if (tcb != current) {
        /* Dequeue the thread if it is not already dequeued.
         * Paused threads, for instance, will already be off
         * the queue. */
        if (tcb->ready_list.next != NULL) {
            schedule_lock.lock();
            dequeue(tcb);
            schedule_lock.unlock();
        }

        /* Release the lock on the thread. */
        tcb->release();
    }
    /* No need to perform a dequeue of the thread if it is the
     * currently running thread
     */
    fast_schedule(current, current != tcb ?
                  current_tcb_schedulable : current_tcb_unschedulable,
                  continuation, flags);
}

INLINE void
scheduler_t::deactivate_activate_sched(tcb_t * deactivated_tcb,
                                       tcb_t * activated_tcb,
                                       thread_state_t new_inactive_state,
                                       thread_state_t new_active_state,
                                       tcb_t * current,
                                       continuation_t continuation,
                                       flags_t flags)
{
    ASSERT(DEBUG, deactivated_tcb->get_state().is_runnable());
    ASSERT(DEBUG, !activated_tcb->get_state().is_runnable());
    ASSERT(DEBUG, !new_inactive_state.is_runnable());
    ASSERT(DEBUG, new_active_state.is_runnable());
    ASSERT(ALWAYS, activated_tcb != get_idle_tcb());
    ASSERT(ALWAYS, deactivated_tcb != get_idle_tcb());
    ASSERT(DEBUG, activated_tcb != current);

    if (deactivated_tcb != current) {
        schedule_lock.lock();
        dequeue(deactivated_tcb);
        schedule_lock.unlock();
    }

    deactivated_tcb->set_state(new_inactive_state);
    activated_tcb->set_state(new_active_state);

    if (deactivated_tcb != current) {
        deactivated_tcb->release();
    }

    fast_schedule(current,
            current != deactivated_tcb
            ? current_tcb_schedulable : current_tcb_unschedulable,
            activated_tcb, continuation, flags);
}

INLINE void
scheduler_t::update_active_state(tcb_t * tcb, thread_state_t new_state)
{
    ASSERT(DEBUG, tcb->get_state().is_runnable());
    ASSERT(DEBUG, new_state.is_runnable());

    tcb->set_state(new_state);
}

INLINE void
scheduler_t::update_inactive_state(tcb_t * tcb, thread_state_t new_state)
{
    ASSERT(DEBUG, !tcb->get_state().is_runnable());
    ASSERT(DEBUG, !new_state.is_runnable());

    tcb->set_state(new_state);
}

INLINE void
scheduler_t::set_priority(tcb_t * tcb, prio_t prio)
{
    ASSERT(DEBUG, prio >= 0 && prio <= MAX_PRIO);
    schedule_lock.lock();

    /* Update the base priority of the thread. */
    prio_queue.set_base_priority(tcb, prio);

    /* Propagate the priority changes. */
    if (tcb->get_waiting_for() != NULL) {
        tcb->get_waiting_for()->refresh(tcb);
    }

#ifdef CONFIG_MUNITS
    change_active_thread_priority(tcb);
#endif
    schedule_lock.unlock();
}

INLINE void
scheduler_t::init_tcb(tcb_t * tcb, prio_t prio = DEFAULT_PRIORITY,
                      word_t timeslice_length = DEFAULT_TIMESLICE_LENGTH,
                      word_t context_bitmask = DEFAULT_CONTEXT_BITMASK)
{
    set_timeslice_length(tcb, timeslice_length);
    set_priority(tcb, prio);
    set_context_bitmask(tcb, context_bitmask);
}

#if defined(CONFIG_KEEP_CURRENT_TIME)
INLINE u64_t
scheduler_t::get_current_time(void)
{
    /* no real need to lock here */
    return current_time;
}
#endif

#if defined(CONFIG_MUNITS)

INLINE void
scheduler_t::change_active_thread_priority(tcb_t * tcb)
{
    SMT_ASSERT(NORMAL, schedule_lock.is_locked(true));

    word_t i;
    ASSERT(NORMAL, tcb);

    for (i = 0; i < CONFIG_MAX_UNITS_PER_DOMAIN; i++) {
        if (smt_threads[i].schedule == tcb) {
            priorities[i].prio = tcb->effective_prio;
        }
    }
}

INLINE void
scheduler_t::request_reschedule(cpu_context_t context)
{
    okl4_atomic_or(&reschedule_unit_requests, 1UL << context.unit);
}

INLINE bool
scheduler_t::clear_reschedule_request(cpu_context_t context)
{
    word_t val = okl4_atomic_and_return_old(&reschedule_unit_requests, ~(1ul << context.unit));
    return (val & (1ul << context.unit)) != 0;
}

#endif

INLINE void
scheduler_t::set_context_bitmask(tcb_t * tcb, word_t x)
{
    ASSERT(DEBUG, tcb);
#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    tcb->context_bitmask = x;
#endif
}

INLINE void
scheduler_t::delete_tcb(tcb_t * tcb)
{
    ASSERT(ALWAYS, tcb->is_grabbed_by_me());
    ASSERT(ALWAYS, !tcb->ready_list.is_queued());

    tcb->set_state(thread_state_t::aborted);
    tcb->base_prio = 0;
    tcb->effective_prio = 0;
}

/** Mark the given thread as having been preempted, allowing the
 *  thread to take action if it has preemption notification
 *  enabled. */
INLINE void
scheduler_t::mark_thread_as_preempted(tcb_t * tcb)
{
    /* Has the thread requested to be signalled if it is preempted? */
    if (EXPECT_FALSE(tcb->get_preempt_flags().is_signaled())) {

        /* Yes --- let it know about the preemption when it next wakes up. */
        TRACEPOINT(PREEMPTION_SIGNALED,
                   printf("preemption signalled for %t\n", tcb));

        tcb->set_preempted_ip(tcb->get_user_ip());
        tcb->set_user_ip(tcb->get_preempt_callback_ip());
    }
}

INLINE NORETURN void
scheduler_t::context_switch(tcb_t * current, tcb_t * dest,
        thread_state_t new_current_state, thread_state_t new_dest_state,
        continuation_t cont)
{
    /*
     * After a switch_to(), we don't know anything about the state of
     * the system. Perform a full schedule (with no funky optimisations)
     * next time we have to schedule a new thread.
     */
    ASSERT(ALWAYS, dest->is_grabbed_by_me());
    ASSERT(ALWAYS, current != get_idle_tcb());

    current->set_state(new_current_state);
    dest->set_state(new_dest_state);
    schedule_lock.lock();
    scheduling_invariants_violated = true;
    switch_from(current, cont);
    current->release();
    if (current->get_state().is_runnable()) {
        enqueue(current);
    }
    schedule_lock.unlock();
    switch_to(dest, get_active_schedule());
}

#if !defined(CONFIG_MUNITS)
INLINE void
scheduler_t::pause(tcb_t *tcb)
{
}

INLINE void
scheduler_t::unpause(tcb_t *tcb)
{
}
#endif

/**********************************************************************
 *
 *                  Global function declarations
 *
 **********************************************************************/

/**
 * @fn tcb_t * get_active_schedule(void)
 *
 * Return a pointer to thread whose schedule we are currently using.
 *
 * This may not be the same as get_current_tcb() if schedule inheritance
 * is enabled on this thread and  this thread is currently donating to
 * another  thread.
 *
 * @return  TCB of the thread whose schedule (scheduling properties)
 *          are currently be used and accounted to.
 */

/**
 * @fn void set_active_schedule(tcb_t * tcb)
 *
 * Store the thread whose schedule that we are currently running on.
 *
 * @param tcb  The TCB of the thread whose schedule is to be used and
 *             accounted to.
 */

/**
 * @return The current scheduler.
 */
CONST INLINE scheduler_t *
get_current_scheduler(void)
{
    extern scheduler_t __scheduler;

    return &__scheduler;
}

#if defined(CONFIG_KEEP_CURRENT_TIME)
/** Get the current l4 scheduler time */
INLINE u64_t
get_current_time(void)
{
    return get_current_scheduler()->get_current_time();
}
#endif

INLINE void scheduler_t::scheduler_lock()
{
    schedule_lock.lock();
}

INLINE void scheduler_t::scheduler_unlock()
{
    schedule_lock.unlock();
}

#endif /* !__SCHEDULE_H__ */
