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
 * Description:   i.MX31 Interrupt Controller Support
 */
#ifndef __PLATFORM__IMX31__INTCTRL_H__
#define __PLATFORM__IMX31__INTCTRL_H__

/* Interrupt Controller */
#define AVIC_VADDR              (IO_AREA2_VADDR | IMX31_AVIC_OFFSET)

/* Interrupt sources */
#define TIMER_IRQ              28 /* GP timer 2 */

extern word_t arm_high_vector;

/* intcntl bits */
#define NIDIS       (1UL << 22)
#define FIDIS       (1UL << 21)

struct IMX31_AVIC {
    volatile word_t  intcntl;
    volatile word_t  nimask;
    volatile word_t  intennum;
    volatile word_t  intdisnum;
    volatile word_t  intenableh;
    volatile word_t  intenablel;
    volatile word_t  inttypeh;
    volatile word_t  inttypel;
    volatile word_t  nipriority[8];
    volatile word_t  nivecsr;
    volatile word_t  fivecsr;
    volatile word_t  intsrch;
    volatile word_t  intsrcl;
    volatile word_t  intfrch;
    volatile word_t  intfrcl;
    volatile word_t  nipndh;
    volatile word_t  nipndl;
    volatile word_t  fipndh;
    volatile word_t  fipndl;
    volatile word_t  vector[64];
};

extern volatile struct IMX31_AVIC * imx31_avic;

#define IH_READ(x)                      \
    do {                                \
        arm_cache::cache_drain_wb();    \
        x;                              \
    } while (0)

#define IH_WRITE(x)                     \
    do {                                \
        arm_cache::cache_drain_wb();    \
        x;                              \
        arm_cache::cache_drain_wb();    \
    } while (0)


#endif /*__PLATFORM__IMX31__INTCTRL_H__ */
