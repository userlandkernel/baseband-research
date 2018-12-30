/*
 * Copyright (c) 2002, 2004-2003, Karlsruhe University
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
 * Copyright (c) 2005, National ICT Australia
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
 * Description:   Generic page table manipluation for IA-32
 */
#ifndef __ARCH__IA32__PGENT_H__
#define __ARCH__IA32__PGENT_H__

#include <kernel/kmemory.h>
#include <kernel/debug.h>
#include <kernel/arch/config.h>
#include <kernel/arch/hwspace.h>
#include <kernel/arch/ptab.h>
#include <kernel/arch/mmu.h>

/* ARCH specific pagefault identifiers */
#define ARCH_READ       0
#define ARCH_WRITE      2
#define ARCH_READWRITE  -1
#define ARCH_EXECUTE    0

#define HW_PGSHIFTS             { 12, 22, 32 }
#define HW_VALID_PGSIZES        ((1 << 12) | (1 << 22))

#define MDB_BUFLIST_SIZES       { {12}, {8}, {4096}, {0} }
#define MDB_PGSHIFTS            { 12, 22, 32 }
#define MDB_NUM_PGSIZES         (2)


EXTERN_KMEM_GROUP (kmem_pgtab);

class generic_space_t;

class pgent_t
{
public:
    union {
        ia32_pgent_t    pgent;
        u32_t           raw;
    };

    enum pgsize_e {
        size_4k = 0,
        size_4m = 1,
        min_tree = size_4m,
        size_4g = 2,
        size_max = size_4m,
        size_min = size_4k
    };

private:

    // Linknode access
#if !defined(CONFIG_SMP)
    void sync (generic_space_t * s, pgsize_e pgsize) { }
#else

    pgent_t * get_pgent_cpu(unsigned cpu)
        { return (pgent_t*) ((word_t) this + IA32_PAGE_SIZE * cpu); }

    void sync (generic_space_t * s, pgsize_e pgsize)
        {
            // synchronize first level entries only
            if (pgsize == size_4m)
                for (unsigned cpu = 1; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
                {
                    pgent_t * cpu_pgent = get_pgent_cpu(cpu);
                    //printf("synching %p=%p\n", cpu_pgent, this);
                    *cpu_pgent = *this;
                }
        }
public:
    // creates a CPU local subtree for CPU local data
    void make_cpu_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            ASSERT(ALWAYS, kernel); // cpu-local subtrees are _always_ kernel
            addr_t base = kmem.alloc (kmem_pgtab, IA32_PAGE_SIZE, true);
            ASSERT(ALWAYS, base);
            pgent.set_ptab_entry (virt_to_phys (base),
                            IA32_PAGE_USER|IA32_PAGE_WRITABLE);
        }
#endif /* CONFIG_SMP */

public:

    // Predicates

    bool is_valid (generic_space_t * s, pgsize_e pgsize)
        { return pgent.is_valid (); }

    bool is_writable (generic_space_t * s, pgsize_e pgsize)
        { return pgent.is_writable (); }

    bool is_readable (generic_space_t * s, pgsize_e pgsize)
        { return pgent.is_valid(); }

    bool is_executable (generic_space_t * s, pgsize_e pgsize)
        { return pgent.is_valid(); }

    bool is_subtree (generic_space_t * s, pgsize_e pgsize)
        { return (pgsize == size_4m) && !pgent.is_superpage (); }

    bool is_kernel (generic_space_t * s, pgsize_e pgsize)
        { return pgent.is_kernel(); }

    // Retrieval

    addr_t address (generic_space_t * s, pgsize_e pgsize)
        { return pgent.get_address (); }

    pgent_t * subtree (generic_space_t * s, pgsize_e pgsize)
        { return (pgent_t *) phys_to_virt (pgent.get_ptab ()); }

    word_t reference_bits (generic_space_t * s, pgsize_e pgsize, addr_t vaddr)
        {
            word_t rwx = 0;
#if defined(CONFIG_SMP)
            if (pgsize == size_4m)
                for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
                {
                    pgent_t * cpu_pgent = get_pgent_cpu (cpu);
                    if (cpu_pgent->pgent.is_accessed ()) { rwx |= 5; }
                    if (cpu_pgent->pgent.is_dirty ()) { rwx |= 6; }
                }
            else
#endif
            {
                if (pgent.is_accessed ()) { rwx |= 5; }
                if (pgent.is_dirty ()) { rwx |= 6; }
            }
            return rwx;
        }

    // Modification

    void clear (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
            pgent.clear ();
            sync (s, pgsize);

        }

    void flush (generic_space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
        }

    bool make_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            addr_t base = kmem.alloc (kmem_pgtab, IA32_PAGE_SIZE, true);
            if (!base) return false;

            pgent.set_ptab_entry (virt_to_phys (base),
                            IA32_PAGE_USER|IA32_PAGE_WRITABLE);

            sync(s, pgsize);
            return true;
        }

    void remove_subtree (generic_space_t * s, pgsize_e pgsize, bool kernel)
        {
            addr_t ptab = pgent.get_ptab ();
            pgent.clear();
            sync(s, pgsize);
            kmem.free (kmem_pgtab, phys_to_virt (ptab),
                    IA32_PAGE_SIZE);

        }

    void set_entry (generic_space_t * s, pgsize_e pgsize,
                    addr_t paddr, bool readable,
                    bool writable, bool executable,
                    bool kernel, memattrib_e attrib)
        {
            pgent.set_entry (paddr, writable,
                             (ia32_pgent_t::pagesize_e) pgsize,
                             (kernel ? (IA32_PAGE_KERNEL
                                        | IA32_PAGE_GLOBAL
                                 )
                                     : IA32_PAGE_USER) |
                             IA32_PAGE_VALID);
            this->pgent.set_cacheability(attrib);


            sync(s, pgsize);

        }

    void set_entry (generic_space_t * s, pgsize_e pgsize,
                    addr_t paddr, bool readable,
                    bool writable, bool executable,
                    bool kernel)
        {
            set_entry(s, pgsize, paddr, readable, writable, executable, kernel, cached);
        }

    void set_entry (generic_space_t * s, pgsize_e pgsize, pgent_t pgent)
        {
            this->pgent = pgent.pgent;
            sync(s, pgsize);
        }

    memattrib_e get_attributes (generic_space_t * s, pgsize_e pgsize)
        {
            return this->pgent.get_cacheability();
        }

    void set_global (generic_space_t * s, pgsize_e pgsize, bool global)
        {
            pgent.set_global (global);
            sync (s, pgsize);
        }

    void revoke_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        { if (rwx & 2) raw &= ~IA32_PAGE_WRITABLE; sync(s, pgsize); }

    void update_rights (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        { if (rwx & 2) raw |= IA32_PAGE_WRITABLE; sync(s, pgsize); }

    void reset_reference_bits (generic_space_t * s, pgsize_e pgsize)
        { raw &= ~(IA32_PAGE_ACCESSED | IA32_PAGE_DIRTY); sync(s, pgsize); }

    void update_reference_bits (generic_space_t * s, pgsize_e pgsize, word_t rwx)
        { raw |= ((rwx >> 1) & 0x3) << 5; }

    // Movement

    pgent_t * next (generic_space_t * s, pgsize_e pgsize, word_t num)
        { return this + num; }

    // Debug

    void dump_misc (generic_space_t * s, pgsize_e pgsize)
        {
            if (pgent.pg.global)
                printf ("global ");

            printf (pgent.pg.cache_disabled ? "UC" :
                    pgent.pg.write_through  ? "WT" : "WB");
        }
};


#endif /* !__ARCH__IA32__PGENT_H__ */
