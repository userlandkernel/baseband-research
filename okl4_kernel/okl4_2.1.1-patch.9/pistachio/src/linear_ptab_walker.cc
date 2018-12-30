/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   Linear page table manipulation
 */
#ifndef __GENERIC__LINEAR_PTAB_WALKER_CC__
#define __GENERIC__LINEAR_PTAB_WALKER_CC__

#include <l4.h>
#include <arch/pgent.h>
#include <fpage.h>
#include <tcb.h>
#include <space.h>
#include <schedule.h>

#include <kdb/tracepoints.h>
#include <linear_ptab.h>


DECLARE_TRACEPOINT (FPAGE_MAP);
DECLARE_TRACEPOINT (FPAGE_OVERMAP);
DECLARE_TRACEPOINT (FPAGE_UNMAP);
DECLARE_TRACEPOINT (FPAGE_READ);

DECLARE_KMEM_GROUP (kmem_pgtab);

#define TRACE_MAP(x...)
//#define TRACE_MAP(x...)               printf(x);
#define TRACE_UNMAP(x...)
//#define TRACE_UNMAP(x...)     printf(x);
#define TRACE_READ(x...)
//#define TRACE_READ(x...)      printf(x);

const word_t hw_pgshifts[] = HW_PGSHIFTS;

class mapnode_t;


/*
 * Helper functions.
 */

/* Note: this function returns 0x10 for a complete fpage rather than 0 */
static inline addr_t address (fpage_t fp, word_t size) PURE;
static inline addr_t address (fpage_t fp, word_t size)
{
    return (addr_t) (fp.raw & ~((1UL << size) - 1));
}

static inline word_t dbg_pgsize (word_t sz)
{
    return (sz >= GB (1) ? sz >> 30 : sz >= MB (1) ? sz >> 20 : sz >> 10);
}

static inline char dbg_szname (word_t sz)
{
    return (sz >= GB (1) ? 'G' : sz >= MB (1) ? 'M' : 'K');
}


/**
 * Map fpage to another address space. The access bits in the fpage
 * indicate which access rights the mappings in the destination address
 * space should have.
 *
 * @param base          physical address and page attributes
 * @param dest_fp       fpage description
 */
bool generic_space_t::map_fpage(phys_desc_t base, fpage_t dest_fp)
{
    word_t t_num = dest_fp.get_size_log2 ();
    pgent_t::pgsize_e pgsize, t_size;
    addr_t t_addr, p_addr;
    pgent_t * tpg;
    bool flush = false;

#ifdef CONFIG_PT_LEVEL_SKIP
    pgent_t::hw_pgsize_e hw_pgsize;
#endif

    pgent_t * r_tpg[pgent_t::size_max];
    word_t r_tnum[pgent_t::size_max];

    u64_t phys = base.get_base();

    /*
     * Check for valid base and page_size
     */
    if (EXPECT_FALSE(
                (phys & (((u64_t)1 << t_num)-1)) ||
                (t_num < page_shift(pgent_t::size_min))
                ))
    {
        get_current_tcb ()->set_error_code (EINVALID_PARAM);
        return false;
    }

    t_addr = address (dest_fp, t_num);

    if (t_num > page_shift(pgent_t::size_max+1))
        t_num = page_shift(pgent_t::size_max+1);

    memattrib_e page_attributes = to_memattrib(base.get_attributes());
    /*
     * Mask phys to correct page size
     */
    phys &= ~(((u64_t)1 << t_num)-1);
    // XXX Add PAE support
    p_addr = (addr_t)(word_t)phys;

    /*
     * Find pagesize to use, and number of pages to map.
     */
    for (pgsize = pgent_t::size_max; page_shift(pgsize) > t_num; pgsize --) { }

#ifdef CONFIG_PT_LEVEL_SKIP
    /*
     * Find hardware pagesize to use.
     */
    for (hw_pgsize = pgent_t::hw_size_max; hw_page_shift(hw_pgsize) > t_num; hw_pgsize --) { }
#endif

    t_num = 1UL << (t_num - page_shift(pgsize));
    t_size =  pgent_t::size_max;

    tpg = this->pgent(0)->next(this, t_size, page_table_index(t_size, t_addr));

    begin_update ();

    TRACEPOINT (FPAGE_MAP,
                word_t tsz = page_size (pgsize);
                word_t msz = dest_fp.get_size();
                printf ("map_fpage (fpg=%p  t_spc=%p) : "
                        "addr=%p, phys=%p (%ld) map=%d%cB, sz=%d%cB\n",
                        dest_fp.raw, this, t_addr, p_addr,
                        base.get_attributes(),
                        dbg_pgsize (msz), dbg_szname (msz),
                        dbg_pgsize (tsz), dbg_szname (tsz)));

    /*
     * Lookup destination pagetable, inserting levels when needed,
     * then insert mappings.
     */
    while (t_num > 0)
    {
        TRACE_MAP("  . LOOP: t_num = %d, t_size = %d, pgsize = %d, tpg = %p\n",
                t_num, t_size, pgsize, tpg);

        if (! is_user_area (t_addr))
            /* Do not mess with kernel area. */
            break;

        if (EXPECT_FALSE(t_size > pgsize))
        {
            /*
             * We are currently working on too large pages.
             */
            t_size --;
            r_tpg[t_size] = tpg->next (this, t_size+1, 1);
            r_tnum[t_size] = t_num - 1;
map_next_level:

            if (EXPECT_FALSE(! tpg->is_valid (this, t_size+1)))
            {
                /*
                 * Subtree does not exist. Create one.
                 */
                if (!tpg->make_subtree (this, t_size+1, false))
                    goto map_fpage_fail;
                TRACE_MAP("   - made subtree on tpg = %p\n", tpg);
            }
            else if (EXPECT_FALSE(! tpg->is_subtree (this, t_size+1)))
            {
                t_size ++;
                /*
                 * A larger mapping exists where a smaller mapping
                 * is being inserted.
                 */
                if ( /* Make sure that we do not overmap KIP,UTCB or kernel area */
                        !this->is_mappable(t_addr, addr_offset (t_addr, page_size (t_size)))
                   )
                {
                    goto map_next_pgentry;
                }

                /*
                 * Delete the larger mapping.
                 */
                /* We might have to flush some TLB entries */
                this->flush_tlbent_local (get_current_space (), t_addr, page_shift (t_size));

                tpg->clear(this, t_size, false, t_addr);
                TRACE_MAP("   - removed blocking superpage, tpg = %p, size %d, addr %p\n",
                        tpg, t_size, t_addr);

                /* XXX flush remote cpus here */

                /* restart with mapping removed */
                continue;
            }
            /*
             * Entry is valid and a subtree.
             */

            /*
             * Just follow tree and continue
             */
            tpg = tpg->subtree (this, t_size+1)->next
                (this, t_size, page_table_index (t_size, t_addr));
            continue;
        }
        else if (EXPECT_FALSE(tpg->is_valid (this, t_size) &&

                /* Check if we're simply extending access rights */
                (tpg->is_subtree (this, t_size) ||
                 (tpg->address (this, t_size) != p_addr)) &&

                /* Make sure that we do not overmap KIP,UTCB or kernel area */
                this->is_mappable(t_addr, addr_offset (t_addr, page_size (t_size)))
                ))
        {
            /*
             * We are doing overmapping.  Need to remove existing
             * mapping or subtree from address space.
             */
            TRACEPOINT (FPAGE_OVERMAP,
                        word_t tsz = page_size (t_size);
                        printf ("overmapping: "
                                "taddr=%p tsz=%d%cB "
                                "(%s)\n",
                                t_addr, dbg_pgsize (tsz), dbg_szname (tsz),
                                tpg->is_subtree (this, t_size) ?
                                "subtree" : "single map"));

            word_t num = 1;
            pgent_t::pgsize_e s_size = t_size;
            addr_t vaddr = t_addr;

            while (num > 0)
            {
                TRACE_MAP("    ovr - tpg = %p, t_size = %d, v_addr = %p\n", tpg, t_size, vaddr);
                if (! tpg->is_valid (this, t_size))
                {
                    /* Skip invalid entries. */
                }
                else if (tpg->is_subtree (this, t_size))
                {
                    /* We have to search each single page in the subtree. */
                    t_size--;

                    r_tpg[t_size] = tpg;
                    r_tnum[t_size] = num - 1;

                    tpg = tpg->subtree (this, t_size+1);
                    num = page_table_size (t_size);
                    continue;
                }
                else
                {
                    /* We might have to flush some TLB entries */
                    if (! this->does_tlbflush_pay (page_shift (t_size)))
                    {
                        this->flush_tlbent_local (get_current_space (), vaddr, page_shift (t_size));
                    }

                    tpg->clear(this, t_size, false, vaddr);
                }

                if (t_size < s_size)
                {
                    /* Skip to next entry. */
                    vaddr = addr_offset (vaddr, page_size (t_size));
                    tpg = tpg->next (this, t_size, 1);
                }

                num--;
                while (num == 0 && t_size < s_size)
                {
                    /* Recurse up and remove subtree. */
                    tpg = r_tpg[t_size];
                    num = r_tnum[t_size];
                    t_size++;
                    tpg->remove_subtree (this, t_size, false);
                    if (t_size < s_size)
                        tpg = tpg->next (this, t_size, 1);
                };

            }

            /* We might have to flush the TLB after removing mappings. */
            if (does_tlbflush_pay (page_shift (t_size)))
            {
                flush = true;
            }
        }
        else if (EXPECT_FALSE(tpg->is_valid (this, t_size) &&
                    tpg->is_subtree (this, t_size)))
        {
            /* Target mappings are of smaller page size.
             * Recurse to lower level
             */
            goto map_recurse_down;
        }

        if (EXPECT_FALSE( !is_page_size_valid (t_size) ||
                          (((word_t)t_size > 0) &&
                           !this->is_mappable(t_addr, addr_offset (t_addr, page_size (t_size))) )))
        {
            /*
             * Pagesize is ok but is not a valid hardware pagesize or
             * target mappings are of smaller page size.
             * Need to recurse to a smaller size.
             */
map_recurse_down:
            t_size--;
            r_tpg[t_size] = tpg->next (this, t_size+1, 1);
            r_tnum[t_size] = t_num - 1;

            t_num = page_table_size (t_size);
            goto map_next_level;
        }

        /*
         * If we get here `tpg' will be the page table entry that we
         * are going to change.
         */

        if (EXPECT_TRUE( this->is_mappable(t_addr) ))
        {
            bool valid = tpg->is_valid (this, t_size);

            /*
             * This is where the real work is done.
             */
            TRACE_MAP("  * set entry: %p: t_size %d, v_addr = %p, p_addr = %p, r=%d,w=%d,x=%d  attr=%d\n",
                tpg, t_size, t_addr, p_addr, dest_fp.is_read(),
                dest_fp.is_write(), dest_fp.is_execute(), base.get_attributes());

            /* We might need to flush some tlb entries */
            if (EXPECT_FALSE(valid && !flush))
            {
                if (!does_tlbflush_pay (page_shift (t_size)))
                    flush_tlbent_local (get_current_space (), t_addr, page_shift (t_size));
                else
                    flush = true;
            }

            tpg->set_entry (this,
#ifndef CONFIG_PT_LEVEL_SKIP
                    t_size, p_addr,
#else
                    hw_pgsize, hw_round_down(p_addr, hw_pgsize),
#endif
                    dest_fp.is_read(), dest_fp.is_write(),
                    dest_fp.is_execute(), false,
                    page_attributes);

            if (EXPECT_FALSE(valid && !flush))
            {
                /* XXX flush remote cpus here */
            }
        }

        /* Move on to the next map entry */
map_next_pgentry:

        t_addr = addr_offset (t_addr, page_size (t_size));
        p_addr = addr_offset (p_addr, page_size (t_size));
        t_num--;

        if (EXPECT_TRUE(t_num > 0))
        {
            /* Go to next entry */
            tpg = tpg->next (this, t_size, 1);
            continue;
        }
        else if (t_size < pgsize)
        {
            do {
                /* Recurse up */
                tpg = r_tpg[t_size];
                t_num = r_tnum[t_size];
                t_size++;
            } while (t_size < pgsize && t_num == 0);
        }
    }

    if (flush)
        this->flush_tlb (get_current_space ());

    end_update ();
    return true;

map_fpage_fail:

    end_update ();
    this->unmap_fpage (dest_fp, false);
    get_current_tcb ()->set_error_code (ENO_MEM);
    return false;
}

/**
 * Unmaps the indicated fpage.
 *
 * @param fpage''       fpage to unmap
 * @param unmap_all'    also unmap kernel mappings (i.e., UTCB and KIP)
 *
 * @returns_
 */
void generic_space_t::unmap_fpage(fpage_t fpage, bool unmap_all)
{
    pgent_t::pgsize_e size, pgsize;
    pgent_t * pg;
    addr_t vaddr;
    word_t num;
    bool flush = false;

    pgent_t *r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];

    num = fpage.get_size_log2 ();
    vaddr = address (fpage, num);

    TRACEPOINT (FPAGE_UNMAP,
                printf ("unmap_fpage (fpg=%p  t_spc=%p)\n",
                        fpage.raw, this));

    if (num < hw_pgshifts[0])
    {
        return;
    }

    /*
     * Some architectures may not support a complete virtual address
     * space.  Enforce unmaps to only cover the supported space.
     */

    if (num > page_shift(pgent_t::size_max+1))
        num = page_shift(pgent_t::size_max+1);

    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max; page_shift(pgsize) > num; pgsize--) {}

    num = 1UL << (num - page_shift(pgsize));
    size = pgent_t::size_max;
    pg = this->pgent(0)->next(this, size, page_table_index(size, vaddr));
    //printf("[1] pg=%p, *pg=%lx\n", pg, *pg);

    begin_update ();

    while (num)
    {
        TRACE_UNMAP("  . LOOP: num = %d, size = %d, pgsize = %d, pg = %p, vaddr = %p\n", num, size, pgsize, pg, vaddr);
        if (! is_user_area (vaddr))
            /* Do not mess with kernel area. */
            break;

        bool valid = pg->is_valid (this, size);
        bool subtree = pg->is_subtree(this, size);

        if (EXPECT_FALSE(size > pgsize))
        {
            /* We are operating on too large page sizes. */
            if (!valid)
                break;
            else if (subtree)
            {
                size--;
                pg = pg->subtree (this, size+1)->next
                    (this, size, page_table_index (size, vaddr));
                //printf("[2] pg=%p, *pg=%lx\n", pg, *pg);
                continue;
            }
            else
            {
                /*
                 * A larger mapping exists where a smaller page
                 * is being removed.
                 */
                if ( /* Make sure that we do not remove KIP,UTCB or kernel area */
                        !this->is_mappable(vaddr, addr_offset (vaddr, page_size (size)))
                   )
                {
                    goto unmap_next_pgentry;
                }

                /*
                 * Delete the larger mapping.
                 */
                TRACE_UNMAP("   - remove superpage, pg = %p, size %d, addr %p\n",
                        pg, size, vaddr);
                pg->clear(this, size, false, vaddr);

                /* We might have to flush some TLB entries */
                flush = true;

                /* restart with mapping removed */
                continue;
            }
        }

        if (EXPECT_FALSE(!valid))
            goto unmap_next_pgentry;

        if (EXPECT_FALSE(subtree))
        {
            if ( (size == pgent_t::min_tree) &&
                    (unmap_all ||
                     /* Make sure that we do not remove KIP and UTCB area */
                     this->is_mappable(vaddr, addr_offset (vaddr, page_size (size))))
               )
            {
                /* Just remove the subtree. */
                TRACE_UNMAP("   - remove tree, pg = %p, size %d, addr %p\n",
                        pg, size, vaddr);
                pg->remove_subtree (this, size, false);
                flush = true;
                goto unmap_next_pgentry;
            }
            else
            {
                /* We have to flush each single entry in the subtree. */
                size--;
                r_pg[size] = pg;
                r_num[size] = num - 1;

                pg = pg->subtree (this, size+1);
                num = page_table_size (size);
                continue;
            }
        }

        if (EXPECT_TRUE(is_mappable (vaddr) || unmap_all))
        {
            TRACE_UNMAP("   - remove %p: vaddr = %p\n", pg, vaddr);

            if (EXPECT_FALSE(!flush))
            {
                if (!does_tlbflush_pay (page_shift (size)))
                    flush_tlbent_local (get_current_space (), vaddr, page_shift (size));
                else
                    flush = true;
            }

            pg->clear (this, size, true, vaddr);

            /* XXX flush remote cpus here */
        }

unmap_next_pgentry:
        pg = pg->next (this, size, 1);
        //printf("[3] pg=%p, *pg=%lx\n", pg, *pg);
        vaddr = addr_offset (vaddr, page_size (size));
        num--;


        /* Delete all subtrees below fpage size */
        while (num == 0 && size < pgsize)
        {
            /* Recurse up */
            pg = r_pg[size];
            num = r_num[size];
            size++;

            fpage_t fp;
            fp.set ((word_t) vaddr - page_size (size), page_shift(size), false, false, false);

            if (unmap_all || is_mappable(fp))
            {
                TRACE_UNMAP("   - rm tree %p: size = %x\n", pg, page_size(size));
                pg->remove_subtree (this, size, false);
            }

            pg = pg->next (this, size, 1);
            //printf("[4] pg=%p, *pg=%lx\n", pg, *pg);
        }
    }

    if (flush)
    {
        flush_tlb (get_current_space ());
    }

    end_update ();
}

void generic_space_t::read_fpage(fpage_t fpage, phys_desc_t *base, perm_desc_t *perm)
{
    word_t num = fpage.get_size_log2 ();
    addr_t vaddr = address (fpage, num);
    word_t rwx, RWX;
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;
    bool exists;

    TRACEPOINT (FPAGE_READ,
                printf ("read_fpage (fpg=%p  t_spc=%p)\n",
                        fpage.raw, this));

    exists = lookup_mapping (vaddr, &pg, &pgsize);

    base->clear();
    perm->clear();

    if (exists)
    {
        base->set_base((word_t)pg->address(this, pgsize));
        base->set_attributes(to_l4attrib(pg->get_attributes(this, pgsize)));

        rwx = (pg->is_readable(this, pgsize) << 2) |
              (pg->is_writable(this, pgsize) << 1) |
              (pg->is_executable(this, pgsize) << 0);

        /* fpage specifies a smaller page than exists */
        RWX = pg->reference_bits(this, pgsize, vaddr);

        perm->set_perms(rwx);
#ifdef CONFIG_PT_LEVEL_SKIP
        perm->set_size(hw_page_shift(pg->hw_pgsize(this, pgsize)));
#else
        perm->set_size(page_shift(pgsize));
#endif
        perm->set_reference(RWX);
    }
}

/**
 * Read word from address space.  Parses page tables to find physical
 * address of mapping and reads the indicated word directly from
 * kernel-mapped physical memory.
 *
 * @param vaddr         virtual address to read
 * @param contents      returned contents in given virtual location
 *
 * @return true if mapping existed, false otherwise
 */
bool generic_space_t::readmem (addr_t vaddr, word_t * contents)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;

    if (! lookup_mapping (vaddr, &pg, &pgsize))
        return false;

    addr_t paddr = pg->address (this, pgsize);
    paddr = addr_offset (paddr, (word_t) vaddr & page_mask (pgsize));
    addr_t paddr1 = addr_mask (paddr, ~(sizeof (word_t) - 1));

    if (paddr1 == paddr)
    {
        // Word access is properly aligned.
        *contents = readmem_phys (vaddr, paddr);
    }
    else
    {
        // Word access not properly aligned.  Need to perform two
        // separate accesses.
        addr_t vaddr1 = addr_mask (vaddr, ~(sizeof (word_t) - 1));
        addr_t vaddr2 = addr_offset (vaddr1, sizeof (word_t));
        addr_t paddr2 = addr_offset (paddr1, sizeof (word_t));
        word_t mask = ~page_mask (pgsize);

        if (addr_mask (paddr1, mask) != addr_mask (paddr2, mask))
        {
            // Word access crosses page boundary.
            vaddr = addr_offset (vaddr, sizeof (word_t));
            if (! lookup_mapping (vaddr, &pg, &pgsize))
                return false;
            paddr2 = pg->address (this, pgsize);
            paddr2 = addr_offset (paddr2, (word_t) vaddr & page_mask (pgsize));
            paddr2 = addr_mask (paddr2, ~(sizeof (word_t) - 1));
        }

        word_t idx = ((word_t) vaddr) & (sizeof (word_t) - 1);

#if defined(CONFIG_BIGENDIAN)
        *contents =
            (readmem_phys (vaddr1, paddr1) << (idx * 8)) |
            (readmem_phys (vaddr2, paddr2) >> ((sizeof (word_t) - idx) * 8));
#else
        *contents =
            (readmem_phys (vaddr1, paddr1) >> (idx * 8)) |
            (readmem_phys (vaddr2, paddr2) << ((sizeof (word_t) - idx) * 8));
#endif
    }

    return true;
}


/**
 * Lookup mapping in address space.  Parses the page table to find a
 * mapping for the indicated virtual address.  Also returns located
 * page table entry and page size in R_PG and R_SIZE (if non-nil).
 *
 * @param vaddr         virtual address
 * @param r_pg          returned page table entry for mapping
 * @param r_size        returned page size for mapping
 *
 * @return true if mapping exists, false otherwise
 */
bool generic_space_t::lookup_mapping (addr_t vaddr, pgent_t ** r_pg,
                              pgent_t::pgsize_e * r_size)
{
    pgent_t * pg = this->pgent(0);
    pg = pg->next(this, pgent_t::size_max, page_table_index(pgent_t::size_max, vaddr));
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    for (;;)
    {
        if (pg->is_valid (this, pgsize))
        {
            if (pg->is_subtree (this, pgsize))
            {
                // Recurse into subtree
                if ((word_t)pgsize == 0)
                    return false;

                pg = pg->subtree (this, pgsize)->next
                    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
                pgsize--;
            }
            else
            {
                // Return mapping
                if (r_pg)
                    *r_pg = pg;
                if (r_size)
                    *r_size = pgsize;
                return true;
            }
        }
        else
            // No valid mapping or subtree
            return false;
    }

    NOTREACHED();
}

#endif /* !__GENERIC__LINEAR_PTAB_WALKER_CC__ */
