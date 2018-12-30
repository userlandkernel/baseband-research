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
#include <l4/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <l4/kdebug.h>
#include <l4/config.h>
#include <l4e/map.h>

#include <l4test/l4test.h>
#include <l4test/utility.h>

static L4_ThreadId_t main_thread;
static L4_SpaceId_t target_space_1, target_space_2;
static L4_Word_t   utcb_root, utcb_target_1;
#if !defined(NO_UTCB_RELOCATE)
static L4_Word_t utcb_target_2;
#endif

#define free_space(i)   \
    space_bitmap[i/L4_BITS_PER_WORD] &= ~(1ul << (i % L4_BITS_PER_WORD))

extern L4_Word_t space_bitmap[];

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}

static void test_teardown(void)
{

}

#if !defined(NO_UTCB_RELOCATE)
static void get_utcb_target_thread(void)
{
    utcb_target_1 = (L4_Word_t)L4_GetUtcbBase(); //Can't use get_arch_utcb_base()
    L4_Call(main_thread);
    //do nothing.
    while(1);
}
#endif

static void get_utcb_root_thread(void)
{
    utcb_root = (L4_Word_t)L4_GetUtcbBase(); //Can't use get_arch_utcb_base()
    //do nothing.
    while(1)
        L4_Call(main_thread);
}

/*
\begin{test}{SPACESWITCH0010}
  \TestDescription{Test switch space by privileged thread}
  \TestFunctionalityTested{\Func{L4\_SpaceSwitch}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create test thread in root space.
      \item Create new space.
      \item In main\_thread switch test thread to new space.
      \item Check new space is not empty.
      \item In main_thread switch test thread back to root space.
      \item Check new space is empty.
      \item Delete new space and test thread.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SPACESWITCH0010)
{
    L4_ThreadId_t switch_tid;
#if !defined(NO_UTCB_RELOCATE)
    L4_ThreadId_t dummy_tid;
#endif
    int r;

    switch_tid = createThread(get_utcb_root_thread);
    
    L4_Receive(switch_tid);

    //create a new space.
    target_space_1 = createSpace();

#if defined(NO_UTCB_RELOCATE)
    //switch switch_tid to new space.
    r = L4_SpaceSwitch(switch_tid, target_space_1, (void *)(L4_PtrSize_t)-1UL);
#else
    //try to get utcb area of target address space
    dummy_tid = createThreadInSpace(target_space_1, get_utcb_target_thread);
    L4_Receive(dummy_tid);
    deleteThread(dummy_tid);
    r = L4_SpaceSwitch(switch_tid, target_space_1, (void *)(L4_PtrSize_t)utcb_target_1);
#endif   
    //switch switch_tid to new space.
    if (r != 1) {
        printf("\nSpace switch failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);
    //try to delete new space, should fail.
    r = L4_SpaceControl(target_space_1, L4_SpaceCtrl_delete, L4_rootclist, L4_Nilpage, 0, NULL);
    if ((r == 1) || ((r != 1) && (L4_ErrorCode() != 9 /*ESPACE_NOT_EMPTY*/))) {
        printf("\nFailed to switch thread 0x%lx to space 0x%lx\n", switch_tid.raw, target_space_1.raw);
    }
    assert((r != 1) && (L4_ErrorCode() == 9 /*ESPACE_NOT_EMPTY*/));

    //check sender space works in fastpath
    updateSpace(switch_tid, target_space_1);
    L4_Call(switch_tid);
    assert(__L4_TCR_SenderSpace() == target_space_1.raw);

    //switch switch_tid back to root space.
#if defined(NO_UTCB_RELOCATE)
    r = L4_SpaceSwitch(switch_tid, L4_rootspace, (void *)(L4_PtrSize_t)-1UL);
#else
    r = L4_SpaceSwitch(switch_tid, L4_rootspace, (void *)(L4_PtrSize_t)utcb_root);
#endif
    if (r != 1) {
        printf("\nSpace switch failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    //try again to delete new space, this time should succeed.
    r = L4_SpaceControl(target_space_1, L4_SpaceCtrl_delete, L4_rootclist, L4_Nilpage, 0, NULL);
    if (r != 1) {
        printf("\nFailed to switch thread 0x%lx back to root space\n", switch_tid.raw);
    }
    assert(r == 1);

    free_space(target_space_1.raw);

    deleteThread(switch_tid);
}
END_TEST


static L4_ThreadId_t switch_tid, target_tid;

static void test_thread(void)
{
    int r;

    L4_Call(main_thread);

    //Try to switch target thread to space 2, should fail.
#if defined(NO_UTCB_RELOCATE)
    //switch target_tid to space 2.
    r = L4_SpaceSwitch(target_tid, target_space_2, (void *)(L4_PtrSize_t)-1UL);
#else
    //switch target_tid to space 2.
    r = L4_SpaceSwitch(target_tid, target_space_2, (void *)(L4_PtrSize_t)utcb_target_2);
#endif
    if ((r == 1) || ((r != 1) && (L4_ErrorCode() != 1) /*ENO_PRIVILEGE*/))
    {
        printf("\nSpace switch check failed, should not allow switch!\n");
    }
    assert((r != 1) && (L4_ErrorCode() == 1 /*ENO_PRIVILEGE*/));

    //Switch back to main_thread to add capability.
    L4_Call(main_thread);

    //At this point, target thread is about to switched to space 2,
    //need to update this information in threads[], otherwise,
    //when later ctxt switch to target thread, handler will send
    //fpage to wrong space.
    updateSpace(target_tid, target_space_2);

    //Try again, this time should succeed.
#if defined(NO_UTCB_RELOCATE)
        //switch target_tid to space 2.
        r = L4_SpaceSwitch(target_tid, target_space_2, (void *)(L4_PtrSize_t)-1UL);
#else
        //switch target_tid to space 2.
        r = L4_SpaceSwitch(target_tid, target_space_2, (void *)(L4_PtrSize_t)utcb_target_2);
#endif
    if (r != 1)
    {
        printf("\nSpace switch failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    //Switch back to main_thread to check space 2 is not empty.
    L4_Call(main_thread);

    //Try to switch target thread back to space 1, should fail.
#if defined(NO_UTCB_RELOCATE)
        r = L4_SpaceSwitch(target_tid, target_space_1, (void *)(L4_PtrSize_t)-1UL);
#else
        r = L4_SpaceSwitch(target_tid, target_space_1, (void *)(L4_PtrSize_t)utcb_target_1);
#endif
    if ((r == 1) || ((r != 1) && (L4_ErrorCode() != 1) /*ENO_PRIVILEGE*/))
    {
        printf("\nSpace switch check failed, should not allow switch!\n");
    }
    assert((r != 1) && (L4_ErrorCode() == 1 /*ENO_PRIVILEGE*/));

    //Switch back to main_thread to let target thread try itself.
    L4_Call(main_thread);
}

static void target_thread(void)
{
    int r;
    utcb_target_1 = (L4_Word_t)L4_GetUtcbBase(); //Can't use get_arch_utcb_base()

    L4_Call(main_thread);

    //now we update threads[], this should be done before
    //actually switched back to space 1, since there could be
    //pagefault when doing space switch.
    //we set it to indicate we are about to spaceswitch
    updateSpace(target_tid, L4_nilspace);

    //try to switch back itself to space 1.
#if defined(NO_UTCB_RELOCATE)
    r = L4_SpaceSwitch(target_tid, target_space_1, (void *)(L4_PtrSize_t)-1UL);
#else
    r = L4_SpaceSwitch(target_tid, target_space_1, (void *)(L4_PtrSize_t)utcb_target_1);
#endif
    // now we update to the new space
    updateSpace(target_tid, target_space_1);

    if (r != 1)
    {
        printf("\nSpace switch failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    //Switch back to main_thread.
    L4_Call(main_thread);
}

#if !defined(NO_UTCB_RELOCATE)
static void get_utcb_target_2_thread(void)
{
    utcb_target_2 = (L4_Word_t)L4_GetUtcbBase(); //Can't use get_arch_utcb_base()
    L4_Call(main_thread);
    //do nothing.
    while(1);
}
#endif

/*
\begin{test}{SPACESWITCH0020}
  \TestDescription{Test space switch by non-privileged thread in the same space}
  \TestFunctionalityTested{\Func{L4\_SpaceSwitch}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create space 1 and create test thread and target thread in this
        space.
      \item Create space 2.
      \item Without security control check space switch by test thread failed.
      \item Using security control and switch target thread to space 2 by test
        thread.
      \item Check space 2 is not empty.
      \item Now check in test thread, can no longer switch target thread back
        to space 1, since they are not in the same space.
      \item Using security control and make target thread switch back to space
        1 by itself.
      \item Check space 2 is empty.
      \item Cleanup.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SPACESWITCH0020)
{
#if !defined(NO_UTCB_RELOCATE)
    L4_ThreadId_t dummy_tid;
#endif
    int r;

    target_space_1 = createSpace();
    target_space_2 = createSpace();

#if !defined(NO_UTCB_RELOCATE)
    //try to get utcb area of space 2.
    dummy_tid = createThreadInSpace(target_space_2, get_utcb_target_2_thread);
    L4_Receive(dummy_tid);
    deleteThread(dummy_tid);
#endif

    switch_tid = createThreadInSpace(target_space_1, test_thread);
    L4_Receive(switch_tid);

    target_tid = createThreadInSpace(target_space_1, target_thread);
    L4_Receive(target_tid);

    //Let test thread try switch target thread to space 2, should fail.
    L4_Call(switch_tid);

    //Set capability to switch from target_space_1 to target_space_2.
    r = L4_AllowSpaceSwitch(target_space_1, target_space_2);
    if (r != 1)
    {
        printf("\nSecurity Control SpcSwitch Domain failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    //try again.
    L4_Call(switch_tid);

    //try to delete space 2, should fail.
    r = L4_SpaceControl(target_space_2, L4_SpaceCtrl_delete, L4_rootclist, L4_Nilpage, 0, NULL);
    if ((r == 1) || ((r != 1) && (L4_ErrorCode() != 9 /*ESPACE_NOT_EMPTY*/)))
    {
        printf("\nFailed to switch thread 0x%lx to space 0x%lx\n", switch_tid.raw, target_space_1.raw);
    }
    assert((r != 1) && (L4_ErrorCode() == 9 /*ESPACE_NOT_EMPTY*/));

    //Set capability to switch from target_space_2 to target_space_1.
    r = L4_AllowSpaceSwitch(target_space_2, target_space_1);
    if (r != 1)
    {
        printf("\nSecurity Control SpcSwitch Domain failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    }
    assert(r == 1);

    //let test thread try switch target thread back, should fail.
    L4_Call(switch_tid);

    //let target thread try switch itself, should succeed.
    L4_Call(target_tid);

    //try again to delete space 2, this time should succeed.
    r = L4_SpaceControl(target_space_2, L4_SpaceCtrl_delete, L4_rootclist, L4_Nilpage, 0, NULL);
    if (r != 1)
    {
        printf("\nFailed to switch thread 0x%lx back to root space\n", switch_tid.raw);
    }
    assert(r == 1);

    free_space(target_space_2.raw);
    //Clean up.
    deleteThread(switch_tid);
    deleteThread(target_tid);
    deleteSpace(target_space_1);
}
END_TEST

/*--------------------------------------------------------------------------*/

#define TEST_SPACE_START    10
#define TEST_STACK_SIZE          0x400
static L4_SpaceId_t space_id[2];
static L4_Word_t test_stack[TEST_STACK_SIZE];
static L4_Word_t test2_stack[TEST_STACK_SIZE];
static L4_Word_t pager_stack[TEST_STACK_SIZE];
static L4_Word_t test_area1[(1UL << 12)/sizeof(L4_Word_t)];
static L4_Word_t test_area2[(1UL << 12)/sizeof(L4_Word_t)];

static L4_ThreadId_t main_tid, pager_tid, thread_tid, test2_tid;

static void thread1(void)
{
    ARCH_THREAD_INIT
    int r;
    L4_Word_t utcb;
    test_area1[0] = 0x12345678;

    L4_Call(main_tid);
    
#if defined(NO_UTCB_RELOCATE)
    utcb = -1UL;
#else
    utcb = (L4_Word_t) ((L4_PtrSize_t) 16 * 1024 * 1024 + L4_GetUtcbSize() * (17));
#endif

    /* space switch self to space 1*/
    r = L4_SpaceSwitch(thread_tid, space_id[1], (void *)utcb);
    if (r == 0)
        printf("Space switch Error: %lx\n", L4_ErrorCode()); 
    assert(r == 1);

    printf("Read from test_area1[0] = 0x%lx\n", test_area1[0]);
    assert(test_area1[0] == 0xdeadbeef);
#if defined(NO_UTCB_RELOCATE)
    utcb = -1UL;
#else
    utcb = (L4_Word_t) ((L4_PtrSize_t) 16 * 1024 * 1024 + L4_GetUtcbSize() * (16));
#endif
    /* space switch self back to space 0*/
    r = L4_SpaceSwitch(thread_tid, space_id[0], (void *)utcb);
    if (r == 0)
        printf("Space switch Error: %lx\n", L4_ErrorCode());
    assert(r == 1);

    printf("Read from test_area1[0] = 0x%lx\n", test_area1[0]);
    assert(test_area1[0] == 0x12345678);
    L4_Call(main_tid);
}

static void thread2(void)
{
    ARCH_THREAD_INIT
    test_area1[0] = 0xdeadbeef;
    while (1)
    {
        L4_Call(main_tid);
    }
}

static void pager(void)
{
    ARCH_THREAD_INIT
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_Word_t label;
    long fault;
    char type;

    L4_Send(main_tid);

    while(1)
    {
        tag = L4_Wait(&from);

        L4_MsgStore(tag, &msg);
        if (!L4_IpcSucceeded(tag))
        {
            printf("Was receiving an IPC which was cancelled?\n");
            continue;
        }
        
        label = L4_MsgLabel(&msg);
        
        tag = L4_Niltag;
        fault = (((long)(label<<16)) >> 20);
        type = label & 0xf;

        switch(fault)
        {
            case -2: 
            {
                L4_Word_t phys, fault_addr, ip;
                L4_Fpage_t fpage;
                L4_SpaceId_t space;
                int r;

                fault_addr = L4_MsgWord(&msg, 0);
                ip = L4_MsgWord(&msg, 1);
//printf(">>>>>>>>>Got pagefault addr: %p ip: %p from:0x%lx, space: 0x%lx\n", (void *)fault_addr, (void *)ip, from.raw, __L4_TCR_SenderSpace());
                phys = l4e_get_phys(L4_rootspace, fault_addr);
                if (phys == ~0UL)
                {
                    printf("invalid pagefault addr: %p ip: %p from:0x%lx\n", (void *)fault_addr, (void *)ip, from.raw);
                    tag = L4_MsgTagAddLabel(tag, (-2UL)>>4<<4); //indicate fail as pagefault
                    L4_Set_MsgTag(tag);
                    L4_Send(L4_Pager());
                    break;
                }
                space.raw = __L4_TCR_SenderSpace();
                if ((space.raw == space_id[0].raw) && (fault_addr == (L4_Word_t)test_area1))
                {
                    phys = l4e_get_phys(L4_rootspace, (L4_Word_t)test_area1);
                }
                else if ((space.raw == space_id[1].raw) && (fault_addr == (L4_Word_t)test_area1))
                {
                    phys = l4e_get_phys(L4_rootspace, (L4_Word_t)test_area2);
                }
                fault_addr &= ~((1UL << 12)-1);
                phys &= ~((1UL << 12)-1);

                fpage = L4_FpageLog2(fault_addr, 12);
                L4_Set_Rights(&fpage, L4_FullyAccessible);
                r = l4e_map_fpage(space, fpage, phys, L4_DefaultMemory);
                assert(r == 1);

                L4_MsgClear(&msg);
                L4_MsgLoad(&msg);
                L4_Reply(from);
                break;
            }
            default:
                assert(0);
                break;
        }
    }
}

/*
\begin{test}{SPACESWITCH0030}
  \TestDescription{Test space switch actually switches address spaces.}
  \TestFunctionalityTested{\Func{L4\_SpaceSwitch}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create space 1 and create test thread1 and target thread in this
        space.
      \item Create space 2 and create test thread2 and target thread in this
        space.
      \item Enable space switching in both directions betwen spaces 1 and 2.
      \item thread 2 writes value2 into test memory location and then spins.
      \item thread 1 writes value1 into test memory location.
      \item thread 1 switches to space 2.
      \item thread 1 ensures read from test memory location returns value2.
      \item thread 1 switches returns to space 1.
      \item thread 1 ensures read from test memory location returns value1.
      \item Cleanup.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(SPACESWITCH0030)
{
    int r,i;
    L4_Fpage_t utcb_area;
    L4_Word_t utcb;
    L4_Word_t dummy;

    main_tid = main_thread;
    pager_tid = L4_GlobalId (L4_ThreadNo (main_tid) + (NTHREADS * 2), 2);
    thread_tid = L4_GlobalId (L4_ThreadNo (main_tid) + (NTHREADS * 2) + 1, 2);
    test2_tid = L4_GlobalId (L4_ThreadNo (main_tid) + (NTHREADS * 2) + 2, 2);

    /* create pager */
#if defined(NO_UTCB_RELOCATE)
    utcb_area = L4_Nilpage;
    utcb = -1UL;
#else
    utcb_area = L4_Fpage (16 * 1024 * 1024, L4_GetUtcbSize() * 1024),
    utcb = (L4_Word_t) (L4_PtrSize_t)L4_GetUtcbBase() + (2)*L4_GetUtcbSize();
#endif
    r = L4_ThreadControl(pager_tid, L4_rootspace, main_tid, main_tid, main_tid, 0, (void *)(L4_PtrSize_t)utcb);
    if (r == 0)
        printf("Thread Create failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    assert(r == 1);
    L4_Schedule(pager_tid, -1, 1, -1, -1, 0, &dummy);
    L4_Set_Priority(pager_tid, 253);
    L4_Start_SpIp(pager_tid, (L4_Word_t)(L4_PtrSize_t)(&pager_stack[TEST_STACK_SIZE]), (L4_Word_t) (L4_PtrSize_t) pager);
    L4_KDB_SetThreadName(pager_tid, "pager");

    L4_Receive(pager_tid);

    /* create test space */
    for (i = 0; i< 2; i++)
    {
        space_id[i] = L4_SpaceId(TEST_SPACE_START + i);
        r = L4_SpaceControl(space_id[i], L4_SpaceCtrl_new, L4_rootclist, utcb_area, 0, &dummy);
        if (r == 0) 
        {
            printf("Create Space Error: %lx\n", L4_ErrorCode());
        }
        assert(r == 1);
    }

    /*Allow space switch */
    r = L4_AllowSpaceSwitch(space_id[0], space_id[1]);
    assert(r == 1);
    r = L4_AllowSpaceSwitch(space_id[1], space_id[0]);
    assert(r == 1);
    /* create test threads */
#if defined(NO_UTCB_RELOCATE)
    utcb = -1UL;
#else
    utcb = (L4_Word_t) ((L4_PtrSize_t) 16 * 1024 * 1024 + L4_GetUtcbSize() * (16));
#endif
    r = L4_ThreadControl(thread_tid, space_id[0], pager_tid, pager_tid, pager_tid, 0, (void *)(L4_PtrSize_t)utcb);
    assert(r == 1);

    L4_Schedule(thread_tid, -1, 1, -1, -1, 0, &dummy);

    L4_Start_SpIp(thread_tid, (L4_Word_t)(L4_PtrSize_t)(&test_stack[TEST_STACK_SIZE]), (L4_Word_t) (L4_PtrSize_t) thread1);

    L4_Receive(thread_tid);

    /* test2 */
#if defined(NO_UTCB_RELOCATE)
    utcb = -1UL;
#else
    utcb = (L4_Word_t) ((L4_PtrSize_t) 16 * 1024 * 1024 + L4_GetUtcbSize() * (16));
#endif
    r = L4_ThreadControl(test2_tid, space_id[1], pager_tid, pager_tid, pager_tid, 0, (void *)(L4_PtrSize_t)utcb);
    assert(r == 1);

    L4_Schedule(test2_tid, -1, 1, -1, -1, 0, &dummy);
    L4_Start_SpIp(test2_tid, (L4_Word_t)(L4_PtrSize_t)(&test2_stack[STACK_SIZE]), (L4_Word_t) (L4_PtrSize_t) thread2);

    L4_Receive(test2_tid);

    /* start space switch test*/

    L4_Call(thread_tid);

    /* Delete test thread and pager */
    /* test_thread: */
    r = L4_ThreadControl(thread_tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
    if (r == 0)
        printf("Thread Delete failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    assert(r == 1);
    /* test2_tid */
    r = L4_ThreadControl(test2_tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
    if (r == 0)
        printf("Thread Delete failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    assert(r == 1);

    /* pager: */
    r = L4_ThreadControl(pager_tid, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
    if (r == 0)
        printf("Thread Delete failed, ErrorCode = %d\n", (int)L4_ErrorCode());
    assert(r == 1);


    /* Delete test space */
    for (i = 0; i < 2; i++)
    {
        r = L4_SpaceControl(space_id[i], L4_SpaceCtrl_delete, L4_rootclist, L4_Nilpage, 0, NULL);
        if (r == 0)
        {
            printf("Delete Space Error: %lx\n",  L4_ErrorCode());
        }
        assert(r == 1);
    }

}
END_TEST


/*--------------------------------------------------------------------------*/
TCase * make_spaceswitch_tcase(void)
{
    TCase *tc;

    initThreads(0);
    
    tc = tcase_create("SpaceSwitch Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, SPACESWITCH0010);
    tcase_add_test(tc, SPACESWITCH0020);
    tcase_add_test(tc, SPACESWITCH0030);

    return tc;
}


