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
 * Description:   Helper functions for generic linear page table
 */
#ifndef __LINEAR_PTAB_H__
#define __LINEAR_PTAB_H__

#include <kernel/arch/pgent.h>
#include <kernel/space.h>


/**
 * Array containing the actual page sizes (as bit-shifts) for the
 * various page table numbers.  Array is indexed by page size number
 * (i.e., pgent_t::pgsize_e).  Last entry must be the bit-shift for the
 * complete address space.
 */
extern const word_t hw_pgshifts[];

#ifdef CONFIG_PT_LEVEL_SKIP
extern const word_t real_hw_pgshifts[];
#endif


/*
 * Define some operators on the pgsize enum to make code more
 * readable.
 */

INLINE pgent_t::pgsize_e operator-- (pgent_t::pgsize_e & l, int)
{
    pgent_t::pgsize_e ret = l;
    l = (pgent_t::pgsize_e) ((word_t) l - 1);
    return ret;
}

#ifdef CONFIG_PT_LEVEL_SKIP
INLINE pgent_t::hw_pgsize_e operator-- (pgent_t::hw_pgsize_e & l, int)
{
       pgent_t::hw_pgsize_e ret = l;
       l = (pgent_t::hw_pgsize_e) ((word_t) l - 1);
       return ret;
}
#endif

INLINE pgent_t::pgsize_e operator++ (pgent_t::pgsize_e & l, int)
{
    pgent_t::pgsize_e ret = l;
    l = (pgent_t::pgsize_e) ((word_t) l + 1);
    return ret;
}

INLINE pgent_t::pgsize_e operator+ (pgent_t::pgsize_e l, int r)
{
    return (pgent_t::pgsize_e) ((word_t) l + r);
}

INLINE pgent_t::pgsize_e operator- (pgent_t::pgsize_e l, int r)
{
    return (pgent_t::pgsize_e) ((word_t) l - r);
}


/**
 * Get page size in bytes for given page size number.
 *
 * @param pgsize        page size number
 *
 * @return number of bytes for given page size
 */
word_t page_size (pgent_t::pgsize_e pgsize) PURE;

INLINE word_t page_size (pgent_t::pgsize_e pgsize)
{
    return 1UL << hw_pgshifts[pgsize];
}


/**
 * Get page size in shift length for given page size.
 *
 * @param pgsize        page size number
 *
 * @return page shift for given page size
 */
word_t page_shift (pgent_t::pgsize_e pgsize) PURE;

INLINE word_t page_shift (pgent_t::pgsize_e pgsize)
{
    return hw_pgshifts[pgsize];
}


/**
 * Get page mask for a given page size (i.e., bitmask which mask out
 * least significant bits).
 *
 * @param pgsize        page size number
 *
 * @return address mask for given page size number
 */
word_t page_mask (pgent_t::pgsize_e pgsize) PURE;

INLINE word_t page_mask (pgent_t::pgsize_e pgsize)
{
    return ((1UL << hw_pgshifts[pgsize]) - 1);
}


/**
 * Get number of entries for page table of a given page size.
 *
 * @param pgsize        page size number
 *
 * @return page size (number of entries) for given page size
 */
INLINE word_t page_table_size (pgent_t::pgsize_e pgsize)
{
    return 1UL << (hw_pgshifts[pgsize+1] - hw_pgshifts[pgsize]);
}


/**
 * Get page table index of virtual address for a particular page size.
 *
 * @param pgsize        page size number
 * @param vaddr         virtual address
 *
 * @return page table index for a table of the given page size
 */
INLINE word_t page_table_index (pgent_t::pgsize_e pgsize, addr_t vaddr)
{
    return ((word_t) vaddr >> hw_pgshifts[pgsize]) &
        (page_table_size (pgsize) - 1);
}

#ifdef CONFIG_PT_LEVEL_SKIP
/**
 * Round an address down to the given hardware pagesize
 *
 * @param addr          the address to round down
 * @param hw_pgsize     hardware page size number
 *
 * @return rounded address
 */
INLINE addr_t hw_round_down (addr_t addr, pgent_t::hw_pgsize_e hw_pgsize)
{
    u64_t phys = (u64_t)(word_t)addr;
    phys &= ~(((u64_t)1 << real_hw_pgshifts[hw_pgsize])-1);
    // XXX Add PAE support
    return (addr_t)(word_t)phys;
}

/**
 * Get hardware page size in shift length for given page size.
 *
 * @param hw_pgsize     page size number
 *
 * @return hardware page shift for given page size
 */
word_t hw_page_shift (pgent_t::hw_pgsize_e hw_pgsize) PURE;

INLINE word_t hw_page_shift (pgent_t::hw_pgsize_e hw_pgsize)
{
    return real_hw_pgshifts[hw_pgsize];
}
#endif


/**
 * Check if page size is supported by hardware.
 *
 * @param pgsize        page size number
 *
 * @return true if page size is supported by hardware, false otherwise
 */
bool is_page_size_valid (pgent_t::pgsize_e pgsize) PURE;

INLINE bool is_page_size_valid (pgent_t::pgsize_e pgsize)
{
    return (1UL << hw_pgshifts[pgsize]) & HW_VALID_PGSIZES;
}

/**
 * Safely read from memory.  If memory is a user address, the page
 * tables are parsed to find the physical address to read from.  If
 * memory is not a user-address, the memory is accessed directly.
 *
 * @param space         space to read from
 * @param vaddr         virtual memory location to read from
 * @param v             returned value
 *
 * @return true if memory could be read, false otherwise
 */
template<typename T>
INLINE bool readmem (space_t * space, addr_t vaddr, T * v)
{
    word_t w;

    /* Is it a request for physical mem? */
    if (space == (void*)-1UL)
    {
        addr_t paddr = vaddr;
        addr_t paddr1 = addr_mask (paddr, ~(sizeof (word_t) - 1));

        if (paddr1 == paddr)
        {
            // Word access is properly aligned.
            w = get_kernel_space()->readmem_phys (vaddr, paddr);
        }
        else
        {
            addr_t vaddr1 = addr_mask (vaddr, ~(sizeof (word_t) - 1));
            addr_t vaddr2 = addr_offset (vaddr1, sizeof (word_t));
            addr_t paddr2 = addr_offset (paddr1, sizeof (word_t));

            word_t idx = ((word_t) vaddr) & (sizeof (word_t) - 1);

#if defined(CONFIG_BIGENDIAN)
            w = (get_kernel_space()->readmem_phys (vaddr1, paddr1) << (idx * 8)) |
                (get_kernel_space()->readmem_phys (vaddr2, paddr2) >> ((sizeof (word_t) - idx) * 8));
#else
            w = (get_kernel_space()->readmem_phys (vaddr1, paddr1) >> (idx * 8)) |
                (get_kernel_space()->readmem_phys (vaddr2, paddr2) << ((sizeof (word_t) - idx) * 8));
#endif
        }
    }
    else
    {
        if (! space->is_user_area (vaddr))
        {
            // We are not reading user memory.  Just access it directly
            *v = *(T *) vaddr;
            return true;
        }

        // Check if memory is accessible
        if (! space->readmem (vaddr, &w))
            return false;
    }

#if defined(CONFIG_BIGENDIAN)
    w >>= (sizeof (word_t) - sizeof (T)) * 8;
#endif

    switch (sizeof (T))
    {
    case 1: *v = (T) (w & 0xff); break;
    case 2: *v = (T) (w & 0xffff); break;
    case 4: *v = (T) (w & 0xffffffff); break;
    case 8: *v = (T) (w); break;
    }

    return true;
}


#endif /* !__LINEAR_PTAB_H__ */
