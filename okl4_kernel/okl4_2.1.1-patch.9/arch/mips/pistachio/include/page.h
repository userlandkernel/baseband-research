/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Copyright (c) 2004-2006, National ICT Australia (NICTA)
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
 * Created:       30/07/2002 10:48:30 by Daniel Potts (danielp)
 * Description:   MIPS specific MM 
 */ 

#ifndef __ARCH__MIPS__PAGE_H__
#define __ARCH__MIPS__PAGE_H__

#include <kernel/config.h>
#include <kernel/arch/addrspace.h>
#include <kernel/cpu/config.h>

#if defined(L4_32BIT)

/* HW_PGSHIFTS must match pgsize_e (pgent.h) */
/* toplevel is (2048)*4  - 2^11 entries */
/* so we have 12 -> 21 -> 32 */
#define PGTABLE_TOP_BITS        11
#define TOPLEVEL_PT_BITS        21

#define HW_PGSHIFTS            { 12, 13, 15, 17, 19, 21, 23, 25, 32 } /* 32 - virtual ktcb */

#define HW_VALID_PGSIZES       ((1ul << 12) /*   4KB */ | \
                                (1ul << 13) /*   8KB */ | \
                                (1ul << 15) /*  32KB */ | \
                                (1ul << 17) /* 128KB */ | \
                                (1ul << 19) /* 512KB */ | \
                                (1ul << 21) /*   2MB */ | \
                                (1ul << 23) /*   8MB */ | \
                                (1ul << 25) /*  32MB */)



#elif defined(L4_64BIT)

#if CONFIG_MIPS_USER_ADDRESS_BITS == 40

#define TOPLEVEL_PT_BITS (31)

/* HW_PGSHIFTS must match pgsize_e (pgent.h) */
/* so we have 12 -> 21 -> 31 -> 41 */
#define PGTABLE_TOP_BITS        10

#define HW_PGSHIFTS             { 12, 13, 15, 17, 19, 21, 23, 25, 31, 41 } /* 41 - virtual ktcb */

#define HW_VALID_PGSIZES        ((1UL << 12) /*   4KB */ | \
                                 (1UL << 13) /*   8KB */ | \
                                 (1UL << 15) /*  32KB */ | \
                                 (1UL << 17) /* 128KB */ | \
                                 (1UL << 19) /* 512KB */ | \
                                 (1UL << 21) /*   2MB */ | \
                                 (1UL << 23) /*   8MB */ | \
                                 (1UL << 25) /*  32MB */)


#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44

#define TOPLEVEL_PT_BITS (37)

/* HW_PGSHIFTS must match pgsize_e (pgent.h) */
/* so we have 12 -> 21 -> 29 -> 37 -> 45 */
#define PGTABLE_TOP_BITS        8

#define HW_PGSHIFTS             { 12, 13, 15, 17, 19, 21, 23, 25, 27, 29, 37, 45 } /* 45 - virtual ktcb */

#define HW_VALID_PGSIZES        ((1 << 12)
#if 0
        |  /*   4KB */ \
                                (1 << 14) |  /*  16KB */ \
                                (1 << 16) |  /*  64KB */ \
                                (1 << 18) |  /* 256KB */ \
                                (1 << 20) |  /*   1MB */ \
                                (1 << 22) |  /*   4MB */ \
                                (1 << 24) |  /*  16MB */ \
                                (1 << 26) |  /*  64MB */ \
                                (1 << 28))   /* 256MB */
#endif


#else
#error We only support 40 and 44 bit address spaces!
#endif /* CONFIG_MIPS_USER_ADDRESS_BITS */

#endif /* 64-BIT */

/* Basic page sizes etc. */

#define MIPS_PAGE_BITS         12
#define MIPS_PAGE_SIZE         (1UL << MIPS_PAGE_BITS)
#define MIPS_PAGE_MASK         (~(MIPS_PAGE_SIZE - 1))
#define MIPS_OFFSET_MASK       (~MIPS_PAGE_MASK)


/* Address space layout */

#if defined(L4_32BIT)

#define AS_KUSEG_START          (KUSEG)
#define AS_KUSEG_SIZE           (1UL << CONFIG_MIPS_USER_ADDRESS_BITS)
#define AS_KUSEG_END            (AS_KUSEG_START + AS_KUSEG_SIZE - 1)

#define KSEG_BITS               29

#define AS_KSEG0_START          (KSEG0)
#define AS_KSEG0_SIZE           (1UL << KSEG_BITS)
#define AS_KSEG0_END            (AS_KSEG0_START + AS_KSEG0_SIZE - 1)

#define AS_KSEG1_START          (KSEG1)
#define AS_KSEG1_SIZE           (1UL << KSEG_BITS)
#define AS_KSEG1_END            (AS_KSEG2_START + AS_KSEG2_SIZE - 1)

#define AS_KSSEG_START          (KSSEG)
#define AS_KSSEG_SIZE           (1UL << KSEG_BITS)
#define AS_KSSEG_END            (AS_KSEG2_START + AS_KSEG2_SIZE - 1)

#define AS_KSEG3_START          (KSEG3)
#define AS_KSEG3_SIZE           (1UL << KSEG_BITS)
#define AS_KSEG3_END            (AS_KSEG3_START + AS_KSEG3_SIZE - 1)

#define AS_KSEG_START           AS_KSEG0_START
#define AS_KSEG_END             AS_KSEG0_END
#define AS_KSEG_SIZE            AS_KSEG0_SIZE

#elif defined(L4_64BIT)

#define AS_XKUSEG_START         (0)
#define AS_XKUSEG_SIZE          (1UL << (CONFIG_MIPS_USER_ADDRESS_BITS))
#define AS_XKUSEG_END           (AS_XKUSEG_START + AS_XKUSEG_SIZE - 1)

/* XKPHYS is 8 segments of 2^PABITS each with different mapping modes.
 * SIZE and END are defined only for one of these segments.
 * If we want different cache attributes for this region, we could adjust START.
 */
#define AS_XKPHYS_START         (XKPHYS)
#define AS_XKPHYS_SIZE          (1UL << CONFIG_MIPS64_PHYS_ADDRESS_BITS)
#define AS_XKPHYS_END           (AS_XKPHYS_START + AS_XKPHYS_SIZE - 1)

#define AS_CKSEG0_START         (CKSEG0)
#define AS_CKSEG0_SIZE          (1UL << 29)
#define AS_CKSEG0_END           (AS_CKSEG0_START + AS_CKSEG0_SIZE - 1)

#define AS_CKSEG1_START         (CKSEG1)
#define AS_CKSEG1_SIZE          (1UL << 29)
#define AS_CKSEG1_END           (AS_CKSEG2_START + AS_CKSEG2_SIZE - 1)

#define AS_CKSSEG_START         (CKSSEG)
#define AS_CKSSEG_SIZE          (1UL << 29)
#define AS_CKSSEG_END           (AS_CKSEG2_START + AS_CKSEG2_SIZE - 1)

#define AS_CKSEG3_START         (CKSEG3)
#define AS_CKSEG3_SIZE          (1UL << 29)
#define AS_CKSEG3_END           (AS_CKSEG3_START + AS_CKSEG3_SIZE - 1)

#define AS_XKSSEG_START         (XKSSEG)
#define AS_XKSSEG_SIZE          ((1UL << CONFIG_MIPS_USER_ADDRESS_BITS))
#define AS_XKSSEG_END           (AS_XKSSEG_START + AS_XKSSEG_SIZE - 1)

/* define where kernel starts */
/*
#define AS_KSEG_START           AS_XKPHYS_START
#define AS_KSEG_END             AS_XKPHYS_END
*/
#define AS_KSEG_START           AS_CKSEG0_START
#define AS_KSEG_END             AS_CKSEG0_END
#define AS_KSEG_SIZE            AS_CKSEG0_SIZE

#endif

#endif /* __ARCH__MIPS__PAGE_H__ */
