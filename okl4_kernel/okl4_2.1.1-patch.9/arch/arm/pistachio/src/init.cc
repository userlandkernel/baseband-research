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
 * Description:   ARM specific initialization code
 */

#include <l4.h>
#include <debug.h>
#include <schedule.h>
#include <space.h>
#include <arch/memory.h>
#include <intctrl.h>
#include <interrupt.h>
#include <memdesc.h>
#include <rootserver.h>
#include <config.h>
#include <arch/hwspace.h>
#include <cpu/syscon.h>
#include <arch/init.h>
#include <generic/lib.h>
#include <arch/platform.h>
#include <arch/platsupport.h>

#if defined(CONFIG_ARM_VFP)
#include <cpu/vfp.h>
#endif

extern "C" void startup_system_mmu();
extern void init_spaceids(void);


#if !defined(CONFIG_ARM_THREAD_REGISTERS)
/* UTCB reference page. This needs to be revisited */
ALIGNED(UTCB_AREA_PAGESIZE) char arm_utcb_page[UTCB_AREA_PAGESIZE] UNIT("utcb");
#endif

/*
 * Length of the timer tick in microseconds.  Set from the return
 * value of Platform::init_clocks().
 */
word_t timer_tick_length;

#if defined(CONFIG_TRUSTZONE)

/* System Mode: Are we running in secure or non-seceure mode */
bool arm_secure_mode;

#endif

extern "C" void SECTION(".init") init_arm_globals(word_t *physbase)
{
    extern char __kernel_space_object[];

    /* Copy physical address bases first */
    get_arm_globals()->phys_addr_ram = *physbase;
#if defined (CONFIG_XIP)
    get_arm_globals()->phys_addr_rom = *(physbase+1);
#endif

    get_arm_globals()->kernel_space = (space_t*)&__kernel_space_object;

#if defined(CONFIG_ENABLE_FASS)
    extern char _kernel_space_pagetable[];
    extern arm_fass_t arm_fass;

    get_arm_globals()->cpd = (pgent_t*)((word_t)(&_kernel_space_pagetable)
            - VIRT_ADDR_RAM + VIRT_ADDR_PGTABLE);
    get_arm_globals()->arm_fass = &arm_fass;
#endif
#if defined(CONFIG_TRACEPOINTS)
    get_arm_globals()->tracepoints_enabled = false;
#endif
}

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)


/**
 * Get page table index of virtual address for a 1m size
 *
 * @param vaddr         virtual address
 *
 * @return page table index for a table of the given page size
 */
INLINE word_t page_table_1m (addr_t vaddr)
{
    return ((word_t) vaddr >> 20) & (ARM_HWL1_SIZE - 1);
}

/*
 * Add 1MB mappings during init.
 * This function works with the mmu disabled
 */
extern "C" void SECTION(".init") add_mapping_init(pgent_t *pdir, addr_t vaddr,
                addr_t paddr, memattrib_e attrib)
{
    pgent_t *pg = pdir + (page_table_1m(vaddr));

    pg->set_entry_1m(NULL, paddr, true, true, true, attrib);
}

extern "C" void SECTION(".init") add_rom_mapping_init(pgent_t *pdir, addr_t vaddr,
                addr_t paddr, memattrib_e attrib)
{
    pgent_t *pg = pdir + (page_table_1m(vaddr));

    pg->set_entry_1m(NULL, paddr, true, false, true, attrib);
}

SECTION(".init")
addr_t find_kernel_heap(word_t *size, word_t *physbase)
{
    memdesc_t *mdesc;
    struct roinit *init_phys;

    init_phys = (struct roinit *)virt_to_phys_init(get_init_data(), physbase);

    /*
     * Search the memory descriptors for our kernel memory pool
     * which was provided by the buildtool/bootloader
     */
    memdesc_set_t *mds = &init_phys->memory_descriptor_set;
    for (word_t i = 0; i < mds->num_descriptors; i++) {
        addr_t high, low;
        mdesc = &mds->descriptors[i];

        if((memdesc_type_get(*mdesc) != memdesc_type_arch) ||
           (memdesc_subtype_get(*mdesc) != 0) || memdesc_is_virtual(*mdesc)) {
            continue;
        }
        low  = addr_align_up((addr_t)memdesc_low_get(*mdesc), ARM_PAGE_SIZE);
        high = addr_align((addr_t)(memdesc_high_get(*mdesc) + 1),
                          ARM_PAGE_SIZE);

        *size = (word_t)high - (word_t)low;
        return low;
    }

    return INVALID_ADDR;
}

#ifdef ARM_CPU_HAS_TLBLOCK
void kernel_tlb_lockdown(void);
#endif

#ifdef CONFIG_USE_L2_CACHE
#include <arch/l2cache.h>

static void init_enable_l2cache(void)
{
    arm_l2_cache_t *l2cc = get_arm_l2cc();
#ifdef CONFIG_HAS_L2_EVTMON
    arm_l2_event_monitor_t *l2evtmon = get_arm_l2evtmon();

    l2evtmon->init();
#endif

    l2cc->init();
    l2cc->enable();
    l2cc->cache_flush();
#if 0
    /* XXX Experimental code for the moment */
#ifdef CONFIG_L2CACHE_LOCK_WAY
    /* only allow way 0 */
    word_t way_bit = ~1UL & ((1UL << l2cc->get_associativity()) - 1);
    l2cc->cache_lockway_i(way_bit);
    l2cc->cache_lockway_d(way_bit);
    word_t vaddr;
    /* lock except handler code vaddr=0xf0021000 phys=0x80021000, size 0x800 */
    for (vaddr = 0xf0021000; vaddr < 0xf0021800; vaddr += 1UL << L2_LINE_SIZE_LOG2)
        arm_cache::prefetch_icache((addr_t)vaddr);
    /* lock way 0, allow every way else */
    way_bit = 0x1UL;
    l2cc->cache_lockway_i(way_bit);
    l2cc->cache_lockway_d(way_bit);
#endif
#endif
}
#endif

/*
 * Setup the kernel page table and map the kernel and data areas.
 *
 * This function is entered with the MMU disabled and the code
 * running relocated in physical addresses. We setup the page
 * table and bootstrap ourselves into virtual mode/addresses.
 */
extern tcb_t * tcb_array;
extern word_t num_tcbs;
extern "C" void NORETURN SECTION(".init") init_memory(word_t *physbase)
{
    extern char _kernel_space_pagetable[];

    word_t i;
    addr_t start, end;

    pgent_t * kspace_phys = (pgent_t *)virt_to_phys_init( _kernel_space_pagetable, physbase);

    /* Zero out the level 1 translation table */
    for (i = 0; i < ARM_HWL1_SIZE/sizeof(word_t); ++i)
        ((word_t*)kspace_phys)[i] = 0;

    /* Calculate the area for the kernel to use as boot memory */
    word_t bootmem_size;
    addr_t start_bootmem;
    addr_t start_bootmem_phys = 0;

    start_bootmem_phys = find_kernel_heap(&bootmem_size, physbase);

    if (start_bootmem_phys == INVALID_ADDR) {
        while (1);
    }

    start_bootmem_phys = addr_align_up(start_bootmem_phys, ARM_PAGE_SIZE);

    /* Get the bootmem addresses */
    start_bootmem = phys_to_virt_init(start_bootmem_phys, physbase);
    addr_t end_bootmem =        (addr_t)((word_t)start_bootmem + bootmem_size);
    addr_t end_bootmem_phys =   (addr_t)virt_to_phys_init(end_bootmem, physbase);

    /* The kernel can only map 64MB for kernel memory */
    if (((word_t)end_bootmem - KERNEL_AREA_START) > 64*PAGE_SIZE_1M)
        while (1);
    /*lint -e506 flint thinks this is a constant boolean, as it treats
     * address of variables as constant, when this is not really practical
     * in this case.
     */
    if (((word_t)end_kernel - KERNEL_AREA_START) > 64*PAGE_SIZE_1M)
        while (1);

#if (VIRT_ADDR_ROM != VIRT_ADDR_RAM)
    /* Map the rom area to its virtual address (if any) */
    if (end_rom > start_rom)
    {
        start = addr_align(virt_to_phys_init(start_rom, physbase), PAGE_SIZE_1M);
        end = virt_to_phys_init(end_rom, physbase);

        for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
        {
            add_rom_mapping_init( kspace_phys, (addr_t)phys_to_virt_init(i, physbase),
                    (addr_t)i, writeback );
        }
    }
#endif

    /* Map the kernel area to its virtual/pagetable address */
    {
        start = addr_align(virt_to_phys_init(start_ram, physbase), PAGE_SIZE_1M);
        end = virt_to_phys_init(end_kernel, physbase);

        for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
        {
            add_mapping_init( kspace_phys, (addr_t)phys_to_virt_init(i, physbase),
                    (addr_t)i, writeback );
            add_mapping_init( kspace_phys, (addr_t)phys_to_page_table_init(i, physbase),
                    (addr_t)i, writethrough );
        }

        /* Map the kernel bootmem area if not covered by the kernel mapping */
        if ((word_t)addr_align_up(end_bootmem_phys, PAGE_SIZE_1M) >
                (word_t)addr_align_up(end, PAGE_SIZE_1M))
        {
            start = addr_align(start_bootmem_phys, PAGE_SIZE_1M);

            if ((word_t)start < (word_t)addr_align_up(end, PAGE_SIZE_1M))
                start = addr_align_up(end, PAGE_SIZE_1M);

            end = end_bootmem_phys;

            /* Map the kernel bootmem */
            for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
            {
                add_mapping_init( kspace_phys, (addr_t)phys_to_virt_init(i, physbase),
                        (addr_t)i, writeback );
                add_mapping_init( kspace_phys, (addr_t)phys_to_page_table_init(i, physbase),
                        (addr_t)i, writethrough );
            }
        }
    }
    /* Enable kernel domain (0) */
    write_cp15_register(C15_domain, C15_CRm_default, C15_OP2_default, 0x0001);
    /* Set the translation table base to use the kspace_phys */
    write_cp15_register(C15_ttbase, C15_CRm_default, C15_OP2_default, (word_t) kspace_phys);

    /* Map the current code area 1:1 */
    start = addr_align(virt_to_phys_init(start_init, physbase), PAGE_SIZE_1M);
    end = virt_to_phys_init(end_init, physbase);
    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
        add_mapping_init( kspace_phys, (addr_t)i, (addr_t)i, uncached );
    }

    /* Enable virtual memory, caching etc */
    write_cp15_register(C15_control, C15_CRm_default,
                    C15_OP2_default, C15_CONTROL_KERNEL);
    CPWAIT;

    /* Switch to virtual memory code and stack */
    switch_to_virt();

    /* Initialize global pointers (requres 1:1 mappings for physbase) */
    init_arm_globals(physbase);

    /* Initialize variables used by cache routines */
    Platform::dcache_attributes(&DCACHE_SIZE, &DCACHE_LINE_SIZE, &DCACHE_SETS, &DCACHE_WAYS);
    Platform::icache_attributes(&ICACHE_SIZE, &ICACHE_LINE_SIZE, &ICACHE_SETS, &ICACHE_WAYS);

    /* Initialise the kernel memory */
    kmem.init(start_bootmem, end_bootmem);

#ifdef CONFIG_ENABLE_FASS
    static char _kernel_top_level[ARM_L0_SIZE];

    pgent_t * current = (pgent_t *)_kernel_top_level;
    /* Link kernel space top level to CPD */
    for (i = 0; i < ARM_L0_ENTRIES; i++, current++)
        current->tree = ((pgent_t *) virt_to_page_table(_kernel_space_pagetable)) + i * ARM_L1_ENTRIES;
#else
#define _kernel_top_level virt_to_page_table(_kernel_space_pagetable)
#endif

    /* Initialize the kernel space data structure and link it to the pagetable */
    space_t::set_kernel_page_directory((pgent_t *)_kernel_top_level);

    space_t * kspace = get_kernel_space();
    bool r;

    /* Map TCB array */
    start = addr_align(tcb_array, PAGE_SIZE_1M);
    end = (addr_t)(((word_t)start) + (num_tcbs * KTCB_SIZE));

    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
        r = kspace->add_mapping((addr_t)phys_to_virt(i),
                (addr_t)i, pgent_t::size_1m, space_t::read_write, true, writeback );
        ASSERT(ALWAYS, r);
    }
    tcb_array = phys_to_virt(tcb_array);

    /* Setup L2 Cache */
#ifdef CONFIG_USE_L2_CACHE
#ifdef CONFIG_ARM_L2_INSTRUCTIONS
    #error "Unimplemented yet"
#else
    /* Map L2 Cache Controller */
    r = kspace->add_mapping((addr_t)L2CC_VIRT_BASE, (addr_t)L2CC_PHYS_BASE,
            pgent_t::size_4k, space_t::read_write, false, uncached);
    ASSERT(ALWAYS, r);
#ifdef CONFIG_HAS_L2_EVTMON
    /* Map L2 Event Monitor */
    r = kspace->add_mapping((addr_t)L2EVTMON_VIRT_BASE, (addr_t)L2EVTMON_PHYS_BASE,
            pgent_t::size_4k, space_t::read_write, false, uncached);
#endif
#endif
    init_enable_l2cache();
#ifdef CONFIG_ARM_HWWALKER_IN_L2
    /* remap pagetables as L2-cacheable */
    {
        start = addr_align(virt_to_phys_init(start_ram, physbase), PAGE_SIZE_1M);
        end = virt_to_phys_init(end_kernel, physbase);

        for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
        {
            kspace->add_mapping((addr_t)phys_to_page_table_init(i, physbase),
                    (addr_t)i, pgent_t::size_1m, space_t::read_write, true, WT_WBa );
        }

        /* Map the kernel bootmem area if not covered by the kernel mapping */
        if ((word_t)addr_align_up(end_bootmem_phys, PAGE_SIZE_1M) >
                (word_t)addr_align_up(end, PAGE_SIZE_1M))
        {
            start = addr_align(start_bootmem_phys, PAGE_SIZE_1M);

            if ((word_t)start < (word_t)addr_align_up(end, PAGE_SIZE_1M))
                start = addr_align_up(end, PAGE_SIZE_1M);

            end = end_bootmem_phys;

            /* Map the kernel bootmem */
            for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
            {
                kspace->add_mapping((addr_t)phys_to_page_table_init(i, physbase),
                        (addr_t)i, pgent_t::size_1m, space_t::read_write, true, WT_WBa );
            }
        }
        /* change hw-page-walker cache attributes */
        write_cp15_register(C15_ttbase, C15_CRm_default, C15_OP2_default, (word_t) kspace_phys | (1<<3));
        arm_cache::tlb_flush();
    }
#endif
#endif

#if !defined(CONFIG_ARM_THREAD_REGISTERS)
    /* Map the UTCB reference page */
    r = kspace->add_mapping((addr_t)USER_UTCB_PAGE, virt_to_phys((addr_t) arm_utcb_page),
            UTCB_AREA_PGSIZE, space_t::read_execute, false);
    ASSERT(ALWAYS, r);
    arm_cache::cache_flush();

// FIXME: Confirm that this can go away PJH
#if 0
#if CONFIG_ARM_VER >= 6
    /* Unfortunately, none of the internal APIs support encoding a mapping as
     * user-accessible, kernel RW and marked as global. For ARM11, update the
     * KIP's pagetable entry here to be global.
     */
    {
        pgent_t *pg;
        r = kspace->lookup_mapping((addr_t)USER_UTCB_PAGE, &pg, NULL);
        ASSERT(ALWAYS, r);

        /* Fixup the entry to be global */
        armv6_l2_desc_t l2_entry;
        l2_entry.raw = pg->raw;
        l2_entry.small.nglobal = 0;
        pg->raw = l2_entry.raw;

        arm_cache::cache_drain_write_buffer();
    }
#endif
#endif
#endif

    start = addr_align(virt_to_phys(start_init), PAGE_SIZE_1M);
    end = virt_to_phys(end_init);

    /* Remove 1:1 mapping */
    for (i = (word_t)start; i < (word_t)end; i += PAGE_SIZE_1M)
    {
        ((pgent_t *)_kernel_space_pagetable)[i/ARM_SECTION_SIZE].clear(kspace, pgent_t::size_1m, true);
    }

    jump_to((word_t)startup_system_mmu);
}


NORETURN void SECTION(".init") generic_init();

extern "C" void SECTION(".init")
show_processor_info()
{
    unsigned int c15_id;
    read_cp15_register(C15_id, C15_CRm_default, C15_OP2_default, c15_id);

    TRACE_INIT("Processor Id => %lx: ", c15_id);

    switch ((c15_id >> 16) & 0xf)
    {
    case 0x1: TRACE_INIT("v4"); break;
    case 0x2: TRACE_INIT("v4T"); break;
    case 0x3: TRACE_INIT("v5"); break;
    case 0x4: TRACE_INIT("v5T"); break;
    case 0x5: TRACE_INIT("v5TE"); break;
    case 0x6: TRACE_INIT("v5TEJ"); break;
    case 0x7: TRACE_INIT("v6"); break;
    case 0xf: TRACE_INIT("Extended Info"); break;
    default:  TRACE_INIT("UNKNOWN"); break;
    }

    switch (c15_id >> 24)
    {
    case 0x41:
        TRACE_INIT(", ARM");
        switch ((c15_id >> 4) & 0xfff)
        {
        default:
            TRACE_INIT("%d%x", (c15_id >> 12) & 0xf, (c15_id >> 4) & 0xff);
            break;
        }
        break;
    case 0x44:
        TRACE_INIT(", DEC ");
        switch ((c15_id >> 4) & 0xfff)
        {
        case 0xa10:
        case 0xa11: TRACE_INIT("SA1100"); break;
        default:
            TRACE_INIT("0x%x", (c15_id >> 4) & 0xfff);
            break;
        }
        break;
    case 0x69:
        TRACE_INIT(", Intel ");
        switch ((c15_id >> 4) & 0x3f)
        {
        case 0x10: TRACE_INIT("PXA255"); break;
        case 0x1c: TRACE_INIT("IXP42X 533MHz"); break;
        case 0x1d: TRACE_INIT("IXP42X 400MHz"); break;
        case 0x1f: TRACE_INIT("IXP42X 266MHz"); break;
        case 0x20: TRACE_INIT("IXP45X/IXP46X"); break;
        default:
            TRACE_INIT("0x%x", (c15_id >> 4) & 0xfff);
            break;
        }
        break;
    default:
        TRACE_INIT(": UNKNOWN ");
        break;
    }
    TRACE_INIT(", rev %d", c15_id & 0xf);

    TRACE_INIT("\n");

#if defined(CONFIG_ARM_VFP)
    word_t cp_avail;
    /* Check which coprocessors are available */
#if defined(CONFIG_CPU_ARM_ARM1136JS) || defined(CONFIG_CPU_ARM_ARM926EJS)
    word_t cp_acr;

    write_cp15_register(c1, c0, 2, 0xffffffff);
    read_cp15_register(c1, c0, 2, cp_acr);
    write_cp15_register(c1, c0, 2, 0x0);

    cp_avail = 0;

    for (int i=0; i <= 13; i++) {
        if ((cp_acr >> (i*2))&0x3) {
            cp_avail |= (1<<i);
            TRACE_INIT("CP%d available\n", i);
        }
    }

    /* ARM11 enable VFP copro interface (if avail) */
    write_cp15_register(c1, c0, 2, (0xf<<20));
    write_cp15_register(c7, c5, 4, 0);  /* Flush Prefetch Buffer (IMB) */
#else
    /* FIXME for ARMv5/v4 */
    cp_avail = 0;
#endif

    if (((cp_avail >> 10) & 0x3) == 0x3)
    {
        word_t fpsid;
        TRACE_INIT("VFP CoProcessor Supported\n");

        vfp_getsr(FPSID, fpsid);

        /* Disable VFP System */
        arm_vfp_t::disable();

        TRACE_INIT("VFP System Id => %lx: ", fpsid);

        switch (fpsid >> 24)
        {
            case 0x41:
                TRACE_INIT("ARM, ");
                break;
            default:
                TRACE_INIT("UNKNOWN, ");
                break;
        }
        if (fpsid & (1<<23))
            TRACE_INIT(" No HW support!");
        else
        {
            TRACE_INIT("Format %d, ", ((fpsid >> 21) & 3) + 1);
            TRACE_INIT("Precision %s, ", (fpsid & (1<< 20)) ? "Single" : "Single+Double");
            TRACE_INIT("VFPv%d, ", ((fpsid >> 16) & 0xf) + 1);
            TRACE_INIT("PartNo %x, ", ((fpsid >> 8) & 0xff));
            TRACE_INIT("Rev %d, ", ((fpsid >> 0) & 0xf) + 1);
        }

        TRACE_INIT("\n");

    }
#endif

#if defined(CONFIG_TRUSTZONE)
    /* Determine CPU mode */
    word_t c15_control, temp;

    arm_secure_mode = false;

    /*
     * If the processor supports trustzone, only secure-mode
     * can modify the L4-bit in the control register
     */
    read_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, c15_control);

    c15_control ^= C15_CONTROL_L4;

    write_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, c15_control);
    for (volatile int i = 0; i < 2; i++);
    read_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, temp);

    /* If update occured - we are in secure mode */
    if (temp == c15_control)
    {
        arm_secure_mode = true;

        /* Restore register */
        c15_control ^= C15_CONTROL_L4;
        write_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, c15_control);

    }

    printf("Booting in %s world\n", arm_secure_mode ? "secure" : "non-secure");

#endif
}

/**
 * Entry point from the platform ASM into C kernel
 * Precondition: MMU and page tables are disabled
 * Warning: Do not use local variables in arch_init()
 */
extern "C" void SECTION(".init") PlatformSupport::arch_init(void* physbase)
{
    /* Map the L4 kernel, enable virtual memory and setup bootmem */
    init_memory((word_t*) physbase);
    /* init_memory returns to startup_system_mmu */
}

extern "C" void SECTION(".init") startup_system_mmu()
{
    /*
     * Initialise the platform, map the console and other memory.
     *
     * If something goes wrong then loop forever.
     */
    if (!Platform::init(Platform::API_VERSION, PlatformSupport::API_VERSION))
        for(;;);

    /* Initialise the L4 console */
    init_console();
    init_hello();

#if CONFIG_ARM_VER >= 6
    get_asid_cache()->init();
    get_asid_cache()->set_valid(1, CONFIG_MAX_NUM_ASIDS - 1);
#endif

    /* Initialise the tracebuffer */
    init_tracebuffer();

    /* Initialise TCB memory allocator */
    init_tcb_allocator();

    /* Allocate spaceid translation table */
    init_spaceids();

    /* Setup the kernel address space */
    init_kernel_space();

    TRACE_INIT("Initializing kernel debugger...\n");

    /* initialize kernel debugger if any */
    if (kdebug_entries.kdebug_init) {
        kdebug_entries.kdebug_init();
    }
    else {
        TRACE_INIT("No kernel debugger!\n\r");
    }
    /* Configure IRQ hardware */
    TRACE_INIT("Initializing interrupts...\n");
    init_arm_interrupts();

    init_idle_tcb();
    get_arm_globals()->current_tcb = get_idle_tcb();

    show_processor_info();

    TRACE_INIT("Initializing timer...\n");
    /* initialize the kernel's timer source */
    timer_tick_length = Platform::init_clocks();

#ifdef ARM_CPU_HAS_TLBLOCK
    kernel_tlb_lockdown();
#endif

    /* Architecture independent initialisation. Should not return. */
    generic_init();

    NOTREACHED();
}

#ifdef ARM_CPU_HAS_TLBLOCK
void SECTION(".init") kernel_tlb_lockdown(void)
{
    TRACE_INIT("TLB lock: vectors @ %lx\n", ARM_HIGH_VECTOR_VADDR);
    arm_cache::lock_tlb_addr((addr_t)ARM_HIGH_VECTOR_VADDR);

    TRACE_INIT("TLB lock: utcb @ %lx\n", USER_UTCB_PAGE);
    arm_cache::lock_tlb_addr((addr_t)USER_UTCB_PAGE);

    TRACE_INIT("TLB lock: kernel @ %p\n", start_rom);
    arm_cache::lock_tlb_addr(start_rom);
#if (VIRT_ADDR_ROM != VIRT_ADDR_RAM)
    arm_cache::lock_tlb_addr(start_ram);
#endif
    arm_cache::lock_tlb_addr(virt_to_page_table(start_ram));

    TRACE_INIT("Locked kernel into TLB\n");
}
#endif

#ifdef ARM_CPU_HAS_CACHELOCK
void SECTION(".init") kernel_cache_lockdown(void)
{
    /* FIXME: This is broken -> need to establish a 1:1 mapping for kernel before calling
     * switch_to_phys
     */
    word_t * physbase = virt_to_phys(&get_arm_globals()->phys_addr_ram);
    /* Switch to physical memory code and stack */
    switch_to_phys();
    /* From here we are running 1:1 physical uncached!!! */

    arm_cache::cache_flush();
    arm_cache::unlock_icache();

    arm_cache::lock_icache_range((addr_t)ARM_HIGH_VECTOR_VADDR, 0x7e0, 0);

    /* Switch back to virtual memory code and stack */
    switch_to_virt();
}
#endif


