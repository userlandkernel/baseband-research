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
 * Copyright (c) 2005-2006, National ICT Australia
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
 * Description:   configuration of IA32 architecture
 */
#ifndef __GLUE__V4_IA32__CONFIG_H__
#define __GLUE__V4_IA32__CONFIG_H__

#include <kernel/arch/offsets.h>
#include <l4/config.h>
#include <kernel/arch/ia32.h> /* IA32 defines (cache, pagesize) */


/**********************************************************************
 *                  Kernel interface page values
 **********************************************************************/

/* configuration for KTCBs */
#define KTCB_ALIGN              32

#define CONFIG_UTCB_SIZE        12

/* configuration for UTCBs */
#define UTCB_AREA_PGSIZE        pgent_t::size_4k
#define UTCB_AREA_PAGESIZE      (IA32_PAGE_SIZE)

#define SMALL_OBJECT_BLOCKSIZE          (IA32_PAGE_SIZE)

/* use half a page for the kernel stack */
#define STACK_SIZE              (IA32_PAGE_SIZE>>1)
#define STACK_TOP               (STACK_SIZE-4)
/* test that the sp is within 16 bytes of the top of the stack */
#define STACK_TEST              (STACK_TOP & ~(0xf))


/**********************************************************************
 *                  Virtual Address Space Layout
 **********************************************************************/

#define         VIRT_ADDR_BASE          0xF0000000
#define         VIRT_ADDR_RAM           VIRT_ADDR_BASE

/* user area */
#define USER_AREA_START         __UL(0x00000000)
#define USER_AREA_END           __UL(0xC0000000)

/* small space area */
#define SMALLSPACE_AREA_START   (USER_AREA_END)
#define SMALLSPACE_AREA_SIZE    __UL(0x10000000)
#define SMALLSPACE_AREA_END     (SMALLSPACE_AREA_START + SMALLSPACE_AREA_SIZE)

/* unused areas, 8MB each */
#define UNUSED_AREA_COUNT       1
#define UNUSED_AREA_START       (SMALLSPACE_AREA_END)
#define UNUSED_AREA_SIZE        __UL(0x00800000)
#define UNUSED_AREA_END         (UNUSED_AREA_START + (UNUSED_AREA_COUNT * UNUSED_AREA_SIZE))

/* readmem_phys remap area */
#define MEMREAD_AREA_START      (UNUSED_AREA_END)

/* abused some entries */
#define KIP_AREA_INFO           (MEMREAD_AREA_START + IA32_PAGEDIR_SIZE)
#define UTCB_AREA_INFO          (KIP_AREA_INFO + IA32_PAGEDIR_SIZE)
#define THREAD_COUNT            (UTCB_AREA_INFO + IA32_PAGEDIR_SIZE)
#define SMALL_SPACE_ID          (THREAD_COUNT + IA32_PAGEDIR_SIZE)
#define SEGDESC_LOW             (SMALL_SPACE_ID + IA32_PAGEDIR_SIZE)
#define SEGDESC_HIGH            (SEGDESC_LOW + IA32_PAGEDIR_SIZE)

#define PAGEDIR_STUFF_END       (THREAD_COUNT + IA32_PAGEDIR_SIZE)

/* V4 UTCB addressed via %gs:0 */
#define MYUTCB_MAPPING          __UL(0xDF000000)

/* device memory */
#define APIC_MAPPINGS           (MYUTCB_MAPPING + IA32_PAGE_SIZE)
#define VIDEO_MAPPING           (0xb8000)

/* EFI mappings */
#ifdef CONFIG_EFI
#define EFI_MAPPINGS            (APIC_MAPPINGS + IA32_PAGE_SIZE)
#define EFI_MAPPINGS_SIZE       (16)
#define DEVICE_NEXT             (EFI_MAPPINGS + (EFI_MAPPINGS_SIZE*IA32_PAGE_SIZE))
#else
#define DEVICE_NEXT             (APIC_MAPPINGS + IA32_PAGE_SIZE)
#endif

#ifdef CONFIG_KDB_CONS_DBG1394
/* Firewire address */
#define DBG1394_MAPPINGS        (DEVICE_NEXT)
#endif

/* KERNEL_AREA
 * synched on AS creation, so include all necessary regions!!! */
#define KERNEL_AREA_START       (MYUTCB_MAPPING)
#define KERNEL_AREA_END         (0xFF000000UL)

/* address of UTCB and KIP for root servers, at end of user-AS */
#define ROOT_UTCB_START         (0xBF000000)
#define ROOT_KIP_START          (0xBFF00000)

/* startup address for application processors */
#define SMP_STARTUP_ADDRESS     (0x4000)

/* some additional memory for the kernel*/
#define ADDITIONAL_KMEM_SIZE    (0x04000000)

/* defines the granularity kernel code and data is mapped */
#define KERNEL_PAGE_SIZE        (IA32_SUPERPAGE_SIZE)

/* Cache configuration */
#define CACHE_LINE_SIZE         (IA32_CACHE_LINE_SIZE_L2)


/* tracebuffer size */
#define TBUFF_SIZE (CONFIG_TRACEBUF_PAGES * IA32_PAGE_SIZE)


/**********************************************************************
 *          Architectural defines (segments, interrupts, etc.)
 **********************************************************************/

#define SEG_IDX(x)              ((x) >> 3)
#define SEGDESC(idx, rpl, l)    (((idx) << 3) | (rpl) | (l << 2))

/* Segment register values */
#define IA32_INV                SEGDESC(0, 0, 0)
#define IA32_KCS                SEGDESC(1, 0, 0)
#define IA32_KDS                SEGDESC(2, 0, 0)
#define IA32_UCS                SEGDESC(3, 3, 0)
#define IA32_UDS                SEGDESC(4, 3, 0)
#define IA32_LDT                SEGDESC(5, 0, 0)
#define IA32_TSS                SEGDESC(6, 0, 0)
#define IA32_KDB                SEGDESC(7, 0, 0)
#define IA32_UTCB               SEGDESC(9, 3, 0)

/*
 * User LDT entries at the front.  System LDT entries at the end.  The
 * IA32_UTLS is here for symbolic reasons, the function to set the 
 * LDT treats it the same.  The UTLS entry is at the last of the 
 * user settable entries.  This is for the L4_Set_Tls() convenience call.
 */
#define LDT_SIZE                10    /* 10 entries all up */
#define SYS_LDT_ENTRIES         1     /* none at the moment */
#define USER_LDT_ENTRIES        (LDT_SIZE - SYS_LDT_ENTRIES)
#define IA32_UTLS               (USER_LDT_ENTRIES - 1)

#define IA32_KERNEL_FLAGS       (IA32_EFL_IOPL(0) | 2)

/* user mode e-flags */
#define IA32_USER_FLAGS         (IA32_EFL_IOPL(3) | IA32_EFL_IF | 2)
#define IA32_USER_FLAGMASK      (IA32_EFL_CF | IA32_EFL_PF | IA32_EFL_AF | IA32_EFL_ZF | IA32_EFL_SF | IA32_EFL_OF)

/* IDT, GDT, etc. */
#define IDT_SIZE                128
#define GDT_SIZE                16

/* global IDT entries */
#define IDT_LAPIC_SPURIOUS_INT  0x3f /* spurious int vector must
                                      * have lowermost 4 bits set */
#define IDT_LAPIC_TIMER         0x40
#define IDT_LAPIC_THERMAL       0x41
#define IDT_LAPIC_XCPU_IPI      0x42
#define IDT_IOAPIC_BASE         0x44

/* Page size for APIC and ACPI mappings */
#define APIC_PGENTSZ            pgent_t::size_4k
#define ACPI_PGENTSZ            pgent_t::size_4m

#define EXC_INTERRUPT(name)     IA32_EXC_NO_ERRORCODE(name, 0)

/* timer frequency */
#ifdef CONFIG_IOAPIC
# define TIMER_TICK_LENGTH      (CONFIG_APIC_TIMER_TICK)
#else
/* 1.953ms per timer tick
 * VU: the hardware clock can only be configured to tick in 2^n Hz
 * 1000 / 512 Hz = 1.953125 ms/tick */
# define TIMER_TICK_LENGTH      (1953)
#endif

/* enable synchronous XCPU-requests, for TLB shoot-downs */
#define CONFIG_SMP_SYNC_REQUEST



/**********************************************************************
 *                       kernel stack layout
 **********************************************************************/

#define KSTACK_USP              (-2)
#define KSTACK_UFLAGS           (-3)
#define KSTACK_UIP              (-5)
#define KSTACK_RET_IPC          (-6)

#endif /* !__GLUE__V4_IA32__CONFIG_H__ */
