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
 * Description:   MIPS kernel specific init.
 */                

#include <l4.h>

#include <kernelinterface.h>
#include <schedule.h>
#include <space.h>
#include <intctrl.h>
#include <interrupt.h>

#include <arch/memory.h>
#include <arch/tlb.h>
#include <syscalls.h>
#include <arch/syscalls.h>
#include <cpu/cache.h>
#include <generic/lib.h>
#ifdef CONFIG_SMP
#include <arch/smp.h>
#endif
#include <arch/platform.h>
#include <arch/platsupport.h>

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)

#ifdef CONFIG_SMP
word_t plat_cpu_freq[CONFIG_SMP_MAX_CPUS], plat_bus_freq[CONFIG_SMP_MAX_CPUS];
#else
word_t plat_cpu_freq[1], plat_bus_freq[1];
#endif

extern void init_spaceids(void);

static void SECTION (".init") dump_info(void)
{
}

static void SECTION (".init") init_cpu_cache(void)
{
    cache_t::init_cpu();

    /*
     * No need to call Platform::dcache_attributes() and
     * Platform::icache_attributes().
     */
}

/*
 * Initialize kernel debugger with initial boot memory, and register
 * kernel memory in the kernel info page.
 */
extern tcb_t * tcb_array;
static void SECTION (".init") init_bootmem (void)
{
    memdesc_t *mdesc;
    bool r;
    addr_t start_bootmem_phys, end_bootmem_phys;

    /*
     * Search the memory descriptors for our kernel memory pool
     * which was provided by the buildtool/bootloader
     */
    for (word_t i = 0; i < memory_info_num_desc_get(get_kip()->memory_info);
         i++) {
        mdesc = memory_info_descriptor_get(get_kip()->memory_info, i);

        if((memdesc_type_get(*mdesc) != memdesc_type_arch) ||
           (memdesc_subtype_get(*mdesc) != 0) || memdesc_is_virtual(*mdesc)) {
            continue;
        }
        start_bootmem_phys =
            addr_align_up((addr_t)memdesc_low_get(*mdesc), MIPS_PAGE_SIZE);
        end_bootmem_phys =
            addr_align((addr_t)(memdesc_high_get(*mdesc) + 1), MIPS_PAGE_SIZE);
    }

    addr_t start_bootmem = phys_to_virt(start_bootmem_phys);
    addr_t end_bootmem = phys_to_virt(end_bootmem_phys);
    tcb_array = (tcb_t *) phys_to_virt(tcb_array);

    /* feed the kernel memory allocator */
    kmem.init (start_bootmem, end_bootmem);

    /* Register reservations in kernel info page. */

    // Define the user's virtual address space.
    r = memory_info_descriptor_insert(kip_memory_info_get(get_kip()),
                                      memdesc_type_conventional, true,
                                      (addr_t)0,
                                      (addr_t)(1ULL <<
                                               CONFIG_MIPS_USER_ADDRESS_BITS));
    ASSERT(ALWAYS, r);

    /* XXX this needs to be fixed if a plat does not have mem at 0 */
    // Define the area reserved for the exception vectors.
    r = memory_info_descriptor_insert(kip_memory_info_get(get_kip()),
                                      memdesc_type_reserved, false, 
                                      (addr_t)0, (addr_t)MIPS_PAGE_SIZE);
    ASSERT(ALWAYS, r);
#ifdef CONFIG_SMP
    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
            (addr_t)0x1800000, (addr_t)0x1840000);
#endif
}

#if defined(CONFIG_SMP)
void SECTION (".init") init_processors()
{
    word_t smp_cpu;
    smp_cpu = 1;

    while (mips64_is_processor_available(smp_cpu))
    {
        if (smp_cpu > CONFIG_SMP_MAX_CPUS)
        {
            printf("found more CPUs than Pistachio supports\n");
            spin_forever();
        }
        cache_t::flush_cache_all();
        mips64_start_processor(smp_cpu);
        if (! mips64_wait_for_processor (smp_cpu))
            printf ("Failed to start processor %d\n", smp_cpu);
        smp_cpu ++;
    }
}
#endif
#if 0
static void SECTION (".init") finalize_cpu_init (word_t cpu_id)
{
#if defined(CONFIG_VERBOSE_INIT) || defined(CONFIG_MDOMAINS)
    scheduler_domain_t cpuid = cpu_id;
#endif

#if defined(CONFIG_MDOMAINS)
    // Mark CPU as being active
    mips64_processor_online(cpuid);

    if (cpuid == 0)
        init_processors ();
#endif
    TRACE_INIT("Finalized CPU %d\n", cpuid);
}
#endif
/*
 * Setup MIPS CPU
 */
extern "C" void init_cpu(void);

/*
 * Setup the Page tables and ASIDs
 */
extern "C" void SECTION(".init") init_pageing(void)
{
    /* Create and init kernel space */
    init_kernel_space();
}

extern word_t _start_cpu_local;

#if defined(CONFIG_MDOMAINS)
void SECTION (".init") init_cpulocal(int cpuid)
{
    int wired = 1, index = 0, i;
    word_t pagemask = (0xf<<12);
    word_t entryhi, entrylo0, entrylo1;

    TRACE_INIT("Initialize CPU Local (%d)\n", cpuid);

    // we make a 16kb cpu local area
    __asm__ __volatile__ (
        "mtc0 %0,"STR(CP0_WIRED)"\n\t"
        :: "r" (wired)
    );

    __asm__ __volatile__ (
        "mtc0 %0,"STR(CP0_PAGEMASK)"\n\t"
        :: "r" (pagemask)
    );

    entryhi = (3ul<<62) | (((word_t)0xffffffffc0000000ul>>13)<<13);
    entrylo0 = ((((0x1800000+0x40000*cpuid)&0xfffffff)>>12)<<6) | (3<<3) | 7; // XXX
    entrylo1 = 1;

    __asm__ __volatile__ (
        "mtc0  %0,"STR(CP0_INDEX)"\n\t"
        "dmtc0 %1,"STR(CP0_ENTRYHI)"\n\t"
        "dmtc0 %2,"STR(CP0_ENTRYLO0)"\n\t"
        "dmtc0 %3,"STR(CP0_ENTRYLO1)"\n\t"
        "nop;nop;nop;\n\t"
        "tlbwi\n\t"
        :: "r" (index), "r" (entryhi), "r" (entrylo0), "r" (entrylo1)
    );

    __asm__ __volatile__ (
        "nop;nop;nop;\n\t"
        "mtc0 $0,"STR(CP0_ENTRYHI)"\n\t"
        "mtc0 %0,"STR(CP0_PAGEMASK)"\n\t"
        :
        : "r" (CONFIG_MIPS_PAGEMASK_4K)
    );

    /* Zero out region */
    for (i = 0; i < 0x4000; i++)
        *(char*)(0xffffffffc0000000+i) = 0;
}
#endif

/*
 * Setup MIPS Architecture
 */
extern "C" void SECTION(".init") init_arch(void)
{
    TRACE_INIT("MIPS Processor: %d bits/word\n", BITS_WORD);

    init_tlb();

    /* configure IRQ hardware - global part */
    mips_init_intctrl();

#if defined(CONFIG_SMP)
    init_cpulocal(0);
#endif

    get_asid_cache()->init();
    get_asid_cache()->set_valid(0, CONFIG_MAX_NUM_ASIDS-1);

    init_bootmem();

    /* initialize kernel interface page */
    kip_init();

    /* Initialise TCB memory allocator */
    init_tcb_allocator();

    /* Allocate spaceid translation table */
    init_spaceids();

    /* initialise page tables */
    init_pageing();

    /* initialize kernel debugger if any */
    if (kdebug_init_t kdebug_init = kip_kdebug_init_get()) {
        kdebug_init();
    }

    init_idle_tcb();
    get_idle_tcb()->arch.int_mask = 0;

    /* configure IRQ hardware - local part */
    mips_init_local_intctrl();

    /*
     * Initialize the kernel's timer source.
     * This architecture does not need to keep track of the timer tick
     * length.
     */
    Platform::init_clocks();

#if defined(CONFIG_SMP)
    init_xcpu_handling(0);
#endif
}

NORETURN void SECTION(".init") generic_init();

/*
 * Entry point from ASM into C kernel
 * Precondition: paging is initialized with init_paging
 */

extern "C" void SECTION(".init")
PlatformSupport::arch_init(void* a0)
{
    init_cpu_cache();

    /*
     * Initialise the platform, map the console and other memory.
     *
     * If something goes wrong then loop forever.
     */
    if (!Platform::init(Platform::API_VERSION, PlatformSupport::API_VERSION))
        for(;;);

    init_console();
    /* Indicate if we support KDB console */
#if defined(CONFIG_KDB_CONS)
    get_kip()->kdebug_config[0] |= KDB_FEATURE_CONSOLE;
#endif

    init_hello();

    init_arch();

    dump_info();

    /* Architecture independent initialisation. Should not return. */
    generic_init();

    NOTREACHED();
}

#if defined(CONFIG_SMP)

extern "C" void SECTION (".init") startup_cpu (cpuid_t cpuid)
{
    init_cpu();
    init_cpu_cache();

    TRACE_INIT("CPU %d startup\n", cpuid);

    init_tlb();
    init_cpulocal(cpuid);
    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
            (addr_t)(0x1800000ul+(0x40000*cpuid)), (addr_t)(0x1840000ul+(0x40000*cpuid)));

    get_asid_cache()->init();
    get_asid_cache()->set_valid(0, CONFIG_MAX_NUM_ASIDS-1);

    get_interrupt_ctrl()->init_cpu();
    get_timer()->init_cpu();
    init_xcpu_handling (cpuid);

    get_current_scheduler()->init (false);

    get_idle_tcb ()->notify (finalize_cpu_init, cpuid);

    get_current_scheduler()->start (cpuid);

    spin_forever(1);
}

#endif
