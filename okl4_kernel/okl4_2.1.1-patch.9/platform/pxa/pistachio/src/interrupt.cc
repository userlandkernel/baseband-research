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
 * Description:  PXA Platform Interrupt Handling
 */

#include <kernel/l4.h>
#include <kernel/tcb.h>
#include <kernel/platform_support.h>
#include <kernel/arch/platform.h>
#include <kernel/arch/special.h>
#include <kernel/plat/interrupt.h>
#include <kernel/plat/intctrl.h>
#include <kernel/plat/timer.h>

using namespace Platform;

/* performance counters */
#if defined(CONFIG_PERF)
extern word_t count_CCNT_overflow;
extern word_t count_PMN0_overflow;
extern word_t count_PMN1_overflow;
#endif

namespace Platform {

/* Interrupt Controller */
#define INTERRUPT_POFFSET       0xd00000
#define INTERRUPT_VOFFSET       0x003000

#define XSCALE_INT              (IODEVICE_VADDR + INTERRUPT_VOFFSET)

#define XSCALE_INT_ICMR         (*(volatile word_t *)(XSCALE_INT + 0x04))   /* Mask register */
#define XSCALE_INT_ICLR         (*(volatile word_t *)(XSCALE_INT + 0x08))   /* FIQ / IRQ selection */
#define XSCALE_INT_ICCR         (*(volatile word_t *)(XSCALE_INT + 0x14))   /* Control register */
#define XSCALE_INT_ICIP         (*(volatile word_t *)(XSCALE_INT + 0x00))   /* IRQ pending */
#define XSCALE_INT_ICFP         (*(volatile word_t *)(XSCALE_INT + 0x0c))   /* FIQ pending */
#define XSCALE_INT_ICPR         (*(volatile word_t *)(XSCALE_INT + 0x10))   /* Pending (unmasked) */

#if defined(CONFIG_SUBPLAT_PXA270)
/* PXA270 Extras */
#define XSCALE_INT_ICIP2        (*(volatile word_t *)(XSCALE_INT + 0x9c))   /* IRQ pending 2 */
#define XSCALE_INT_ICMR2        (*(volatile word_t *)(XSCALE_INT + 0xa0))   /* Mask register 2 */
#define XSCALE_INT_ICLR2        (*(volatile word_t *)(XSCALE_INT + 0xa4))   /* FIQ / IRQ select 2 */
#define XSCALE_INT_ICFP2        (*(volatile word_t *)(XSCALE_INT + 0xa8))   /* FIQ pending 2 */
#define XSCALE_INT_ICPR2        (*(volatile word_t *)(XSCALE_INT + 0xac))   /* Pending 2 (unmasked) */
#endif

/* FIXME: These should be defined somewhere central */
#define GPIO_POFFSET            0xe00000
#define GPIO_VOFFSET            0x004000
#define PXA_GPIO                (IODEVICE_VADDR + GPIO_VOFFSET)

#define PXA_GEDR0               (*(volatile word_t *)(PXA_GPIO + 0x48))   /* GPIO edge detect 0 */
#define PXA_GEDR1               (*(volatile word_t *)(PXA_GPIO + 0x4C))   /* GPIO edge detect 1 */
#define PXA_GEDR2               (*(volatile word_t *)(PXA_GPIO + 0x50))   /* GPIO edge detect 2 */

#if defined(CONFIG_SUBPLAT_PXA270)
#define PXA_GEDR3               (*(volatile word_t *)(PXA_GPIO + 0x148))   /* GPIO edge detect 3 */
#endif

#define BIT(x) (1<<(x))

/*
 * Interrupt mapping table for PXA
 */
irq_mapping_t irq_mapping[IRQS];
irq_owner_t   irq_owners[IRQS];
bitmap_t      irq_pending[BITMAP_SIZE(IRQS)];

/**
 * PXA Interrupt handling
 */

void pxa_mask(word_t irq)
{
    ASSERT(DEBUG, irq < (word_t) IRQS);
#if defined(CONFIG_SUBPLAT_PXA255)
    if (irq < (word_t) PRIMARY_IRQS) {
        XSCALE_INT_ICMR &= ~(1UL << irq);
    } else {
        /* This is a GPIO IRQ and we should mask GPIO_IRQ */
        XSCALE_INT_ICMR &= ~(1 << GPIO_IRQ);
    }

#elif defined(CONFIG_SUBPLAT_PXA270)
    if (irq < 32) {
        XSCALE_INT_ICMR &= ~(1UL << irq);
    } else if (irq < PRIMARY_IRQS) {
        XSCALE_INT_ICMR2 &= ~(1UL << (irq-32));
    } else {
        /* This is a GPIO IRQ and we should mask GPIO_IRQ */
        XSCALE_INT_ICMR &= ~(1 << GPIO_IRQ);
    }
#else
#error implement
#endif
}

void pxa_unmask(word_t irq)
{
    ASSERT(DEBUG, irq < (word_t) IRQS);
#if defined(CONFIG_SUBPLAT_PXA255)
    if (irq < (word_t) PRIMARY_IRQS) {
        XSCALE_INT_ICMR |= (1UL << irq);
    } else {
        /* This is a GPIO IRQ, and we should unmask GPIO_IRQ */
        XSCALE_INT_ICMR |= (1 << GPIO_IRQ);
    }

#elif defined(CONFIG_SUBPLAT_PXA270)
    if (irq < 32) {
        XSCALE_INT_ICMR |= (1UL <<irq);
    } else if (irq < PRIMARY_IRQS) {
        XSCALE_INT_ICMR2 |= (1UL << (irq-32));
    } else {
        /* This is a GPIO IRQ, and we should unmask GPIO_IRQ */
        XSCALE_INT_ICMR |= (1<< GPIO_IRQ);
    }
#else
#error implement
#endif
}

void disable_fiq(void)
{
    XSCALE_INT_ICLR = 0x00; /* No FIQs for now */
#if defined(CONFIG_SUBPLAT_PXA270)
    XSCALE_INT_ICLR2 = 0x00;
#endif
}

static bool pxa_do_irq(word_t irq, continuation_t cont)
{
    void *handler = irq_mapping[irq].handler.get_tcb();
    word_t mask = irq_mapping[irq].notify_mask;

    pxa_mask(irq);

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

            pxa_unmask(irq);
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

            pxa_mask(irq);
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
            pxa_unmask(irq);

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

                            (void)pxa_do_irq(pend, NULL);
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
 * PXA platform interrupt handler
 */
extern "C" NORETURN
void handle_interrupt(word_t arg1, word_t arg2, word_t arg3)
{
    int i;
    continuation_t cont = ASM_CONTINUATION;

    //printf("plat: interrupt\n");
    /* Read the state of the ICIP registers */
    word_t status = XSCALE_INT_ICIP;
#if defined(CONFIG_SUBPLAT_PXA270)
    word_t status2 = XSCALE_INT_ICIP2;
#endif
    word_t timer_int = 0, wakeup = 0;
#if defined(CONFIG_PERF)
    //Handle pmu_irq here
    if (EXPECT_FALSE(status & (1UL << Platform::PMU_IRQ)))
    {
        bool overflow = false;
        unsigned long PMNC = 0;
        //Read PMNC
        __asm__ __volatile__ (
                "   mrc p14, 0, %0, c0, c0, 0   \n"
                :"=r" (PMNC)
                ::
                );
        if (PMNC & (1UL << 10)) //CCNT overflow
        {
            if (count_CCNT_overflow == ~0UL) overflow = true;    
            count_CCNT_overflow++;
        }
        if (PMNC & (1UL << 9)) //PMN1 overflow
        {
            if (count_PMN1_overflow == ~0UL) overflow = true;
            count_PMN1_overflow++;
        }
        if (PMNC & (1UL << 8)) //PMN0 overflow
        {
            if (count_PMN0_overflow == ~0UL) overflow = true;
            count_PMN0_overflow++;
        }
        if ( overflow )
        {
            //overflow cout variable overflows, We do not handle this, leave it to user side.
        }
        else
        {
            // Clear interrupt and continue
            __asm__ __volatile__ (
                    "   mcr p14, 0, %0, c0, c0, 0   \n"
                    :
                    :"r" (PMNC)
                    );
            ACTIVATE_CONTINUATION(cont);
        }
    }
#endif
    /* Handle timer interrupt */
    if (status & (1UL << XSCALE_IRQ_OS_TIMER))
    {
        timer_int = 1;
        status &= ~(1UL << XSCALE_IRQ_OS_TIMER);
        if (status == 0) {
            handle_timer_interrupt(false, cont);
            NOTREACHED();
        }
    }

    /* Special case the GPIO interrupt for now
       FIXME: Should be installed as a regular IRQ handler
       which would make this code neater */
    if (EXPECT_FALSE(status & (1UL << GPIO_IRQ))) {
        printf("gpio\n");
        while(1);
        //handle_gpio_irq(GPIO_IRQ, context, ASM_CONTINUATION);
    }

#if defined(CONFIG_SUBPLAT_PXA270)
    /* FIXME XXX: Use ICHP register to directly work out which
       interrupt is pending */

    if (status)
    {
        i = msb(status);
        //interrupt_handlers[i](i, context, ASM_CONTINUATION);
        printf("irq %d\n", i);
    } else if (status2)
    {
        i = 32 + msb(status2);
        //interrupt_handlers[i](i, context, ASM_CONTINUATION);
        printf("irq %d\n", i);
    }

#elif defined(CONFIG_SUBPLAT_PXA255)
    status = status & (~0x7fUL);        /* 0..6 are reserved */

    if (EXPECT_TRUE(status))
    {
        i = msb(status);
        /* This could be a wakeup from sleep, reenable timer interrupt */
        if (EXPECT_FALSE(ticks_disabled)) {
            ticks_disabled = false;
            pxa_unmask(XSCALE_IRQ_OS_TIMER);
        }

        wakeup = pxa_do_irq(i, timer_int ? NULL : cont);
    }
#else
#error subplatform not implemented.
#endif

    if (timer_int) {
        handle_timer_interrupt(wakeup, cont);
        NOTREACHED();
    }

    if (wakeup) {
        PlatformSupport::schedule(cont);
    } else {
        ACTIVATE_CONTINUATION(cont);
    }
    while (1);
}

} /* namespace Platform */
