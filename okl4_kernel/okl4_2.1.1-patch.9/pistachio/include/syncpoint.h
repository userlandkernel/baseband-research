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

#ifndef __SYNCPOINT_H__
#define __SYNCPOINT_H__

#include <kernel/tcb.h>
#include <kernel/queueing.h>
#include <kernel/sync.h>

class tcb_t;

/** @todo FIXME: Doxygen comment for class - awiggins. */
class syncpoint_t
{
public:

    /**
     *  Initialise this syncpoint.
     *
     *  @param donatee The TCB reference of the initial donatee thread
     *  for this syncpoint.
     */
    void init(tcb_t * donatee);

    /** Does this sync-point have threads blocked on it? */
    bool has_blocked(void);

    /**
     * Inspect this synchronisation point's schedule donatee.
     *
     * @return The TCB of the thread receiving schedules inherited via
     * this synchronisation point.
     */
    tcb_t * get_donatee(void);

    /**
     * Return the head of this synchronisation points blocked queue.
     *
     * @return this->blocked_head
     */
    tcb_t * get_blocked_head(void);

    /**
     * Cause a thread to become blocked waiting on the donatee of this
     * syncpoint.
     *
     * @param tcb Reference to the thread to add to this sync-point's
     *   blocking queue.
     */
    void block(tcb_t * tcb);

    /**
     * Cause a thread to become blocked waiting on the donatee of this
     * syncpoint, and perform a schedule. By performing both operations as a
     * single call, optimisations may be made speeding up scheduling.
     *
     * @param tcb Reference to the thread to add to this sync-point's
     *   blocking queue.
     * @parma continuation ...
     */
    NORETURN void block_sched(tcb_t * tcb, continuation_t continuation);

    /**
     * Unblock a thread on a syncpoint.
     *
     * This operation is potentially slow if the syncpoint the thread is
     * being unblocked from has a donatee.
     *
     * @param tcb ...
     */
    void unblock(tcb_t * tcb);

    /**
     * Update this syncpoint to take into account priority changes
     * by the given thread.
     *
     * @param tcb ...
     */
    void refresh(tcb_t * tcb);

    /**
     * Become the donatee of this syncpoint.
     *
     * @param tcb ...
     */
    void become_donatee(tcb_t * tcb);

    /**
     * Cause the current syncpoint to clear its current donatee.
     */
    void release_donatee(void);

private:

    /**
     * Enqueue the given TCB in effective priority ordering into this
     * syncpoints blocked list.
     *
     * @param tcb ...
     *
     */
    void enqueue_tcb_sorted(tcb_t * tcb);

    /**
     * Add a thread to the queue of threads blocked on this
     * synchronisation point.
     *
     * @pre        'tcb' held by the currently running thread.
     *
     * @param tcb  TCB of thread blocking on this synchronisation
     * point.
     */
    void enqueue_blocked(tcb_t * tcb);

    /**
     * Remove a thread from the queue of threads blocked on this
     * synchronisation point.
     *
     * @pre        'tcb' held by the currently running thread.
     *
     * @param tcb TCB of thread unblocking from this synchronisation
     * point.
     */
    void dequeue_blocked(tcb_t * tcb);

    /**
     * Update each thread's priority down the dependency chain until
     * we hit the end of the chain, or hit a thread whose priority
     * does not increase.
     *
     * This is faster than the more generic
     * 'propagate_priority_change' because we don't have to
     * recalculate each thread's priority: merely increase it if it is
     * not high enough already.
     *
     * Deadlocks (i.e., loops in the dependency chain) are magically
     * handled by the fact we eventually reach a thread which already
     * has the correct priority, and hence break out of the loop.
     *
     * @param new_prio ...
     */
    void propagate_priority_increase(prio_t new_prio);

    /**
     * Update each thread's priority down the dependency chain until
     * we hit the end of the chain, or somebody's priority remains
     * unchanged.
     *
     * This function is slower than 'propagate_priority_increase' as
     * we must do more work on each thread to determine its modified
     * priority.
     *
     * Deadlocks (i.e., loops in the dependency chain) are magically
     * handled by the fact we eventually reach a thread which already
     * has the correct priority, and hence break out of the loop.
     */
    void propagate_priority_change(void);

#if defined(CONFIG_TRACEPOINTS)
    /**
     * Determine if the given thread is in a deadlocked state.
     *
     * @param tcb          The TCB to start the search on.
     *
     * @param head_length  If a deadlock is found, the number of TCBs
     *                     that are in the chain until the first TCB
     *                     in the deadlock loop is found.
     *
     * @param loop_length  If a deadlock is found, the number of threads in
     *                     the deadlock loop.
     *
     * @returns            True if a deadlock was found, false otherwise.
     */
    static bool is_deadlocked(tcb_t *tcb, int *head_length, int *loop_length);

    /**
     * Report to the user the presense of deadlock.
     *
     * @param tcb          The TCB to start reporting on.
     *
     * @param head_length  The number of TCBs in the chain until the
     *                     deadlock loop starts.
     *
     * @param loop_length  The number of TCBS in the deadlock loop.
     */
    static void report_deadlock(tcb_t *first, int head_length,
            int loop_length);
#endif /* CONFIG_TRACEPOINTS */

    /**
     * The current holder of this syncpoint, and who schedules will be
     * forwarded to if a thread is currently blocked on us.
     */
    tcb_t * donatee;

    /** First TCB waiting on this syncpoint. */
    tcb_t * blocked_head;

    friend void mkasmsym(void);
};

INLINE bool
syncpoint_t::has_blocked(void)
{
    return this->get_blocked_head() != NULL;
}

INLINE tcb_t *
syncpoint_t::get_donatee(void)
{
    return this->donatee;
}


INLINE tcb_t *
syncpoint_t::get_blocked_head(void)
{
    return this->blocked_head;
}


#endif /* !__SYNCPOINT_H__ */
