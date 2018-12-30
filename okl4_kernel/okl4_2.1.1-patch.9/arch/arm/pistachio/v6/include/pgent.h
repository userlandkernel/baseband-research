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
 * Description:   Generic page table manipluation for ARMv6
 * Author: Carl van Schaik, Jun 2006
 */
#ifndef __ARCH__ARMV6__PGENT_H__
#define __ARCH__ARMV6__PGENT_H__

#include <kernel/arch/ver/pgtable.h>
#include <kernel/arch/hwspace.h>       /* virt<->phys */
#include <kernel/arch/l2cache.h>

/* ARCH specific pagefault identifiers */
#define ARCH_READ       0
#define ARCH_WRITE      1
#define ARCH_READWRITE  2
#define ARCH_EXECUTE    3

#define PGTABLE_OFFSET      (VIRT_ADDR_PGTABLE - get_arm_globals()->phys_addr_ram)

EXTERN_KMEM_GROUP (kmem_pgtab);

class mapnode_t;
class generic_space_t;

typedef u32_t arm_domain_t;

template<typename T> INLINE T phys_to_page_table(T x)
{
    return (T) ((u32_t) x + PGTABLE_OFFSET);
}

/* before VM is initialised we cannot use the 
 * virtual addresses involved in PGTABLE offset
 * so use a pointer to the RAM address and do it
 * the hard way
 */ 
template<typename T> INLINE T phys_to_page_table_init(T x, word_t * p)
{
    return (T) ((u32_t) x + (VIRT_ADDR_PGTABLE - *p));
}

template<typename T> INLINE T virt_to_page_table(T x)
{
    return (T) ((u32_t) x - VIRT_ADDR_RAM + VIRT_ADDR_PGTABLE);
}

template<typename T> INLINE T page_table_to_virt(T x)
{
    return (T) ((u32_t) x - VIRT_ADDR_PGTABLE + VIRT_ADDR_RAM);
}

template<typename T> INLINE T page_table_to_phys(T x)
{
    return (T) ((u32_t) x - PGTABLE_OFFSET);
}


class pgent_t
{
public:
    union {
        armv6_l1_desc_t l1;
        armv6_l2_desc_t l2;
        u32_t           raw;
    };

public:
    enum pgsize_e {
        size_4k     = 0,
        size_min    = size_4k,
        size_64k    = 1,
        size_1m     = 2,
        min_tree    = size_1m,
        size_16m    = 3,
        size_4g     = 4,
        size_max    = size_16m,
    };


private:


    void sync_large (generic_space_t * s)
        {
            for (word_t i = 1; i < 16; i++)
                ((u32_t *) this)[i] = raw;
        }

    void sync_large (generic_space_t * s, word_t val)
        {
            for (word_t i = 1; i < 16; i++)
                ((u32_t *) this)[i] = val;
        }

public:

    // Predicates

    bool is_valid (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_1m:
                return (l1.fault.zero == 1) || (l1.fault.zero == 2);
            case size_16m:
            case size_64k:
                return true;
            case size_4k:
                return (l2.small.one == 1);
            default:
                return false;
            }
        }

    bool is_writable (generic_space_t * s, pgsize_e pgsize)
        {
            return pgsize >= size_1m ?
                    /* 16M, 1M have (ap) at the same location */
                    l1.section.ap == ap_rw :
                    /* 64k, 4k have (ap) at the same location */
                    l2.small.ap == ap_rw;
        }

    bool is_readable (generic_space_t * s, pgsize_e pgsize)
        {
            return pgsize >= size_1m ?
                    /* 16M, 1M have (ap) at the same location */
                    l1.section.ap >= ap_ro :
                    /* 64k, 4k have (ap) at the same location */
                    l2.small.ap >= ap_ro;
        }

    bool is_executable (generic_space_t * s, pgsize_e pgsize)
        {
            /* 16M, 1M have (nx) at the same location */
            return is_readable(s, pgsize) &&
                    (pgsize >= size_1m ? !l1.section.nx :
                    pgsize == size_64k ? !l2.large.nx :
                    !l2.small.nx);

        }

    bool is_kernel (generic_space_t * s, pgsize_e pgsize)
        {
            return pgsize >= size_1m ?
                    /* 16M, 1M have (ap) at the same location */
                    l1.section.ap == ap_na :
                    /* 64k, 4k have (ap) at the same location */
                    l2.small.ap == ap_na;
        }

    bool is_subtree (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_16m:
                return (l1.super_section.two != 2) || (l1.super_section.one != 1);
            case size_1m:
                return (l1.coarse_table.one == 1);
            case size_64k:
                return (l2.large.one != 1);
            case size_4k:
            default:
                return false;
            }
        }

    // Retrieval

    arm_domain_t get_domain(void)
        {
            return l1.section.domain;
        }

    addr_t address (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_16m:
                return l1.address_supersection();
            case size_1m:
                return l1.address_section();
            case size_64k:
                return l2.address_large();
            case size_4k:
            default:
                return l2.address_small();
            }
        }

    pgent_t * subtree (generic_space_t * s, pgsize_e pgsize)
        {
            if (pgsize == size_1m)
            {
                return (pgent_t *) phys_to_page_table(l1.address_coarsetable());
            }
            else
                return this;
        }

    word_t reference_bits (generic_space_t * s, pgsize_e pgsize, addr_t vaddr)
        { return 7; }

    // Modification

public:

    void set_domain(arm_domain_t domain)
        {
            l1.section.domain = domain;
        }

    void clear (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
            clear(s, pgsize, kernel);
        }

    void clear (generic_space_t * s, pgsize_e pgsize, bool kernel )
        {
            raw = 0;
            if (EXPECT_FALSE((pgsize == size_64k) || (pgsize == size_16m)))
            {
                sync_large (s, 0);
            }
        }

    void flush (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
        }

    bool make_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            if (pgsize == size_1m)
            {
                addr_t base = kmem.alloc (kmem_pgtab, ARM_L2_SIZE, true);
                if (base == NULL)
                    return false;
                l1.raw = 0;
                l1.coarse_table.one = 1;
                l1.coarse_table.base_address = (word_t)virt_to_ram(base) >> 10;
                arm_cache::cache_flush_d_ent(base, ARM_L2_BITS);
#if defined(CONFIG_USE_L2_CACHE) && !defined(CONFIG_ARM_HWWALKER_IN_L2)
                if (get_arm_l2cc()->is_enabled()) {
                    get_arm_l2cc()->cache_flush_by_pa_range(virt_to_ram(base), ARM_L2_BITS);
                }
#endif
            }
            return true;
        }

    void remove_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            if (pgsize == size_1m)
            {
                kmem.free (kmem_pgtab, ram_to_virt ((addr_t)l1.address_coarsetable()),
                        ARM_L2_SIZE);
            }

            clear (s, pgsize, kernel);
        }


    void set_entry (generic_space_t * s, pgsize_e pgsize,
                           addr_t paddr, bool readable,
                           bool writable, bool executable,
                           bool kernel, memattrib_e attrib)
        {
            raw = 0;
            word_t ap, apx;

            if (EXPECT_FALSE(kernel))
            {
                ap = ap_na;
                apx = writable ? 0 : 1;
            }
            else
            {
                ap = writable ? ap_rw : ap_ro;
                apx = 0;
            }

            if (EXPECT_TRUE(pgsize == size_4k))
            {
                armv6_l2_desc_t l2_entry;

                l2_entry.raw = 0;
                l2_entry.small.nx = !executable;
                l2_entry.small.one = 1;
                l2_entry.small.BC = (word_t)attrib & 0x3;
                l2_entry.small.ap = ap;
                l2_entry.small.tex = ((word_t)attrib >> 2) & 0x7;
                l2_entry.small.apx = apx;
                l2_entry.small.shared = ((word_t)attrib >> 7) & 0x1;
                l2_entry.small.nglobal = kernel ? 0 : 1;
                l2_entry.small.base_address = (word_t)paddr >> 12;

                l2.raw = l2_entry.raw;
            }
            else if (pgsize >= size_1m)
            {
                armv6_l1_desc_t l1_entry;

                l1_entry.raw = 0;
                l1_entry.section.two = 2;
                l1_entry.section.BC = (word_t)attrib & 0x3;
                l1_entry.section.nx = !executable;
                l1_entry.section.domain = 0;
                l1_entry.section.ecc = 0;       // XXX configure this?
                l1_entry.section.ap = ap;
                l1_entry.section.tex = ((word_t)attrib >> 2) & 0x7;
                l1_entry.section.apx = apx;
                l1_entry.section.shared = ((word_t)attrib >> 7) & 0x1;
                l1_entry.section.nglobal = kernel ? 0 : 1;

                if (pgsize == size_1m)
                {
                    l1_entry.section.base_address = (word_t) paddr >> 20;
                }
                else
                {   /* size_16m */
                    l1_entry.super_section.one = 1;
                    l1_entry.super_section.base_address = (word_t) paddr >> 24;
                }

                l1.raw = l1_entry.raw;

                if (pgsize != size_1m)
                {
                    sync_large (s);
                }
            }
            else
            {   /* size_64k */
                armv6_l2_desc_t l2_entry;

                l2_entry.large.one = 1;
                l2_entry.large.BC = (word_t)attrib & 0x3;
                l2_entry.large.ap = ap;
                l2_entry.large.apx = apx;
                l2_entry.large.shared = ((word_t)attrib >> 7) & 0x1;
                l2_entry.large.nglobal = kernel ? 0 : 1;
                l2_entry.large.tex = ((word_t)attrib >> 2) & 0x7;
                l2_entry.large.nx = !executable;
                l2_entry.large.base_address = (word_t)paddr >> 16;

                l2.raw = l2_entry.raw;
                sync_large (s);
            }
        }

    /* For init code, must be inline. Sets kernel and cached */
    inline void set_entry_1m (generic_space_t * s, addr_t paddr, bool readable,
                           bool writable, bool executable, memattrib_e attrib)
        {
            word_t ap, apx;
            armv6_l1_desc_t l1_entry;

            /* assume kernel mapping */
            ap = ap_na;
            apx = writable ? 0 : 1;

            l1_entry.raw = 0;
            l1_entry.section.two = 2;
            l1_entry.section.BC = (word_t)attrib & 0x3;
            l1_entry.section.nx = !executable;
            l1_entry.section.domain = 0;
            l1_entry.section.ecc = 0;   // XXX configure this?
            l1_entry.section.ap = ap;
            l1_entry.section.tex = ((word_t)attrib >> 2) & 0x7;
            l1_entry.section.apx = apx;
            l1_entry.section.shared = ((word_t)attrib >> 7) & 0x1;
            l1_entry.section.nglobal = 0;
            l1_entry.section.base_address = (word_t) paddr >> 20;

            l1.raw = l1_entry.raw;
        }

    inline void set_entry (generic_space_t * s, pgsize_e pgsize,
                           addr_t paddr, bool readable,
                           bool writable, bool executable,
                           bool kernel)
        {
            set_entry(s, pgsize, paddr, readable, writable, executable, kernel, l4default);
        }

    void revoke_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            /* 16M ap + nx same as 1M, 64k ap same as 4k */
            word_t ap = (pgsize >= size_1m) ? l1.section.ap : l2.small.ap;
            word_t nx = (pgsize >= size_1m) ? l1.section.nx :
                (pgsize == size_4k) ? l2.small.nx : l2.large.nx;

            if ((rwx & 2) && ap == ap_rw)
            {
                ap = ap_ro;

                switch (pgsize)
                {
                case size_16m:
                    l1.super_section.ap = ap;
                    sync_large(s);
                    break;
                case size_1m:
                    l1.section.ap = ap;
                    break;
                case size_64k:
                    l2.large.ap = ap;
                    sync_large(s);
                    break;
                case size_4k:
                default:
                    l2.small.ap = ap;
                    break;
                }
            }
            if ((rwx & 1) && (!nx))
            {
                switch (pgsize)
                {
                case size_16m:
                    l1.super_section.nx = 1;
                    sync_large(s);
                    break;
                case size_1m:
                    l1.section.nx = 1;
                    break;
                case size_64k:
                    l2.large.nx = 1;
                    sync_large(s);
                    break;
                case size_4k:
                default:
                    l2.small.nx = 1;
                    break;
                }
            }
        }

    memattrib_e get_attributes (generic_space_t * s, pgsize_e pgsize)
        {
            /* all page sizes have same BC bits */
            word_t ret = l1.section.BC;

            switch (pgsize)
            {
            case size_16m:  /* same as 1m */
            case size_1m:
                ret |= (l1.section.tex << 2) | (l1.section.shared << 7);
                break;
            case size_64k:
                ret |= (l2.large.tex << 2) | (l2.large.shared << 7);
                break;
            case size_4k:
            default:
                ret |= (l2.small.tex << 2) | (l2.small.shared << 7);
                break;
            }
            return (memattrib_e)ret;
        }


    void update_attributes(generic_space_t * s, pgsize_e pgsize, memattrib_e attrib)
        {
            word_t a = (word_t)attrib;
            /* all page sizes have same BC bits */
            l1.section.BC = a & 0x3;

            switch (pgsize) {
            case size_16m:
                l1.section.tex = (a >> 2) & 0x7;
                l1.section.shared = (a >> 7) & 0x1;
                sync_large(s);
                break;
            case size_1m:
                l1.section.tex = (a >> 2) & 0x7;
                l1.section.shared = (a >> 7) & 0x1;
                break;
            case size_64k:
                l2.large.tex = (a >> 2) & 0x7;
                l2.large.shared = (a >> 7) & 0x1;
                sync_large(s);
                break;
            case size_4k:
            default:
                l2.small.tex = (a >> 2) & 0x7;
                l2.small.shared = (a >> 7) & 0x1;
                break;
            }
        }

    void update_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            /* 16M ap + nx same as 1M, 64k ap same as 4k */
            word_t ap = (pgsize >= size_1m) ? l1.section.ap : l2.small.ap;
            word_t nx = (pgsize >= size_1m) ? l1.section.nx :
                (pgsize == size_4k) ? l2.small.nx : l2.large.nx;

            if ((rwx & 2) && ap == ap_ro)
            {
                ap = ap_rw;

                switch (pgsize)
                {
                case size_16m:
                    l1.super_section.ap = ap;
                    sync_large(s);
                    break;
                case size_1m:
                    l1.section.ap = ap;
                    break;
                case size_64k:
                    l2.large.ap = ap;
                    sync_large(s);
                    break;
                case size_4k:
                default:
                    l2.small.ap = ap;
                    break;
                }
            }
            if ((rwx & 1) && (nx))
            {
                switch (pgsize)
                {
                case size_16m:
                    l1.super_section.nx = 0;
                    sync_large(s);
                    break;
                case size_1m:
                    l1.section.nx = 0;
                    break;
                case size_64k:
                    l2.large.nx = 0;
                    sync_large(s);
                    break;
                case size_4k:
                default:
                    l2.small.nx = 0;
                    break;
                }
            }
        }

    void reset_reference_bits (generic_space_t * s, pgsize_e pgsize)
        { }

    void update_reference_bits (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        { }

    // Movement

    pgent_t * next (generic_space_t * s, pgsize_e pgsize, word_t num)
        {
            if ((pgsize == size_64k) || (pgsize == size_16m))
            {
                return this + (num * 16);
            }
            return this + num;
        }

    // Operators

    bool operator != (const pgent_t rhs)
        {
            return this->raw != rhs.raw;
        }

    // Debug

    void decode_bits(char *str, word_t bits) {
        switch (bits & 0x03) {
            case 0: str[0] = 'N'; str[1] = 'C'; break;
            case 1: str[0] = 'W'; str[1] = 'a'; break;
            case 2: str[0] = 'W'; str[1] = 'T'; break;
            case 3: str[0] = 'W'; str[1] = 'B'; break;
        }
    }

    void dump_misc (generic_space_t * s, pgsize_e pgsize)
        {
            char * str = NULL;
            memattrib_e attr = get_attributes(s, pgsize);
            switch (attr)
            {
            case strong:        str = "*ST"; break;
            case writethrough:  str = " WT"; break;
            case writeback:     str = " WB"; break;
            case uncached:      str = " NC"; break;
            case iomemory:      str = " IO"; break;
            case writethrough_MP    :str = "*WT"; break;
            case writeback_MP:  str = "*WB"; break;
            case uncached_MP:   str = "*NC"; break;
            case iomemory_MP:   str = "*IO"; break;
            default:
                if (((word_t)attr & 0xf0) == 0x10) {
                    str = "   _  ";
                    decode_bits(str+1, (word_t)attr & 0x03);
                    decode_bits(str+4, ((word_t)attr>>2) & 0x03);
                }
                break;
            }
            printf(" %s", str ? str : " ??");
        }
};

#endif /* !__ARCH__ARMV6__PGENT_H__ */
