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
 * Description:   Functions which manipulate the XScale cache
 */

#ifndef __ARCH__ARM__XSCALE_CACHE_H_
#define __ARCH__ARM__XSCALE_CACHE_H_

#include <kernel/debug.h>
#include <kernel/arch/asm.h>
#include <kernel/cpu/syscon.h>

class generic_space_t;

extern word_t DCACHE_SIZE;
extern word_t DCACHE_LINE_SIZE;
extern word_t DCACHE_SETS;
extern word_t DCACHE_WAYS;
extern word_t ICACHE_SIZE;
extern word_t ICACHE_LINE_SIZE;
extern word_t ICACHE_SETS;
extern word_t ICACHE_WAYS;

/* Write-through mode on xscale */
#define CPU_WRITETHROUGH        2

/* XXX: These are seedy macros which lets us share code between gcc and rvct */
#if defined(__GNUC__)
#define LOOP_LABEL      "1:"
#define LOOP_TARGET     "1b"
#elif defined(__RVCT_GNU__)
#define LOOP_LABEL      "L1:"
#define LOOP_TARGET     "L1"
/* In gcc inline asm, operands are symbolic and mapped to variables in
 * a separate part, nor can the names contain '::'.  For RVCT the
 * operands are the actual variables.  Therefore for RVCT directly map
 * the symbolic name to the variable found in the platform namespace.
 */
#define ASM_CACHE_LINE_SIZE DCACHE_LINE_SIZE
#define ASM_CACHE_SIZE DCACHE_SIZE
#elif defined(_lint)
#else
#error "Can't handle inline assembler for selected toolchain"
#endif

class arm_cache
{
public:
    static bool full_flush_cheaper(word_t size)
    {
        return (size >= 32768 ? true : false);
    }

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
        word_t vstart = VAR_AREA_START;
        /* Use unmapped VAR_AREA */
        __asm__ __volatile__ (
            "   add     r0,     "_(vstart)", "_(ASM_CACHE_SIZE)"            ;"
            LOOP_LABEL"                                                 ;"
            "   mcr     p15, 0, "_(vstart)", c7, c2, 5                  ;"  /* Allocate Line */
            "   add     "_(vstart)", "_(vstart)", "_(ASM_CACHE_LINE_SIZE)"  ;"
            "   mcr     p15, 0, "_(vstart)", c7, c2, 5                  ;"  /* Allocate Line */
            "   add     "_(vstart)", "_(vstart)", "_(ASM_CACHE_LINE_SIZE)"  ;"
            "   mcr     p15, 0, "_(vstart)", c7, c2, 5                  ;"  /* Allocate Line */
            "   add     "_(vstart)", "_(vstart)", "_(ASM_CACHE_LINE_SIZE)"  ;"
            "   mcr     p15, 0, "_(vstart)", c7, c2, 5                  ;"  /* Allocate Line */
            "   add     "_(vstart)", "_(vstart)", "_(ASM_CACHE_LINE_SIZE)"  ;"
            "   cmp     r0,     "_(vstart)"                             ;"
            "   bhi     "LOOP_TARGET"                                   ;"
#if defined(__GNUC__)
            : [vstart] "+r" (vstart)
            : [ASM_CACHE_SIZE] "r" (DCACHE_SIZE), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r0"
#endif
        );
        cache_drain_wb();
    }

    static inline void cache_flush_i(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
             "  mcr     p15, 0, "_(zero)", c7, c5, 0    ;"      /* Flush I cache / BTB */
#if defined(__GNUC__)
            :: [zero] "r" (zero)
#endif
        );
    }

    static inline void cache_drain_wb(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, "_(zero)", c7, c10, 4   ;"      /* Drain Write buffer */
#if defined(__GNUC__)
            :: [zero] "r" (zero)
#endif
        );
    }

    static inline void cache_flush_i_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {
            cache_flush_i();
            return;
        }
        word_t size = 1UL << log2size;

        __asm__ __volatile__ (
            "    add     r1,     "_(vaddr)", "_(size)"                  ;"
            LOOP_LABEL"                                                 ;"
            /* flush I cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 1                  ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)"   ;"
            "    cmp     r1, "_(vaddr)"                                 ;"
            "    bhi     "LOOP_TARGET"                                  ;"
            "    mov     "_(vaddr)", #0                                 ;"
            /* flush BTB */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 6                  ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    static inline void cache_flush_d_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {
                cache_flush();
                return;
        }
        word_t size = 1UL << log2size;

        __asm__ __volatile__ (
            "    add     r1, "_(vaddr)", "_(size)"                    ;"
            LOOP_LABEL"                                               ;"
            /* clean D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 1               ;"
            /* flush D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c6, 1                ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)" ;"
            "    cmp     r1,     "_(vaddr)"                           ;"
            "    bhi     "LOOP_TARGET"                                ;"
            "    mov     "_(vaddr)",     #0                           ;"
            /* Should probably also flush mini-D cache here if used */
            /* drain write buffer */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 4               ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    static inline void cache_flush_ent(addr_t vaddr, word_t log2size)
    {
        if (log2size >= 15) {
                cache_flush();
                return;
        }
        word_t size = 1UL << log2size;

        __asm__ __volatile__ (
            "    add     r1, "_(vaddr)", "_(size)"                      ;"
            LOOP_LABEL"                                                 ;"
            /* clean D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 1                 ;"
            /* flush I cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 1                  ;"
            /* flush D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c6, 1                  ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)"   ;"
            "    cmp     r1, "_(vaddr)"                                 ;"
            "    bhi     "LOOP_TARGET"                                  ;"
            "    mov     "_(vaddr)",     #0                             ;"
            /* Should probably also flush mini-D cache here if used */
            /* drain write buffer */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 4                 ;"
            /* flush BTB */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 6                  ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    /* These require DCACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_clean_invalidate_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "    add     r1, "_(vaddr)", "_(size)"                    ;"
            LOOP_LABEL"                                               ;"
            /* clean D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 1               ;"
            /* flush D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c6, 1                ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)" ;"
            "    cmp     r1,     "_(vaddr)"                           ;"
            "    bhi     "LOOP_TARGET"                                ;"
            "    mov     "_(vaddr)",     #0                           ;"
            /* Should probably also flush mini-D cache here if used */
            /* drain write buffer */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 4               ;"
            /* flush BTB */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 6                ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    /* These require DCACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_clean_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "    add     r1,     "_(vaddr)",    "_(size)"               ;"
            LOOP_LABEL"                                                 ;"
            /* clean D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 1                 ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)"   ;"
            "    cmp     r1, "_(vaddr)"                                 ;"
            "    bhi     "LOOP_TARGET"                                  ;"
            "    mov     "_(vaddr)",     #0                             ;"
            /* Should probably also flush mini-D cache here if used */
            /* drain write buffer */
            "    mcr     p15, 0, "_(vaddr)", c7, c10, 4                 ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    /* These require DCACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_invalidate_dlines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "    add     r1, "_(vaddr)", "_(size)"                    ;"
            LOOP_LABEL"                                               ;"
            /* invalidate D cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c6, 1                ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)" ;"
            "    cmp     r1, "_(vaddr)"                               ;"
            "    bhi     "LOOP_TARGET"                                ;"
            "    mov     "_(vaddr)",     #0                           ;"
            /* Should probably also flush mini-D cache here if used */
            /* flush BTB */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 6                ;"
#if defined(__GNUC__)
            : [vaddr] "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
    }

    /* These require DCACHE_LINE_SIZE aligned addresses/size */
    static inline void cache_invalidate_ilines(addr_t vaddr, word_t size)
    {
        __asm__ __volatile__ (
            "    add     r1,     "_(vaddr)",    "_(size)"             ;"
            LOOP_LABEL"                                               ;"
            /* invalidate I cache line */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 1                ;"
            "    add     "_(vaddr)", "_(vaddr)", "_(ASM_CACHE_LINE_SIZE)" ;"
            "    cmp     r1,     "_(vaddr)"                           ;"
            "    bhi     "LOOP_TARGET"                                ;"
            "    mov     "_(vaddr)",     #0                           ;"
            /* Should probably also flush mini-D cache here if used */
            /* flush BTB */
            "    mcr     p15, 0, "_(vaddr)", c7, c5, 6                ;"
#if defined(__GNUC__)
            :[vaddr]  "+r" (vaddr)
            : [size] "r" (size), [ASM_CACHE_LINE_SIZE] "r" (DCACHE_LINE_SIZE)
            : "r1", "memory"
#endif
            );
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
                if (size > (32*1024))
                    cache_flush_d();
                else
                    cache_clean_invalidate_dlines(vaddr, size);
            }
            else if ((attr & CACHE_ATTRIB_OP_INVAL))
                cache_invalidate_dlines(vaddr, size);
            else if ((attr & CACHE_ATTRIB_OP_CLEAN))
                cache_clean_dlines(vaddr, size);
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
        word_t zero = 0;
        __asm__ __volatile__ (
            "   mcr     p15, 0, "_(zero)", c8, c7, 0   ;"      /* flush I+D TLB */
#if defined(__GNUC__)
            :: [zero] "r" (zero)
#endif
        );
        CPWAIT;
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
#if defined(_lint)
        /* Nothing */
#else
        word_t a = (word_t)vaddr;

        for (word_t i=0; i < (1UL << log2size); i += ARM_PAGE_SIZE)
        {
#if defined(__GNUC__)
            __asm__ __volatile__ (
                /* Invalidate I TLB entry */
                "    mcr     p15, 0, "_(a)", c8, c5, 1    ;"
                /* Invalidate D TLB entry */
                "    mcr     p15, 0, "_(a)", c8, c6, 1    ;"
            :: [a] "r" (a)
            );
#elif defined(__RVCT_GNU__)
            __asm__ __volatile__ {
                    mcr     p15, 0, a, c8, c5, 1
                    mcr     p15, 0, a, c8, c6, 1
            }
#else
#error unknown compiler!
#endif
            a += ARM_PAGE_SIZE;
        }
#endif
    }

    
    static inline void tlb_flush_debug(void)
    {
        printf("About to TLB flush... ");
        tlb_flush();
        printf("done.\n");
    }

    static inline void cache_enable()
    {
        __asm__ __volatile__ (
            "mov    r0, #0                   ;"
            "mcr    p15, 0, r0, c7, c10, 4  ;"  /* Drain the pending operations */
            "mrc    p15, 0, r0, c1, c0, 0   ;"  /* Get the control register */
            "orr    r0, r0, #0x1004         ;"  /* Set bit 12 - the I & D bit */
            "mcr    p15, 0, r0, c1, c0, 0   ;"  /* Set the control register */
#if defined(__GNUC__)
            ::: "r0"
#endif
        );
        CPWAIT;
    }

    static inline void cache_invalidate_d_line(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, "_(target)", c7, c6, 1 ;"
#if defined(__GNUC__)
        :: [target] "r" (target)
#endif
        );
        CPWAIT;
    }

    static inline void cache_invalidate_d()
    {
        __asm__ __volatile__ (
            "mov    r0, #0                   ;"
            "mcr p15, 0, r0, c7, c6, 0 ;"
#if defined(__GNUC__)
        ::: "r0"
#endif
        );
        CPWAIT;
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, "_(target)", c7, c10, 1 ;"
#if defined(__GNUC__)
        :: [target] "r" (target)
#endif
        );
        CPWAIT;
    }

    static inline void cache_drain_write_buffer(void)
    {
        word_t zero = 0;
        __asm__ __volatile__ (
            /* drain write buffer */
           "mcr p15, 0, "_(zero)", c7, c10, 4   ;"
#if defined(__GNUC__)
        :: [zero] "r" (zero)
#endif
        );
    }
};

#endif /* __ARCH__ARM__XSCALE_CACHE_H_ */
