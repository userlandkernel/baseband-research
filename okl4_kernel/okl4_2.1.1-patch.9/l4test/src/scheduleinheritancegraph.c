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

#include <l4test/l4test.h>
#include <l4test/utility.h>

#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/kdebug.h>
#include <l4/mutex.h>

#include <stdio.h>
#include <stdlib.h>

/*
 * This schedule inheritance framework sets up the following graph, makes a
 * modification, then measures the effective priority of the enqueued thread.
 * T_n is a thread with priority n:
 * - T_3 holds mutex1,
 * - T_1 holds mutex2,
 * - T_2 and T_5 block trying to acquire mutex1,
 * - T_4 and T_7 block trying to acquire mutex2,
 * - T_6 and T_1 block sending IPC to T_3.
 *
 * The extended graph adds the following 2 threads:
 * - T_8 block sending IPC to T_5,
 * - T_6bis block sending to T_5.
 */

static L4_ThreadId_t setup_thread, main_thread;
static L4_ThreadId_t prio6_thread, prio1_thread, prio2_thread, prio5_thread, prio4_thread, prio7_thread;
static L4_ThreadId_t prio6bis_thread, prio8_thread;
static L4_ThreadId_t measure_thread;

static void
runnable_main_thread(void)
{
    L4_Word_t result, label;
    L4_MsgTag_t tag;
    L4_MutexId_t mutex1 = L4_MutexId(0);

    label = 0;
    result = L4_Lock(mutex1);
    fail_unless(result == 1, "L4_Lock() failed");
    while (1) {
        if (label == 0x3) {
            L4_Unlock(mutex1);
        }
        tag = L4_Make_MsgTag(0x1, 0);
        L4_Set_MsgTag(tag);
        L4_Send(setup_thread);
        tag = L4_Receive(setup_thread);
        label = L4_Label(tag);
    }
}

static void
sending_thread(void)
{
    L4_Word_t result;
    L4_MutexId_t mutex2 = L4_MutexId(1);
    L4_MsgTag_t tag;

    if (L4_UserDefinedHandle()) {
        result = L4_Lock(mutex2);
        fail_unless(result == 1, "L4_Lock() failed");
    }
    L4_LoadMR(0, 0);
    L4_Send(main_thread);

    tag = L4_Receive(setup_thread);
    if (L4_Label(tag) == 0x3) {
        L4_Unlock(mutex2);
        L4_LoadMR(0, 0);
        L4_Send(main_thread);
    }
    L4_WaitForever();
    while(1) ;
}

static void
other_sending_thread(void)
{
    L4_LoadMR(0, 0);
    L4_Send(prio5_thread);

    L4_WaitForever();
    while(1) ;
}

static void
locking_m1_thread(void)
{
    L4_MutexId_t mutex1 = L4_MutexId(0);

    L4_Lock(mutex1);

    L4_WaitForever();
    while(1) ;
}

static void
locking_m2_thread(void)
{
    L4_MutexId_t mutex2 = L4_MutexId(1);

    L4_Lock(mutex2);

    L4_WaitForever();
    while(1) ;
}

static void
measure_effective_prio_thread(void)
{
    L4_MsgTag_t tag;

    while (1) {
        tag = L4_Make_MsgTag(0x2, 0);
        L4_Set_MsgTag(tag);
        L4_Send(setup_thread);
    }
}


static void
setup_graph(void)
{
    main_thread = createThread(runnable_main_thread);
    L4_KDB_SetThreadName(main_thread, "main_thread");
    L4_Set_Priority(main_thread, 3);
    L4_Receive(main_thread);
    prio6_thread = createThread(sending_thread);
    L4_KDB_SetThreadName(prio6_thread, "prio6_thread");
    L4_Set_UserDefinedHandleOf(prio6_thread, 0);
    L4_Set_Priority(prio6_thread, 201);
    L4_Set_Priority(prio6_thread, 6);
    prio1_thread = createThread(sending_thread);
    L4_KDB_SetThreadName(prio1_thread, "prio1_thread");
    L4_Set_UserDefinedHandleOf(prio1_thread, 1);
    L4_Set_Priority(prio1_thread, 201);
    L4_Set_Priority(prio1_thread, 1);
    prio2_thread = createThread(locking_m1_thread);
    L4_KDB_SetThreadName(prio2_thread, "prio2_thread");
    L4_Set_Priority(prio2_thread, 201);
    L4_Set_Priority(prio2_thread, 2);
    prio5_thread = createThread(locking_m1_thread);
    L4_KDB_SetThreadName(prio5_thread, "prio5_thread");
    L4_Set_Priority(prio5_thread, 201);
    L4_Set_Priority(prio5_thread, 5);
    prio4_thread = createThread(locking_m2_thread);
    L4_KDB_SetThreadName(prio4_thread, "prio4_thread");
    L4_Set_Priority(prio4_thread, 201);
    L4_Set_Priority(prio4_thread, 4);
    prio7_thread = createThread(locking_m2_thread);
    L4_KDB_SetThreadName(prio7_thread, "prio7_thread");
    L4_Set_Priority(prio7_thread, 201);
    L4_Set_Priority(prio7_thread, 7);

    L4_LoadMR(0, 0);
    L4_Send(main_thread);

    measure_thread = createThread(measure_effective_prio_thread);
    L4_KDB_SetThreadName(measure_thread, "measure_thread");
    L4_Set_Priority(measure_thread, 9);
}

static void
setup_extended_graph(void)
{
    setup_graph();

    prio8_thread = createThread(other_sending_thread);
    L4_KDB_SetThreadName(prio8_thread, "prio8_thread");
    L4_Set_Priority(prio8_thread, 201);
    L4_Set_Priority(prio8_thread, 8);
    prio6bis_thread = createThread(other_sending_thread);
    L4_KDB_SetThreadName(prio6bis_thread, "prio6bis_thread");
    L4_Set_Priority(prio6bis_thread, 201);
    L4_Set_Priority(prio6bis_thread, 6);

    L4_Receive(main_thread);
    L4_LoadMR(0, 0);
    L4_Send(main_thread);
}

static void
measure_effective_prio(L4_Word_t expect_prio)
{
    L4_Word_t label, prio;
    L4_ThreadId_t any_thread;
    L4_MsgTag_t tag;

    prio = 9;
    do {
        prio--;
        L4_Set_Priority(measure_thread, prio);
        tag = L4_Wait(&any_thread);
        label = L4_Label(tag);
    } while ((label != 0x1) && (prio > 1));
    _fail_unless(prio == expect_prio, __FILE__, __LINE__, "Wrong effective priority: %lu", prio);
}

static void
delete_all(void)
{
    if (!L4_IsNilThread(prio7_thread)) {
        deleteThread(prio7_thread);
    }
    deleteThread(prio4_thread);
    if (!L4_IsNilThread(prio5_thread)) {
        deleteThread(prio5_thread);
    }
    deleteThread(prio2_thread);
    if (!L4_IsNilThread(prio1_thread)) {
        deleteThread(prio1_thread);
    }
    if (!L4_IsNilThread(prio6_thread)) {
        deleteThread(prio6_thread);
    }
    if (!L4_IsNilThread(prio6bis_thread)) {
        deleteThread(prio6bis_thread);
    }
    if (!L4_IsNilThread(prio8_thread)) {
        deleteThread(prio8_thread);
    }
    deleteThread(main_thread);
    deleteThread(measure_thread);
}

/*
\begin{test}{SIGRAPH0100}
  \TestDescription{Schedule inheritance graph: Null test}
  \TestImplementationProcess{
    No changes to the graph, effective priority of main thread should be 7.
  }
  \TestPostConditions{}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0100)
{
    setup_graph();
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0200}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 7 to 1, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0200)
{
    setup_graph();
    L4_Set_Priority(prio7_thread, 1);
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0201}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 7 to 1 and 6 to 1, effective priority of main thread should be 5.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0201)
{
    setup_graph();
    L4_Set_Priority(prio7_thread, 1);
    L4_Set_Priority(prio6_thread, 1);
    measure_effective_prio(5);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0202}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 1 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0202)
{
    setup_graph();
    L4_Set_Priority(prio1_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0203}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 2 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0203)
{
    setup_graph();
    L4_Set_Priority(prio2_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0204}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 3 (main thread) to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0204)
{
    setup_graph();
    L4_Set_Priority(main_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0205}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 4 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0205)
{
    setup_graph();
    L4_Set_Priority(prio4_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0206}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 5 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0206)
{
    setup_graph();
    L4_Set_Priority(prio5_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0207}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 6 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0207)
{
    setup_graph();
    L4_Set_Priority(prio6_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0208}
  \TestDescription{Schedule inheritance graph: change priority test}
  \TestImplementationProcess{
    Change priority of thread 7 to 8, effective priority of main thread should be 8.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0208)
{
    setup_graph();
    L4_Set_Priority(prio7_thread, 8);
    measure_effective_prio(8);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0300}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel edge between thread 7 and mutex2, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0300)
{
    setup_graph();
    L4_AbortOperation(prio7_thread);
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0301}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel edge between thread 4 and mutex2, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0301)
{
    setup_graph();
    L4_AbortOperation(prio4_thread);
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0302}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel IPC between thread 6 and main thread, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0302)
{
    setup_graph();
    L4_AbortOperation(prio6_thread);
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0303}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel IPC between thread 1 and main thread, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0303)
{
    setup_graph();
    L4_AbortOperation(prio1_thread);
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0304}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel edge between thread 2 and mutex1, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0304)
{
    setup_graph();
    L4_AbortOperation(prio2_thread);
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0305}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel edge between thread 5 and mutex1, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0305)
{
    setup_graph();
    L4_AbortOperation(prio5_thread);
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0306}
  \TestDescription{Schedule inheritance graph: cancel operation test}
  \TestImplementationProcess{
    Cancel IPC between thread 1 and main thread and between thread 6 and main thread, 
    effective priority of main thread should be 5.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0306)
{
    setup_graph();
    L4_AbortOperation(prio1_thread);
    L4_AbortOperation(prio6_thread);
    measure_effective_prio(5);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0400}
  \TestDescription{Schedule inheritance graph: mutex release test}
  \TestImplementationProcess{
    Thread 1 releases mutex2, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0400)
{
    L4_MsgTag_t tag;

    setup_graph();
    L4_AbortOperation(prio1_thread);
    tag = L4_Make_MsgTag(0x3, 0);
    L4_Set_MsgTag(tag);
    L4_Send(prio1_thread);
    L4_Set_Priority(prio1_thread, 201);
    L4_Set_Priority(prio1_thread, 1);
    L4_Receive(main_thread);
    L4_LoadMR(0, 0);
    L4_Send(main_thread);
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0401}
  \TestDescription{Schedule inheritance graph: mutex release test}
  \TestImplementationProcess{
    Thread 3 (main thread) releases mutex1, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0401)
{
    L4_MsgTag_t tag;

    setup_graph();
    L4_Receive(main_thread);
    tag = L4_Make_MsgTag(0x3, 0);
    L4_Set_MsgTag(tag);
    L4_Send(main_thread);
    L4_Receive(main_thread);
    L4_LoadMR(0, 0);
    L4_Send(main_thread);
    measure_effective_prio(7);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0500}
  \TestDescription{Schedule inheritance graph: thread deletion test}
  \TestImplementationProcess{
    Delete thread 7, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0500)
{
    setup_graph();
    deleteThread(prio7_thread);
    prio7_thread = L4_nilthread;
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0501}
  \TestDescription{Schedule inheritance graph: thread deletion test}
  \TestImplementationProcess{
    Delete thread 1, effective priority of main thread should be 6.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0501)
{
    setup_graph();
    deleteThread(prio1_thread);
    prio1_thread = L4_nilthread;
    measure_effective_prio(6);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0502}
  \TestDescription{Schedule inheritance graph: thread deletion test}
  \TestImplementationProcess{
    Delete thread 1 and thread 6, effective priority of main thread should be 5.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0502)
{
    setup_graph();
    deleteThread(prio1_thread);
    prio1_thread = L4_nilthread;
    deleteThread(prio6_thread);
    prio6_thread = L4_nilthread;
    measure_effective_prio(5);
    delete_all();
}
END_TEST

/*
\begin{test}{SIGRAPH0503}
  \TestDescription{Schedule inheritance graph: thread deletion test}
  \TestImplementationProcess{
    Delete thread 5, effective priority of main thread should be 7.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SIGRAPH0503)
{
    setup_extended_graph();
    deleteThread(prio5_thread);
    prio5_thread = L4_nilthread;
    measure_effective_prio(7);
    delete_all();
}
END_TEST

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    L4_MutexId_t mutex1 = L4_MutexId(0);
    L4_MutexId_t mutex2 = L4_MutexId(1);

    initThreads(0);
    setup_thread = test_tid;
    L4_CreateMutex(mutex1);
    L4_CreateMutex(mutex2);
    prio6bis_thread = L4_nilthread;
    prio8_thread = L4_nilthread;
}

static void test_teardown(void)
{
    L4_MutexId_t mutex1 = L4_MutexId(0);
    L4_MutexId_t mutex2 = L4_MutexId(1);

    L4_DeleteMutex(mutex1);
    L4_DeleteMutex(mutex2);
}

TCase *
make_schedule_inheritance_graph_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Schedule inheritance graph");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, SIGRAPH0100);
    tcase_add_test(tc, SIGRAPH0200);
    tcase_add_test(tc, SIGRAPH0201);
    tcase_add_test(tc, SIGRAPH0202);
    tcase_add_test(tc, SIGRAPH0203);
    tcase_add_test(tc, SIGRAPH0204);
    tcase_add_test(tc, SIGRAPH0205);
    tcase_add_test(tc, SIGRAPH0206);
    tcase_add_test(tc, SIGRAPH0207);
    tcase_add_test(tc, SIGRAPH0208);
    tcase_add_test(tc, SIGRAPH0300);
    tcase_add_test(tc, SIGRAPH0301);
    tcase_add_test(tc, SIGRAPH0302);
    tcase_add_test(tc, SIGRAPH0303);
    tcase_add_test(tc, SIGRAPH0304);
    tcase_add_test(tc, SIGRAPH0305);
    tcase_add_test(tc, SIGRAPH0306);
    tcase_add_test(tc, SIGRAPH0400);
    tcase_add_test(tc, SIGRAPH0401);
    tcase_add_test(tc, SIGRAPH0500);
    tcase_add_test(tc, SIGRAPH0501);
    tcase_add_test(tc, SIGRAPH0502);
    tcase_add_test(tc, SIGRAPH0503);

    return tc;
}
