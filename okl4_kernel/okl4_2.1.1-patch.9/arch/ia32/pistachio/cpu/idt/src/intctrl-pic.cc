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
 * Description:   Implementation of class handling the PIC cascade in
 */

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <arch/platform.h>
#include <platform_support.h>
#include <interrupt.h>
#include <arch/interrupt.h>
#include <arch/hwirq.h>
#include <arch/idt.h>
#include <arch/trapgate.h>

extern stack_t __stack;

/* Low level IRQ handler entry points */
HW_IRQ( 0)
HW_IRQ( 1)
HW_IRQ( 2)
HW_IRQ( 3)
HW_IRQ( 4)
HW_IRQ( 5)
HW_IRQ( 6)
HW_IRQ( 7)
HW_IRQ( 8)
HW_IRQ( 9)
HW_IRQ(10)
HW_IRQ(11)
HW_IRQ(12)
HW_IRQ(13)
HW_IRQ(14)
HW_IRQ(15)

/* have this asm _after_ the entry points to get forward jumps */
void hw_irq_common_wrapper()
{
    HW_IRQ_COMMON()
}

/* 8259 specific controllers */
i8259_pic_t<0x20> master;
i8259_pic_t<0xa0> slave;

/*
 * Interrupt mapping table for IA32
 */
irq_mapping_t irq_mapping[IRQS];
irq_owner_t   irq_owners[IRQS];
bitmap_t      irq_pending[BITMAP_SIZE(IRQS)];

/*
 * Initialize PIC
 */
void ia32_init_intctrl()
{
    const int base = 0x20;
    /* setup the IDT */
#if 0
    for (int i=0; i<16; i++)
        idt.add_int_gate(base+i, (void(*)()) &hwirq_0+((hwirq1-hwirq0)*i));
#else
    idt.add_int_gate(base+ 0, (void(*)()) &hwirq_0);
    idt.add_int_gate(base+ 1, (void(*)()) &hwirq_1);
    idt.add_int_gate(base+ 2, (void(*)()) &hwirq_2);
    idt.add_int_gate(base+ 3, (void(*)()) &hwirq_3);
    idt.add_int_gate(base+ 4, (void(*)()) &hwirq_4);
    idt.add_int_gate(base+ 5, (void(*)()) &hwirq_5);
    idt.add_int_gate(base+ 6, (void(*)()) &hwirq_6);
    idt.add_int_gate(base+ 7, (void(*)()) &hwirq_7);
    idt.add_int_gate(base+ 8, (void(*)()) &hwirq_8);
    idt.add_int_gate(base+ 9, (void(*)()) &hwirq_9);
    idt.add_int_gate(base+10, (void(*)()) &hwirq_10);
    idt.add_int_gate(base+11, (void(*)()) &hwirq_11);
    idt.add_int_gate(base+12, (void(*)()) &hwirq_12);
    idt.add_int_gate(base+13, (void(*)()) &hwirq_13);
    idt.add_int_gate(base+14, (void(*)()) &hwirq_14);
    idt.add_int_gate(base+15, (void(*)()) &hwirq_15);
#endif

    /* initialize master
       - one slave connected to pin 2
       - reports to vectors 0x20-0x21,0x23-0x27 */
    master.init(base, (1 << 2));

    /* initialize slave
       - its slave id is 2
       - reports to vectors 0x28-0x2F*/
    slave.init(base+8, 2);

    /* Initialize IA32 interrupt handling */
    for (word_t i = 0; i < IRQS; i++) {
        irq_mapping[i].handler = NULL;

        ia32_mask_irq(i);
    }
    bitmap_init(irq_pending, IRQS, false);

    /* unmask the slave on the master */
    master.unmask(2);
}

/*
 * We don't do any CPU local init for PIC right now
 */
void ia32_init_local_intctrl(void)
{
}

void ia32_ack_irq(word_t irq)
{
    if (irq >= 8)
    {
        slave.ack(irq-8);
        master.ack(2);
    }
    else
    {
        master.ack(irq);
    }
}

void ia32_mask_irq(word_t irq)
{
    (irq < 8) ? master.mask(irq) : slave.mask(irq-8);
}

void ia32_unmask_irq(word_t irq)
{
    (irq < 8) ? master.unmask(irq) : slave.unmask(irq-8);
}

void ia32_mask_and_ack_irq(word_t irq)
{
    if (irq >= 8)
    {
        slave.mask(irq-8);
        slave.ack(irq-8);
        master.ack(2);
    }
    else
    {
        master.mask(irq);
        master.ack(irq);
    }
}

/* check if interrupt is masked on  PIC */
bool ia32_is_masked_irq(word_t irq)
{
    return (irq >= 8) ? slave.is_masked(irq-8) : master.is_masked(irq);
}

/* IA32 common interrupt handling */
static bool ia32_do_irq(word_t irq, continuation_t cont)
{
    void *handler = irq_mapping[irq].handler;
    word_t mask = irq_mapping[irq].notify_mask;

    ia32_mask_and_ack_irq(irq);

    if (EXPECT_TRUE(handler)) {
        word_t *irq_desc = &((tcb_t*)handler)->get_utcb()->platform_reserved[0];

        //printf("irq %d, handler: %p : %08lx\n", irq, handler, mask);
        if (EXPECT_TRUE(*irq_desc == -1UL)) {
            *irq_desc = irq;
            return PlatformSupport::deliver_notify(handler, mask, cont);
        } else {
            //printf(" - mark pending\n");
            bitmap_set(irq_pending, irq);
        }
    } else {
        printf("spurious? mask %d\n", irq);
    }
    return false;
}

namespace Platform {

extern "C" void handle_interrupt(word_t arg1, word_t arg2, word_t arg3)
{
    continuation_t cont = ASM_CONTINUATION;
    word_t irq = arg1;
    word_t wakeup = 0;

    if (EXPECT_FALSE(ia32_is_masked_irq(irq))) {
        TRACE("spurious IRQ %d raised\n", irq);
        ACTIVATE_CONTINUATION(cont);
    }
    /* unmask timer interrupt */
    ia32_unmask_irq(8);

    wakeup = ia32_do_irq(irq, cont);

    if (wakeup) {
        PlatformSupport::schedule(cont);
    } else {
        ACTIVATE_CONTINUATION(cont);
    }
    NOTREACHED();
}

word_t security_control_interrupt(irq_desc_t *desc, void *owner, word_t control)
{
    word_t irq, *irq_desc;
    //printf("plat: sec control: %p, owner %p, ctrl %lx\n",
    //        desc, owner, control);

    irq_desc = (word_t*)desc;

    irq = *irq_desc;

    //printf(" - sec: irq %d\n", irq);
    if (irq >= IRQS) {
        return EINVALID_PARAM;
    }

    switch (control >> 16) {
    case 0: // grant
        if (irq_owners[irq].owner) {
            return EINVALID_PARAM;
        }
        //printf(" - grant\n");
        irq_owners[irq].owner = owner;
        break;
    case 1: // revoke
        if (irq_owners[irq].owner != owner) {
            return EINVALID_PARAM;
        }
        //printf(" - revoke\n");
        irq_owners[irq].owner = NULL;
        break;
    default:
        return EINVALID_PARAM;
    }

    return 0;
}

word_t config_interrupt(irq_desc_t *desc, void *handler, irq_control_t control,
        void *utcb)
{
    word_t irq, *irq_desc;
    //printf("plat: config interrupt: %p, handler %p, ctrl %lx, utcb: %p\n",
    //        desc, handler, control.get_raw(), utcb);

    irq_desc = (word_t*)desc;

    irq = *irq_desc;

    //printf(" - conf: irq %d\n", irq);
    if (irq >= IRQS) {
        return EINVALID_PARAM;
    }

    switch (control.get_op()) {
    case irq_control_t::op_register:
        {
            void *owner = (void*)((tcb_t*)handler)->get_space();

            if (irq_owners[irq].owner != owner) {
                return ENO_PRIVILEGE;
            }
            if (irq_mapping[irq].handler) {
                return EINVALID_PARAM;
            }
            //printf(" - reg\n");
            irq_mapping[irq].handler = handler;
            irq_mapping[irq].notify_mask = (1UL << control.get_notify_bit());

            /* XXX this is bad */
            if (((tcb_t*)handler)->get_utcb()->platform_reserved[0] == 0) {
                //printf(" - init\n");
                ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = -1UL;
            }

            ia32_unmask_irq(irq);
        }
        break;
    case irq_control_t::op_unregister:
        {
            void *owner = (void*)((tcb_t*)handler)->get_space();

            if (irq_owners[irq].owner != owner) {
                return ENO_PRIVILEGE;
            }
            if (irq_mapping[irq].handler != handler) {
                return EINVALID_PARAM;
            }
            //printf(" - unreg\n");
            irq_mapping[irq].handler = 0;
            irq_mapping[irq].notify_mask = 0;

            /* XXX this is bad */
            ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = -1UL;

            ia32_mask_irq(irq);
        }
        break;
    case irq_control_t::op_ack:
    case irq_control_t::op_ack_wait:
        {
            int pend;
            word_t i;

            bitmap_t tmp_pending[BITMAP_SIZE(IRQS)];

            //printf(" - ack\n");
            if (irq_mapping[irq].handler != handler) {
                return ENO_PRIVILEGE;
            }

            // XXX checks here
            ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = -1UL;
            ia32_unmask_irq(irq);

            pend = bitmap_findfirstset(irq_pending, IRQS);

            /* handle pending interrupts for handler */
            if (EXPECT_FALSE(pend != -1)) {
                for (i = 0; i < BITMAP_SIZE(IRQS); i++) {
                    tmp_pending[i] = irq_pending[i];
                }

                do {
                    if (pend != -1) {
                        bitmap_clear(tmp_pending, pend);

                        void *handler = irq_mapping[irq].handler;
                        if (handler == handler) {
                            bitmap_clear(irq_pending, pend);
                            //printf(" - pend = %ld\n", pend);

                            (void)ia32_do_irq(pend, NULL);
                            break;
                        }
                    }
                    pend = bitmap_findfirstset(tmp_pending, IRQS);
                } while (pend != -1);
            }
        }
        break;
    }

    return 0;
}

}
