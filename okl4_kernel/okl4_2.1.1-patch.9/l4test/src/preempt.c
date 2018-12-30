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

#include <stdio.h>
#include <assert.h>
#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/schedule.h>

static L4_ThreadId_t main_thread, high_prio_thread;
static int g_preempted = 0;

/* Utility functions ------------------------------------------------------- */

static L4_Bool_t
threadWasHalted(L4_ThreadId_t thread)
{
    L4_Word_t dummy;
    L4_ThreadId_t r, dummy_tid;
    L4_ThreadState_t state;

    r = L4_ExchangeRegisters(thread, L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                             &state.raw, &dummy, &dummy, &dummy, &dummy,
                             &dummy_tid);
    assert(r.raw == thread.raw);

    return L4_ThreadWasHalted(state);
}

static void
high_priority(void)
{
    int i;

    L4_KDB_SetThreadName(L4_Myself(), "hi");
    L4_KDB_PrintChar('H');
    L4_KDB_PrintChar('\n');

    for (i = 0; i < 4; i++) {
        L4_Start(main_thread);
        L4_Stop_Thread(L4_Myself());    /* Suspend itself */
        L4_KDB_PrintChar('T');
        L4_KDB_PrintChar('\n');
    }
    L4_Start(main_thread);
    L4_Send(L4_Myself());
}

static void *restart_exception;

static void
foo(void)
{
    static int iter = 0;

    L4_DisablePreemptionCallback();
    if (threadWasHalted(main_thread)) {
        L4_Start(main_thread); /* We ensure the test will not block if priorities are not respected */
    }
    L4_KDB_PrintChar('F');
    L4_KDB_PrintChar('\n');
    fail_unless(threadWasHalted(high_prio_thread),
                "High priority thread is resumed and do not run");
    iter++;
    if (iter == 4) {
        iter = 0;
        L4_Start(high_prio_thread); /* Resume high prio thread */
    }
    /* 
     * Note, there must not be any libc or other calls which using locking code 
     * between L4_EnablePreemptionCallback() and L4_DisablePreemptionCallback().
     */
    L4_Set_PreemptCallbackIP((L4_Word_t) restart_exception);
    L4_EnablePreemptionCallback();
    g_preempted = 0;
    while (1) {
        if (g_preempted != 0) {
            L4_DisablePreemptionCallback();
            fail("Preemption Callback failed : thread did not restart at expected IP after preemption");
        }
    }
}

static void
low_priority(void)
{
    L4_KDB_SetThreadName(L4_Myself(), "lo");
    L4_KDB_PrintChar('L');
    L4_KDB_PrintChar('\n');

    restart_exception = (void *)foo;
    foo();
}

/* Preemption tests -------------------------------------------- */

/* 
\begin{test}{PREEMPT0100}
  \TestDescription{Check the preemption call back functionality}
  \TestFunctionalityTested{L4\_Set\_PreemptCallbackIP}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create two threads
      \item Set a different priority level for each thread
      \item Check that the low priority thread is restarted on preemption while high priority thread is suspended
      \item Check that the low priority is preempted by the high priority thread when the latter is resumed
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test} 
*/
START_TEST(PREEMPT0100)
{
    L4_ThreadId_t low_prio_thread;
    L4_Word_t result;
    int i;

    high_prio_thread = createThread(high_priority);
    result = L4_Set_Priority(high_prio_thread, 120);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    result = L4_Set_Timeslice(high_prio_thread, 10000);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Timeslice() failed to set Time slice");

    low_prio_thread = createThread(low_priority);
    result = L4_Set_Priority(low_prio_thread, 100);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    result = L4_Set_Timeslice(low_prio_thread, 10000);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Timeslice() failed to set Time slice");

    for (i = 0; i < 25; i++) {
        if (checkSending(high_prio_thread))
            break;
        if (threadWasHalted(high_prio_thread)) {
            g_preempted = 1;
            L4_ThreadSwitch(low_prio_thread);
        } else {
            L4_Stop(L4_Myself());
        }
    }
    fail_if(i == 25, "High priority thread did not finish");

    deleteThread(high_prio_thread);
    deleteThread(low_prio_thread);
}
END_TEST

/* ------------------------------------------------------------------------- */

extern L4_ThreadId_t test_tid;

static void
test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}

static void
test_teardown(void)
{
}

TCase *
make_preempt_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Preempt Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    /* Disable PREEMPT0100 test until it can be fixed up */
    //tcase_add_test(tc, PREEMPT0100);
    (void)PREEMPT0100;

    return tc;
}
