/*
 * Copyright (c) 2006, National ICT Australia
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
 * Description:   ARMv6/v7 page table structures
 * Author: Carl van Schaik, June 2006
 */
#ifndef __ARCH__ARMV6__PGTABLE_H__
#define __ARCH__ARMV6__PGTABLE_H__

#include <kernel/cache.h>

#if !defined(CONFIG_TEX_REMAPPING)

/**
 * Encoding format of memattrib_e:
 *  B   - bit 0
 *  C   - bit 1
 *  TEX - bit 4..2
 *        bit 6..5 (zero - reserved)
 *  S   - bit 7
 *  NS  - bit 8 (non-secure - trustzone only)
 */

enum memattrib_e {
    strong              = 0,    /* MP cohearent,    strongly-ordered    */
    writethrough        = 2,    /* Non cohearent,   no write allocate   */
    writeback           = 3,    /* Non cohearent,   no write allocate   */
    uncached            = 4,    /* Non cohearent */
    iomemory            = 8,    /* Non cohearent */

    writethrough_MP     = 0x82, /* MP cohearent,    no write allocate   */
    writeback_MP        = 0x83, /* MP cohearent,    no write allocate   */
    uncached_MP         = 0x84, /* MP cohearent */
    iomemory_MP         = 1,    /* MP cohearent */

    NC_NC               = 0x10, /* Non cohearent,   Internal NC,    External NC     */
    NC_WT               = 0x18, /* Non cohearent,   Internal NC,    External WT     */
    NC_WB               = 0x1c, /* Non cohearent,   Internal NC,    External WB     */
    NC_WBa              = 0x14, /* Non cohearent,   Internal NC,    External WBa    */
    WT_NC               = 0x12, /* Non cohearent,   Internal WT,    External NC     */
    WT_WT               = 0x1a, /* Non cohearent,   Internal WT,    External WT     */
    WT_WB               = 0x1e, /* Non cohearent,   Internal WT,    External WB     */
    WT_WBa              = 0x16, /* Non cohearent,   Internal WT,    External WBa    */
    WB_NC               = 0x13, /* Non cohearent,   Internal WB,    External NC     */
    WB_WT               = 0x1b, /* Non cohearent,   Internal WB,    External WT     */
    WB_WB               = 0x1f, /* Non cohearent,   Internal WB,    External WB     */
    WB_WBa              = 0x17, /* Non cohearent,   Internal WB,    External WBa    */
    WBa_NC              = 0x11, /* Non cohearent,   Internal WBa,   External NC     */
    WBa_WT              = 0x19, /* Non cohearent,   Internal WBa,   External WT     */
    WBa_WB              = 0x1d, /* Non cohearent,   Internal WBa,   External WB     */
    WBa_WBa             = 0x15, /* Non cohearent,   Internal WBa,   External WBa    */

    NC_NC_MP            = 0x90, /* MP cohearent,    Internal NC,    External NC     */
    NC_WT_MP            = 0x98, /* MP cohearent,    Internal NC,    External WT     */
    NC_WB_MP            = 0x9c, /* MP cohearent,    Internal NC,    External WB     */
    NC_WBa_MP           = 0x94, /* MP cohearent,    Internal NC,    External WBa    */
    WT_NC_MP            = 0x92, /* MP cohearent,    Internal WT,    External NC     */
    WT_WT_MP            = 0x9a, /* MP cohearent,    Internal WT,    External WT     */
    WT_WB_MP            = 0x9e, /* MP cohearent,    Internal WT,    External WB     */
    WT_WBa_MP           = 0x96, /* MP cohearent,    Internal WT,    External WBa    */
    WB_NC_MP            = 0x93, /* MP cohearent,    Internal WB,    External NC     */
    WB_WT_MP            = 0x9b, /* MP cohearent,    Internal WB,    External WT     */
    WB_WB_MP            = 0x9f, /* MP cohearent,    Internal WB,    External WB     */
    WB_WBa_MP           = 0x97, /* MP cohearent,    Internal WB,    External WBa    */
    WBa_NC_MP           = 0x91, /* MP cohearent,    Internal WBa,   External NC     */
    WBa_WT_MP           = 0x99, /* MP cohearent,    Internal WBa,   External WT     */
    WBa_WB_MP           = 0x9d, /* MP cohearent,    Internal WBa,   External WB     */
    WBa_WBa_MP          = 0x95, /* MP cohearent,    Internal WBa,   External WBa    */

#if defined(CONFIG_SMP)
 #if defined(CONFIG_DEFAULT_CACHE_ATTR_WB)
    l4default           = writeback_MP,
 #else
    l4default           = writethrough_MP,      /* NB: this will affect performance */
 #endif
#else
 #if defined(CONFIG_DEFAULT_CACHE_ATTR_WB)
    l4default           = writeback,
 #else
    l4default           = writethrough, /* NB: this will affect performance */
 #endif
#endif

};

#else /* CONFIG_TEX_REMAPPING */
#error TEX remapping not implemented
#endif

INLINE memattrib_e to_memattrib(l4attrib_e x)
{
    switch (x) {
        case l4mem_default:
        case l4mem_cached:
            return l4default;
        case l4mem_writeback:
            return writeback;
        case l4mem_coherent:
#if defined(CONFIG_DEFAULT_CACHE_ATTR_WB)
            return writeback_MP;
#else
            return writethrough_MP;
#endif
        case l4mem_writethrough:
            return writethrough;
        case l4mem_io_combined:
            return iomemory;
        case l4mem_io:
        case l4mem_uncached:
            return uncached;
        default:
            UNIMPLEMENTED();
            return uncached;
    }
}

INLINE l4attrib_e to_l4attrib(memattrib_e x)
{
    /* XXX fixme */
    switch (x) {
        case writethrough:
            return l4mem_writethrough;
        case writeback:
            return l4mem_writeback;
        case uncached:
            return l4mem_uncached;
        case writeback_MP:
            return l4mem_coherent;
        case iomemory:
            return l4mem_io_combined;
        default:
            return l4mem_uncached;
    }
}

/* ARM page attributes */
enum arm_ap_e {
    ap_na = 1,  /* user no access / apx: 0 - kernel rw, 1 - kernel ro */
    ap_ro = 2,  /* user read only / apx = 0 */
    ap_rw = 3,  /* user readwrite / apx = 0 */
};

class armv6_l2_desc_t
{
public:
    union {
        struct {
            BITFIELD2(word_t,
                zero            : 2,
                ign             : 30
            );
        } fault;

        struct {
            BITFIELD10(word_t,
                one             : 2,
                BC              : 2,
                ap              : 2,
                sbz             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                tex             : 3,
                nx              : 1,
                base_address    : 16
            );
        } large;

        struct {
            BITFIELD9(word_t,
                nx              : 1,
                one             : 1,
                BC              : 2,
                ap              : 2,
                tex             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                base_address    : 20
            );
        } small;

        u32_t raw;
    };

    // Predicates
//    bool is_valid (void)
//      { return fault.zero != 0; }

    memattrib_e attributes (void)
        {
            switch (fault.zero) {
                case 0:
                    return (memattrib_e)0;
                case 1: /* large */
                    return (memattrib_e)(large.BC | (large.tex << 2) | (large.shared << 7));
                default: /* small (2 || 3) */
                    return (memattrib_e)(small.BC | (small.tex << 2) | (small.shared << 7));
            }
        }

    void clear()
        { raw = 0; }

    addr_t address_large()
        { return (addr_t)(large.base_address << 16); }

    addr_t address_small()
        { return (addr_t)(small.base_address << 12); }
};


class armv6_l1_desc_t
{
public:
    union {
        struct {
            BITFIELD2(word_t,
                zero            : 2,
                ign             : 30
            );
        } fault;

        struct {
#if defined(CONFIG_TRUSTZONE)
            BITFIELD7(word_t,
                one             : 2,
                sbz1            : 1,
                ns              : 1,    /* Non-secure bit */
                sbz3            : 1,
                domain          : 4,
                ecc             : 1,
                base_address    : 22
            );
#else
            BITFIELD7(word_t,
                one             : 2,
                sbz1            : 1,
                sbz2            : 1,
                sbz3            : 1,
                domain          : 4,
                ecc             : 1,
                base_address    : 22
            );
#endif
        } coarse_table;

        struct {
#if defined(CONFIG_TRUSTZONE)
            BITFIELD13(word_t,
                two             : 2,
                BC              : 2,
                nx              : 1,
                domain          : 4,
                ecc             : 1,
                ap              : 2,
                tex             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                zero            : 1,
                ns              : 1,    /* Non-secure bit */
                base_address    : 12
            );
#else
            BITFIELD13(word_t,
                two             : 2,
                BC              : 2,
                nx              : 1,
                domain          : 4,
                ecc             : 1,
                ap              : 2,
                tex             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                zero            : 1,
                sbz             : 1,
                base_address    : 12
            );
#endif
        } section;

        struct {
#if defined(CONFIG_TRUSTZONE)
            BITFIELD14(word_t,
                two             : 2,
                BC              : 2,
                nx              : 1,
                ignored         : 4,
                ecc             : 1,
                ap              : 2,
                tex             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                one             : 1,
                ns              : 1,    /* Non-secure bit */
                sbz             : 4, /* XXX Xscale3 uses this for 36-bit extension */
                base_address    : 8
            );
#else
            BITFIELD14(word_t,
                two             : 2,
                BC              : 2,
                nx              : 1,
                ignored         : 4,
                ecc             : 1,
                ap              : 2,
                tex             : 3,
                apx             : 1,
                shared          : 1,
                nglobal         : 1,
                one             : 1,
                unused          : 1,
                sbz             : 4, /* XXX Xscale3 uses this for 36-bit extension */
                base_address    : 8
            );
#endif
        } super_section;

        struct {
            BITFIELD2(word_t,
                three           : 2,
                res             : 30
            );
        } reserved;

        u32_t raw;
    };

    // Predicates

//    bool is_valid (void)    /* valid if bottom 2 bits not 0 or 3 */
//      { return (fault.zero == 1) || (fault.zero == 2); }

    memattrib_e attributes (void)
        {   /* attrib bits are the same for section + supersection */
            return (memattrib_e)(section.BC |
                    (section.tex << 2) | (section.shared << 7));
        }

    void clear()
        { raw = 0; }

    addr_t address_section()
        { return (addr_t)(section.base_address << 20); }

    addr_t address_supersection()
        { return (addr_t)(super_section.base_address << 24); }

    word_t address_coarsetable()
        { return (coarse_table.base_address << 10); }
};

#define ARM_HWL1_BITS   (ARM_SECTION_BITS+2)
#define ARM_HWL1_SIZE   (1UL << ARM_L1_BITS)

#define ARM_L1_BITS     (ARM_SECTION_BITS+2)
#define ARM_L1_SIZE     (1UL << ARM_L1_BITS)

#define ARM_L2_BITS     (32-ARM_SECTION_BITS-PAGE_BITS_4K+2)
#define ARM_L2_SIZE     (1UL << ARM_L2_BITS)

#endif /* !__ARCH__ARMV6__PGTABLE_H__ */
