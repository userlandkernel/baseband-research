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
#include <l4/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern L4_ThreadId_t test_tid;

static L4_ThreadId_t main_thread;
static L4_ThreadId_t handler;
static L4_ThreadId_t irq_thread;
#define NOTIFYBITS_0001 0x44440000UL
#define NOTIFYMASK_0001 0xaaaaffffUL
#define NOTIFYMSG_0001  0xffff1234UL

static void apic0001_receive_any_check_thread(void)
{
    L4_ThreadId_t from = L4_nilthread;
    L4_MsgTag_t tag;
    
    L4_Word_t data = 0;
    while(1)
    {
        L4_Set_NotifyBits(NOTIFYBITS_0001);
        L4_Set_NotifyMask(NOTIFYMASK_0001); 
        L4_Accept(L4_NotifyMsgAcceptor); //allow notify receive.
        tag.raw = TAG_SRBLOCK;
        tag = L4_Ipc(from, L4_anythread, tag, &from);
        if (getTagE(tag))
        {
            printf("handler wait_reply ipc error %lx\n", L4_ErrorCode());
        }
        L4_StoreMR(1, &data);
        /*printf("Get async message = 0x%lx, should be 0x%lx\n", data, (NOTIFYBITS_0001 | NOTIFYMSG_0001) & NOTIFYMASK_0001);
        printf("Notify Bits = 0x%lx, should be 0x%lx\n", L4_Get_NotifyBits(), (NOTIFYBITS_0001 | NOTIFYMSG_0001) & ~NOTIFYMASK_0001);*/
        if (data)
        {
            //get async message
            _fail_unless(data == ((NOTIFYBITS_0001 | NOTIFYMSG_0001) & NOTIFYMASK_0001), __FILE__, __LINE__, "Get async message = 0x%lx which is incorrect, should be 0x%lx", data, (NOTIFYBITS_0001 | NOTIFYMSG_0001) & NOTIFYMASK_0001);
            _fail_unless(L4_Get_NotifyBits() == ((NOTIFYBITS_0001 | NOTIFYMSG_0001) & ~NOTIFYMASK_0001), __FILE__, __LINE__, "Notify Bits = 0x%lx which is incorrect, should be 0x%lx", L4_Get_NotifyBits(), (NOTIFYBITS_0001 | NOTIFYMSG_0001) & ~NOTIFYMASK_0001);
            L4_ThreadSwitch(main_thread);
        }

    }
}

static void apic0001_send_normal_ipc_thread(void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    //L4_Msg_t msg;
    int i;

    for (i = 0; i < 3; i++)
    {
        tag.raw = TAG_SRBLOCK | 0x1;
        L4_LoadMR(1, 0x0);
        //printf("normal ipc send to handler\n");
        tag = L4_Ipc(handler, handler, tag, &from);
       // printf("receive from handler: handler is %lx, from is %lx, tag is %lx\n", handler.raw, from.raw, tag.raw);
        if (getTagE(tag))
        {
            printf("wait_reply normal ipc error %lx\n", L4_ErrorCode());
        }
    }
    while (1)
        L4_ThreadSwitch(main_thread);
}

static void apic0001_send_aipc_thread(void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    
    tag.raw = TAG_SRBLOCK | 0x2001;
    L4_LoadMR(1, NOTIFYMSG_0001);
    tag = L4_Ipc(handler, L4_nilthread, tag, &from);
    if (getTagE(tag))
        {
            printf("Send async ipc error %lx\n", L4_ErrorCode());
        }
    L4_ThreadSwitch(handler);
}
#if defined(PLATFORM_PXA)
#include <l4e/map.h>
#include <l4/kdebug.h>
#define XSCALE_TIMERS_PHYS  0x40A00000
#define XSCALE_OS_TIMER_OSMR2   (*(volatile uintptr_t *)(XSCALE_TIMERS_PHYS + 0x8))
#define XSCALE_OS_TIMER_OSSR    (*(volatile uintptr_t *)(XSCALE_TIMERS_PHYS + 0x14))
#define XSCALE_OS_TIMER_OSCR    (*(volatile uintptr_t *)(XSCALE_TIMERS_PHYS + 0x10))
#define XSCALE_OS_TIMER_OIER    (*(volatile uintptr_t *)(XSCALE_TIMERS_PHYS + 0x1C))
#define IRQ_OSTMR2  28

static void apic0002_receive_any_check_thread(void)
{
    L4_Word_t dummy, data;
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Schedule(L4_Myself(), -1, 1, -1, -1, 0, &dummy);
    L4_Send(main_thread);
    from = L4_nilthread;

    while(1)
    {
        L4_Set_NotifyBits(NOTIFYBITS_0001);
        L4_Set_NotifyMask(NOTIFYMASK_0001);
        L4_Accept(L4_NotifyMsgAcceptor);
        tag.raw = TAG_SRBLOCK;
        //printf("Before receiving irq/msg\n"); 
        tag = L4_Ipc(from, L4_anythread, tag, &from);
        //printf("After receiving from irq/msg\n");
        L4_StoreMR(1, &data);

                
        //Clear possible pending hardware interrupts
        XSCALE_OS_TIMER_OSSR = 0x4;

        //Set match register 2 to be 0x5a000 after now.
        XSCALE_OS_TIMER_OSMR2 = XSCALE_OS_TIMER_OSCR + 0x5a000;
       
        if (from.raw == L4_nilthread.raw)
        {
            //get async message
            _fail_unless(data == ((NOTIFYBITS_0001 | 0x80000000) & NOTIFYMASK_0001), __FILE__, __LINE__, "Get async message = 0x%lx which is incorrect, should be 0x%lx\n", data, (NOTIFYBITS_0001 | 0x80000000) & NOTIFYMASK_0001);
            _fail_unless(L4_Get_NotifyBits() == ((NOTIFYBITS_0001 | 0x80000000) & ~NOTIFYMASK_0001), __FILE__, __LINE__, "Notify Bits = 0x%lx which is incorrect, should be 0x%lx\n", L4_Get_NotifyBits(), (NOTIFYBITS_0001 | 0x80000000) & ~NOTIFYMASK_0001);
            _fail_unless(__L4_TCR_PlatformReserved(0) == IRQ_OSTMR2, __FILE__, __LINE__, "Get IRQ number is incorrect, got %ld, should be %ld\n", __L4_TCR_PlatformReserved(0), IRQ_OSTMR2);
        }
        
        L4_Send(main_thread);
    }
}
#endif

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}

static void test_teardown(void)
{

}

START_TEST(ARM_AIPC_0001)
{
    L4_ThreadId_t aipc_sender;
    irq_thread = L4_nilthread;

    handler = createThread(apic0001_receive_any_check_thread);
    
    waitReceiving(handler);
    assert(checkReceiving(handler));

    //Create a thread that send 3 normal ipc.
    irq_thread = createThread(apic0001_send_normal_ipc_thread);
    
    L4_ThreadSwitch(irq_thread);

    //Create a thread that send a async ipc.
    aipc_sender = createThread(apic0001_send_aipc_thread);

    L4_ThreadSwitch(aipc_sender);

    deleteThread(aipc_sender);
    deleteThread(irq_thread);
    deleteThread(handler);
}
END_TEST

#if defined(PLATFORM_PXA)
START_TEST(ARM_AIPC_0002)
{
    L4_Word_t r;
    L4_ThreadId_t ipc_sender;

    handler = createThread(apic0002_receive_any_check_thread);

    L4_Receive(handler);
    //printf("Setting up timer\n");
    //Map pxa250 memory
    r = l4e_map(L4_rootspace, XSCALE_TIMERS_PHYS, XSCALE_TIMERS_PHYS + 0xfff, XSCALE_TIMERS_PHYS, L4_Readable | L4_Writable, L4_IOMemory);
    assert(r == 1);

    //Clear possible pending hardware interrupts
    XSCALE_OS_TIMER_OSSR = 0x4;

    
    //Set match register 2 to be 0x5a000 after now.
    XSCALE_OS_TIMER_OSMR2 = (XSCALE_OS_TIMER_OSCR + 0x5a000) & ~0UL;

    L4_LoadMR(0, IRQ_OSTMR2);
    r = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(r == 1, "failed security control");

    L4_LoadMR(0, IRQ_OSTMR2);
    r = L4_RegisterInterrupt(handler, 31, 0, 0);
    fail_unless(r == 1, "failed interrupt control");

    //Enable irq timer 0, 2
    XSCALE_OS_TIMER_OIER = 0x5;

    L4_Receive(handler);
    //L4_KDB_Enter("get irq");

    //wait until handler recieve block.
    L4_ThreadSwitch(handler);
    waitReceiving(handler);
    assert(checkReceiving(handler));

    ipc_sender = createThread(apic0001_send_normal_ipc_thread);
    L4_ThreadSwitch(ipc_sender);

    L4_Receive(handler);
    //L4_KDB_Enter("after sending ipc");

    //Disable irq timer 2
    XSCALE_OS_TIMER_OIER = 0x1;

    L4_LoadMR(0, IRQ_OSTMR2);
    r = L4_UnregisterInterrupt(handler, 0, 0);
    fail_unless(r == 1, "failed interrupt control");

    L4_LoadMR(0, IRQ_OSTMR2);
    r = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(r == 1, "failed security control");

    deleteThread(ipc_sender);
    deleteThread(handler);
}
END_TEST

#endif

TCase * arm_aipc_tcase(TCase *tc);

TCase * arm_aipc_tcase(TCase *tc)
{
    initThreads(0);
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    
    tcase_add_test(tc, ARM_AIPC_0001);
#if defined(PLATFORM_PXA)
    tcase_add_test(tc, ARM_AIPC_0002);
#endif
    return tc;
}

