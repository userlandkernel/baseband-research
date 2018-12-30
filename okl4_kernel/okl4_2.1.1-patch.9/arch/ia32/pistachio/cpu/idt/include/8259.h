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
 * Description:   Driver for i8259 Programmable Interrupt Controller
 */
#ifndef __ARCH__IA32__IDT__8259_H__
#define __ARCH__IA32__IDT__8259_H__

#include <arch/ioport.h>                /* for in_u8/out_u8     */

/**
 * Driver for i8259 PIC
 * @param base  the base address of the registers
 *
 * The template parameter BASE enables compile-time resolution of the
 * PIC's control register addresses.
 *
 * Note:
 *   Depending on whether I8259_CACHE_PICSTATE is defined or not
 *   objects will cache the mask register or not. Thus it is not wise
 *   to blindly instanciate them all over the place because the cached
 *   state would not be shared. Making the cached state static
 *   wouldn't work either because there are two PICs in a
 *   PC99. Intended use is a single object per PIC.
 *
 * Assumptions:
 * - BASE can be passed as port to in_u8/out_u8
 * - The PIC's A0=0 register is located at BASE
 * - The PIC's A0=1 register is located at BASE+1
 * - PICs in unbuffered cascade mode
 *
 * Uses:
 * - out_u8, in_u8
 */

// Enable PIC state caching
#define I8259_CACHE_PICSTATE

template<u16_t base> class i8259_pic_t {
 private:

#if defined(I8259_CACHE_PICSTATE)
    u8_t mask_cache;
#endif
 public:

    /**
     *  Unmask interrupt
     *  @param irq      interrupt line to unmask
     */
    void unmask(word_t irq)
        {
#if !defined(I8259_CACHE_PICSTATE)
            u8_t mask_cache = in_u8(base+1);
#endif
            mask_cache &= ~(1 << (irq));
            out_u8(base+1, mask_cache);
        }

    /**
     *  Mask interrupt
     *  @param irq      interrupt line to mask
     */
    void mask(word_t irq)
        {
#if !defined(I8259_CACHE_PICSTATE)
            u8_t mask_cache = in_u8(base+1);
#endif
            mask_cache |= (1 << (irq));
            out_u8(base+1, mask_cache);
        }

    /**
     *  Send specific EOI
     *  @param irq      interrupt line to ack
     */
    void ack(word_t irq)
        {
            out_u8(base, 0x60 + irq);
        }

    /**
     *  Check if interrupt is masked
     *  @param irq      interrupt line
     *
     *  @return         true when masked, false otherwise
     */
    bool is_masked(word_t irq)
        {
#if !defined(I8259_CACHE_PICSTATE)
            u8_t mask_cache = in_u8(base+1);
#endif
            return (mask_cache & (1 << irq));
        }

    /**
     *  initialize PIC
     *  @param vector_base      8086-style vector number base
     *  @param slave_info       slave mask for master or slave id for slave
     *
     *  Initializes the PIC in 8086-mode:
     *  - not special-fully-nested mode
     *  - reporting vectors VECTOR_BASE...VECTOR_BASE+7
     *  - all inputs masked
     */
    void init(u8_t vector_base, u8_t slave_info)
        {
#if !defined(I8259_CACHE_PICSTATE)
            u8_t
#endif
            mask_cache = 0xFF;
            /*
              ICW1:
                0x10 | NEED_ICW4 | CASCADE_MODE | EDGE_TRIGGERED
            */
            out_u8(base, 0x11);

            /*
              ICW2:
              - 8086 mode irq vector base
                PIN0->IRQ(base), ..., PIN7->IRQ(base+7)
            */
            out_u8(base+1, vector_base);

            /*
              ICW3:
               - master: slave list
                 Set bits mark input PIN as connected to a slave
               - slave: slave id
                 This PIC is connected to the master's pin SLAVE_ID
               Note: The caller knows whether its a master or not -
                     the handling is the same.
            */
            out_u8(base+1, slave_info);

            /*
              ICW4:
                8086_MODE | NORMAL_EOI | NONBUFFERED_MODE | NOT_SFN_MODE
             */
            out_u8(base+1, 0x01); /* mode - *NOT* fully nested */

            /*
              OCW1:
               - set initial mask
            */
            out_u8(base+1, mask_cache);
        }
};

#endif /* !__ARCH__IA32__IDT__8259_H__ */
