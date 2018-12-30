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

/*
 * This is an initial implementation of a QCI driver for the iBox platform.
 * Its pretty clearly inflexible
 */

/*
 * FIXME: This file shouldn't be built if we're not on a PXA270
 */

#ifdef CONFIG_PXA270

#include <stdio.h>
#include <assert.h>
#include <naming/naming.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/misc.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>

#include "pxa.h"
#include "pxa_dma.h"

#define QCI_WIDTH    720
#define QCI_HEIGHT   288

#define DEBUG 1
#if DEBUG > 0
#define DPRINT(x, arg...)  printf("qci: " x, ##arg);
#else
#define DPRINT(x, arg...)
#endif

/*
 * This should be obtained from a DMA channel allocator
 */

#define MY_HARDCODED_DMA_CHANNEL 1

struct g_qci_data {
    uint32_t *fb;
    uintptr_t fb_obj;

    uint32_t num_desc;
    pxa_dma_desc *desc;
    uintptr_t desc_obj;

    int dma_chan;
} qcid;

int
qci_get_width(void)
{
    return QCI_WIDTH;
}

int
qci_get_height(void)
{
    return QCI_HEIGHT;
}

static void
qci_setup_gpios(void)
{
    DPRINT("Setting up GPIOs\n");

    /*
     * FIXME: This is board specific
     */
    pxa_gpio_func(47, 1, GPIO_IN);      /* CIF_DD[0] */
    pxa_gpio_func(105, 1, GPIO_IN);     /* CIF_DD[1] */
    pxa_gpio_func(116, 1, GPIO_IN);     /* CIF_DD[2] */
    pxa_gpio_func(115, 2, GPIO_IN);     /* CIF_DD[3] */
    pxa_gpio_func(90, 3, GPIO_IN);      /* CIF_DD[4] */
    pxa_gpio_func(91, 3, GPIO_IN);      /* CIF_DD[5] */
    pxa_gpio_func(17, 2, GPIO_IN);      /* CIF_DD[6] */
    pxa_gpio_func(12, 2, GPIO_IN);      /* CIF_DD[7] */
    pxa_gpio_func(107, 1, GPIO_IN);     /* CIF_DD[8] */
    pxa_gpio_func(106, 1, GPIO_IN);     /* CIF_DD[9] */

    pxa_gpio_func(45, 3, GPIO_IN);      /* CIF_PCLK */
    pxa_gpio_func(44, 3, GPIO_IN);      /* CIF_LV */
    pxa_gpio_func(43, 3, GPIO_IN);      /* CIF_FV */
}

static int
qci_teardown_dma(void)
{
    if (qcid.desc) {
        /* Should free the DMA descriptor memory here */
        assert(0);

    }

    if (qcid.dma_chan >= 0) {
        /* We're just hard-coding the channel number for the moment */
    }

    return 0;
}

static int
qci_setup_dma(uint32_t *fb, uint32_t size)
{
    pxa_dma_desc *desc;
    int i = 0;
    uint32_t length;

    qcid.num_desc = (size + DESC_TRANS_MAX - 1) / DESC_TRANS_MAX;

    DPRINT("Attempting to allocate %ld descriptors\n", qcid.num_desc);
    qcid.desc_obj = memsection_create_direct(qcid.num_desc *
                                             sizeof(pxa_dma_desc),
                                             (uintptr_t *)&(qcid.desc));
    assert(qcid.desc_obj != 0);

    memsection_set_attributes(qcid.desc_obj, L4_IOMemory);

    if (qcid.desc == NULL) {
        DPRINT("Could not allocate space for descriptors\n");
        return -1;
    }

    DPRINT("Allocated descriptors: (p: 0x%p)\n", qcid.desc);

    if (((int)qcid.desc % 16) != 0) {
        DPRINT("qci_desc wasn't 16-byte aligned!\n");
        qci_teardown_dma();
        return -1;
    }

    for (i = 0; size > 0; i++) {
        desc = &(qcid.desc[i]);
        if (size > DESC_TRANS_MAX)
            length = DESC_TRANS_MAX;
        else
            length = size;

        DPRINT("Setting up descriptor[%d]: p: 0x%p\n", i, qcid.desc);

        /* Set the ddadr to point to the next descriptor.  */
        desc->ddadr = (uint32_t)&(qcid.desc[i + 1]);

        /* Set the source address to be wherever we're up to */
        desc->dsadr = 0x50000028;

        /* Set the target address to be the QCI FIFO 0 */
        desc->dtadr = (uint32_t)(fb) + i * DESC_TRANS_MAX;

        /* Set the command */
        desc->dcmd = DCMD_INCTRGADDR |
            DCMD_FLOWSRC | DCMD_WIDTH4 | DCMD_BURST32 | length;

        /*
         * Subtract what this descriptor does from the amount remaining to be
         * transferred
         */
        size -= length;

        /* If this is the last one, set the stop bit */
        if (size == 0)
            desc->ddadr |= 0x01;        /* Set the stop bit */

        DPRINT("\tddadr: %08lx\n", desc->ddadr);
        DPRINT("\tdsadr: %08lx\n", desc->dsadr);
        DPRINT("\tdtadr: %08lx\n", desc->dtadr);
        DPRINT("\tdcmd:  %08lx\n", desc->dcmd);
    }

    qcid.dma_chan = MY_HARDCODED_DMA_CHANNEL;

    if (qcid.dma_chan < 0) {
        DPRINT("PXA DMA request failed!\n");
        qci_teardown_dma();
        return -1;
    }

    DPRINT("Allocated PXA DMA channel %d\n", qcid.dma_chan);

    /* Set up the DMA channel */
    DCSR(qcid.dma_chan) &= ~(DCSR_RUN);
    DCSR(qcid.dma_chan) &= ~(DCSR_NODESC);
    DDADR(qcid.dma_chan) = (uint32_t)&(qcid.desc[0]);

    /* Set up the request mapping */
    DRCMRQCIFIFO0 = DRCMR_MAPVLD | DRCMR_CHLNUM_VAL(qcid.dma_chan);
}

static int
qci_enable_dma(void)
{
    if (qcid.dma_chan >= 0) {
        DDADR(qcid.dma_chan) = (uint32_t)&(qcid.desc[0]);
        DCSR(qcid.dma_chan) |= DCSR_RUN;
    }
}

#if 0
static int
qci_disable_dma(void)
{
    if (qcid.dma_chan >= 0)
        DCSR(qcid.dma_chan) &= ~(DCSR_RUN);
}
#endif

#if DEBUG>1
static void
qci_dump_regs(void)
{
    /*
     * Dump the registers so we know what's going on
     */
    DPRINT("CICR0: %08lx\n", CICR0);
    DPRINT("CICR1: %08lx\n", CICR1);
    DPRINT("CICR2: %08lx\n", CICR2);
    DPRINT("CICR3: %08lx\n", CICR3);
    DPRINT("CICR4: %08lx\n", CICR4);
    DPRINT("CIFR:  %08lx\n", CIFR);
    DPRINT("CISR:  %08lx\n", CISR);
    DPRINT("\n");
}
#endif

static inline void
qci_enable_capture(void)
{
    /* Empty the FIFOs */
    CIFR |= CIFR_RESET_F;

    /* Enable the QCI */
    CICR0 |= CICR0_ENB;
}

static inline void
qci_disable_capture(void)
{
    CICR0 &= ~CICR0_ENB;
}

int
qci_capture_frame(void)
{
    qci_disable_capture();

    /* Make sure we always get the same field */
    while ((GPLR(22) & GPIO_BIT(22)) == 0);

    qci_enable_capture();
    qci_enable_dma();

    /* Wait for the DMA to finish */
    while ((DCSR(qcid.dma_chan) & DCSR_STOPINTR) == 0);

    qci_disable_capture();

    return 0;
}

int
qci_init(uint32_t **buf)
{
    uint32_t size;
    uint32_t tmp;

    /* Calculate the size in bytes (for luminance) */
    size = QCI_HEIGHT * QCI_WIDTH;

    DPRINT("Attempting to allocate %dx%d pixel framebuffer\n",
           QCI_WIDTH, QCI_HEIGHT);
    qcid.fb_obj = memsection_create_direct(size, (uintptr_t *)&tmp);
    assert(qcid.fb_obj != 0);
    memsection_set_attributes(qcid.fb_obj, L4_IOMemory);

    qcid.fb = (uint32_t *)tmp;
    if (qcid.fb == NULL) {
        DPRINT("Could not allocate space for descriptors\n");
        return -1;
    }

    DPRINT("Allocated frame buffer: (p: 0x%p)\n", qcid.fb);

    qci_setup_dma(qcid.fb, size);

    DPRINT("Setting up quick capture interface\n");

    /* Set the GPIOs up for the QCI interface for the IBOX */
    qci_setup_gpios();

    /* Set up the QCI clock */
    pxa_set_cken(CKEN24_CAMERA, 1);

    /* Set up the QCI registers */
    CICR0 &= ~CICR0_ENB;        /* Make sure the QCI is disabled */

    CICR1 = CICR1_PPL_VAL(QCI_WIDTH - 1) | CICR1_YCBCR_F | CICR1_COLOR_SP_VAL(2) |
            /* YCbCr  colour  space */ CICR1_DW_VAL(0x2); /* 8 bit wide interface */

    CICR2 = CICR2_BLW_VAL(254) |
        CICR2_ELW_VAL(0) |
        CICR2_HSW_VAL(0) | CICR2_BFPW_VAL(0) | CICR2_FSW_VAL(0);

    CICR3 = CICR3_BFW_VAL(19) |
        CICR3_EFW_VAL(0) | CICR3_VSW_VAL(0) | CICR3_LPF_VAL(QCI_HEIGHT - 1);

    CICR4 = CICR4_PCLK_EN | CICR4_FR_RATE_VAL(0) | CICR4_DIV_VAL(0);

    CIFR = CIFR_THL_0_VAL(0) |  /* DMA request at 32 bytes */
        CIFR_RESET_F |          /* Clear any data out of the FIFOs */
        CIFR_FEN2 | CIFR_FEN1 | CIFR_FEN0;

    /* Clear the status register */
    CISR = CISR_FTO |
        CISR_RDAV_2 |
        CISR_RDAV_1 |
        CISR_RDAV_0 |
        CISR_FEMPTY_2 |
        CISR_FEMPTY_1 |
        CISR_FEMPTY_0 |
        CISR_EOL |
        CISR_PAR_ERR |
        CISR_CQD |
        CISR_CDD | CISR_SOF | CISR_EOF | CISR_IFO_2 | CISR_IFO_1 | CISR_IFO_0;

#if DEBUG > 1
    qci_dump_regs();
#endif

    /* Enable everything */
    CICR0 = CICR0_ENB | CICR0_SIM_VAL(0x00)
        | CICR0_TOM             /* No time-out interrupt */
        | CICR0_RDAVM
        | CICR0_FEM
        | CICR0_EOLM
        | CICR0_PERRM
        | CICR0_QDM
        | CICR0_CDM | CICR0_SOFM | CICR0_EOFM | CICR0_FOM | CICR0_DMAEN;

    qci_disable_capture();

    qci_capture_frame();

    // qci_output_frame_serial();

    *buf = qcid.fb;

    return 0;
}

int
qci_output_frame_serial(void)
{
    int i;

#define PIXEL(x, y, array)  (((array[(y * QCI_WIDTH + x) / 4] >> (((y * QCI_WIDTH + x) % 4) * 8))) & 0xFF)

    for (i = 0; i < QCI_HEIGHT; i++) {
        int j;

        printf("x: ");
        for (j = 0; j < QCI_WIDTH; j++) {
            printf("%ld ", PIXEL(j, i, qcid.fb));
        }
        printf("\n");
    }

    return 0;
}

#endif /* CONFIG_PXA270 */
