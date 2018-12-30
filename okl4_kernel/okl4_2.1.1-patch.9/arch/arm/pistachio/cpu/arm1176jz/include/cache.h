/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   Functions which manipulate the ARM1176JZ cache
 * Author: Carl van Schaik, June 2006
 */

#ifndef __ARCH__ARM__ARM1176JZ_CACHE_H_
#define __ARCH__ARM__ARM1176JZ_CACHE_H_

#include <debug.h>
#include INC_CPU(syscon.h)
#include INC_PLAT(cache.h)      /* Get cache size */

#define CACHE_WAYS      4

//#define           ARM1176_HARVARD
//#define           ARM1176_ERRATA

#ifdef ARM1176_ERRATA
#define ERRATA_NOP  "   nop     \n"
#else
#define ERRATA_NOP
#endif

#ifndef ARM1176_HARVARD
#error unsupported
#endif

class arm_cache
{
public:

    static inline void cache_flush(void)
    {
        cache_flush_i();
        cache_flush_d();
    }

    static inline void cache_flush_d(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            ERRATA_NOP
            "   mcr     p15, 0, %0, c7, c14, 0  \n" /* clean+invalidate data cache */
            ERRATA_NOP
            ERRATA_NOP
            "   mcr     p15, 0, %0, c7, c10, 4  \n" /* drain write buffer */
        :: "r" (zero)
        );
    }

    static inline void cache_flush_i(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, %0, c7, c5, 0   \n" /* invalidate instruction cache */
        :: "r" (zero)
        );
    }

    static inline void cache_drain_wb(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, %0, c7, c10, 4  \n" /* drain write buffer */
        :: "r" (zero)
        );
    }

    static inline void cache_flush_i_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush_i();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);
        vaddr = addr_align(vaddr, size);
#if ((1<<5) != CACHE_LINE_SIZE)
#error lines size not 32... fixme
#endif

        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* clean and invalidate cache */
            __asm__ __volatile (
#ifdef ARM1176_ERRATA
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way0 */
#else
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c5, 1       \n" /* clean/invalidate - MVA */
                    ERRATA_NOP
#endif
                    :: "r" (i)
                    : "r2"
                    );

#ifdef ARM1176_ERRATA
            for (word_t j = 0; j < (CACHE_LINE_SIZE); j += BTB_FLUSH_SIZE)
            {
                __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 7       \n" /* Flush BTB - way/set*/
                    :: "r" (i+j)
                    );
            }
#endif
        }
#ifndef ARM1176_ERRATA
        {
            word_t zero = 0;
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 6       \n" /* Flush BTB */
                    :: "r" (zero)
                    );
        }
#endif
    }

    static inline void cache_flush_d_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush_d();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);
        vaddr = addr_align(vaddr, size);

        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* clean and invalidate D cache */
            __asm__ __volatile (
                    ERRATA_NOP
#if 0
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c14, 1      \n" /* clean/invalidate data - MVA */
                    ERRATA_NOP
#else
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way0 */
#endif
                    ERRATA_NOP
                    :: "r" (i)
                    : "r2"
                    );
        }

        cache_drain_wb();
    }

    static inline void cache_flush_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);
        vaddr = addr_align(vaddr, size);
#if ((1<<5) != CACHE_LINE_SIZE)
#error lines size not 32... fixme
#endif

        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* clean and invalidate cache */
            __asm__ __volatile (
                    ERRATA_NOP
#if 0
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c14, 1      \n" /* clean/invalidate data - MVA */
                    ERRATA_NOP
#else
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way0 */
#endif
                    ERRATA_NOP
#ifdef ARM1176_ERRATA
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way0 */
#else
                    "   mcr         p15, 0, %0, c7, c5, 1       \n" /* clean/invalidate - MVA */
                    ERRATA_NOP
#endif
                    :: "r" (i)
                    : "r2"
                    );
#ifdef ARM1176_ERRATA
            for (word_t j = 0; j < (CACHE_LINE_SIZE); j += BTB_FLUSH_SIZE)
            {
                __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 7       \n" /* Flush BTB - way/set*/
                    :: "r" (i+j)
                    );
            }
#endif
        }
#ifndef ARM1176_ERRATA
        {
            word_t zero = 0;
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 6       \n" /* Flush BTB */
                    :: "r" (zero)
                    );
        }
#endif

        cache_drain_wb();
    }

    /* These require CACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_clean_invalidate_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* clean and invalidate D cache */
            __asm__ __volatile (
#if 0
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c14, 1      \n" /* clean/invalidate data - MVA */
                    ERRATA_NOP
#else
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c14, 2      \n" /* Invalidate D way0 */
#endif
                    :: "r" (i)
                    : "r2"
                    );
        }

        cache_drain_wb();
    }

    /* These require CACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_clean_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* clean D cache */
            __asm__ __volatile (
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c10, 1      \n" /* clean data - MVA */
                    ERRATA_NOP
                    :: "r" (i)
                    );
        }

        cache_drain_wb();
    }

    /* These require CACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_invalidate_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* invalidate D cache */
            __asm__ __volatile (
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c6, 1       \n" /* invalidate data - MVA */
                    ERRATA_NOP
                    :: "r" (i)
                    );
        }
    }

    /* These require CACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_invalidate_ilines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += CACHE_LINE_SIZE)
        {
            /* invalidate I cache */
            __asm__ __volatile (
#ifdef ARM1176_ERRATA
                    "   orr         r2, %0, #0xc0000000         \n" /* ARM1176 erratum */
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way3 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way2 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way1 */
                    "   sub         r2, r2, #0x40000000         \n"
                    "   mcr         p15, 0, r2, c7, c5, 2       \n" /* Invalidate I way0 */
#else
                    ERRATA_NOP
                    "   mcr         p15, 0, %0, c7, c5, 1       \n" /* clean/invalidate - MVA */
                    ERRATA_NOP
#endif
                    :: "r" (i)
#ifdef ARM1176_ERRATA
                    : "r2"
#endif
                    );
#ifdef ARM1176_ERRATA
            for (word_t j = 0; j < (CACHE_LINE_SIZE); j += BTB_FLUSH_SIZE)
            {
                __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 7       \n" /* Flush BTB - way/set*/
                    :: "r" (i+j)
                    );
            }
#endif
        }
#ifndef ARM1176_ERRATA
        {
            word_t zero = 0;
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 6       \n" /* Flush BTB */
                    :: "r" (zero)
                    );
        }
#endif
    }

    static inline void cache_flush_range_attr(addr_t vaddr, word_t size, word_t attr)
    {
        size = ((size + CACHE_LINE_SIZE-1) + ((word_t)vaddr & (CACHE_LINE_SIZE-1)))
            & (~(CACHE_LINE_SIZE-1));
        vaddr = (addr_t)((word_t)vaddr & (~(CACHE_LINE_SIZE-1)));               // align addresses

        if (attr & 0x2) /* D-cache */
        {
            if ((attr & 0xc) == 0xc)
            {
                if (size > (32*1024))
                {
                    cache_flush_d();
                }
                else
                {
//printf("cid %p, %d\n", vaddr, size);
                    cache_clean_invalidate_dlines(vaddr, size);
//cache_flush_d();
                }
            }
            else if ((attr & 0x8))
            {
                cache_invalidate_dlines(vaddr, size);
            }
            else if ((attr & 0x4))
            {
                cache_clean_dlines(vaddr, size);
            }
        }

        if ((attr & 0x9) == 0x9) /* I-cache - invalidate */
        {
            cache_invalidate_ilines(vaddr, size);
        }
    }

    static inline void cache_flush_debug(void)
    {
        printf("About to cache flush... ");
        cache_flush();
        printf("done.\n");
    }

    static inline void tlb_flush(void)
    {
        /* Flush I&D TLB */
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr    p15, 0, %0, c8, c7, 0    \n"
            :: "r" (zero));
    }

    static inline void tlb_flush_asid(u8_t asid)
    {
        /* Flush I&D TLB */
        word_t val = asid;
        __asm__ __volatile__ (
            "   mcr    p15, 0, %0, c8, c7, 2    \n"
            :: "r" (val));
    }

    static inline void tlb_flush_ent(u8_t asid, addr_t vaddr, word_t log2size)
    {
        vaddr = addr_align(vaddr, PAGE_SIZE_4K);
        word_t a = (word_t)vaddr;

        a = (a & (~0x3fful)) | asid;

        for (; a < (word_t)vaddr + (1ul << log2size); a += PAGE_SIZE_4K)
        {
#ifdef ARM1176_HARVARD
            __asm__ __volatile__ (
                "    mcr     p15, 0, %0, c8, c6, 1    \n"       /* Invalidate D TLB entry */
                ERRATA_NOP
                ERRATA_NOP
                "    mcr     p15, 0, %0, c8, c5, 1    \n"       /* Invalidate I TLB entry */
                ERRATA_NOP
                ERRATA_NOP
            :: "r" (a));
#else
            __asm__ __volatile__ (
                "    mcr     p15, 0, %0, c8, c7, 1    \n"       /* Invalidate TLB entry */
            :: "r" (a));
#endif
        }
    }

    static inline void tlb_flush_debug(void)
    {
        printf("About to TLB flush... ");
        tlb_flush();
        printf("done.\n");
    }

    static inline void cache_enable()
    {
UNIMPLEMENTED();
    }

    static inline void cache_invalidate_d_line(word_t target)
    {
UNIMPLEMENTED();
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c6, 1 \n"
            :: "r" (target));
        CPWAIT;
    }

    static inline void cache_invalidate_i()
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c5, 0 \n"
            :: "r" (zero)
        );
    }

    static inline void cache_invalidate_d()
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c6, 0 \n"
            :: "r" (zero)
        );
    }

    static inline void cache_clean(word_t target)
    {
UNIMPLEMENTED();
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c10, 1 \n"
        :: "r" (target));
        CPWAIT;
    }

    static inline void cache_drain_write_buffer(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, %0, c7, c10, 4  \n" /* drain write buffer */
            :: "r" (zero)
        );
    }
};

#endif /* __ARCH__ARM__ARM1176JZ_CACHE_H_ */
