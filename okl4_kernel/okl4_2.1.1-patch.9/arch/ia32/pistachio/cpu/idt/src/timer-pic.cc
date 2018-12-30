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
 * Description:   Implements RTC timer
 */

#include <l4.h>
#include <arch/idt.h>
#include <arch/trapgate.h>

#include <plat/rtc.h>
#include <arch/interrupt.h>
#include <arch/timer.h>
#include <arch/platform.h>
#include <platform_support.h>
#include <tcb.h>

#define IRQLINE 8

/* global instance of timer object */
timer_t timer;

/*lint -esym(1502,rtc) */

IA32_EXC_NO_ERRORCODE(timer_interrupt, IRQLINE)
{
    /* acknowledge irq on PIC */
    ia32_ack_irq(IRQLINE);

    /* reset intr line on RTC */
    rtc_t<0x70> rtc;
    (void) rtc.read(0x0c);

    /* handle the timer */
    PlatformSupport::scheduler_handle_timer_interrupt(false, ASM_CONTINUATION);
}


word_t timer_t::init_clocks()
{
    /* TODO: Should be irq_manager.register(hwirq, 8, &timer_interrupt); */
    idt.add_int_gate(0x20+IRQLINE, timer_interrupt);

    {
        rtc_t<0x70> rtc;

        /* wait for update-in-progress to finish */
        while(rtc.read(0x0a) & 0x80);

        /* set rtc to 512, rate = 2Hz*2^(15-(x:3..0))*/
        rtc.write(0x0A, (rtc.read(0x0a) & 0xf0) | 0x07);
        /* enable interrupts
           Periodic Interrupt Enable = 0x40     */
        rtc.write(0x0b, rtc.read(0x0b) | 0x40);

        /* read(0x0c) clears all interrupts in RTC */
        (void) rtc.read(0x0c);

        ia32_unmask_irq(IRQLINE);
    }

#if defined(CONFIG_IA32_TSC)
    TRACE_INIT("calculating processor speed...\n");

    /* calculate processor speed */
    wait_for_second_tick();

    u64_t cpu_cycles = ia32_rdtsc();

    wait_for_second_tick();

    cpu_cycles = ia32_rdtsc() - cpu_cycles;

    proc_freq = cpu_cycles / 1000;
    bus_freq = 0;

    TRACE_INIT("CPU speed: %d MHz\n", (word_t)(cpu_cycles / (1000000)));
#else /* !CONFIG_IA32_TSC */

#if defined(CONFIG_CPU_IA32_I486)
/* We just estimate the current cpu speed, this is needed in
 * absence of any TSC.
 * We simply assume that it's really something like an i486
 */
    TRACE_INIT("estimating processor speed...\n");

    rtc_t<0x70> rtc;
    word_t rounds = 0;

    wait_for_second_tick ();

    // wait that update bit is off
    while (rtc.read(0x0a) & 0x80);

    // read second value
    word_t secstart = rtc.read(0);

    while (secstart == rtc.read(0)) {
        __asm__ __volatile__ (
            "       xor      %eax,%eax      \n"
            "       movl     $20000,%ecx    \n"
            "1:     add      %eax,%eax      \n"
            /* We don't use loop here by intention !!! */
            "       dec      %ecx           \n"
            "       jnz      1b             \n"
            );
        rounds++;
    }

    proc_freq = (rounds * 10000) / (99);
    bus_freq = 0;

    TRACE_INIT("Rounds: %d CPU speed: %d kHz\n",
               rounds, (word_t)((rounds * 10000) / (99)));
#else /* !CONFIG_CPU_IA32_I486 */

    proc_freq = 0;
    bus_freq = 0;

#endif /* !CONFIG_CPU_IA32_I486 */
#endif /* !CONFIG_IA32_TSC */

    return 0;
}

word_t
Platform::init_clocks(void)
{
    return timer.init_clocks();
}

void
Platform::disable_timer_tick()
{
    ia32_mask_irq(IRQLINE);
}
