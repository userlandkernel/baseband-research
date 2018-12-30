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
#include <l4e/map.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static L4_ThreadId_t ex_tid;
static L4_ThreadId_t handler_tid;
static L4_ThreadId_t main_tid;

#if defined(__GNUC__)
/* philipo: this has been disable for RVCT for the moment. */
static L4_Word_t saved_sp;
static L4_Word_t saved_cpsr;

extern char illegal1[];

static void simple_dummy_thread(void)
{
    L4_MsgTag_t tag;

    /* Wait for handler to signal test start */
    tag = L4_Receive(handler_tid);
    fail_unless(getTagE(tag) == 0, "Error returned");

    /* Illegal instruction in user priviledge */
#if defined(__GNUC__)
    __asm__ __volatile__ (
                          "     str     sp, %0                  \n"
                          "     mrs     r0, cpsr                \n"
                          "     str     r0, %1                  \n"
                          "illegal1:                            \n"
                          "     mcr     p7, 0, r0, c0, c0, 0    \n"
                          :: "m" (saved_sp), "m" (saved_cpsr)
                          : "r0"
                         );
#else
    __asm {
        str     __current_sp(), [saved_sp];
        mrs     r0, cpsr                ;
        str     r0, [saved_cpsr]        ;
illegal1:
        mcr     p7, 0, r0, c0, c0, 0    ;
    };
#endif

    L4_Send(handler_tid);

    /* Wait forever */
    L4_Receive(handler_tid);
}
#endif

/* ARM */
#define UNDEF_IP                0
#define UNDEF_SP                1
#define UNDEF_FLAGS             2
#define UNDEF_EXCEPTNO          3
#define UNDEF_ERRORCODE         4

static void handler_thread(void)
{
    L4_MsgTag_t tag = L4_Niltag;
    L4_MsgTag_t tag_expected = L4_Niltag;
    L4_Msg_t msg;

    L4_Set_SendBlock(&tag);
    L4_MsgClear(&msg);

    L4_Set_MsgMsgTag(&msg, tag);
    L4_MsgLoad(&msg);

    tag = L4_Send(ex_tid);

    /* Wait for handler to signal test start */
    tag = L4_Receive(ex_tid);
    fail_unless(getTagE(tag) == 0, "Error returned");

    L4_MsgStore(tag, &msg);

    tag_expected.X.u = 5;
    tag_expected.X.flags = 0;
    tag_expected.X.label = -5 << 4;
    fail_unless(tag.raw == tag_expected.raw, "incorrect tag");

#if 0
    printf("tag: u = %d, flags = %x, label = %x\n", tag.X.u, tag.X.flags, tag.X.label);
    printf("%p\n", (void*)L4_MsgWord(&msg, UNDEF_IP));
    printf("%p\n", (void*)L4_MsgWord(&msg, UNDEF_SP));
    printf("%p\n", (void*)L4_MsgWord(&msg, UNDEF_FLAGS));
    printf("%p\n", (void*)L4_MsgWord(&msg, UNDEF_EXCEPTNO));
    printf("%p\n", (void*)L4_MsgWord(&msg, UNDEF_ERRORCODE));
#endif

#if defined(__GNUC__)
    fail_unless((void*)&illegal1 == (void*)L4_MsgWord(&msg, UNDEF_IP), "incorrect faulting IP");
    fail_unless(saved_sp == L4_MsgWord(&msg, UNDEF_SP), "incorrect SP");
    fail_unless(saved_cpsr == L4_MsgWord(&msg, UNDEF_FLAGS), "incorrect FLAGS");
    fail_unless(1 == L4_MsgWord(&msg, UNDEF_EXCEPTNO), "incorrect ERRORNUM");
    fail_unless(*(L4_Word_t*)&illegal1 == L4_MsgWord(&msg, UNDEF_ERRORCODE), "incorrect ERRCODE");

    L4_MsgPutWord(&msg, UNDEF_IP, (L4_Word_t)&illegal1 + 4);
#endif

    L4_MsgLoad(&msg);

    /* reply to exception */
    L4_Reply(ex_tid);

    /* wait for ack */
    L4_Receive(ex_tid);

    /* Signal end of test */
    L4_Reply(main_tid);

    /* Wait forever */
    L4_Receive(main_tid);
}

/*
\begin{test}{ARM\_UNDEF01}
  \TestDescription{Check exception-IPC works for undefined instructions}
  \TestFunctionalityTested{Undefined instruction delivery}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Setup a thread with exception handler.
        \item Exception handler waits for IPC messages.
        \item Test thread generates an undefined instruction exception.
        \item Exception handler verifies message.
        \item Exception handler simulates instruction, restarts test thread.
        \item Test thread verifies instruction was simulated correctly.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In Regression}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
extern L4_ThreadId_t test_tid;

START_TEST(ARM_UNDEF01)
{
    L4_MsgTag_t tag;
    L4_Word_t retval;

    initThreads(0);

    main_tid = test_tid;
#if defined(__GNUC__)
    ex_tid = createThreadInSpace(L4_nilspace, simple_dummy_thread);
#endif
    handler_tid = createThread(handler_thread);

    retval =  L4_ThreadControl(ex_tid, L4_nilspace, L4_nilthread, L4_nilthread,
                               handler_tid, 0, (void*)-1UL);

    tag = L4_Receive(handler_tid);
    fail_unless(getTagE(tag) == 0, "Error returned");
}
END_TEST

/*
\begin{test}{ARM\_FASS01}
  \TestDescription{Check cpd_sync works}
  \TestFunctionalityTested{Fass cpd syncing with pagetable updates.  If a 1Mb section is mapped into an active domain the CPD is meant to be updated.}
  \TestImplementationProcess{
    \begin{enumerate}
	\item Find two bytes 1Mb apart in physical memory that are different from each other
	\item Map the first byte into the test area with a 1Mb mapping
	\item Read the byte to ensure the mapping is in place
	\item Map the second byte into the test area with a 1Mb mapping
	\item Read the byte and check that the value has changed (ie the CPD was actually synced)
	\item Clean up the mapping
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In Regression}
  \TestIsFullyAutomated{Yes}
\end{test}
*/

#define MB (0x100000)
#define KB (0x400)
#define TEST_VBASE (0x30300000)

extern void main (void);

START_TEST(ARM_FASS01)
{
    unsigned char * vaddr;
    unsigned char * paddr[2];
    unsigned char * test_addr[2];
    unsigned char * pbase[2];
    unsigned char values[2];

    initThreads(0);

    vaddr = (unsigned char *)main;
    do {
        vaddr++;
        values[0] = *vaddr;
        values[1] = *(vaddr + MB);
    } while (values[0] == values[1]);

    paddr[0] = (unsigned char *)l4e_get_phys(L4_rootspace,  (uintptr_t)vaddr);
    pbase[0] = (unsigned char *)(((L4_Word_t)paddr[0]) & ~(1*MB-1));
    test_addr[0] = (unsigned char *)(TEST_VBASE + (((L4_Word_t)paddr[0]) & (1*MB-1)));

    paddr[1] = (unsigned char *)l4e_get_phys(L4_rootspace,  (uintptr_t)(vaddr+MB));
    pbase[1] = (unsigned char *)(((L4_Word_t)paddr[1]) & ~(1*MB-1));
    test_addr[1] = (unsigned char *)(TEST_VBASE + (((L4_Word_t)paddr[1]) & (1*MB-1)));

    //printf("vaddr %p, value[0] %d, value[1] %d\n", vaddr, values[0], values[1]);
    //printf("0: paddr %p, pbase %p, test_addr %p\n", paddr[0], pbase[0], test_addr[0]);
    //printf("1: paddr %p, pbase %p, test_addr %p\n", paddr[1], pbase[1], test_addr[1]);

    /* we must map two different physical 1MB pages.  Currently this works,
     * but may break in the future due to layout changes, etc.
     * if so the test must be rewritten so that it works again
     */
    fail_unless(pbase[0]!=pbase[1], "pbases equal (refactor test)");

    l4e_map(L4_rootspace, TEST_VBASE, TEST_VBASE + MB, (L4_Word_t)pbase[0], L4_Readable, L4_DefaultMemory);
    fail_unless(*test_addr[0] == values[0], "Initial mapping failed\n");
    l4e_map(L4_rootspace, TEST_VBASE, TEST_VBASE + MB, (L4_Word_t)pbase[1], L4_Readable, L4_DefaultMemory);
    fail_unless(*test_addr[1] == values[1], "CPD out of sync\n");

    l4e_unmap(L4_rootspace, TEST_VBASE, TEST_VBASE+MB);
}
END_TEST


/* -------------------------------------------------------------------------*/

TCase * arm_append_exceptions(TCase *tc);

TCase * arm_append_exceptions(TCase *tc)
{
#if defined(__GNUC__)
    tcase_add_test(tc, ARM_UNDEF01);
#else
    (void)ARM_UNDEF01;
#endif
    tcase_add_test(tc, ARM_FASS01);

    return tc;
}
