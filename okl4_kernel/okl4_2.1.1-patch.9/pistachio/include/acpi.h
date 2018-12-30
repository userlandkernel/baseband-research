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
 * Description:   ACPI structures
 */
#ifndef __GENERIC__ACPI_H__
#define __GENERIC__ACPI_H__

#include <arch/hwspace.h>

#define ACPI_MEM_SPACE  0
#define ACPI_IO_SPACE   1

class acpi_gas_t {
public:
    u8_t        id;
    u8_t        width;
    u8_t        offset;
private:
    u8_t        _rsvd_3;
    /* the 64-bit address is only 32-bit aligned */
    BITFIELD2 (u32_t,
               addrlo,
               addrhi);
public:
    u64_t address (void) { return (((u64_t) addrhi) << 32) + addrlo; }
} __attribute__((packed));


class acpi_thead_t {
public:
    char        sig[4];
    u32_t       len;
    u8_t        rev;
    u8_t        csum;
    char        oem_id[6];
    char        oem_tid[8];
    u32_t       oem_rev;
    u32_t       creator_id;
    u32_t       creator_rev;
} __attribute__((packed));





class acpi_madt_hdr_t {
public:
    u8_t        type;
    u8_t        len;
} __attribute__((packed));

class acpi_madt_lapic_t {
    acpi_madt_hdr_t     header;
public:
    u8_t                apic_processor_id;
    u8_t                id;
    struct {
        u32_t enabled   :  1;
        u32_t           : 31;
    } flags;
} __attribute__((packed));

class acpi_madt_ioapic_t {
    acpi_madt_hdr_t     header;
public:
    u8_t        id;       /* APIC id                    */
private:
    u8_t        _rsvd_3;
public:
    u32_t       address;  /* physical address           */
    u32_t       irq_base; /* global irq number base     */
} __attribute__((packed));


class acpi_madt_lsapic_t {
    acpi_madt_hdr_t     header;
public:
    u8_t                apic_processor_id;
    u8_t                id;
    u8_t                eid;
private:
    u8_t                __reserved[3];
public:
    struct {
        u32_t enabled   :  1;
        u32_t           : 31;
    } flags;
} __attribute__((packed));


class acpi_madt_iosapic_t {
    acpi_madt_hdr_t     header;
public:
    u8_t        id;       /* APIC id                    */
private:
    u8_t        __reserved;
public:
    u32_t       irq_base; /* global irq number base     */
    u64_t       address;  /* physical address           */
} __attribute__((packed));


class acpi_madt_irq_t {
    acpi_madt_hdr_t     header;
public:
    u8_t        src_bus;        /* source bus, fixed 0=ISA      */
    u8_t        src_irq;        /* source bus irq               */
    u32_t       dest;           /* global irq number            */
    union {
        u16_t   flags;          /* irq flags */
        struct {
            BITFIELD3 (
                u16_t,
                polarity        : 2,
                trigger_mode    : 2,
                reserved        : 12);
        } x;
    };

public:
    enum polarity_t {
        conform_polarity = 0,
        active_high = 1,
        reserved_polarity = 2,
        active_low = 3
    };

    enum trigger_mode_t {
        conform_trigger = 0,
        edge = 1,
        reserved_trigger = 2,
        level = 3
    };

    polarity_t get_polarity()
        { return (polarity_t) x.polarity; }
    trigger_mode_t get_trigger_mode()
        { return (trigger_mode_t) x.trigger_mode; }

} __attribute__((packed));


class acpi_madt_nmi_t
{
    acpi_madt_hdr_t     header;
public:
    union {
        u16_t           flags;
        struct {
            BITFIELD3 (
                u16_t,
                polarity        : 2,
                trigger_mode    : 2,
                reserved        : 12);
        } x;
    };
    u32_t               irq;

    enum polarity_t {
        conform_polarity        = 0,
        active_high             = 1,
        active_low              = 3
    };

    enum trigger_mode_t {
        conform_trigger         = 0,
        edge                    = 1,
        level                   = 3
    };

    polarity_t get_polarity (void)
        { return (polarity_t) x.polarity; }

    trigger_mode_t get_trigger_mode (void)
        { return (trigger_mode_t) x.trigger_mode; }
};


class acpi_madt_t {
    acpi_thead_t header;
public:
    u32_t       local_apic_addr;
    u32_t       apic_flags;
private:
    u8_t        data[0];
public:
    acpi_madt_hdr_t* find(u8_t type, int index);
public:
    acpi_madt_lapic_t* lapic(int index);
    acpi_madt_ioapic_t* ioapic(int index);
    acpi_madt_lsapic_t* lsapic(int index);
    acpi_madt_iosapic_t* iosapic(int index);
    acpi_madt_irq_t* irq(int index);
    acpi_madt_nmi_t* nmi(int index);

    friend void dump_apic (acpi_madt_t * madt);
} __attribute__((packed));


/* xxx */
extern bool mapit( addr_t addr );

/*
  RSDT and XSDT differ in their pointer size only
  rsdt: 32bit, xsdt: 64bit
*/
template<typename T> class acpi__sdt_t {
    acpi_thead_t        header;
    T                   ptrs[0];
public:
    /* find table with a given signature */
    acpi_thead_t* find(const char sig[4]) {
        for (word_t i = 0; i < ((header.len-sizeof(header))/sizeof(ptrs[0])); i++)
        {
            acpi_thead_t* t= (acpi_thead_t*)((word_t)ptrs[i]);
            mapit(t);
            if (t->sig[0] == sig[0] &&
                t->sig[1] == sig[1] &&
                t->sig[2] == sig[2] &&
                t->sig[3] == sig[3])
                return t;
        };
        return NULL;
    };
#if 1
    void list() {
        for (word_t i = 0; i < ((header.len-sizeof(header))/sizeof(ptrs[0])); i++)
        {
            acpi_thead_t* t= (acpi_thead_t*)((word_t)ptrs[i]);
            printf("%c%c%c%c @ %p\n",
                   t->sig[0], t->sig[1], t->sig[2], t->sig[3], t);
        };
    };
#endif

    friend class kdb_t;
} __attribute__((packed));

typedef acpi__sdt_t<u32_t> acpi_rsdt_t;
typedef acpi__sdt_t<u64_t> acpi_xsdt_t;

class acpi_rsdp_t {
    char        sig[8];
    u8_t        csum;
    char        oemid[6];
    u8_t        rev;
    u32_t       rsdt_ptr;
    u32_t       rsdt_len;
    u64_t       xsdt_ptr;
    u8_t        xcsum;
    u8_t        _rsvd_33[3];
private:
public:
    static acpi_rsdp_t* locate();

    acpi_rsdt_t* rsdt() {
        /* verify checksum */
        u8_t csum = 0;
        for (int i = 0; i < 20; i++)
            csum += ((char*)this)[i];
        if (csum != 0)
            return NULL;
        return (acpi_rsdt_t*) (word_t)rsdt_ptr;
    };
    acpi_xsdt_t* xsdt() {
        /* check version - only ACPI 2.0 knows about an XSDT */
        if (rev != 2)
            return NULL;
        /* verify checksum
           hopefully it's wrong if there's no xsdt pointer*/
        u8_t csum = 0;
        for (int i = 0; i < 36; i++)
            csum += ((char*)this)[i];
        if (csum != 0)
            return NULL;
        return (acpi_xsdt_t*) (word_t)xsdt_ptr;
    };

    friend class kdb_t;
} __attribute__((packed));


#endif /* !__GENERIC__ACPI_H__ */
