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

#include <l4test/utility.h>
#include "ipccomplex_utility.h"
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/caps.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <check/check.h>
#include <stdio.h>
#include <stdlib.h>

static int g_received = -1;
static L4_ThreadId_t xas_main_thread, receiver, sender, deblocker;
static L4_ThreadId_t ipc_main_thread;

static L4_SpaceId_t xas_main_space, receiver_space, sender_space;
static L4_ClistId_t xas_main_clist, receiver_clist, sender_clist;

static L4_ThreadId_t xas_main_cap, ipc_main_cap, receiver_cap, sender_cap, pager_cap;

static void check_send_receive_ok(L4_MsgTag_t rcv_tag, L4_Msg_t rcv_msg);
static void check_send_receive_nok(L4_MsgTag_t rcv_tag, L4_Word_t ErrorCode, L4_Word_t ExpectedError);
static void check_sendOK_receiveNOK(L4_MsgTag_t rcv_tag, L4_Word_t ErrorCode, L4_Word_t ExpectedError);

static volatile int receiver_start = 0;
static volatile int sender_start = 0;
static volatile int main_start = 0;

L4_ThreadId_t
get_xas_main_tid(void)
{
    return xas_main_thread;
}

L4_ThreadId_t
get_receiver_tid(void)
{
    return receiver;
}

L4_ThreadId_t
get_sender_tid(void)
{
    return sender;
}

L4_ThreadId_t
get_deblocker_tid(void)
{
    return deblocker;
}

static void
ipccomplex_cleanup_clist(L4_ClistId_t clist)
{
    L4_DeleteCap(clist, xas_main_cap);
    L4_DeleteCap(clist, ipc_main_cap);
    L4_DeleteCap(clist, receiver_cap);
    L4_DeleteCap(clist, sender_cap);
    L4_DeleteCap(clist, pager_cap);
    L4_DeleteClist(clist);
}

void
ipccomplex_cleanup(void)
{
    deleteThread(receiver);
    deleteThread(sender);
    deleteThread(xas_main_thread);
    deleteThread(deblocker);
    deleteSpace(xas_main_space);
    deleteSpace(receiver_space);
    deleteSpace(sender_space);
    ipccomplex_cleanup_clist(xas_main_clist);
    ipccomplex_cleanup_clist(receiver_clist);
    ipccomplex_cleanup_clist(sender_clist);
}

void
ipccomplex_test(L4_Word_t case_code)
{
    L4_ThreadId_t xas_main_thread;
    L4_MsgTag_t tag;

    create_customised_thread(case_code);

    xas_main_thread = get_xas_main_tid();
    tag = L4_Receive(xas_main_thread);
    fail_unless(L4_IpcSucceeded(tag), "Thread did not complete");

    ipccomplex_cleanup();
}

static void
deblocker_thread(L4_ThreadId_t me)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    L4_Start(xas_main_thread);
    L4_Yield();

    L4_Start(ipc_main_thread);
    
    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    L4_Send(me);
}

static void
receiving_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_Word_t blocking;

    while (!receiver_start);

    blocking = L4_UserDefinedHandle();

    if (blocking) {
        tag.raw = TAG_RBLOCK;
    } else {
        tag.raw = TAG_NOBLOCK;
    }
   
    tag = L4_Ipc(L4_nilthread, xas_main_cap, tag, &dummy);

    g_received = 1;
    if (getTagE(tag) == 1) {
        g_received = 0;
    } else {
        L4_MsgStore(tag, &msg);
        if ((getTagU(L4_MsgMsgTag(&msg)) != 10) || (L4_MsgLabel(&msg) != MAGIC_LABEL2)) {
            g_received = 0;
        }
        if ((L4_MsgWord(&msg, 0) != MAGIC_MR0) || (L4_MsgWord(&msg, 1) != MAGIC_MR1) ||
            (L4_MsgWord(&msg, 2) != MAGIC_MR2) || (L4_MsgWord(&msg, 3) != MAGIC_MR3) ||
            (L4_MsgWord(&msg, 4) != MAGIC_MR4) || (L4_MsgWord(&msg, 5) != MAGIC_MR5) ||
            (L4_MsgWord(&msg, 6) != MAGIC_MR6) || (L4_MsgWord(&msg, 7) != MAGIC_MR7) ||
            (L4_MsgWord(&msg, 8) != MAGIC_MR8) || (L4_MsgWord(&msg, 9) != MAGIC_MR9)) {
            g_received = 0;
        }
    }

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    tag = L4_Ipc(xas_main_cap, xas_main_cap, L4_MsgMsgTag(&msg), &dummy);
    _fail_unless(L4_IpcSucceeded(tag), __FILE__, __LINE__,
                 "IPC failed, Error: %lu", L4_ErrorCode());
}

static void
sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_Word_t blocking;

    while (!sender_start);
    
    tag.raw = 0;
    blocking = L4_UserDefinedHandle();

    if (blocking) {
        tag.raw = TAG_SBLOCK;
    } else {
        tag.raw = TAG_NOBLOCK;
    }
    fillMessage(&msg, tag);
    L4_MsgLoad(&msg);

    L4_Ipc(xas_main_cap, L4_nilthread, L4_MsgMsgTag(&msg), &dummy);

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    L4_Send(sender_cap);
}

static void
check_send_receive_ok(L4_MsgTag_t rcv_tag, L4_Msg_t rcv_msg)
{
    fail_if(g_received == 0,
            "main thread sent message but the message has been corrupted");
    fail_unless(g_received == 1, "main thread was not able to send message");
    fail_unless(getTagE(rcv_tag) == 0,
                "Error returned : main thread did not receive message");
    checkMessage(&rcv_msg, 1);
}

static void
check_send_receive_nok(L4_MsgTag_t rcv_tag, L4_Word_t ErrorCode, L4_Word_t ExpectedError)
{
    L4_Bool_t phase;
    L4_Word_t IpcErrorCode;
    
    fail_if(g_received > 0, "main thread managed to send message");
    fail_unless(getTagE(rcv_tag) == 1,
                "Failed to return Error : main thread received message");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == ExpectedError, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);
}

static void
check_sendOK_receiveNOK(L4_MsgTag_t rcv_tag, L4_Word_t ErrorCode, L4_Word_t ExpectedError)
{
    L4_Bool_t phase;
    L4_Word_t IpcErrorCode;

    fail_if(g_received == 0,
            "main thread sent message but the message has been corrupted");
    fail_unless(g_received == 1, "main thread was not able to send message");
    fail_unless(getTagE(rcv_tag) == 1,
                "Failed to return Error : main thread received message");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrRecvPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == ExpectedError, __FILE__, __LINE__,
                 "Wrong IPC error code : %lu, expected %lu", IpcErrorCode, ExpectedError);
}

static void
ownas_main_thread(void)
{
    L4_ThreadId_t dummy_tid;
    L4_MsgTag_t tag, rcv_tag;
    L4_Msg_t msg, rcv_msg;
    L4_Word_t case_code, reduced_case_code, ErrorCode;

    while (!main_start);
    
    case_code = L4_UserDefinedHandle();
    reduced_case_code = case_code & CASE01;
    g_received = -1;
    tag = L4_Niltag;
    tag.raw = 0;

    if (case_code &  MAIN_SBLOCK) {
        L4_Set_SendBlock(&tag);
    } else {
        L4_Clear_SendBlock(&tag);
    }
    if (case_code & MAIN_RBLOCK) {
        L4_Set_ReceiveBlock(&tag);
    } else {
        L4_Clear_ReceiveBlock(&tag);
    }
    fillMessage(&msg, tag);
    L4_MsgLoad(&msg);

    rcv_tag = L4_Ipc(receiver_cap, sender_cap, L4_MsgMsgTag(&msg), &dummy_tid);
    L4_MsgStore(rcv_tag, &rcv_msg);
    ErrorCode = L4_ErrorCode();

    waitThreadTimeout(receiver_cap);
    if ((case_code &  MAIN_SBLOCK) && (case_code & MAIN_RBLOCK)) {

        if ((case_code & RECV_IPC_ALLOW) && (case_code & SENDER_IPC_ALLOW)) {
            check_send_receive_ok(rcv_tag, rcv_msg);
        } else {
            if (!(case_code & RECV_IPC_ALLOW)) {
                check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrNonExist);
            } else {
                check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrNonExist);
            }
        }

    }

    if (!(case_code &  MAIN_SBLOCK) && (case_code & MAIN_RBLOCK)) {
        if (reduced_case_code == CASE01 || reduced_case_code == CASE04 ||
            reduced_case_code == CASE10) {
            check_send_receive_ok(rcv_tag, rcv_msg);
        }
        if (reduced_case_code == CASE02 || reduced_case_code == CASE03 ||
            reduced_case_code == CASE08 || reduced_case_code == CASE09 ||
            reduced_case_code == CASE11 || reduced_case_code == CASE12) {
            check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrTimeout);
        }
        if (reduced_case_code == CASE05 || reduced_case_code == CASE06) {
            check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
        if (reduced_case_code == CASE07) {
            check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
    }

    if ((case_code &  MAIN_SBLOCK) && !(case_code & MAIN_RBLOCK)) {
        if (reduced_case_code == CASE01 || reduced_case_code == CASE02 ||
            reduced_case_code == CASE08) {
            check_send_receive_ok(rcv_tag, rcv_msg);
        }
        if (reduced_case_code == CASE05 || reduced_case_code == CASE06) {
            check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
        if (reduced_case_code == CASE03 || reduced_case_code == CASE04 ||
            reduced_case_code == CASE09 || reduced_case_code == CASE10 ||
            reduced_case_code == CASE11) {
            check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrTimeout);
        }
        if ( reduced_case_code == CASE12 ||
              reduced_case_code == CASE07) {
            check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
    }

    if (!(case_code &  MAIN_SBLOCK) && !(case_code & MAIN_RBLOCK)) {
        if (reduced_case_code == CASE01) {
            check_send_receive_ok(rcv_tag, rcv_msg);
        }
        if (reduced_case_code == CASE02 || reduced_case_code == CASE03 ||
            reduced_case_code == CASE08 || reduced_case_code == CASE09 ||
            reduced_case_code == CASE11 || reduced_case_code == CASE12) {
            check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrTimeout);
        }
        if (reduced_case_code == CASE05 || reduced_case_code == CASE06) {
            check_send_receive_nok(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
        if (reduced_case_code == CASE04 || reduced_case_code == CASE10) {
            check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrTimeout);
        }
        if (reduced_case_code == CASE07) {
            check_sendOK_receiveNOK(rcv_tag, ErrorCode, L4_ErrNonExist);
        }
    }

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    tag = L4_Ipc(ipc_main_cap, ipc_main_cap, L4_MsgMsgTag(&msg), &dummy_tid);
    L4_IpcError(L4_ErrorCode(), &ErrorCode);
    _fail_unless(L4_IpcSucceeded(tag), __FILE__, __LINE__,
                 "IPC failed, Error: %lu", ErrorCode);
}

void
create_customised_thread(L4_Word_t case_code)
{
    L4_Word_t result, ErrorCode;
    int i;
    L4_Word_t size = 0x100;
    
    ipc_main_thread = get_ipc_main_tid();

    /* Initialize cap ids */
    pager_cap = L4_GlobalId(2,1);
    xas_main_cap = L4_GlobalId(3,1);
    ipc_main_cap = L4_GlobalId(4,1);
    receiver_cap = L4_GlobalId(5,1);
    sender_cap = L4_GlobalId(6,1);

    /* deblocker in rootspace with rootclist */
    deblocker = createThreadInSpace(L4_rootspace, (void (*)(void))deblocker_thread);
    
    /* xas_main in own space with own clist */
    xas_main_clist = L4_ClistId(2);
    L4_CreateClist(xas_main_clist, size);
    L4_CreateIpcCap(default_thread_handler, L4_rootclist, pager_cap, xas_main_clist);
    xas_main_space = createSpaceWithClist(xas_main_clist);
    xas_main_thread = createThreadInSpace(xas_main_space, ownas_main_thread);
    L4_Set_UserDefinedHandleOf(xas_main_thread, case_code);

    /* receiver in own space with own clist */
    receiver_clist = L4_ClistId(3);
    L4_CreateClist(receiver_clist, size);
    L4_CreateIpcCap(default_thread_handler, L4_rootclist, pager_cap, receiver_clist);
    receiver_space = createSpaceWithClist(receiver_clist);
    receiver = createThreadInSpace(receiver_space, receiving_thread);

    if (case_code & RECV_RBLOCK) {
        L4_Set_UserDefinedHandleOf(receiver, 1);
    } else {
        L4_Set_UserDefinedHandleOf(receiver, 0);
    }

    if (case_code & RECV_IPC_ALLOW) {

        /* Cap for xas_main to talk to receiver and ipc_main */
        L4_CreateIpcCap(receiver, L4_rootclist, receiver_cap, xas_main_clist);
        L4_CreateIpcCap(xas_main_thread, L4_rootclist, xas_main_cap, receiver_clist);
        L4_CreateIpcCap(ipc_main_thread, L4_rootclist, ipc_main_cap, xas_main_clist);

    } else {

        /* Create no cap for xas_main to talk to receiver
         * But still create one for xas_main to talk to ipc_main */
        L4_CreateIpcCap(ipc_main_thread, L4_rootclist, ipc_main_cap, xas_main_clist);
    }

    /* sender in own space with own clist */
    sender_clist = L4_ClistId(4);
    L4_CreateClist(sender_clist, size);
    L4_CreateIpcCap(default_thread_handler, L4_rootclist, pager_cap, sender_clist);
    sender_space = createSpaceWithClist(sender_clist);
    sender = createThreadInSpace(sender_space, sending_thread);

    if (case_code & SENDER_SBLOCK) {
        L4_Set_UserDefinedHandleOf(sender, 1);
    } else {
        L4_Set_UserDefinedHandleOf(sender, 0);
    }

    if (case_code & SENDER_IPC_ALLOW) {

        /* Cap for sender to xas_main, xas_main to ipc_main, sender to sender */
        L4_CreateIpcCap(xas_main_thread, L4_rootclist, xas_main_cap, sender_clist);
        L4_CreateIpcCap(sender, L4_rootclist, sender_cap, xas_main_clist);
        L4_CreateIpcCap(ipc_main_thread, L4_rootclist, ipc_main_cap, xas_main_clist);
        L4_CreateIpcCap(sender, L4_rootclist, sender_cap, sender_clist);

    } else {

        /* Cap for sender to sender, xas_main to ipc_main, but no sender to xas_main */
        L4_CreateIpcCap(ipc_main_thread, L4_rootclist, ipc_main_cap, xas_main_clist);
        L4_CreateIpcCap(sender, L4_rootclist, sender_cap, sender_clist);
    }
    
    result = L4_Set_Priority(xas_main_thread, 130);
    ErrorCode = L4_ErrorCode();
    _fail_unless(result != L4_SCHEDRESULT_ERROR, __FILE__, __LINE__,
                 "L4_Set_Priority() failed to set the new priority, error code : %lu", ErrorCode);
    result = L4_Set_Priority(receiver, 120);
    ErrorCode = L4_ErrorCode();
    _fail_unless(result != L4_SCHEDRESULT_ERROR, __FILE__, __LINE__,
                 "L4_Set_Priority() failed to set the new priority, error code : %lu", ErrorCode);
    result = L4_Set_Priority(sender, 110);
    ErrorCode = L4_ErrorCode();
    _fail_unless(result != L4_SCHEDRESULT_ERROR, __FILE__, __LINE__,
                 "L4_Set_Priority() failed to set the new priority, error code : %lu", ErrorCode);

    receiver_start = 1;

    if ((case_code & RECV_READY) && (case_code & RECV_RBLOCK)) {
        for (i = 0 ; i < 2 ; i++)
            L4_ThreadSwitch(receiver);
        while (!checkReceiving(receiver)) {
            L4_ThreadSwitch(receiver);
        }
    }

    sender_start = 1;

    if ((case_code & SENDER_READY) && (case_code & SENDER_SBLOCK)) {
        for (i = 0 ; i < 2 ; i++)
            L4_ThreadSwitch(sender);
        while (!checkSending(sender)) {
            L4_ThreadSwitch(sender);
        }
    }

    main_start = 1;
}
