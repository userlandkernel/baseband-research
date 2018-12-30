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

#ifndef IPCCOMPLEX_UTILITY_H
#define IPCCOMPLEX_UTILITY_H

#include <stdint.h>
#include <l4/thread.h>

#define MAIN_SBLOCK      0x80
#define MAIN_RBLOCK      0x40
#define RECV_RBLOCK      0x20
#define RECV_IPC_ALLOW   0x10
#define RECV_READY       0x08
#define SENDER_SBLOCK    0x04
#define SENDER_IPC_ALLOW 0x02
#define SENDER_READY     0x01

#define CASE01  (RECV_RBLOCK | RECV_IPC_ALLOW | RECV_READY | SENDER_SBLOCK | SENDER_IPC_ALLOW | SENDER_READY)
#define CASE02  (RECV_RBLOCK | RECV_IPC_ALLOW | SENDER_SBLOCK | SENDER_IPC_ALLOW | SENDER_READY)
#define CASE03  (RECV_RBLOCK | RECV_IPC_ALLOW | SENDER_SBLOCK | SENDER_IPC_ALLOW)
#define CASE04  (RECV_RBLOCK | RECV_IPC_ALLOW | RECV_READY | SENDER_SBLOCK | SENDER_IPC_ALLOW)
#define CASE05  (SENDER_SBLOCK | SENDER_IPC_ALLOW | SENDER_READY)
#define CASE06  0x00
#define CASE07  (RECV_RBLOCK | RECV_IPC_ALLOW | RECV_READY)
#define CASE08  (RECV_IPC_ALLOW | SENDER_SBLOCK | SENDER_IPC_ALLOW | SENDER_READY)
#define CASE09  (RECV_IPC_ALLOW | SENDER_IPC_ALLOW)
#define CASE10  (RECV_RBLOCK | RECV_IPC_ALLOW | RECV_READY | SENDER_IPC_ALLOW)
#define CASE11  (RECV_IPC_ALLOW | SENDER_SBLOCK | SENDER_IPC_ALLOW)
#define CASE12  (RECV_RBLOCK | RECV_IPC_ALLOW | SENDER_SBLOCK)

void create_customised_thread(L4_Word_t);
void ipccomplex_test(L4_Word_t);
void ipccomplex_cleanup(void);
L4_ThreadId_t get_ipc_main_tid(void);
L4_ThreadId_t get_xas_main_tid(void);
L4_ThreadId_t get_receiver_tid(void);
L4_ThreadId_t get_sender_tid(void);
L4_ThreadId_t get_deblocker_tid(void);

#endif
