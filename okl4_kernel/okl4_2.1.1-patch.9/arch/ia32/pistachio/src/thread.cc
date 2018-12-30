/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Description:
 */
#include <l4.h>
#include <arch/config.h>
#include <arch/ldt.h>
#include <arch/trapgate.h>
#include <thread.h>
#include <tcb.h>
#include <arch/tss.h>
#include <schedule.h>
#include <tcb_layout.h>
#include <ktcb_layout.h>
#include <profile.h>


void
iret_path(void)
{
    EXCEPTION_ENTRY_COMMON();

    __asm__ (
        ".globl initial_to_user         \n"
        ".globl ia32_return_to_user     \n"
        ".globl ia32_exception_return   \n"
        "initial_to_user:               \n"
        "ia32_return_to_user:           \n"
        /* get current tcb */
        "orl    %[stack_mask], %%esp    \n"
        "movl   (%%esp), %%esp          \n"
        "addl   %[ofs_context], %%esp   \n"
        "1:                             \n"
        /* skip over reason */
        "addl   $4, %%esp               \n"
        /* restore segment registers */
        "popl   %%gs                    \n"
        "popl   %%fs                    \n"
        "popl   %%es                    \n"
        "popl   %%ds                    \n"
        /* restore gprs */
        "popa                           \n"
        /* skip over error */
        "addl   $4, %%esp               \n"
        "iret                           \n"
        "                               \n"
        "ia32_exception_return:         \n"
        /* check if we need to switch back to tcb context  */
        "movl   %%esp, %%ebx            \n"
        "andl   %[stack_test], %%ebx    \n"
        "cmp    %[stack_test], %%ebx    \n"
        "jne    1b                      \n"
        "orl    %[stack_mask], %%esp    \n"
        "movl   (%%esp), %%esp          \n"
        "addl   %[ofs_context], %%esp   \n"
        "jmp     1b                     \n"
        :
        :
          [ofs_context] "i" (OFS_TCB_ARCH + OFS_ARCH_KTCB_CONTEXT),
          [stack_mask] "i" (STACK_TOP),
          [stack_test] "i" (STACK_TEST)
        );
}

/**
 * we set up a frame which looks like a normal pagefault frame
 * to allow to ex-regs a newly created thread
 */
void
tcb_t::create_startup_stack(void (* func)(void))
{
    init_stack();
    arch.context.esi = 0;
    arch.context.cs = IA32_UCS;
    arch.context.eip = 0x87654321;
    arch.context.ss = IA32_UDS;
    arch.context.esp = 0;
    arch.context.eflags = IA32_USER_FLAGS;
    arch.context.cs = IA32_UCS;
    arch.context.eip = 0;
    arch.context.error = 0;
    arch.context.ds = IA32_UDS;
    arch.context.es = IA32_UDS;
    arch.context.fs = IA32_UDS;
    arch.context.gs = IA32_UTCB;

    notify(func);
}

void thread_switch_helper(void);

/* The previously running TCB. */
static tcb_t * last_active_tcb = NULL;

void
switch_from(tcb_t * current, continuation_t continuation)
{
    /* Save return continuation. */
    current->cont = continuation;

    /* Save any resources held by the current thread. */
    if (EXPECT_FALSE(current->resource_bits)) {
        current->resources.save(current);
    }

    /* Stop timing this TCB. */
    PROFILE_SWITCH_FROM(current);

    /* Cache the previously running TCB. */
    last_active_tcb = current;
}

void
switch_to(tcb_t *dest, tcb_t * schedule)
{
    extern ia32_tss_t tss;

    /* Update the global schedule variable. */
    if (schedule != NULL) {
        set_active_schedule(schedule);
    }

    /* Load any resources held by the destination thread. */
    if (EXPECT_FALSE(dest->resource_bits)) {
        dest->resources.load(dest);
    }

    /* If we are switching to a different TCB, we need to update
     * the processor's state. */
    if (dest != last_active_tcb) {
        /* Modify stack in TSS. */
        tss.set_esp0((u32_t)((&(dest->arch.context)) + 1));

        /* Modify current TCB pointer (top word of stack) */
        *((tcb_t **) stack_top) = dest;

        /* Save information about the context switch to the tracebuffer. */

        /*lint -e641 */
        TBUF_RECORD(SWITCH_TO, "switch %t => %t",
                (word_t)last_active_tcb, (word_t)dest);

        /* Update LDT base in GDTR and reload LDTR */
        update_ldt(dest);

        /* update utcb */
        *(u32_t *) ((u32_t) MYUTCB_MAPPING) = dest->utcb_location;

        /* Start account time to the new TCB. */
        PROFILE_SWITCH_TO(dest);

        space_t *space = dest->get_space();

        if (EXPECT_TRUE(space != NULL)) {
            /* Activate the destination's address space. */
            space->activate(dest);
        }


    }

    /* Jump to the destination. */
    ACTIVATE_CONTINUATION(
            dest->resource_bits ? thread_switch_helper : dest->cont);
}

CONTINUATION_FUNCTION(finish_do_ipc)
{
    ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(get_current_tcb())->do_ipc_continuation);
}

