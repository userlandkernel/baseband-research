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

#include <stddef.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/cache.h>
#include <l4/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <l4test/l4test.h>
#include <l4test/utility.h>

#define NONEXISTANT_TID ((L4_ThreadId_t) (L4_Word_t) 0x13371401)

static L4_ThreadId_t main_thread;

static void unpriv_thread(void)
{
    L4_Word_t control = 0;
    L4_Word_t result;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_ThreadId_t dummy;

    L4_Word_t start = (L4_Word_t)make_cache_tcase;
    L4_Word_t size = 4096;
    size |= ((L4_Word_t)1<<3)<<(L4_BITS_PER_WORD-4);// I bit - invalidate
    size |= ((L4_Word_t)1<<1)<<(L4_BITS_PER_WORD-4);// d bit - data cache

    control |= (1<<6);
    printf("control %"PRI_X_WORD", size %"PRI_X_WORD"\n", control, size);

    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    result = L4_CacheControl(L4_rootspace, control);

    fail_unless(result == 0, "CacheControl from unpriv did not error");
    _fail_unless(L4_ErrorCode() == 1, __FILE__, __LINE__, "ErrorCode=%"PRI_D_WORD", should have been 1", L4_ErrorCode());

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    tag = L4_Ipc(main_thread, main_thread, L4_MsgMsgTag(&msg), &dummy);
    if (L4_IpcFailed(tag)) {
        printf("ERROR CODE %"PRI_D_WORD"\n", L4_ErrorCode());
        assert(!"should not get here");
    }
}


/*
\begin{test}{CACHE0100}
  \TestDescription{Verify CacheControl returns error if value of count in control field too large}
  \TestFunctionalityTested{\Func{control}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new thread
      \item Invoke \Func{CacheControl} on that thread with the count bits greater than the number of available message registers divided by 2; e.g. 37 will always be too large even on 64bit architectures
      \item Check that it returns 0
      \item Check that the \Func{ErrorCode} is set to \Func{EINVALID\_PARAM}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(CACHE0100)
{
    L4_Word_t result;
    L4_Word_t ErrorCode;
    L4_Word_t control = 0;
    control |= (1 << 6);
    control |= 35;
    result = L4_CacheControl(L4_rootspace, control);
    fail_unless(result == 0, "CacheControl did not return error");
    ErrorCode = L4_ErrorCode();
    _fail_unless(ErrorCode == 5, __FILE__, __LINE__, "ErrorCode=%"PRI_D_WORD" should have been 5", ErrorCode);
}
END_TEST

/*
\begin{test}{CACHE0200}
  \TestDescription{Verify CacheControl invalidates from non-privileged thread}
  \TestFunctionalityTested{\Func{control}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new thread
      \item New thread invokes \Func{CacheControl} on roottask, with op bits I and d set, and C unset. This should invalidate data in the cache lines of the roottask without writing to memory
      \item Check that the return value is 0
      \item Check that the \Func{ErrorCode} is set to \Func{EINVALID\_PARAM}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(CACHE0200)
{
    L4_ThreadId_t unpriv;
    L4_MsgTag_t tag;

    unpriv = createThreadInSpace(L4_nilspace, unpriv_thread);
    L4_ThreadSwitch(unpriv);
    tag = L4_Receive(unpriv);
    fail_unless(!L4_IpcFailed(tag), "unpriv thread did not complete");
    deleteThread(unpriv);
}
END_TEST

/*
\begin{test}{CACHE0300}
  \TestDescription{Verify CacheControl returns an error for unused op codes}
  \TestFunctionalityTested{\Func{control}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CacheControl} with op bits set to 0,4,5,8,9..63 
      \item Check that the return value is 0
      \item Check that the \Func{ErrorCode} is set to \Func{EINVALID\_PARAM}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(CACHE0300)
{
    L4_Word_t result;
    L4_Word_t ErrorCode;
    L4_Word_t control = 0;
    int i;
    control |= 1;
    for(i=0; i<63; ++i){
        if(i == 1 || i == 6) //valid values for op bits of control
            continue;
        //control op bits now invalid
        control |= (i << 6);
        result = L4_CacheControl(L4_rootspace, control);
        fail_unless(result == 0, "CacheControl did not return error");
        ErrorCode = L4_ErrorCode();
        _fail_unless(ErrorCode == 5, __FILE__, __LINE__, "ErrorCode=%d should have been 5", ErrorCode);
    }

}
END_TEST

/*
\begin{test}{CACHE0400}
  \TestDescription{Valid op code}
  \TestFunctionalityTested{\Func{control}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CacheControl} on own space with op bits set to 1, 6 
      \item Check that the return value is 1
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(CACHE0400)
{
    L4_Word_t result;
    L4_Word_t control = 0;
    int i;

    //valid values for op bits are 1 or 6
    for(i=1; i!=6; i=6){
        control |= (i << 6);
        result = L4_CacheControl(L4_rootspace, control);
        fail_unless(result == 1, "CacheControl returned error");
    }

}
END_TEST

/*
\begin{test}{CACHE0500}
  \TestDescription{Invalidate own cache line - unprivileged}
  \TestFunctionalityTested{\Func{control}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CacheControl} with MRs s control with op bits I d both set C unset  
      \item Check that the return value is 1
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CACHE0500)
{
    L4_Word_t result;
    L4_Word_t control = 0;

    L4_Word_t start = (L4_Word_t)make_cache_tcase;
    L4_Word_t size = 4096;
    size |= ((L4_Word_t)1<<3)<<(L4_BITS_PER_WORD-4);// I bit - invalidate
    size |= ((L4_Word_t)1<<1)<<(L4_BITS_PER_WORD-4);// d bit - data cache

    control |= (1<<6);

    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    result = L4_CacheControl(L4_rootspace, control);
    fail_unless(result == 1, "CacheControl returned error ");
}
END_TEST


/*
\begin{test}{CACHE0600}
  \TestDescription{Flush own cache range - bad space}
  \TestFunctionalityTested{\Func{FlushRange}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{FlushRange} with a bad space and confirm failure
      \item Check that the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CACHE0600)
{
    L4_Word_t success;

    L4_Word_t       start1 = (L4_Word_t)make_cache_tcase;
    L4_Word_t       start2 = 0x0;
    L4_Word_t       size   = 0x80;

    success = L4_CacheFlushRange(L4_SpaceId(-2UL), start1, start1 + size);
    fail_unless(!success, "Should have failed");

    success = L4_CacheFlushRange(L4_rootspace, start2, start2+ size);
    fail_unless(success, "Bad virtual range should have still succeeded\n");
}
END_TEST


/*
\begin{test}{CACHE0700}
  \TestDescription{Flush own cache range - unprivileged}
  \TestFunctionalityTested{\Func{FlushRange}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{FlushRange} with varing starts and sizes
      \item Check that the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CACHE0700)
{
    L4_Word_t success;

    L4_Word_t       start  = (L4_Word_t)make_cache_tcase;
    L4_Word_t       size   = 0x100;
    const L4_Word_t incr   = 0x1000;
    const L4_Word_t finish = 0x10100;

    while (size < finish) {
        /* explicit space - range flush */
        success = L4_CacheFlushRange(L4_rootspace, start, start + size);
        fail_unless(success, "Couldn't flush valid range");
        /* implicit space - range flush */
        success = L4_CacheFlushRange(L4_nilspace, start, start + size);
        fail_unless(success, "Couldn't flush valid range");

        size += incr;
    }
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
make_cache_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Cache Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, CACHE0100);
    tcase_add_test(tc, CACHE0200);
    tcase_add_test(tc, CACHE0300);
    tcase_add_test(tc, CACHE0400);
    tcase_add_test(tc, CACHE0500);
    tcase_add_test(tc, CACHE0600);
    tcase_add_test(tc, CACHE0700);

    return tc;
}
