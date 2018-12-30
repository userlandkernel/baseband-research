/*
 * Copyright (c) 2004, National ICT Australia (NICTA)
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
 * Description:   Functions which manipulate the OMAP1510 interrupt controller
 */

#ifndef __PLATFORM__INNOVATOR__INTCTRL_H_
#define __PLATFORM__INNOVATOR__INTCTRL_H_

#include <intctrl.h>
#include <arch/hwspace.h>
#include <arch/thread.h>
#include <space.h>
#include INC_CPU(io.h)

#define IRQS        64

#define REG_ARM_IRQHDL1_BASE    0xFFFECB00
#define REG_ARM_IRQHDL2_BASE    0xFFFE0000
#define IRQHDL_ITR              0x0
#define IRQHDL_MIR              0x4
#define IRQHDL_IRQ_CODE         0x10
#define IRQHDL_FIQ_CODE         0x14
#define IRQHDL_CTL_REG          0x18
#define ILR_BASE                0x1C


#define REG_IRQHDL1_MIR *((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL1_BASE)) + IRQHDL_MIR))
#define REG_IRQHDL2_MIR *((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL2_BASE)) + IRQHDL_MIR))
#define REG_IRQHDL2_CTL *((volatile word_t*)((io_to_virt(REG_ARM_IRQHDL2_BASE)) + IRQHDL_CTL_REG))

#define REG_IRQHDL_IRQ_CODE(base)   *((volatile word_t*) ((base) + IRQHDL_IRQ_CODE))
#define REG_IRQHDL_ITR(base)    *((volatile word_t*) ((base) + IRQHDL_ITR))
#define REG_IRQHDL_CTL(base)    *((volatile word_t*) ((base) + IRQHDL_CTL_REG))

extern word_t arm_high_vector;
extern word_t interrupt_handlers[IRQS];

class intctrl_t : public generic_intctrl_t {

public:
    void init_arch();
    void init_cpu();

    word_t get_number_irqs(void)
    {
        return IRQS;
    }

    void register_interrupt_handler (word_t vector, void (*handler)(word_t,
            arm_irq_context_t *))
    {
        ASSERT(DEBUG, vector >= 0 && vector < IRQS);
        interrupt_handlers[vector] = (word_t) handler;
        TRACE_INIT("interrupt vector[%d] = %p\n", vector,
                interrupt_handlers[vector]);
    }

    static inline void mask(word_t irq)
    {
        ASSERT(DEBUG, irq >=0 && irq < IRQS);

        if (irq > 31)
            REG_IRQHDL2_MIR |= (1 << (irq - 32));
        else
            REG_IRQHDL1_MIR |= (1 << irq);

    }

    static inline bool unmask(word_t irq)
    {
        ASSERT(DEBUG, irq < IRQS);

        if (irq > 31)
            REG_IRQHDL2_MIR &= ~(1<<(irq - 32));
        else
            REG_IRQHDL1_MIR &= ~(1<<irq);

        return false;
    }

    static inline void disable(word_t irq)
    {
        mask(irq);
    }

    static inline bool enable(word_t irq)
    {
        return unmask(irq);
    }

    void disable_fiq(void)
    {
        /* No fiq control on interrupt handler,
         * this should be done by setting CP15.
         */
    }

    bool is_irq_available(int irq)
    {
        /* irq0 is used by indicating 2nd level irq. */
        return irq >= 0 && irq < IRQS;
    }

    void set_cpu(word_t irq, word_t cpu) {}
};

#endif /* __PLATFORM__INNOVATOR__INTCTRL_H_ */
