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

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/caps.h>
#include <l4/kdebug.h>

extern L4_ThreadId_t test_tid;
static L4_ThreadId_t main_thread;

static L4_ThreadId_t thrd1, thrd2, thrd3;
static L4_ThreadId_t thrd1_cap, thrd3_cap, main_thread_cap;

#define PAGESIZE 0x1000
ALIGNED(PAGESIZE) static char stack1[PAGESIZE] = {1};  
#define stack_top1 ((L4_Word_t)(stack1 + PAGESIZE))
ALIGNED(PAGESIZE) static char stack2[PAGESIZE] = {1};  
#define stack_top2 ((L4_Word_t)(stack2 + PAGESIZE))
ALIGNED(PAGESIZE) static char stack3[PAGESIZE] = {1};  
#define stack_top3 ((L4_Word_t)(stack3 + PAGESIZE))

static void
test1(void)
{
    L4_MsgTag_t tag;

    /* Get IPC from thread 3 */
    L4_Receive(thrd3_cap);

    /* Alert the main thread that we are done */
    L4_LoadMR(0,0);
    tag = L4_Call(main_thread_cap);
    assert(L4_IpcSucceeded(tag));

    while (1) { }
}

static void
test2(void)
{
    /* Get IPC from thread 3 */
    L4_Receive(thrd3_cap);

    while (1) { }
}

static void
test3(void)
{
    L4_MsgTag_t tag;

    /* Call thread 2. No cap - should fail */
    L4_LoadMR(0, 0);
    tag = L4_Call(thrd2);
    fail_unless(!L4_IpcSucceeded(tag), "IPC succeeded to an invalid cap");

    /* Call thread 1. Valid cap - should pass */
    L4_LoadMR(0, 0);
    tag = L4_Call(thrd1_cap);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed to a valid cap");
    
    while (1) { }
}

static void
create_clist(L4_ClistId_t clist, L4_Word_t size)
{
    L4_Word_t success, ErrorCode;

    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
}

static void
create_address_space(L4_SpaceId_t space, L4_ClistId_t clist, L4_Fpage_t utcb_fpage)
{
    L4_Word_t success, ErrorCode;

#ifdef NO_UTCB_RELOCATE     
    utcb_fpage = L4_Nilpage;
#endif

    success = L4_SpaceControl(space, L4_SpaceCtrl_new, clist, utcb_fpage, 0, NULL);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space1 create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
}

static void
create_thread_in_space(L4_ThreadId_t tid, L4_SpaceId_t space, L4_Word_t utcb)
{
    L4_Word_t success, ErrorCode, dummy;

#ifdef NO_UTCB_RELOCATE
    utcb = -1UL;
#endif

    success = L4_ThreadControl(tid, space, default_thread_handler, default_thread_handler,
                        L4_nilthread, 0, (void *)utcb);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Thread create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

   L4_Schedule(tid, -1, 0x00000001, -1, -1, 0, &dummy);  
}

static void
delete_thread(L4_ThreadId_t tid)
{
    L4_Word_t success, ErrorCode;

    success = L4_ThreadControl(tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Thread delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
}

/*
  \begin{test}{CAPCPX0100}
  \TestDescription{IPC between three threads in two spaces with differing caps}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create two spaces with two different clists
  \item Add IPC caps to the default pager to both clists
  \item Create two threads in space 1
  \item Create one thread in space 2
  \item Add an IPC Cap for main thread to space 1
  \item Add an IPC Cap for thread 1 to space 2
  \item Add an IPC Cap for thread 3 to space 1
  \item Start threads and await IPC from thread 1
  \item Thread 1 and thread 2, both in space 1, L4_Receive on thread 3
  \item Thread 3 attempts to IPC thread 2. Attempt should fail, since no valid cap.
  \item Thread 3 attempts to IPC thread 1. Attempt should pass, since valid cap.
  \item Thread 1 IPCs main thread to alert that we are done
  \item Main thread cleans up, deleting all caps, clists, spaces, and threads.
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPCPX0100)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist1;
    L4_SpaceId_t space1;
    L4_ClistId_t clist2;
    L4_SpaceId_t space2;
    L4_Word_t size;

    main_thread = test_tid;

    size = 0x100;
    clist1 = L4_ClistId(2);
    space1 = L4_SpaceId(8);
    clist2 = L4_ClistId(3);
    space2 = L4_SpaceId(9);

    /* Create clist1 and space1 */
    create_clist(clist1, size);
    create_address_space(space1, clist1, L4_Fpage(0xb10000, 0x1000));

    /* Create clist2 and space2 */
    create_clist(clist2, size);
    create_address_space(space2, clist2, L4_Fpage(0xb10000, 0x1000));

    /* Add caps to the pager to both clists */
    L4_CreateIpcCap(default_thread_handler, L4_rootclist, default_thread_handler, clist1);
    L4_CreateIpcCap(default_thread_handler, L4_rootclist, default_thread_handler, clist2);

    /* Create three threads */
    thrd1 = L4_GlobalId(8,1);
    create_thread_in_space(thrd1, space1, 0xb10000);
    thrd2 = L4_GlobalId(9,1);
    create_thread_in_space(thrd2, space1, 0xb10000 + sizeof(utcb_t));
    thrd3 = L4_GlobalId(10,1);
    create_thread_in_space(thrd3, space2, 0xb10000);

    /* Create a cap from clist1 to the main thread */
    main_thread_cap = L4_GlobalId(17,1);
    success = L4_CreateIpcCap(main_thread, L4_rootclist, main_thread_cap, clist1);
    ErrorCode = L4_ErrorCode(); 
    _fail_unless(success, __FILE__, __LINE__, "L4_CreateIpcCap failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a cap from clist2 to thread 1 */
    thrd1_cap = L4_GlobalId(12,1);
    success = L4_CreateIpcCap(thrd1, L4_rootclist, thrd1_cap, clist2);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "L4_CreateIpcCap failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a cap from clist1 to thread 3 */
    thrd3_cap = L4_GlobalId(10,1);
    success = L4_CreateIpcCap(thrd3, L4_rootclist, thrd3_cap, clist1);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "L4_CreateIpcCap failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Start the threads and wait to hear back from thread 1 */
    L4_Start_SpIp(thrd1, stack_top1, (L4_Word_t)test1);
    L4_Start_SpIp(thrd2, stack_top2, (L4_Word_t)test2);
    L4_Start_SpIp(thrd3, stack_top3, (L4_Word_t)test3);
    L4_Receive(thrd1);

    /* Cleanup */
    delete_thread(thrd1);
    delete_thread(thrd2);
    delete_thread(thrd3);
    L4_SpaceControl(space1, L4_SpaceCtrl_delete, clist1, L4_Nilpage, 0, NULL);
    L4_SpaceControl(space2, L4_SpaceCtrl_delete, clist2, L4_Nilpage, 0, NULL);
    L4_DeleteCap(clist1, default_thread_handler);
    L4_DeleteCap(clist1, main_thread_cap);
    L4_DeleteCap(clist1, thrd3_cap);
    L4_DeleteClist(clist1);
    L4_DeleteCap(clist2, default_thread_handler);
    L4_DeleteCap(clist2, thrd1_cap);
    L4_DeleteClist(clist2);
}
END_TEST

TCase *
make_capcomplex_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Capability Complex Tests");
    
    tcase_add_test(tc, CAPCPX0100);

    return tc;
}
