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
 * Description:   MIPS Virtual Address Space implementation
 */                

#include <l4.h>
#include <kmemory.h>

#include <space.h>              /* space_t              */
#include <kernelinterface.h>
#include <tcb.h>

#include <arch/pgent.h>
#include <linear_ptab.h>
#include "bitmap.h"
#include <generic/lib.h> /* memset */

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_utcb);

#define PGSIZE_KIP      (pgent_t::size_4k)
#define PGSIZE_UTCB     (pgent_t::size_4k)

asid_cache_t asid_cache UNIT("cpulocal");
extern bitmap_t ipc_bitmap_ids;

void SECTION(".init.memory") generic_space_t::init_kernel_mappings()
{
    /* No kernel mapping is needed */
}

INLINE word_t pagedir_idx (addr_t addr)
{
    return page_table_index (pgent_t::size_max, addr);
}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init.memory") init_kernel_space()
{
    bool alloc;

    /* init the ipc control allocation bitmaps. */
    bitmap_init(&ipc_bitmap_ids, CONFIG_MAX_SPACES, 1);

    space_t * kernel_space = get_kernel_space();
    ASSERT(ALWAYS, kernel_space);

    /* allocate kernel page table */
    alloc = kernel_space->allocate_page_directory();
    ASSERT(ALWAYS, alloc);

    kernel_space->get_asid()->init();
    kernel_space->init_kernel_mappings();

    kernel_space->set_space_id(spaceid_t::kernelspace());

    kernel_space->enqueue_spaces();
}

/**
 * initialize a space
 *
 * @param utcb_area     fpage describing location of UTCB area
 * @param kip_area      fpage describing location of KIP
 */
bool generic_space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    return ((space_t*)this)->add_mapping(kip_area.get_base(),
                                         virt_to_phys((addr_t)get_kip ()),
                                         PGSIZE_KIP, false, false);
}

/**
 * Clean up a Space
 */
void generic_space_t::arch_free()
{
    asid_t *asid = ((space_t*)this)->get_asid();

    asid->release();
}

/**
 * Allocate a UTCB
 * @param tcb   Owner of the utcb
 *
 */
utcb_t * generic_space_t::allocate_utcb (tcb_t * tcb)
{
    ASSERT (DEBUG, tcb);
    addr_t utcb = (addr_t) tcb->get_utcb_location ();

    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (lookup_mapping ((addr_t) utcb, &pg, &pgsize))
    {
        addr_t kaddr = addr_mask (pg->address(this, pgsize),
                                  ~page_mask (pgsize));
        utcb_t *retv = (utcb_t *) phys_to_virt
            (addr_offset (kaddr, (word_t) utcb & page_mask (pgsize)));
        (void)memset(retv, 0, sizeof(*retv));
        return retv;
    }

#define MIPS_ALIGN    ((ICACHE_SIZE/MIPS_PAGE_SIZE/CACHE_WAYS - 1UL) << MIPS_PAGE_BITS)

    addr_t page = kmem.alloc_aligned (kmem_utcb, page_size (PGSIZE_UTCB),
                                        (word_t) utcb, MIPS_ALIGN);
    if (page == NULL)
            return NULL;
    cache_t::flush_cache_page((word_t)page, PGSIZE_UTCB);

    if (((space_t*)this)->add_mapping((addr_t) utcb, virt_to_phys(page),
                    PGSIZE_UTCB, true, false) == false)
        return NULL;

    return (utcb_t *)
        addr_offset (page, addr_mask (utcb, page_size (PGSIZE_UTCB) - 1));
}

void generic_space_t::free_utcb(utcb_t * utcb)
{
    /* do nothing, on some archs, utcb isn't allocated by kernel,
     * kernel isn't responsible for removing utcb mapping from space
     * when deleting or switching thread from the space,
     * the mapping will only be removed when space in deleted.
     */
}

void generic_space_t::activate(tcb_t *tcb)
{
    space_t *new_space = (space_t *)this;
    K_CURRENT_CLIST = (word_t)new_space->get_clist();
#if defined(L4_32BIT)
    extern space_t* K_SAVED_PGTABLE;
    __asm__ __volatile__ (
        "mtc0       %[asid], "STR(CP0_ENTRYHI)"     \n" /* set new ASID */
        "sw         %[space], 0(%[saved_pgtable])   \n" /* save current PgTable */
        :
        : [asid] "r" (new_space->get_asid()->get(new_space)),
          [space] "r" ((word_t)new_space),
          [saved_pgtable] "r" (&K_SAVED_PGTABLE)
        : "$1", "$31"
        );
#elif defined(L4_64BIT)
    __asm__ __volatile__ (
        "dmtc0      %[asid], "STR(CP0_ENTRYHI)"     \n" /* set new ASID */
        "dsll       $1, %[space], 32                \n"
        "dmtc0      $1, "STR(CP0_CONTEXT)"          \n" /* save current PgTable */
        : 
        : [asid] "r" (new_space->get_asid()->get(new_space)),
          [space] "r" ((word_t)new_space)  
        : "$1", "$31", "memory"
        );
#endif
}

DEFINE_SPINLOCK (tlb_lock);     // XXX
/**
 * Handle a XTLB Refill
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
extern "C" void handle_xtlb_miss (addr_t faddr, mips_irq_context_t * frame)
{
    space_t * space = get_saved_pagetable();
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (space == NULL)
        space = get_kernel_space();

    // TRACEF("(%016lx, %p) %d\n", frame->badvaddr, space, get_current_cpu());
    // TRACEF("(%08x, %08x, %p)\n", faddr, frame->epc, space);

    // Check if mapping exist in page table
    if (space->lookup_mapping (faddr, &pg, &pgsize) && (pgsize == pgent_t::size_min))
    {
        /* matthewc HACK: fill out both halves of the pair straight away
         * or things can get nasty with nested faults (TCB case).
         */
        /*lint -e529 temp hidden by ASM */
        word_t temp, mapsize = (1 << MIPS_PAGE_BITS);
        bool odd = (word_t)faddr & mapsize;
        pgent_t * buddy, * pg1, * pg2;
        buddy = odd ? (pg-1) : (pg+1);

        if (buddy->is_valid(space, pgsize) && (!buddy->is_subtree(space, pgsize)))
        {
            /*lint -esym(550, pg1, pg2) */
            pg1 = odd ? buddy : pg;
            pg2 = odd ? pg : buddy;
            faddr = MIPS_TCB_CLEAR(faddr);
            __asm__ __volatile__ (
                "   mtc0       %0,"STR(CP0_PAGEMASK)"\n\t"
                : : "r" ((0x2 << (12))-1)
            );
//printf("buddy insert: %p (%p, %p)\n", (((word_t)faddr>>13)<<13) | space->get_asid()->get(space),
//              pg1->translation()->get_raw(), pg2->translation()->get_raw());
//printf("      insert: %p (%p, %p)\n", (((word_t)faddr>>13)<<13) | space->get_asid()->get(space),
//              pg1->translation()->phys_addr(), pg2->translation()->phys_addr());
            __asm__ __volatile__ (
                MFC0"   %0,"STR(CP0_ENTRYHI)"   \n\t"
                "  nop;                         \n\t"
                MTC0"   %1,"STR(CP0_ENTRYHI)"   \n\t"
                MTC0"   %2,"STR(CP0_ENTRYLO0)"  \n\t"
                MTC0"   %3,"STR(CP0_ENTRYLO1)"  \n\t"
                "nop;nop;nop;                   \n\t"
                "tlbwr                          \n\t"
                "nop;nop;nop;                   \n\t"
                MTC0"   %0,"STR(CP0_ENTRYHI)"   \n\t"
                : "=r" (temp)
                : "r" ((((word_t)faddr>>13)<<13) | space->get_asid()->get(space)),
                  "r" (pg1->translation()->get_raw()), "r" (pg2->translation()->get_raw())
            );
            return;
        }
        pg->translation ()->put_tc (faddr, page_shift (pgsize),
                                        space->get_asid()->get(space));
        return;
    }

    while (1) {
        __asm__ __volatile__ (
            "move       $29, %0     \n\r"
            "j  _mips_xtlb_fall_through \n\r"
            :: "r" (frame)
        );
    }
}

/**
 * Handle a TLB and Software TLB Cache miss
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
static void finish_stlb_miss (void);

extern "C" void handle_stlb_miss (addr_t faddr, mips_irq_context_t * frame)
{
    space_t * space = get_current_tcb ()->get_space();
    pgent_t::pgsize_e pgsize;
    space_t::access_e access;
    pgent_t * pg;
    bool kernel, twice;
    tcb_t * current = get_current_tcb();

    if (space == NULL)
        space = get_kernel_space();

    // TRACEF("(%016lx, %p) %d\n", frame->badvaddr, space, get_current_cpu());

    access = (frame->cause & CAUSE_EXCCODE) == (3<<2) ? /* TLBS (write) */
            space_t::write : space_t::read;

    twice = false;

    while(1)
    {
        // Check if mapping exist in page table
        if (space->lookup_mapping (faddr, &pg, &pgsize))
        {
            if (((access == space_t::write) && pg->is_writable(space, pgsize)) ||
                    ((access == space_t::read) && pg->is_readable(space, pgsize)) )
            {
                pg->translation ()->put_tc (faddr, page_shift (pgsize),
                        space->get_asid()->get(space));
                return;
            }
        }
        if (twice) return;

        if (space->is_user_area(faddr))
            kernel = frame->status & ST_KSU ? false : true;
        else
            kernel = true;      /* User-space will cause address error */

        if (EXPECT_TRUE(!kernel))
        {
            current->arch.access = access;
            current->arch.faddr = faddr;
            current->arch.stlb_continuation = ASM_CONTINUATION;
        }
        space->handle_pagefault (faddr, (addr_t)frame->epc, access, kernel, (continuation_t)finish_stlb_miss);
        /* if user (!kernel), this will return to finish_stlb_miss */

        twice = true;
    }
}

static void finish_stlb_miss (void)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (space == NULL)
        space = get_kernel_space();

    // Check if mapping exist in page table
    if (space->lookup_mapping (current->arch.faddr, &pg, &pgsize))
    {
        if (((current->arch.access == space_t::write) && pg->is_writable(space, pgsize)) ||
                ((current->arch.access == space_t::read) && pg->is_readable(space, pgsize)) )
        {
            pg->translation ()->put_tc (current->arch.faddr, page_shift (pgsize),
                    space->get_asid()->get(space));
        }
    }
    ACTIVATE_CONTINUATION(current->arch.stlb_continuation);
}       
/**
 * Handle a TLB MOD exception
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
extern "C" void handle_tlb_mod (addr_t faddr, mips_irq_context_t * frame)
{
    space_t * space = get_current_tcb ()->get_space();
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (space == NULL)
        space = get_kernel_space();

    // Check if mapping exist in page table
    if (space->lookup_mapping (faddr, &pg, &pgsize))
    {
        if (pg->is_writable (space, pgsize))
        {
            pg->translation ()->put_tc (faddr, page_shift (pgsize),
                                        space->get_asid()->get(space));
            return;
        }
    }

    space->handle_pagefault (faddr, (addr_t)frame->epc, space_t::write,
            frame->status & ST_KSU ? false : true, ASM_CONTINUATION);
}

/**
 * Add a mapping into this address space
 */
bool space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
                          bool writable, bool kernel)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    pgent_t *pg = this->pgent(0);
    pg = pg->next(this, pgsize, page_table_index(pgsize, vaddr));

    /*
     * Sanity checking on page size
     */

    if (! is_page_size_valid (size))
    {
        printf ("Mapping invalid pagesize (%dKB)\n", page_size (pgsize) >> 10);
        enter_kdebug ("invalid page size");
        return false;
    }

    /*
     * Lookup mapping
     */

    tlb_lock.lock();
    while (pgsize > size)
    {
        if (pg->is_valid (this, pgsize))
        {
            // Sanity check
            if (! pg->is_subtree (this, pgsize))
            {
                printf ("%dKB mapping @ %p space %p already exists.\n",
                        page_size (pgsize) >> 10, vaddr, this);
                enter_kdebug ("mapping exists");

                tlb_lock.unlock();
                return false;
            }
        }
        else
        {
            // Create subtree
            if (pg->make_subtree (this, pgsize, kernel) == false)
                return false;
        }

        pg = pg->subtree (this, pgsize)->next
            (this, pgsize-1, page_table_index (pgsize-1, vaddr));
        pgsize--;
    }

    /*
     * Modify page table
     */

    pg->set_entry (this, pgsize, paddr, true, writable, true, kernel);
    pg->translation ()->set (l4default, true, writable, kernel, paddr);
    tlb_lock.unlock();

    /*
     * Insert translation into TLB
     */

    pg->translation ()->put_tc (vaddr, page_shift (pgsize), this->get_asid()->get(this));
    return true;
}

#define PGTABLE_SIZE        ((1ULL << PGTABLE_TOP_BITS)*sizeof(word_t))

/*
 * Allocate top level pagetable object
 */
bool generic_space_t::allocate_page_directory()
{
    pdir = (pgent_t *)kmem.alloc(kmem_pgtab, PGTABLE_SIZE, true);
    return pdir != NULL;
}

void generic_space_t::free_page_directory()
{
    kmem.free(kmem_pgtab, pdir, PGTABLE_SIZE);
    pdir = NULL;
}

void generic_space_t::flush_tlb(space_t * curspace)
{
    asid_t * asid = ((space_t*)this)->get_asid();

    /* XXX we need to flush range here if cache has aliases!! */

    if (asid->is_valid()) {
        flush_asid (asid->value());
    }
}

void generic_space_t::flush_tlbent_local(space_t * curspace, addr_t vaddr, word_t log2size)
{
    asid_t * asid = ((space_t*)this)->get_asid();

    /* XXX we need to flush range here if cache has aliases!! */

    if (asid->is_valid()) {
        purge_tc (vaddr, log2size, asid->value());
    }
}

