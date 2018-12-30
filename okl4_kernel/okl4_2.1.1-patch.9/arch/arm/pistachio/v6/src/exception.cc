/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   ARMv6 syscall and page fault handlers
 */

#include <l4.h>
#include <debug.h>
#include <space.h>
#include <tcb.h>
#include <tracebuffer.h>
#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT(ARM_PAGE_FAULT);

extern "C" NORETURN void send_exception_ipc (word_t exc_no, word_t exc_code,
        arm_irq_context_t *context, continuation_t continuation);

/*
 * ARM data/instruction abort handler
 *
 * This function handles ARM pagefaults including domain faults,
 * data and prefetch aborts and alignment faults.
 */
extern "C" void arm_memory_abort(word_t fault_status, addr_t fault_addr,
        arm_irq_context_t *context)
{
    /* Note, fault_status = 0x8000000X for execute fault */
    word_t fs;
    continuation_t continuation = ASM_CONTINUATION;
    space_t *space = get_current_tcb()->get_space();

    bool is_kernel = ((context->cpsr & CPSR_MODE_MASK) != CPSR_USER_MODE);
    bool data_abt;

    fs = ((fault_status & 0x400) >> 6) | (fault_status & 0xf);

    if (space == NULL)
        space = get_kernel_space();

    TRACEPOINT(ARM_PAGE_FAULT,
        printf("pf @ %p, pc = %p, tcb = %p, fs = %x\n", fault_addr,
            (addr_t)context->pc, get_current_tcb(), fs)
        );

    if ((fault_status & 0x80000000))
        data_abt = false;
    else
        data_abt = true;

    switch(fs) {
        case 3: case 5: case 6: case 7: case 13: case 15:
            break;
        default:    /* Unhandled exception */
        {
            tcb_t * current = get_current_tcb();
            bool is_kernel = ((context->cpsr & CPSR_MODE_MASK) != CPSR_USER_MODE);

            if (is_kernel) {
                panic("kernel raised data abort exception: @ %p, ip = %p, fs = %ld\n",
                        fault_addr, (addr_t)context->pc, fs);
            }

            current->arch.misc.exception.exception_continuation = ASM_CONTINUATION;
            current->arch.misc.exception.exception_context = context;
            // XXX encode DFSR bit 11 (write-flag) in the message?
            send_exception_ipc(0x100 + fs, (word_t)fault_addr,
                    context, continuation);
        }
    }

    space_t::access_e access;

    access = data_abt ? ((fault_status >> 11) & 1 ?
                            space_t::write : space_t::read)
                        : (space_t::execute);
    if (access == space_t::read)
    {
        /* the hardware reports a read fault for swp instruction
         * (at least) so lookup to see if there is a mapping
         * already, if so this is a write fault
         */

        /* FIXME: optimize this lookup */
        if (space->lookup_mapping(fault_addr, NULL, NULL))
        {
            access = space_t::write;
        }
    }

    space->handle_pagefault(fault_addr, (addr_t)context->pc, access, is_kernel,
            continuation);
}

