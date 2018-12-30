/*
 * Copyright (c) 2004-2006, National ICT Australia (NICTA)
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
 * Description:   PXA Periodic timer handling
 */

#include <kernel/l4.h>
#include <kernel/platform_support.h>
#include <kernel/plat/intctrl.h>
#include <kernel/plat/interrupt.h>
#include <kernel/plat/timer.h>
#include <kernel/plat/kernel_counter.h>

volatile word_t xscale_match;

namespace Platform
{

void NORETURN
handle_timer_interrupt(bool wakeup, continuation_t cont)
{
    word_t value = XSCALE_OS_TIMER_TCR;
    word_t match = xscale_match;
    match += TIMER_RATE / (1000000/TIMER_TICK_LENGTH);

    if (((s32_t)(match - value)) < 100)
    {
        match = value + TIMER_RATE / (1000000/TIMER_TICK_LENGTH);
    }

    XSCALE_OS_TIMER_MR0 = match;
    XSCALE_OS_TIMER_TSR = 0x01;  /* Clear interrupt */

#ifdef KERNEL_COUNTER_EXAMPLE
    if (handle_kernel_counter_interrupt() == true) {
        wakeup = true;
    }
#endif

    xscale_match = match;
    PlatformSupport::scheduler_handle_timer_interrupt(wakeup, cont);
}

bool ticks_disabled;

void disable_timer_tick(void)
{
    ticks_disabled = true;
    pxa_mask(XSCALE_IRQ_OS_TIMER);
}

word_t Platform::init_clocks(void)
{
    /* Setup initial timer interrupt */
    XSCALE_OS_TIMER_TSR = 0x0f;
    XSCALE_OS_TIMER_MR0 = xscale_match = TIMER_RATE / (1000000/TIMER_TICK_LENGTH);
    XSCALE_OS_TIMER_TCR = 0x00000000;
    XSCALE_OS_TIMER_IER = 0x01;     /* Enable timer channel 0 */
    /* In this case we don't care if the timer interrupt is already
     * pending, so we ignore the return value
     */
    ticks_disabled = false;
    pxa_unmask(XSCALE_IRQ_OS_TIMER);

#if defined(CONFIG_DEBUG)
    {
        word_t cccr = XSCALE_CLOCKS_CCCR;
        word_t bus = 0, cpu = 0;
#if defined(CONFIG_SUBPLAT_PXA270)
        word_t l, n;
        word_t clkcfg;
#elif defined(CONFIG_SUBPLAT_PXA255)
        word_t x, y, z;

        /* XXX add checks - only do this for
         * 400MHz capable chips */
#if 0
        cccr &= ~ ((0x7) << 7);
        cccr |= (0x4 << 7);
        XSCALE_CLOCKS_CCCR = cccr;
#endif

        /* Do frequency change + enable TURBO mode */
#if 0
FIXME!
        __asm__  __volatile__ (
                "   mov     r0,     #3              \n"
                "   mcr     p14, 0, r0, c6, c0, 0   \n"
                ::: "r0"
        );

#endif
#endif

        /* Print out the clock settings.
         * This is processor specific */

#if defined (CONFIG_SUBPLAT_PXA270)
        /* Fetch CP14 CLK_CFG */
        __asm__ __volatile__ (
                "       mrc             p14, 0, %0, c6, c0, 0   \n"
                : "=r"(clkcfg) ::
                );

        /* Can never be less than 2; else equal to L */
        if (cccr & 0x1c)
            l = (cccr & 0x1f);
        else
            l = 2;
        n = ((cccr >> 7) & 0xf) / 2; /* 2n / 2 */

        bus = l * 13000000;
        cpu = bus;

        /* Bus config */
        if (clkcfg & CLKCFG_B)
            printf("Fast Bus Mode Enabled\n");
        else
            bus = bus / 2; /* bus = run-speed /2 */

        /* CPU config */
        if (clkcfg & CLKCFG_HT){
            printf("Half Turbo Mode\n");
            cpu = (cpu * n) / 2;
        }
        else if (clkcfg * CLKCFG_T) {
            printf("Turbo Mode\n");
            cpu = cpu * n;
        }

        TRACE_INIT("Mem Speed = %dkHz\n", bus);
        TRACE_INIT("CPU Speed = %dkHz\n", cpu);

#elif defined(CONFIG_SUBPLAT_PXA255)
        switch (cccr & 0x1f)
        {
        case 0x1:   x = 27; break;
        case 0x3:   x = 36; break;
        case 0x5:   x = 45; break;
        default:    x = 0;  break;
        }
        bus = x * 3686400 / 1000;
        TRACE_INIT("Mem Speed = %dkHz\n", bus);

        switch ((cccr >> 5) & 0x3)
        {
        case 0x1:   y = 1; break;
        case 0x2:   y = 2; break;
        case 0x3:   y = 4; break;
        default:    y = 0;  break;
        }
        TRACE_INIT("Run Speed = %dkHz\n", x * 3686400 * y / 1000);

        switch ((cccr >> 7) & 0x7)
        {
        case 0x2:   z = 10; break;
        case 0x3:   z = 15; break;
        case 0x4:   z = 20; break;
        case 0x6:   z = 30; break;
        default:    z = 0;  break;
        }
        cpu = x * 36864 * y * z / 100;
        TRACE_INIT("Turbo Speed = %dkHz\n", cpu);
#else
#error Invalid sub-architecture defined.
#endif
    }
#endif

    return TIMER_TICK_LENGTH;
}

}
