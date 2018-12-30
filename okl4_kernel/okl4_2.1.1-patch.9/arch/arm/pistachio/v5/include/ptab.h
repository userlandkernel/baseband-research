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
 * Copyright (c) 2004-2006, National ICT Australia
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
 * Description:   ARM page table structures
 */
#ifndef __ARCH__ARM__PTAB_H__
#define __ARCH__ARM__PTAB_H__

#include <kernel/cache.h>

enum memattrib_e {
    cached              = 3,    /* writeback */
    writeback           = 3,
    uncached            = 0,
    iomemory            = uncached, /* Alias on this architecture. */
    writecombine        = 2,
    writethrough        = CPU_WRITETHROUGH,
#if defined(CONFIG_DEFAULT_CACHE_ATTR_WB)
    l4default           = cached,
#else
    l4default           = writethrough, /* NB: this will affect performance */
#endif
};

INLINE memattrib_e to_memattrib(l4attrib_e x)
{
    switch (x) {
        case l4mem_default:
            return l4default;
        case l4mem_cached:
        case l4mem_writeback:
        case l4mem_coherent:
            return cached;
        case l4mem_writethrough:
            return writethrough;
        case l4mem_io_combined:
            return writecombine;
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
        case writecombine:
            return l4mem_io_combined;
#if CPU_WRITETHROUGH == 1
        case writethrough:
            return l4mem_writethrough;
#endif
        default:
            return l4mem_uncached;
    }
}

class arm_l2_desc_t
{
public:
    enum ap_e {
        special = 0,
        none    = 1,
        ro      = 2,
        rw      = 3,
    };

    union {
        struct {
            BITFIELD2(word_t,
                zero            : 2,
                ign             : 30
                      );
        } fault;

        struct {
            BITFIELD5(word_t,
                one             : 2,
                attrib          : 2,
                ap              : 8,
                sbz             : 4,
                base_address    : 16
                      );
        } large;

        struct {
            BITFIELD4(word_t,
                two             : 2,
                attrib          : 2,
                ap              : 8,
                base_address    : 20
                      );
        } small;

        struct {
            BITFIELD5(word_t,
                three           : 2,
                attrib          : 2,
                ap              : 2,
                sbz             : 4,
                base_address    : 22
                      );
        } tiny;

        u32_t raw;
    };

    // Predicates
    bool is_valid (void)
        { return fault.zero != 0; }

    memattrib_e attributes (void)
        { return (memattrib_e)large.attrib; }

    void clear()
        { raw = 0; }

    addr_t address_large()
        { return (addr_t)(large.base_address << 16); }

    addr_t address_small()
        { return (addr_t)(small.base_address << 12); }

    addr_t address_tiny()
        { return (addr_t)(tiny.base_address << 10); }
};


class arm_l1_desc_t
{
public:
    enum ap_e {
        special = 0,
        none    = 1,
        ro      = 2,
        rw      = 3,
    };

    union {
        struct {
            BITFIELD2(word_t,
                zero            : 2,
                ign             : 30
                      );
        } fault;

        struct {
            BITFIELD5(word_t,
                one             : 2,
                imp             : 3,
                domain          : 4,
                sbz             : 1,
                base_address    : 22
                      );
        } coarse_table;

        struct {
            BITFIELD8(word_t,
                two             : 2,
                attrib          : 2,
                imp             : 1,
                domain          : 4,
                sbz1            : 1,
                ap              : 2,
                sbz2            : 8,
                base_address    : 12
                      );
        } section;

        struct {
            BITFIELD5(word_t,
                three           : 2,
                imp             : 3,
                domain          : 4,
                sbz             : 3,
                base_address    : 20
                      );
        } fine_table;

        u32_t raw;
    };

    // Predicates

    bool is_valid (void)
        { return fault.zero != 0; }

    memattrib_e attributes (void)
        { return (memattrib_e)section.attrib; }

    void clear()
        { raw = 0; }

    addr_t address_section()
        { return (addr_t)(section.base_address << 20); }

    word_t address_finetable()
        { return (fine_table.base_address << 12); }

    word_t address_coarsetable()
        { return (coarse_table.base_address << 10); }
};

#define ARM_L0_BITS             ((32 -ARM_TOP_LEVEL_BITS) + 2)
#define ARM_L1_BITS             (ARM_SECTION_BITS - (32 - ARM_TOP_LEVEL_BITS) + 2)
#define ARM_L2_NORMAL_BITS      (32 - ARM_SECTION_BITS - PAGE_BITS_4K + 2)
#define ARM_L2_TINY_BITS        (32 - ARM_SECTION_BITS - PAGE_BITS_1K + 2)
#define ARM_HWL1_BITS           (ARM_SECTION_BITS + 2)

#define ARM_L0_ENTRIES          ( 1 << (ARM_L0_BITS - 2))
#define ARM_L1_ENTRIES          ( 1 << (ARM_L1_BITS - 2))
#define ARM_L2_ENTRIES_NORMAL   ( 1 << (ARM_L2_NORMAL_BITS - 2))
#define ARM_L2_ENTRIES_TINY     ( 1 << (ARM_L2_TINY_BITS - 2))
#define ARM_HWL1_ENTRIES                ( 1 << (ARM_HWL1_BITS - 2))

#define ARM_L0_SIZE             (1 << ARM_L0_BITS)
#define ARM_L1_SIZE             (1 << ARM_L1_BITS)
#define ARM_L2_SIZE_NORMAL      (1 << ARM_L2_NORMAL_BITS)
#define ARM_L2_SIZE_TINY        (1 << ARM_L2_TINY_BITS)
#define ARM_HWL1_SIZE           (1 << ARM_HWL1_BITS)

#endif /* !__ARCH__ARM__PTAB_H__ */
