/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
/*
 * Description:   exception handling
 */

#include <l4.h>
#include <debug.h>
#include <kdb/tracepoints.h>
#include <kdb/console.h>
#include <arch/traps.h>
#include <arch/trapgate.h>
#include <arch/hwirq.h>
#include <tcb.h>
#include <space.h>
#include <schedule.h>

DECLARE_TRACEPOINT (IA32_GP);
DECLARE_TRACEPOINT (IA32_UD);
DECLARE_TRACEPOINT (IA32_NOMATH);
DECLARE_TRACEPOINT (IA32_SEGRELOAD);
DECLARE_TRACEPOINT (EXCEPTION_IPC);

static void finish_exception_ipc(void);

static void
send_exception_ipc(ia32_exceptionframe_t * frame, word_t exception, continuation_t continuation)
{
    PROFILE_START(send_ex_ipc);
    tcb_t * current = get_current_tcb();

    if (current->get_exception_handler() == NULL) {
        TRACEF("unhandled exception: %T, halting\n",
                current->get_global_id().get_raw());
        get_current_scheduler()->deactivate_sched(current,
                thread_state_t::halted, current,
                current->arch.exception_continuation,
                scheduler_t::sched_default);
    }

    current->arch.exception_continuation = continuation;

    TRACEPOINT_TB (EXCEPTION_IPC,
                   printf ("exception ipc at %x, %T (%p) -> %T \n", frame->eip, current->get_global_id().get_raw(),
                       current, current->get_exception_handler()),
                   "exception_ipc at %x (current=%p)", frame->eip, (u32_t)current);

    current->arch.exc_code = exception + 2;

    /* setup exception IPC */
    msg_tag_t tag;

    // save message registers
    current->save_state(1);

    tag.set(12, -5 << 4, true, true);
    current->set_mr(0, tag.raw);

    current->do_ipc(current->get_exception_handler()->get_global_id(),
        current->get_exception_handler()->get_global_id(), finish_exception_ipc);
}

static void finish_exception_ipc(void)
{
    tcb_t * current = get_current_tcb();
    msg_tag_t tag;
    tag.raw = current->get_mr(0);

    if (tag.is_error()) {
        enter_kdebug("exception delivery error");
    }

    current->restore_state(1);
    current->clear_exception_ipc();

    PROFILE_STOP(send_ex_ipc);

    ACTIVATE_CONTINUATION(current->arch.exception_continuation);
}


IA32_EXC_NO_ERRORCODE(exc_catch, -1)
{
    continuation_t cont = ASM_CONTINUATION;
    send_exception_ipc(frame, (word_t)IA32_EXC_CATCH_COMMON, cont);
}

IA32_EXC_NO_ERRORCODE(exc_invalid_opcode, IA32_EXC_INVALIDOPCODE)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();
    addr_t addr = (addr_t)frame->eip;

    TRACEPOINT_TB (IA32_UD,
            printf ("invalid opcode by %t at IP %p\n", current, addr),
            "ia32_ud at %x (current=%x)", (u32_t)addr, (u32_t)current);

    /* instruction emulation, only in user area! */
    if (space->is_user_area(addr)) {
        switch(space->get_from_user(addr)) {
        case 0xf0: /* lock prefix */
            /* fall trough */
        default:
            printf("invalid opcode  at IP %p\n", addr);
            enter_kdebug("invalid opcode");
        }
    }

    continuation_t cont = ASM_CONTINUATION;
    send_exception_ipc(frame, IA32_EXC_INVALIDOPCODE, cont);
}

IA32_EXC_WITH_ERRORCODE(exc_gp, IA32_EXC_GENERAL_PROTECTION)
{
    continuation_t cont = ASM_CONTINUATION;

    PROFILE_START(exception_gp);

    TRACEPOINT_TB(IA32_GP,
                  printf("general protection fault @ %p, error: %x\n",
                         frame->eip, frame->error),
                  "ia32_gp at %x (error=%d)",
                  frame->eip, frame->error);

    PROFILE_STOP(exception_gp);
    send_exception_ipc(frame, IA32_EXC_GENERAL_PROTECTION, cont);
}

IA32_EXC_NO_ERRORCODE(exc_nomath_coproc, IA32_EXC_NOMATH_COPROC)
{
    tcb_t * current = get_current_tcb();

    TRACEPOINT(IA32_NOMATH,
               printf("IA32_NOMATH %t @ %p\n", current, frame->eip));

    current->resources.ia32_no_math_exception(current);
}

IA32_EXC_NO_ERRORCODE(exc_fpu_fault, IA32_EXC_FPU_FAULT)
{
    continuation_t cont = ASM_CONTINUATION;
    send_exception_ipc(frame, IA32_EXC_FPU_FAULT, cont);
}

IA32_EXC_NO_ERRORCODE(exc_simd_fault, IA32_EXC_SIMD_FAULT)
{
    continuation_t cont = ASM_CONTINUATION;
    send_exception_ipc(frame, IA32_EXC_SIMD_FAULT, cont);
}

IA32_EXC_WITH_ERRORCODE(exc_catch_common, IA32_EXC_CATCH_COMMON)
{
    enter_kdebug("Catchall exception");
}

/* Interface to allow sysenter in trap.spp to call send_exception_ipc */
#define IA32_OKLINUX_GP 13
#define IA32_OKLINUX_SYSENTER 130

extern "C" void __attribute__ ((regparm (1))) sysenter_exception_ipc(ia32_exceptionframe_t * frame)
{
    continuation_t cont = ASM_CONTINUATION;
    send_exception_ipc(frame, IA32_OKLINUX_SYSENTER, cont);
}
