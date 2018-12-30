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
 * Description: Scheduler priority queue functions.
 */

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <prioqueue.h>
#include <queueing.h>
#include <smp.h>
#include <config.h>

/* Initialise the priority queue data structure. */
void
prio_queue_t::init(void)
{
    word_t i;
    /* prio queues */
    for (i = 0; i < MAX_PRIO; i++) {
        prio_queue[i] = (tcb_t *)NULL;
    }
    index_bitmap = 0;
    for (i = 0; i < BITMAP_WORDS; i++) {
        prio_bitmap[i] = 0;
    }
}

void
prio_queue_t::enqueue(tcb_t * tcb)
{
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    SMT_ASSERT(ALWAYS, !tcb->is_grabbed_by_me());
    ASSERT(ALWAYS, tcb != get_idle_tcb());
    ASSERT(ALWAYS, !tcb->ready_list.is_queued());

    prio_t prio = tcb->effective_prio;

    /* Enqueue the TCB. */
    ASSERT(DEBUG, prio >= 0 && prio <= MAX_PRIO);
    ENQUEUE_LIST_TAIL(tcb_t, prio_queue[prio], tcb, ready_list);

    /* Finally, update the bitmap tree. */
    index_bitmap |= (1UL << ((word_t)prio / BITS_WORD));
    prio_bitmap[(word_t)prio / BITS_WORD] |= 1UL << ((word_t)prio % BITS_WORD);
}

/* Dequeue the given TCB from the priority queue. */
void
prio_queue_t::dequeue(tcb_t * tcb)
{
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));
    ASSERT(DEBUG, tcb);
    ASSERT(ALWAYS, tcb != get_idle_tcb());
    ASSERT(ALWAYS, tcb->ready_list.is_queued());

    prio_t priority = tcb->effective_prio;

    ASSERT(DEBUG, priority >= 0 && priority <= MAX_PRIO);
    DEQUEUE_LIST(tcb_t, prio_queue[priority], tcb, ready_list);
    if (prio_queue[priority] == 0) {
        remove_sched_bitmap_bit(priority);
    }
}

void
prio_queue_t::set_base_priority(tcb_t *tcb, prio_t new_priority)
{
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));

    /* Recalculate effective priorities. */
    prio_t old_effective_prio = tcb->effective_prio;
    tcb->base_prio = new_priority;

#ifdef CONFIG_SCHEDULE_INHERITANCE
    prio_t effective_prio = tcb->calc_effective_priority();
#else
    prio_t effective_prio = new_priority;
#endif

    /* If there is a change, we need to propagate it. */
    if (effective_prio != old_effective_prio) {
        set_effective_priority(tcb, effective_prio);
    }
}

void
prio_queue_t::set_effective_priority(tcb_t * tcb, prio_t new_priority)
{
    ASSERT(DEBUG, new_priority >= 0 && new_priority <= MAX_PRIO);
    SMT_ASSERT(ALWAYS, get_current_scheduler()->schedule_lock.is_locked(true));

    /* Thread is not on the priority queue. Just update its priority,
     * with no more to be done. */
    if (!tcb->ready_list.is_queued()) {
        tcb->effective_prio = new_priority;
        return;
    }

    /* Otherwise, dequeue the thread and enqueue it again at its new
     * priority. */
    dequeue(tcb);
    tcb->effective_prio = new_priority;
    enqueue(tcb);
}

