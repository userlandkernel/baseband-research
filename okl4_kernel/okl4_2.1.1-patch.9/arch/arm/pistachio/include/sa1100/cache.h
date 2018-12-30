/*
 * Copyright (c) 2003-2004, National ICT Australia (NICTA)
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
 * Description:   Functions which manipulate the SA-1100 cache
 */

#ifndef __PLATFORM__ARM__SA1100_CACHE_H_
#define __PLATFORM__ARM__SA1100_CACHE_H_

#include <debug.h>
#include <cache.h>

#define ZERO_BANK_VADDR         (IO_AREA1_VADDR)

/* No write-though mode on SA1100 - revert to uncached */
#define CPU_WRITETHROUGH        0

class arm_cache
{
public:

    static inline void cache_flush(void)
    {
        cache_flush_i();
        cache_flush_d();
    }

    static inline void cache_flush_i(void)
    {
        word_t zero_bank = ZERO_BANK_VADDR;
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* Invalidate I-cache */
            "                                   \n" /* clean cache routine */
            "   add     r1,     %0, #8192       \n" /* taken from sa1100   */
            "1:                                 \n" /* reference manual    */
            "   ldr     r2,     [%0], #32       \n" /* page 58 (sect 6.2.3)*/
            "   teq     r1,     %0              \n"
            "   bne     1b                      \n"
            "                                   \n"
            /* Should probably also flush mini-D cache here if used */
            "                                   \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            "                                   \n"
            : "+r" (zero_bank)
            :: "r0", "r1", "r2", "memory");
    }

    static inline void cache_flush_d(void)
    {
        word_t zero_bank = ZERO_BANK_VADDR;
        __asm__ __volatile__ (
            "                                   \n" /* clean cache routine */
            "   add     r0,     %0, #8192       \n" /* taken from sa1100   */
            "1:                                 \n" /* reference manual    */
            "   ldr     r1,     [%0], #32       \n" /* page 58 (sect 6.2.3)*/
            "   teq     r0,     %0              \n"
            "   bne     1b                      \n"
            "   mov     r0,     #0              \n"
            /* Should probably also flush mini-D cache here if used */
            "                                   \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            "                                   \n"
            : "+r" (zero_bank)
            :: "r0", "r1", "memory");
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

    static inline void cache_flush_i_ent(addr_t vaddr, word_t log2size)
    {
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* flush I cache */
            ::
            : "r0", "memory");
    }

    static inline void cache_flush_d_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush_d();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);

        cache_clean_invalidate_dlines(vaddr, size);
        cache_drain_write_buffer();
    }

    static inline void cache_flush_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {   /* for >32k, whole flush is better */
            cache_flush();
            return;
        }
        word_t size = 1ul << (log2size > 5 ? log2size : 5);

        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c5, 0   \n" /* flush I cache */
            ::: "r0");
        cache_clean_invalidate_dlines(vaddr, size);
        cache_drain_write_buffer();
    }

    static inline void cache_flush_range_attr(addr_t vaddr, word_t size, word_t attr)
    {
        size = ((size + 31) + ((word_t)vaddr & 31)) & (~31UL);
        vaddr = (addr_t)((word_t)vaddr & (~31UL));              // align addresses

        if (attr & CACHE_ATTRIB_MASK_D) /* D-cache */
        {
            if ((attr & CACHE_ATTRIB_OP_CLEAN_INVAL) == CACHE_ATTRIB_OP_CLEAN_INVAL)
            {
                if (size > (32*1024))
                    cache_flush_d();
                else
                    cache_clean_invalidate_dlines(vaddr, size);
            }
            else if ((attr & CACHE_ATTRIB_OP_INVAL))
                cache_invalidate_dlines(vaddr, size);
            else if ((attr & CACHE_ATTRIB_OP_CLEAN))
                cache_clean_dlines(vaddr, size);

            cache_drain_write_buffer();
        }

        if ((attr & CACHE_ATTRIB_INVAL_I) == CACHE_ATTRIB_INVAL_I) /* I-cache - invalidate */
        {
            __asm__ __volatile__ (
                "       mov     r0,     #0              \n"
                "       mcr     p15, 0, r0, c7, c5, 0   \n" /* flush I cache */
                ::: "r0"
            );
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
            "   mcr     p15, 0, r0, c8, c7, 0   \n"
        ::);
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
        word_t a = (word_t)vaddr;

        __asm__ __volatile__ (
            "   mcr     p15, 0, r0, c8, c5, 0   \n"     /* Invalidate I TLB */
            ::);
        for (word_t i=0; i < (1ul << log2size); i += ARM_PAGE_SIZE)
        {
            __asm__ __volatile__ (
                "   mcr     p15, 0, %0, c8, c6, 1   \n" /* Invalidate D TLB entry */
            :: "r" (a));
            a += ARM_PAGE_SIZE;
        }
    }

    static inline void cache_clean_invalidate_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c10, 1  \n" /* clean D cache line */
            "   mcr     p15, 0, %0, c7, c6, 1   \n" /* flush D cache line */
            "   add     %0,     %0,     #32     \n"
            "   cmp     r0,     %0              \n"
            "   bhi     1b                      \n"
            : "+r" (vaddr)
            : "r" (size)
            : "r0", "memory");
    }

    static inline void cache_clean_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c10, 1  \n" /* clean D cache line */
            "   add     %0,     %0,     #32     \n"
            "   cmp     r0,     %0              \n"
            "   bhi     1b                      \n"
            : "+r" (vaddr)
            : "r" (size)
            : "r0", "memory");
    }

    static inline void cache_invalidate_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "   add     r0,     %0,     %1      \n"
            "1:                                 \n"
            "   mcr     p15, 0, %0, c7, c6, 1   \n" /* flush D cache line */
            "   add     %0,     %0,     #32     \n"
            "   cmp     r0,     %0              \n"
            "   bhi     1b                      \n"
            : "+r" (vaddr)
            : "r" (size)
            : "r0", "memory");
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
            "   mcr     p15, 0, %0, c7, c6, 1 \n"
        :: "r" (target));
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
            "   mcr     p15, 0, %0, c7, c10, 1 \n"
        :: "r" (target));
    }

    static inline void cache_drain_write_buffer(void)
    {
        __asm__ __volatile__ (
            "   mov     r0,     #0              \n"
            "   mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */
            ::: "r0"
        );
    }
};

#endif /* __PLATFORM__ARM__SA1100_CACHE_H_ */
