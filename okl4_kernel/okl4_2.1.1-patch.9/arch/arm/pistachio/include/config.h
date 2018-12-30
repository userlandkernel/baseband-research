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
 * Description:   ARM Basic kernel config
 */

#ifndef __ARM__CONFIG_H__
#define __ARM__CONFIG_H__

#include <kernel/l4.h>
#include <kernel/arch/page.h>
#include <l4/arch/config.h>

#if defined(ASSEMBLY)
#define UL(x)   x
#else
#define UL(x)   x##UL
#endif

#define         VIRT_ADDR_BASE          0xF0000000
#define         VIRT_ADDR_PGTABLE       0xF4000000

#if defined(CONFIG_XIP)
#define         VIRT_ADDR_ROM           VIRT_ADDR_BASE
#define         VIRT_ADDR_RAM           (VIRT_ADDR_BASE + 0x01000000)
#else   /* !CONFIG_XIP */
#define         VIRT_ADDR_ROM           VIRT_ADDR_BASE
#define         VIRT_ADDR_RAM           VIRT_ADDR_BASE
#endif

#define KTCB_ALIGN      16


/**
 * Size of a UTCB in bytes
 */
#define UTCB_MASK       (~(UTCB_SIZE - 1))

#define UTCB_AREA_PAGESIZE      PAGE_SIZE_4K
#define UTCB_AREA_PAGEBITS      PAGE_BITS_4K
#define UTCB_AREA_PGSIZE        pgent_t::size_4k

/* Page size for small object allocator */
#define SMALL_OBJECT_BLOCKSIZE  PAGE_SIZE_4K

#define STACK_BITS      10
#define STACK_SIZE      (UL(1) << STACK_BITS)
#define STACK_SIZE_ASM  (1 << STACK_BITS)
#define STACK_MASK      (~(STACK_SIZE - 1))
/* to get the top of the stack logical or any valid sp with this mask */
#define STACK_TOP       (STACK_SIZE - 8)
#define STACK_TOP_ASM   (STACK_SIZE_ASM - 8)
/* on arm the stack top mask must be split in two for use in assembler */
#define STACK_TEST              (STACK_TOP)

/* ARM version specific config - including UTCB defines */
#include <kernel/arch/ver/config.h>

/* tracebuffer size */
#define TBUFF_SIZE (CONFIG_TRACEBUF_PAGES * PAGE_SIZE_4K)

/* memory layout
 *
 * The first (14/16ths = 3.54GB) are user address space.  Immediately after this, is the
 * beginning of kernel space (2/16ths = 512MB).
 * There is 256 MB of KTCBs, giving 18 valid bits for thread IDs.
 *
 * Next is 128 MB of space for kernel code and structures (such as the CPD,
 * root page table and bootstack).
 *
 * After this is 64 MB of space for the copy area.
 *
 * Following this is 16 MB of space for storing variables for space_t such as
 * kip_area, utcb_area, thread_count and domain. It is important that the
 * 2 LSBs _MUST_ be 00, when using these for data or security may be compromised.
 *
 * Following is 47 MB for IO space mappings, then
 *
 * Finally a 1 MB section is reserved for mapping in the exception vectors.
 */


/* 0x00000000 */
#define USER_AREA_START         0UL
#define USER_AREA_SIZE          (USER_AREA_SECTIONS * ARM_SECTION_SIZE)
#define USER_AREA_END           (USER_AREA_START + USER_AREA_SIZE)

#if ((KERNEL_AREA_START) != 0xf0000000)
#error KERNEL_AREA_START should be 0xf0000000
#endif
/* 0xf0000000 */
#define KERNEL_AREA_SECTIONS    64
#define KERNEL_AREA_SIZE        (KERNEL_AREA_SECTIONS * ARM_SECTION_SIZE)
#define KERNEL_AREA_END         (KERNEL_AREA_START + KERNEL_AREA_SIZE)

/* 0xf4000000 */
#define UNCACHE_AREA_SECTIONS   64
#define UNCACHE_AREA_START      KERNEL_AREA_END
#define UNCACHE_AREA_SIZE       (UNCACHE_AREA_SECTIONS * ARM_SECTION_SIZE)
#define UNCACHE_AREA_END        (UNCACHE_AREA_START + UNCACHE_AREA_SIZE)

/* 0xf8000000 */
#define VAR_AREA_SECTIONS       16
#define VAR_AREA_START          UNCACHE_AREA_END
#define VAR_AREA_SIZE           (VAR_AREA_SECTIONS * ARM_SECTION_SIZE)
#define VAR_AREA_END            (VAR_AREA_START + VAR_AREA_SIZE)

/* 0xf9000000 */
#define IO_AREA_SECTIONS        96
#define IO_AREA_START           VAR_AREA_END
#define IO_AREA_SIZE            (IO_AREA_SECTIONS * ARM_SECTION_SIZE)
#define IO_AREA_END             (IO_AREA_START + IO_AREA_SIZE)

/* Note: First page in MISC area is used for UTCB reference page
 *    We reserve the next (14) entries for the UTCB used bit
 *    array. This limits us to 420 threads per space ( 14 * 30 used bits )
 *    { we can't use the bottom two bits in the pt entries, they must be 0 }
 */
/* 0xff000000 */
#define MISC_AREA_SECTIONS      15
#define MISC_AREA_START         IO_AREA_END
#define MISC_AREA_SIZE          (MISC_AREA_SECTIONS * ARM_SECTION_SIZE)
#define MISC_AREA_END           (MISC_AREA_START + MISC_AREA_SIZE)

/* User UTCB reference at 0xff000000 */

#define USER_UTCB_PAGE          MISC_AREA_START

/* Except page at 0xfff00000 */

/* 0xfff00000 */
#define EXCPT_AREA_START        MISC_AREA_END
#define EXCPT_AREA_SIZE         ARM_SECTION_SIZE
#define EXCPT_AREA_END          (EXCPT_AREA_START + EXCPT_AREA_SIZE)

/* 1MB IO Areas in the Virtual Address space. Define more if needed */
#define IO_AREA0_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*0))
#define IO_AREA1_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*1))
#define IO_AREA2_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*2))
#define IO_AREA3_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*3))
#define IO_AREA4_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*4))
#define IO_AREA5_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*5))
#define IO_AREA6_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*6))
#define IO_AREA7_VADDR          (IO_AREA_START + (ARM_SECTION_SIZE*7))
#define PHYSMAPPING_VADDR       (IO_AREA_START + (ARM_SECTION_SIZE*10))

/* Note on io area usage:
 *  ## These mappings are for in kernel drivers ONLY ##
 *
 * SA1100 uses:
 *  #define CONSOLE_VADDR               IO_AREA0_VADDR
 *  #define ZERO_BANK_VADDR             IO_AREA1_VADDR
 *  #define SA1100_OS_TIMER_BASE        IO_AREA2_VADDR
 *
 * XSCALE uses:
 *  #define IODEVICE_VADDR              IO_AREA0_VADDR
 *
 * OMAP1510 uses:
 *  #define IODEVICE_VADDR              IO_AREA0_VADDR
 *
 * AT91RM9200 uses:
 *  #define SYS_VADDR                   IO_AREA0_VADDR
 *  #define SYS_USART                   IO_AREA1_VADDR
 *
 * LN2410SBC users:
 *  #define CONSOLE_VADDR               IO_AREA0_VADDR
 *  #define TIMER_VADDR                 IO_AREA1_VADDR
*/

/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START         (USER_AREA_END - ARM_SECTION_SIZE)

/**
 * Address of start of arm_high_vector - exception handling code
 */
#define ARM_HIGH_VECTOR_VADDR   (EXCPT_AREA_START | 0x000f0000)

#include <kernel/cpu/syscon.h>

#endif /* !__ARM__CONFIG_H__ */
