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
 * Description:   Functions which manipulate the OMAP1510 cache
 */

#ifndef __PLATFORM__ARM__OMAP1510__CACHE_H_
#define __PLATFORM__ARM__OMAP1510__CACHE_H_

#include <debug.h>


#define CACHE_SIZE          8192
#define CACHE_LINE_SIZE     16

/* Write-through mode on arm925t */
#define CPU_WRITETHROUGH        2

class arm_cache
{
public:
    /* OMAP1510 dcache is 8KB write back, no lock-down, Harvard cache,
     * with 2-way associativity, 4 words per cache line. */
    static inline void cache_flush(void)
    {
        cache_flush_d();
        cache_flush_i();
    }

    static inline void cache_flush(word_t attr)
    {
        if (attr & CACHE_ATTRIB_MASK_I) /* I-cache */
        {
            cache_flush_i();
        }
        if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
        {
            cache_flush_d();
        }
    }

    static inline void cache_flush_d(void)
    {
        word_t index;
        for (index = 0; index < CACHE_SIZE; index += CACHE_LINE_SIZE)
        {
            /* clean and invalidate D cache */
            __asm__ __volatile (
                "   mcr     p15, 0, %0, c7, c14, 2      \n"
                :: "r" (index)
            );
        }
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }

    static inline void cache_flush_i(void)
    {
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* invalidate I caches */
            ::: "r0"
        );
    }

    static inline void cache_flush_debug(void)
    {
        printf("About to cache flush... ");
        cache_flush();
        printf("done.\n");
    }

    static inline void cache_flush_i_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 13) {   /* for >8k, whole flush is better */
            cache_flush_i();
            return;
        }
        word_t size = 1ul << (log2size > 4 ? log2size : 4);

        __asm__ __volatile__ (
            "                                   \n"
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c5, 1   \n" /* invalidate I cache line */
            "   add     %0,     %0,    %2       \n"
            "   mcr     p15, 0, %0, c7, c5, 1   \n" /* invalidate I cache line */
            "   add     %0,     %0,    %2       \n"
            "   cmp     r0,     %0              \n"
            "   blo     1b                      \n"
            "   mov     r0,     #0              \n"
            "                                   \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            "                                   \n"
            : "+r" (vaddr)
            : "r" (size), "i" (CACHE_LINE_SIZE)
            : "r0", "memory");
    }

    static inline void cache_flush_d_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 13) {   /* for >8k, whole flush is better */
            cache_flush_d();
            return;
        }
        word_t size = 1ul << (log2size > 4 ? log2size : 4);

        __asm__ __volatile__ (
            "                                   \n"
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c14, 1  \n" /* clean + invalidate D cache line */
            "   add     %0,     %0,    %2       \n"
            "   mcr     p15, 0, %0, c7, c14, 1  \n" /* clean + invalidate D cache line */
            "   add     %0,     %0,    %2       \n"
            "   cmp     r0,     %0              \n"
            "   blo     1b                      \n"
            "   mov     r0,     #0              \n"
            "                                   \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            "                                   \n"
            : "+r" (vaddr)
            : "r" (size), "i" (CACHE_LINE_SIZE)
            : "r0", "memory");
    }

    static inline void cache_flush_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 13) {   /* for >8k, whole flush is better */
            cache_flush();
            return;
        }
        word_t size = 1ul << (log2size > 4 ? log2size : 4);

        __asm__ __volatile__ (
            "                                   \n"
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c14, 1  \n" /* clean + invalidate D cache line */
            "   mcr     p15, 0, %0, c7, c5, 1   \n" /* invalidate I cache line */
            "   add     %0,     %0,    %2       \n"
            "   mcr     p15, 0, %0, c7, c14, 1  \n" /* clean + invalidate D cache line */
            "   mcr     p15, 0, %0, c7, c5, 1   \n" /* invalidate I cache line */
            "   add     %0,     %0,    %2       \n"
            "   cmp     r0,     %0              \n"
            "   blo     1b                      \n"
            "   mov     r0,     #0              \n"
            "                                   \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            "                                   \n"
            : "+r" (vaddr)
            : "r" (size), "i" (CACHE_LINE_SIZE)
            : "r0", "memory");
    }

    static inline void tlb_flush(void)
    {
        __asm__ __volatile__ (
            "    mov    r0,     #0              \n"
            "    mcr     p15, 0, r0, c8, c7, 0  \n"
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
};

#endif /* __PLATFORM__ARM__OMAP1510__CACHE_H_ */
