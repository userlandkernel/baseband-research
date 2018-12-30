/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description: ARMv6 space_t implementation.
 */

#include <l4.h>
#include <space.h>              /* space_t              */
#include <tcb.h>
#include <linear_ptab.h>
#include <bitmap.h>
#include <generic/lib.h>    /* memset */

EXTERN_KMEM_GROUP(kmem_utcb);
EXTERN_KMEM_GROUP(kmem_pgtab);

#if defined(CONFIG_TRUSTZONE)
FEATURESTRING(trustzone);
FEATURESTRING(secure);
#endif

asid_cache_t asid_cache UNIT("cpulocal");

/* The kernel space is statically defined beause it is needed
 * before the virtual memory has been setup or the kernel
 * memory allocator.
 */
ALIGNED(ARM_L1_SIZE) char UNIT("kspace") _kernel_space_pagetable[ARM_L1_SIZE];

void SECTION(".init") init_kernel_space()
{
    TRACE_INIT("Initializing kernel space @ %p...\n\r", get_kernel_space());

    space_t * kspace = get_kernel_space();

    kspace->get_asid()->init();
    kspace->init_kernel_mappings();

    kspace->enqueue_spaces();
}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init") generic_space_t::init_kernel_mappings()
{
    /* Kernel space's mappings already setup in initial root page table */
}

/**
 * initialize a space
 *
 * @param utcb_area     fpage describing location of UTCB area
 */
bool generic_space_t::init (fpage_t utcb_area)
{
    word_t i;
    word_t offset = USER_AREA_SECTIONS;

    this->utcb_area = utcb_area;

    pgent_t *pg_to = pgent(offset);
    pgent_t *pg_from = get_kernel_space()->pgent(offset);

    for (i=0; i < (KERNEL_AREA_SECTIONS); i++)
        *pg_to++ = *pg_from++;
    for (i=0; i < (UNCACHE_AREA_SECTIONS); i++)
        *pg_to++ = *pg_from++;

    pg_to += VAR_AREA_SECTIONS;
    pg_from += VAR_AREA_SECTIONS;

    for (i=0; i < (IO_AREA_SECTIONS + MISC_AREA_SECTIONS); i++)
        *pg_to++ = *pg_from++;

    *pg_to = *pg_from;  /* high_int_vector */

    return true;
}

/**
 * Clean up a Space
 */
void generic_space_t::arch_free()
{
    asid_t *asid = ((space_t *)this)->get_asid();

    asid->release();
}

#define PAGE_COLOR_ALIGN ((DCACHE_SIZE/PAGE_SIZE_4K/DCACHE_WAYS - 1UL) << PAGE_BITS_4K)

/**
 * Allocate a UTCB
 * @param tcb   Owner of the utcb
 */
utcb_t * generic_space_t::allocate_utcb(tcb_t * tcb)
{
    ASSERT (DEBUG, tcb);

    addr_t utcb = (addr_t) tcb->get_utcb_location ();

    /* Try lookup the UTCB page for this utcb */
    word_t section = (word_t)utcb >> PAGE_BITS_1M;

    pgent_t leaf;
    pgent_t level1 = *this->pgent(section);
    bool is_valid = true;

    if (EXPECT_TRUE(level1.is_valid(this, pgent_t::size_1m)))
    {
        if (EXPECT_TRUE(level1.is_subtree(this, pgent_t::size_1m))) {
            leaf = *level1.subtree(this, pgent_t::size_1m)->next(
                    this, UTCB_AREA_PGSIZE,
                    ((word_t)utcb & (PAGE_SIZE_1M-1)) >> UTCB_AREA_PAGEBITS);

            if (leaf.l2.fault.zero == 0)
            {
                is_valid = false;
            }
        } else {
            enter_kdebug("1MB page in UTCB area");
            return (utcb_t *)0;
        }
    } else {
        is_valid = false;
    }

    if (is_valid)
    {
        addr_t kaddr =
            addr_mask (leaf.address(this, UTCB_AREA_PGSIZE),
                    ~page_mask (UTCB_AREA_PGSIZE));
        utcb_t *retv = (utcb_t *)ram_to_virt(
                addr_offset (kaddr, (word_t) utcb & page_mask (UTCB_AREA_PGSIZE)));
        (void)memset(retv, 0, UTCB_SIZE);
        return retv;
    }

    addr_t page = kmem.alloc_aligned (kmem_utcb, page_size (UTCB_AREA_PGSIZE),
            (word_t) utcb, PAGE_COLOR_ALIGN);

    if (page == NULL)
        return NULL;

    if (! ((space_t *)this)->add_mapping((addr_t)addr_align(utcb, UTCB_AREA_PAGESIZE),
            virt_to_ram(page), UTCB_AREA_PGSIZE, space_t::read_write, false))
    {
        get_current_tcb()->set_error_code(ENO_MEM);
        kmem.free(kmem_utcb, page, page_size (UTCB_AREA_PGSIZE));
        return NULL;
    }

    return (utcb_t *)
        addr_offset (page, addr_mask (utcb, page_mask(UTCB_AREA_PGSIZE)));
}

/** 
 * Free a UTCB in this space
 * @param utcb  The utcb
 */
void generic_space_t::free_utcb(utcb_t * utcb)
{
    /* do nothing, since in arm v6, utcb isn't allocated by kernel,
     * kernel isn't responsible for removing utcb mapping from space
     * when delting or switching thread from the space,
     * the mapping will only be removed when space is deleted.
     */
}

/**
 * Set up hardware context to run the tcb in this space.
 */
void generic_space_t::activate(tcb_t *tcb)
{
    USER_UTCB_REF = tcb->get_utcb_location();

    hw_asid_t dest_asid = ((space_t *)this)->get_asid()->get((space_t *)this);
    get_arm_globals()->current_clist = this->get_clist();

    register word_t dest_pt ASM_REG("r10") = (word_t)page_table_to_phys((space_t *)this->pgent(0));

#ifdef CONFIG_ARM_HWWALKER_IN_L2
    dest_pt |= 1 << 3;  // Outter writeback + write allocate
#endif
//    dest_pt |= 1;       // Inner cacheable, non shared
//printf("new pt = %lx, new sp %lx\n", dest_pt, dest->stack);
//printf("*new_sp = %lx\n", *(word_t*)dest->stack);

    /* Flush BTB/BTAC */
    write_cp15_register(C15_cache_con, c5, 0x6, 0x0);
    /* drain write buffer */
    write_cp15_register(C15_cache_con, c10, 0x4, 0x0);
    __asm__ __volatile__ ("nop; nop");
    /* Set new ASID (procID) */
    write_cp15_register(C15_pid, c0, 0x1, dest_asid);
    __asm__ __volatile__ ("nop");
    /* install new PT */
    write_cp15_register(C15_ttbase, C15_CRm_default, C15_OP2_default, dest_pt);
    __asm__ __volatile__ ("nop; nop");
}

/**
 * Try to copy a mapping from kernel space into the current space
 * @param addr the address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 * Synchronization must happen at the highest level, allowing sharing.
 */
bool generic_space_t::sync_kernel_space(addr_t addr)
{
    /* We set everything up at initialisation time */
    if (this == get_kernel_space()) return false;

    /* get the 1m entries in the pagetables to compare */
    pgent_t::pgsize_e size = pgent_t::size_1m;
    word_t section = ((word_t)addr) >> page_shift(size);
    pgent_t * dst_pgent = this->pgent(0)->next(this, size, section);
    pgent_t * src_pgent = get_kernel_space()->pgent(0)->next(this, size, section);

    /* (already valid) || (kernel space invalid) */
    if ((dst_pgent->raw & 0x3) || !(src_pgent->raw & 0x3))
    {
        return false;
    }

    *dst_pgent = *src_pgent;
    return true;
}


void generic_space_t::flush_tlb(space_t *curspace)
{
    asid_t *asid = ((space_t *)this)->get_asid();
    if (asid->is_valid()) {
        flush_asid (asid->value());
    }
}

void generic_space_t::flush_tlbent_local(space_t *curspace, addr_t vaddr, word_t log2size)
{
    asid_t *asid = ((space_t *)this)->get_asid();

    vaddr = addr_align(vaddr, 1 << log2size);

    if (asid->is_valid()) {
        this->activate(get_current_tcb());

        arm_cache::cache_flush_ent_mva(vaddr, log2size);
        arm_cache::tlb_flush_ent(asid->value(), vaddr, log2size);

        curspace->activate(get_current_tcb());
    }
}

bool generic_space_t::allocate_page_directory()
{
    addr_t addr = kmem.alloc(kmem_pgtab, ARM_L1_SIZE, true);
    if (!addr) {
        return false;
    }

    /* kmem.alloc zeros out the page, in cached memory. Since we'll be using
     * this for uncached accesses, need to flush this out now.
     */
    arm_cache::cache_flush_d_ent(addr, ARM_L1_BITS);
#if defined(CONFIG_USE_L2_CACHE) && !defined(CONFIG_ARM_HWWALKER_IN_L2)
    if (get_arm_l2cc()->is_enabled()) {
        get_arm_l2cc()->cache_flush_by_pa_range(virt_to_ram(addr), ARM_L1_BITS);
    }
#endif
    pdir = (pgent_t*)virt_to_page_table(addr);
    return true;
}

void generic_space_t::free_page_directory()
{
    kmem.free(kmem_pgtab, page_table_to_virt(pdir), ARM_L1_SIZE);
}
