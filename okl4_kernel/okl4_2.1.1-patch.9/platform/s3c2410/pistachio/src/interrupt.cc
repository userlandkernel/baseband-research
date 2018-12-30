/*
 * Copyright (c) 2003-2004, National ICT Australia (NICTA)
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
 * Description:  s3c2410 Platform Interrupt Handling
 */

#include <kernel/l4.h>
#include <kernel/tcb.h>
#include <kernel/platform_support.h>
#include <kernel/arch/platform.h>
#include <kernel/arch/platsupport.h>
#include <kernel/arch/special.h>
#include <kernel/plat/interrupt.h>
#include <kernel/plat/intctrl.h>
#include <kernel/plat/timer.h>

namespace Platform {

/*
 * Interrupt mapping table for PXA
 */
irq_mapping_t irq_mapping[IRQS];
irq_owner_t   irq_owners[IRQS];
bitmap_t      irq_pending[BITMAP_SIZE(IRQS)];
bool ticks_disabled;

void disable_fiq(void)
{
}

void s3c2410_mask_irq(word_t irq)
{
    PLAT_ASSERT(DEBUG, irq < (word_t) IRQS);

    if (irq >= 32) {
        LN2410_INT_SUBMASK |= 1 << (irq-32);
    } else {
        word_t mask = LN2410_INT_MASK;
        mask |= (1 << irq);
        LN2410_INT_MASK = (mask & ~(1UL<<31) & ~(1UL<<28) & ~(1UL<<23) & ~(1UL<<15));
    }
    //printf("Mask: %d -- %lx\n", irq, LN2410_INT_MASK);
}

void s3c2410_unmask_irq(word_t irq)
{
    PLAT_ASSERT(DEBUG, irq < (word_t) IRQS);
    //printf("Unmask: %d -- %lx\n", irq);

    if (irq >= 32) {
        LN2410_INT_SUBMASK &= ~(1 << (irq-32));
    } else {
        LN2410_INT_MASK &= ~(1 << irq);
    }
    //printf("Unmask: %d -- %lx\n", irq, LN2410_INT_MASK);
}

void disable_timer_tick()
{
    ticks_disabled = true;
    LN2410_INT_MASK |= (1 << TIMER4_INT); /* Mask it */
}

void s3c2410_init_interrupts()
{
    PLAT_TRACEF("LN2410_INT_MASK\n");

    LN2410_INT_MOD = 0; /* Set all to IRQ mode (not fiq mode) */
    LN2410_INT_SRCPND = LN2410_INT_SRCPND; /* Ack src pending */
    LN2410_INT_PND = LN2410_INT_PND; /* Ack int pending */
    LN2410_INT_SUBMASK = 0xffff;
    LN2410_INT_SUBSRCPND = 0;
    /* Mask off most all interrupts, except
     * those that are dealt with by sub controller */
    LN2410_INT_MASK = (0xffffffffUL & ~(1UL<<31) & ~(1UL<<28) & ~(1UL<<23) & ~(1UL<<15));

    PLAT_TRACEF("LN2410_INT_MASK: %lx\n", LN2410_INT_MASK);

    ticks_disabled = false;
}

static bool s3c2410_do_irq(word_t irq, continuation_t cont)
{
    void *handler = irq_mapping[irq].handler.get_tcb();
    word_t mask = irq_mapping[irq].notify_mask;

    s3c2410_mask_irq(irq);

    if (EXPECT_TRUE(handler)) {
        word_t *irq_desc = &((tcb_t*)handler)->get_utcb()->platform_reserved[0];

        //printf("irq %d, handler: %p : %08lx\n", irq, handler, mask);
        if (EXPECT_TRUE(*irq_desc == ~0UL)) {
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

/**
 * Configure access controls for interrupts
 */
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

/**
 * Acknowledge, register and unregister interrupts
 */
word_t config_interrupt(irq_desc_t *desc, void *handler,
        irq_control_t control, void *utcb)
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
            if (irq_mapping[irq].handler.get_tcb()) {
                return EINVALID_PARAM;
            }
            //printf(" - reg\n");
            irq_mapping[irq].handler.set_thread_cap((tcb_t *)handler);
            cap_reference_t::add_reference((tcb_t *)handler, &irq_mapping[irq].handler);
            irq_mapping[irq].notify_mask = (1UL << control.get_notify_bit());

            /* XXX this is bad */
            if (((tcb_t*)handler)->get_utcb()->platform_reserved[0] == 0) {
                //printf(" - init\n");
                ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = ~0UL;
            }

            s3c2410_unmask_irq(irq);
        }
        break;
    case irq_control_t::op_unregister:
        {
            void *owner = (void*)((tcb_t*)handler)->get_space();

            if (irq_owners[irq].owner != owner) {
                return ENO_PRIVILEGE;
            }
            if (irq_mapping[irq].handler.get_tcb() != handler) {
                return EINVALID_PARAM;
            }
            //printf(" - unreg\n");
            cap_reference_t::remove_reference((tcb_t *)handler, &irq_mapping[irq].handler);
            irq_mapping[irq].notify_mask = 0;

            /* XXX this is bad */
            ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = ~0UL;

            s3c2410_mask_irq(irq);
        }
        break;
    case irq_control_t::op_ack:
    case irq_control_t::op_ack_wait:
        {
            int pend;
            word_t i;

            bitmap_t tmp_pending[BITMAP_SIZE(IRQS)];

            //printf(" - ack\n");
            if (irq_mapping[irq].handler.get_tcb() != handler) {
                return ENO_PRIVILEGE;
            }

            // XXX checks here
            ((tcb_t*)handler)->get_utcb()->platform_reserved[0] = ~0UL;
            s3c2410_unmask_irq(irq);

            pend = bitmap_findfirstset(irq_pending, IRQS);

            /* handle pending interrupts for handler */
            if (EXPECT_FALSE(pend != -1)) {
                for (i = 0; i < BITMAP_SIZE(IRQS); i++) {
                    tmp_pending[i] = irq_pending[i];
                }

                do {
                    if (pend != -1) {
                        bitmap_clear(tmp_pending, pend);

                        void *handler = irq_mapping[irq].handler.get_tcb();
                        if (handler == handler) {
                            bitmap_clear(irq_pending, pend);
                            //printf(" - pend = %ld\n", pend);

                            s3c2410_do_irq(pend, NULL);
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

/**
 * s3c2410 platform interrupt handler
 */
extern "C"
void handle_interrupt(word_t arg1, word_t arg2, word_t arg3)
{
    continuation_t cont = ASM_CONTINUATION;
    int i = LN2410_INT_OFFSET;
    word_t sub;
    int irq = -1, wakeup = 0;

    switch(i) {
    case 15: case 23:
    case 28: case 31:
            /* Look up submask top bits */
            sub = LN2410_INT_SUBSRCPND & (~LN2410_INT_SUBMASK);
            if (sub == 0) {
#ifdef CONFIG_KDB_CONS
                    PlatformSupport::printf("##########: Spurious submask int\n");
#endif
                    LN2410_INT_SRCPND = 1 << i; /* Ack it */
                    LN2410_INT_PND = 1 << i; /* Ack it again (IRQ) */
                    ACTIVATE_CONTINUATION(ASM_CONTINUATION);
            }
            irq = msb(sub);

            LN2410_INT_SUBMASK |= (1<<irq);
            LN2410_INT_SUBSRCPND = (1<<irq);

            irq += 32;
            break;
    case TIMER4_INT:
            LN2410_INT_SRCPND = 1 << i; /* Ack it */
            LN2410_INT_PND = 1 << i; /* Ack it again (IRQ) */

            handle_timer_interrupt(false, cont);
            ACTIVATE_CONTINUATION(cont);
            break;
    default:
            irq = i;
            LN2410_INT_MASK |= (1 << i); /* Mask it */
    }

    if (EXPECT_FALSE(ticks_disabled)) {
        ticks_disabled = false;
        LN2410_INT_MASK &= ~(1 << TIMER4_INT); /* UnMask it */
    }

    LN2410_INT_SRCPND = 1 << i; /* Ack it */
    LN2410_INT_PND = 1 << i; /* Ack it again (IRQ) */

    wakeup = s3c2410_do_irq(irq, cont);

    if (wakeup) {
        PlatformSupport::schedule(cont);
    } else {
        ACTIVATE_CONTINUATION(cont);
    }
    NOTREACHED();
}

}
