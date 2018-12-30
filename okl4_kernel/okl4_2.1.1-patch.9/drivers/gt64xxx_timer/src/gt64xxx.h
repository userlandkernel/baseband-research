/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 
#ifndef _TIMER_GT64XXX_H_
#define _TIMER_GT64XXX_H_

#define GT64XXX_IO_AREA         0x14000000

#define GT_T0_OFFSET            0x850
#define GT_T1_OFFSET            0x854
#define GT_T2_OFFSET            0x858
#define GT_T3_OFFSET            0x85C
#define GT_TC_OFFSET            0x864

#define GT_INT_CAUSE            0xc18
#define GT_INT_MASK             0xc1c

#define GT_TIMERS_PHYS      (GT64XXX_IO_AREA)
extern uintptr_t GT_TIMERS_VADDR;

#define GT_TIMER_0          (volatile uint32_t *)(GT_TIMERS_VADDR + GT_T0_OFFSET)
#define GT_TIMER_1          (volatile uint32_t *)(GT_TIMERS_VADDR + GT_T1_OFFSET)
#define GT_TIMER_2          (volatile uint32_t *)(GT_TIMERS_VADDR + GT_T2_OFFSET)
#define GT_TIMER_3          (volatile uint32_t *)(GT_TIMERS_VADDR + GT_T3_OFFSET)
#define GT_TIMER_CONTROL    (volatile uint32_t *)(GT_TIMERS_VADDR + GT_TC_OFFSET)

#define GT_INTERRUPT_CAUSE  (volatile uint32_t *)(GT_TIMERS_VADDR + GT_INT_CAUSE)
#define GT_INTERRUPT_MASK   (volatile uint32_t *)(GT_TIMERS_VADDR + GT_INT_MASK)

/* XXX U4600 specific interrupt numbers / frequencies */
#define IRQ_GT          5

#define TIMER_RATE      50000000 /* Timer freq in Hz */

/* Allows for > 10000 years before overflow */
#define TICKS_TO_US(ticks)      ((ticks) / 50)
#define US_TO_TICKS(us)         ((us) * 50)

#define REVERSE_ENDIAN

#ifdef REVERSE_ENDIAN
/*lint -e155 */
# define gt_read(adr)       ({          \
        uint32_t x = *(adr);            \
        x = (x >> 24) |                 \
            ((x >> 8) & 0x00ff00) |     \
            ((x << 8) & 0xff0000) |     \
            (x << 24);                  \
        x;                              \
})
# define gt_write(adr, d)   do {           \
        uint32_t x = d;                 \
        x = (x >> 24) |                 \
            ((x >> 8) & 0xff00) |       \
            ((x << 8) & 0xff0000) |     \
            (x << 24);                  \
        *(adr) = x;                     \
    } while(0)
#else
# define gt_read(adr)       *(adr)
# define gt_write(adr, x)   *(adr) = x
#endif

#endif
