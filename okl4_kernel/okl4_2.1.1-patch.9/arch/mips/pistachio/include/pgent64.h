/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Description:   Generic page table manipulation for MIPS64
 * Author:        Carl van Schaik
 */
 
#ifndef __ARCH__MIPS__PGENT64_H__
#define __ARCH__MIPS__PGENT64_H__

#include <kernel/kmemory.h>
#include <kernel/debug.h>

#include <kernel/arch/config.h>
#include <kernel/arch/hwspace.h>
#include <kernel/arch/page.h>

#include <kernel/arch/tlb.h>


#define MIPS_TCB_SET(addr)      (addr_t)((word_t)(addr) | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1)))
#define MIPS_TCB_CLEAR(addr)    (addr_t)((word_t)(addr) & ~(1UL << (hw_pgshifts[pgent_t::size_max+1]-1)))

EXTERN_KMEM_GROUP (kmem_pgtab);

extern const word_t hw_pgshifts[];

class generic_space_t;

class pgent_t
{
public:
    union {
        struct {
            BITFIELD5(word_t,
                subtree         : 56,   /* Pointer to subtree */
                pgsize          : 4,    /* pgsize_e */
                is_subtree      : 1,    /* 1 if valid subtree */
                is_valid        : 1,    /* 0 for subtrees */
                __rv2           : 2     /* Defined by translation */
                      );
        } tree;
        struct {
            BITFIELD6(word_t,
                __rv1           : 30,   /* Defined by translation */
                __pad           : 26,
                pgsize          : 4,    /* pgsize_e */
                is_subtree      : 1,    /* 0 for mapping */
                is_valid        : 1,    /* 1 if valid mapping */
                __rv2           : 2     /* Defined by translation */
                      );
        } map;
        u64_t           raw;
    };

    /*
     * tree (41) structure: 12->21->31->41      (2t->2g->2m->4k)
     * tree (45) structure: 12->21->29->37->45  (32t->128g->512m->2m->4k)
     */
    enum pgsize_e {
                size_4k = 0,
                size_min = size_4k,
                size_8k,   /* 1 */
                size_32k,  /* 2 */
                size_128k, /* 3 */
                size_512k, /* 4 */
                size_2m,   /* 5 */
                min_tree = size_2m,
                size_8m,   /* 6 */
                size_32m,  /* 7 */
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
                size_2g,   /* 8 */
                size_2t,   /* 9 */

                size_max = size_2g
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
                size_128m, /* 8 */
                size_512m, /* 9 */
                size_128g, /* 10 */
                size_32t,  /* 12 */

                size_max = size_32t
#else
#error We only support 40 and 44 bit address spaces!
#endif
    };

public:

    translation_t * translation (void)
        { return (translation_t *) this; }

    // Predicates

    bool is_valid (generic_space_t * s, pgsize_e pgsize)
        {
            return map.is_valid || (map.is_subtree &&
                                    map.pgsize < (word_t) pgsize);
        }

    bool is_subtree (generic_space_t * s, pgsize_e pgsize)
        {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            if (pgsize == size_2t || pgsize == size_2g || pgsize == size_2m)
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
                return tree.is_subtree;
            else
                return map.pgsize < (word_t) pgsize;
        }

    bool is_readable (generic_space_t * s, pgsize_e pgsize)
        {
            return translation ()->get_valid ();
        }

    bool is_writable (generic_space_t * s, pgsize_e pgsize)
        {
            return translation ()->get_dirty () &&
                    translation ()->get_valid ();
        }

    bool is_executable (generic_space_t * s, pgsize_e pgsize)
        {
            return translation ()->get_valid ();
        }

    bool is_kernel (generic_space_t * s, pgsize_e pgsize)
        {
            return translation ()->is_kernel();
        }

    memattrib_e get_attributes (generic_space_t * s, pgsize_e pgsize)
        {
            return (memattrib_e)translation ()->memattrib();
        }

    // Retrieval

    addr_t address (generic_space_t * s, pgsize_e pgsize)
        {
            return addr_mask (translation ()->phys_addr (),
                              ~((1UL << hw_pgshifts[pgsize]) - 1));
        }

    pgent_t * subtree (generic_space_t * s, pgsize_e pgsize)
        {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            if (pgsize == size_2t || pgsize == size_2g || pgsize == size_2m)
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
                return (pgent_t *) tree_to_addr ((addr_t) raw);
            else
                return (pgent_t *) this;
        }

    word_t reference_bits (generic_space_t * s, pgsize_e pgsize, addr_t vaddr)
        {
            word_t rwx = 0;
            if (translation ()->get_dirty ())
                rwx = 6;
            else if (translation ()->get_valid())
                rwx = 4;
            return rwx;
        }

    void update_reference_bits (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            // XXX: Implement me
        }

    // Modification

    void clear (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
            switch (pgsize)
            {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            case size_2m: case size_2g: case size_2t:
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
                raw = 0;
                break;
            default:
                tree.is_valid = 0;
                tree.is_subtree = 1;
                break;
            }
        }

    void flush (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
        }

    bool make_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            switch (pgsize) {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            /* tree (41) structure: 41->21->21->12      (2t->2g->2m->4k)
             * level sizes (10bits), (10bits), (9bits)
             */
            case size_2m:
                tree.subtree = (word_t)
                    kmem.alloc (kmem_pgtab, (1<<9) * sizeof(u64_t), true);
                if (!tree.subtree)
                    return false;
                tree.is_valid = 0;
                tree.is_subtree = 1;
                tree.pgsize = pgsize - 1;
                break;
            case size_2g: case size_2t:
                tree.subtree = (word_t)
                    kmem.alloc (kmem_pgtab, (1<<10) * sizeof(u64_t), true);
                if (!tree.subtree)
                    return false;
                tree.is_valid = 0;
                tree.is_subtree = 1;
                tree.pgsize = pgsize - 1;
                break;
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
            default:
                map.is_valid = 0;
                map.is_subtree = 1;
                map.pgsize = pgsize - 1;
                break;
            }
            return true;
        }

    void remove_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            switch (pgsize) {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            case size_2m:
            {
                //addr_t ptab = mips64_phys_to_virt (7, (addr_t) tree.subtree);
                addr_t ptab = (addr_t) raw; /* FIXME */
                raw = 0;
                kmem.free (kmem_pgtab, tree_to_addr(ptab), (1<<9) * sizeof(u64_t));
                break;
            }
            case size_2g: case size_2t:
            {
                //addr_t ptab = mips64_phys_to_virt (7, (addr_t) tree.subtree);
                addr_t ptab = (addr_t) raw; /* FIXME */
                raw = 0;
                kmem.free (kmem_pgtab, tree_to_addr(ptab), (1<<10) * sizeof(u64_t));
                break;
            }
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
            default:
                break;
            }
            raw = 0;
            map.is_subtree = 1;
            map.is_valid = 0;
            map.pgsize = pgsize;
        }

    /* This is technically an assignment operator */
    void set_entry(generic_space_t *space, pgsize_e pgsize, pgent_t pgent)
        {
            raw = pgent.raw;
        }

    void set_entry (generic_space_t * s, pgsize_e pgsize,
                           addr_t paddr, bool readable,
                           bool writable, bool executable, bool kernel)
        {
            translation_t newtr (
                                 l4default,
                                 readable || writable || executable, writable,
                                 kernel, paddr);
            raw = newtr.get_raw ();
            map.is_valid = 1;
            map.is_subtree = 0;
            map.pgsize = pgsize;
        }

    void set_entry (generic_space_t * s, pgsize_e pgsize,
                           addr_t paddr, bool readable,
                           bool writable, bool executable,
                           bool kernel, memattrib_e attrib)
        {
            translation_t newtr (
                                 attrib,
                                 readable || writable || executable, writable,
                                 kernel, paddr);
            raw = newtr.get_raw ();
            map.is_valid = 1;
            map.is_subtree = 0;
            map.pgsize = pgsize;
        }

    void revoke_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            translation_t * tr = translation ();
            if (rwx & 2) tr->set_dirty(0);
        }

    void update_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        {
            translation_t * tr = translation ();
            if (rwx & 2) tr->set_dirty(1);
        }

    void reset_reference_bits (generic_space_t * s, pgsize_e pgsize)
        { translation ()->reset_reference_bits (); }

    // Movement

    pgent_t * next (generic_space_t * s, pgsize_e pgsize, word_t num)
        {
#if CONFIG_MIPS_USER_ADDRESS_BITS == 40
            /* 2^ (21, 31, 41) -> next level */
            switch (pgsize) {
            case size_8k:
                return this + 2*num;
            case size_8m:
                return this + 4*num;
            case size_32k:
                return this + 8*num;
            case size_32m:
                return this + 16*num;
            case size_128k:
                return this + 32*num;
            case size_512k:
                return this + 128*num;

            case size_4k: case size_2m: case size_2g: case size_2t:
                return this + num;
#elif CONFIG_MIPS_USER_ADDRESS_BITS == 44
#error fixme
#endif
            }
        }

    // Debug

    void dump_misc (generic_space_t * s, pgsize_e pgsize)
        {
        }
};


#endif /* !__ARCH__MIPS__PGENT64_H__ */
