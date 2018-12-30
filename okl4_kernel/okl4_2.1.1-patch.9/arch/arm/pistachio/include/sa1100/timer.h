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
 * Description:   Functions which manipulate the SA-1100 OS timer
 */

#ifndef __ARCH__ARM__SA1100__TIMER_H_
#define __ARCH__ARM__SA1100__TIMER_H_

#define SA1100_TIMER_PHYS       0x90000000ul
#define SA1100_POWER_PHYS       0x90020000ul

#define TIMER_TICK_LENGTH       10000 /* usec */
#define TIMER_RATE              3686400
#define TIMER_PERIOD            (TIMER_RATE / (1000000/TIMER_TICK_LENGTH))

#define SA1100_OS_TIMER_BASE    (IO_AREA2_VADDR)
#define SA1100_OS_TIMER_OSMR0   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x00))
#define SA1100_OS_TIMER_OSMR1   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x04))
#define SA1100_OS_TIMER_OSMR2   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x08))
#define SA1100_OS_TIMER_OSMR3   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x0c))
#define SA1100_OS_TIMER_OSCR    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x10))
#define SA1100_OS_TIMER_OSSR    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x14))
#define SA1100_OS_TIMER_OWER    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x18))
#define SA1100_OS_TIMER_OIER    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x1c))

#define SA1100_POWER_BASE       (IO_AREA2_VADDR + 0x20000ul)
#define SA1000_POWER_PMCR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x00))    /* Power manager control register           */
#define SA1000_POWER_PSSR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x04))    /* Power manager sleep status register      */
#define SA1000_POWER_PSPR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x08))    /* Power manager scratchpad register        */
#define SA1000_POWER_PWER       (*(volatile word_t *)(SA1100_POWER_BASE + 0x0c))    /* Power manager wakeup enable register     */
#define SA1000_POWER_PCFR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x10))    /* Power manager configuration register     */
#define SA1000_POWER_PPCR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x14))    /* Power manager PLL configuration register */
#define SA1000_POWER_PGSR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x18))    /* Power manager GPIO sleep state register  */
#define SA1000_POWER_POSR       (*(volatile word_t *)(SA1100_POWER_BASE + 0x1c))    /* Power manager oscillator status register */

#endif /* __ARCH__ARM__SA1100__TIMER_H_*/
