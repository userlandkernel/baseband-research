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

#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/thread.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern L4_ThreadId_t test_tid;
TCase * ia32_ldtcontrol_tcase(TCase *tc);

static L4_ThreadId_t ldt_control_tid;

static void
test_thread(void)
{
    L4_Word_t err;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t dummy_id, rc;
    L4_SegmentDescriptor_t seg;

    L4_Set_SegmentDescriptor(&seg, 0, (L4_Word_t)-1);

    args[0] = 0;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(ldt_control_tid,
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    err = L4_ErrorCode();

    fail_unless(rc.raw == L4_nilthread.raw, 
       "L4_LDTControl did not return expected 0.");
    fail_unless(err == L4_ErrNoPrivilege, "ErrorCode not NoPrivilege");

    L4_Send(ldt_control_tid);
}


/*
 *
 */
START_TEST(IA32_LDTCONTROL01)
{
    L4_Word_t base, index, selector;
    register L4_Word_t base1;
    L4_SegmentDescriptor_t seg;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t dummy_id, rc;

    base = 0xfee1dead;
    index = 0;
    selector = ((index << 3) | 3 | (1 << 2));

    L4_Set_SegmentDescriptor(&seg, (L4_Word_t)&base, (L4_Word_t)-1);
    args[0] = index;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(), 
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    __asm__ __volatile__(
        "push    %%gs              \n"
        "movw    %[seg], %%ax      \n"
        "movw    %%ax, %%gs        \n"
        "mov     %%gs:0, %[base1]  \n"
        "pop     %%gs              \n"
        : [base1] "=r"(base1)
        : [seg] "m"(selector)
    );
    fail_unless(base1 == base, "Incorrect base installed.");
    fail_unless(rc.raw == test_tid.raw, "Unable to set LDT.");
}
END_TEST

/*
 * 
 */
START_TEST(IA32_LDTCONTROL02)
{
    L4_Word_t err;
    register L4_Word_t gs1, gs2;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t dummy_id, rc;
    L4_SegmentDescriptor_t seg;

    __asm__ __volatile__("mov %%gs, %0\n" : "=r"(gs1));

    L4_Set_SegmentDescriptor(&seg, 0, (L4_Word_t)-1);

    args[0] = -1;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(), 
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    err = L4_ErrorCode();
    /*
     * XXX We can't really test for failure, only of failure 
     * to change the %gs register. 
     */
    __asm__ __volatile__("mov %%gs, %0\n" : "=r"(gs2));
    fail_unless(gs1 == gs2, 
        "L4_LDTControl unexpectedly changed gs.");
}
END_TEST


/*
 * 
 */
START_TEST(IA32_LDTCONTROL03)
{
    L4_ThreadId_t local_tid;

    ldt_control_tid = test_tid;

    local_tid = createThreadInSpace(L4_nilspace, test_thread);
    L4_Receive(local_tid);
    deleteThread(local_tid);
}
END_TEST

/*
 *
 */
START_TEST(IA32_LDTCONTROL04)
{
    L4_Word_t err;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t rc, dummy_id;
    L4_SegmentDescriptor_t seg;

    L4_Set_SegmentDescriptor(&seg, 0, (L4_Word_t)-1);

    args[0] = 0;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_nilthread, 
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    err = L4_ErrorCode();

    fail_unless(rc.raw == L4_nilthread.raw, 
        "L4_LDTControl did not return expected 0.");
    fail_unless(err == L4_ErrInvalidThread, "ErrorCode not InvalidThread");
}
END_TEST

START_TEST(IA32_LDTCONTROL05)
{
    L4_Word_t base, selector, index;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t rc, dummy_id;
    L4_SegmentDescriptor_t seg;
    register L4_Word_t val;

    base = 0xfee1dead;
    index = 0;
    selector = ((index << 3) | 3 | (1 << 2));
    val = 0;

    L4_Set_SegmentDescriptor(&seg, (L4_Word_t)&base, (L4_Word_t)-1);
    args[0] = index;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(),
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    fail_unless(rc.raw == test_tid.raw, "Unable to set LDT");

    L4_Set_SegmentDescriptor(&seg, 0, 0);
    args[0] = index;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(),
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    fail_unless(rc.raw == test_tid.raw, "Unable to remove LDT");

    /* We should test that loading %gs with ldt[0] generates an exception ipc */

}
END_TEST

START_TEST(IA32_LDTCONTROL06)
{
    L4_Word_t selector, index;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t rc, dummy_id;
    L4_SegmentDescriptor_t seg;
    register L4_Word_t val;

    index = 0;
    selector = ((index << 3) | 3 | (1 << 2));
    val = 0;

    L4_Set_SegmentDescriptor(&seg, 0, (L4_Word_t)-1);
    seg.x.d.dpl = 0;
    args[0] = index;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(),
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    fail_unless(rc.raw == test_tid.raw, "Unable to set LDT");

    /* We should test that loading %gs with ldt[0] generates an exception ipc */
}
END_TEST

START_TEST(IA32_LDTCONTROL07)
{
    L4_Word_t selector, index;
    L4_Word_t dummy, args[3];
    L4_ThreadId_t rc, dummy_id;
    L4_SegmentDescriptor_t seg;
    register L4_Word_t val;

    index = 0;
    selector = ((index << 3) | 3 | (1 << 2));
    val = 0;

    L4_Set_SegmentDescriptor(&seg, 0, (L4_Word_t)-1);
    seg.x.d.s = 0;
    args[0] = index;
    args[1] = seg.x.raw[0];
    args[2] = seg.x.raw[1];
    L4_LoadMRs(0, 3, args);
    rc = L4_ExchangeRegisters(L4_Myself(),
        L4_ExReg_Tls, 0, 0, 0, 0, L4_nilthread,
        &dummy, &dummy, &dummy, &dummy, &dummy, 
        &dummy_id);
    fail_unless(rc.raw == test_tid.raw, "Unable to set LDT");

    /* We should test that loading %gs with ldt[0] generates an exception ipc */
}
END_TEST

TCase * ia32_ldtcontrol_tcase(TCase *tc)
{
    tcase_add_test(tc, IA32_LDTCONTROL01);
    tcase_add_test(tc, IA32_LDTCONTROL02);
    tcase_add_test(tc, IA32_LDTCONTROL03);
    tcase_add_test(tc, IA32_LDTCONTROL04);
    tcase_add_test(tc, IA32_LDTCONTROL05);
    tcase_add_test(tc, IA32_LDTCONTROL06);
    tcase_add_test(tc, IA32_LDTCONTROL07);

    return tc;
}


