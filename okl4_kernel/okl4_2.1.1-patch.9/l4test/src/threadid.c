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

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <stddef.h>
#include <l4/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <l4/schedule.h>
#include <l4/config.h>

/* Calculate how many bits should be in each field. */

#if defined(L4_64BIT)
# define VERSION_BITS   32
# define THREAD_BITS    32
#else
# define VERSION_BITS   14
# define THREAD_BITS    18
#endif


/*
 * Define a random thread and version number.  We don't really care
 * about the value so long as it is non-zero.
 */
#define RANDOM_THREAD_NO 8
#define RANDOM_VERSION 16

/*
\begin{test}{tid0100}
  \TestDescription{Verify values for constant tids nilthread, anythread, anylocalthread, waitnotify}
  \TestFunctionalityTested{Thread ID constants}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Check that L4\_nilthread is all-bits 0
      \item Check that L4\_anythread is all-bits 1
      \item Check that the bottom bits of L4\_anylocalthread are 0
      \item Check that L4\_anylocalthread is -64L
      \item Check that L4\_waitnotify is -2L
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0100)
{
    L4_ThreadId_t tmp;
    char bit_buf[sizeof(L4_ThreadId_t)];

    /* Check the values of the constants. */
    fail_unless(L4_nilthread.raw == 0, "Non zero L4_nilthread.");

    memset(bit_buf, '\0', sizeof(bit_buf));
    tmp = L4_nilthread;
    fail_unless(memcmp (&tmp, bit_buf, sizeof(bit_buf)) == 0,
                 "L4_nilthread isn't all-bits 0.");

    memset(bit_buf, '\xff', sizeof(bit_buf));
    tmp = L4_anythread;
    fail_unless(memcmp (&tmp, bit_buf, sizeof(bit_buf)) == 0,
                 "L4_anythread isn't all-bits 1.");

    fail_unless((L4_anylocalthread.raw & 0x3f) == 0,
                 "Bottom bits of L4_anylocalthread not 0.");
    fail_unless(L4_anylocalthread.raw == -64L,
                 "L4_anylocalthread not -64L.");

    fail_unless(L4_waitnotify.raw == -2L,
                 "L4_waitnotify not 0.");

}
END_TEST

/*
\begin{test}{tid0200}
  \TestDescription{Test GlobalId with simple arguments}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Initialise a ThreadId using \Func{GlobalId} using arguments (0, 0)
      \item Check that the returned tid is a nilthread
      \item Initialise a ThreadId using \Func{GlobalId} using arguments (1, 2)
      \item Check that its thread\_no and version are correct 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0200)
{
    L4_ThreadId_t id;

    id = L4_GlobalId (0, 0);
    fail_unless(id.raw == 0,
                 "(0, 0) not a nilthread.");

    id = L4_GlobalId (1, 2);
    fail_unless(id.global.X.thread_no == 1, "Thread Id not set.");
    fail_unless(id.global.X.version == 2, "Version not set.");

    /* Boundary cases really checked in other tests. */
}
END_TEST

/*
\begin{test}{tid0300}
  \TestDescription{Test GlobalId correctly truncates incoming thread\_no argument}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{GlobalId} with thread\_no argument 1 smaller than largest possible value, and again with largest possible value
      \item Check that all bits are retained
      \item Invoke \Func{GlobalId} with thread\_no argument larger than largest possible value
      \item Check that these extra bits are duly discarded
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0300)
{
    L4_ThreadId_t id;

    id = L4_GlobalId (((L4_Word_t)1 << THREAD_BITS) - 2, RANDOM_VERSION);
    fail_unless(L4_ThreadNo (id) == ((L4_Word_t)1 << THREAD_BITS) - 2,
                 "Thread Id unexpectedly changed.");
    fail_unless(L4_Version (id) == RANDOM_VERSION,
                 "Version unexpectedly changed.");

    id = L4_GlobalId (((L4_Word_t)1 << THREAD_BITS) - 1, RANDOM_VERSION);
    fail_unless(L4_ThreadNo (id) == ((L4_Word_t)1 << THREAD_BITS) - 1,
                 "Thread Id unexpectedly changed.");
    fail_unless(L4_Version (id) == RANDOM_VERSION,
                 "Version unexpectedly changed.");

    id = L4_GlobalId (((L4_Word_t)1 << THREAD_BITS), RANDOM_VERSION);
    fail_unless(L4_ThreadNo (id) == 0
                 , "Thread Id not truncated.");
    fail_unless(L4_Version (id) == RANDOM_VERSION,
                 "Version affected by truncation.");

    id = L4_GlobalId (((L4_Word_t)1 << THREAD_BITS) + 1, RANDOM_VERSION);
    fail_unless(L4_ThreadNo (id) == 1,
                 "Thread Id not truncated.");
    fail_unless(L4_Version (id) == RANDOM_VERSION,
                 "Version affected by truncation.");

}
END_TEST

/*
\begin{test}{tid0400}
  \TestDescription{Test GlobalId correctly truncates incoming version argument}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{GlobalId} with thread\_no argument 1 smaller than largest possible value, and again with largest possible value
      \item Check that all bits are retained
      \item Invoke \Func{GlobalId} with thread\_no argument larger than largest possible value
      \item Check that these extra bits are duly discarded
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0400)
{
    L4_ThreadId_t id;

    id = L4_GlobalId (RANDOM_THREAD_NO, ((L4_Word_t)1 << VERSION_BITS) - 2);
    fail_unless(L4_ThreadNo (id) == RANDOM_THREAD_NO,
                 "Thread Id unexpectedly changed.");
    fail_unless(L4_Version (id) == ((L4_Word_t)1 << VERSION_BITS) - 2,
                 "Version unexpectedly changed.");

    id = L4_GlobalId (RANDOM_THREAD_NO, ((L4_Word_t)1 << VERSION_BITS) - 1);
    fail_unless(L4_ThreadNo (id) == RANDOM_THREAD_NO,
                 "Thread Id unexpectedly changed.");
    fail_unless(L4_Version (id) == ((L4_Word_t)1 << VERSION_BITS) - 1,
                 "Version unexpectedly changed.");

    id = L4_GlobalId (RANDOM_THREAD_NO, ((L4_Word_t)1 << VERSION_BITS));
    fail_unless(L4_ThreadNo (id) == RANDOM_THREAD_NO,
                 "Thread Id affected by truncation.");
    fail_unless(L4_Version (id) == 0,
                 "Version not truncated");

    id = L4_GlobalId (RANDOM_THREAD_NO, ((L4_Word_t)1 << VERSION_BITS) + 1);
    fail_unless(L4_ThreadNo (id) == RANDOM_THREAD_NO,
                 "Thread Id affected by truncation.");
    fail_unless(L4_Version (id) == 1,
                 "Version not truncated.");
}
END_TEST

/*
\begin{test}{tid0500}
  \TestDescription{Verify correct functionality of L4\_Version}
  \TestFunctionalityTested{\Func{Version}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{GlobalId} for multiple valid version arguments
      \item Check that the value obtained by running L4\_Version match the original inputs
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0500)
{
    int i;

    for (i = 0; i < VERSION_BITS; i++) {
        L4_ThreadId_t id = L4_GlobalId (RANDOM_THREAD_NO, (1UL << i));

        _fail_unless(L4_Version (id) == (1UL << i), __FILE__, __LINE__,
                     "Unexpected version (expected 0x%lx, got 0x%lx).",
                     (1UL << i), L4_Version (id));
    }
}
END_TEST

/*
\begin{test}{tid0600}
  \TestDescription{Verify correct functionality of L4\_ThreadNo}
  \TestFunctionalityTested{\Func{ThreadNo}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{GlobalId} for multiple valid thread_no arguments
      \item Check that the value obtained by running L4\_Version match the original inputs
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0600)
{
    int i;

    for (i = 0; i < THREAD_BITS; i++) {
        L4_ThreadId_t id = L4_GlobalId ((1UL << i), RANDOM_VERSION);

        _fail_unless(L4_ThreadNo (id) == (1UL << i), __FILE__, __LINE__,
                     "Unexpected thread id (expected 0x%lx, got 0x%lx).",
                     (1UL << i), L4_Version (id));
    }
}
END_TEST

/*
\begin{test}{tid0700}
  \TestDescription{Verify correct functionality of IsThreadEqual}
  \TestFunctionalityTested{\Func{IsThreadEqual}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Check constant tids (nilthread, anythread, etc) equal themselves
      \item Check constant tids do not equal each other
      \item Check normal tid matches itself, and not other normal tids with a different thread\_no or version
      \item Check normal tid does not equal constant tid anythread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0700)
{
    L4_ThreadId_t id1 = L4_GlobalId (20, 20);

    fail_unless(L4_IsThreadEqual (L4_nilthread, L4_nilthread),
                 "nilthread doesn't equal itself");
    fail_unless(L4_IsThreadEqual (L4_anythread, L4_anythread),
                 "anythread doesn't equal itself");
    fail_unless(L4_IsThreadEqual (L4_anylocalthread, L4_anylocalthread),
                 "anylocalthread doesn't equal itself");
    fail_unless(L4_IsThreadEqual (L4_waitnotify, L4_waitnotify),
                 "waitnotify doesn't equal itself");

    fail_unless(!L4_IsThreadEqual (L4_nilthread, L4_anythread),
                 "nilthread equals anythread");
    fail_unless(!L4_IsThreadEqual (L4_anythread, L4_anylocalthread),
                 "anythread equals anylocalthread");
    fail_unless(!L4_IsThreadEqual (L4_anylocalthread, L4_waitnotify),
                 "anylocalthread equals waitnotify");
    fail_unless(!L4_IsThreadEqual (L4_waitnotify, L4_nilthread),
                 "waitnotify equals nilthread");

    fail_unless(L4_IsThreadEqual (id1, L4_GlobalId (20, 20)),
                 "Normal Id doesn't equal itself.");
    fail_unless(!L4_IsThreadEqual (id1, L4_GlobalId (50, 20)),
                 "Different threads equal");
    fail_unless(!L4_IsThreadEqual (id1, L4_GlobalId (20, 21)),
                 "Different versions equal");
    fail_unless(!L4_IsThreadEqual (id1, L4_anythread),
                 "Anythread matches normal thread");
}
END_TEST

/*
\begin{test}{tid0800}
  \TestDescription{Verify correct functionality of IsThreadNotEqual}
  \TestFunctionalityTested{\Func{IsThreadNotEqual}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Check constant tids (nilthread, anythread, etc) against themselves
      \item Check constant tids against each other
      \item Check normal tid against itself, and against other normal tids with differing thread\_no or version
      \item Check normal tid does not equal constant tid anythread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0800)
{
    L4_ThreadId_t id1 = L4_GlobalId (20, 20);

    fail_unless(!L4_IsThreadNotEqual (L4_nilthread, L4_nilthread),
                 "nilthread doesn't equal itself");
    fail_unless(!L4_IsThreadNotEqual (L4_anythread, L4_anythread),
                 "anythread doesn't equal itself");
    fail_unless(!L4_IsThreadNotEqual (L4_anylocalthread, L4_anylocalthread),
                 "anylocalthread doesn't equal itself");
    fail_unless(!L4_IsThreadNotEqual (L4_waitnotify, L4_waitnotify),
                 "waitnotify doesn't equal itself");

    fail_unless(L4_IsThreadNotEqual (L4_nilthread, L4_anythread),
                 "nilthread equals anythread");
    fail_unless(L4_IsThreadNotEqual (L4_anythread, L4_anylocalthread),
                 "anythread equals anylocalthread");
    fail_unless(L4_IsThreadNotEqual (L4_anylocalthread, L4_waitnotify),
                 "anylocalthread equals waitnotify");
    fail_unless(L4_IsThreadNotEqual (L4_waitnotify, L4_nilthread),
                 "waitnotify equals nilthread");

    fail_unless(!L4_IsThreadNotEqual (id1, L4_GlobalId (20, 20)),
                 "Normal Id doesn't equal itself.");
    fail_unless(L4_IsThreadNotEqual (id1, L4_GlobalId (50, 20)),
                 "Different threads equal");
    fail_unless(L4_IsThreadNotEqual (id1, L4_GlobalId (20, 21)),
                 "Different versions equal");
    fail_unless(L4_IsThreadNotEqual (id1, L4_anythread),
                 "Anythread matches normal thread");
}
END_TEST

/*
\begin{test}{tid0900}
  \TestDescription{Verify correct functionality of IsNilThread}
  \TestFunctionalityTested{\Func{IsNilThread}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Check constant tid nilthread
      \item Check none of the other constant tids are a nilthread
      \item Check none of Myself(), MyGlobalId() or a normal tid are a nilthread
      \item Check GlobalId(0, 0) is a nilthread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid0900)
{
    fail_unless(L4_IsNilThread (L4_nilthread),
                 "nilthread isn't.");
    fail_unless(!L4_IsNilThread (L4_anythread),
                 "anythread is a nilthread.");
    fail_unless(!L4_IsNilThread (L4_waitnotify),
                 "waitnotify is a nilthread");
    fail_unless(!L4_IsNilThread (L4_anylocalthread),
                 "anylocalthread is a nilthread");
    fail_unless(!L4_IsNilThread (L4_Myself ()),
                 "Myself() is a nilthread");
    fail_unless(!L4_IsNilThread (L4_MyGlobalId ()),
                 "MyGlobalId() is a nilthread");
    fail_unless(!L4_IsNilThread (L4_GlobalId (10, 10)),
                 "GlobalId(10, 10) is a nilthread");
    fail_unless(L4_IsNilThread (L4_GlobalId (0, 0)),
                 "GlobalId(0, 0) isn't a nilthread");

}
END_TEST


void waiting_thread(void);
void waiting_notify_thread(void);
void waiting_cancel_thread(void);

void waiting_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t partner = L4_anythread;

    tag = L4_Wait(&partner);
    fail_unless(L4_IpcFailed(tag), "Thread unexpectedly received message successfully");

    L4_Send(L4_Myself());
}

void waiting_notify_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t mask;

    L4_Set_NotifyMask(1);
    L4_Accept(L4_AsynchItemsAcceptor);
    tag = L4_WaitNotify(&mask);
    fail_unless(L4_IpcFailed(tag), "Thread unexpectedly received asynchronous message successfully");

    L4_Send(L4_Myself());
}

void waiting_cancel_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t partner = L4_anythread;

    tag = L4_Wait(&partner);
    fail_unless(L4_IpcFailed(tag), "Thread unexpectedly received message successfully");
    fail_if(L4_ErrorCode() == L4_ErrCanceled, "IPC operation has been cancelled");

    L4_Send(L4_Myself());
}

/*
\begin{test}{tid1000}
  \TestDescription{Verify failure of synchronous IPCs sent to invalid tids}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new waiting thread
      \item Try to send synchronous IPC to all possible invalid tids obtained by invoking \Func{GlobalId}
      \item Verify that all fail and none are received by the waiting thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid1000)
{
    L4_Word_t tno, max_tno, min_tno, waiting_tno;
    L4_ThreadId_t tid, waiting_tid;
    L4_MsgTag_t tag;
    L4_Word_t result;


    waiting_tid = createThread(waiting_thread);
    result = L4_Set_Priority(waiting_tid, 200);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    L4_ThreadSwitch(waiting_tid); // Give it time to start waiting for IPC operation

    min_tno = 3; // Exclude root, test handler and main test threads from the list of thread ids
    max_tno = 1UL << L4_GetThreadBits();
    waiting_tno = L4_ThreadNo(waiting_tid);
    for (tno = min_tno; tno < max_tno; tno++)
    {
        if (tno != waiting_tno) {
            tid = L4_GlobalId(tno, 1);
            tag = L4_Send_Nonblocking(tid);
            fail_unless(L4_IpcFailed(tag), "Message succesfully sent to invalid thread id");
        }
    }

    L4_ThreadSwitch(waiting_tid); // Give it time to fail if something wrong happened
    deleteThread(waiting_tid);
}
END_TEST

/*
\begin{test}{tid1001}
  \TestDescription{Verify failure of asynchronous IPCs sent to invalid tids}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new waiting thread
      \item Try to send asynchronous IPC to all possible invalid tids obtained by invoking \Func{GlobalId}
      \item Verify that none are received by the waiting thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid1001)
{
    L4_Word_t tno, max_tno, min_tno, waiting_tno;
    L4_ThreadId_t invalid_tid, waiting_tid;
    L4_Word_t result;


    waiting_tid = createThread(waiting_notify_thread);
    result = L4_Set_Priority(waiting_tid, 200);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    L4_ThreadSwitch(waiting_tid); // Give it time to start waiting for IPC operation

    min_tno = 3; // Exclude root, test handler and main test threads from the list of thread ids
    max_tno = 1UL << L4_GetThreadBits();
    waiting_tno = L4_ThreadNo(waiting_tid);
    for (tno = min_tno; tno < max_tno; tno++)
    {
        if (tno != waiting_tno) {
            invalid_tid = L4_GlobalId(tno, 1);
            L4_Notify(invalid_tid, 0x1);
        }
    }

    L4_ThreadSwitch(waiting_tid); // Give it time to fail if something wrong happened
    deleteThread(waiting_tid);
}
END_TEST

/*
\begin{test}{tid1100}
  \TestDescription{Verify that attempts to abort IPC to invalid tids fail}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new waiting thread
      \item Attempt to abort IPC to all possible invalid tids obtained by invoking \Func{GlobalId}
      \item Verify that no abort notification is received by the waiting thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid1100)
{
    L4_Word_t tno, max_tno, min_tno, waiting_tno;
    L4_ThreadId_t invalid_tid, waiting_tid;
    L4_Word_t result, dummy;
    L4_ThreadId_t dummy_id;

    waiting_tid = createThread(waiting_cancel_thread);
    result = L4_Set_Priority(waiting_tid, 200);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    L4_ThreadSwitch(waiting_tid); // Give it time to start waiting for IPC operation

    min_tno = 3; // Exclude root, test handler and main test threads from the list of thread ids
    max_tno = 1UL << L4_GetThreadBits();
    waiting_tno = L4_ThreadNo(waiting_tid);
    for (tno = min_tno; tno < max_tno; tno++)
    {
        if (tno != waiting_tno) {
            invalid_tid = L4_GlobalId(tno, 1);
            (void)L4_ExchangeRegisters(invalid_tid, L4_ExReg_AbortIPC,
                                       0, 0, 0, 0, L4_nilthread,
                                       &dummy, &dummy, &dummy, &dummy, &dummy,
                                       &dummy_id);
        }
    }

    L4_ThreadSwitch(waiting_tid); // Give it time to fail if something wrong happened
    deleteThread(waiting_tid);
}
END_TEST

/*
\begin{test}{tid1200}
  \TestDescription{Verify that attempts to delete invalid tids fail}
  \TestFunctionalityTested{\Func{GlobalId}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new waiting thread
      \item Attempt to delete known invalid tids by invoking \Func{ThreadControl}
      \item Verify that they fail and returns the correct error code, and that no attempts are received by the waiting thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tid1200)
{
    L4_Word_t tno, max_tno, min_tno, waiting_tno;
    L4_ThreadId_t invalid_tid, waiting_tid;
    L4_Word_t result;

    waiting_tid = createThread(waiting_thread);
    result = L4_Set_Priority(waiting_tid, 200);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    L4_ThreadSwitch(waiting_tid); // Give it time to start waiting for IPC operation

    min_tno = 3; // Exclude root, test handler and main test threads from the list of thread ids
    max_tno = 1UL << L4_GetThreadBits();
    waiting_tno = L4_ThreadNo(waiting_tid);
    for (tno = min_tno; tno < max_tno; tno++)
    {
        invalid_tid = L4_GlobalId(tno, 1);
        if (tno != waiting_tno && !isSystemThread(invalid_tid)) {
            result = L4_ThreadControl(invalid_tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
            fail_unless(result == 0, "L4_ThreadControl() deleted an invalid thread id");
            fail_unless(L4_ErrorCode() == L4_ErrInvalidThread, "Wrong Error code");
        }
    }

    deleteThread(waiting_tid);
}
END_TEST

static void test_setup(void)
{
    initThreads(0);
}


static void test_teardown(void)
{
}

TCase *
make_thread_id_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Thread Id Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, tid0100);
    tcase_add_test(tc, tid0200);
    tcase_add_test(tc, tid0300);
    tcase_add_test(tc, tid0400);
    tcase_add_test(tc, tid0500);
    tcase_add_test(tc, tid0600);
    tcase_add_test(tc, tid0700);
    tcase_add_test(tc, tid0800);
    tcase_add_test(tc, tid0900);
    tcase_add_test(tc, tid1000);
    tcase_add_test(tc, tid1001);
    tcase_add_test(tc, tid1100);
    tcase_add_test(tc, tid1200);

    return tc;
}
