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
 * Copyright (c) 2007-2008 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   Interrupt registration and acknowledgement
 */
#ifndef __L4__INTERRUPT_H__
#define __L4__INTERRUPT_H__

#include <l4/types.h>
#include <l4/arch/syscalls.h>

/*
 * L4_RegisterInterrupt
 *
 * Asks L4 kernel to send an asynchronous IPC to a thread within the caller's
 * address space, when an interrupt occurs.
 *
 * In addition to the following function arguments, the caller must store
 * information about the requested interrupt in IPC message registers.  The
 * number of message registers used for this purpose should be specified
 * in the 'mrs' argument.
 *
 * The encoding of interupts in message registers is platform-specific, and
 * should be described in the Platform BSP documentation.  For OKL4 platforms,
 * the encoding should simply be storing the interrupt number in MR[0], and
 * setting the 'mrs' argument to 0.
 *
 * Arguments
 *
 * - target:        the thread that will receive interrupt notifications.  Must
 *                  be a thread within the caller's address space.
 * - notify_bit:    the async notify bit that will used as the interrupt notify
 *                  flag.
 * - mrs:           the number of message registers used to encode interrupt
 *                  information.
 * - request:       optional argument, interpretation is specific to BSP.
 */
L4_INLINE L4_Word_t
L4_RegisterInterrupt(L4_ThreadId_t target, L4_Word_t notify_bit, L4_Word_t mrs, L4_Word_t request)
{
    return
        L4_InterruptControl
        (
            target,
            L4_InterruptControl_count(mrs) |
            L4_InterruptControl_op(L4_InterruptControl_RegisterIrq) |
            L4_InterruptControl_request(request) |
            L4_InterruptControl_notifybit(notify_bit)
        );
}

/*
 * See comments for L4_RegisterInterrupt()
 */
L4_INLINE L4_Word_t
L4_UnregisterInterrupt(L4_ThreadId_t target, L4_Word_t mrs, L4_Word_t request)
{
    return
        L4_InterruptControl
        (
            target,
            L4_InterruptControl_count(mrs) |
            L4_InterruptControl_op(L4_InterruptControl_UnregisterIrq) |
            L4_InterruptControl_request(request) |
            L4_InterruptControl_notifybit(0)
        );
}

/*
 * See comments for L4_RegisterInterrupt()
 *
 * The interrupt to acknowledge should be described in message registers
 */
L4_INLINE L4_Word_t
L4_AcknowledgeInterrupt(L4_Word_t mrs, L4_Word_t request)
{
    return
        L4_InterruptControl
        (
            L4_nilthread,
            L4_InterruptControl_count(mrs) |
            L4_InterruptControl_op(L4_InterruptControl_AcknowledgeIrq) |
            L4_InterruptControl_request(request) |
            L4_InterruptControl_notifybit(0)
        );
}

/*
 * See comments for L4_RegisterInterrupt()
 *
 * The interrupt to acknowledge should be described in message registers.
 * The kernel 
 */
L4_INLINE L4_Word_t
L4_AcknowledgeInterruptOnBehalf(L4_ThreadId_t on_behalf, L4_Word_t mrs, L4_Word_t request)
{
    return
        L4_InterruptControl
        (
            on_behalf,
            L4_InterruptControl_count(mrs) |
            L4_InterruptControl_op(L4_InterruptControl_AcknowledgeIrq) |
            L4_InterruptControl_request(request) |
            L4_InterruptControl_notifybit(0)
        );
}

/*
 * See comments for L4_RegisterInterrupt()
 *
 * The interrupt to acknowledge should be described in message registers
 */
L4_INLINE L4_Word_t
L4_AcknowledgeWaitInterrupt(L4_Word_t mrs, L4_Word_t request)
{
    return
        L4_InterruptControl
        (
            L4_nilthread,
            L4_InterruptControl_count(mrs) |
            L4_InterruptControl_op(L4_InterruptControl_AckWaitIrq) |
            L4_InterruptControl_request(request) |
            L4_InterruptControl_notifybit(0)
        );
}

#endif /* !__L4__INTERRUPT_H__ */

