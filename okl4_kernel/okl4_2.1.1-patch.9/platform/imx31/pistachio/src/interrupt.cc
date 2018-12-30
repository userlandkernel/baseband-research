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
 * Description:  imx31 Platform Interrupt Handling
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

#if defined(CONFIG_PERF)
extern unsigned long count_CCNT_overflow;
extern unsigned long count_PMN0_overflow;
extern unsigned long count_PMN1_overflow;
#endif

namespace Platform {

/* IMX31 Interrupt controller */
volatile struct IMX31_AVIC * imx31_avic = (struct IMX31_AVIC *)AVIC_VADDR;

/*
 * Interrupt mapping table
 */
irq_mapping_t irq_mapping[IRQS];
irq_owner_t   irq_owners[IRQS];
bitmap_t      irq_pending[BITMAP_SIZE(IRQS)];

void imx31_mask_irq(word_t irq)
{
    ASSERT(DEBUG, irq < (word_t) IRQS);

    imx31_avic->intdisnum = irq;
}

void imx31_unmask_irq(word_t irq)
{
    ASSERT(DEBUG, irq < (word_t) IRQS);

    imx31_avic->intennum = irq;
}

void imx31_set_priority_irq(word_t irq, word_t priority)
{
    ASSERT(DEBUG, irq < (word_t) IRQS);
    ASSERT(DEBUG, priority < 16);
    int ofs = (IRQS - 1 - irq) >> 3;
    int shift = (irq & 0x7) * 4;
    imx31_avic->nipriority[ofs] = (imx31_avic->nipriority[ofs] & ~(0xf << shift)) | 
                                        (priority & 0xf) << shift;
}

bool ticks_disabled;

void disable_timer_tick(void)
{
    ticks_disabled = true;
    imx31_mask_irq(TIMER_IRQ);
}

void disable_fiq(void)
{
    imx31_avic->intcntl |= FIDIS;
}

void SECTION(".init")
imx31_init_interrupts()
{
    imx31_avic->intcntl = 0;         /* set up int controller */

    /* disable all interrupts */
    imx31_avic->intenableh = 0;
    imx31_avic->intenableh = 0;

    /* all interrupts normal (not fast) */
    imx31_avic->inttypeh = 0;
    imx31_avic->inttypel = 0;

    ticks_disabled = false;
}

static bool imx31_do_irq(word_t irq, continuation_t cont)
{
    void *handler = irq_mapping[irq].handler.get_tcb();
    word_t mask = irq_mapping[irq].notify_mask;

    imx31_mask_irq(irq);

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

            imx31_unmask_irq(irq);
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

            imx31_mask_irq(irq);
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
            imx31_unmask_irq(irq);

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

                            imx31_do_irq(pend, NULL);
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
 * IMX31 platform interrupt handler
 */
extern "C"
void handle_interrupt(word_t arg1, word_t arg2, word_t arg3)
{
    continuation_t cont = ASM_CONTINUATION;
    word_t irq, wakeup = 0;

    /* Look for a pending interrupt in the controllers */
    irq = (imx31_avic->nivecsr & 0xFFFF0000) >> 16;

#ifdef CONFIG_USE_L2_CACHE
#ifdef CONFIG_HAS_L2_EVTMON
    if (EXPECT_FALSE(irq == L2EVTMON_IRQ))
    {
        arm_l2_event_monitor_t *l2evtmon = get_arm_l2evtmon();
        arm_l2_cache_t *l2cc = get_arm_l2cc();

        if (l2evtmon->is_EMC_event_occured(l2evtmon->counter_number - 1))
        {
            /**
             * L2 Cache Write Buffer occured, this is un-recoverable,
             * we can only clean and invalidate l2 cache to make
             * l2 cache to be consistent with l3, but the
             * write back could still be permanently lost.
             */
            l2evtmon->clear_EMC_event(l2evtmon->counter_number - 1);
            l2cc->cache_flush();

            panic("!!!Unrecoverable L2 cache write back abort occured!!!\n");
        }
    }
#endif
#endif

#if defined(CONFIG_PERF)
    //Handle pmu_irq here
    if (EXPECT_FALSE(irq == Platform::PMU_IRQ))
    {
        unsigned long PMNC = 0;
        bool overflow = false;
        //Read PMNC
        __asm__ __volatile__ (
                "   mrc p15, 0, %0, c15, c12, 0 \n"
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
            count_PMN0_overflow++;
            if (count_PMN0_overflow == ~0UL) overflow = true;
        }
        if ( overflow )
        {
            //Overflow count variable overflows, We do not handle this, leave it to user side.
        }
        else
        {
            //clear interrupt and continue
            __asm__ __volatile__ (
                    "   mcr p15, 0, %0, c15, c12, 0 \n"
                    : :"r" (PMNC)
            );
            ACTIVATE_CONTINUATION(cont);
        }
    }
#endif
    if (EXPECT_TRUE(ticks_disabled)) {
        ticks_disabled = false;
        imx31_unmask_irq(TIMER_IRQ);
    }

    if (EXPECT_FALSE(irq == 0xFFFF)) {
        TRACEF("spurious irq\n");
        printf("nipndh  = %08lx\n", imx31_avic->nipndh);
        printf("nipndl  = %08lx\n", imx31_avic->nipndl);
        printf("fivecsr = %08lx\n", imx31_avic->fivecsr);
        printf("nipndh  = %08lx\n", imx31_avic->fipndh);
        printf("nipndl  = %08lx\n", imx31_avic->fipndl);
        ACTIVATE_CONTINUATION(cont);
    }

    /* Handle timer interrupt */
    if (EXPECT_FALSE(irq == TIMER_IRQ))
    {
        handle_timer_interrupt(false, cont);
        NOTREACHED();
    }

    wakeup = imx31_do_irq(irq, cont);

    if (wakeup) {
        PlatformSupport::schedule(cont);
    } else {
        ACTIVATE_CONTINUATION(cont);
    }
    while (1);
}

}
