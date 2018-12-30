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
 * Description: Methods for managing the IA-32 MMU
 */
#ifndef __ARCH_IA32_MMU_H__
#define __ARCH_IA32_MMU_H__

#include <kernel/arch/cpu.h>

class ia32_mmu_t
{
public:
    static void flush_tlb(bool global = false);
    static void flush_tlbent(u32_t addr);
    static void enable_super_pages();
    static void enable_global_pages();
    static void enable_paged_mode();
    static u32_t get_pagefault_address(void);
    static u32_t get_active_pagetable(void);
    static void set_active_pagetable(u32_t pdir);
};

/**
 * Flushes the tlb
 *
 * @param global        specifies whether global TLB entries are also flushed
 */
INLINE void ia32_mmu_t::flush_tlb(bool global)
{
    u32_t dummy, dummy2;
    if (!global)
        __asm__ __volatile__("movl %%cr3, %0\n"
                             "movl %0, %%cr3\n"
                             : "=r" (dummy));
    else
        __asm__ __volatile__("movl      %%cr4, %0       \n"
                             "andl      %2, %0          \n"
                             "movl      %0, %%cr4       \n"
                             "movl      %%cr3, %1       \n"
                             "movl      %1, %%cr3       \n"
                             "orl       %3, %0          \n"
                             "movl      %0, %%cr4       \n"
                             : "=r"(dummy), "=r"(dummy2)
                             : "i" (~IA32_CR4_PGE), "i" (IA32_CR4_PGE));
}

/**
 * Flushes the TLB entry for a linear address
 *
 * @param addr  linear address
 */
INLINE void ia32_mmu_t::flush_tlbent(u32_t addr)
{
    __asm__ __volatile__ ("invlpg (%0)  \n"
                          :
                          :"r" (addr));
}


/**
 * Enables extended page size (4M) support for IA32
 */
INLINE void ia32_mmu_t::enable_super_pages()
{
    ia32_cr4_set(IA32_CR4_PSE);
}

/**
 * Enables global page support for IA32
 */
INLINE void ia32_mmu_t::enable_global_pages()
{
    ia32_cr4_set(IA32_CR4_PGE);
}

/**
 * Enables paged mode for IA32
 */
INLINE void ia32_mmu_t::enable_paged_mode()
{
    asm("mov %0, %%cr0\n"
        "nop;nop;nop\n"
        :
        : "r"(IA32_CR0_PG | IA32_CR0_WP | IA32_CR0_PE)
        );
}

/**
 * @returns the linear address of the last pagefault
 */
INLINE u32_t ia32_mmu_t::get_pagefault_address(void)
{
    register u32_t tmp;

    asm ("movl  %%cr2, %0\n"
         :"=r" (tmp));

    return tmp;
}

/**
 * Get the active page table
 *
 * @returns the physical base address of the currently active page directory
 */
INLINE u32_t ia32_mmu_t::get_active_pagetable(void)
{
    u32_t tmp;

    asm volatile ("movl %%cr3, %0       \n"
                  :"=a" (tmp));

    return tmp;
}

/**
 * Sets the active page table
 *
 * @param pdir  page directory base address (physical)
 */
INLINE void ia32_mmu_t::set_active_pagetable(u32_t pdir)
{
    asm volatile ("mov %0, %%cr3\n"
                  :
                  : "r"(pdir));
}

#endif /* __ARCH_IA32_MMU_H__ */

