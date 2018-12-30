/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Copyright (c) 2005-2006, National ICT Australia (NICTA)
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
 * Description:   Scheduling functions
 */
#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <interrupt.h>
#include <queueing.h>
#include <syscalls.h>
#include <smp.h>
#include <cache.h>
#include <config.h>
#include <arch/special.h>
#include <platform_support.h>
#include <arch/platform.h>

#include <kdb/tracepoints.h>
#include <kdb/console.h>

#include <mp.h>

#include <profile.h>

volatile u64_t scheduler_t::current_time = 0;

DECLARE_TRACEPOINT(SYSCALL_THREAD_SWITCH);
DECLARE_TRACEPOINT(SYSCALL_SCHEDULE);
DECLARE_TRACEPOINT(TIMESLICE_EXPIRED);
DECLARE_TRACEPOINT(PREEMPTION_FAULT);
DECLARE_TRACEPOINT(PREEMPTION_SIGNALED);

CONTINUATION_FUNCTION(idle_thread);

#if defined (CONFIG_MUNITS)
void
scheduler_t::update_lowest_unit()
{
    /* Update the global variable 'lowest_unit'. We assume that the ipc_lock is
     * already held by us. */
    prio_t lowest = MAX_PRIO + 1;
    for (word_t unit = 0; unit < CONFIG_MAX_UNITS_PER_DOMAIN; unit++) {
        if (priorities[unit].prio < lowest) {
            lowest = priorities[unit].prio;
            lowest_unit = unit | (lowest<<16);
        }
    }
}
#endif /* CONFIG_MUNITS */

void
scheduler_t::set_active_thread(tcb_t * tcb, tcb_t * schedule)
{
    set_active_schedule(schedule);

#if defined(CONFIG_MUNITS)
    ASSERT(ALWAYS, tcb);
    ASSERT(ALWAYS, schedule);
    word_t unit = get_current_context().unit;

    /* Update the data structures. */
    smt_threads[unit].tcb = tcb;
    smt_threads[unit].schedule = schedule;
    priorities[unit].prio = schedule->effective_prio;

    /* Update data structure's lowest unit. */
    update_lowest_unit();
#if defined(CONFIG_CONTEXT_BITMASKS)
    /* This could be a wakeup from sleep, reenable timer interrupt */
    Platform::enable_timer_tick();
#endif
#endif /* CONFIG_MUNITS */
}

/**
 * find_next_thread: selects the next tcb to be dispatched
 *
 * returns the selected tcb, if no runnable thread is available,
 * the idle tcb is returned.
 */
#if defined(CONFIG_CONTEXT_BITMASKS)
tcb_t *
scheduler_t::find_next_thread()
{
    SMT_ASSERT(ALWAYS, schedule_lock.is_locked(true));

    /* Generate bitmask for current context. */
    word_t unit = 1 << get_current_context().unit;

    word_t index_bitmap = prio_queue.index_bitmap;

    /* Search down the first-level bitmap. */
    while (index_bitmap) {
        word_t index = msb(index_bitmap);
        word_t bitmap = prio_queue.prio_bitmap[index];

        /* Search down the second-level bitmap. */
        while (bitmap) {
            /* Determine highest priority. */
            word_t bit = msb(bitmap);
            word_t prio = bit + (index * (BITS_WORD));

            /* Search through this priority. */
            tcb_t *head = prio_queue.get(prio);
            tcb_t *first = head;
            ASSERT(ALWAYS, head != NULL);
            do {
                /* Does it run on this unit? */
                if ((head->context_bitmask & unit) != 0) {
                    /* Yes, return it back to the caller. */
                    (void)head->grab();
                    ASSERT(ALWAYS, head->is_grabbed_by_me());
                    dequeue(head);
                    return head;
                }
                head = head->ready_list.next;
            } while (head != first);

            /* Clear this bit, and keep searching at this level. */
            bitmap &= ~(1UL << bit);
        }

        /* Clear this bit, and keep searching. */
        index_bitmap &= ~(1UL << index);
    }

    /* We can't find a schedulable thread; switch to idle. */
    tcb_t *idle = get_idle_tcb();
    (void)idle->grab();
    ASSERT(ALWAYS, idle->is_grabbed_by_me());
    return idle;
}
#else
prio_t
scheduler_t::get_highest_priority(void)
{
    SMT_ASSERT(ALWAYS, schedule_lock.is_locked(true));

    /* Determine the second-level bitmap to use. */
    word_t first_level_bitmap = prio_queue.index_bitmap;
    if (!first_level_bitmap) {
        return (prio_t)-1;
    }
    word_t first_level_index = msb(first_level_bitmap);

    /* Fetch the second-level bitmap. */
    word_t second_level_bitmap = prio_queue.prio_bitmap[first_level_index];
    ASSERT(ALWAYS, second_level_bitmap);
    word_t second_level_index = msb(second_level_bitmap);

    /* Calculate the top priority. */
    prio_t top_prio = first_level_index * BITS_WORD + second_level_index;

    return top_prio;
}

#endif

void
scheduler_t::set_effective_priority(tcb_t *tcb, prio_t prio)
{
    SMT_ASSERT(ALWAYS, schedule_lock.is_locked(true));
    ASSERT(DEBUG, prio >= 0 && prio <= MAX_PRIO);

    prio_queue.set_effective_priority(tcb, prio);
#ifdef CONFIG_MUNITS
    change_active_thread_priority(tcb);
#endif
}

/**
 * Selects the next runnable thread and activates it.  Will insert the
 * current thread into the ready queue if it is not already present.
 *
 * This function updates the scheduled variable in the current tcb to
 * true if the schedule was successful (another thread was switched
 * to) or false if the schedule was unsuccessful.
 *
 * @param current The currently executing tcb
 * @param continuation The continuation to activate upon completion
 *
 * @return true if a runnable thread was found, false otherwise
 */
#if defined(CONFIG_CONTEXT_BITMASKS) || (!defined(CONFIG_ENABLE_FASTPATHS) || !defined(HAVE_SCHEDULE_FASTPATH))
void
scheduler_t::schedule(tcb_t * current, continuation_t continuation,
                      flags_t flags)
{
    ASSERT(DEBUG, current);
    ASSERT(ALWAYS, !current->ready_list.is_queued());
    ASSERT(ALWAYS, current->is_grabbed_by_me());

#if defined(CONFIG_CONTEXT_BITMASKS)

    /* Switch away from the current thread. */
    switch_from(current, continuation);
    schedule_lock.lock();
    current->release();


    /* Enqueue the thread if necessary. */
    bool current_runnable = current->get_state().is_runnable()
        && !current->is_reserved();
    if (current_runnable && current != get_idle_tcb()) {
        enqueue(current);
    }

    /* Find the next thread */
    tcb_t *next = find_next_thread();
    ASSERT(ALWAYS, next->is_grabbed_by_me());

    /* Switch to the new thread. */
    set_active_thread(next, next);
    if (next != current && current_runnable && current != get_idle_tcb()) {
        smt_reschedule(current);
    }

    schedule_lock.unlock();

    switch_to(next, next);

#else
    schedule_lock.lock();

    /* No longer violating the scheduler. */
    scheduling_invariants_violated = false;

    /* Determine the next priority thread. */
    prio_t max_prio = get_highest_priority();

    /* If the current thread has an equal priority, we can keep running
     * it. */
    bool current_runnable = current->get_state().is_runnable()
        && !current->is_reserved();
    if (current_runnable) {
        /* If we are the highest priority, or we are not doing a round
         * robin and we are equal highest, just keep running current. */
        if (current->effective_prio > max_prio ||
                (!(flags & sched_round_robin)
                        && current->effective_prio == max_prio)) {
            schedule_lock.unlock();
            ACTIVATE_CONTINUATION(continuation);
        }
        /* If we are pre-empting the thread and the thread has asked for
         * pre-emption notification, inform the thread of this schedule. */
        if (flags & preempting_thread) {
            mark_thread_as_preempted(current);
        }

    }

    /* Otherwise, we are switching to another thread. */
    tcb_t *next;
    if (max_prio >= 0) {
        next = prio_queue.get(max_prio);
        dequeue(next);
    } else {
        next = get_idle_tcb();
    }

    /* Grab the thread. */
    (void)next->grab();
    ASSERT(ALWAYS, next->is_grabbed_by_me());

    /* Switch away from the current thread, enqueuing if necessary. */
    switch_from(current, continuation);
    current->release();
    if (current_runnable && current != get_idle_tcb()) {
        enqueue(current);
        smt_reschedule(current);
    }

    /* Activate the new thread. */
    set_active_thread(next, next);
    schedule_lock.unlock();
    switch_to(next, next);
#endif
}
#endif

extern "C" void SECTION(".init")
c_schedule(scheduler_t * scheduler, tcb_t * current,
        continuation_t continuation, word_t flags)
{
    scheduler->schedule(current, continuation, flags);
}

/**
 * Expire the given schedule, moving the owner of the schedule to the back of
 * the queue associated with its priority.
 *
 * For SMT systems, scheduler is assumed to be locked before function entry
 */
void
scheduler_t::end_of_timeslice(tcb_t * schedule)
{
    ASSERT(DEBUG, schedule != get_idle_tcb());

    /*
     * Pull the thread to the back of the queue by removing it
     * from the list and appending it to the end. Other threads
     * should remain in the same order.
     *
     * If it is the end of the timeslice for the currently running
     * thread, it is already off the ready queue and will be put
     * at the back when it is enqueued again.
     */
    if (EXPECT_FALSE(schedule != get_current_tcb())) {
        schedule_lock.lock();
        if (schedule->ready_list.is_queued()) {
            dequeue(schedule);
            enqueue(schedule);
        }
        schedule_lock.unlock();
    }

    /* Renew the schedule's timeslice. */
    schedule->current_timeslice = schedule->timeslice_length;
}

/**
 *  Throw away the rest of the current timeslice and yield control to
 *  any other thread with our same priority or higher.
 *
 *  Note that the most likely outcome of this function is that
 *  'current' is scheduled again. The only exception is if there
 *  happens to be another thread at the same priority as 'current'
 *  ready to run.
 *
 *  In a strict-priority scheduler, it is generally wrong to call this
 *  function for any reason other than carrying out a user-space
 *  request to yield, which in turn was probably wrong to ask for a
 *  yield().
 */
void
scheduler_t::yield(tcb_t * current, tcb_t * active_schedule,
        continuation_t continuation)
{
    /* Expire the current schedule's timeslice. */
    end_of_timeslice(get_active_schedule());

    /* Perform a schedule. */
    schedule(current, continuation, scheduler_t::sched_round_robin);
}

/**
 * handle a timer interrupt - update scheduling information
 * and pick a new thread to run according to the scheduling algorithm
 *
 * This is a control function, so it returns by activating the given
 * continuation
 *
 * @param continuation The continuation to activate upon completion
 */
INLINE NORETURN void
scheduler_t::handle_timer_interrupt(bool wakeup, continuation_t continuation)
{
    tcb_t * current = get_current_tcb();
    tcb_t * schedule = get_active_schedule();

#if defined(CONFIG_KDB_BREAKIN)
    kdebug_check_breakin();
#endif

#if defined(CONFIG_KEEP_CURRENT_TIME)
#if defined(CONFIG_MUNITS) || defined(CONFIG_MDOMAINS)
    if (get_current_context().raw == 0)
#endif
    {
        /* update the time global time*/
        current_time += get_timer_tick_length();
    }
#endif

#ifdef CONFIG_MUNITS
    process_domain_timer_tick();
#endif

    /* If the idle thread is running, return. */
    if (EXPECT_FALSE(current == get_idle_tcb())) {
        ACTIVATE_CONTINUATION(continuation);
    }

    /* Get the current schedule we are using. */
    word_t current_timeslice = schedule->current_timeslice;

    /* Check for infinite timeslice. */
    if (EXPECT_TRUE(current_timeslice != 0)) {

        /* The current timeslice is finite. */
        word_t timer_tick_len = get_timer_tick_length();

        /* Determine if 'current_timeslice - timer_tick_len <= 0'. */
        if (EXPECT_FALSE(current_timeslice <= timer_tick_len)) {
            /* We have end-of-timeslice. */
            TRACEPOINT(TIMESLICE_EXPIRED,
                       printf("timeslice expired for %t\n", current));

            /* If not running on a donated timeslice, optimize */
            if (EXPECT_TRUE(current == schedule)) {
                /* Any other threads ready at this priority? */
                if (!wakeup && !get_current_scheduler()->
                        prio_queue.get(schedule->effective_prio)) {
                    /* Renew the schedule's timeslice. */
                    schedule->current_timeslice = schedule->timeslice_length;
                    ACTIVATE_CONTINUATION(continuation);
                }
            }

            end_of_timeslice(schedule);
            get_current_scheduler()->schedule(current, continuation,
                    scheduler_t::sched_round_robin |
                    scheduler_t::preempting_thread);
            NOTREACHED();
        }

        /* Still have time left. Deduct the time we just took. */
        schedule->current_timeslice = current_timeslice - timer_tick_len;
    }

    if (wakeup) {
        /* Somthing is pending, requiring a wakeup */
        get_current_scheduler()->schedule(current, continuation,
                scheduler_t::preempting_thread);
    } else {
        /* Return directly as we have not finished our timeslice. */
        ACTIVATE_CONTINUATION(continuation);
    }
}

#if defined (CONFIG_MUNITS)

inline void
scheduler_t::process_domain_timer_tick()
{
    cpu_context_t current = get_current_context();
    cpu_context_t j = current;

    /*
     *     We check each running thread for timeslice expiry. If so,
     *     we tell that hardware thread to reschedule.
     *     Also, this stage ensures we update the timeslice
     *     available to each thread (so we MUST execute this stage).
     */

    for (j.unit=0; j.unit < CONFIG_NUM_UNITS; j.unit++)
    {
        // propogate timer tick
        if (j != current) {
            Platform::send_ipi(j);
        }
    }
}

extern "C" NORETURN void do_xcpu_reschedule(continuation_t continuation);

/**
 * Processes a timer tick event for a particular
 * hardware thread. We confirm that we need to reschedule due to timeslice
 * expiry. Note that the timeslice for the current running thread has
 * already been updated by the original timer tick recipient.
 *
 * This is a control function, so it returns by activating the given
 * continuation.
 *
 * @param current The current TCB
 * @param continuation The continuation to activate upon completion
 */
void NORETURN
scheduler_t::hwthread_timer_tick(tcb_t *current, continuation_t continuation)
{
    tcb_t * schedule = get_active_schedule();

    word_t current_timeslice = schedule->current_timeslice;

    /* Check for not infinite timeslice and expired */
    if (current_timeslice != 0) {

        /* Determine if 'current_timeslice - timer_tick_len <= 0'. */
        if (current_timeslice <= get_timer_tick_length()) {
            /* We have end-of-timeslice */
            TRACEPOINT(TIMESLICE_EXPIRED,
                    printf("timeslice expired for %t\n", current));

            /* If not running on a donated timeslice, optimize */
            if (EXPECT_TRUE(current == schedule)) {
                /* Any other threads ready at this priority? */
                if (!get_current_scheduler()->prio_queue.get(schedule->effective_prio)) {
                    /* Renew the schedule's timeslice. */
                    schedule->current_timeslice = schedule->timeslice_length;
                    ACTIVATE_CONTINUATION(continuation);
                }
            }

            end_of_timeslice(schedule);
            this->schedule(current, continuation,
                    scheduler_t::sched_round_robin |
                    scheduler_t::preempting_thread);
            NOTREACHED();
        }

        /* Deduct time for the timer tick. */
        schedule->current_timeslice = current_timeslice - get_timer_tick_length();
    }
    /* is someone trying to pause us? if so, reschedule */
    if (current->is_reserved()) {
        do_xcpu_reschedule(continuation);
        NOTREACHED();
    }
    /* return directly as time slice is not yet over */
    ACTIVATE_CONTINUATION(continuation);
}

/**
 * stub function to call hwthread_timer_tick with the correct
 * arguments (It is a member function)
 *
 * Called on each non-primary hwthread to inform them of end of a
 * timer tick, and invoke their limited schedule logic.
 *
 *  This is a control function so it returns by activating the given
 *  continuation
 *
 * @param entry The entry in the processor mailbox that caused this
 *   function to be invoked
 * @param continuation The continuation to activate upon completion
 */
extern "C" NORETURN void
do_smt_timer_tick(continuation_t continuation)
{
    tcb_t * current = get_current_tcb();

    /* If the idle thread is running, no accounting to do, just skip to the ipi handling
     * if there are any schedulable threads that can be picked up
     * to run. */
    if (current == get_idle_tcb()) {
        ACTIVATE_CONTINUATION(continuation);
    }

    get_current_scheduler()->hwthread_timer_tick(current, continuation);
    NOTREACHED();
}

#if defined(CONFIG_ENABLE_FASTPATHS)
void NORETURN
scheduler_t::hwthread_timer_tick_fast(tcb_t *current, continuation_t continuation)
{
    tcb_t * schedule = get_active_schedule();

    end_of_timeslice(schedule);
    this->schedule(current, continuation,
            scheduler_t::sched_round_robin |
            scheduler_t::preempting_thread);
    NOTREACHED();

}

extern "C" NORETURN void
do_smt_timer_tick_fast(continuation_t continuation)
{
    tcb_t * current = get_current_tcb();
#if defined(CONFIG_KDB_BREAKIN)
    kdebug_check_breakin();
    if (current == get_idle_tcb()) {
        ACTIVATE_CONTINUATION(continuation);
    }
#endif

    /* idle thread should have infinite timeslice and never get here */
    ASSERT(ALWAYS, current != get_idle_tcb());

    get_current_scheduler()->hwthread_timer_tick_fast(current, continuation);
    NOTREACHED();
}
#endif
#endif

/**
 * The idle thread merely carrys out a system sleep, normally resulting
 * in the processor halting until the next interrupt arrives.
 *
 * The idle thread should never be executed while other threads in the
 * system are still runnable.
 */
CONTINUATION_FUNCTION(idle_thread)
{
#if !(defined(CONFIG_DEBUG) && defined(CONFIG_KDB_BREAKIN))
    /*
     * Disable timer tick when sleeping.
     * Platform code should reenable it on receiving a device interrupt
     */
#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    get_current_scheduler()->schedule_lock.lock();
    bool all_units_sleeping = true;
    /*
     * With CONFIG_CONTEXT_BITMASKS, we need to ensure all other units
     * are sleeping before disabling timer interrupts
     */
    word_t current_unit = get_current_context().unit;
    for (word_t unit = 0; unit < CONFIG_MAX_UNITS_PER_DOMAIN; unit++) {
        /* Searching only on other hardware units, ensure they are all sleeping */
        if ((unit != current_unit) && (get_current_scheduler()->priorities[unit].prio != -1)) {
            all_units_sleeping = false;
        }
    }
    if (all_units_sleeping) {
        Platform::disable_timer_tick();
    }
    get_current_scheduler()->schedule_lock.unlock();
#else
    Platform::disable_timer_tick();
#endif

#endif /* DEBUG + BREAKIN */

    preempt_enable(idle_thread);
    while (1) {
        processor_sleep();
    }
}

/**
 * initialise all of the kernel threads in the system, along with the
 * roottask  in userland.
 *
 * It also contains the optional startup call to KDB
 *
 * It also flushes the kip from the cache so that there are no cache
 * aliases of the kip to trip up users of virtually addressed cache
 * architectures
 *
 * This is a continuation function but does not use any parameters
 * stored in the TCB
 *
 * This function activates the idle_thread function upon completion
 */
CONTINUATION_FUNCTION(init_all_threads)
{
    init_kernel_threads();
    init_root_servers();

#if defined(CONFIG_KDB_ON_STARTUP)
    enter_kdebug ("System started (press 'g' to continue)");
#endif

    /* Perform the first schedule, and become the idle-thread proper when we
     * once again regain control. */
    get_current_scheduler()->schedule(get_current_tcb(), idle_thread, scheduler_t::sched_default);
}

SYS_THREAD_SWITCH(threadid_t dest)
{
    PROFILE_START(sys_threadswitch);

    /* Make sure we are in the ready queue to
     * find at least ourself and ensure that the thread
     * is rescheduled */
    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();
    continuation_t continuation = ASM_CONTINUATION;
    NULL_CHECK(continuation);
    TRACEPOINT(SYSCALL_THREAD_SWITCH,
               printf("SYS_THREAD_SWITCH current=%t, dest=%t\n",
                      current, TID(dest)));

    /* Are we performing a simple yield? */
    if (dest.is_nilthread()) {
        /* Yield to another thread running at the same priority. */
        PROFILE_STOP(sys_threadswitch);
        scheduler->yield(current, get_active_schedule(), continuation);
        NOTREACHED();
    }

    /* Otherwise, we a performing an explicit timeslice donation. Ensure that
     * the destination thread is valid. */
    tcb_t *dest_tcb;
    if (dest.is_threadhandle()) {
        dest_tcb = lookup_tcb_by_handle_locked(dest.get_raw());
    } else {
        dest_tcb = get_current_clist()->lookup_ipc_cap_locked(dest);
    }

    /* If we are donating to ourself, or invalid thread we have no work to do. */
    if (EXPECT_FALSE(dest_tcb == NULL || dest_tcb == current)) {
        PROFILE_STOP(sys_threadswitch);
        if (dest_tcb != NULL) {
            dest_tcb->unlock_read();
        }
        return_thread_switch(continuation);
    }
    dest_tcb->unlock_read();

    /* Donate timeslice to the given thread. */
    PROFILE_STOP(sys_threadswitch);
    scheduler->donate(dest_tcb, get_current_tcb(), continuation);
    NOTREACHED();
}


/* local part of schedule */
bool
scheduler_t::do_schedule(tcb_t * tcb, word_t ts_len, word_t prio, word_t flags)
{
    bool schedule_required = false;

    /* Change priority of the thread if requested. */
    if ((prio != (~0UL)) && ((prio_t)prio != tcb->base_prio)) {
        set_priority(tcb, (prio_t)(prio & 0xff));
        schedule_required = true;
    }

    /* Modify the threads timeslice length if requested. */
    if (ts_len != (~0UL)) {
        set_timeslice_length(tcb, ts_len);
    }

    return schedule_required;
}

#if defined (CONFIG_MUNITS)
extern "C" NORETURN void
do_xcpu_reschedule(continuation_t continuation)
{
    tcb_t * current = get_current_tcb();
    get_current_scheduler()->schedule(current, continuation, scheduler_t::sched_default);
}
#endif /* CONFIG_MUNITS */

void
scheduler_t::release_and_reschedule_tcb(tcb_t * tcb)
{
    SMT_ASSERT(ALWAYS, tcb->is_grabbed_by_me());
    SMT_ASSERT(ALWAYS, schedule_lock.is_locked(true));

    if (tcb->get_state().is_runnable() && tcb != get_idle_tcb()) {
        tcb->release();
        enqueue(tcb);
        smt_reschedule(tcb);
    } else {
        tcb->release();
    }
}

void
scheduler_t::smt_reschedule(tcb_t * tcb)
{
#if defined(CONFIG_MUNITS)
    ASSERT(ALWAYS, !tcb->is_grabbed_by_me());
    ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    cpu_context_t context = get_current_context();

#if defined(CONFIG_CONTEXT_BITMASKS)
    /* Find the lowest-priority hardware unit that is usable by us. */
    word_t new_unit = -1UL;
    prio_t lowest = MAX_PRIO + 1;
    word_t current_unit = context.unit;
    for (word_t unit = 0; unit < CONFIG_MAX_UNITS_PER_DOMAIN; unit++) {
        /* Searching only on other hardware units, ensure this thread is
         * allowed to run on this unit and that this is the smallest thread we
         * have found so far. */
        if (unit != current_unit
                && priorities[unit].prio < lowest
                && (tcb->context_bitmask & (1 << unit)) != 0) {
            /* This is the smallest usable unit we have found so far.
             * Use it. */
            lowest = priorities[unit].prio;
            new_unit = unit;
        }
    }

    /* If the lowest unit has a priority less than us, send an IPI to it
     * asking it to perform a reschedule. */
    if (new_unit != -1UL) {
        tcb_t * old = smt_threads[new_unit].tcb;
        if (old == NULL || old->effective_prio < tcb->effective_prio) {
            cpu_context_t dest = dest;
            dest.unit = new_unit;
            request_reschedule(dest);
            Platform::send_ipi(dest);
        }
    }
#else /* !CONFIG_CONTEXT_BITMASKS */
    /* Ensure that the thread running on the lowest hardware unit
     * has a lower priority than us. */
    word_t current_unit = context.unit;
    word_t new_unit = lowest_unit & 0xffff;
    tcb_t * old = smt_threads[new_unit].tcb;
    if (new_unit != current_unit
            && (old == NULL || old->effective_prio < tcb->effective_prio)) {
        cpu_context_t dest = dest;
        dest.unit = new_unit;
        request_reschedule(dest);
        Platform::send_ipi(dest);
    }
#endif /* !CONFIG_CONTEXT_BITMASKS */
#endif /* !CONFIG_MUNITS */
}

static CONTINUATION_FUNCTION(finish_sys_schedule);

SYS_SCHEDULE(threadid_t dest_tid, word_t ts_len, word_t context_bitmask,
        word_t domain_control, word_t prio, word_t flags)
{
    PROFILE_START(sys_sched);
    continuation_t continuation = ASM_CONTINUATION;
    tcb_t * current = get_current_tcb();
    tcb_t * sched_tcb, *dest_tcb;
    scheduler_t * scheduler = get_current_scheduler();
    bool schedule_required = false;

    TRACEPOINT(SYSCALL_SCHEDULE,
        printf("SYS_SCHEDULE: curr=%t, dest=%t, hwtid mask = %08lx, "
               "timeslice=%x, domain_ctrl=%x, prio=%x\n",
               current, TID(dest_tid), context_bitmask, ts_len,
               domain_control, prio));

    if (dest_tid.is_myself()) {
        dest_tcb = acquire_read_lock_tcb(current);
    } else {
        dest_tcb = get_current_clist()->lookup_thread_cap_locked(dest_tid);
    }

    /* make sure the thread is valid */
    if (EXPECT_FALSE(dest_tcb == NULL))
    {
        current->set_error_code (EINVALID_THREAD);
        goto error_out;
    }

    /* are we in the same address space as the scheduler of the thread? 
     * if not, the root space should be allowed to continue anyway
     * if executing in an extended space, also allow operations on the
     * base space.
     */
    sched_tcb = dest_tcb->get_scheduler();

    if (EXPECT_FALSE(sched_tcb == NULL))
    {
        current->set_error_code (EINVALID_THREAD);
        goto error_out_locked;
    }

    if (EXPECT_FALSE(!is_privileged_space(get_current_space()) &&
                     sched_tcb->get_space() != get_current_space() &&
                     dest_tcb->get_space() != current->get_base_space()))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out_locked;
    }

    /* Check that we are not increasing the thread's priority, unless we are
     * a privileged thread. */
    if (prio != (~0UL) && !is_privileged_space(get_current_space()) &&
            (prio_t)(prio & 0xff) > sched_tcb->base_prio) {
        current->set_error_code (EINVALID_PARAM);
        goto error_out_locked;
    }

    current->sys_data.set_action(tcb_syscall_data_t::action_schedule);

    TCB_SYSDATA_SCHED(current)->dest_tcb = dest_tcb;

    if (dest_tcb->is_local_domain()) {
        scheduler->pause(dest_tcb);
        dest_tcb->unlock_read();

        schedule_required = scheduler->do_schedule(dest_tcb,
                ts_len, prio, flags);

#if defined(CONFIG_CONTEXT_BITMASKS) && defined(CONFIG_MUNITS)
        if (context_bitmask != (~0UL)) {
            scheduler->set_context_bitmask(dest_tcb, context_bitmask);
            schedule_required = true;
        }
#endif /* CONFIG_CONTEXT_BITMASKS && CONFIG_MUNITS */

        if ((domain_control != ~0UL) &&
            (domain_control < (word_t)get_mp()->get_num_scheduler_domains())) {
#ifdef CONFIG_MDOMAINS
                cpu_context_t to_context;
                to_context.domain = domain_control;
                dest_tcb->migrate_to_domain (to_context.root_context());
#endif
        }
    }
    /* FIXME: this stuff needs to be updated */
    else {
        UNIMPLEMENTED();
#if 0

        xcpu_request(dest_tcb->get_context(), do_xcpu_schedule,
                     dest_tcb, ts_len, prio, context_bitmask);

        if (domain_control != ~0UL) {
            cpu_context_t to_context;
            to_context.domain = domain_control;
            dest_tcb->migrate_to_domain (to_context.root_context());
        }
#endif
    }

    /* Save the sys_schedule return address */
    TCB_SYSDATA_SCHED(current)->schedule_continuation = continuation;

#if defined(CONFIG_CONTEXT_BITMASKS) && defined(CONFIG_MUNITS)
    /* If we have just made modifications to ourself, ensure that we are still
     * allowed to run on this hardware thread. */
    if (get_current_tcb() == dest_tcb &&
        (dest_tcb->context_bitmask & (1 << get_current_context().unit)) == 0) {
            scheduler->schedule(get_current_tcb(), finish_sys_schedule, scheduler_t::sched_default);
        }
#endif /* CONFIG_CONTEXT_BITMASKS */

    /* Unpause the thread. */
    if (get_current_tcb() != dest_tcb) {
        scheduler->unpause(dest_tcb);
    }

    /* Perform a schedule if we have modified the scheduling parameters of any
     * thread, which may affect which thread should now be running. */
    if (schedule_required) {
        scheduler->schedule(current, finish_sys_schedule,
                            scheduler_t::sched_default);
    } else {
        ACTIVATE_CONTINUATION(finish_sys_schedule);
    }
    NOTREACHED();

error_out_locked:
    dest_tcb->unlock_read();
error_out:
    PROFILE_STOP(sys_sched);
    return_schedule(0, 0, continuation);
}

/**
 *  Map a thread's state into a schedule system call return value.
 *
 *  @param state The thread state being inspected.
 *
 *  @return The schedule system call return value mapping to the
 *    inspected state.
 */
static word_t
sys_schedule_return(thread_state_t state)
{
    return state.is_aborted() ? L4_SCHEDRESULT_DEAD :
            state.is_halted() ? L4_SCHEDRESULT_INACTIVE :
            state.is_running() ? L4_SCHEDRESULT_RUNNING :
            state.is_polling() ? L4_SCHEDRESULT_PENDING_SEND :
            state.is_sending() ? L4_SCHEDRESULT_SENDING :
            state.is_waiting() ? L4_SCHEDRESULT_WAITING :
            state.is_waiting_mutex() ? L4_SCHEDRESULT_WAITING_MUTEX :
            state.is_waiting_notify() ? L4_SCHEDRESULT_WAITING_NOTIFY :
            state.is_xcpu_waiting() ? L4_SCHEDRESULT_WAITING_XCPU :
            L4_SCHEDRESULT_ERROR;
}

/**
 * Calculate the appropriate return value for the schedule
 * syscall, and then return to user
 *
 * This is a continuation function but does not use any arguments
 * stored in the TCB
 */
static
CONTINUATION_FUNCTION(finish_sys_schedule)
{
    tcb_t * current = get_current_tcb();
    tcb_t * dest_tcb = TCB_SYSDATA_SCHED(current)->dest_tcb;

    thread_state_t state = dest_tcb->get_state();
    word_t result = sys_schedule_return(state);

    if (result == 0) {
        TRACEF("invalid state (%x)\n", (word_t)state);
    }

    PROFILE_STOP(sys_sched);
    return_schedule(result, dest_tcb->current_timeslice,
            TCB_SYSDATA_SCHED(current)->schedule_continuation);
}

#if defined(CONFIG_MUNITS)
/**
 * When pause() returns the thread is scheduled and must be
 * released by the caller.
 *
 * @param tcb The TCB of the thread being paused.
 */
void
scheduler_t::pause(tcb_t *tcb)
{
    /* Doesn't make sense to pause the current tcb. */
    if (tcb == get_current_tcb()) {
        return;
    }

    schedule_lock.lock();

    /* Reserve the thread, ensuring that it does not return to
     * the ready queue. */
    tcb->reserve();

    while (1) {
        /* If we can grab the thread, do so and return the thread. */
        if (tcb->grab()) {
            if (tcb->ready_list.is_queued()) {
                dequeue(tcb);
            }
            tcb->unreserve();
            schedule_lock.unlock();
            return;
        }

        /* Send an IPI to whichever hardware thread currently owns it. */
        cpu_context_t context = tcb->get_context();
        if (context != context.root_context()) {
            request_reschedule(context);
            Platform::send_ipi(context);
        }

        /* Wait for the thread to stop running. */
        schedule_lock.unlock();
        do {
            okl4_atomic_barrier();
        } while (tcb->get_context() != context.root_context());

        /* Try again. */
        schedule_lock.lock();
    }
}

/** When unpause() returns, the thread is no longer scheduled.
 *
 *  @param tcb The TCB of the thread being unpaused.
 */
void
scheduler_t::unpause(tcb_t *tcb)
{
    ASSERT(ALWAYS, !tcb->ready_list.is_queued());

    if (tcb != get_current_tcb()) {
        schedule_lock.lock();
        release_and_reschedule_tcb(tcb);
        schedule_lock.unlock();
    }
}
#endif /* CONFIG_MUNITS */

/**
 * Hook for platform code to cause a rescheduler.
 * This is used in the case of interrupt delivery
 */
namespace PlatformSupport {

void scheduler_handle_timer_interrupt(bool wakeup, continuation_t cont)
{
    get_current_scheduler()->handle_timer_interrupt(wakeup, cont);
    NOTREACHED();
}

void schedule(continuation_t cont)
{
    //printf("resched!\n");
    get_current_scheduler()->schedule(get_current_tcb(), cont,
            scheduler_t::preempting_thread);

    NOTREACHED();
}

}

NORETURN void
scheduler_t::fast_schedule(tcb_t * current, fast_schedule_type_e type,
                           tcb_t * tcb, continuation_t continuation,
                           flags_t flags = sched_default)
{
    /**
     * @todo FIXME: Mothra issue #2070. For SMT, check that threads
     * are allowed to run on the cores they are being scheduled on -
     * davidg.
     */

    ASSERT(DEBUG, (type == current_tcb_unschedulable)
           || current->get_state().is_runnable());
    ASSERT(DEBUG, tcb->get_state().is_runnable());
    ASSERT(DEBUG, current != tcb);
    ASSERT(ALWAYS, tcb->is_grabbed_by_me());

    bool can_schedule_current = type == current_tcb_schedulable;

    /* Determine which thread should be scheduled. */
    run_e action = should_schedule_thread(current, tcb,
            can_schedule_current, flags);

    switch (action) {
    case run_new:
        /* Should schedule the new thread. */
        if (flags & preempting_thread) {
            mark_thread_as_preempted(current);
        }
        switch_from(current, continuation);
        schedule_lock.lock();
        set_active_thread(tcb, tcb);
        release_and_reschedule_tcb(current);
        schedule_lock.unlock();
        switch_to(tcb, tcb);
        NOTREACHED();
        break;

    case run_current:
        /* We should keep running. */
        ASSERT(DEBUG, can_schedule_current);
        schedule_lock.lock();
        release_and_reschedule_tcb(tcb);
        schedule_lock.unlock();
        ACTIVATE_CONTINUATION(continuation);
        NOTREACHED();
        break;

    case full_schedule:
    default:
        /* Not sure: do a full schedule. */
        schedule_lock.lock();
        release_and_reschedule_tcb(tcb);
        schedule_lock.unlock();
        schedule(current, continuation, flags);
        NOTREACHED();
        break;
    }
}

/**********************************************************************
 *
 *                     Initialisation
 *
 **********************************************************************/

void SECTION(SEC_INIT)
scheduler_t::start(cpu_context_t context)
{
#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
    TRACE_INIT ("Switching to idle thread (Context %d)\n", context.raw);
#else
    TRACE_INIT ("Switching to idle thread\n");
#endif

#ifdef CONFIG_MUNITS
    (void)get_idle_tcb()->grab();
    set_active_thread(get_idle_tcb(), get_idle_tcb());
#endif

    initial_switch_to(get_idle_tcb());
}

void SECTION(SEC_INIT)
scheduler_t::init(bool bootcpu)
{
    prio_queue.init();
    scheduling_invariants_violated = true;

#ifdef CONFIG_MUNITS
    schedule_lock.init();
    schedule_lock.lock();
    for (int i = 0; i < CONFIG_MAX_UNITS_PER_DOMAIN; i++) {
        smt_threads[i].tcb = NULL;
        smt_threads[i].schedule = NULL;
        priorities[i].unit = i;
        priorities[i].prio = -1;
        lowest_unit = 0;
    }
    schedule_lock.unlock();
#endif
}

