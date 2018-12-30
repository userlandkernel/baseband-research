/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   MIPS32 TLB management
 * Author:        Carl van Schaik
 */

#ifndef __ARCH__MIPS__TLB32_H__
#define __ARCH__MIPS__TLB32_H__

#include <kernel/arch/mipsregs.h>

/* memory coherency attributes (map to EntryLo bits) */
enum memattrib_e {
    write_through_noalloc       = CONFIG_CACHABLE_NO_WA,
    write_through               = CONFIG_CACHABLE_WA,
    uncached            = CONFIG_NOCACHE,
    write_back          = CONFIG_CACHABLE_NONCOHERENT,
#if defined(CONFIG_UNCACHED)
    l4default           = uncached,
#else
#if defined(CONFIG_SMP)
    coherent            = CONFIG_CACHABLE_COW,
    l4default           = coherent,
#else
    l4default           = write_back,
#endif
#endif
};

/* MIPS specific memory attributes */
#define l4mem_writethrough_noalloc  8

INLINE memattrib_e to_memattrib(l4attrib_e x)
{
    switch (x) {
        case l4mem_coherent:
#if defined(CONFIG_SMP)
            return coherent;
#endif
        case l4mem_cached:
        case l4mem_default:
            return l4default;
        case l4mem_writeback:
            return write_back;
        case l4mem_writethrough:
            return write_through;
        case l4mem_io:
        case l4mem_io_combined:
        case l4mem_uncached:
            return uncached;
        case l4mem_writethrough_noalloc:
            return write_through_noalloc;
        default:
            return uncached;
    }
}

INLINE l4attrib_e to_l4attrib(memattrib_e x)
{
    switch (x) {
#if defined(CONFIG_SMP)
        case coherent:
            return l4mem_coherent;
#endif
        case uncached:
            return l4mem_uncached;
        case write_through_noalloc:
            return (l4attrib_e)l4mem_writethrough_noalloc;
        case write_through:
            return l4mem_writethrough;
        case write_back:
            return l4mem_writeback;
        default:
            return l4mem_uncached;
    }
}

/**
 * A translation specifying a mapping to a physical address, including
 * page access rights, memory attributes, etc.  Virtual address, page
 * size, and so on are specified when inserting the translation into
 * the TLB.
 */
class translation_t
{
    union {
        struct {
            BITFIELD6(u32_t,
                global          : 1,
                valid           : 1,
                dirty           : 1,
                memory_attrib   : 3,
                phys_addr       : 24,
                __rv1           : 2
            );
        } x;
        u32_t raw;
    };

public:


    /*
     * Retrieval
     */

    /*
     * reset accessed and dirty bit for page
     */
    void reset_reference_bits (void)
        { /* Do nothing */ }

    /*
     * @return memory attributes for page
     */
    memattrib_e memattrib (void)
        { return (memattrib_e) x.memory_attrib; }

    /**
     * @return physical address of mapping
     */
    addr_t phys_addr (void)
        { return (addr_t) (x.phys_addr << CONFIG_MIPS_VPN_SHIFT); }

    /**
     * @return raw contents of translation
     */
    u32_t get_raw (void)
        { return raw; }


    word_t get_valid (void)
        { return x.valid; }

    word_t get_dirty (void)
        { return x.dirty; }

    word_t get_global (void)
        { return x.global; }

    bool is_kernel (void)
        { return (x.global != 0); } /* 0 - user, 1 is kernel */

    /*
     * Modification
     */

    void set_dirty (word_t d)
        { x.dirty = d; }

    void set_valid (word_t v)
        { x.valid = v; }

    void set_global (word_t g)
        { x.global = g; }

    void set_attrib (memattrib_e memattrib)
        { x.memory_attrib = (word_t) memattrib; }

    void set_phys_addr(addr_t phys_addr)
        {
            x.phys_addr = (word_t) phys_addr >> CONFIG_MIPS_VPN_SHIFT;
        }

    void set (memattrib_e memattrib,
              bool allow_access,
              bool allow_modify,
              bool kernel,
              addr_t phys_addr);


    /*
     * Creation
     */

    /**
     * Create new translation with undefined contents.
     */
    translation_t (void) {}

    /**
     * Create new translation with the given raw contents.
     * @param x         raw contenents of new translation
     */
    translation_t (word_t x) { raw = x; }

    translation_t (memattrib_e memattrib,
                   bool allow_access,
                   bool allow_modify,
                   bool kernel,
                   addr_t phys_addr);

    /*
     * Translation cache access
     */

    void put_tc (addr_t vaddr, word_t page_size, word_t asid);
};


/**
 * Create new translation and set initialize it according to input
 * parameters.
 *
 * @param memattrib             memory attribute for mapping
 * @param allow_access          allow accesses on mapped page
 * @param allow_modify          allow modifications on mapped page
 * @param phys_addr             physical address
 */
INLINE
translation_t::translation_t (memattrib_e memattrib,
                              bool allow_access,
                              bool allow_modify,
                              bool kernel,
                              addr_t phys_addr)
{
    raw =                       0;
    x.global =                  kernel;
    x.valid =                   allow_access;
    x.dirty =                   allow_modify;
    x.memory_attrib =           (word_t) memattrib;
    x.phys_addr =               (word_t) phys_addr >> CONFIG_MIPS_VPN_SHIFT;
}


/**
 * Set translation according to input parameters.
 *
 * @param present               is page present or not
 * @param memattrib             memory attribute for mapping
 * @param allow_access          allow accesses on mapped page
 * @param allow_modify          allow modifications on mapped page
 * @param access_rights         access rights for mapping
 * @param phys_addr             physical address
 * @param defer_exceptions      shoule exception in page be deferred
 */
INLINE void
translation_t::set (memattrib_e memattrib,
                    bool allow_access,
                    bool allow_modify,
                    bool kernel,
                    addr_t phys_addr)
{
    x.global =                  kernel;
    x.valid =                   allow_access;
    x.dirty =                   allow_modify;
    x.memory_attrib =           (word_t) memattrib;
    x.phys_addr =               (word_t) phys_addr >> CONFIG_MIPS_VPN_SHIFT;
}


/**
 * Insert code and/or data translation into translation cache.
 *
 * @param vaddr         virtual address
 * @param page_size     page size (log2)
 * @param asid          address space ID
 */
INLINE void
translation_t::put_tc (addr_t vaddr, word_t page_size, word_t asid)
{
    word_t entry_hi, save;
    word_t entry_lo0, entry_lo1;
    word_t temp;
    bool match = false;
    bool superpage = false;

    ASSERT(DEBUG, asid < CONFIG_MAX_NUM_ASIDS);

    __asm__ __volatile__ (
        "mfc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : "=r" (save)
    );

    if (x.global) asid = 0;    /* Global? */

    /* XXX need to extend for VPNx2 - mips32 r2 */
    if (page_size > 12) {
        page_size--;
        superpage = true;
    }
    entry_hi = (asid) | (((word_t)vaddr >> (page_size+1)) << (page_size+1));

    __asm__ __volatile__ (
        "mtc0   %1,"STR(CP0_ENTRYHI)"\n\t"
        "nop;nop;nop;\n\t"
        "tlbp;\n\t"
        "nop;nop;nop;\n\t"
        "mfc0   %0,"STR(CP0_INDEX)"\n\t"
        : "=r" (temp) : "r" (entry_hi)
    );

    if (superpage)
    {
        if (!(temp & 0x80000000ul))    /* is top bit not set? */
            match = true;
        goto superpg;
    }


    if (temp & 0x80000000ul)    /* is top bit set? */
        goto none;

    __asm__ __volatile__ (
        "tlbr\n\t"
        "nop;nop;nop;\n\t"
    );
    __asm__ __volatile__ (
        "mtc0       %0,"STR(CP0_PAGEMASK)"\n\t"
        : : "r" ((0x2 << (12))-1)
    );

    if ((word_t)vaddr & (1<<page_size))     /* Odd entry */
    {
        __asm__ __volatile__ (
            "mtc0       %0,"STR(CP0_ENTRYLO1)"\n\t"
            : : "r" (raw)
        );
    } else  /* Even entry */
    {
        __asm__ __volatile__ (
            "mtc0       %0,"STR(CP0_ENTRYLO0)"\n\t"
            : : "r" (raw)
        );
    }
    __asm__ __volatile__ (
        "nop;nop;nop;\n\t"
        "tlbwi\n\t"
        "nop;nop;nop;\n\t"
    );
    goto exit;

none:
    __asm__ __volatile__ (
        "mtc0       %0,"STR(CP0_PAGEMASK)"\n\t"
        : : "r" ((0x2 << (12))-1)
    );

    if ((word_t)vaddr & (1<<page_size))     /* Odd entry */
    {
        entry_lo0 = raw & 1;    /* Set global bit (both must be global for kernel) */
        entry_lo1 = raw;
    } else  /* Even entry */
    {
        entry_lo0 = raw;
        entry_lo1 = raw & 1;    /* Set global bit (both must be global for kernel) */
    }
    __asm__ __volatile__ (
        "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        "mtc0   %1,"STR(CP0_ENTRYLO0)"\n\t"
        "mtc0   %2,"STR(CP0_ENTRYLO1)"\n\t"
        "nop;nop;nop;\n\t"
        "tlbwr\n\t"
        "nop;nop;nop;\n\t"
        : : "r" (entry_hi), "r" (entry_lo0), "r" (entry_lo1)
    );
    goto exit;

superpg:
    if (match) {
        __asm__ __volatile__ (
            "tlbr\n\t"
            "nop;nop;nop;\n\t"
        );
    }
    entry_lo0 = raw;
    entry_lo1 = raw | ((1ul << page_size) >> 12 << 6);  // set next phys page

    __asm__ __volatile__ (
        "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        "mtc0   %1,"STR(CP0_ENTRYLO0)"\n\t"
        "mtc0   %2,"STR(CP0_ENTRYLO1)"\n\t"
        "mtc0   %3,"STR(CP0_PAGEMASK)"\n\t"
        : : "r" (entry_hi), "r" (entry_lo0), "r" (entry_lo1),
            "r" ((0x2 << page_size)-1)
    );

    if (match) {
        __asm__ __volatile__ (
            "nop;nop;nop;\n\t"
            "tlbwi\n\t"
            "nop;nop;nop;\n\t"
        );
    } else {
        __asm__ __volatile__ (
            "nop;nop;nop;\n\t"
            "tlbwr\n\t"
            "nop;nop;nop;\n\t"
        );
    }

exit:
    __asm__ __volatile__ (
        "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : : "r" (save)
    );
}

/**
 * Purge all data and instruction translations in the translation
 * with the indicated virtual memory region.
 *
 * @param vaddr         virtual address to purge
 * @param page_size     size of region to purge (log2)
 * @param asid          address space ID of mapping to purge
 */
INLINE void purge_tc (addr_t vaddr, word_t page_size, word_t asid)
{
    word_t entry_hi, save;
    word_t temp;
    bool superpage = false;

    ASSERT(DEBUG, asid < CONFIG_MAX_NUM_ASIDS);

    __asm__ __volatile__ (
        "mfc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : "=r" (save)
    );

    /* XXX need to extend for VPNx2 - mips32 r2 */
    if (page_size > 12) {
        page_size--;
        superpage = true;
    }
    entry_hi = (asid) | (((word_t)vaddr >> (page_size+1)) << (page_size+1));

    __asm__ __volatile__ (
        "mtc0   %1,"STR(CP0_ENTRYHI)"\n\t"
        "nop;nop;nop;tlbp;nop;nop;nop;"
        "mfc0   %0,"STR(CP0_INDEX)"\n\t"
        : "=r" (temp) : "r" (entry_hi)
    );

    if (temp & 0x80000000ul)    /* is top bit set? */
        goto exit;

    __asm__ __volatile__ (
        "tlbr\n\t"
        "nop;nop;nop;\n\t"
    );

    if (superpage)
    {
        __asm__ __volatile__ (
            "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
            "mtc0   %1,"STR(CP0_ENTRYLO0)"\n\t"
            "mtc0   %2,"STR(CP0_ENTRYLO1)"\n\t"
            "nop;nop;nop;\n\t"
            "tlbwi\n\t"
            "nop;nop;nop;\n\t"
            : : "r" (entry_hi), "r" (0), "r" (0)
        );
        goto exit;
    }

    if ((word_t)vaddr & (1UL<<page_size))           /* Odd entry */
    {
        __asm__ __volatile__ (
            "mfc0       %0,"STR(CP0_ENTRYLO0)"\n\t"
            : "=r" (temp)
        );
        __asm__ __volatile__ (
            "mtc0       %0,"STR(CP0_ENTRYLO1)"\n\t"
            :: "r" (temp & 1)   // leave global bit incase kernel entry
        );
    } else  /* Even entry */
    {
        __asm__ __volatile__ (
            "mfc0       %0,"STR(CP0_ENTRYLO1)"\n\t"
            : "=r" (temp)
        );
        __asm__ __volatile__ (
            "mtc0       %0,"STR(CP0_ENTRYLO0)"\n\t"
            :: "r" (temp & 1)   // leave global bit incase kernel entry
        );
    }
    __asm__ __volatile__ (
        "nop;nop;nop;\n\t"
        "tlbwi\n\t"
        "nop;nop;nop;\n\t"
    );

exit:
    __asm__ __volatile__ (
        "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : : "r" (save)
    );
}

/**
 * Flush TLB and page cache for asid specified
 * 
 * @param asid          address space ID of space to purge
 */
INLINE void flush_asid (word_t asid)
{
    word_t i, temp, save;

    __asm__ __volatile__ (
        "mfc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : "=r" (save)
    );


    for (i=0; i < CONFIG_MIPS_TLB_SIZE; i++)
    {
        __asm__ __volatile__ (
            "mtc0 %[idx],"STR(CP0_INDEX)"       \n\t"
            "nop;nop;nop;                       \n\t"
            "tlbr                               \n\t"
            "nop;nop;nop;                       \n\t"
            "mfc0 %[temp],"STR(CP0_ENTRYHI)"    \n\t"
            : [temp] "+r" (temp)
            : [idx] "r" (i)
        );

        if ((temp & 0xff) == asid)
        {
            __asm__ __volatile__ (
                "mtc0 %[idx],"STR(CP0_ENTRYHI)" \n\t"
                "mtc0 $0,"STR(CP0_ENTRYLO0)"    \n\t"
                "mtc0 $0,"STR(CP0_ENTRYLO1)"    \n\t"
                "nop;nop;nop;                   \n\t"
                "tlbwi                          \n\t"
                "nop;nop;nop;                   \n\t"
                :
                : [idx] "r" (i)
            );
        }
    }

    __asm__ __volatile__ (
        "mtc0   %0,"STR(CP0_ENTRYHI)"\n\t"
        : : "r" (save)
    );
}

#endif /* !__ARCH__MIPS__TLB32_H__ */
