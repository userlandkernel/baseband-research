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

#include <stdio.h>
#include <iguana/thread.h>
#include <iguana/env.h>
#include <l4/ipc.h>
#include "util.h"
#include <l4/kdebug.h>


#define PASSED()                printf("PASSED.\n")
#define FAILED()                printf("FAILED (line=%d).\n", __LINE__)

static L4_ThreadId_t main_tid;

/*
 * Get current value, request timeout in the future for main thread, check
 * notified correctly.
 */
static void
test01(void)
{
    uint32_t cur_value;
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;
    L4_MsgTag_t tag;
    L4_Word_t mask;

    printf("%s: ", __func__);

    result = counter_current_value(&cur_value);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_request(cur_value + 10, main_tid, notifybits);
    if (result == 0) {
        FAILED();
        return;
    }
    L4_Set_NotifyMask(notifybits);
    L4_Accept(L4_NotifyMsgAcceptor);
    tag = L4_WaitNotify(&mask);
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    PASSED();
}

static void
waiting_notify_thread(void *arg)
{
    L4_Word_t notifybits;
    L4_MsgTag_t tag;
    L4_Word_t mask;

    tag = L4_Receive(main_tid);
    notifybits = L4_Label(tag);
    L4_Set_NotifyMask(notifybits);
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_LoadMR(0, 0);
    L4_Send(main_tid);
    tag = L4_WaitNotify(&mask);
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    L4_Set_Label(&tag, notifybits);
    L4_Set_MsgTag(tag);
    L4_Call(main_tid);

    while (1) { }
}

/*
 * Get current value, request timeout for different threads, check all notified
 * correctly.
 */
static void
test02(void)
{
    uint32_t cur_value;
    L4_Word_t result, i;
    thread_ref_t thread1, thread2, thread3;
    L4_ThreadId_t tid1, tid2, tid3, any_thread;
    L4_Word_t notifybits1 = 0x1;
    L4_Word_t notifybits2 = 0x2;
    L4_Word_t notifybits3 = 0x3;
    L4_MsgTag_t tag;
    L4_Word_t label;    

    printf("%s: ", __func__);

    thread1 = thread_create_simple(waiting_notify_thread, NULL, 99);
    thread2 = thread_create_simple(waiting_notify_thread, NULL, 99);
    thread3 = thread_create_simple(waiting_notify_thread, NULL, 99);
    tid1 = thread_l4tid(thread1);
    tid2 = thread_l4tid(thread2);
    tid3 = thread_l4tid(thread3);
    tag = L4_Make_MsgTag(notifybits1, 0);
    L4_Set_MsgTag(tag);
    tag = L4_Call(tid1);
    tag = L4_Make_MsgTag(notifybits2, 0);
    L4_Set_MsgTag(tag);
    tag = L4_Call(tid2);
    tag = L4_Make_MsgTag(notifybits3, 0);
    L4_Set_MsgTag(tag);
    tag = L4_Call(tid3);

    result = counter_current_value(&cur_value);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_request(cur_value + 10, tid1, notifybits1);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_request(cur_value + 12, tid2, notifybits2);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_request(cur_value + 14, tid3, notifybits3);
    if (result == 0) {
        FAILED();
        return;
    }

    for (i = 0; i < 3; i++) {
        tag = L4_Wait(&any_thread);
        label = L4_Label(tag);
        if (((i == 0) && (label != 0x1)) ||
            ((i == 1) && (label != 0x2)) ||
            ((i == 2) && (label != 0x3))) {
            thread_delete(tid1);
            thread_delete(tid2);
            thread_delete(tid3);
            FAILED();
            return;
        }
    }

    thread_delete(tid1);
    thread_delete(tid2);
    thread_delete(tid3);

    PASSED();
}

/*
 * Get current value, request timeout for next value, increment counter, check
 * notified correctly.
 */
static void
test03(void)
{
    uint32_t cur_value;
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;
    L4_MsgTag_t tag;
    L4_Word_t mask;

    printf("%s: ", __func__);

    result = counter_current_value(&cur_value);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_request(cur_value + 1, main_tid, notifybits);
    if (result == 0) {
        FAILED();
        return;
    }
    result = counter_increment(1);
    if (result == 0) {
        FAILED();
        return;
    }
    L4_Set_NotifyMask(notifybits);
    L4_Accept(L4_NotifyMsgAcceptor);
    tag = L4_WaitNotify(&mask);
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    PASSED();
}

int
main(void)
{
    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    printf("\nKernel counter example\n");
    printf("----------\n");

    test01();
    test02();
    test03();

    printf("Kernel counter example finished\n");
    L4_KDB_Enter("done");

    return 0;
}
