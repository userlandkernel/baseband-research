/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Description:   pagetable management
 */
#ifndef __ARCH__IA32__PTAB_H__
#define __ARCH__IA32__PTAB_H__

#include <kernel/arch/ia32.h>

#if !defined(ASSEMBLY)

class pgent_t;

enum memattrib_e {
    cached      = 0,
    uncached    = 2,
    writethr    = 1,
};

INLINE memattrib_e to_memattrib(l4attrib_e x)
{
    switch (x) {
        case l4mem_default:
        case l4mem_cached:
        case l4mem_writeback:
        case l4mem_coherent:
            return cached;
        case l4mem_writethrough:
            return writethr;
        case l4mem_io_combined:
        case l4mem_uncached:
        case l4mem_io:
        default:
            return uncached;
    }
}

INLINE l4attrib_e to_l4attrib(memattrib_e x)
{
    switch (x) {
        case cached:
            return l4mem_cached;
        case uncached:
            return l4mem_uncached;
        case writethr:
            return l4mem_writethrough;
        default:
            return l4mem_uncached;
    }
}


class ia32_pgent_t
{
public:
    enum pagesize_e {
        size_4k = 0,
        size_4m = 1
    };

    // predicates
    bool is_valid()
        { return pg.present == 1; }

    bool is_writable()
        { return pg.rw == 1; }

    bool is_accessed()
        { return pg.accessed == 1; }

    bool is_dirty()
        { return pg.dirty == 1; }

    bool is_superpage()
        { return pg.size == 1; }

    bool is_kernel()
        { return pg.privilege == 0; }

    // retrival
    addr_t get_address()
        { return (addr_t) (raw & IA32_PAGE_MASK); }

    ia32_pgent_t * get_ptab()
        { return (ia32_pgent_t*)(raw & IA32_PAGE_MASK); }

    u32_t get_raw()
        { return raw; }

    // modification
    void clear()
        { raw = 0; }

    void set_entry(addr_t addr, bool writable, pagesize_e size, u32_t attrib)
        {
            if (size == size_4k)
                raw = ((u32_t)(addr) & IA32_PAGE_MASK);
            else
                raw = ((u32_t)(addr) & IA32_SUPERPAGE_MASK) | IA32_PAGE_SUPER;

            raw |= (attrib & IA32_PAGE_FLAGS_MASK) |
                (writable ? IA32_PAGE_WRITABLE : 0);
        }

    void set_ptab_entry(addr_t addr, u32_t attrib)
        {
            raw = ((u32_t)(addr) & IA32_PAGE_MASK) |
                IA32_PAGE_VALID |
                (attrib & IA32_PTAB_FLAGS_MASK);
        }

    void copy(const ia32_pgent_t pgent)
        {
            raw = pgent.raw;
        }

    u32_t get_pdir_idx(addr_t addr)
        {
            return (u32_t)addr >> IA32_PAGEDIR_BITS;
        }

    u32_t get_ptab_idx(addr_t addr)
        {
            return (((u32_t)addr) & (~IA32_PAGEDIR_MASK)) >> IA32_PAGE_BITS;
        }

    void set_cacheability (memattrib_e attr)
        {
            this->pg.write_through = !!(attr & 1);
            this->pg.cache_disabled = !!(attr & 2);
        }

    memattrib_e get_cacheability ()
        {
            return (memattrib_e)((this->pg.cache_disabled << 1) |
                    this->pg.write_through);
        }

    void set_global (bool global)
        {
            this->pg.global = global;
        }

private:
    union {
        struct {
            unsigned present            :1;
            unsigned rw                 :1;
            unsigned privilege          :1;
            unsigned write_through      :1;

            unsigned cache_disabled     :1;
            unsigned accessed           :1;
            unsigned dirty              :1;
            unsigned size               :1;

            unsigned global             :1;
            unsigned avail              :3;

            unsigned base               :20;
        } pg;
        u32_t raw;
    };

    friend class pgent_t;
};

#endif /* !ASSEMBLY */

#endif /* !__ARCH__IA32__PTAB_H__ */
