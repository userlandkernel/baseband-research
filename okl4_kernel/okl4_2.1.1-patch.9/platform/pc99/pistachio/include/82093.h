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
 * Description:   Driver for IO-APIC 82093
 */
#ifndef __PLATFORM__PC99__82093_H__
#define __PLATFORM__PC99__82093_H__

// the 82093 supports 24 IRQ lines
#define I82093_NUM_IRQS         24

typedef union {
    struct {
        u32_t version   : 8;
        u32_t           : 8;
        u32_t max_lvt   : 8;
        u32_t reserved0 : 8;
    } __attribute__((packed)) ver;
    u32_t raw;
} ioapic_version_t;

class ioapic_redir_t {
public:
    word_t vector                       :  8;
    word_t delivery_mode                :  3;
    word_t dest_mode                    :  1;
    word_t delivery_status              :  1;
    word_t polarity                     :  1;
    word_t irr                          :  1;
    word_t trigger_mode                 :  1;
    word_t mask                         :  1;
    word_t                              : 15;
    union {
        struct {
            word_t                      : 24;
            word_t physical_dest        :  4;
            word_t                      :  4;
        } __attribute__((packed)) physical;

        struct {
            word_t                      : 24;
            word_t logical_dest         :  8;
        } __attribute__((packed)) logical;
    } dest;

public:
    void set_fixed_hwirq(u32_t vector, bool low_active,
                         bool level_triggered, bool masked,
                         u32_t apicid)
        {
            this->vector = vector;
            this->delivery_mode = 0;    // fixed
            this->dest_mode = 0;        // physical mode
            this->polarity = low_active ? 1 : 0;
            this->trigger_mode = level_triggered ? 1 : 0;
            this->mask = masked ? 1 : 0;
            this->dest.physical.physical_dest = apicid;
        }
    void set_phys_dest(u32_t apicid)
        { this->dest.physical.physical_dest = apicid; }
    u32_t get_phys_dest()
        { return this->dest.physical.physical_dest; }

    void mask_irq()     { mask = 1; }
    void unmask_irq()   { mask = 0; }
    bool is_masked_irq()
        {return (mask == 1); }
    bool is_edge_triggered()
        { return this->trigger_mode == 0; }
} __attribute__((packed));


class i82093_t {
    /* IOAPIC register ids */
    enum regno_t {
        IOAPIC_ID       =0x00,
        IOAPIC_VER      =0x01,
        IOAPIC_ARBID    =0x02,
        IOAPIC_REDIR0   =0x10
    };


private:
    u32_t get(u32_t reg)
        {
            *(__volatile__ u32_t*) this = reg;
            return *(__volatile__ u32_t*)(((word_t) this) + 0x10);

        }

    void set(u32_t reg, u32_t val)
        {
            *(__volatile__ u32_t*) this = reg;
            *(__volatile__ u32_t*)(((word_t) this) + 0x10) = val;
        }

    u32_t reread()
        {
            return *(__volatile__ u32_t*)(((word_t) this) + 0x10);
        }

public:
    u8_t id() { return get(IOAPIC_ID) >> 24; };
    ioapic_version_t version() {
        return (ioapic_version_t) { raw : get(IOAPIC_VER) };
    }
    /* VU: masking an IRQ on the IOAPIC only becomes active after
     * performing a read on the data register.  Therefore, when an IRQ
     * is masked we always perform the read assuming that masking/
     * unmasking is the operation performed most frequently */
    void set_redir_entry(word_t idx, ioapic_redir_t redir)
        {
            ASSERT(ALWAYS, idx < 24);
            set(0x11 + (idx * 2), *((u32_t*)(&redir) + 1));
            set(0x10 + (idx * 2), *((u32_t*)(&redir) + 0));
            if (redir.mask) reread();
        }
    void set_redir_entry_low(word_t idx, ioapic_redir_t redir)
        {
            ASSERT(ALWAYS, idx < 24);
            set(0x10 + (idx * 2), *(u32_t*)&redir);
            if (redir.mask) reread();
        }

    ioapic_redir_t get_redir_entry(word_t idx)
        {
            ASSERT(ALWAYS, idx < 24);
            ioapic_redir_t redir;
            (*((u32_t*)(&redir) + 1)) = get(0x11 + (idx * 2));
            (*((u32_t*)(&redir) + 0)) = get(0x10 + (idx * 2));
            return redir;
        }
};

#endif /* !__PLATFORM__PC99__82093_H__ */
