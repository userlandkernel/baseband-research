/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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

#include <stdio.h>
#include <assert.h>
#include <l4/thread.h>
#include <l4/schedule.h>

#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/types.h>

#include <interfaces/iguana_client.h>

#include "utility.h"

/*
 * Maximum number of supported SMT threads in the system.
 */
#define MAX_SMT_THREADS       64

/*
 * Amount of time we should spin before we assume that another
 * hardware thread is not running the counted spinner.
 */
#define SPIN_WAIT_TIME        10000

/*
 * Stack size for our threads.
 */
#define THREAD_STACK_SIZE     4096

/*
 * Created threads.
 */
static memsection_ref_t stacks[MAX_SMT_THREADS + 1];
static thread_ref_t threads[MAX_SMT_THREADS + 1];
static int allocated_threads;

/*
 * Number of loops performed by the 'counted_spinner'.
 */
static volatile int spinner_count;

/*
 * Simple thread increasing a counter.
 */
static void counted_spinner(void)
{
    ARCH_THREAD_INIT
    
    L4_KDB_SetThreadName(L4_myselfconst, "Count Spin");
    
    for (;;) {
        spinner_count++;
    }
}

/*
 * Simple spinning thread.
 */
static void spinner(void)
{
    ARCH_THREAD_INIT
    
    L4_KDB_SetThreadName(L4_myselfconst, "Spinner");
    
    for (;;) {}
}

/*
 * Create a new thread.
 */
static thread_ref_t
create_thread(void (*entry_point)(void), int prio)
{
    void **stack_top;
    L4_ThreadId_t dummy;

    /* Allocate a stack. */
    stacks[allocated_threads] = memsection_create(
            THREAD_STACK_SIZE, (void *)&stack_top);
    if (stacks[allocated_threads] == 0) {
        assert(!"Error in creating stack.\n");
    }
    stack_top = (void **)((uintptr_t)stack_top + THREAD_STACK_SIZE);
    stack_top--;
    stack_top--;

    /* Start the thread. */
    threads[allocated_threads] = thread_create_priority(prio, &dummy);
    thread_start(threads[allocated_threads],
            (uintptr_t)entry_point,
            (uintptr_t)stack_top);

    /* Return the thread information back to the user. */
    allocated_threads++;
    return threads[allocated_threads - 1];
}

/*
 * If the processor happens to have more than one hardware thread,
 * keep creating spinning processes until we fill up all the other
 * hardware units except the one we are occupying.
 */
int
create_filler_threads(void)
{
    int created_threads;
    thread_ref_t counter;

    /* Create a thread that increases a global counter. The thread has low
     * priority, so as long as this thread is running we know we have more
     * hardware units to fill. */
    counter = create_thread(counted_spinner, 1);

    /* Keep creating threads until the counter stops running. */
    for (created_threads = 0;; created_threads++) {
        int i;
        int first_count = spinner_count;

        /* Determine if the initial thread is still spinning. */
        for (i = 0; i < SPIN_WAIT_TIME; i++) {
            if (first_count != spinner_count) {
                /* The spinner still appears to be running. */
                break;
            }
        }

        /* If the initial spinner has stopped running, we have created enough
         * threads. */
        if (i == SPIN_WAIT_TIME) {
            break;
        }

        /* Otherwise, we create another one and try again. */
        create_thread(spinner, 254);
    }

    /* Display diagnostic message if any thread was created. */
    if (created_threads > 0) {
        printf("%d spinning threads created for this test.\n",
                created_threads);
    }

    return created_threads;
}

/*
 * Delete threads created by create_filler_threads.
 */
void
delete_filler_threads(void)
{
    while (allocated_threads > 0) {
        thread_delete(thread_l4tid(threads[allocated_threads - 1]));
        allocated_threads--;
    }
}
