/*
 * Copyright (c) 2006, National ICT Australia
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
  Author: Daniel Potts
  Created: Tue Febuary 14, 2006
*/


#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <stddef.h>
#include <stdint.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if defined(SMT_ENABLED_ARCHITECTURE)
static L4_ThreadId_t main_thread;


//#define VERBOSE_HIGH(x...) printf(x)
#define VERBOSE_HIGH(...)

#define MAX_THREADS (NTHREADS+1)

static volatile int thread_simple_spinner_cleanup;
static volatile int thread_simple_spinner_start;
static volatile uint64_t counter[MAX_THREADS];
static volatile uint32_t thread_running[MAX_THREADS];


static uint64_t
min_counter(int i, int up_to)
{
    uint64_t min = counter[i];
    for(i++; i<up_to; i++) {
        if (counter[i] < min) {
            min = counter[i];
        }
    }
    return min;
}

static uint64_t
max_counter(int i, int up_to)
{
    uint64_t max = counter[i];
    for(i++; i<up_to; i++) {
        if (counter[i] > max) {
            max = counter[i];
        }
    }
    return max;
}

static uint64_t
sum_counter(int i, int up_to)
{
    uint64_t count = 0;
    for ( ;i < up_to; i++) {
        count += counter[i];
    }
    return count;
}

static void
clear_counters(int up_to)
{
    int i;
    for (i=0; i < up_to; i++) {
        counter[i] = 0;
    }
}


static uint64_t
avg_counter(int i, int up_to)
{
    return sum_counter(i, up_to) / (up_to - i);
}

static uint64_t
counter_var(int from, int up_to)
{
    uint64_t avg_ = avg_counter(from, up_to);
    uint64_t max_ = max_counter(from, up_to) - avg_;
    uint64_t min_ = avg_ - min_counter(from, up_to);
    uint64_t worst = min_;
    if (max_ > min_)
        worst = max_;
    return (worst * 100) / avg_;
}


static void
print_counter(int up_to)
{
    int i;
    for (i=0; i < up_to; i++) {
        printf("%lld ", counter[i]);
    }
    printf("\n");
}


static int
get_thread_num(L4_ThreadId_t t)
{
    return ((t.raw - main_thread.raw) / 0x00004000)  - 1;
}
static L4_ThreadId_t
get_thread_id(int i)
{
    L4_ThreadId_t tid;
    tid.raw = (main_thread.raw + (i+1)*0x00004000);
    return tid;
}

static void
thread_simple_spinner (void *arg)
{
    int thrd_num = get_thread_num(L4_Myself());
    int dummy;

    while(thread_simple_spinner_start == 0)
        ;
    while(thread_simple_spinner_cleanup == 0) {
        if(counter[thrd_num]++ % 50000 == 0) {
            VERBOSE_HIGH("### %d %d\n", (int)thrd_num, (int)counter[thrd_num]);
        }
        if (counter[thrd_num] > 0x00100000)
            dummy = 1; // done
    }

    L4_Call(L4_Myself());
}


static void
thread_simple_spinner_main (void)
{
    int thrd_num = get_thread_num(L4_Myself());

    clear_counters(5);
    thread_simple_spinner_start = 1;

    while(thread_simple_spinner_cleanup == 0) {
        if(counter[thrd_num]++ % 50000 == 0) {
            VERBOSE_HIGH("### %d %d\n", (int)thrd_num, (int)counter[thrd_num]);
        }
        if (counter[thrd_num] > 0x00100000)
            thread_simple_spinner_cleanup = 1; // done
    }
    L4_Call(L4_Myself());
}


static volatile int ping_counter;
static volatile int pong_counter;
static volatile int pingpong_counter;
static volatile int send_pingpong_counter;
static volatile int pingpong_cleanup;


static void
synch_recv (void *arg)
{
    L4_ThreadId_t caller;
    L4_MsgTag_t tag;
    int i;
    int j=1000;

    while(pingpong_cleanup == 0) {
        tag = L4_Wait(&caller);
        pingpong_counter++;

        for (i=0; i < j; i++)
            ;
        j+=1000;

        if ((pingpong_counter % 50) == 0) {
            printf(" %d: %d\n", pingpong_counter, j);
        }
    }

    L4_Call(L4_Myself());
}


static void
synch_pong (void *arg)
{
    L4_ThreadId_t caller;
    L4_MsgTag_t tag;

    VERBOSE_HIGH("Pong running\n");

    while(pingpong_cleanup == 0) {
        tag = L4_Wait(&caller);
        pingpong_counter++;
        L4_LoadMR (0, 0);
        L4_Send(caller);

    }
    L4_Call(L4_Myself());
}


static void
synch_ping (void *arg)
{
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;

    VERBOSE_HIGH("Ping running\n");

    /* XXX partner always thread 0 (created before us) */
    partner = get_thread_id(0);

    while(pingpong_cleanup == 0) {
        L4_LoadMR (0, 0);
        L4_Send(partner);

        tag = L4_Wait(&partner);
    }

    L4_Call(L4_Myself());
}

/*
\begin{test}{SMT0100}
  \TestDescription{Verify scheduling of constrained threads in a single scheduling domain}
  \TestFunctionalityTested{SMT, Global Scheduler, Constraints}
  \TestImplementationProcess{
    Five spinning threads are created. Each thread is constrained to an unused
    hardware thread.
    The main threads are constrained to running on hardware thread 0.
    }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0100)
{
    uint64_t diff;
    L4_ThreadId_t rthread[5];

    thread_simple_spinner_cleanup = 0;
    thread_simple_spinner_start = 0;

    /* FIXME: Restrict to a single processor */

    changeThreadCreationSMTBitmask(2);
    rthread[0] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    changeThreadCreationSMTBitmask(4);
    rthread[1] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    changeThreadCreationSMTBitmask(8);
    rthread[2] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    changeThreadCreationSMTBitmask(16);
    rthread[3] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    changeThreadCreationSMTBitmask(32);
    rthread[4] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner_main);

    while(thread_simple_spinner_cleanup == 0)
        L4_ThreadSwitch(L4_nilthread);

    (void) print_counter;

    diff = counter_var(0, 5);
    if (diff > 15) {
        fail_unless(0, "It appears there was a scheduling error\n"
                    "We expect each thread to get an equal share, but\n"
                    "one thread, ran more or less often than expected.\n"
                    "(Specifically, it had greater than 15% difference to the average\n");
    }
}
END_TEST

/*
\begin{test}{SMT0200}
  \TestDescription{Verify scheduling of unconstrained threads in a single
  scheduling domain }
  \TestFunctionalityTested{Global Scheduler, Constraints}
  \TestImplementationProcess{
    Five spinning threads are created. All threads are scheduled across all
    available hardware threads.
    The main threads are constrained to running on hardware thread 0.
    }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0200)
{
    uint64_t diff;
    L4_ThreadId_t rthread[5];

    thread_simple_spinner_cleanup = 0;
    thread_simple_spinner_start = 0;

    /* FIXME: Restrict to a single processor */

    changeThreadCreationSMTBitmask(63);
    rthread[0] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    rthread[1] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    rthread[2] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    rthread[3] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);
    rthread[4] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner_main);

    while(thread_simple_spinner_cleanup == 0)
        L4_ThreadSwitch(main_thread);


    diff = counter_var(0, 5);
    if (diff > 15) {
        fail_unless(0, "It appears there was a scheduling error\n"
                    "We expect each thread to get an equal share, but\n"
                    "one thread, ran more or less often than expected.\n"
                    "(Specifically, it had greater than 15% difference to the average\n");
    }
}
END_TEST

static void
sleep_a_bit(void)
{
    int i = 0;
    for(i=0;i<300000U;i++);
}

#define TEST3_MAJOR_ITERATIONS 20
/*
\begin{test}{SMT0300}
  \TestDescription{Thread creation and deletion of running thread}
  \TestFunctionalityTested{Global scheduler, thread creation and deletion}
  \TestImplementationProcess{
    Five spinning threads are created. All threads are scheduled across all
    available hardware threads. The threads are then deleted.
    The main threads are constrained to running on hardware thread 0.
}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0300)
{
    int i, j;
    L4_ThreadId_t rthread[5];

    thread_simple_spinner_cleanup = 0;

    changeThreadCreationSMTBitmask(63);

    for (j=0;j<TEST3_MAJOR_ITERATIONS;j++) {
        for (i=0;i<5;i++)
            rthread[i] = createThreadInSpace(L4_nilthread, (void *)thread_simple_spinner);

        /* Give threads time to start up or our pager may complain
         *  about us going away.
         */
        L4_ThreadSwitch(main_thread);
        sleep_a_bit();
        L4_ThreadSwitch(main_thread);

        for (i=0;i<5;i++)
            deleteThread(rthread[i]);
    }
}
END_TEST

/*
\begin{test}{SMT0301}
  \TestDescription{Thread creation and deletion of receiving thread}
  \TestFunctionalityTested{Global scheduler, thread creation and deletion}
  \TestImplementationProcess{
    The following process is repeated:
    \begin{enumerate}
      \item Five receiving threads are created. All threads are scheduled across all
    available hardware threads. 
    \item The threads are then deleted.
      \end{enumerate}
    The main threads are constrained to running on hardware thread 0.
}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0301)
{
    int i, j;
    L4_ThreadId_t rthread[5];

    pingpong_cleanup = 0;

    changeThreadCreationSMTBitmask(63);

    for (j=0;j<TEST3_MAJOR_ITERATIONS;j++) {
        for (i=0;i<5;i++)
            rthread[i] = createThreadInSpace(L4_nilthread, (void *)synch_recv);

        /* Give threads time to start up or our pager may complain
         *  about us going away.
         */
        L4_ThreadSwitch(main_thread);
        sleep_a_bit();
        L4_ThreadSwitch(main_thread);

        for (i=0;i<5;i++)
            deleteThread(rthread[i]);
    }


}
END_TEST

/*
\begin{test}{SMT0302}
  \TestDescription{Thread creation and deletion of ping pong threads}
  \TestFunctionalityTested{Global scheduler, thread creation and deletion}
  \TestImplementationProcess{
    The following process is repeated:
    \begin{enumerate}
      \item Create two threads running IPC ping pong. Each thread is
      constrained to run on separate unused hardware threads.
      \item The threads are deleted.
    \end{enumerate}
    The main threads are constrained to running on hardware thread 0.

}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0302)
{
    int j;
    L4_ThreadId_t rthread[5];

    pingpong_cleanup = 0;

    for (j=0;j<TEST3_MAJOR_ITERATIONS;j++) {
        changeThreadCreationSMTBitmask(2);
        rthread[0] = createThreadInSpace(L4_nilthread, (void *)synch_pong);
        changeThreadCreationSMTBitmask(4);
        rthread[1] = createThreadInSpace(L4_nilthread, (void *)synch_ping);

        /* Give threads time to start up or our pager may complain
         *  about us going away.
         */
        L4_ThreadSwitch(main_thread);
        sleep_a_bit();
        L4_ThreadSwitch(main_thread);

        /* alternate deleting threads first */
        deleteThread(rthread[(j+1)%2]);
        deleteThread(rthread[j%2]);
    }
}
END_TEST

#define TEST303_MINOR 4
/*
\begin{test}{SMT0303}
  \TestDescription{Thread unwind of sending threads}
  \TestFunctionalityTested{Global scheduler, thread creation and deletion}
  \TestImplementationProcess{
    This is a thrashing ping pong.
    The following process is repeated:
    \begin{enumerate}
      \item Create one threads running IPC pong thread. This thread is
      constrained to one hardware thread.
      \item Create four IPC ping threads. These threads are not constrained.
      \item The pong thread is deleted.
      \item The remaining threads are then deleted.
    \end{enumerate}
    The main threads are constrained to running on hardware thread 0.

}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0303)
{
    int j, i;
    L4_ThreadId_t rthread[6];

    pingpong_cleanup = 0;
    pingpong_counter = 0;

    for (j=0;j<TEST3_MAJOR_ITERATIONS*4;j++) {
        changeThreadCreationSMTBitmask(2);
        rthread[0] = createThreadInSpace(L4_nilthread, (void *)synch_pong);

        changeThreadCreationSMTBitmask(63);
        for(i=0;i<TEST303_MINOR;i++) {
            rthread[i+1] = createThreadInSpace(L4_nilthread, (void *)synch_ping);
        }

        /* Give threads time to start up or our pager may complain
         *  about us going away.
         */
        sleep_a_bit();
        sleep_a_bit();

        deleteThread(rthread[0]); // delete pong

        for(i=0; i<TEST303_MINOR; i++)
            deleteThread(rthread[i+1]);
    }
}
END_TEST

START_TEST(SMT0304)
{
    int j, i;
    L4_ThreadId_t rthread[6];

    // smt0303 but with deletes in reverse order

    pingpong_cleanup = 0;
    pingpong_counter = 0;

    for (j=0;j<TEST3_MAJOR_ITERATIONS;j++) {
        changeThreadCreationSMTBitmask(2);
        rthread[0] = createThreadInSpace(L4_nilthread, (void *)synch_pong);

        changeThreadCreationSMTBitmask(63);
        for(i=0;i<TEST303_MINOR;i++) {
            rthread[i+1] = createThreadInSpace(L4_nilthread, (void *)synch_ping);
        }

        /* Give threads time to start up or our pager may complain
         *  about us going away.
         */
        L4_ThreadSwitch(main_thread);
        sleep_a_bit();
        sleep_a_bit();

        for(i=0; i<TEST303_MINOR; i++)
            deleteThread(rthread[i+1]);

        deleteThread(rthread[0]); // delete pong
    }
}
END_TEST

/*
\begin{test}{SMT0500}
  \TestDescription{IPC Ping pong constrained to same hardware thread}
  \TestFunctionalityTested{Global scheduler, IPC}
  \TestImplementationProcess{
    Two threads are created and constrained to the same hardware thread.
    This test passes if progress is made.
}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0500)
{
    L4_ThreadId_t rthread[2];


    pingpong_cleanup = 0;
    pingpong_counter = 0;

    changeThreadCreationSMTBitmask(8);
    rthread[0] = createThreadInSpace(L4_nilthread, (void *)synch_pong);
    rthread[1] = createThreadInSpace(L4_nilthread, (void *)synch_ping);

    L4_ThreadSwitch(main_thread);
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();

    printf("ppc %d\n", pingpong_counter);

    deleteThread(rthread[0]);
    deleteThread(rthread[1]);


    fail_unless(pingpong_counter > 10, "poor smt ping pong result\n");
}
END_TEST

/*
\begin{test}{SMT0600}
  \TestDescription{IPC Ping pong, constrained to different hardware threads}
  \TestFunctionalityTested{Global scheduler, IPC}
  \TestImplementationProcess{
    Two threads are created and constrained to different hardware threads.
    This test passes if progress is made.
}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
  \TestNotes{Hardware constraint specification is only used on supported
    architectures. For others, the threads will execute round robin.}
    \TestSeeAlso{See document \emph{L4 SMT Support} 10020:2005}
\end{test}
*/
START_TEST(SMT0600)
{
    L4_ThreadId_t rthread[2];


    pingpong_cleanup = 0;
    pingpong_counter = 0;

    changeThreadCreationSMTBitmask(16);
    rthread[0] = createThreadInSpace(L4_nilthread, (void *)synch_pong);
    changeThreadCreationSMTBitmask(32);
    rthread[1] = createThreadInSpace(L4_nilthread, (void *)synch_ping);

    L4_ThreadSwitch(main_thread);
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();
    sleep_a_bit();

    printf("ppc %d\n", pingpong_counter);

    deleteThread(rthread[0]);
    deleteThread(rthread[1]);


    fail_unless(pingpong_counter > 10, "poor smt ping pong result\n");
}
END_TEST


extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}

static void test_teardown(void)
{

}

TCase *
make_smt_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Multi-threaded (SMT) Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, SMT0100);
    tcase_add_test(tc, SMT0200);
    tcase_add_test(tc, SMT0300);
    tcase_add_test(tc, SMT0301);
    tcase_add_test(tc, SMT0302);
    tcase_add_test(tc, SMT0303);
    tcase_add_test(tc, SMT0304);
    tcase_add_test(tc, SMT0500);
    tcase_add_test(tc, SMT0600);

    return tc;
}
#else /* defined(SMT_ENABLED_ARCHITECTURE) */
TCase *
make_smt_tcase(void)
{
    return NULL;
}
#endif /* defined(SMT_ENABLED_ARCHITECTURE) */
