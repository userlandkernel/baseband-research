/*
 * Copyright (c) 2005, National ICT Australia
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
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/misc.h>
#include <l4/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static L4_ThreadId_t NONEXISTANT_TID;

static L4_ThreadId_t main_thread;

static L4_Word_t utcb_init_fail = 0;

static void new_dummy_thread(void){
    while(1){
        L4_ThreadSwitch(main_thread);
    }

    assert(!"dummy thread completed!\n");
}

static void test_utcb_init_thread_1st(void){
    ARCH_THREAD_INIT
    L4_Word_t *utcb = get_arch_utcb_base();
    (utcb)[__L4_TCR_PREEMPTED_IP] = 0xDEADBEEF;
    while(1){
        L4_ThreadSwitch(main_thread);
    }

    assert(!"dummy thread completed!\n");
}

static void test_utcb_init_thread_2nd(void){
    ARCH_THREAD_INIT
    L4_Word_t *utcb = get_arch_utcb_base();
    if ((utcb)[__L4_TCR_PREEMPTED_IP] != 0)
        utcb_init_fail = 1;
    while(1){
        L4_ThreadSwitch(main_thread);
    }

    assert(!"dummy thread completed!\n");
}

static void
calls_threadcontrol(void) {
    L4_Word_t retval;
    L4_Word_t ErrorCode;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;

    tag.raw = TAG_SRBLOCK;
    retval = L4_ThreadControl(main_thread, L4_rootspace,
                              main_thread, main_thread,
                              L4_nilthread, 0, (void *)0);
    fail_unless(retval == 0, "L4_ThreadControl failed to return error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrNoPrivilege, "ErrorCode not set to 1");

    L4_Ipc(main_thread, main_thread, tag, &from);
}

#if 0
/*
\begin{test}{THREAD0100}
  \TestDescription{Create two threads in same address space}
  \TestFunctionalityTested{\Func{ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new thread
    \item Create a second thread in the same address space as the first thread
    \item Check it succeeds
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0100)
{

    L4_ThreadId_t first, second;

    first = createThreadInSpace(L4_nilspace, new_dummy_thread);
    second = createThreadInSpace(first, new_dummy_thread);

    deleteThread(first);
    deleteThread(second);
}
END_TEST
#endif

/*
\begin{test}{THREAD0200}
  \TestDescription{Verify unprivileged threads cannot call ThreadControl}
  \TestFunctionalityTested{\Func{ThreadControl}}
  \TestPostConditions{Unprivileged thread calling \Func{ThreadControl} returns error}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new unprivileged thread  
    \item Make the thread call \Func{L4\_ThreadControl()} with \Func{dest} set to root thread
    \item Check that the return value is 0 and that the \Func{ErrorCode} is set to 1 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0200)
{

    L4_ThreadId_t unpriv;

    unpriv = createThreadInSpace(L4_nilspace, calls_threadcontrol);

    L4_Receive(unpriv);
    deleteThread(unpriv);
}
END_TEST

/*
\begin{test}{THREAD0300}
  \TestDescription{Invalid destination thread (L4\_nilthread)}
  \TestPostConditions{Check error returned for invalid \Func{dest} parameter}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_ThreadControl()} with \Func{dest} = \Func{L4\_nilthread}
    \item Check that the return value is 0 and that the \Func{ErrorCode} is set to \Func{L4\_ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0300)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t dest = L4_nilthread;

    retval = L4_ThreadControl(dest, L4_rootspace, 
                              main_thread, L4_Pager(), 
                              L4_nilthread, 0, (void *)0);
    fail_unless(retval == 0, "L4_ThreadControl failed to return error");
    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "ErrorCode=%d not set to L4_ErrInvalidThread", errcode);
}
END_TEST

/*
\begin{test}{THREAD0301}
  \TestDescription{Invalid destination thread (L4\_anythread)}
  \TestPostConditions{Check error returned for invalid \Func{dest} parameter}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_ThreadControl()} with \Func{dest} = \Func{L4\_anythread}
    \item Check that the return value is 0 and that the \Func{ErrorCode} is set to \Func{L4\_ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0301)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t dest = L4_anythread;
    retval = L4_ThreadControl(dest, L4_rootspace,
                              main_thread, L4_Pager(),
                              L4_nilthread, 0, (void *)0);
    fail_unless(retval == 0, "L4_ThreadControl failed to return error");
    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrInvalidThread, 
                 __FILE__, __LINE__,
                 "ErrorCode=%d not set to L4_ErrInvalidThread", errcode);

}
END_TEST

/*
\begin{test}{THREAD0302}
  \TestDescription{Invalid destination thread (L4\_waitnotify)}
  \TestPostConditions{Check error returned for invalid \Func{dest} parameter}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_ThreadControl()} with \Func{dest} = \Func{L4\_waitnotify}
    \item Check that the return value is 0 and that the \Func{ErrorCode} is set to \Func{L4\_ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0302)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t dest = L4_waitnotify;

    retval = L4_ThreadControl(dest, L4_rootspace, 
                              main_thread, L4_Pager(),
                              L4_nilthread, 0, (void *)0);

    fail_unless(retval == 0, "L4_ThreadControl failed to return error");
    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrInvalidThread, 
                 __FILE__, __LINE__, 
                 "ErrorCode=%d not set to L4_ErrInvalidThread", errcode);

}
END_TEST

#if 0
/* thread versions no longer supported */
static void waiting_thread(void){
    L4_ThreadId_t from;
    L4_ThreadId_t me;
    L4_MsgTag_t tag;
    tag.raw = TAG_SRBLOCK;
    me = L4_Myself();

    //blocking receive
    L4_Ipc(main_thread, L4_nilthread, tag, &from);

    //version should now be updated
    fail_unless(L4_Myself().raw == (me.raw |NEW_VERSION), "failed to update version bits");

    L4_Ipc(main_thread, main_thread, tag, &from);
}

/*
\begin{test}{THREAD0400}
  \TestDescription{Updating thread version bits}
  \TestPostConditions{Update the version number of existing thread ID}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new thread in a new address space
    \item Cause threads to wait on each other
    \item Call \Func{L4\_ThreadControl()} on new thread with a different version number
    \item Check that the version number is updated to the new version number
    \item Check that outstanding IPC operations are aborted
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0400)
{
    L4_Word_t retval;
    L4_ThreadId_t waiting;
    L4_ThreadId_t waiting_new_vers;

    waiting = createThreadInSpace(L4_nilspace, (void (*)(void))waiting_thread);
    waitReceiving(waiting);
    waiting_new_vers.raw = waiting.raw | NEW_VERSION;
    retval = L4_ThreadControl(waiting_new_vers, L4_nilspace, L4_Myself(), 
                              L4_Pager(), L4_nilthread, 0, (void*)-1);

    fail_unless(retval == 1, "L4_ThreadControl returned error");
    waitSending(waiting_new_vers);

    fail_unless(checkSending(waiting_new_vers), "thread did not cancel outstanding IPC");

    update_thread_id(waiting, waiting_new_vers);
    deleteThread(waiting_new_vers);
}
END_TEST
#endif

/*
\begin{test}{THREAD0500}
  \TestDescription{Create thread in invalid space}
  \TestPostConditions{\Func{ThreadControl} fails when creating a thread with \Func{SpaceSpecifier} set to \Func{L4\_nilspace}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_ThreadControl()} with a \Func{dest} that does not exist and the \Func{SpaceSpecifier}
                  set to \Func{L4\_nilthread}
    \item Check that the return value is 0
    \item Check that the \Func{ErrorCode} is \Func{L4\_ErrInvalidSpace}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0500)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t new_thread = NONEXISTANT_TID;

    retval = L4_ThreadControl(new_thread, L4_nilspace, main_thread, L4_Pager(), L4_nilthread, 0, (void*)0);
    fail_unless(retval == 0, "L4_ThreadControl did not return error");

    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrInvalidThread, 
                 __FILE__, __LINE__, 
                 "L4_ErrorCode=%d not set to L4_ErrInvalidThread", errcode);
}
END_TEST

/*
\begin{test}{THREAD0600}
  \TestDescription{ThreadControl invalid thread}
  \TestPostConditions{\Func{ThreadControl} fails if \Func{dest} is a huge thread ID}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{ThreadControl} with \Func{dest} = \Func{L4\_Pager()}
    \item Check that the return value is 0
    \item Check that the \Func{ErrorCode} is set to \Func{L4\_ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Unimplemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0600)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t new_thread;

    new_thread = L4_GlobalId(0x10000, 1);

    retval = L4_ThreadControl(new_thread, L4_rootspace, 
                              main_thread, L4_Pager(), 
                              L4_nilthread, 0, (void*)-1);

    fail_unless(retval == 0, "L4_ThreadControl did not return error");

    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "L4_ErrorCode=%d not set to L4_ErrInvalidThread", errcode);
}
END_TEST

/*
\begin{test}{THREAD0900}
  \TestDescription{Invalid UTCB}
  \TestPostConditions{\Func{ThreadControl} fails if \Func{utcb} is outside utcb area}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{ThreadControl} with the \Func{utcb} outside utcb area
    \item Check that the return value is 0
    \item Check that the \Func{ErrorCode} is set to \Func{L4\_ErrUtcbArea}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD0900)
{
    L4_Word_t retval;
    L4_Word_t ErrorCode;
    L4_ThreadId_t new_thread = NONEXISTANT_TID;
    L4_Word_t utcb;

    utcb = ((L4_PtrSize_t)L4_GetUtcbBase())+((-100) * L4_GetUtcbSize());
    retval = L4_ThreadControl(new_thread, L4_rootspace, main_thread, L4_Pager(), L4_nilthread, -1, (void*)(L4_PtrSize_t)utcb);
    fail_unless(retval == 0, "L4_ThreadControl did not return error");
    ErrorCode = L4_ErrorCode();
    _fail_unless(ErrorCode == L4_ErrUtcbArea, __FILE__, __LINE__, "L4_ErrorCode=%d not set to L4_ErrUtcbArea", ErrorCode);
}
END_TEST

/*
\begin{test}{THREAD1000}
  \TestDescription{Change UTCB}
  \TestPostConditions{\Func{ThreadControl} fails if the utcb is different to the current running thread addressed}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new thread
    \item Call \Func{L4\_ThreadControl} with a different utcb location
    \item Check that the return value is 0
    \item Check that the \Func{ErrorCode} is set to \Func{L4\_ErrUtcbArea}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD1000)
{
    L4_Word_t retval;
    L4_Word_t errcode;
    L4_ThreadId_t new_thread = createThreadInSpace(L4_rootspace, new_dummy_thread);
    L4_Word_t utcb;

    utcb = ((L4_PtrSize_t)L4_GetUtcbBase())+((2) * L4_GetUtcbSize());

    retval = L4_ThreadControl(new_thread, L4_nilspace, 
                              main_thread, L4_Pager(), 
                              L4_nilthread, -1, (void*)(L4_PtrSize_t)utcb);
    fail_unless(retval == 0, "L4_ThreadControl did not return error");

    errcode = L4_ErrorCode();
    _fail_unless(errcode == L4_ErrUtcbArea,
                 __FILE__, __LINE__,
                 "L4_ErrorCode=%d not set to L4_ErrUtcbArea", errcode);

    deleteThread(new_thread);
}
END_TEST


/*
\begin{test}{THREAD1300}
  \TestDescription{Modify thread's pager}
  \TestPostConditions{Thread has new pager}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a thread in a new address space
    \item Call \Func{L4\_ThreadControl} with the pager set to \Func{L4\_Myself()}
    \item Check that the return value is 1
    \item Check that the pager is updated using \Func{L4\_PagerOf()}
    \end{enumerate}
  }
  \TestImplementationStatus{Partially Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(THREAD1300)
{
    L4_Word_t retval;
    L4_ThreadId_t new_thread;

    new_thread = createThreadInSpace(L4_nilspace, new_dummy_thread);

    fail_unless(L4_PagerOf(new_thread).raw != L4_Myself().raw, "started with L4_Myself() as pager");
    retval = L4_ThreadControl(new_thread, L4_nilspace, 
                              L4_nilthread, main_thread, L4_nilthread, 0, (void*) -1);
    _fail_unless(retval == 1, __FILE__, __LINE__, "L4_ThreadControl returned error=%d",L4_ErrorCode());
    // FIXME: check the new_thread's Pager
    fail_unless (L4_PagerOf(new_thread).raw == main_thread.raw, "did not update pager");
    deleteThread(new_thread);
}
END_TEST

/*
\begin{test}{THREAD1400}
  \TestDescription{Delete thread}
  \TestPostConditions{Thread is deleted}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a thread in a new address space
    \item Call \Func{L4\_ThreadControl} with the pager set to \Func{L4\_nilthread}
    \item Check that the return value is 1
    \item Check that the thread is deleted using \Func{L4\_ExchangeRegisters}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(THREAD1400)
{
    L4_Word_t retval;
    L4_ThreadId_t new_thread;
    L4_Word_t control, dummy;
    L4_ThreadId_t dummy_tid;

    new_thread = createThreadInSpace(L4_nilspace, new_dummy_thread);

    //delete it
    retval = L4_ThreadControl(new_thread, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void*) -1);
    _fail_unless(retval == 1, __FILE__, __LINE__, "L4_ThreadControl returned error=%d",L4_ErrorCode());

    fail_unless (L4_ExchangeRegisters(new_thread, L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread, &control, &dummy, &dummy, &dummy, &dummy, &dummy_tid).raw != L4_nilthread.raw, "did not delete thread");
}
END_TEST

/*
\begin{test}{THREAD1405}
  \TestDescription{Ensure halted threads can be deleted.}
  \TestPostConditions{Thread is deleted}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new thread.
    \item Use \Func{L4\_ExchangeRegsiters} to halt the thread.
    \item Delete the thread.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(THREAD1405)
{
    L4_ThreadId_t new_thread;

    /* Create the new thread. */
    new_thread = createThread(new_dummy_thread);

    /* Halt the thread. */
    L4_Stop_Thread(new_thread);

    /* Delete it. */
    deleteThread(new_thread);
}
END_TEST

/*
\begin{test}{THREAD1500}
  \TestDescription{Initialize utcb when recreating thread}
  \TestPostConditions{none}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a thread in root space
    \item Switch to the thread and write to utcb area
    \item Delete the thread
    \item recreate a thread using the same tid in root space
    \item Switch to the thread and checking if utcb area is cleared
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(THREAD1500)
{
    //L4_Word_t old_control = 0;
    int i=5;
    L4_ThreadId_t tid;
    int r;
    L4_Word_t utcb;
    L4_Word_t *stacks; 

    // Make sure we found an unused thread
    assert(i < NTHREADS);
    tid = main_thread;
    tid.global.X.thread_no += (i+16);

#ifndef NO_UTCB_RELOCATE
    utcb = (L4_Word_t) (L4_PtrSize_t)L4_GetUtcbBase() + (i+16)*UTCB_SIZE;
#else
    utcb = -1UL;
#endif

    stacks = malloc(sizeof(L4_Word_t)*STACK_SIZE);
    assert(stacks);

    r = L4_ThreadControl(tid, L4_rootspace, default_thread_handler, default_thread_handler, default_thread_handler, 0, (void *)(L4_PtrSize_t)utcb);

    if(r != 1)
    {
        printf("\nThread Create failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    L4_Start_SpIp(tid, (L4_Word_t)(L4_PtrSize_t)(stacks + STACK_SIZE), (L4_Word_t) (L4_PtrSize_t)test_utcb_init_thread_1st);

    L4_ThreadSwitch(tid);

    //delete the thread.
    r = L4_ThreadControl(tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void*)0);
    if(r != 1)
    {
        printf("\nThread Delet failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }

    //recreate the thread.
    r = L4_ThreadControl(tid, L4_rootspace, default_thread_handler, default_thread_handler, default_thread_handler, 0, (void *)(L4_PtrSize_t)utcb);
    
    if(r != 1)
    {
        printf("\nThread Recreation failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);
    
    L4_Start_SpIp(tid, (L4_Word_t)(L4_PtrSize_t)(stacks + STACK_SIZE), (L4_Word_t) (L4_PtrSize_t)test_utcb_init_thread_2nd);
   
    L4_ThreadSwitch(tid);
    //delete the thread.
    r = L4_ThreadControl(tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void*)0);
    if(r != 1)
    {
        printf("\nThread Delet failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    
    //release resources.
    free(stacks);
    fail_unless(!utcb_init_fail, "Failed to initialize utcb when creating new thread!");
}
END_TEST

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;

    NONEXISTANT_TID = L4_GlobalId(555, 1);
}

static void test_teardown(void)
{
}

TCase *
make_thread_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Thread Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    //tcase_add_test(tc, THREAD0100);
    tcase_add_test(tc, THREAD0200);
    tcase_add_test(tc, THREAD0300);
    tcase_add_test(tc, THREAD0301);
    tcase_add_test(tc, THREAD0302);
    //tcase_add_test(tc, THREAD0400);
    tcase_add_test(tc, THREAD0500);
    tcase_add_test(tc, THREAD0600);
    tcase_add_test(tc, THREAD0900);
    tcase_add_test(tc, THREAD1000);
    tcase_add_test(tc, THREAD1300);
    tcase_add_test(tc, THREAD1400);
    tcase_add_test(tc, THREAD1405);
    tcase_add_test(tc, THREAD1500);

    return tc;
}
