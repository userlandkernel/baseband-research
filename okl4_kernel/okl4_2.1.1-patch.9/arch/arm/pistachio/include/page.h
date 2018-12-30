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
 * Description:   ARM specific MM
 */

#ifndef __ARCH__ARM__PAGE_H__
#define __ARCH__ARM__PAGE_H__

/*
 * Supported Page Sizes:
 * - 1k ("tiny" pages) -- ARMv5 *broken*
 * - 4k
 * - 64k
 * - 1M ("sections")
 * - 16M ("supersections")  -- ARMv6
 */
#define ARM_SECTION_BITS        12
#define ARM_SECTION_SIZE        (1UL << (32 - ARM_SECTION_BITS))
#define ARM_NUM_SECTIONS        (1UL << ARM_SECTION_BITS)


#if CONFIG_ARM_VER >= 6
#define ARM_SUPER_SECTION_BITS  8
#define ARM_SUPER_SECTION_SIZE  (1UL << (32 - ARM_SUPER_SECTION_BITS))
#define ARM_NUM_SUPER_SECTIONS  (1UL << ARM_SUPER_SECTION_BITS)
#endif

#if CONFIG_ARM_VER  == 5

/* 256/256 byte levels */
#define ARM_TOP_LEVEL_BITS      26

#if defined(CONFIG_ARM_TINY_PAGES)
 #define ARM_PAGE_SIZE          PAGE_SIZE_1K
 #define ARM_PAGE_BITS          PAGE_BITS_1K
 #define TINY_PAGE              (1<<10) |
 #define HW_PGSHIFTS            { 10, 12, 16, 20, ARM_TOP_LEVEL_BITS, 32 }
#else
 #define ARM_PAGE_SIZE          PAGE_SIZE_4K
 #define ARM_PAGE_BITS          PAGE_BITS_4K
 #define TINY_PAGE
 #define HW_PGSHIFTS            { 12, 16, 20, ARM_TOP_LEVEL_BITS, 32 }
#endif

#define HW_VALID_PGSIZES (      \
                TINY_PAGE       \
                (1<<12) |       \
                (1<<16) |       \
                (1<<20))

#define PAGE_SIZE_TOP_LEVEL     (1UL << ARM_TOP_LEVEL_BITS)
#define PAGE_BITS_TOP_LEVEL     (ARM_TOP_LEVEL_BITS)
#define PAGE_MASK_TOP_LEVEL     (~(PAGE_SIZE_TOP_LEVEL - 1))

#else /* CONFIG_ARM_VER >= 6 */

#define ARM_PAGE_SIZE           PAGE_SIZE_4K
#define HW_PGSHIFTS             { 12, 16, 20, 24, 32 }

#define HW_VALID_PGSIZES (      \
                (1<<12) |       \
                (1<<16) |       \
                (1<<20) |       \
                (1<<24))

#endif

#define PAGE_SIZE_1K            (1UL << 10)
#define PAGE_SIZE_4K            (1UL << 12)
#define PAGE_SIZE_64K           (1UL << 16)
#define PAGE_SIZE_1M            (1UL << 20)
#define PAGE_SIZE_16M           (1UL << 24)

#define PAGE_BITS_1K            (10)
#define PAGE_BITS_4K            (12)
#define PAGE_BITS_64K           (16)
#define PAGE_BITS_1M            (20)
#define PAGE_BITS_16M           (24)

#define PAGE_MASK_1K            (~(PAGE_SIZE_1K - 1))
#define PAGE_MASK_4K            (~(PAGE_SIZE_4K - 1))
#define PAGE_MASK_64K           (~(PAGE_SIZE_64K -1))
#define PAGE_MASK_1M            (~(PAGE_SIZE_1M - 1))
#define PAGE_MASK_16M           (~(PAGE_SIZE_16M- 1))

/* Define PID masks. SA1100 has 6-bit PID, ARM9/10/11 have 7-bits */
#ifdef CPU_ARM_SA1100
#define PID_MASK        0x3f
#else
#define PID_MASK        0x7f
#endif

#endif /* __ARCH__ARM__PAGE_H__ */
