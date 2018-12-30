/*
 * Copyright (c) 2003-2005, National ICT Australia (NICTA)
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
 * Description: ARM space_t implementation.
 */

#include <l4.h>
#include <debug.h>                      /* for UNIMPLEMENTED    */
#include <linear_ptab.h>
#include <space.h>              /* space_t              */
#include <tcb.h>
#include <arch/pgent.h>
#include <arch/memory.h>
#include <config.h>
#include <cpu/syscon.h>
#include <arch/special.h>
#include <bitmap.h>
#include <generic/lib.h>
#include <bitmap.h>


DECLARE_KMEM_GROUP(kmem_utcb);

word_t DCACHE_SIZE = 0;
word_t DCACHE_LINE_SIZE = 0;
word_t DCACHE_SETS = 0;
word_t DCACHE_WAYS = 0;
word_t ICACHE_SIZE = 0;
word_t ICACHE_LINE_SIZE = 0;
word_t ICACHE_SETS = 0;
word_t ICACHE_WAYS = 0;

extern bitmap_t ipc_bitmap_ids;

/**
 * reads a word from a given physical address, uses a remap window and
 * maps a 1MB page for the access
 *
 * @param vaddr         virtual address (if caches need to be flushed)
 * @param paddr         physical address to read from
 * @return the value at the given address
 */
word_t generic_space_t::readmem_phys (addr_t vaddr, addr_t paddr)
{
    word_t data;
    space_t *space;
    addr_t phys_page;
    bool r;

#ifdef CONFIG_ENABLE_FASS
    /* on fass the CPD (kernel space) is always the current pagetable/space */
    space = get_kernel_space();
#else
    space = get_current_tcb()->get_space();
    if (!space) {
        space = get_kernel_space();/*get_current_tcb()->get_space();*/
    }
#endif

#if CONFIG_ARM_VER < 6
    arm_cache::cache_clean_dlines(vaddr, sizeof(word_t));
#else
    arm_cache::cache_flush();
#endif

    phys_page = (addr_t)((word_t)paddr & (~(PAGE_SIZE_1M-1)));

#ifdef CONFIG_USE_L2_CACHE
    /* if we have l2 cache and it is enabled, have to map it l2 cachable. */
    r = space->add_mapping((addr_t) PHYSMAPPING_VADDR,
            phys_page, pgent_t::size_1m, space_t::read_only, true, NC_WB );
#else
    r = space->add_mapping((addr_t) PHYSMAPPING_VADDR,
            phys_page, pgent_t::size_1m, space_t::read_only, true, uncached );
#endif
    ASSERT(ALWAYS, r == true);

    space->flush_tlbent_local (get_current_space(), (addr_t)PHYSMAPPING_VADDR, PAGE_BITS_1M);

    data = *(word_t*)(PHYSMAPPING_VADDR + ((word_t)paddr & (PAGE_SIZE_1M-1)));

    return data;
}

bool space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
                rwx_e rwx, bool kernel, memattrib_e attrib)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    pgent_t *pg = this->pgent(0);
    pg = pg->next(this, pgsize, page_table_index(pgsize, vaddr));

#ifdef CONFIG_ENABLE_FASS
    if (is_utcb_area(vaddr) && (this != get_kernel_space())) {
        /* Special case for UTCB mappings */
        //TRACEF("Utcb mapping at %p\n", vaddr);
        pgsize = pgent_t::size_1m;
        pg = this->pgent_utcb();
    }
#endif

    //TRACEF("vaddr = %p paddr = %p %d\n", vaddr, paddr, size);

    /*
     * Lookup mapping
     */
    while (1) {
        if ( pgsize == size )
            break;

        // Create subtree
        if ( !pg->is_valid( this, pgsize ) ) {
            if (!pg->make_subtree( this, pgsize, kernel ))
                return false;
        }

        pg = pg->subtree( this, pgsize )->next
            ( this, pgsize-1, page_table_index( pgsize-1, vaddr ) );
        pgsize--;
    }

    bool r = (word_t)rwx & 4;
    bool w = (word_t)rwx & 2;
    bool x = (word_t)rwx & 1;

    pg->set_entry(this, pgsize, paddr, r, w, x, kernel, attrib);
    arm_cache::cache_drain_write_buffer();

    return true;
}

