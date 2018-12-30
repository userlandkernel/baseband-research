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
 * Description:   address space management
 */

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <l4/memregion.h>
#include <generic/lib.h>

#include <tcb.h>
#include <smp.h>

#include <arch/mmu.h>
#include <arch/trapgate.h>
#include <arch/pgent.h>
#include <linear_ptab.h>
#include <arch/hwirq.h>

#include <arch/memory.h>
#include <space.h>
#include "bitmap.h"

#define PGSIZE_KTCB     (pgent_t::size_4k)
#define PGSIZE_UTCB     (pgent_t::size_4k)
#if (KERNEL_PAGE_SIZE == IA32_SUPERPAGE_SIZE)
#define PGSIZE_KERNEL    pgent_t::size_4m
#else
#define PGSIZE_KERNEL    pgent_t::size_4k
#endif
#define PGSIZE_KIP      (pgent_t::size_4k)
#define PGSIZE_SIGMA    PGSIZE_KERNEL

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_utcb);

extern stack_t __stack;
void * stack_top;
extern bitmap_t space_ids;

word_t ia32_current_clist = NULL;

/* helpers */
INLINE void align_memregion(mem_region_t & region, word_t size)
{
    region.low = (word_t)addr_t(mem_region_low_get(region) & ~(size - 1));
    region.high = (word_t)addr_t((mem_region_high_get(region) + size - 1) &
                                 ~(size - 1));
}

bool space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
                          rwx_e rwx, bool kernel, bool global, memattrib_e attrib)
{
    pgent_t::pgsize_e curr_size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(curr_size, vaddr));

    //TRACEF("space=%p, v=%p, p=%p, size=%d, w=%d, k=%d\n", this, vaddr, paddr, size, rwx, kernel);

    if (size < curr_size)
    {
        if (!pgent->is_valid(this, curr_size))
        {
            if (pgent->make_subtree(this, curr_size, kernel) == false)
                return false;
        }
        else
        {
            /* check if already mapped as a 4MB page */
            if (!pgent->is_subtree(this, curr_size))
            {
                /* check that alignement of virtual and physical page fits */
                ASSERT(NORMAL, addr_mask(vaddr, ~IA32_SUPERPAGE_MASK) ==
                       addr_mask(paddr,~IA32_SUPERPAGE_MASK));

                if (((addr_t)pgent->address(this, curr_size)) ==
                    addr_mask(paddr, IA32_SUPERPAGE_MASK))
                    return true;
                panic("Shouldn't reach here");
            }
        }
        curr_size--;
        //TRACEF("%p, %p\n", pgent->subtree(this, curr_size + 1), pgent);
        pgent = pgent->subtree(this, curr_size + 1)->next(
            this, curr_size, page_table_index(curr_size, vaddr));
    }
    //TRACEF("%p\n", pgent);
    pgent->set_entry(this, curr_size, paddr, true, rwx == read_write ? true : false, true, kernel,
            attrib);

    // default: kernel->global, user->non-global
    if (kernel != global) pgent->set_global(this, curr_size, global);

    return true;
}

void space_t::remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
                         word_t len, rwx_e rwx, bool kernel, bool global)
{
    TRACE_INIT("remap area %p->%p len=%x, w=%d, k=%d, size=%d\n",
               vaddr, paddr, len, writable, kernel, pgsize);
    bool r;
    word_t page_size = (pgsize == pgent_t::size_4k) ? IA32_PAGE_SIZE :
        IA32_SUPERPAGE_SIZE;

    // length must be page-size aligned
    ASSERT(NORMAL, (len & (page_size - 1)) == 0);

    for (word_t offset = 0; offset < len; offset += page_size) {
        r = add_mapping(addr_offset(vaddr, offset), addr_offset(paddr, offset),
                        pgsize, rwx, kernel, global, cached);
        ASSERT(ALWAYS, r);
    }
    r = add_mapping(addr_offset(vaddr, len-1), addr_offset(paddr, len-1),
                    pgsize, rwx, kernel, global, cached);
    ASSERT(ALWAYS, r);
}


/**
 * reads a word from a given physical address, uses a remap window and
 * maps a 4MB page for the access
 *
 * @param vaddr         virtual address (if caches need to be flushed)
 * @param paddr         physical address to read from
 * @return the value at the given address
 */
word_t generic_space_t::readmem_phys (addr_t vaddr, addr_t paddr)
{
#if defined CONFIG_MDOMAINS
    int cpu = get_current_context().domain;
#else
    //int cpu = 0;
#endif

    // get the _real_ space, use CR3 for that
    space_t * space = get_kernel_space()/*ptab_to_space(ia32_mmu_t::get_active_pagetable(), cpu)*/;

    pgent_t* pgent = space->pgent( page_table_index(pgent_t::size_max, (addr_t)MEMREAD_AREA_START));

    if (!pgent->is_valid(space, pgent_t::size_max)) {
        bool r;
        r = pgent->make_subtree(space, pgent_t::size_max, true);
        ASSERT(ALWAYS, r);
    }
    pgent = pgent->subtree(space, pgent_t::size_max)->next(
        space, pgent_t::size_4k, page_table_index(pgent_t::size_4k, paddr));

    pgent->set_entry(space, pgent_t::size_4k, paddr, true, false, false, true);

    // kill potentially stale TLB entry in remap-window
    ia32_mmu_t::flush_tlbent(
        (word_t)addr_offset(addr_mask(paddr, page_mask (pgent_t::size_4m)),
            MEMREAD_AREA_START));

    return *(word_t*)addr_offset(addr_mask(paddr,~IA32_SUPERPAGE_MASK),
                                 MEMREAD_AREA_START);
}


/**********************************************************************
 *
 *                        SMP handling
 *
 **********************************************************************/

#ifdef CONFIG_SMP

struct {
    space_t * space;
    char __pad [CACHE_LINE_SIZE - sizeof(space_t*)];
} active_cpu_space[CONFIG_SMP_MAX_CPUS];

static word_t cpu_remote_flush UNIT("cpulocal");
static word_t cpu_remote_flush_global UNIT("cpulocal");

#define __FLUSH_GLOBAL__        false

static void do_xcpu_flush_tlb(cpu_mb_entry_t * entry)
{
    spin(60, get_current_cpu());
    ia32_mmu_t::flush_tlb (__FLUSH_GLOBAL__);
}

INLINE void tag_flush_remote (space_t * curspace)
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
    {
        if (cpu == get_current_cpu())
            continue;
        if (active_cpu_space[cpu].space == curspace)
            cpu_remote_flush |= (1 << cpu);

    }
}

void generic_space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace || IS_SPACE_SMALL (this))
        ia32_mmu_t::flush_tlb (IS_SPACE_GLOBAL (this));
    tag_flush_remote (this);
}

void generic_space_t::flush_tlbent (space_t * curspace, addr_t addr, word_t log2size)
{
    if (this == curspace || IS_SPACE_SMALL (this))
        ia32_mmu_t::flush_tlbent ((u32_t) addr);
    tag_flush_remote (this);
}

void generic_space_t::end_update ()
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
        if (cpu_remote_flush & (1 << cpu))
            sync_xcpu_request(cpu, do_xcpu_flush_tlb, NULL,
                              cpu_remote_flush_global & (1 << cpu));
    cpu_remote_flush = 0;
    cpu_remote_flush_global = 0;
}
#endif



/**********************************************************************
 *
 *                         System initialization
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != IA32_SUPERPAGE_SIZE) && \
    (KERNEL_PAGE_SIZE != IA32_PAGE_SIZE)
# error invalid kernel page size - please adapt
#endif

extern mem_region_t kernel_image_rgn;

void SECTION(SEC_INIT) generic_space_t::init_kernel_mappings()
{
    bool r;

    stack_top = (void *) (((word_t)(&__stack)) | STACK_TOP);

    /* we map both reserved areas into the kernel area */
    mem_region_t reg = kernel_image_rgn;
    ASSERT(NORMAL, !mem_region_is_empty(reg));

    align_memregion(reg, KERNEL_PAGE_SIZE);
    ((space_t *)this)->remap_area(phys_to_virt((addr_t)mem_region_low_get(reg)),
                                  (addr_t)mem_region_low_get(reg),
                                  PGSIZE_KERNEL, mem_region_size_get(reg),
                                  space_t::read_write, true, true);
                                  

    /* map init memory */
    /* FIXME: put in a region desc? */
    mem_region_set(&reg, (word_t)start_init, (word_t)end_init);
    align_memregion(reg, KERNEL_PAGE_SIZE);
    ((space_t *)this)->remap_area((addr_t)mem_region_low_get(reg),
                                  (addr_t)mem_region_low_get(reg),
                                  PGSIZE_KERNEL, mem_region_size_get(reg),
                                  space_t::read_write, true, true);

    /* map low 4MB pages for initialization */
    mem_region_set(&reg, 0, 0x00400000);
    align_memregion(reg, IA32_SUPERPAGE_SIZE);
    ((space_t *)this)->remap_area((addr_t)mem_region_low_get(reg),
                                  (addr_t)mem_region_low_get(reg),
                                  PGSIZE_KERNEL, mem_region_size_get(reg),
                                  space_t::read_write, true, false);

    /* map video mem to kernel */
#ifndef CONFIG_EFI  /* XXX: should be ifdef PC99 or something  */
    r = ((space_t *)this)->add_mapping(phys_to_virt((addr_t)VIDEO_MAPPING),
                                       (addr_t)VIDEO_MAPPING,
                                       pgent_t::size_4k, space_t::read_write, true,
                                       true, cached);
    ASSERT(ALWAYS, r == true);
#endif

#ifdef CONFIG_KDB_CONS_DBG1394
    /* we need to insert a mapping for the firewire device into the
     * the new pagetable.
     *
     * FIXME: do it for init PT and switch to virt FW now?
     */
    {
        extern word_t fwaddr_phys;

        printf( "mapping 1394 phys %lx to virt %lx\n",
                (addr_t) fwaddr_phys, (addr_t) DBG1394_MAPPINGS );

        // map the OHCI registers
        r = ((space_t *)this)->add_mapping( (addr_t)DBG1394_MAPPINGS,
                                            (addr_t)fwaddr_phys,
                                            pgent_t::size_4k, true, true, true,
                                            cached );
        ASSERT(ALWAYS, r == true);
    }
#endif

    /* MYUTCB mapping
     * allocate a full page for all myutcb pointers.
     * access must be performed via gs:0, when setting up the gdt
     * each processor gets a full cache line to avoid bouncing
     * page is user-writable and global
     */
    EXTERN_KMEM_GROUP(kmem_misc);
    addr_t virt = kmem.alloc(kmem_misc, IA32_PAGE_SIZE, true);
    ASSERT(ALWAYS, virt);
    r = ((space_t *)this)->add_mapping((addr_t)MYUTCB_MAPPING,
                    virt_to_phys(virt), pgent_t::size_4k, space_t::read_write, false, true, cached);
    ASSERT(ALWAYS, r == true);

}



void generic_space_t::init_cpu_mappings(scheduler_domain_t cpu)
{
#if defined(CONFIG_SMP)
    /* CPU 0 gets the always initialized page table */
    if (cpu == 0) return;

    TRACE_INIT("init cpu mappings for cpu %d\n", cpu);

    mem_region_t reg = { start_cpu_local, end_cpu_local };
    align_memregion(reg, IA32_PAGEDIR_SIZE);

    TRACE_INIT("remapping CPU local memory %p - %p (%p - %p)\n",
               start_cpu_local, end_cpu_local, reg.low, reg.high);

    pgent_t::pgsize_e size = pgent_t::size_max;

    for (addr_t addr = reg.low; addr < reg.high;
         addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
    {
        pgent_t * dst_pgent = this->pgent(page_table_index(size, addr), cpu);
        pgent_t * src_pgent = this->pgent(page_table_index(size, addr), 0);
        dst_pgent->make_cpu_subtree(this, size, true);

        ASSERT(DEBUG, src_pgent->is_subtree(this, size));

        src_pgent = src_pgent->subtree(this, size);
        dst_pgent = dst_pgent->subtree(this, size);

        size--;

        // now copy the page table
        for (addr_t pgaddr = addr;
             pgaddr < addr_offset(addr, IA32_PAGEDIR_SIZE);
             pgaddr = addr_offset(pgaddr, IA32_PAGE_SIZE))
        {
            //TRACE_INIT("copying ptab @ %p (%p, %p)\n", pgaddr, src_pgent, dst_pgent);
            if (pgaddr < start_cpu_local || pgaddr > end_cpu_local)
                *dst_pgent = *src_pgent; // global data
            else
            {
                addr_t page = kmem.alloc(kmem_pgtab, IA32_PAGE_SIZE, true);
                TRACE_INIT("allocated cpu local page %p -> %p\n", pgaddr, page);
                dst_pgent->set_entry(this, size, virt_to_phys(page),
                                     true, true, true, true);
                TRACE_INIT("pgent = %x\n", dst_pgent->raw);
                memcpy(page, pgaddr, IA32_PAGE_SIZE);
            }
            src_pgent = src_pgent->next(this, size, 1);
            dst_pgent = dst_pgent->next(this, size, 1);
        }
        size++;
    }
    TRACE_INIT("switching to CPU local pagetable %p\n", get_pdir(cpu));
    ia32_mmu_t::set_active_pagetable((u32_t)get_pdir(cpu));
    ia32_mmu_t::flush_tlb(true);
    TRACE_INIT("cpu pagetable activated (%x)\n",
               ia32_mmu_t::get_active_pagetable());
#endif
}

void SECTION(SEC_INIT) init_kernel_space()
{
    ASSERT(ALWAYS, get_kernel_space());
    pgent_t * pt = (pgent_t*)kmem.alloc(kmem_pgtab, IA32_PTAB_BYTES, true);
    ASSERT(ALWAYS, pt);
    generic_space_t::set_kernel_page_directory(pt);

    get_kernel_space()->init_kernel_mappings();

    get_kernel_space()->set_space_id(spaceid_t::kernelspace());

    ia32_mmu_t::set_active_pagetable((u32_t)virt_to_phys(get_kernel_space()->pgent(0)));

#ifdef CONFIG_KDB_CONS_DBG1394
    /* at this point we can no longer print via firewire because the
     * the physical address is no longer mapped (may be masked by the kernel!)
     * we now tell it to use the setup its virtual address instead.
     *
     * FIXME: can we make it use virtual earlier on the init PT?
     */
    {
            extern word_t fwaddr_virt;
            fwaddr_virt = DBG1394_MAPPINGS;
    }
#endif

    get_kernel_space()->enqueue_spaces();
}

/**********************************************************************
 *
 *                    Small address spaces
 *
 **********************************************************************/


word_t space_t::space_control (word_t ctrl)
{
    return 0;
}


void generic_space_t::arch_free (void)
{

}


/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/

bool generic_space_t::sync_kernel_space(addr_t addr)
{
    if (this == get_kernel_space()) return false;

    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * dst_pgent = this->pgent(page_table_index(size, addr));
    pgent_t * src_pgent = get_kernel_space()->pgent(page_table_index(size, addr));

    /* (already valid) || (kernel space invalid) */
    if (dst_pgent->is_valid(this, size) ||
        (!src_pgent->is_valid(get_kernel_space(), size)))
    {
#if 0
        TRACE("sync ksp @ %p (src=%p (%d), dst=%p (%d))\n",
              addr, kernel_space, src_pgent->is_valid(kernel_space, size),
              this, dst_pgent->is_valid(this, size));
#endif
        return false;
    }

#if !defined(CONFIG_SMP)
    *dst_pgent = *src_pgent;
#else
    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
        *this->pgent(page_table_index(size, addr), cpu) = *kernel_space->pgent(page_table_index(size, addr), cpu);
#endif
    return true;
}

/**
 * generic_space_t::init initializes the space_t
 *
 * maps the kernel area and initializes shadow ptabs etc.
 */
bool generic_space_t::init(fpage_t utcb_area)
{
        /*#warning VU: space_t has to be restructured having an init and activate function*/
    for (addr_t addr = (addr_t)KERNEL_AREA_START;
         addr < (addr_t)KERNEL_AREA_END;
         addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
        (void) sync_kernel_space(addr);

    set_utcb_area (utcb_area);
    return true;
}


/* precondition: this is a valid utcb location for the space */
utcb_t * generic_space_t::allocate_utcb(tcb_t * tcb)
{
    ASSERT(DEBUG, tcb);
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    // TODO - This looks like it should be refactored to use lookup_mapping -TGB

    /* walk ptab, to see if a page is already mapped */
    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(size, utcb));
    while (size > PGSIZE_UTCB && pgent->is_valid(this, size))
    {
        ASSERT(DEBUG, pgent->is_subtree(this, size));

        pgent = pgent->subtree(this, size);
        size--;
        pgent = pgent->next(this, size, page_table_index(size, utcb));
    }

    /* if pgent is valid a page is mapped, otherwise allocate a new one */
    if (pgent->is_valid(this, size)) {
        utcb_t *new_utcb = (utcb_t*)phys_to_virt(addr_offset(pgent->address(this, size),
                                                 (word_t)utcb & (~IA32_PAGE_MASK)));
        memset(new_utcb, 0, sizeof(*new_utcb));
        return new_utcb;
    }
    else
    {
        // allocate new UTCB page, non-global
        addr_t page = kmem.alloc(kmem_utcb, IA32_PAGE_SIZE, true);
        if (page == NULL) {
            get_current_tcb()->set_error_code(ENO_MEM);
            return NULL;
        }
        if (((space_t *)this)->add_mapping((addr_t)utcb, virt_to_phys(page), PGSIZE_UTCB,
                        space_t::read_write, false, false, cached) == false)
        {
            get_current_tcb()->set_error_code(ENO_MEM);
            kmem.free(kmem_utcb, page, IA32_PAGE_SIZE);
            return NULL;
        }
        return (utcb_t*)addr_offset(page, (word_t)utcb & (~IA32_PAGE_MASK));
    }
}

/**********************************************************************
 *
 *                    global functions
 *
 **********************************************************************/

/**
 * exc_pagefault: trap gate for ia32 pagefault handler
 */
IA32_EXC_WITH_ERRORCODE(exc_pagefault, 0)
{
    PROFILE_START(exception_pf);
    u32_t pf = ia32_mmu_t::get_pagefault_address();
    //TRACEF("pagefault @ %p, ip=%p, sp=%p, error=%p, ksp=%p\n", pf, frame->eip, frame->esp, frame->error, &pf);
    space_t * space = get_current_space();
    if (!space)
        space = get_kernel_space();


    /* if the idle thread accesses the tcb area -
     * we will get a pagefault with an invalid space
     * so we use CR3 to figure out the space
     */
#ifdef CONFIG_MDOMAINS
    if (EXPECT_FALSE( space == NULL ))
        space = ptab_to_space(ia32_mmu_t::get_active_pagetable(),
                              get_current_context().domain);
#else
    if (EXPECT_FALSE( space == NULL ))
        space = ptab_to_space(ia32_mmu_t::get_active_pagetable(),
                              0);
#endif

    space->handle_pagefault(
        (addr_t)pf,
        (addr_t)frame->eip,
        (generic_space_t::access_e)(frame->error & IA32_PAGE_WRITABLE),
        (frame->error & 4) ? false : true, ASM_CONTINUATION);
}

bool generic_space_t::allocate_page_directory()
{
    pdir = (pgent_t *)kmem.alloc(kmem_pgtab, IA32_PTAB_BYTES, true);
    return pdir != NULL;
}

void generic_space_t::free_page_directory()
{
    kmem.free(kmem_pgtab, pdir, IA32_PTAB_BYTES);
    pdir = NULL;
}

void generic_space_t::free_utcb(utcb_t * utcb)
{
    /* do nothing, since in IA32, utcb isn't allocated by kernel,
     * kernel isn't responsible for removing utcb mapping from space
     * when deleting or switching thread from the space,
     * the mapping will only be removed when space in deleted.
     */
}

void generic_space_t::activate(tcb_t *tcb)
{
    ia32_current_clist = (word_t)this->get_clist();
    static u32_t last_active_pagetable = NULL;

    if (last_active_pagetable != (u32_t) tcb->page_directory) {
        last_active_pagetable = (u32_t) tcb->page_directory;
        ia32_mmu_t::set_active_pagetable((u32_t) virt_to_phys(tcb->page_directory));
    }
}

