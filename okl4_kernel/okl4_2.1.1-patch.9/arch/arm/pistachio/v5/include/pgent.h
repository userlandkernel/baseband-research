/*
 * Copyright (c) 2002, 2003-2004, Karlsruhe University
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
 * Description:   Generic page table manipluation for ARMv5
 */
#ifndef __ARCH__ARM__V5__PGENT_H__
#define __ARCH__ARM__V5__PGENT_H__

#include <kernel/arch/fass.h>
//#include <arch/page.h>
#include <kernel/arch/ver/ptab.h>
//#include <cpu/cache.h>
#include <kernel/smallalloc.h>
//#include <arch/globals.h>

/* ARCH specific pagefault identifiers */
#define ARCH_READ       0
#define ARCH_WRITE      1
#define ARCH_READWRITE  2
#define ARCH_EXECUTE    3

#if defined(CONFIG_ARM_TINY_PAGES)
#error Tiny pages not working
#endif

#define PGTABLE_OFFSET      (VIRT_ADDR_PGTABLE - get_arm_globals()->phys_addr_ram)

EXTERN_KMEM_GROUP (kmem_pgtab);

class mapnode_t;
class generic_space_t;

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

INLINE bool has_tiny_pages (generic_space_t * space)
{
#if defined(CONFIG_ARM_TINY_PAGES)
    return true;
#else
    return false;
#endif
}

INLINE word_t arm_l2_ptab_size (generic_space_t * space)
{
    return has_tiny_pages (space) ? ARM_L2_SIZE_TINY : ARM_L2_SIZE_NORMAL;
}


class pgent_t
{
public:
    union {
        pgent_t *       tree;
        arm_l1_desc_t   l1;
        arm_l2_desc_t   l2;
        u32_t           raw;
    };

public:
    enum pgsize_e {
#if defined(CONFIG_ARM_TINY_PAGES)
        size_1k,
        size_4k,
        size_min    = size_1k,
#else
        size_4k     = 0,
        size_min    = size_4k,
#endif
        size_64k    = 1,
        size_1m     = 2,
        min_tree    = size_1m,
        size_level0 = 3,
        size_4g     = 4,
        size_max    = size_level0,
    };


private:


    void sync_64k (generic_space_t * s)
        {
            for (word_t i = 1; i < (arm_l2_ptab_size (s) >> 6); i++)
                ((u32_t *) this)[i] = raw;
        }

    void sync_64k (generic_space_t * s, word_t val)
        {
            for (word_t i = 1; i < (arm_l2_ptab_size (s) >> 6); i++)
                ((u32_t *) this)[i] = val;
        }

    void sync_4k (generic_space_t * s)
        {
            if (has_tiny_pages (s))
                for (word_t i = 1; i < 4; i++)
                    ((u32_t *) this)[i] = raw;
        }

public:

    // Predicates

    bool is_valid (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_level0:
                return (tree != NULL);
            case size_1m:
                return (l1.fault.zero != 0);
            case size_64k:
                return true;
            case size_4k:
                if (has_tiny_pages(s)) {
                    return true;
                } else {
                    return (l2.small.two == 2);
                }
#if defined(CONFIG_ARM_TINY_PAGES)
            case size_1k:
                return (l2.tiny.three == 3);
#endif
            default:
                return false;
            }
        }

    word_t is_window (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_level0:
                return 2;
            case size_1m:
                return ((l1.fault.zero == 0) && (l1.raw != 0)) ? 1 : 0;
            default:
                return 0;
            }
        }

    word_t is_window_level (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_level0:
                return 2;
            case size_1m:
                return 1;
            default:
                return 0;
            }
        }

    bool is_writable (generic_space_t * s, pgsize_e pgsize)
        {
            return pgsize == size_1m ?
                    l1.section.ap == (word_t)arm_l1_desc_t::rw :
                    /* 64k, 4k, 1k have ap0 in the same location */
                    l2.tiny.ap == (word_t)arm_l1_desc_t::rw;
        }

    /* in is_readable() and is_executable(), the fields are identical for
     * all pagesizes, hence the optimization to not check pgsize */
    bool is_readable (generic_space_t * s, pgsize_e pgsize)
        { return l1.section.ap >= (word_t)arm_l1_desc_t::ro; }

    bool is_executable (generic_space_t * s, pgsize_e pgsize)
        { return l1.section.ap >= (word_t)arm_l1_desc_t::ro; }

    bool is_subtree (generic_space_t * s, pgsize_e pgsize)
        {
            switch (pgsize)
            {
            case size_level0:
                return (tree != NULL);
            case size_1m:
                return (l1.section.two != 2) && (l1.fault.zero != 0);
            case size_64k:
                return (l2.large.one != 1);
            case size_4k:
                if (has_tiny_pages(s)) {
                    return (l2.small.two != 2);
                }
            default:
                return false;
            }
        }

    bool is_kernel (generic_space_t * s, pgsize_e pgsize)
        { return l1.section.ap <= (word_t)arm_l1_desc_t::none; }

    // Retrieval
    arm_domain_t get_domain(void)
        {
            return l1.section.domain;
        }

    addr_t address (generic_space_t * s, pgsize_e pgsize)
        {
#if defined(CONFIG_ARM_TINY_PAGES)
            if (pgsize == size_1k)
                return l1.tiny.address();
#endif
            /* The address for 256M, 1M, 64k, 4k is always padded to 20 bits on ARMv5 */
            return (addr_t)(raw & ~(0xfff));
        }

    pgent_t * subtree (generic_space_t * s, pgsize_e pgsize)
        {
            if (pgsize == size_level0)
            {
                return tree;
            }
            else if (pgsize == size_1m)
            {
                if (has_tiny_pages (s))
                    return (pgent_t *) phys_to_page_table(l1.address_finetable());
                else
                    return (pgent_t *) phys_to_page_table(l1.address_coarsetable());
            }
            else
                return this;
        }

    word_t reference_bits (generic_space_t * s, pgsize_e pgsize, addr_t vaddr)
        { return 7; }

    // Modification

private:

    void cpd_sync (generic_space_t * s);

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
            if (EXPECT_FALSE(pgsize == size_64k))
            {
                sync_64k (s, 0);
            }
#ifdef CONFIG_ENABLE_FASS
            else if (EXPECT_FALSE(pgsize == size_1m))
            {
                cpd_sync(s);
            }
#endif
#if defined(CONFIG_ARM_TINY_PAGES)
            else if (EXPECT_TRUE(pgsize == size_4k))
            {
                sync_4k (s);
            }
#endif
        }

    void flush (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
        }

    bool make_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            extern small_alloc_t l1_allocator;
            if (pgsize == size_level0)
            {
#if (ARM_L1_SIZE < KMEM_CHUNKSIZE)
                addr_t base = l1_allocator.allocate();
#else
                addr_t base = kmem.alloc (kmem_pgtab, ARM_L1_SIZE, true);
#endif
                if (base == NULL)
                    return false;
                tree = (pgent_t*)base;
            }
            else if (pgsize == size_1m)
            {
                if (has_tiny_pages (s))
                {
                    addr_t base = kmem.alloc (kmem_pgtab, ARM_L2_SIZE_TINY, true);
                    if (base == NULL)
                        return false;
                    l1.raw = 0;
                    l1.fine_table.three = 3;
                    l1.fine_table.base_address = (word_t)virt_to_ram(base) >> 12;
                    arm_cache::cache_flush_d_ent(base, ARM_L2_TINY_BITS);
                }
                else
                {
                    addr_t base = kmem.alloc (kmem_pgtab, ARM_L2_SIZE_NORMAL, true);
                    if (base == NULL)
                        return false;
                    l1.raw = 0;
                    l1.coarse_table.one = 1;
                    l1.coarse_table.base_address = (word_t)virt_to_ram(base) >> 10;
                    arm_cache::cache_flush_d_ent(base, ARM_L2_NORMAL_BITS);
                }
#ifdef CONFIG_ENABLE_FASS
                cpd_sync(s);
#endif
            }
            return true;
        }

    void remove_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            extern small_alloc_t l1_allocator;
            if (pgsize == size_level0)
            {
#if (ARM_L1_SIZE < KMEM_CHUNKSIZE)
                l1_allocator.free(tree);
#else
                kmem.free(kmem_pgtab, tree, ARM_L1_SIZE);
#endif
            }
            if (pgsize == size_1m)
            {
                if (has_tiny_pages (s))
                    kmem.free (kmem_pgtab, ram_to_virt (
                               (addr_t) (l1.fine_table.base_address << 12)),
                               ARM_L2_SIZE_TINY);
                else
                    kmem.free (kmem_pgtab, ram_to_virt (
                               (addr_t) (l1.coarse_table.base_address << 10)),
                               ARM_L2_SIZE_NORMAL);
            }

            clear (s, pgsize, kernel);
        }

    void set_window_faulting(generic_space_t * s, generic_space_t * target)
        {
            l1.raw = (word_t)target & (~0x3);
        }

    void set_window_callback(generic_space_t * s, generic_space_t * target)
        {
            l1.raw = (word_t)target & (~0x80000003);
        }

    void clear_window(generic_space_t * s)
        {
            l1.raw = 0x0;
        }

    void * get_window(generic_space_t * s)
        {
            return (void*)(l1.raw | 0x80000000);
        }

    bool is_callback(void)
        {
            return (l1.raw & 0x80000000) == 0;
        }

#define QUAD(x) (x | (x<<2) | (x<<4) | (x<<6))

    void set_entry (generic_space_t * s, pgsize_e pgsize,
                           addr_t paddr, bool readable,
                           bool writable, bool executable,
                           bool kernel, memattrib_e attrib)
        {
            word_t ap;
            arm_l1_desc_t base;

            if (EXPECT_FALSE(kernel))
                ap = writable ? QUAD((word_t)arm_l1_desc_t::none) : QUAD((word_t)arm_l1_desc_t::special);
            else
                ap = writable ? QUAD((word_t)arm_l1_desc_t::rw) : QUAD((word_t)arm_l1_desc_t::ro);

            base.raw = 0;
            /* attrib bits in the same position for all page sizes */
            base.section.attrib = (word_t)attrib;

            if (EXPECT_TRUE(pgsize == size_4k))
            {
                arm_l2_desc_t l2_entry;

                l2_entry.raw = base.raw;
                l2_entry.small.two = 2;
                l2_entry.small.ap = ap;
                l2_entry.small.base_address = (word_t) paddr >> 12;

                l2.raw = l2_entry.raw;
            }
            else if (pgsize == size_1m)
            {
                arm_l1_desc_t l1_entry;

                l1_entry.raw = base.raw;
                l1_entry.section.two = 2;
                l1_entry.section.domain = 0;
                l1_entry.section.ap = ap;
                l1_entry.section.base_address = (word_t) paddr >> 20;

                l1.raw = l1_entry.raw;
#ifdef CONFIG_ENABLE_FASS
                cpd_sync(s);
#endif
            }
#if defined(CONFIG_ARM_TINY_PAGES)
            else if (pgsize == size_1k)
            {
                arm_l2_desc_t l2_entry;

                l2_entry.raw = base.raw;
                l2_entry.tiny.three = 3;
                l2_entry.tiny.ap = ap;
                l2_entry.tiny.base_address = (word_t) paddr >> 10;

                l2.raw = l2_entry.raw;
            }
#endif
            else
            {   /* size_64k */
                arm_l2_desc_t l2_entry;

                l2_entry.raw = base.raw;
                l2_entry.large.one = 1;
                l2_entry.large.ap = ap;
                l2_entry.large.base_address = (word_t) paddr >> 16;

                l2.raw = l2_entry.raw;
                sync_64k (s);
            }
        }

    /* For init code, must be inline. Sets kernel and cached */
    inline void set_entry_1m (generic_space_t * s, addr_t paddr, bool readable,
                           bool writable, bool executable, memattrib_e attrib)
        {
            word_t ap;
            arm_l1_desc_t l1_entry;

            ap = writable ? (word_t)arm_l1_desc_t::none : (word_t)arm_l1_desc_t::special;

            l1_entry.raw = 0;
            l1_entry.section.two = 2;
            l1_entry.section.attrib = (word_t)attrib;
            l1_entry.section.domain = 0;
            l1_entry.section.ap = ap;
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
            word_t ap = (pgsize == size_1m) ? l1.section.ap : l2.tiny.ap;

            if ((rwx & 2) && (ap == (word_t)arm_l1_desc_t::rw))
            {
                ap = QUAD((word_t)arm_l1_desc_t::ro);

                switch (pgsize)
                {
                case size_1m:
                    l1.section.ap = ap;
#ifdef CONFIG_ENABLE_FASS
                    cpd_sync(s);
#endif
                    break;
                case size_64k:
                    l2.large.ap = ap;
                    sync_64k (s);
                    break;
                case size_4k:
                    l2.small.ap = ap;
                    sync_4k (s);
                    break;
#if defined(CONFIG_ARM_TINY_PAGES)
                case size_1k:
                    l2.tiny.ap = ap;
                    break;
#endif
                default:
                    break;
                }
            }
        }

    memattrib_e get_attributes (generic_space_t * s, pgsize_e pgsize)
        {
            /* bits in the same position for all page sizes */
            return (memattrib_e)l2.small.attrib;
        }


    void update_attributes(generic_space_t * s, pgsize_e pgsize, memattrib_e attrib)
        {
            /* bits in the same position for all page sizes */
            l1.section.attrib = (word_t)attrib;

            switch (pgsize) {
            case size_1m:
#ifdef CONFIG_ENABLE_FASS
                cpd_sync(s);
                break;
#endif
#if defined(CONFIG_ARM_TINY_PAGES)
            case size_1k:
                break;
#endif
            case size_64k:
                sync_64k (s);
                break;
            case size_4k:
                sync_4k (s);
                break;
            default:
                break;
            }
        }

    void update_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            word_t ap = pgsize == size_1m ? l1.section.ap : l2.tiny.ap;

            if ((rwx & 2) && ap == (word_t)arm_l1_desc_t::ro)
            {
                ap = QUAD((word_t)arm_l1_desc_t::rw);

                switch (pgsize)
                {
                case size_1m:
                    l1.section.ap = ap;
#ifdef CONFIG_ENABLE_FASS
                    cpd_sync(s);
#endif
                    break;
                case size_64k:
                    l2.large.ap = ap;
                    sync_64k (s);
                    break;
                case size_4k:
                    l2.small.ap = ap;
                    sync_4k (s);
                    break;
#if defined(CONFIG_ARM_TINY_PAGES)
                case size_1k:
                    l2.tiny.ap = ap;
                    break;
#endif
                default:
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
#if defined(CONFIG_ARM_TINY_PAGES)
            if (pgsize == size_4k)
            {
                return this + (num * 4);
            } else
#endif
            if (pgsize == size_64k)
            {
                return this + (num * (arm_l2_ptab_size (s) >> 6));
            }
            return this + num;
        }

    // Operators

    bool operator != (const pgent_t rhs)
        {
            return this->raw != rhs.raw;
        }

    // Debug

    void dump_misc (generic_space_t * s, pgsize_e pgsize)
        {
            printf(" %s",
                    get_attributes(s, pgsize) == cached ? "WB" :
                    get_attributes(s, pgsize) == uncached ? "NC" :
                    get_attributes(s, pgsize) == writethrough ? "WT" :
                    get_attributes(s, pgsize) == writecombine ? "WC" :
                    "??");
        }
};

#endif /* !__ARCH__ARM__V5__PGENT_H__ */
