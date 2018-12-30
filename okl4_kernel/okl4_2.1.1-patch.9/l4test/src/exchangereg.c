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
#include <l4/thread.h>
#include <l4/schedule.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#if 0
/* Test User Defined Handle values. */
#define HANDLE_VALUE1 ((L4_Word_t) 0xdeafbeef)
#define HANDLE_VALUE2 ((L4_Word_t) 0xbeefdead)

static L4_ThreadId_t main_thread;
static void* simple_ip_range[2];
static void* progressive_ip_range[3];
static L4_ThreadId_t dummy_pager1;
static L4_ThreadId_t dummy_pager2;

/* A simple thread that always defers to the main thread. */
static void simple_dummy_thread(void)
{
    simple_ip_range[0] = &&start;
    simple_ip_range[1] = &&end;
    L4_Set_UserDefinedHandle (HANDLE_VALUE1);
    L4_Set_PagerOf (L4_Myself (), dummy_pager1);

start:
    /*
     * Just spin while waiting to be killed.  Making syscalls here can put
     * the PC outside the range we're testing for.
     */
    while (1)
        ;
end:

    assert(!"simple dummy thread completed!\n");
}

/*
 * A more complex thread that does a two stage wait for the master
 * thread.  The waiting is done by polling the variables a and b
 * below.
 */
static volatile int a = 0, b = 0;

static void progressive_dummy_thread(void)
{
    progressive_ip_range[0] = &&start;
    progressive_ip_range[1] = &&middle;
    progressive_ip_range[2] = &&end;

    L4_Set_UserDefinedHandle (HANDLE_VALUE1);
    L4_Set_PagerOf (L4_Myself (), dummy_pager1);

start:
    /*
     * Just spin while waiting for 'a' to change.  Making syscalls here can put
     * the PC outside the range we're testing for.
     */
    while (!a)
        ;
middle:
    /* Change values and tell the main thread that we've moved on. */
    L4_Set_UserDefinedHandle (HANDLE_VALUE2);
    L4_Set_PagerOf (L4_Myself (), dummy_pager2);
    b = 1;

    /*
     * Just spin while waiting to be killed.  Making syscalls here can put
     * the PC outside the range we're testing for.
     */
    while (1)
        ;
end:

    assert(!"progressive dummy thread completed!\n");
}

static void single_ip_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, ip;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &ip, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (ip >= (L4_Word_t) simple_ip_range[0] &&
                         ip <= (L4_Word_t) simple_ip_range[1],
                         "Instruction pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) ip, simple_ip_range[0], simple_ip_range[1]);
        }

    deleteThread (thread);
}

START_TEST(exreg0100)
{
    single_ip_fetch (createThread (simple_dummy_thread));
}
END_TEST

START_TEST(exreg0101)
{
    single_ip_fetch (createThreadInSpace (L4_nilspace, simple_dummy_thread));
}
END_TEST
 
static void single_stack_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, sp;
    L4_Word_t* stack_buffer;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    stack_buffer = threadStack (thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &sp, &dummy, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (sp >= (L4_Word_t) stack_buffer &&
                         sp <= (L4_Word_t) (stack_buffer + STACK_SIZE),
                         "Stack pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) sp, stack_buffer, stack_buffer + STACK_SIZE);
        }

    deleteThread (thread);
}

START_TEST(exreg0200)
{
    single_stack_fetch (createThread (simple_dummy_thread));
}
END_TEST

START_TEST(exreg0201)
{
    single_stack_fetch (createThreadInSpace (L4_nilspace, simple_dummy_thread));
}
END_TEST

static void single_handle_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &handle,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (handle == HANDLE_VALUE1,
                         "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                         (long) handle, (long) HANDLE_VALUE1);
        }

    handle = L4_UserDefinedHandleOf (thread);
    fail_unless (handle == HANDLE_VALUE1,
                 "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                 (long) handle, (long) HANDLE_VALUE1);

    deleteThread (thread);
}

START_TEST(exreg0300)
{
    single_handle_fetch (createThread (simple_dummy_thread));
}
END_TEST

START_TEST(exreg0301)
{
    single_handle_fetch (createThreadInSpace (L4_nilspace, simple_dummy_thread));
}
END_TEST

START_TEST(exreg0302)
{
    L4_ThreadId_t ids[5];
    L4_Word_t old_control;
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int i, s, r;

    ids[0] = L4_nilthread;
    ids[1] = L4_anythread;
    ids[2] = L4_anylocalthread;
    ids[3] = L4_waitnotify;
    ids[4] = L4_GlobalId (255, 255);

    for (i = 0; i < 5; i++) {
        for (s = 0; s < 2; s++)
            for (r = 0; r < 2; r++) {
                result = L4_ExchangeRegisters (ids[i], (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                               &old_control, &dummy, &dummy, &dummy, &handle,
                                               &dummy_id);

                fail_unless (L4_IsNilThread (result),
                             "L4_ExchangeRegisters did not fail.");
                fail_unless (L4_ErrorCode () == 2,
                             "Unexpected error code %d.\n", (int) L4_ErrorCode ());
            }
    }
}
END_TEST

static void single_pager_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy;
    L4_ThreadId_t pager;
    L4_ThreadId_t result;
    int s, r;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &dummy,
                                           &pager);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (L4_IsThreadEqual (pager, dummy_pager1),
                         "Unexpected pager value (0x%lx, wanted 0x%lx).", pager, dummy_pager1);
        }

    pager = L4_PagerOf (thread);
    fail_unless (L4_IsThreadEqual (pager, dummy_pager1),
                 "Unexpected pager value.");

    deleteThread (thread);
}

START_TEST(exreg0400)
{
    single_pager_fetch (createThread (simple_dummy_thread));
}
END_TEST

#if 0
/* Threads outside the roottask space can't change their own pager. */
START_TEST(exreg0401)
{
    single_pager_fetch (createThreadInSpace (L4_nilspace, simple_dummy_thread));
}
END_TEST
#endif

static void multiple_ip_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, ip;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    a = b = 0;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &ip, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (ip >= (L4_Word_t) progressive_ip_range[0] &&
                         ip <= (L4_Word_t) progressive_ip_range[1],
                         "Instruction pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) ip, progressive_ip_range[0], progressive_ip_range[1]);
        }

    a = 1;
    L4_ThreadSwitch(thread);

    while (!b);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &ip, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (ip >= (L4_Word_t) progressive_ip_range[1] &&
                         ip <= (L4_Word_t) progressive_ip_range[2],
                         "Instruction pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) ip, progressive_ip_range[1], progressive_ip_range[2]);
        }

    deleteThread (thread);
}

START_TEST(exreg0500)
{
    multiple_ip_fetch (createThread (progressive_dummy_thread));

}
END_TEST

START_TEST(exreg0501)
{
    multiple_ip_fetch (createThreadInSpace (L4_nilspace, progressive_dummy_thread));
}
END_TEST

static void multiple_stack_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, sp;
    L4_Word_t* stack_buffer;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    a = b = 0;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    stack_buffer = threadStack (thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &sp, &dummy, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (sp >= (L4_Word_t) stack_buffer &&
                         sp <= (L4_Word_t) (stack_buffer + STACK_SIZE),
                         "Stack pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) sp, stack_buffer, stack_buffer + STACK_SIZE);
        }

    a = 1;
    L4_ThreadSwitch(thread);

    while (!b);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &sp, &dummy, &dummy, &dummy,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (sp >= (L4_Word_t) stack_buffer &&
                         sp <= (L4_Word_t) (stack_buffer + STACK_SIZE),
                         "Stack pointer (0x%lx) is outside the expected range (%p -- %p).",
                         (long) sp, stack_buffer, stack_buffer + STACK_SIZE);
        }

    deleteThread (thread);
}

START_TEST(exreg0600)
{
    multiple_stack_fetch (createThread (progressive_dummy_thread));
}
END_TEST

START_TEST(exreg0601)
{
    multiple_stack_fetch (createThreadInSpace (L4_nilspace, progressive_dummy_thread));
}
END_TEST

static void multiple_handle_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;
    int s, r;

    a = b = 0;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &handle,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (handle == HANDLE_VALUE1,
                         "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                         (long) handle, (long) HANDLE_VALUE1);
        }

    handle = L4_UserDefinedHandleOf (thread);
    fail_unless (handle == HANDLE_VALUE1,
                 "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                 (long) handle, (long) HANDLE_VALUE1);

    a = 1;
    L4_ThreadSwitch(thread);

    while (!b);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &handle,
                                           &dummy_id);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (handle == HANDLE_VALUE2,
                         "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                         (long) handle, (long) HANDLE_VALUE2);
        }

    handle = L4_UserDefinedHandleOf (thread);
    fail_unless (handle == HANDLE_VALUE2,
                 "User Defined Handle unexpected value (is 0x%lx, want 0x%lx).",
                 (long) handle, (long) HANDLE_VALUE2);

    deleteThread (thread);
}

START_TEST(exreg0700)
{
    multiple_handle_fetch (createThread (progressive_dummy_thread));
}
END_TEST

START_TEST(exreg0701)
{
    multiple_handle_fetch (createThreadInSpace (L4_nilspace, progressive_dummy_thread));
}
END_TEST

static void multiple_pager_fetch (L4_ThreadId_t thread)
{
    L4_Word_t old_control;
    L4_Word_t dummy;
    L4_ThreadId_t pager;
    L4_ThreadId_t result;
    int s, r;

    a = b = 0;

    /* Give the thread a chance to run. */
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);
    mark_point (); /* Mark around a problem on MIPS64. */
    L4_ThreadSwitch(thread);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &dummy,
                                           &pager);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (L4_IsThreadEqual (pager, dummy_pager1),
                         "Unexpected pager value (0x%lx, wanted 0x%lx).", pager, dummy_pager1);
        }

    pager = L4_PagerOf (thread);
    fail_unless (L4_IsThreadEqual (pager, dummy_pager1),
                 "Unexpected pager value.");

    a = 1;
    L4_ThreadSwitch(thread);

    while (!b);

    for (s = 0; s < 2; s++)
        for (r = 0; r < 2; r++) {
            result = L4_ExchangeRegisters (thread, (s << 2) | (r << 1) | L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                                           &old_control, &dummy, &dummy, &dummy, &dummy,
                                           &pager);

            fail_unless (!L4_IsNilThread (result),
                         "Error return: %ld", L4_ErrorCode ());
            fail_unless (old_control == 0, "Non-zero control word: 0x%lx.", (long) old_control);
            fail_unless (L4_IsThreadEqual (pager, dummy_pager2),
                         "Unexpected pager value (0x%lx, wanted 0x%lx).", pager, dummy_pager1);
        }

    pager = L4_PagerOf (thread);
    fail_unless (L4_IsThreadEqual (pager, dummy_pager2),
                 "Unexpected pager value.");

    deleteThread (thread);
}

START_TEST(exreg0800)
{
    multiple_pager_fetch (createThread (progressive_dummy_thread));
}
END_TEST

#if 0
/* Threads outside the roottask space can't change their own pager. */
START_TEST(exreg0801)
{
    multiple_pager_fetch (createThreadInSpace (L4_nilspace, progressive_dummy_thread));
}
END_TEST
#endif

static void high_priority_sending_thread (void)
{
    int i = 0;
    L4_MsgTag_t tag;
    tag.raw = TAG_SRBLOCK;
    while(1)
    {
        L4_MsgTagAddLabelTo(&tag, i++);
        L4_LoadMR(0, (L4_Word_t)tag.raw);
        L4_Send(main_thread);
    }
}

/* rename this test when it is inserted properly into the test plan */
/* temporarilty here to test a corner case for SKS */
/* Check cancelling a higher priority threads IPC */
START_TEST(exregSKS)
{
    L4_MsgTag_t tag;

    L4_ThreadId_t thread = createThreadInSpace(L4_rootspace, high_priority_sending_thread);
    L4_Set_Priority(thread, 225);
    L4_Yield();

    waitSending(thread);
    L4_AbortIpc_and_stop(thread);

    waitSending(thread);
    tag = L4_Receive(thread);
    fail_unless(L4_Label(tag) == 1, "Wrong label");

    deleteThread(thread);
}
END_TEST

static void test_setup(void)
{
    initThreads(0);
    main_thread = L4_Myself();
    dummy_pager1 = L4_GlobalId (255, 255);
    dummy_pager2 = L4_GlobalId (1024, 1024);
}

static void test_teardown(void)
{
}
#endif

/* -------------------------------------------------------------------------- */

static L4_ThreadId_t main_thread;
extern L4_ThreadId_t test_tid;

static void ipc_main_thread(void)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_ThreadId_t dummy;

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    tag = L4_Ipc(main_thread, main_thread, L4_MsgMsgTag(&msg), &dummy);
    if (L4_IpcFailed(tag)) {
        printf("ERROR CODE %ld\n", L4_ErrorCode());
        assert(!"should not get here");
    }
}

/* -------------------------------------------------------------------------- */

/*
\begin{test}{EXREG0100}
  \TestDescription{Verify ExchangeRegisters handles L4\_nilthread as the target}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{ExchangeRegisters} with \Func{L4\_nilthread} as the target  
      \item Check that \Func{ExchangeRegisters} returns an error in result
      \item Check the \Func{ErrorCode} for invalid thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0100)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(L4_nilthread, 0, 0, 0, 0, 0, L4_nilthread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(L4_IsNilThread(res), "ExchangeRegisters succeded on L4_nilthread");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

/*
\begin{test}{EXREG0101}
  \TestDescription{Verify ExchangeRegisters handles L4\_anythread as the target}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{ExchangeRegisters} with \Func{L4\_anythread} as the target 
      \item Check that \Func{ExchangeRegisters} returns an error in result
      \item Check the \Func{ErrorCode}  for invalid thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0101)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(L4_nilthread, 0, 0, 0, 0, 0, L4_anythread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(L4_IsNilThread(res), "ExchangeRegisters succeded on L4_anythread");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

/*
\begin{test}{EXREG0102}
  \TestDescription{Verify ExchangeRegisters handles L4\_anylocalthread as the target}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{ExchangeRegisters} with \Func{L4\_anylocalthread} as the target 
      \item Check that \Func{ExchangeRegisters} returns an error in result
      \item Check the \Func{ErrorCode} for invalid thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0102)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(L4_anylocalthread, 0, 0, 0, 0, 0, L4_anythread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(L4_IsNilThread(res), "ExchangeRegisters succeded on L4_anylocalthread");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

/*
\begin{test}{EXREG0103}
  \TestDescription{Verify ExchangeRegisters handles L4\_waitnotify as the target}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{ExchangeRegisters} with \Func{L4\_waitnotify} as the target  
      \item Check that \Func{ExchangeRegisters} returns an error in result
      \item Check the \Func{ErrorCode} for invalid thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0103)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(L4_waitnotify, 0, 0, 0, 0, 0, L4_anythread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(L4_IsNilThread(res), "ExchangeRegisters succeded on L4_waitnotify");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

static void unpriv_thread(void)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(main_thread, 0, 0, 0, 0, 0, L4_anythread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(L4_IsNilThread(res), "ExchangeRegisters succeded from unprivileged thread");
    _fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __FILE__, __LINE__, "L4 Error code is incorrect (%d)", L4_ErrorCode());

    CLEAR_ERROR_CODE;

    ipc_main_thread();
    L4_WaitForever();

}

/*
\begin{test}{EXREG0200}
  \TestDescription{Verify that ExReg fails when called from an unprivileged thread}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread that is in a different address space, is not the pager the target thread
            and is not a privileged thread
      \item Call \Func{ExchangeRegisters} on the parent thread from the child
      \item Check that \Func{ExchangeRegisters} failed
      \item Check that the \Func{L4\_ErrorCode} is set to \Func{L4\_ErrNoPrivilege}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0200)
{
    L4_ThreadId_t unpriv;

    initThreads(0);

    main_thread = test_tid;
    unpriv = createThreadInSpace(L4_nilspace, unpriv_thread);
    L4_Receive(unpriv);
    deleteThread(unpriv);
}
END_TEST

static void priv_thread(void)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control, sp, ip, flags, user_data;

    res = L4_ExchangeRegisters(main_thread, 0, 0, 0, 0, 0, L4_anythread,
                               &control, &sp, &ip, &flags, &user_data, &pager);
    fail_unless(!L4_IsNilThread(res), "ExchangeRegisters failed");

    ipc_main_thread();
}

/*
\begin{test}{EXREG0201}
  \TestDescription{Verify that ExReg succeeds when called from a thread in the same address space}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread that is in the same address space
      \item Call \Func{ExchangeRegisters} on the parent thread from the child
      \item Check that \Func{ExchangeRegisters} succeeded
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0201)
{
    L4_ThreadId_t priv;

    initThreads(0);

    main_thread = test_tid;
    priv = createThreadInSpace(L4_rootspace, priv_thread);
    waitSending(priv);
    fail_unless(checkSending(priv), "priv thread did not complete");
    deleteThread(priv);
}
END_TEST

/*
\begin{test}{EXREG0300}
  \TestDescription{Verify that ExReg can read the registers from the calling thread}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{ExchangeRegisters} with the 
        \Func{L4\_ExReg\_Deliver} flag and the target thread set to \Func{L4\_Myself()}
      \item Check that the stack pointer returned is the same
      \item Check that the flags, \Func{user\_data} and \Func{pager} are set
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0300)
{
    L4_ThreadId_t res, pager;
    L4_Word_t control=-1, sp=-1, ip=-1, flags=-1, user_data=-1;

    pager.raw = -1;
    res = L4_ExchangeRegisters(L4_Myself(), L4_ExReg_Deliver, 0, 0, 0, 0,
                               L4_anythread, &control, &sp, &ip, &flags,
                               &user_data, &pager);

    fail_unless(!L4_IsNilThread(res), "ExchangeRegisters failed");
    fail_unless(sp <= (L4_Word_t)&user_data + 0x400 && sp > (L4_Word_t)&user_data - 0x400, "ExchangeRegisters failed to get sp");
    fail_unless(flags != -1, "ExchangeRegisters failed to get flags");
    fail_unless(user_data != -1, "ExchangeRegisters failed to get user_data");
    fail_unless(pager.raw != -1, "ExchangeRegisters failed to get pager");
    fail_unless(L4_IsThreadEqual(pager, L4_Pager()), "ExchangeRegisters failed to get pager");
}
END_TEST

static void
simple_thread(void)
{
    while (1)
        ;
}

/*
\begin{test}{EXREG0301}
  \TestDescription{Verify that ExReg can read the ip address of another thread}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a simple test thread the runs with a very small set of ip addresses
      \item Call \Func{ExchangeRegisters} with the 
        \Func{L4\_ExReg\_Deliver} flag on the thread created
      \item Check that the instruction pointer returned is in the range
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(EXREG0301)
{
    L4_ThreadId_t res, pager, simple_thread_id;
    L4_Word_t control=-1, sp=-1, ip=-1, flags=-1, user_data=-1;

    simple_thread_id = createThreadInSpace(L4_rootspace, simple_thread);
    fail_unless(!L4_IsNilThread(simple_thread_id), "Failed to create test thread");

    pager.raw = -1;
    res = L4_ExchangeRegisters(L4_Myself(), L4_ExReg_Deliver, 0, 0, 0, 0,
                               L4_anythread, &control, &sp, &ip, &flags,
                               &user_data, &pager);
    fail_unless(!L4_IsNilThread(res), "ExchangeRegisters failed");
    printf("simple_thread=%p ip=%lx\n", simple_thread, ip);
}
END_TEST


#if defined(ARCH_ARM)
static volatile int exreg0900_counter;

static void exreg0900_suspend_thread(void)
{
    L4_ThreadId_t tid;
    exreg0900_counter = 0;
    L4_Wait(&tid);
    exreg0900_counter = 1;
    for (;;);
}


/*
\begin{test}{EXREG0900}
  \TestDescription{Test Pausing/Resuming of Threads}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Start a thread mid IPC.
      \item Suspend the thread.
      \item Ensure that the thread is still mid IPC.
      \item Allow the IPC operation to complete.
      \item Ensure that the thread does not return to userspace execution.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
\end{test}
*/
START_TEST(EXREG0900)
{
    L4_ThreadId_t thread;
    int i;

    thread = createThread(exreg0900_suspend_thread);
    waitReceiving(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");

    // Suspend and resume, and ensure it is always still receiving
    L4_SuspendThread(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");
    L4_SuspendThread(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");
    L4_UnsuspendThread(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");
    L4_SuspendThread(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");

    // Send to the thread
    L4_Reply(thread);
    fail_unless(!checkReceiving(thread), "Thread is receiving after reply.");

    // Ensure thread is suspended
    for (i = 0; i < 10; i++) {
        fail_unless(exreg0900_counter == 0, "Suspended thread did some work.");
        L4_ThreadSwitch(thread);
    }

    // Unsuspend the thread
    L4_UnsuspendThread(thread);
    for (i = 0; i < 10; i++) {
        if (exreg0900_counter != 0) {
            break;
        }
        L4_ThreadSwitch(thread);
    }
    fail_unless(exreg0900_counter != 0, "Unsuspend thread is not working.");

    deleteThread(thread);
}
END_TEST

static volatile int exreg0901_counter;

static void exreg0901_exception_thread(void)
{
    exreg0901_counter++;

    // Cause a null pointer dereference
    *((int *)0) = 1;

    fail("Should not reach");
    for (;;);
}

/*
\begin{test}{EXREG0901}
  \TestDescription{Test Pausing/Resuming of Threads in Exception State}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Start a thread mid IPC.
      \item Suspend the thread.
      \item Ensure that the thread is still mid IPC.
      \item Allow the IPC operation to complete.
      \item Ensure that the thread does not return to userspace execution.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
\end{test}
*/
START_TEST(EXREG0901)
{
    L4_ThreadId_t thread;
    int i;

    thread = createThread(exreg0901_exception_thread);

    // Set up the current thread as the pager/exception handler/scheduler
    // of the new thread.
    L4_ThreadControl(thread, L4_nilspace, test_tid, test_tid,
            test_tid, 0, (void *)-1UL);

    // wait for the thread to trigger an exception
    exreg0901_counter = 0;
    waitSending(thread);
    fail_unless(checkSending(thread), "Thread is not sending exception IPC.");
    fail_unless(exreg0901_counter == 1, "Thread did not update counter.");

    // Suspend and resume, and ensure it is always still receiving
    L4_SuspendThread(thread);
    fail_unless(checkSending(thread), "Thread is not sending exception IPC.");

    // Tell the thread to retry reading the word. We do this by sending
    // it back the same PC/SP/flags that it sent to us in its exception
    // IPC.
    L4_Receive(thread);
    fail_unless(!checkSending(thread), "Thread is sending after receive.");
    fail_unless(checkReceiving(thread), "Thread is not receiving.");
    L4_Send(thread);
    fail_unless(!checkSending(thread), "Thread is sending after reply.");
    fail_unless(!checkReceiving(thread), "Thread is receiving after reply.");

    // Ensure thread is still suspended
    for (i = 0; i < 10; i++) {
        L4_ThreadSwitch(thread);
        fail_unless(!checkSending(thread), "Suspended thread did some work.");
    }

    // Unsuspend the thread so that it faults again
    L4_UnsuspendThread(thread);
    for (i = 0; i < 10; i++) {
        L4_ThreadSwitch(thread);
    }
    fail_unless(checkSending(thread), "Thread is not sending exception IPC.");

    deleteThread(thread);
}
END_TEST

static volatile int exreg0902_counter;
static volatile int exreg0902_spin;
L4_ThreadId_t suspend_thread, partner_thread;

static void exreg0902_suspend_thread(void)
{
    L4_KDB_SetThreadName(L4_Myself(), "902_s");
    while (exreg0902_spin) {
        exreg0902_counter = 0;
        L4_LoadMR(0, 0);
        L4_Call(partner_thread);
        exreg0902_counter = 1;
    }
    for (;;);
}

static void exreg0902_partner_thread(void)
{
    int i = 0;
    L4_MsgTag_t tag;

    L4_KDB_SetThreadName(L4_Myself(), "902_p");
    L4_Receive(suspend_thread);
    while (1) {
        L4_LoadMR(0, 0);
        tag = L4_Call(suspend_thread);
        fail_unless(L4_IpcSucceeded(tag), "IPC failed");
        if (++i == 10) {
            L4_Call(main_thread);
        }
    }
}

static int
checkIPCing(L4_ThreadId_t thread)
{
    L4_Word_t control, dummy;
    L4_ThreadId_t r, dummy_tid;

    r = L4_ExchangeRegisters(thread, L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread, &control, &dummy, &dummy, &dummy, &dummy, &dummy_tid);
    assert(r.raw == thread.raw);

    return (control & 0x6);
}

/*
\begin{test}{EXREG0902}
  \TestDescription{Test Pausing/Resuming of Threads in fastpath IPC}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Start a thread mid fastpath IPC.
      \item Suspend the thread.
      \item Ensure that the thread is still mid IPC.
      \item Allow the IPC operation to complete.
      \item Ensure that the thread does not return to userspace execution.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
\end{test}
*/
START_TEST(EXREG0902)
{
    int i;

    main_thread = test_tid;
    exreg0902_spin = 1;
    exreg0902_counter = 1;
    suspend_thread = createThread(exreg0902_suspend_thread);
    partner_thread = createThread(exreg0902_partner_thread);
    L4_Set_Priority(partner_thread, 200);
    L4_Set_Priority(suspend_thread, 200);

    // let threads run
    L4_Receive(partner_thread);
    L4_SuspendThread(suspend_thread);
    fail_unless(checkIPCing(suspend_thread) && (exreg0902_counter == 0), "Could not catch thread in mid of fastpath IPC");
    exreg0902_spin = 0;
    L4_Send(partner_thread);

    // Ensure thread is suspended
    for (i = 0; i < 10; i++) {
        fail_unless(exreg0902_counter == 0, "Suspended thread did some work.");
        L4_Yield();
    }

    // Unsuspend the thread
    L4_UnsuspendThread(suspend_thread);
    for (i = 0; i < 10; i++) {
        if (exreg0902_counter != 0) {
            break;
        }
        L4_Yield();
    }
    fail_unless(exreg0902_counter != 0, "Unsuspend thread is not working.");

    deleteThread(suspend_thread);
    deleteThread(partner_thread);
}
END_TEST

static volatile int exreg0910_counter;
static volatile int exreg0910_resume;

static void exreg0910_suspend_notification_thread(void)
{
    L4_ThreadId_t thread;

    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyBits(0);
    L4_Set_NotifyMask(1);
    exreg0910_counter = 0;
    exreg0910_resume = 0;

    // Call back the master
    L4_Wait(&thread);

    // Allow main thread to know we have run
    exreg0910_counter = 1;

    // Wait for main thread to let us continue
    while (!exreg0910_resume);

    // Ensure we received the second notification
    fail_unless(L4_Get_NotifyBits() == 2, "Wrong notification bits.");
    exreg0910_counter = 2;

    // Done
    L4_WaitForever();
}

/*
\begin{test}{EXREG0910}
  \TestDescription{Test suspended thread accepts asynchronous notifications.}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Start a thread waiting for asynchronous notification.
      \item Suspend the thread.
      \item Send the thread a notification.
      \item Ensure thread is not running.
      \item Unsuspend the thread.
      \item Ensure the thread has woken up due to notification.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
\end{test}
*/
START_TEST(EXREG0910)
{
    L4_ThreadId_t thread;
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    int i;

    thread = createThread(exreg0910_suspend_notification_thread);
    waitReceiving(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");

    // Suspend and resume, and ensure it is always still receiving
    L4_SuspendThread(thread);
    fail_unless(checkReceiving(thread), "Thread is not receiving.");

    // Send notification to the thread
    tag.raw = 0;
    tag.raw = TAG_SRBLOCK | 0x2001;

    //L4_Set_Notify(&tag);
    L4_LoadMR(1, 1);
    tag = L4_Ipc(thread, L4_nilthread, tag, &dummy);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed.");

    // Ensure thread is suspended
    for (i = 0; i < 10; i++) {
        fail_unless(exreg0910_counter == 0, "Suspended thread did some work.");
        L4_ThreadSwitch(thread);
    }

    // Allow thread to continue
    L4_UnsuspendThread(thread);
    while (!exreg0910_counter) {
        L4_ThreadSwitch(thread);
    }

    // Pause the thread
    L4_SuspendThread(thread);

    // Send notification
    tag.raw = 0;
    tag.raw = TAG_SRBLOCK | 0x2001;
    //L4_Set_Notify(&tag);
    L4_LoadMR(1, 2);
    tag = L4_Ipc(thread, L4_nilthread, tag, &dummy);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed.");

    // Unsuspend the thread.
    exreg0910_resume = 1;

    // Ensure the thread got the two bits
    L4_UnsuspendThread(thread);
    while (exreg0910_counter != 2) {
        L4_ThreadSwitch(thread);
    }

    deleteThread(thread);
}
END_TEST
#endif
/* -------------------------------------------------------------------------- */

TCase *
make_exchange_registers_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Exchange Register Tests");
    tcase_add_test(tc, EXREG0100);
    tcase_add_test(tc, EXREG0101);
    tcase_add_test(tc, EXREG0102);
    tcase_add_test(tc, EXREG0103);
    tcase_add_test(tc, EXREG0200);
    tcase_add_test(tc, EXREG0201);
    tcase_add_test(tc, EXREG0300);
    tcase_add_test(tc, EXREG0301);

#if 0
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, exreg0100);
    tcase_add_test(tc, exreg0101);
    tcase_add_test(tc, exreg0200);
    tcase_add_test(tc, exreg0201);
    tcase_add_test(tc, exreg0300);
    tcase_add_test(tc, exreg0301);
    tcase_add_test(tc, exreg0302);
    tcase_add_test(tc, exreg0400);
#if 0
    tcase_add_test(tc, exreg0401);
#endif
    tcase_add_test(tc, exreg0500);
    tcase_add_test(tc, exreg0501);
    tcase_add_test(tc, exreg0600);
    tcase_add_test(tc, exreg0601);
    tcase_add_test(tc, exreg0700);
    tcase_add_test(tc, exreg0701);
    tcase_add_test(tc, exreg0800);
#if 0
    tcase_add_test(tc, exreg0801);
#endif
    tcase_add_test(tc, exregSKS);
#endif

#if defined(ARCH_ARM)
    tcase_add_test(tc, EXREG0900);
    tcase_add_test(tc, EXREG0901);
    tcase_add_test(tc, EXREG0902);
    tcase_add_test(tc, EXREG0910);
#endif

    return tc;
}
