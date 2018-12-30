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
 * Description:   v4 specific idt implementation
 */

#include <l4.h>
#include <arch/idt.h>
#include <arch/sysdesc.h>

/* trap definition */
#include <arch/traps.h>

#include <arch/traphandler.h>
#include <arch/syscalls.h>

#include <debug.h>

/**
 * idt: the global IDT (see: IA32 Vol 3)
 *
 * FIXME: how did this ever work on SMP?
 */
idt_t idt UNIT("ia32.idt");
 
static u64_t exc_catch_all[IDT_SIZE] UNIT("ia32.exc_all");

extern "C" void exc_user_timer(void);

void SECTION(SEC_INIT)
    idt_t::add_int_gate(word_t index, void (*address)())
{
    ASSERT(NORMAL, index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::interrupt, 0);
}


void SECTION(SEC_INIT)
    idt_t::add_syscall_gate(word_t index, void (*address)())
{
    ASSERT(NORMAL, index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::interrupt, 3);
}

void SECTION(SEC_INIT)
    idt_t::add_trap_gate(word_t index, void (*address)())
{
    ASSERT(NORMAL, index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::trap, 0);
}

/**
 * idt_t::activate: activates the previously set up IDT
 */
void SECTION(SEC_INIT_CPU)
    idt_t::activate()
{
    /*lint -e529 idt_desc usage is hidden in __asm__ */
    ia32_sysdesc_t idt_desc = {sizeof(idt_t), (u32_t) descriptors, 0};
    __asm__ __volatile__ ("lidt %0"
                          :
                          : "m"(idt_desc)
                          );
}

void idt_t::init_idt_t(void)
{
    for (int i=0;i<IDT_SIZE;i++)
    {
        /*
         * XXX: Synthesize call to exc_catch_common
         *
         * idt
         * exc_catch_all[IDT_SIZE]
         * exc_catch_common
         *
         * e8 = Near call with 4 byte offset (5 byte)
         *
         */
        // exc_catch_all[i] = ( (sizeof(exc_catch_all) - i * sizeof(u64_t) - 5) << 8) | 0xe8;

        u64_t instr = 0;
        u32_t addr, inst2;

        /* encode "int $3" trap in case of return */
        /* XXX: doesnt work since we iret from the handler? */
        instr <<= 8;
        instr |= (u8_t) 0xcc;  /* 0xcc = int3 */

        /** compute relative address to jump to **/
        /* first, address of following instruction */
        addr = ((u32_t)&exc_catch_all[i]) + 5;
        inst2 = addr;
        /* second, address of branch target */
        addr = ((u32_t)&exc_catch_common) - inst2;
        instr <<= 32 ;
        instr |= (u64_t) addr;

        /* add the opcode */
        instr <<= 8;
        instr |= (u8_t) 0xe8;  /* 0xe8 = call */

        /* set it and setup the gate */
        exc_catch_all[i] = instr;
        /*lint -e611 We are happy to cast from fn pointer to
         * object pointer here
         */
        add_int_gate(i, (func_exc) &exc_catch_all[i]);
    }

    /* setup the exception gates */
    add_int_gate(IA32_EXC_DIVIDE_ERROR, exc_catch);
    add_int_gate(IA32_EXC_DEBUG, exc_debug);
    add_int_gate(IA32_EXC_NMI, exc_nmi);
    add_syscall_gate(IA32_EXC_BREAKPOINT, exc_breakpoint);
    add_int_gate(IA32_EXC_OVERFLOW, exc_catch);
    add_int_gate(IA32_EXC_BOUNDRANGE, exc_catch);
    add_int_gate(IA32_EXC_INVALIDOPCODE, exc_invalid_opcode);
    add_int_gate(IA32_EXC_NOMATH_COPROC, exc_nomath_coproc);
    add_int_gate(IA32_EXC_DOUBLEFAULT, exc_catch);
    add_int_gate(IA32_EXC_COPSEG_OVERRUN, exc_catch);
    add_int_gate(IA32_EXC_INVALID_TSS, exc_catch);
    add_int_gate(IA32_EXC_SEGMENT_NOT_PRESENT, exc_catch);
    add_int_gate(IA32_EXC_STACKSEG_FAULT, exc_catch);
    add_int_gate(IA32_EXC_GENERAL_PROTECTION, exc_gp);
    add_int_gate(IA32_EXC_PAGEFAULT, exc_pagefault);
    // 15 reserved
    add_int_gate(IA32_EXC_FPU_FAULT, exc_fpu_fault);
    add_int_gate(IA32_EXC_ALIGNEMENT_CHECK, exc_catch);
    add_int_gate(IA32_EXC_MACHINE_CHECK, exc_catch);
    add_int_gate(IA32_EXC_SIMD_FAULT, exc_simd_fault);

    // syscalls
    add_syscall_gate(0x32, exc_user_timer);
}
