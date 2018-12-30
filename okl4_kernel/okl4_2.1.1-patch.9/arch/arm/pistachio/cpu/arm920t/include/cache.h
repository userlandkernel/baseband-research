/*
 * Copyright (c) 2004, National ICT Australia (NICTA)
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
 * Description:   Functions which manipulate the ARM920T cache
 */

#ifndef __PLATFORM__ARM__ARM920T__CACHE_H_
#define __PLATFORM__ARM__ARM920T__CACHE_H_

#include <kernel/debug.h>
#include <kernel/cache.h>

class generic_space_t;

extern word_t DCACHE_SIZE;
extern word_t DCACHE_LINE_SIZE;
extern word_t DCACHE_SETS;
extern word_t DCACHE_WAYS;
extern word_t ICACHE_SIZE;
extern word_t ICACHE_LINE_SIZE;
extern word_t ICACHE_SETS;
extern word_t ICACHE_WAYS;

/* Write-through mode on arm920t */
#define CPU_WRITETHROUGH        2

#define ARM_CPU_HAS_TLBLOCK     1
/* how many of the 64 I/D entries should we allow */
#define CONFIG_LOCKDOWN_MAX     4

class arm_cache
{
public:
    static bool full_flush_cheaper(word_t size)
    {
        return (size >= DCACHE_SIZE ? true : false);
    }

    /* ARM920T dcache is 16KB,
     * with 64-way associativity, 8 words per cache line. */
    static inline void cache_flush(void)
    {
        word_t sets, ways, index;
        for (sets = 0; sets < DCACHE_SETS; sets++)
        {
            for (ways = 0; ways < DCACHE_WAYS; ways++)
            {
                index = (sets << 5) | (ways << 26);
                /* clean and invalidate D cache */
                __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c14, 2      \n" /* clean/invalidate index */
                    :: "r" (index)
                );
            }
        }

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* invalidate I caches */
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_flush(word_t attr)
    {
        if (attr & CACHE_ATTRIB_MASK_I) /* I-cache */
        {
            __asm__ __volatile__ (
                "       mov     r0,     #0              \n"
                "       mcr     p15, 0, r0, c7, c5, 0   \n" /* invalidate I caches */
                ::: "r0"
            );
        }
        if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
        {
            cache_flush_d();
        }
    }

    static inline void cache_flush_d(void)
    {
        word_t sets, ways, index;
        for (sets = 0; sets < DCACHE_SETS; sets++)
        {
            for (ways = 0; ways < DCACHE_WAYS; ways++)
            {
                index = (sets << 5) | (ways << 26);
                /* clean and invalidate D cache */
                __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c14, 2      \n"
                    :: "r" (index)
                );
            }
        }

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_flush_i_ent(addr_t vaddr, word_t log2size)
    {
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* invalidate I caches */
            ::: "r0"
        );
    }

    static inline void cache_flush_d_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush_d();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);

        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += DCACHE_LINE_SIZE)
        {
            /* clean and invalidate D cache */
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c14, 1      \n" /* clean/invalidate - MVA */
                    :: "r" (i)
                    );
        }

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_clean_invalidate_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += DCACHE_LINE_SIZE)
        {
            /* clean and invalidate D cache */
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c14, 1      \n" /* clean/invalidate - MVA */
                    :: "r" (i)
                    );
        }

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_clean_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += DCACHE_LINE_SIZE)
        {
            /* clean D cache */
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c10, 1      \n" /* clean - MVA */
                    :: "r" (i)
                    );
        }

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_invalidate_dlines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += DCACHE_LINE_SIZE)
        {
            /* invalidate D cache */
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c6, 1       \n" /* invalidate - MVA */
                    :: "r" (i)
                    );
        }
    }

    static inline void cache_invalidate_ilines(addr_t vaddr, word_t size)
    {
        for (word_t i = (word_t)vaddr; i < ((word_t)vaddr + size); i += DCACHE_LINE_SIZE)
        {
            /* invalidate I cache */
            __asm__ __volatile (
                    "   mcr         p15, 0, %0, c7, c5, 1       \n" /* invalidate - MVA */
                    :: "r" (i)
                    );
        }
    }

    static inline void cache_flush_ent(addr_t vaddr, word_t log2size)
    {
        cache_flush_d_ent(vaddr, log2size);
        cache_flush_i_ent(vaddr, log2size);
    }

    static inline void cache_flush_range_attr(generic_space_t *space,
            addr_t vaddr, word_t size, word_t attr)
    {
        size = ((size + DCACHE_LINE_SIZE-1) + ((word_t)vaddr & (DCACHE_LINE_SIZE-1)))
            & (~(DCACHE_LINE_SIZE-1));
        vaddr = (addr_t)((word_t)vaddr & (~(DCACHE_LINE_SIZE-1)));               // align addresses

        if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
        {
            if ((attr & CACHE_ATTRIB_OP_CLEAN_INVAL) == CACHE_ATTRIB_OP_CLEAN_INVAL)
            {
                cache_clean_invalidate_dlines(vaddr, size);
            }
            else if ((attr & CACHE_ATTRIB_OP_INVAL))
            {
                cache_invalidate_dlines(vaddr, size);
            }
            else if ((attr & CACHE_ATTRIB_OP_CLEAN))
            {
                cache_clean_dlines(vaddr, size);
            }
        }

        if ((attr & CACHE_ATTRIB_INVAL_I) == CACHE_ATTRIB_INVAL_I) /* I-cache - invalidate */
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
        __asm__ __volatile__ (
            "    mov    r0,     #0              \n"
            "    mcr    p15, 0, r0, c8, c7, 0   \n"
            ::: "r0"
        );
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
        word_t a = (word_t)vaddr;

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c8, c5, 0   \n"     /* Invalidate I TLB */
            ::: "r0"
        );
        for (word_t i=0; i < (1ul << log2size); i += ARM_PAGE_SIZE)
        {
            __asm__ __volatile__ (
                "   mcr p15, 0, %0, c8, c6, 1    \n"    /* Invalidate D TLB entry */
                :: "r" (a)
            );
            a += ARM_PAGE_SIZE;
        }
    }

    static inline void tlb_flush_debug(void)
    {
        printf("About to TLB flush... ");
        tlb_flush();
        printf("done.\n");
    }

    static inline void cache_invalidate(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c6, 1 \n"
            :: "r" (target)
        );
    }

    static inline void cache_invalidate_d()
    {
        __asm__ __volatile__ (
            "mcr p15, 0, r0, c7, c6, 0 \n"
        ::: "r0"
        );
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c10, 1 \n"
            :: "r" (target)
        );
    }

    static inline void cache_drain_write_buffer(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, %0, c7, c10, 4  \n" /* drain write buffer */
            :: "r" (zero)
        );
    }

    static inline void lock_tlb_addr(addr_t vaddr)
    {
        /* XXX */
        __asm__ __volatile__ (
                ""
#if defined(__GNUC__)
                :: [vaddr] "r" (vaddr)
                : "r0"
#endif
        );
    }

};

#endif /* __PLATFORM__ARM__ARM920T__CACHE_H_ */
