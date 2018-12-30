/*
 * Copyright (c) 2002, University of New South Wales
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
 * Description:   Functions which manipulate the MIPS cache
 */ 

#ifndef __CPU__R4600__CACHE_H__
#define __CPU__R4600__CACHE_H__

#include <kernel/arch/mipsregs.h>
#include <kernel/arch/addrspace.h>
#include <kernel/linear_ptab.h>

#define CONFIG_MIPS_DCACHE_SIZE         (16*1024)
#define CONFIG_MIPS_ICACHE_SIZE         (16*1024)
#define CONFIG_MIPS_CACHE_LINE_SIZE     16
#define CONFIG_MIPS_CACHE_WAYS          2

#define CACHE_WAYS              CONFIG_MIPS_CACHE_WAYS
//#define CACHE_LINE_SIZE               CONFIG_MIPS_CACHE_LINE_SIZE     in config.h
#define DCACHE_SIZE             CONFIG_MIPS_DCACHE_SIZE
#define ICACHE_SIZE             CONFIG_MIPS_ICACHE_SIZE

/*
 * Cache Operations
 */
#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Invalidate_SI     0x02
#define Index_Writeback_Inv_SD  0x03
#define Index_Load_Tag_I        0x04
#define Index_Load_Tag_D        0x05
#define Index_Load_Tag_SI       0x06
#define Index_Load_Tag_SD       0x07
#define Index_Store_Tag_I       0x08
#define Index_Store_Tag_D       0x09
#define Index_Store_Tag_SI      0x0A
#define Index_Store_Tag_SD      0x0B
#define Create_Dirty_Excl_D     0x0d
#define Create_Dirty_Excl_SD    0x0f
#define Hit_Invalidate_I        0x10
#define Hit_Invalidate_D        0x11
#define Hit_Invalidate_SI       0x12
#define Hit_Invalidate_SD       0x13
#define Fill                    0x14
#define Hit_Writeback_Inv_D     0x15
                                        /* 0x16 is unused */
#define Hit_Writeback_Inv_SD    0x17
#define Hit_Writeback_I         0x18
#define Hit_Writeback_D         0x19
                                        /* 0x1a is unused */
#define Hit_Writeback_SD        0x1b
                                        /* 0x1c is unused */
                                        /* 0x1e is unused */
#define Hit_Set_Virtual_SI      0x1e
#define Hit_Set_Virtual_SD      0x1f

#define cache16_unroll32(base,op)                               \
        __asm__ __volatile__("                                  \
                .set noreorder;                                 \
                cache %1, 0x000(%0); cache %1, 0x010(%0);       \
                cache %1, 0x020(%0); cache %1, 0x030(%0);       \
                cache %1, 0x040(%0); cache %1, 0x050(%0);       \
                cache %1, 0x060(%0); cache %1, 0x070(%0);       \
                cache %1, 0x080(%0); cache %1, 0x090(%0);       \
                cache %1, 0x0a0(%0); cache %1, 0x0b0(%0);       \
                cache %1, 0x0c0(%0); cache %1, 0x0d0(%0);       \
                cache %1, 0x0e0(%0); cache %1, 0x0f0(%0);       \
                cache %1, 0x100(%0); cache %1, 0x110(%0);       \
                cache %1, 0x120(%0); cache %1, 0x130(%0);       \
                cache %1, 0x140(%0); cache %1, 0x150(%0);       \
                cache %1, 0x160(%0); cache %1, 0x170(%0);       \
                cache %1, 0x180(%0); cache %1, 0x190(%0);       \
                cache %1, 0x1a0(%0); cache %1, 0x1b0(%0);       \
                cache %1, 0x1c0(%0); cache %1, 0x1d0(%0);       \
                cache %1, 0x1e0(%0); cache %1, 0x1f0(%0);       \
                .set reorder"                                   \
                :                                               \
                : "r" (base),                                   \
                  "i" (op));

/**
 * @brief Roll through dcache and clear
 *
 * @param start   Pass in start location to allow for restart after 
 *                preemption from where we werer up to.
 */
static inline void blast_dcache16(word_t *start)
{
    if (! *start)
         *start = KSEG0;
    unsigned long end = (KSEG0 + DCACHE_SIZE);

    while (*start < end) {
        cache16_unroll32(*start, Index_Writeback_Inv_D);
        *start += 0x200;
    }
    *start = 0;
}


/**
 * @brief Roll through icache and clear
 *
 * @param start   Pass in start location to allow for restart after 
 *                preemption from where we werer up to.
 */
static inline void blast_icache16(word_t *start)
{
    if (! *start)
        *start = KSEG0;

    unsigned long end = (KSEG0 + ICACHE_SIZE);

    while (*start < end) {
        cache16_unroll32(*start, Index_Invalidate_I);
        *start += 0x200;
    }
    *start = 0;
}


static inline void init_dcache(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + DCACHE_SIZE);

    asm (
        "mtc0   $0, "STR(CP0_TAGLO)"\n\t"
        "mtc0   $0, "STR(CP0_TAGHI)"\n\t"
    );

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Index_Store_Tag_D)
        );
        start += CACHE_LINE_SIZE;
    }
}

static inline void init_icache(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + ICACHE_SIZE);

    asm (
        "mtc0   $0, "STR(CP0_TAGLO)"\n\t"
        "mtc0   $0, "STR(CP0_TAGHI)"\n\t"
    );

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Index_Store_Tag_I)
        );
        start += CACHE_LINE_SIZE;
    }
}


INLINE void cache_t::init_cpu(void)
{
    word_t temp;

    __asm__ __volatile__ (
        "la     %0, 1f\n\t"
        "or     %0, 0xffffffffa0000000\n\t"
        "jr     %0\n\t"
        "1:\n\t"
        "mfc0   %0, "STR(CP0_CONFIG)"\n\t"
        : "=r" (temp)
    );
    temp &= (~CONFIG_CACHE_MASK);
#if defined(CONFIG_UNCACHED)
    temp |= CONFIG_NOCACHE;
#else
    temp |= CONFIG_CACHABLE_NONCOHERENT;
#endif

    __asm__ __volatile__ (
        "mtc0   %0, "STR(CP0_CONFIG)"\n\t"
        : : "r" (temp)
    );

    /* Important that these inline! */
    init_dcache();
    init_icache();

    __asm__ __volatile__ (
        "la     %0, 2f\n\t"
        "jr     %0\n\t"
        "2:\n\t"
        : : "r" (temp)
    );
}

INLINE void cache_t::flush_cache_all(void)
{
    word_t start = 0;
    blast_dcache16(&start); blast_icache16(&start);
}

INLINE void cache_t::flush_cache_l1(void)
{
    word_t start = 0;
    blast_dcache16(&start); blast_icache16(&start);
}

INLINE void cache_t::flush_dcache_range(unsigned long start, unsigned long end)
{
    start &= (~(CACHE_LINE_SIZE-1));
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Hit_Writeback_Inv_D)
        );
        start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::clean_dcache_range(unsigned long start, unsigned long end)
{
    start &= (~(CACHE_LINE_SIZE-1));
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Hit_Writeback_D)
        );
        start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::invalidate_dcache_range(unsigned long start, unsigned long end)
{
    start &= (~(CACHE_LINE_SIZE-1));
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Hit_Invalidate_D)
        );
        start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::flush_icache_range(unsigned long start, unsigned long end)
{
    start &= (~(CACHE_LINE_SIZE-1));
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    while (start < end) {
        asm (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Hit_Invalidate_I)
        );
        start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::flush_range_attribute(generic_space_t *space,
                                           addr_t start,
                                           addr_t end,
                                           cacheattr_e attr)
{
    if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
    {
        if ((attr & CACHE_ATTRIB_OP_CLEAN_INVAL) == CACHE_ATTRIB_OP_CLEAN_INVAL)
            flush_dcache_range((unsigned long)start, (unsigned long)end);
        else if ((attr & CACHE_ATTRIB_OP_CLEAN))
            clean_dcache_range((unsigned long)start, (unsigned long)end);
        else if ((attr & CACHE_ATTRIB_OP_INVAL))
            invalidate_dcache_range((unsigned long)start, (unsigned long)end);
    }
    if ((attr & CACHE_ATTRIB_INVAL_I) == CACHE_ATTRIB_INVAL_I) /* I-cache - invalidate */
    {
        flush_icache_range((unsigned long)start, (unsigned long)end);
    }
}

INLINE void cache_t::flush_all_attribute(cacheattr_e attr, word_t *start)
{
    if (attr & CACHE_ATTRIB_MASK_I) /* I-cache */
    {
        blast_icache16(start);
    }
    if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
    {
        blast_dcache16(start);
    }
}

INLINE void cache_t::flush_cache_page(unsigned long page, pgent_t::pgsize_e pgsize)
{
    unsigned long start = page & (~page_mask(pgsize));
    unsigned long end = start + page_size(pgsize);

    if (end > (start + DCACHE_SIZE))
        end = start + DCACHE_SIZE;

    while (start < end) {
        cache16_unroll32(start, Index_Writeback_Inv_D);
        start += 0x200;
    }
}

INLINE void cache_t::flush_icache_page(unsigned long page, pgent_t::pgsize_e pgsize)
{
    unsigned long start = page & (~page_mask(pgsize));
    unsigned long end = start + page_size(pgsize);

    if (end > (start + ICACHE_SIZE))
        end = start + ICACHE_SIZE;

    while (start < end) {
        cache16_unroll32(start, Index_Invalidate_I);
        start += 0x200;
    }
}

INLINE bool cache_t::full_flush_cheaper(word_t size)
{
    return (size >= DCACHE_SIZE) ? true : false;
}

#endif /*__CPU__R4600__CACHE_H__*/
