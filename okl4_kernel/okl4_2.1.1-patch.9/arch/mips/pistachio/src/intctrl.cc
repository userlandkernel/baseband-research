/*
 * Copyright (c) 2002-2003, University of New South Wales
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
 * Description:   Implementation of interrupt control functionality 
 */                

#include <l4.h>
#include <tcb.h>
#include <interrupt.h>
#include <arch/intctrl.h>
#include <arch/mips_cpu.h>
#include <arch/platform.h>
#include <plat/interrupt.h>

word_t exception_handlers[32];

/**
 * Setup MIPS exception vector jump table
 */
static void SECTION (".init")
setup_exception_vectors()
{
    extern void (*_mips_interrupt);
    extern void (*_mips64_tlb_mod);
    extern void (*_mips_stlb_miss);
    extern void (*_mips_l4syscall);

    mips_register_exception_handler(0, &_mips_interrupt);
    mips_register_exception_handler(1, &_mips64_tlb_mod);

    mips_register_exception_handler(2, &_mips_stlb_miss);
    mips_register_exception_handler(3, &_mips_stlb_miss);

    mips_register_exception_handler(8, &_mips_l4syscall);

    cache_t::flush_cache_all();
}

#if 0// defined(CONFIG_IPC_FASTPATH)
#define __mips_interrupt __mips_interrupt_fp
#endif


#define EXCEPTION_HANDLER_SIZE 0x80
#define EXCEPTION_HANDLER(x) (void *)(KSEG0 + (0x080 * (x)))

/**
 * Setup the MIPS architecture interrupts
 */
void SECTION (".init")
mips_init_intctrl()
{
    unsigned int i;
    /* declare assembly functions */
    extern char __mips_tlb_refill[EXCEPTION_HANDLER_SIZE];
    extern char __mips_xtlb_refill[EXCEPTION_HANDLER_SIZE];
    extern char __mips64_cache_error[EXCEPTION_HANDLER_SIZE];
    extern char __mips_interrupt[EXCEPTION_HANDLER_SIZE];
    extern char __mips64_extra_vector[EXCEPTION_HANDLER_SIZE];
    extern void (*_mips_exception);

#if defined(CONFIG_IPC_FASTPATH)
    TRACE_INIT("init_arch: using FASTPATH\n");
#endif

    mips_cpu::cli();

    /* Copy the MIPS exception vectors to KSEG0 0xFFFFFFFF80000000  */
    memcpy(EXCEPTION_HANDLER(0), &__mips_tlb_refill,
           EXCEPTION_HANDLER_SIZE);
    memcpy(EXCEPTION_HANDLER(1), &__mips_xtlb_refill,
           EXCEPTION_HANDLER_SIZE);
    memcpy(EXCEPTION_HANDLER(2), &__mips64_cache_error,
           EXCEPTION_HANDLER_SIZE);
    memcpy(EXCEPTION_HANDLER(3), &__mips_interrupt,
           EXCEPTION_HANDLER_SIZE);
    /* Some MIPS CPU's have an extra vector for interrupts */
    memcpy(EXCEPTION_HANDLER(4), &__mips64_extra_vector,
           EXCEPTION_HANDLER_SIZE);

    cache_t::flush_cache_all();

    for (i=0; i<32; i++)
        exception_handlers[i] = (word_t)&_mips_exception;

    setup_exception_vectors();
}

void SECTION (".init")
mips_init_local_intctrl()
{
    /* Mask out all interrupts */
    (void) mips_cpu::clear_cp0_status(ST_IM);

    /* Clear BEV: set vector base to 0xFFFFFFFF80000000 */
    (void) mips_cpu::clear_cp0_status(ST_BEV);

    for (word_t i = 0; i < Platform::IRQS; i++) {
        Platform::mask_irq(i);
    }
}

