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
 * Description:   ia32-specific space implementation
 */
#ifndef __IA32__SPACE_H__
#define __IA32__SPACE_H__

#include <kernel/types.h>

#include <kernel/debug.h>
#include <kernel/fpage.h>
#include <kernel/thread.h>
#include <kernel/queueing.h>
#include <kernel/map.h>
#include <kernel/arch/ptab.h>
#include <kernel/arch/pgent.h>
#include <kernel/arch/config.h>
#include <kernel/clist.h>

#define HAVE_ARCH_FREE_SPACE

class space_t : public generic_space_t
{
public:
    enum rwx_e {
        read_only       = 0x1,
        read_write      = 0x3,
        read_execute    = read_only,
        read_write_ex   = read_write,
    };

    word_t space_control (word_t ctrl);
    u8_t get_from_user(addr_t addr);
    void remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
                         word_t len, rwx_e rwx, bool kernel, bool global);
    bool add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
                          rwx_e rwx, bool kernel, bool global, memattrib_e attrib);
    bool add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
                          rwx_e rwx, bool kernel, bool global)
    {
        return add_mapping (vaddr, paddr, size, rwx, kernel, global, cached);
    }
};




/**********************************************************************
 *
 *                      inline functions
 *
 **********************************************************************/

INLINE bool generic_space_t::is_user_area(addr_t addr)
{
    return (addr >= (addr_t)USER_AREA_START &&
            addr < (addr_t)USER_AREA_END);
}


INLINE u8_t space_t::get_from_user(addr_t addr)
{
    return *(u8_t *) (addr);
}

INLINE pgent_t * generic_space_t::pgent (word_t num, word_t cpu)
{
#ifdef CONFIG_MDOMAINS
    return pdir[cpu]->next (this, pgent_t::size_4m, num);
#else
    return pdir->next (this, pgent_t::size_4m, num);
#endif
}

#ifndef CONFIG_MDOMAINS
/**
 * Flush complete TLB
 */
INLINE void generic_space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace) {
        ia32_mmu_t::flush_tlb (false);
    }
}

/**
 * Flush a specific TLB entry
 * @param addr  virtual address of TLB entry
 */
INLINE void generic_space_t::flush_tlbent_local (space_t * curspace,
        addr_t addr, word_t log2size)
{
    if (this == curspace) {
        ia32_mmu_t::flush_tlbent ((u32_t) addr);
    }
}

#else

INLINE void set_active_space (scheduler_domain_t cpu, space_t * space)
{
    /* glue/space.cc */
    extern struct {
        space_t * space;
        char __pad [CACHE_LINE_SIZE - sizeof(space_t*)];
    } active_cpu_space[CONFIG_NUM_DOMAINS];

    if (space)
        active_cpu_space[cpu].space = space;
}
#endif /* !CONFIG_MDOMAINS */


/**********************************************************************
 *
 *                 global function declarations
 *
 **********************************************************************/

/**
 * converts the current page table into a space_t
 */
INLINE space_t * ptab_to_space(u32_t ptab, scheduler_domain_t cpu = 0)
{
#if defined(CONFIG_MDOMAINS)
    return phys_to_virt((space_t*)(ptab - (cpu * IA32_PAGE_SIZE)));
#else
    return phys_to_virt((space_t*)(ptab));
#endif
}

INLINE bool generic_space_t::does_tlbflush_pay (word_t log2size)
{
    return log2size >= 32;
}
#endif /* !__IA32__SPACE_H__ */
