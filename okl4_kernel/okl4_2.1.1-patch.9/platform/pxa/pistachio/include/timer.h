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
 * Description:   PXA XSCALE OS Timers
 */
#ifndef __PLATFORM__PXA__TIMER_H__
#define __PLATFORM__PXA__TIMER_H__

#include <kernel/arch/platsupport.h>

/* PXA2XX Timer */
static const int TIMER_TICK_LENGTH = 5000;
    
#define TIMER_POFFSET           0x0a00000
#define TIMER_VOFFSET           0x0001000
#define CLOCKS_POFFSET          0x1300000
#define CLOCKS_VOFFSET          0x0002000

#if defined(CONFIG_SUBPLAT_PXA270)
#define TIMER_RATE              3250000
#elif defined(CONFIG_SUBPLAT_PXA255)
#define TIMER_RATE              3686400
#else
#error Invalid sub-architecture
#endif

#if defined(CONFIG_PXA270)
/* CLKCFG Modes */
#define CLKCFG_B        (1<<3)  /* Fast Bus Mode        */
#define CLKCFG_HT       (1<<2)  /* Half Turbo           */
#define CLKCFG_F        (1<<1)  /* Start Freq Change    */
#define CLKCFG_T        (1)     /* Turbo Mode           */
#endif

#if !defined(ASSEMBLY)

#define XSCALE_TIMERS           (IODEVICE_VADDR + TIMER_VOFFSET)

extern volatile word_t xscale_match;

/* Match registers */
#define XSCALE_OS_TIMER_MR0     (*(volatile word_t *)(XSCALE_TIMERS + 0x00))
#define XSCALE_OS_TIMER_MR1     (*(volatile word_t *)(XSCALE_TIMERS + 0x04))
#define XSCALE_OS_TIMER_MR2     (*(volatile word_t *)(XSCALE_TIMERS + 0x08))
#define XSCALE_OS_TIMER_MR3     (*(volatile word_t *)(XSCALE_TIMERS + 0x0c))

/* Interrupt enable register */
#define XSCALE_OS_TIMER_IER     (*(volatile word_t *)(XSCALE_TIMERS + 0x1c))
/* Watchdog match enable register */
#define XSCALE_OS_TIMER_WMER    (*(volatile word_t *)(XSCALE_TIMERS + 0x18))
/* Timer count register */
#define XSCALE_OS_TIMER_TCR     (*(volatile word_t *)(XSCALE_TIMERS + 0x10))
/* Timer status register */
#define XSCALE_OS_TIMER_TSR     (*(volatile word_t *)(XSCALE_TIMERS + 0x14))

#define XSCALE_CLOCKS           (IODEVICE_VADDR + CLOCKS_VOFFSET)

#define XSCALE_CLOCKS_CCCR      (*(volatile word_t *)(XSCALE_CLOCKS + 0x00))

#endif

namespace Platform {

    void handle_timer_interrupt(bool wakeup, continuation_t cont) NORETURN;

}

#endif /*__PLATFORM__PXA__TIMER_H__ */
