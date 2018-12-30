/*
 * Copyright (c) 2002, Karlsruhe University
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
 * Description:  debugging of scheduling related stuff
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <sync.h>

#include <tcb.h>
#include <schedule.h>

tcb_t * global_present_list UNIT("kdebug") = NULL;
spinlock_t present_list_lock;

DECLARE_CMD(cmd_show_ready, root,
        'q', "showqueue",  "show scheduling queue");
#if defined(CONFIG_MUNITS)
DECLARE_CMD(cmd_show_units, root,
        'U', "showunits",  "show execution unit status");
#endif

/**
 * The maximum number of threads we would expect on the ready queue at a given
 * priority. When this value is exceeded, we assume that the ready-queue data
 * structure has become corrupt.
 */
#define MAX_EXPECTED_THREADS   16000

/**
 * If we detect that the ready list is in an invalid state, it
 * is useful to print out its current state information.
 *
 * Given a TCB, print out the ready list from that TCB.
 */
static void
print_broken_ready_list(tcb_t *starting_point)
{
    tcb_t *walk = starting_point;

    /* Print header. */
    printf("\nSuspected present-list fault:\n");

    for (int i = 0; i < MAX_EXPECTED_THREADS; i++) {
        /* Print this element. */
        printf("  %t: prev=%t, next=%t\n",
                walk->get_global_id().get_raw(),
                walk->present_list.prev->get_global_id().get_raw(),
                walk->present_list.next->get_global_id().get_raw());

        /* Determine if this node is incorrect. */
        if (walk->present_list.prev->present_list.next != walk ||
                walk->present_list.next->present_list.prev != walk) {
            printf("  *** ERROR ***\n");
        }

        /* Walk to the next element. */
        walk = walk->present_list.next;
        if (walk == starting_point) {
            printf("  --- END ---\n");
            break;
        }
    }
    printf("\n");

}

/**
 * Print all the threads in the system at the given priority to console.
 *
 * If no threads exist at the given priority, nothing is printed to screen.
 */
static void
print_threads_at_prio(prio_t prio)
{
    bool thread_exists = false;
    tcb_t *walk = global_present_list;
    int num_threads = 0;

    /* Walk through each thread in the systemm, printing it out
     * if it is at this priority. */
    do {
        /* If at the correct priority, print this thread. */
        if (walk->effective_prio == prio) {
            /* If this is the first thread at this priority, print a
             * priority header. */
            if (!thread_exists) {
                printf("[%03d]:", prio);
                thread_exists = true;
            }

            /* Print the actual thread information. */
            word_t id = walk->get_global_id().get_raw();
            bool queued = walk->ready_list.is_queued();
            bool running = walk->get_state().is_running();
            bool halted = walk->get_state().is_halted();
            bool aborted = walk->get_state().is_aborted();

            printf(" ");
            if (running) {
                if (!queued) printf("<");
            } else {
                if (halted) printf("{");
                else if (aborted) printf("!");
                else printf("(");
            }

#if defined(CONFIG_MUNITS)
            word_t unit = walk->get_context().unit;
            if (unit == 0xffff) {
                unit = (word_t)-1;
            }

            printf("%t:%d", id, unit);
#else
            printf("%.wt", id);
#endif
            if (running) {
                if (!queued) printf(">");
            } else {
                if (halted) printf("}");
                else if (aborted) printf("!");
                else printf(")");
            }
        }

        /* Move to the next thread. */
        walk = walk->present_list.next;

        /* If we have hit too many threads, assume there is an
         * error in the data structure. */
        if (num_threads++ > MAX_EXPECTED_THREADS) {
            print_broken_ready_list(walk);
            return;
        }
    } while (walk != global_present_list);

    /* If a thread exists at this priority, print a new-line. */
    if (thread_exists) {
        printf("\n");
    }
}

CMD(cmd_show_ready, cg)
{
    present_list_lock.lock();
    get_current_scheduler()->schedule_lock.lock();
    printf("Key: (X) blocked, <X> on CPU, {X} halted, !X! aborted\n");
    printf("\n");

    /* Iterate through each priority. */
    for (prio_t prio = MAX_PRIO; prio >= 0; prio--) {
        print_threads_at_prio(prio);
    }

    /* Print idle thread information. */
    printf("idle : %t\n\n", get_idle_tcb()->get_global_id().get_raw());

    get_current_scheduler()->schedule_lock.unlock();
    present_list_lock.unlock();
    return CMD_NOQUIT;
}

#if defined(CONFIG_MUNITS)
CMD(cmd_show_units, cg)
{
    scheduler_t *scheduler = get_current_scheduler();
    scheduler->schedule_lock.lock();

    word_t cached_lowest_unit = scheduler->lowest_unit & 0xffff;
    word_t cached_lowest_prio = scheduler->lowest_unit >> 16;

    /* Print information about what thread each execution unit is running. */
    printf("Execution Units:\n");
    for (word_t i = 0; i < CONFIG_NUM_UNITS; i++) {
        tcb_t *tcb = scheduler->smt_threads[i].tcb;
        printf("  %3d: %8.8t (e.prio: %3d)", i,
                tcb != NULL ? tcb                 : NULL,
                tcb != NULL ? tcb->effective_prio : -1);
        if (cached_lowest_unit == i) {
            printf(" [lowest_unit]");
        }
        printf("\n");
    }
    printf("\n");
    printf("Cached lowest unit: %d @ e.prio %d\n\n",
            (short int)cached_lowest_unit,
            (short int)cached_lowest_prio);

    scheduler->schedule_lock.unlock();
    return CMD_NOQUIT;
}
#endif /* CONFIG_MUNITS */

