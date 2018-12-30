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

#include <l4/mutex.h>
#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/config.h>
#include <stdio.h>
#include <stdlib.h>

extern word_t kernel_max_mutexes;

static L4_ThreadId_t main_thread;

static void
create_delete_mutex_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t label;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    tag = L4_Receive(main_thread);
    label = L4_Label(tag);

    if (label == 0xc) {
        res = L4_CreateMutex(m);
        fail_unless(res == 0, "L4_CreateMutex() did not fail");
        fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
    }
    if (label == 0xd) {
        res = L4_DeleteMutex(m);
        fail_unless(res == 0, "L4_DeleteMutex() did not fail");
        fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
    }

    L4_Call(main_thread);
    L4_WaitForever();
}

static void
lock_mutex_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t label;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    tag = L4_Receive(main_thread);
    while (1) {
        label = L4_Label(tag);
        if (label == 0xa) {
            res = L4_Lock(m);
            fail_unless(res == 1, "L4_Lock() failed");
            L4_Unlock(m);
        }
        if (label == 0xb) {
            res = L4_TryLock(m);
            fail_unless(res == 0, "L4_TryLock() did not fail");
            fail_unless(L4_ErrorCode() == L4_ErrMutexBusy, "Wrong error code");
        }

        tag = L4_Call(main_thread);
    }

    L4_WaitForever();
}


/* Mutex creation and deletion ----------------------------------------------*/

/*
\begin{test}{MUTEX0100}
  \TestDescription{Create a mutex}
  \TestFunctionalityTested{\Func{L4\_CreateMutex}}
  \TestImplementationProcess{
    Call \Func{L4\_CreateMutex} and check it returns successfully.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0100)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
}
END_TEST

/*
\begin{test}{MUTEX0101}
  \TestDescription{Delete a mutex}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_DeleteMutex} with the newly created mutex and 
        checks it returns successfully.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0101)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST

/*
\begin{test}{MUTEX0102}
  \TestDescription{Verify \Func{DeleteMutex} correctly deletes a mutex}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_DeleteMutex} with the newly created mutex and 
        checks it returns successfully.
        \item Call \Func{L4\_CreateMutex} with the deleted mutex and check 
        it returns successfully.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0102)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
}
END_TEST

/*
\begin{test}{MUTEX0103}
  \TestDescription{Verify there is no mutex memory leak}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex} and \Func{L4\_CreateMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create the maximum number of mutexes allowed.
        \item Delete all of them.
        \item Create the same number again.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0103)
{
    int i;
    word_t res;
    for (i = 0; i < kernel_max_mutexes; i++) {
        res = L4_CreateMutex(L4_MutexId(i));
        fail_unless(res == 1, "L4_CreateMutex() failed");
    }
    for (i = 0; i < kernel_max_mutexes; i++) {
        res = L4_DeleteMutex(L4_MutexId(i));
        fail_unless(res == 1, "L4_DeleteMutex() failed");
    }
    for (i = 0; i < kernel_max_mutexes; i++) {
        res = L4_CreateMutex(L4_MutexId(i));
        fail_unless(res == 1, "L4_CreateMutex() failed");
    }
}
END_TEST


/* L4_MutexControl Error codes (Create, Delete) ------------------------------------*/

/*
\begin{test}{MUTEX0200}
  \TestDescription{Verify MutexControl does not overcreate a mutex}
  \TestFunctionalityTested{\Func{L4\_CreateMutex}}
  \TestImplementationProcess{
     \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_CreateMutex} with the newly created mutex and 
        check it returns an error.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0200)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_CreateMutex(m);
    fail_unless(res == 0, "L4_CreateMutex() did not fail");
    fail_unless(L4_ErrorCode() != L4_ErrMutexBusy, "Wrong error code");
}
END_TEST

/*
\begin{test}{MUTEX0201}
  \TestDescription{Verify MutexControl checks validity of mutex}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex}}
  \TestImplementationProcess{
    Call \Func{L4\_DeleteMutex} with the a non yet created mutex and 
    checks it returns an error.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0201)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_DeleteMutex(m);
    fail_unless(res == 0, "L4_DeleteMutex() did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidMutex, "Wrong error code");
}
END_TEST

/*
\begin{test}{MUTEX0202}
  \TestDescription{Verify MutexControl handles an invalid mutex id}
  \TestFunctionalityTested{\Func{L4\_CreateMutex}}
  \TestImplementationProcess{
    Call \Func{L4\_CreateMutex} with an invalid mutex id and 
    check it returns an error.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0202)
{
    word_t res;
    L4_MutexId_t m = L4_MutexId(kernel_max_mutexes + 1);

    res = L4_CreateMutex(m);
    fail_unless(res == 0, "L4_CreateMutex() did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidMutex, "Wrong error code");
}
END_TEST

/*
\begin{test}{MUTEX0203}
  \TestDescription{Verify a locked mutex can not be deleted}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_Lock} and check it returns successfully.
        \item Call \Func{L4\_DeleteMutex} and checks it returns an error.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0203)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 0, "L4_DeleteMutex() did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrMutexBusy, "Wrong error code");
    L4_Unlock(m);
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST


/* Unprivileged thread ------------------------------------------------------*/

/*
\begin{test}{MUTEX0300}
  \TestDescription{Verify unprivileged thread can not create mutex}
  \TestFunctionalityTested{\Func{L4\_CreateMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create unprivileged thread.
        \item Unprivileged thread call \Func{L4\_CreateMutex} and checks it 
        returns an error.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0300)
{
    L4_ThreadId_t unpriv_thread;
    L4_MsgTag_t tag;

    unpriv_thread = createThreadInSpace(L4_nilspace, create_delete_mutex_thread);
    L4_Set_Label(&tag, 0xc);
    L4_Set_MsgTag(tag);
    L4_Call(unpriv_thread);

    deleteThread(unpriv_thread);
}
END_TEST

/*
\begin{test}{MUTEX0301}
  \TestDescription{Verify unprivileged thread can not delete mutex}
  \TestFunctionalityTested{\Func{L4\_DeleteMutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Create unprivileged thread.
        \item Unprivileged thread call \Func{L4\_DeleteMutex} with the created mutex.
        \item Checks \Func{L4\_DeleteMutex} returns an error.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0301)
{
    L4_ThreadId_t unpriv_thread;
    L4_MsgTag_t tag;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    unpriv_thread = createThreadInSpace(L4_nilspace, create_delete_mutex_thread);
    L4_Set_Label(&tag, 0xd);
    L4_Set_MsgTag(tag);
    L4_Call(unpriv_thread);

    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
    deleteThread(unpriv_thread);
}
END_TEST

/*
\begin{test}{MUTEX0302}
  \TestDescription{Verify unprivileged thread can acquire a mutex}
  \TestFunctionalityTested{\Func{L4\_Lock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Create unprivileged thread.
        \item Unprivileged thread call \Func{L4\_Lock} with the created mutex.
        \item Checks \Func{L4\_Lock} returns successfully.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0302)
{
    L4_ThreadId_t unpriv_thread;
    L4_MsgTag_t tag;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    unpriv_thread = createThreadInSpace(L4_nilspace, lock_mutex_thread);
    L4_Set_Label(&tag, 0xa);
    L4_Set_MsgTag(tag);
    L4_Call(unpriv_thread);

    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
    deleteThread(unpriv_thread);
}
END_TEST


/* Lock and unlock mutex ----------------------------------------------------*/

/*
\begin{test}{MUTEX0400}
  \TestDescription{Lock mutex}
  \TestFunctionalityTested{\Func{L4\_Lock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_Lock} and check it returns successfully.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0400)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    L4_Unlock(m);
}
END_TEST

/*
\begin{test}{MUTEX0401}
  \TestDescription{Unlock mutex}
  \TestFunctionalityTested{\Func{L4\_Unlock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_Lock} and check it returns successfully.
        \item Call \Func{L4\_Unlock} and check it returns successfully.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0401)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
}
END_TEST

/*
\begin{test}{MUTEX0402}
  \TestDescription{Verify L4_TryLock on a locked mutex}
  \TestFunctionalityTested{\Func{L4\_TryLock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Call \Func{L4\_CreateMutex} and check it returns successfully.
        \item Call \Func{L4\_Lock} and check it returns successfully.
        \item Create a thread.
        \item Thread calls L4\_TryLock on the held mutex.
        \item Check it returns an error.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0402)
{
    L4_ThreadId_t try_lock_thread;
    L4_MsgTag_t tag;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    try_lock_thread = createThread(lock_mutex_thread);
    L4_Set_Label(&tag, 0xb);
    L4_Set_MsgTag(tag);
    L4_Call(try_lock_thread);

    L4_Unlock(m);
    L4_DeleteMutex(m);
}
END_TEST

/* Multiple Threads using Mutex ------------------------------------------*/

static L4_ThreadId_t mutex0403_lock_holder;

static void mutex0403_worker_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    int i;
    for (i = 0; i < 100; i++) {
        L4_Lock(m);
        mutex0403_lock_holder = L4_Myself();
        L4_Yield();
        fail_unless(mutex0403_lock_holder.raw == L4_Myself().raw,
                "Protected counter changed while we held the lock.");
        L4_Unlock(m);
        L4_Yield();
    }
    L4_Send(main_thread);
    L4_WaitForever();
}

/*
\begin{test}{MUTEX0403}
  \TestDescription{Verify that multiple threads are unable to acquire the
    same mutex at the same time.}
  \TestFunctionalityTested{\Func{L4\_Mutex}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item The master thread creates a mutex.
        \item Two threads simulataneously attempt to acquire the mutex, perform
              some work, and then release the mutex.
        \item Check to ensure that no to threads hold the mutex at the same
              time.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0403)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t a, b;
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed.");

    /* Create worker threads. */
    a = createThread(mutex0403_worker_thread);
    b = createThread(mutex0403_worker_thread);
    L4_Set_Timeslice(a, 1);
    L4_Set_Timeslice(b, 1);

    /* Wait for threads to finish. */
    L4_Receive(a);
    L4_Receive(b);

    /* Clean up. */
    deleteThread(a);
    deleteThread(b);
    L4_DeleteMutex(m);
}
END_TEST

static void
mutex0405_child_thread(void)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    L4_Send(main_thread);
    L4_WaitForever();
}

/*
\begin{test}{MUTEX0405}
  \TestDescription{Mutex release on thread delete.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Create a second thread that locks that mutex.
        \item Delete that thread.
        \item Ensure that the mutex can now be locked by a different thread.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0405)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t child;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Get our child to lock the mutex. */
    child = createThread(mutex0405_child_thread);
    L4_Receive(child);

    /* Delete our child. */
    deleteThread(child);

    /* Ensure we can still acquire/release the lock. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST

static void mutex0406_child_thread(void)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    res = L4_Lock(m);
    fail("Unexpectantly acquired lock\n");
    L4_WaitForever();
}

/*
\begin{test}{MUTEX0406}
  \TestDescription{Delete thread waiting on mutex.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Lock the created mutex.
        \item Create a second thread that attempts to lock the mutex.
        \item Delete that thread.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0406)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t child;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Lock the mutex. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");

    /* Get our child to lock the mutex. */
    child = createThread(mutex0406_child_thread);
    L4_Set_Priority(child, 255);
    L4_ThreadSwitch(child);

    /* Delete our child. */
    deleteThread(child);

    /* Ensure we can still acquire/release the lock. */
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST

/*
\begin{test}{MUTEX0407}
  \TestDescription{Delete thread about to acquire a mutex.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Lock the created mutex.
        \item Create a second thread that attempts to lock the mutex.
        \item Release the mutex.
        \item Delete that thread.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0407)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t child;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Lock the mutex. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");

    /* Get our child to lock the mutex. */
    child = createThread(mutex0406_child_thread);
    L4_Set_Priority(child, 255);
    L4_ThreadSwitch(child);

    /* Restore the priority to below us. */
    L4_Set_Priority(child, 1);

    /* Release the mutex. */
    res = L4_Unlock(m);

    /* Delete our child. */
    deleteThread(child);

    /* Ensure we can still acquire/release the lock. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST

/* L4_Mutex Error codes (Lock, unlock) ------------------------------------------*/

/*
\begin{test}{MUTEX0500}
  \TestDescription{Verify a non created mutex can not be locked}
  \TestFunctionalityTested{\Func{L4\_Lock}}
  \TestImplementationProcess{
    Call \Func{L4\_Lock} and check it returns an error.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0500)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_Lock(m);
    fail_unless(res == 0, "L4_Lock() did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidMutex, "Wrong error code");
}
END_TEST

/*
\begin{test}{MUTEX0501}
  \TestDescription{Verify a non locked mutex can not be unlocked}
  \TestFunctionalityTested{\Func{L4\_Unlock}}
  \TestImplementationProcess{
    Call \Func{L4\_Unlock} and check it returns an error.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0501)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_Unlock(m);
    fail_unless(res == 0, "L4_Unlock() did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrMutexBusy, "Wrong error code");

    L4_DeleteMutex(m);
}
END_TEST

/* L4_Mutex Unwinding ------------------------------------------*/

static int mutex0600_progress;
static void mutex0600_child_thread(void)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    mutex0600_progress = 1;
    res = L4_Lock(m);
    mutex0600_progress = 2;
    fail_unless(res == 0, "L4_Lock() did not fail");
    L4_WaitForever();
}

/*
\begin{test}{MUTEX0600}
  \TestDescription{Unwind thread waiting on a mutex.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Lock the created mutex.
        \item Create a second thread that attempts to lock the mutex.
        \item Abort the mutex operation on that thread.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0600)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t child;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Lock the mutex. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");

    /* Get our child to lock the mutex. */
    child = createThread(mutex0600_child_thread);
    L4_Set_Priority(child, 255);
    fail_unless(mutex0600_progress == 1, "Child thread did not start.");

    /* Unwind the mutex. */
    L4_AbortOperation(child);

    /* Ensure that the child has made progress. */
    fail_unless(mutex0600_progress == 2, "Child thread did not continue.");
    L4_ThreadSwitch(child);

    /* Ensure we can still acquire/release the lock. */
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");
    res = L4_DeleteMutex(m);
    fail_unless(res == 1, "L4_DeleteMutex() failed");
}
END_TEST

/* L4_Mutex Priority Tests ------------------------------------------*/

#define NUM_CHILDREN 3

static void mutex0700_child_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);

    /* Acquire the mutex. */
    L4_Lock(m);

    /* Inform master thread we have the lock. */
    L4_Send(main_thread);

    /* Release the lock for somebody else to have. */
    L4_Unlock(m);

    L4_WaitForever();
}

/*
\begin{test}{MUTEX0700}
  \TestDescription{Ensure the order that threads acquire the mutex observes priorities.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_Unlock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create and lock a mutex.
        \item Create 3 threads, each which attempt to lock that mutex.
        \item Ensure that when the mutex is unlocked, the highest the acquires
              the lock.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0700)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t children[NUM_CHILDREN];
    L4_ThreadId_t from_thread;
    int i;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Lock the mutex. */
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");

    /* Create our contesting children. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        L4_ThreadId_t child;
        child = children[i] = createThread(mutex0700_child_thread);

        /* Let our child run until they hit the lock. */
        L4_Set_Priority(child, 255);
        L4_ThreadSwitch(child);

        /* Give the child an low priority. */
        L4_Set_Priority(child, NUM_CHILDREN - i + 1);
    }

    /* Allow children to access the lock. */
    L4_Unlock(m);

    /* Ensure children get mutex in priority order. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        L4_Wait(&from_thread);
        fail_unless(from_thread.raw == lookupReplyHandle(children[i]).raw,
                "Threads not released in priority order.");
    }

    /* Delete our children. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        deleteThread(children[i]);
    }

    L4_DeleteMutex(m);
}
END_TEST

static void mutex0705_low_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);

    /* Acquire the mutex. */
    L4_Lock(m);

    /* Inform master thread we have the lock. */
    L4_Send(main_thread);

    /* Release the lock for somebody else to have. */
    L4_Unlock(m);

    /* Ensure that we don't run ever again. */
    fail("Low priority thread should not run.");

    L4_WaitForever();
}

static void mutex0705_high_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);

    /* Acquire the mutex. */
    L4_Lock(m);
    L4_Unlock(m);

    /* Inform master thread we have the lock. */
    L4_Send(main_thread);

    L4_WaitForever();
}

/*
\begin{test}{MUTEX0705}
  \TestDescription{Ensure that low priority thread is preempted when it
          releases a lock desired by a high-priority thread.}
  \TestFunctionalityTested{\Func{L4\_Lock} and \Func{L4\_Unlock}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create a mutex.
        \item Create a low-priority thread and a high-priority thread.
        \item Allow low-priority thread to acquire mutex.
        \item Low-priority thread releases the mutex.
        \item Ensure that the high priority thread preempts the low priority
              thread.
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEX0705)
{
    L4_Word_t res;
    L4_MutexId_t m = L4_MutexId(0);
    L4_ThreadId_t high_child;
    L4_ThreadId_t low_child;

    /* Create a mutex. */
    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    /* Create low child, and let the acquire the mutex. */
    low_child = createThread(mutex0705_low_thread);
    L4_Set_Priority(low_child, 1);
    L4_Receive(low_child);

    /* Create high prio thread. */
    high_child = createThread(mutex0705_high_thread);
    L4_Set_Priority(high_child, 2);

    /* High priority thread should now preempt the low priority thread,
     * and return control back to us. */
    L4_Receive(high_child);

    L4_DeleteMutex(m);
    deleteThread(low_child);
    deleteThread(high_child);
}
END_TEST


extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    int i;

    initThreads(0);
    main_thread = test_tid;

    /* Delete any created mutex. */
    for (i = 0; i < kernel_max_mutexes; i++) {
        L4_DeleteMutex(L4_MutexId(i));
    }
}

static void test_teardown(void)
{
}

TCase *
make_mutex_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Mutex");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, MUTEX0100);
    tcase_add_test(tc, MUTEX0101);
    tcase_add_test(tc, MUTEX0102);
    tcase_add_test(tc, MUTEX0103);
    tcase_add_test(tc, MUTEX0200);
    tcase_add_test(tc, MUTEX0201);
    tcase_add_test(tc, MUTEX0202);
    tcase_add_test(tc, MUTEX0203);
    tcase_add_test(tc, MUTEX0300);
    tcase_add_test(tc, MUTEX0301);
    tcase_add_test(tc, MUTEX0302);
    tcase_add_test(tc, MUTEX0400);
    tcase_add_test(tc, MUTEX0401);
    tcase_add_test(tc, MUTEX0402);
    tcase_add_test(tc, MUTEX0403);
    tcase_add_test(tc, MUTEX0405);
    tcase_add_test(tc, MUTEX0406);
    tcase_add_test(tc, MUTEX0407);
    tcase_add_test(tc, MUTEX0500);
    tcase_add_test(tc, MUTEX0501);
    tcase_add_test(tc, MUTEX0600);
    tcase_add_test(tc, MUTEX0700);
    tcase_add_test(tc, MUTEX0705);

    return tc;
}
