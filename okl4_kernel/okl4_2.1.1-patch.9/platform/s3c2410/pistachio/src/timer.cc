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
 * Description:   Periodic timer handling
 */

#include <kernel/l4.h>
#include <kernel/plat/timer.h>
#include <kernel/plat/offsets.h>
#include <kernel/plat/interrupt.h>
#include <kernel/arch/platform.h>
#include <kernel/platform_support.h>

struct timer_sn2410x {
    volatile word_t  tcfg0;     /* Config register 0 */
    volatile word_t  tcfg1;     /* Config register 1 */
    volatile word_t  tcon;      /* Control register */

    volatile word_t  tcntb0;    /* Timer 0 count register */
    volatile word_t  tcmpb0;    /* Timer 0 control register */
    volatile word_t  tcnto0;    /* Timer 0 observable count register */

    volatile word_t  tcntb1;    /* Timer 1 count register */
    volatile word_t  tcmpb1;    /* Timer 1 control register */
    volatile word_t  tcnto1;    /* Timer 1 observable count register */

    volatile word_t  tcntb2;    /* Timer 2 count register */
    volatile word_t  tcmpb2;    /* Timer 2 control register */
    volatile word_t  tcnto2;    /* Timer 2 observable count register */

    volatile word_t  tcntb3;    /* Timer 3 count register */
    volatile word_t  tcmpb3;    /* Timer 3 control register */
    volatile word_t  tcnto3;    /* Timer 3 observable count register */

    volatile word_t  tcntb4;    /* Timer 4 count register */
    volatile word_t  tcnto4;    /* Timer 4 observable count register */
};

namespace Platform {

static volatile struct timer_sn2410x* st =
            (struct timer_sn2410x*)TIMER_VADDR;

void handle_timer_interrupt(bool wakeup, continuation_t cont)
{
    //printf("handle timer interrupt\n");
    PlatformSupport::scheduler_handle_timer_interrupt(wakeup, cont);
}

word_t init_clocks(void)
{
    st->tcfg0 = 7<<8; /*1/8*/
    st->tcfg1 = (st->tcfg0 & 0xfff0ffff)|0x00<<16; /* 1/2 */

    st->tcntb4 = 40625-1; /* 10ms samples */
    st->tcon = 0x2 << 20; /* Manual update timer4 */

    s3c2410_unmask_irq(TIMER4_INT);

    st->tcon = 0x5 << 20; /* Start */         // Interval Mode, Update

    return TIMER_TICK_LENGTH;
}

}
