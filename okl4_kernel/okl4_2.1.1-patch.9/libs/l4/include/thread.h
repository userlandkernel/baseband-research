/*
 * Copyright (c) 2001-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
/*
 * Description:   Thread handling interfaces
 */
#ifndef __L4__THREAD_H__
#define __L4__THREAD_H__

#include <compat/c.h>
#include <l4/types.h>
#include <l4/arch/syscalls.h>
#include <l4/arch/vregs.h>
#include <l4/arch/thread.h>

/*
 * Thread states
 */

typedef struct L4_ThreadState {
    L4_Word_t raw;
} L4_ThreadState_t;

L4_INLINE L4_Bool_t
L4_ThreadWasHalted(L4_ThreadState_t s)
{
    return s.raw & (1 << 0);
}

L4_INLINE L4_Bool_t
L4_ThreadWasReceiving(L4_ThreadState_t s)
{
    return s.raw & (1 << 1);
}

L4_INLINE L4_Bool_t
L4_ThreadWasSending(L4_ThreadState_t s)
{
    return s.raw & (1 << 2);
}

L4_INLINE L4_Bool_t
L4_ThreadWasIpcing(L4_ThreadState_t s)
{
    return s.raw & (3 << 1);
}

/*
 * TCR related functions
 */

L4_INLINE L4_ThreadId_t
L4_MyGlobalId(void)
{
    L4_ThreadId_t ret;
    ret.raw = L4_myselfconst.raw;
    return ret;
}


L4_INLINE L4_ThreadId_t
L4_Myself(void)
{
    return L4_MyGlobalId();
}

L4_INLINE L4_ThreadId_t
L4_MyMutexThreadHandle(void)
{
    return utcb_base_get()->mutex_thread_handle;
}

L4_INLINE L4_Word_t
L4_ProcessorNo(void)
{
    return __L4_TCR_ProcessorNo();
}

L4_INLINE L4_Word_t
L4_UserDefinedHandle(void)
{
    return __L4_TCR_UserDefinedHandle();
}

L4_INLINE void
L4_Set_UserDefinedHandle(L4_Word_t NewValue)
{
    __L4_TCR_Set_UserDefinedHandle(NewValue);
}

L4_INLINE L4_Word_t
L4_ErrorCode(void)
{
    return __L4_TCR_ErrorCode();
}

L4_INLINE L4_SpaceId_t
L4_SenderSpace(void)
{
    L4_SpaceId_t id;

    id.raw = __L4_TCR_SenderSpace();
    return id;
}

/*
 * Derived functions
 */

/*
 * These are the functions derived from the exchange register syscall.
 * For normal C, function overloading is not supported.  Functions
 * with unique names (i.e., with the suffix inside <> appended) have
 * therefore been provided.
 *
 *   L4_UserDefinedHandle<Of>                   (t)
 *   L4_Set_UserDefinedHandle<Of>               (t, handle)
 *   L4_Pager<Of>                               (t)
 *   L4_Set_Pager<Of>                           (t, pager)
 *   L4_Start                                   (t)
 *   L4_Start<_SpIp>                            (t, sp, ip)
 *   L4_Start<_SpIpFlags>                       (t, sp, ip, flags)
 *   L4_Stop                                    (t)
 *   L4_Stop_Thread                             (t)     // No return state
 *   L4_Stop<_SpIpFlags>                        (t, &sp, &ip, &flags)
 *   L4_SuspendThread                           (t)
 *   L4_UnsuspendThread                         (t)
 *   L4_AbortOperation                          (t)
 *   L4_AbortOperationAndHalt                   (t)
 *   L4_Copy_regs<_SpIp>                        (src, dest)
 *   L4_Copy_regs_to_mrs                        (src)
 *
 */

L4_INLINE L4_Word_t
L4_UserDefinedHandleOf(L4_ThreadId_t t)
{
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &handle,
                               &dummy_id);

    return handle;
}

L4_INLINE void
L4_Set_UserDefinedHandleOf(L4_ThreadId_t t, L4_Word_t handle)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_user, 0, 0, 0, handle, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

#if defined(__l4_cplusplus)
L4_INLINE L4_Word_t
L4_UserDefinedHandle(L4_ThreadId_t t)
{
    return L4_UserDefinedHandleOf(t);
}

L4_INLINE void
L4_Set_UserDefinedHandle(L4_ThreadId_t t, L4_Word_t handle)
{
    L4_Set_UserDefinedHandleOf(t, handle);
}
#endif /* __l4_cplusplus */

L4_INLINE L4_ThreadId_t
L4_PagerOf(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t pager;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy, &pager);

    return pager;
}

L4_INLINE L4_Word_t
L4_Set_PagerOf(L4_ThreadId_t t, L4_ThreadId_t pager)
{
    return L4_ThreadControl(t, L4_nilspace, L4_nilthread, pager,
                            L4_nilthread, 0, (void *)-1UL);
}

L4_INLINE L4_ThreadId_t
L4_Pager(void)
{
    return L4_PagerOf(L4_Myself());
}

L4_INLINE L4_Word_t
L4_Set_ExceptionHandlerOf(L4_ThreadId_t t, L4_ThreadId_t except_handler)
{
    return L4_ThreadControl(t, L4_nilspace, L4_nilthread, L4_nilthread,
                            except_handler, 0, (void *)-1UL);
}

#if defined(__l4_cplusplus)
L4_INLINE L4_ThreadId_t
L4_Pager(L4_ThreadId_t t)
{
    return L4_PagerOf(t);
}

L4_INLINE void
L4_Set_Pager(L4_ThreadId_t t, L4_ThreadId_t p)
{
    L4_Set_PagerOf(t, p);
}
#endif /* __l4_cplusplus */

L4_INLINE void
L4_Start(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Resume + L4_ExReg_AbortIPC, 0, 0, 0,
                               0, L4_nilthread, &dummy, &dummy, &dummy, &dummy,
                               &dummy, &dummy_id);
}

L4_INLINE void
L4_Start_SpIp(L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_sp_ip + L4_ExReg_Resume +
                               L4_ExReg_AbortIPC, sp, ip, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

L4_INLINE void
L4_Start_SpIpFlags(L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip, L4_Word_t flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_sp_ip_flags + L4_ExReg_Resume +
                               L4_ExReg_AbortIPC, sp, ip, flags, 0,
                               L4_nilthread, &dummy, &dummy, &dummy, &dummy,
                               &dummy, &dummy_id);
}

#if defined(__l4_cplusplus)
L4_INLINE void
L4_Start(L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip)
{
    L4_Start_SpIp(t, sp, ip);
}

L4_INLINE void
L4_Start(L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip, L4_Word_t flags)
{
    L4_Start_SpIpFlags(t, sp, ip, flags);
}
#endif

L4_INLINE L4_ThreadState_t
L4_Stop(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt + L4_ExReg_Deliver,
                               0, 0, 0, 0, L4_nilthread,
                               &state.raw, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);

    return state;
}

L4_INLINE void
L4_Stop_Thread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt, 0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

L4_INLINE L4_ThreadState_t
L4_Stop_SpIpFlags(L4_ThreadId_t t,
                  L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt + L4_ExReg_Deliver,
                               0, 0, 0, 0, L4_nilthread,
                               &state.raw, sp, ip, flags, &dummy, &dummy_id);

    return state;
}

#if defined(__l4_cplusplus)
L4_INLINE L4_ThreadState_t
L4_Stop(L4_ThreadId_t t, L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    return L4_Stop_SpIpFlags(t, sp, ip, flags);
}
#endif

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortReceive_and_stop(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortRecvIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);

    return state;
}

DEPRECATED L4_INLINE void
L4_AbortReceive_and_stop_Thread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt + L4_ExReg_AbortRecvIPC,
                               0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortReceive_and_stop_SpIpFlags(L4_ThreadId_t t,
                                   L4_Word_t *sp,
                                   L4_Word_t *ip, L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortRecvIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, sp, ip, flags, &dummy, &dummy_id);

    return state;
}

#if defined(__l4_cplusplus)
DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortReceive_and_stop(L4_ThreadId_t t,
                         L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    return L4_AbortReceive_and_stop_SpIpFlags(t, sp, ip, flags);
}
#endif

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortSend_and_stop(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortSendIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);

    return state;
}

DEPRECATED L4_INLINE void
L4_AbortSend_and_stop_Thread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt + L4_ExReg_AbortSendIPC,
                               0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortSend_and_stop_SpIpFlags(L4_ThreadId_t t,
                                L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{

    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortSendIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, sp, ip, flags, &dummy, &dummy_id);

    return state;
}

#if defined(__l4_cplusplus)
DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortSend_and_stop(L4_ThreadId_t t,
                      L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    return L4_AbortSend_and_stop_SpIpFlags(t, sp, ip, flags);
}
#endif

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortIpc_and_stop(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);

    return state;
}

DEPRECATED L4_INLINE void
L4_AbortIpc_and_stop_Thread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Halt + L4_ExReg_AbortIPC,
                               0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortIpc_and_stop_SpIpFlags(L4_ThreadId_t t,
                               L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t,
                               L4_ExReg_Halt + L4_ExReg_AbortIPC +
                               L4_ExReg_Deliver, 0, 0, 0, 0, L4_nilthread,
                               &state.raw, sp, ip, flags, &dummy, &dummy_id);

    return state;
}

#if defined(__l4_cplusplus)
DEPRECATED L4_INLINE L4_ThreadState_t
L4_AbortIpc_and_stop(L4_ThreadId_t t,
                     L4_Word_t *sp, L4_Word_t *ip, L4_Word_t *flags)
{
    return L4_AbortIpc_and_stop_SpIpFlags(t, sp, ip, flags);
}
#endif

L4_INLINE L4_ThreadState_t
L4_AbortOperation(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t, L4_ExReg_AbortOperation,
            0, 0, 0, 0, L4_nilthread, &state.raw, &dummy, &dummy, &dummy,
            &dummy, &dummy_id);

    return state;
}

L4_INLINE L4_ThreadState_t
L4_AbortOperationAndHalt(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;

    (void)L4_ExchangeRegisters(t, L4_ExReg_AbortOperation | L4_ExReg_Halt,
            0, 0, 0, 0, L4_nilthread, &state.raw, &dummy, &dummy, &dummy,
            &dummy, &dummy_id);

    return state;
}

L4_INLINE void
L4_Copy_regs(L4_ThreadId_t src, L4_ThreadId_t dest)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(dest,
                               L4_ExReg_CopyRegs +
                               L4_ExReg_SrcThread(src), 0, 0, 0, 0,
                               L4_nilthread, &dummy, &dummy, &dummy, &dummy,
                               &dummy, &dummy_id);
}

L4_INLINE void
L4_Copy_regs_SpIp(L4_ThreadId_t src, L4_ThreadId_t dest, L4_Word_t sp,
                  L4_Word_t ip)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(dest, L4_ExReg_sp_ip + L4_ExReg_CopyRegs +
                               L4_ExReg_SrcThread(src),
                               sp, ip, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

L4_INLINE void
L4_Copy_mrs_to_regs(L4_ThreadId_t dest)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(dest, L4_ExReg_MRsToRegs,
                               0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

L4_INLINE void
L4_Copy_regs_to_mrs(L4_ThreadId_t src)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(src, L4_ExReg_RegsToMRs,
                               0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

#if 0
L4_INLINE L4_Word_t
L4_AssociateInterrupt(L4_ThreadId_t InterruptThread,
                      L4_ThreadId_t HandlerThread)
{
    return L4_ThreadControl(InterruptThread, L4_nilspace,
                            L4_nilthread, HandlerThread,
                            L4_nilthread, 0, (void *)-1UL);
}

L4_INLINE L4_Word_t
L4_DeassociateInterrupt(L4_ThreadId_t InterruptThread)
{
    return L4_ThreadControl(InterruptThread, L4_nilspace,
                            L4_nilthread, InterruptThread,
                            L4_nilthread, 0, (void *)-1UL);
}
#endif

L4_INLINE void
L4_SuspendThread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Suspend, 0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

L4_INLINE void
L4_UnsuspendThread(L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;

    (void)L4_ExchangeRegisters(t, L4_ExReg_Unsuspend, 0, 0, 0, 0, L4_nilthread,
                               &dummy, &dummy, &dummy, &dummy, &dummy,
                               &dummy_id);
}

#endif /* !__L4__THREAD_H__ */
