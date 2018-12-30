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

/*
 * Author: David Snowdon
 */

#ifndef __PXA_DMA__
#define __PXA_DMA__

#define DMA_REG(x) (*(volatile uint32_t*)((char*)dma_virt + x))
#define DMAH_REG(x) (*(volatile uint32_t*)((char*)dmah_virt + x))

extern uint32_t dma_virt;

#define DMA_BASE   (0x40000000)

extern uint32_t dmah_virt;

#define DMAH_BASE   (0x40001000)

#define BIT(x)     (1<<(x))     /* A mask for bit X */
#define MASK(x, b) (((1 << (b)) - 1) << (x))    /* A mask for bits x+b-1:x */
#define VAL(x, b, v) (((v) << (x)) & MASK(x, b))        /* A value for bits
                                                         * x+b-1:x */

/* DMA registers */
#define DCSR(x)     DMA_REG(0x0000 + ((x) << 2))        /* DCSR are 4 byte
                                                         * registers starting
                                                         * at 0x0000 */
#define DALGN       DMA_REG(0x00A0)
#define DPCSR       DMA_REG(0x00A4)
#define DINT        DMA_REG(0x00F0)
#define DRCMR(x)    *((x < 64) ?                        \
                      &DMA_REG(0x0100 + ((x) << 2)) :   \
                      &DMAH_REG(0x100 + ((x-64) << 2)))
/* DRCMR are 4 byte registers startng at 0x0100 */
#define DDADR(x)    DMA_REG(0x0200 + ((x) << 4))        /* DDADR0 are 4 byte
                                                         * registers starting
                                                         * at 0x0200 that only
                                                         * occur once every
                                                         * four words */
#define DSADR(x)    DMA_REG(0x0204 + ((x) << 4))
#define DTADR(x)    DMA_REG(0x0208 + ((x) << 4))
#define DCMD(x)     DMA_REG(0x020C + ((x) << 4))

/* Named mapping registers */
#define DRCMRRXSADR   DRCMR(2)
#define DRCMRTXSADR   DRCMR(3)
#define DRCMRTXSSDR   DRCMR(14)
#define DRCMRQCIFIFO0 DRCMR(68)
#define DRCMRQCIFIFO1 DRCMR(69)
#define DRCMRQCIFIFO2 DRCMR(70)

/* DCSR Bit definitions */
/* Note - not complete */
#define DCSR_RUN      BIT(31)
#define DCSR_NODESC   BIT(30)
#define DCSR_STOPINTR BIT(3)

/* DRCMRx Bit definitions */
#define DRCMR_MAPVLD         BIT(7)
#define DRCMR_CHLNUM_MASK    MASK(0, 5)
#define DRCMR_CHLNUM_VAL(x)  VAL(0, 5, x)

/* DCMD Bit definitions */
#define DCMD_INCSRCADDR       BIT(31)
#define DCMD_INCTRGADDR       BIT(30)
#define DCMD_FLOWSRC          BIT(29)
#define DCMD_FLOWTRG          BIT(28)
#define DCMD_BURST8           (1 << 16)
#define DCMD_BURST16          (2 << 16)
#define DCMD_BURST32          (3 << 16)
#define DCMD_WIDTH1           (1 << 14)
#define DCMD_WIDTH2           (2 << 14)
#define DCMD_WIDTH4           (3 << 14)
#define DCMD_LEN(x)           VAL(0, 13, x)

/*
 * 8000 bytes for a single transfer - we can do slightly more, but this number
 * is nice
 */
#define DESC_TRANS_MAX 8000

typedef struct pxa_dma_desc {
    uint32_t ddadr;
    uint32_t dsadr;
    uint32_t dtadr;
    uint32_t dcmd;
} pxa_dma_desc;

#endif
