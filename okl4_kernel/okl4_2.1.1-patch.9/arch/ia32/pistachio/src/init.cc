/*
 * Copyright (c) 2002, 2004-2003, Karlsruhe University
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
 * Description:   ia32-specific initialization
 */

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <ctors.h>
#include <memdesc.h>
#include <l4/memregion.h>

/* cpu specific types */
#include <arch/cpu.h>

/* pagetable and mmu management */
#include <arch/mmu.h>
#include <arch/ptab.h>

/* idt, tss, gdt etc. */
#include <arch/segdesc.h>
#include <arch/sysdesc.h>
#include <arch/tss.h>

/* floating point unit */
#include <arch/fpu.h>

#include <arch/config.h>
#include <arch/idt.h>
#include <space.h>
#include <arch/memory.h>
#include <arch/timer.h>
#include <arch/syscalls.h>
#include <arch/platform.h>
#include <arch/platsupport.h>
#include <arch/init.h>

#include <smp.h>
#include <l4/config.h>
#include <tcb.h>
#include <intctrl.h>
#include <interrupt.h>
#include <schedule.h>

#include <plat/rtc.h>
#include <generic/lib.h>

extern void init_spaceids(void);
extern "C" void sysenter_entry(void);

#ifdef CONFIG_EFI
#include <plat/apple_efi.h>
#else
#include <arch/bootdesc.h>
extern "C" struct ia32_bootdesc* SECTION(".init") init_platform_mem(void);
#endif

/* = { 0 } necessary because SECTION only sorta works on data :( */
static word_t init_pdir[1024] ALIGNED(4096) SECTION(SEC_INIT_DATA) = { 0 };

#define MAX_KERNEL_MAPPINGS     32
#define PAGE_ATTRIB_INIT        (IA32_PAGE_VALID | IA32_PAGE_WRITABLE)
// 2nd-level page tables for the initial page table
/*lint -esym(551,init_ptable) It doesn't look like we use this but we really do */
static word_t init_ptable[MAX_KERNEL_MAPPINGS][1024] ALIGNED(4096) SECTION(SEC_INIT_DATA) = { { 0 } };

/* physical memory regions for major kernel bits */
/* currently we have one region which covers the whole
 * binary: text, data, bootmem  (not init, though)
 */
mem_region_t kernel_image_rgn = { 0, 0 };


/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/
#if defined(CONFIG_SMP)
extern "C" void _start_ap(void);
spinlock_t smp_boot_lock;

/* commence to sync TSC */
static void smp_bp_commence();
spinlock_t smp_commence_lock;


ia32_segdesc_t  smp_boot_gdt[3];
static void setup_smp_boot_gdt()
{
#   define gdt_idx(x) ((x) >> 3)
    smp_boot_gdt[gdt_idx(IA32_KCS)].set_seg(0, ~0, 0, ia32_segdesc_t::code);
    smp_boot_gdt[gdt_idx(IA32_KDS)].set_seg(0, ~0, 0, ia32_segdesc_t::data);
#   undef gdt_idx
}

INLINE u8_t get_apic_id()
{
    local_apic_t<APIC_MAPPINGS> apic;
    return apic.id();
}
#endif



/**********************************************************************
 *
 *  processor local initialization, performed by all IA32 CPUs
 *
 **********************************************************************/

/* processor local data */
ia32_segdesc_t  gdt[GDT_SIZE] UNIT("ia32.cpulocal");
ia32_tss_t      tss UNIT("ia32.cpulocal");

static void setup_gdt(ia32_tss_t & tss, scheduler_domain_t cpuid)
{
#   define gdt_idx(x) ((x) >> 3)

    gdt[gdt_idx(IA32_KCS)].set_seg(0, ~0, 0, ia32_segdesc_t::code);
    gdt[gdt_idx(IA32_KDS)].set_seg(0, ~0, 0, ia32_segdesc_t::data);
    gdt[gdt_idx(IA32_UCS)].set_seg(0, ~0, 3, ia32_segdesc_t::code);
    gdt[gdt_idx(IA32_UDS)].set_seg(0, ~0, 3, ia32_segdesc_t::data);

    /* MyUTCB pointer,
     * we use a separate page for all processors allocated in space_t
     * and have one UTCB entry per cache line in the SMP case */
    ASSERT(NORMAL, unsigned(cpuid * CACHE_LINE_SIZE) < IA32_PAGE_SIZE);
    gdt[gdt_idx(IA32_UTCB)].set_seg((u32_t)MYUTCB_MAPPING +
                                    (cpuid * CACHE_LINE_SIZE),
                                    sizeof(threadid_t) - 1,
                                    3, ia32_segdesc_t::data);

    /* the TSS
     * The last byte in ia32_tss_t is a stopper for the IO permission bitmap.
     * That's why we set the limit in the GDT to one byte less than the actual
     * size of the structure. (IA32-RefMan, Part 1, Chapter Input/Output) */
    gdt[gdt_idx(IA32_TSS)].set_sys((u32_t)&tss, sizeof(tss)-1,
                                   0, ia32_segdesc_t::tss);
}

static void SECTION(SEC_INIT_CPU) activate_gdt();

/**
 * activate_gdt: activates the previously set up GDT
 */
static void SECTION(SEC_INIT_CPU) activate_gdt()
{
    TRACE_INIT("%s\n", __FUNCTION__);

    /* create a temporary GDT descriptor to load the GDTR from */
    /*lint -e529 gdt_desc is only used inside __asm__ blocks */
    ia32_sysdesc_t gdt_desc = {sizeof(gdt), (u32_t)gdt, 0} ;

    __asm__ __volatile__("lgdt %0               \n"     /* load descriptor table        */
                         "ljmp  %1,$1f          \n"     /* refetch code segment descr.  */
                         "1:                    \n"     /*   by jumping across segments */
                         :
                         : "m"(gdt_desc), "i" (IA32_KCS));

    /* set the segment registers from the freshly installed GDT
       and load the Task Register with the TSS via the GDT */
    /*
     * We need to load some sort of %gs, because we could take a trap/intr
     * from kernel mode which pushes the %gs and pops the %gs.  The
     * %gs is not actually used in the kernel for now, it is anticipated
     * that either %fs or %gs will be used for the cpulocal anchor if that
     * is implemented.
     */
    __asm__ __volatile__("mov  %0, %%ds         \n"
                         "mov  %0, %%es         \n"
                         "mov  %0, %%ss         \n"
                         "mov  %0, %%fs         \n"
                         "mov  %0, %%gs         \n"
                         "movl %1, %%eax        \n"
                         "ltr  %%ax             \n"
                         :
                         : "r" (IA32_KDS), "r" (IA32_TSS)
                         : "eax"
                         );
}

/**
 * setup_msrs: initializes all model specific registers for CPU
 */
static void setup_msrs()
{
    ia32_wrmsr(IA32_SYSENTER_CS_MSR, (u32_t)(IA32_KCS));
    ia32_wrmsr(IA32_SYSENTER_EIP_MSR, (u32_t)(sysenter_entry));
    ia32_wrmsr(IA32_SYSENTER_ESP_MSR, (u32_t)stack_top);
    ia32_fpu_t::enable_osfxsr();
    ia32_fpu_t::disable();
}

/**
 * checks the IA32 features (CPUID) to make sure the processor
 * has all necessary features */
static void SECTION(SEC_INIT_CPU) check_cpu_features()
{
    u32_t req_features = IA32_FEAT_FPU;
    req_features |= IA32_FEAT_PSE;
    req_features |= IA32_FEAT_PGE;
    req_features |= IA32_FEAT_FXSR;
#ifdef CONFIG_IOAPIC
    req_features |= IA32_FEAT_APIC;
#endif
    u32_t avail_features = ia32_get_cpu_features();

    if ((req_features & avail_features) != req_features) {
        printf("CPU does not support all features (%x) -- halting\n", req_features);
#if defined(CONFIG_VERBOSE_INIT)
        const char* ia32_features[] = {
            "fpu",  "vme",    "de",   "pse",   "tsc",  "msr", "pae",  "mce",
            "cx8",  "apic",   "?",    "sep",   "mtrr", "pge", "mca",  "cmov",
            "pat",  "pse-36", "psn",  "cflsh", "?",    "ds",  "acpi", "mmx",
            "fxsr", "sse",    "sse2", "ss",    "ht",   "tm",  "ia64", "pbe" };
        for (int i = 0; i < 32; i++)
            if ((req_features & 1 << i) && (!(avail_features & 1 << i)))
                printf("%s ", ia32_features[i]);
        printf("missing\n");
#endif
        spin_forever();
    }
}

/**
 * init_cpu: initializes the processor
 *
 * this function is called once for each processor to initialize
 * the processor specific data and registers
 */
static scheduler_domain_t SECTION(SEC_INIT_CPU) init_cpu()
{
    scheduler_domain_t cpuid = 0;

    /* configure IRQ hardware - local part
     * this has to be done before reading the cpuid since it may change
     * when having one of those broken BIOSes like ServerWorks */
    ia32_init_local_intctrl();

#if defined(CONFIG_SMP)
    cpuid = get_apic_id();
#endif

    /* initialize the CPU specific mappings */
    get_kernel_space()->init_cpu_mappings(cpuid);

    // call cpu ctors
    // call_cpu_ctors();

    tss.setup(IA32_KDS);
    setup_gdt(tss, cpuid);
    activate_gdt();

    /*lint -e641 */
    TBUF_RECORD_EVENT(IA32_INIT);

    /* can take exceptions from now on,
     * idt is initialized via a constructor */
    idt.activate();

    /* activate msrs */
    setup_msrs();

    return cpuid;
}



/***********************************************************************
 *
 *          system specific initialization and global data
 *
 **********************************************************************/


/**
 * setup_idt: initializes the interrupt descriptor table
 */

#ifndef CONFIG_EFI
/** Find the kernel heap. */
static addr_t SECTION(SEC_INIT)
find_heap(addr_t * limit)
{
    const memdesc_t * mdesc;

    /*
     * Search the memory descriptors for our kernel memory pool
     * which was provided by the buildtool/bootloader
     */
    const memdesc_set_t * mds = &get_init_data()->memory_descriptor_set;

    for (word_t i = 0; i < mds->num_descriptors; i++) {
        addr_t high, low;
        mdesc = &mds->descriptors[i];

        if((memdesc_type_get(*mdesc) != memdesc_type_arch) ||
           (memdesc_subtype_get(*mdesc) != 0) || memdesc_is_virtual(*mdesc)) {
            continue;
        }
        low = addr_align_up((addr_t)memdesc_low_get(*mdesc),
                            IA32_PAGE_SIZE);
        high = addr_align((addr_t)memdesc_high_get(*mdesc),
                          IA32_PAGE_SIZE);
        printf("Found l4 kernel heap! at [%p, %p)\n", low, high);
        *limit = high;

        return low;
    }
    printf("Did not find l4 kernel heap!\n");
    return INVALID_ADDR;
}
#endif

extern word_t num_tcbs;
extern tcb_t * tcb_array;

static addr_t heap_base;
static addr_t heap_limit;

/**
 * Initializes boot memory.
 *
 * At system startup, a fixed amount of kernel memory is allocated
 * to allow basic initialization of the system before roottask is up.
 */
static void SECTION(SEC_INIT)
init_bootmem(u32_t kaddr, u32_t ksize)
{
    bool r;
    TRACE_INIT("Initializing boot memory (%p - %p)\n",
               start_bootmem, end_bootmem);
    kmem.init(start_bootmem, end_bootmem);

    /** setup the memory regions **/
    /* We assume the kernel starts at start_text_phys and ends at
     * the end of the bootmem. It's up to the linker to make sure of this.
     */
    printf("kimg: 0x%x - 0x%x\n",  start_text_phys, end_bootmem_phys);
    mem_region_set(&kernel_image_rgn, (word_t)start_text_phys,
                   (word_t)end_bootmem_phys);

    /* FIXME: insert additional kmem pc99 style?
     *        or do something sane with the args?
     */

#ifdef CONFIG_EFI
    {
        /* On EFI systems we don't come via kickstart, so we need to setup
         * additional kmem more differently
         */
        word_t end = (word_t)(kaddr + ksize + ADDITIONAL_KMEM_SIZE);

        r = get_kip()->memory_info.insert(memdesc_t::reserved, false,
                (addr_t)(end - ADDITIONAL_KMEM_SIZE), (addr_t)(end - 1));
        ASSERT(ALWAYS, r);
    }
#else
    heap_base = find_heap(&heap_limit);

    /* Map the kernel heap to its virtual/pagetable address */
    printf("pc99 heap [%p, %p)\n", heap_base, heap_limit);

    /**
     *  @todo FIXME: assert that range is IA32_SUPERPAGE_SIZE aligned
     *  and sized.
     */
    for (word_t i = (word_t)heap_base; i < (word_t)heap_limit;
            i += IA32_SUPERPAGE_SIZE) {
        printf("mapping heap at 0x%lx \n", i);
        r = get_kernel_space()->add_mapping((addr_t)i, (addr_t)i,
                pgent_t::size_4m, space_t::read_write, true, false, cached);
        ASSERT(ALWAYS, r);
        r = get_kernel_space()->add_mapping((addr_t)i, (addr_t)i,
                pgent_t::size_4m, space_t::read_write, true, false, writethr);
        ASSERT(ALWAYS, r);
    }
#endif
}

static void SECTION(SEC_INIT)
add_more_kmem(void)
{
    addr_t base = phys_to_virt(heap_base);
    addr_t end = phys_to_virt(heap_limit);
    word_t size = (word_t)heap_limit - (word_t)heap_base;

    /* Map kernel heap writable. */
    get_kernel_space()->remap_area(base, heap_base, pgent_t::size_4m, size,
            space_t::read_write, true, true);

    /** @todo FIXME: Replace with memzero - awiggins. */
    {
        char * index = (char *)base;

        printf("zero base %p to end %p\n", base, end);
        while(index < end) {
            *index = 0;
            index++;
        }
        printf("done zero\n");
    }

    /* Add to allocator */
    kmem.add(base, size);

    /* Add tcb area mapping in the kernel space */
    get_kernel_space()->remap_area(phys_to_virt(tcb_array), tcb_array,
            pgent_t::size_4m, ((KTCB_SIZE * num_tcbs) +
            (KERNEL_PAGE_SIZE - 1)) & ~(KERNEL_PAGE_SIZE - 1),
            space_t::read_write, true, true);
    /* Change global tcb area base to its virtual address */
    tcb_array = phys_to_virt(tcb_array);
}


/* function to clear bss. standard ELF, else call out */
#ifdef __ELF__

static void clear_bss()
{
    extern u8_t _bss_start[];
    extern u8_t _bss_end[];
    for (u8_t* p = _bss_start; p < _bss_end; p++)
        *p = 0;
}

#else
extern void clear_bss();
#endif


#ifdef CONFIG_EFI
/*
 * The boot.efi bootloader does not create memory descriptors for us.
 * The Legion(1L) program reserves the amount of memory we need to reserve
 * as bootloader specific memory.  Hence, what we need to do is tack
 * on the rest of the memory here.  This code is mainly taken from
 * the fwload bootloader.
 */
static void
make_memdescs(
    uint32_t efi_mmap_ptr,
    uint32_t efi_mmap_size,
    uint32_t efi_md_size,
    uint32_t efi_md_version,
    uint32_t kaddr,
    uint32_t ksize)
{
    EfiMemoryRange  *md;
    memory_info_t   *mi;
    memdesc_t       l4md;
    uint64_t        i, low, high, type, plow, phigh, ptype;
    int             insert;

    mi = &get_kip()->memory_info;
    //mi->n = 0;

    if (efi_md_version < 0x1/*XXX: EFI_MEMORY_DESCRIPTOR_VERSION*/) {
        printf("%s: wrong version got %d expected 0x1\n",
               __func__, efi_md_version);
        while (1)
            ;
    }

    /*
     * Borrowed from ia64 fbsd loader patches
     */
    ptype = plow = phigh = ~0UL;

    while (i < efi_mmap_size) {
        md = (EfiMemoryRange *)(efi_mmap_ptr + i);
        insert = 1;
        low = md->PhysicalStart;
        high = md->PhysicalStart +
            md->NumberOfPages * 0x1000/*XXX: pagesiz -gl */ - 1;
        switch (md->Type) {
        case kEfiACPIReclaimMemory:
        case kEfiACPIMemoryNVS:
        case kEfiRuntimeServicesCode:
        case kEfiRuntimeServicesData:
            /* XXX: fixme */
            /* type = L4_ACPIMemoryType */
            type = 0x1f;
            break;
        case kEfiUnusableMemory:
        case kEfiMemoryMappedIO:
        case kEfiPalCode: /* probably need this... */
            type = 0x3f;
            break;
        case kEfiLoaderCode:
        case kEfiLoaderData:
        case kEfiBootServicesCode:
        case kEfiBootServicesData:
        case kEfiConventionalMemory:
            type = 0x1;     /* XXX: conventional */
            break;
        default:
            type = 0x0;     /* XXX: undefined */
            break;
        }

        /* Concatenate with end of previous item */
        if (low == phigh + 1 && type == ptype) {
            phigh = high;
            insert = 0;
        }

        /*
         * Concatenate with start of previous item
         */
        if (high + 1 == plow && type == ptype) {
            plow = low;
            insert = 0;
        }

        if (insert) {
            if (ptype != ~0UL) {
                l4md.set((memdesc_t::type_e)(ptype & 0xf),
                         (word_t)((ptype >> 4) & 0xf),
                         false, (addr_t)plow, (addr_t)phigh);
                mi->insert(l4md.type(), l4md.subtype(),
                           l4md.is_virtual(), l4md.low(),
                           l4md.high());
            }
            plow = low;
            phigh = high;
            ptype = type;
        }

        i += efi_md_size;
    }

    /*
     * Finally, insert the boot image.  From experimentation,
     * the boot.efi module seems to reserve everything from the
     * actual image to the boot arguments and the device tree.
     *
     * Not yet sure about the boot modules but I suspect they are there
     * too XXX check XXX -gl
     */
    l4md.set((memdesc_t::type_e)0xe/*bootspecific*/,
             (word_t)0x0, false, (addr_t)kaddr, (addr_t)(kaddr + ksize - 1));
    mi->insert(l4md.type(), l4md.subtype(), l4md.is_virtual(),
               l4md.low(), l4md.high());
}

static int
strcmp(const char *x, const char *y)
{
    int     i, diff;

    for (i = 0;; i++) {
        diff = ((unsigned char *)x)[i] - ((unsigned char *)y)[i];
        if (diff != 0 || x[i] == '\0')
            return (diff);
    }
}

static void
patch_bootinfo(uint32_t addr, uint32_t size, uint32_t devtree, uint32_t devsiz)
{
#define L4_MODULE                       0x1
#define L4_BOOTINFO_MAGIC               ((word_t) 0x14b0021d)
    typedef struct {
        word_t  type;
        word_t  version;
        word_t  offset;
        word_t  start;
        word_t  siz;
        word_t  cmdline_offset;
    } bootrec_t;

    typedef struct {
        word_t  magic;
        word_t  version;
        word_t  siz;
        word_t  first_entry;
        word_t  num_entries;
        word_t  reserved[3];
    } bootinfo_t;

    bootinfo_t      *bi = (bootinfo_t *)(get_kip()->boot_info);
    bootrec_t       *rec;
    word_t          i;

    if (bi->magic != L4_BOOTINFO_MAGIC) {
        printf("invalid bootinfo");
        return;
    }

    for (i = 0, rec = (bootrec_t *)((word_t)bi + bi->first_entry); 
         i < bi->num_entries; 
         i++, rec = (bootrec_t *)((word_t)rec + rec->offset)) {
        if (rec->type != L4_MODULE)
            continue;
        if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                   "kernbootstruct") == 0) {
            /*
             * Found it.  Now we patch in the address of the
             * boot_args structure.
             */
            rec->start = addr;
            rec->siz = size;
        }
        if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                   "device_tree") == 0) {
            /*
             * Patch in device tree
             */
            rec->start = devtree;
            rec->siz = devsiz;
        }
        if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                   "drivers") == 0) {
            /*
             * Found the drivers bootinfo.  Patch that up.
             * Note that we rely on the fact that the drivers
             * get placed right after the boot args, and then
             * finally the device tree.
             *
             * The situation is not really ideal, but it will
             * do for now.
             *
             * -gl.
             */
            rec->start = ((addr + size) + (IA32_PAGE_SIZE - 1)) &
                ~(IA32_PAGE_SIZE - 1);
            rec->siz = devtree - rec->start;
        }

        for (i = 0, rec = (bootrec_t *)((word_t)bi + bi->first_entry); 
            i < bi->num_entries; 
            i++, rec = (bootrec_t *)((word_t)rec + rec->offset)) {
                if (rec->type != L4_MODULE)
                        continue;
                if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                    "kernbootstruct") == 0) {
                        /*
                         * Found it.  Now we patch in the address of the
                         * boot_args structure.
                         */
                        rec->start = addr;
                        rec->siz = size;
                }
                if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                    "device_tree") == 0) {
                        /*
                         * Patch in device tree
                         */
                        rec->start = devtree;
                        rec->siz = devsiz;
                }
                if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                    "drivers") == 0) {
                        /*
                         * Found the drivers bootinfo.  Patch that up.
                         * Note that we rely on the fact that the drivers
                         * get placed right after the boot args, and then
                         * finally the device tree.
                         *
                         * The situation is not really ideal, but it will
                         * do for now.
                         *
                         * -gl.
                         */
                        rec->start = ((addr + size) + (IA32_PAGE_SIZE - 1)) &
                                        ~(IA32_PAGE_SIZE - 1);
                        rec->siz = devtree - rec->start;
                }

                if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                    "efiruntime") == 0) {
                        /*
                         * Set efi runtime.  PageStart is the page number
                         * so we need to shift by the page.
                         */
                        rec->start = ((boot_args *)addr)->efiRuntimeServicesPageStart * IA32_PAGE_SIZE;
                        rec->siz = ((boot_args *)addr)->efiRuntimeServicesPageCount * IA32_PAGE_SIZE;
                }
                if (strcmp((char *)((word_t)rec + rec->cmdline_offset),
                    "efimemdesc") == 0) {
                        /*
                         * Set efi memory descriptors
                         */
                        rec->start = ((boot_args *)addr)->MemoryMap;
                        rec->siz = (((boot_args *)addr)->MemoryMapSize +
                            (IA32_PAGE_SIZE - 1)) & ~(IA32_PAGE_SIZE - 1);
                }
        }
    }
}

#endif /* CONFIG_EFI */


NORETURN void SECTION(".init") generic_init();
extern "C" void SECTION(SEC_INIT) init_paging();

#ifdef CONFIG_EFI
/* XXX: put in header? */
extern void init_efi( addr_t );
#endif

/**
 * startup_system: starts up the system
 *
 * precondition: paging is initialized with init_paging
 *
 * The startup happens in two steps
 *   1) all global initializations are performed
 *      this includes initializing necessary devices and
 *      the kernel debugger. The kernel memory allocator
 *      is set up (see init_arch).
 *
 *   2) the boot processor itself is initialized
 */
extern "C" void SECTION(".init") PlatformSupport::arch_init(void* v)
{
#ifdef CONFIG_EFI
    void *efi_systab;
    boot_args *args = (boot_args*) v;
#endif

    init_paging();

    clear_bss();

    /*
     * Initialise the platform, map the console and other memory.
     *
     * If something goes wrong then loop forever.
     */
    if (!Platform::init(Platform::API_VERSION, PlatformSupport::API_VERSION))
        for(;;);

    // Init the console
    init_console ();

    // what is this magic address?
    // *((char*) 0xa000000) = 0;

    /* system initialization - boot CPU's job */
    {
        /* mention ourself */
        init_hello ();

#ifdef __APPLE_CC__
        /* parse our macho kernel layout */
        init_macho_vars();
#endif

        /* first thing -- check CPU features */
        check_cpu_features();

        /* FIXME: find a more appropriate place for this block of code... */
#ifdef CONFIG_EFI
        /* setup the L4 memdescs */
        printf("EFI boot_args = %p\n", args);
        efi_systab = (void *)args->efiSystemTable;

        make_memdescs(args->MemoryMap,
                      args->MemoryMapSize,
                      args->MemoryMapDescriptorSize,
                      args->MemoryMapDescriptorVersion,
                      args->kaddr,
                      args->ksize);
        patch_bootinfo((uint32_t)args, sizeof(*args), args->deviceTreeP,
                       args->deviceTreeLength);
#endif

        /* feed the kernel memory allocator */
#ifdef CONFIG_EFI
        init_bootmem(args->kaddr, args->ksize);
#else
        init_bootmem(0,0);
#endif

        /* Allocate spaceid translation table */
        init_spaceids();

        /* initialize the kernel pagetable */
        init_kernel_space();

        // initialise the IDT
        idt.init_idt_t();

#ifdef CONFIG_EFI
        /* init EFI */
        init_efi( (addr_t) efi_systab );
#endif

        { /* copied here to catch errors early */
            tss.setup(IA32_KDS);
            setup_gdt(tss, 0);
            activate_gdt();
            idt.activate();
        }

        // b0rked
        add_more_kmem();

        /* Initialise TCB memory allocator */
        init_tcb_allocator();

        /* Initialise the tracebuffer */
        init_tracebuffer();

        init_idle_tcb();

        /* configure IRQ hardware - global part */
        ia32_init_intctrl();

        /* initialize timer - global part */
        Platform::init_clocks();

        /* initialize kernel debugger if any */
        if (kdebug_entries.kdebug_init) {
            kdebug_entries.kdebug_init();
        }

#if defined(CONFIG_MDOMAINS)
        /* start APs on an SMP + rendezvous */
        {
            TRACE_INIT("starting application processors (%p->%p)\n",
                       _start_ap, SMP_STARTUP_ADDRESS);

            // aqcuire commence lock before starting any processor
            smp_commence_lock.init(1);

            // boot gdt
            setup_smp_boot_gdt();

            // IPI trap gates
            init_xcpu_handling();

            // copy startup code to startup page
            for (word_t i = 0; i < IA32_PAGE_SIZE / sizeof(word_t); i++) {
                ((word_t*)SMP_STARTUP_ADDRESS)[i] = ((word_t*)_start_ap)[i];
            }
            /* at this stage we still have our 1:1 mapping at 0 */
            *((volatile unsigned short *) 0x469) = (SMP_STARTUP_ADDRESS >> 4);
            *((volatile unsigned short *) 0x467) = (SMP_STARTUP_ADDRESS) & 0xf;

            local_apic_t<APIC_MAPPINGS> local_apic;

            // make sure we don't try to kick out more CPUs we can handle
            int smp_cpus = 1;

            u8_t apic_id = get_apic_id();
            for (word_t id = 0; id < sizeof(word_t) * 8; id++)
                {
                    if (id == apic_id)
                        continue;

                    if ((get_interrupt_ctrl()->get_lapic_map() & (1 << id)) != 0)
                        {
                            if (++smp_cpus > CONFIG_SMP_MAX_CPUS)
                                {
                                    printf("found more CPUs than Pistachio supports\n");
                                    spin_forever();
                                }
                            smp_boot_lock.lock(); // unlocked by AP
                            TRACE_INIT("sending startup IPI to APIC %d\n", id);
                            local_apic.send_init_ipi(id, true);
                            for (int i = 0; i < 100000; i++);
                            local_apic.send_init_ipi(id, false);
                            local_apic.send_startup_ipi(id, (void(*)(void))SMP_STARTUP_ADDRESS);
                            /* #warning VU: time out on AP call in */
                        }
                }

        }
#endif

        /* local initialization - all are equal */
        (void) init_cpu ();

        TRACE_INIT("%s done\n", __FUNCTION__);

#ifdef CONFIG_MDOMAINS
        smp_bp_commence ();
#endif

        /* Architecture independent initialisation. Should not return. */
        generic_init();

        NOTREACHED();
    }
}
#if defined(CONFIG_SMP)
static void smp_ap_commence()
{
    smp_boot_lock.unlock();

    /* finally we sync the time-stamp counters */
    while( smp_commence_lock.is_locked() );

    ia32_settsc(0);
}

static void smp_bp_commence()
{
    // wait for last processor to call in
    smp_boot_lock.lock();

    // now release all at once
    smp_commence_lock.unlock();

    ia32_settsc(0);
}

/**
 * startup_processor
 */
extern "C" void SECTION(SEC_INIT) startup_processor()
{
    TRACE_INIT("AP processor is alive\n");
    ia32_mmu_t::set_active_pagetable((u32_t)get_kernel_space()->get_pdir());
    TRACE_INIT("AP switched to kernel ptab\n");

    // first thing -- check CPU features
    check_cpu_features();

    /* perform processor local initialization */
    scheduler_domain_t cpuid = init_cpu();

    get_current_scheduler()->init (false);
    get_idle_tcb()->notify (smp_ap_commence);
    get_current_scheduler()->start (cpuid);

    spin_forever(cpuid);
}
#endif

void add_init_root_mapping( void *vaddr );

/* Map a 4m section based on what some guy says */
void SECTION(SEC_INIT) add_init_root_mapping( void *vaddr )
{
    word_t foo, idx;

    foo = (word_t) vaddr;
    idx = foo >> IA32_PAGEDIR_BITS;

    /* add a 4m mapping for vaddr */
    /* XXX: super-pages? */
    init_pdir[idx] = (idx << IA32_PAGEDIR_BITS) | PAGE_ATTRIB_INIT | IA32_PAGE_SUPER;
}

/**
 * init_paging:  initializes the startup pagetable
 *
 * The startup-pagetable contains two regions, a physical 1:1 mapping
 * at virtual address 0 and upwards and a virtual mapping at the
 * virtual kernel address range.  These mappings are initialized as
 * 4MB mappings which reduces the needed memory to the page directory only.
 * After initializing the kernel memory allocator a new kernel pagetable
 * is set up which can be populated using kmem_alloc etc.
 */
extern "C" void SECTION(SEC_INIT) init_paging()
{
    /* zero pagetable */
    for (int i = 0; i < 1024; i++)
        init_pdir[i] = 0;

    /* Setup the initial mappings.  The first MAX_KERNEL_MAPPINGS*4MB
       are mapped 1:1.  The same region is also visible at
       KERNEL_RW_OFFSET */
    /* #warning "use virt_to_phys for page mappings "*/
    for (int i = 0; i < MAX_KERNEL_MAPPINGS; i++)
        {
            // Fill 2nd-level page table
            for (int j = 0; j<1024; j++)
                init_ptable[i][j] = ((i << IA32_PAGEDIR_BITS) |
                                     (j << IA32_PAGE_BITS) |
                                     PAGE_ATTRIB_INIT);
            // Install page table in page directory
            init_pdir[i] =
                init_pdir[(KERNEL_RW_OFFSET >> IA32_PAGEDIR_BITS) + i] =
                (word_t)(init_ptable[i]) | PAGE_ATTRIB_INIT;
        }

    /* now activate the startup pagetable */
    ia32_mmu_t::enable_super_pages();
    ia32_mmu_t::enable_global_pages();
    ia32_mmu_t::set_active_pagetable((u32_t)init_pdir);
    ia32_mmu_t::enable_paged_mode();
}
