/*
 * Copyright (c) 2006, National ICT Australia
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

#ifndef _TIMER_PXA250_H_
#define _TIMER_PXA250_H_

#define PXA250_IO_AREA      0x40000000

#define PXA250_OST_OFFSET   0x00A00000

#define OST_MR0_OFFSET      0x000
#define OST_MR1_OFFSET      0x004
#define OST_MR2_OFFSET      0x008
#define OST_MR3_OFFSET      0x00c
#define OST_CR_OFFSET       0x010
#define OST_SR_OFFSET       0x014
#define OST_WER_OFFSET      0x018
#define OST_IER_OFFSET      0x01C

#define XSCALE_TIMERS_PHYS  (PXA250_IO_AREA + PXA250_OST_OFFSET)
extern uintptr_t XSCALE_TIMERS_VADDR;

#define XSCALE_OS_TIMER_OSCR     (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_CR_OFFSET ))
#define XSCALE_OS_TIMER_OIER     (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_IER_OFFSET))

#define XSCALE_OS_TIMER_OSMR0    (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_MR0_OFFSET))
#define XSCALE_OS_TIMER_OSMR1    (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_MR1_OFFSET))
#define XSCALE_OS_TIMER_OSMR2    (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_MR2_OFFSET))
#define XSCALE_OS_TIMER_OSMR3    (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_MR3_OFFSET))

#define XSCALE_OS_TIMER_OSSR     (*(volatile uintptr_t *)(XSCALE_TIMERS_VADDR + OST_SR_OFFSET ))

#define IRQ_OSTMR0      26      /* OS timer match register 0 IRQ, used by L4
                                 * ticks */
#define IRQ_OSTMR1      27      /* OS timer match register 1 IRQ, we use it to
                                 * detect rollover */
#define IRQ_OSTMR2      28      /* OS timer match register 2 IRQ we use to
                                 * sleep() */

#define TIMER_RATE      3686400 /* Timer freq in Hz */

/*
 * Allows for 253 years before overflow
 */
#define TICKS_TO_US(ticks)      ((ticks) * 625ULL / 2304ULL)
#define US_TO_TICKS(us)         ((us) * 2304ULL / 625ULL)

#endif
