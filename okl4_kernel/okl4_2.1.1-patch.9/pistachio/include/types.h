/*
 * Copyright (c) 2002, Karlsruhe University
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
 * Description: Defines some basic, global types.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#if !defined(ASSEMBLY)

#include <kernel/arch/types.h>
/* At this point we should have word_t defined */
#include <l4/types.h>

/**
 *      addr_t - data type to store addresses
 */
typedef void*                   addr_t;

#if defined(__cplusplus)
/**
 * Add offset to address.
 * @param addr          original address
 * @param off           offset to add
 * @return new address
 */
INLINE addr_t addr_offset(addr_t addr, addr_t off)
{
    return (addr_t)((word_t)addr + (word_t)off);
}
#endif

/**
 * Add offset to address.
 * @param addr          original address
 * @param off           offset to add
 * @return new address
 */
INLINE addr_t addr_offset(addr_t addr, word_t off)
{
    return (addr_t)((word_t)addr + off);
}

/**
 * Apply mask to an address.
 * @param addr          original address
 * @param mask          address mask
 * @return new address
 */
INLINE addr_t addr_mask (addr_t addr, word_t mask)
{
    return (addr_t) ((word_t) addr & mask);
}

/**
 * Align address downwards.  It is assumed that the alignment is a power of 2.
 * @param addr          original address
 * @param align         alignment
 * @return new address
 */
INLINE addr_t addr_align (addr_t addr, word_t align)
{
    return addr_mask (addr, ~(align - 1));
}

/**
 * Align address upwards.  It is assumed that the alignment is a power of 2.
 * @param addr          original address
 * @param align         alignment
 * @return new address
 */
INLINE addr_t addr_align_up (addr_t addr, word_t align)
{
    return addr_mask (addr_offset (addr, align - 1), ~(align - 1));
}

#define INVALID_ADDR ((addr_t)~0UL)

enum l4attrib_e {
    l4mem_default       = 0,    /* CPU specific optimimal/default page attributes (for code+data) */
    l4mem_cached        = 1,    /* Cached memory (CPU specific writeback/writethrough mode) */
    l4mem_uncached      = 2,    /* Uncached memory */
    l4mem_writeback     = 3,    /* Cached - write-back */
    l4mem_writethrough  = 4,    /* Cached - write-through */
    l4mem_coherent      = 5,    /* Cached-coherent */
    l4mem_io            = 6,    /* Device memory */
    l4mem_io_combined   = 7,    /* Device buffer memory - write-combined */
    /* Other numbers are CPU specific */
};

#ifndef NULL
#define NULL 0
#endif

/* Scheduling-related types */
typedef int prio_t;

// Mark as a data-structure alias - AGW
#define BITS_WORD WORD_T_BIT

typedef void (*continuation_t)(void);
#define CONTINUATION_FUNCTION(name) NORETURN void name (void)

#if defined(_lint)
void __asm_continuation(void);
#define ASM_CONTINUATION __asm_continuation
#else
#define ASM_CONTINUATION (continuation_t) __return_address()
#endif

/* System callback functions that need to activate the given
 * continuation when they complete. */
typedef void (*callback_func_t)(continuation_t);

extern u64_t get_current_time();

typedef u16_t execution_unit_t;
typedef u16_t scheduler_domain_t;

#if defined (CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
class cpu_context_t
{
public:
    inline void operator= (cpu_context_t a) { raw = a.raw; }
    inline bool operator== (const cpu_context_t rhs) const { return (raw == rhs.raw); }
    inline bool operator!= (const cpu_context_t rhs) const { return (raw != rhs.raw); }

    cpu_context_t root_context(void)
    {
        cpu_context_t root;
        root.domain = domain;
        root.unit = 0xFFFF;
        return root;
    }

    union {
        struct {
            execution_unit_t unit;
            scheduler_domain_t domain;
        };
        word_t raw;
    };
} ;
#else
typedef word_t cpu_context_t;
#endif

#endif /* !defined(ASSEMBLY) */

#endif /* !__TYPES_H__ */
