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

#ifndef __PXA_QCI__
#define __PXA_QCI__

#define QCI_REG(x) (*(volatile uint32_t*)((char*)qci_virt + x))

extern uint32_t qci_virt;

#define QCI_BASE   (0x50000000)

/* Registers */
#define CICR0       QCI_REG(0x0000)
#define CICR1       QCI_REG(0x0004)
#define CICR2       QCI_REG(0x0008)
#define CICR3       QCI_REG(0x000C)
#define CICR4       QCI_REG(0x0010)
#define CISR        QCI_REG(0x0014)
#define CIFR        QCI_REG(0x0018)
#define CITOR       QCI_REG(0x001C)
#define CIBR0       QCI_REG(0x0028)
#define CIBR1       QCI_REG(0x0030)
#define CIBR2       QCI_REG(0x0038)

#define BIT(x)     (1<<(x))     /* A mask for bit X */
#define MASK(x, b) (((1 << (b)) - 1) << (x))    /* A mask for bits x+b-1:x */
#define VAL(x, b, v) (((v) << (x)) & MASK(x, b))        /* A value for bits
                                                         * x+b-1:x */

/* Bit definitions */
#define CICR0_DMAEN     BIT(31)
#define CICR0_PAR_EN    BIT(30)
#define CICR0_SL_CAP_EN BIT(29)
#define CICR0_ENB       BIT(28)
#define CICR0_DIS       BIT(27)
#define CICR0_SIM       MASK(24, 3)
#define CICR0_SIM_VAL(x) VAL(24, 3, x)
#define CICR0_TOM       BIT(9)
#define CICR0_RDAVM     BIT(8)
#define CICR0_FEM       BIT(7)
#define CICR0_EOLM      BIT(6)
#define CICR0_PERRM     BIT(5)
#define CICR0_QDM       BIT(4)
#define CICR0_CDM       BIT(3)
#define CICR0_SOFM      BIT(2)
#define CICR0_EOFM      BIT(1)
#define CICR0_FOM       BIT(0)

#define CICR1_TBIT      BIT(31)
#define CICR1_RGBT_CONV MASK(30, 2)
#define CICR1_PPL       MASK(15, 11)
#define CICR1_PPL_VAL(x) VAL(15, 11, x)
#define CICR1_RGB_CONV  MASK(12, 3)
#define CICR1_RGB_F     BIT(11)
#define CICR1_YCBCR_F   BIT(10)
#define CICR1_RGB_BPP   MASK(7, 3)
#define CICR1_RAW_BPP   MASK(5, 2)
#define CICR1_COLOR_SP  MASK(3, 2)
#define CICR1_COLOR_SP_VAL(x) VAL(3, 2, x)
#define CICR1_DW        MASK(0, 3)
#define CICR1_DW_VAL(x) VAL(0, 3, x)

/* CICR2 */
#define CICR2_BLW       MASK(24, 8)
#define CICR2_BLW_VAL(x) VAL(24, 8, x)
#define CICR2_ELW       MASK(16, 8)
#define CICR2_ELW_VAL(x) VAL(16, 8, x)
#define CICR2_HSW       MASK(10, 6)
#define CICR2_HSW_VAL(x) VAL(10, 6, x)
#define CICR2_BFPW      MASK(3, 6)
#define CICR2_BFPW_VAL(x) VAL(3, 6, x)
#define CICR2_FSW       MASK(0, 3)
#define CICR2_FSW_VAL(x) VAL(0, 3, x)

#define CICR3_BFW       MASK(24, 8)
#define CICR3_BFW_VAL(x) VAL(24, 8, x)
#define CICR3_EFW       MASK(16, 8)
#define CICR3_EFW_VAL(x) VAL(16, 8, x)
#define CICR3_VSW       MASK(11, 5)
#define CICR3_VSW_VAL(x) VAL(11, 5, x)

#define CICR3_LPF       MASK(0, 11)
#define CICR3_LPF_VAL(x) VAL(0, 11, x)

#define CICR4_MCLK_DLY  MASK(24, 2)
#define CICR4_PCLK_EN   BIT(23)
#define CICR4_PCP       BIT(22)
#define CICR4_HSP       BIT(21)
#define CICR4_VSP       BIT(20)
#define CICR4_MCLK_EN   BIT(19)
#define CICR4_FR_RATE   MASK(8, 3)
#define CICR4_FR_RATE_VAL(x) VAL(8, 3, x)
#define CICR4_DIV       MASK(0, 8)
#define CICR4_DIV_VAL(x) VAL(0, 8, x)

#define CISR_FTO        BIT(15)
#define CISR_RDAV_2     BIT(14)
#define CISR_RDAV_1     BIT(13)
#define CISR_RDAV_0     BIT(12)
#define CISR_FEMPTY_2   BIT(11)
#define CISR_FEMPTY_1   BIT(10)
#define CISR_FEMPTY_0   BIT(9)
#define CISR_EOL        BIT(8)
#define CISR_PAR_ERR    BIT(7)
#define CISR_CQD        BIT(6)
#define CISR_CDD        BIT(5)
#define CISR_SOF        BIT(4)
#define CISR_EOF        BIT(3)
#define CISR_IFO_2      BIT(2)
#define CISR_IFO_1      BIT(1)
#define CISR_IFO_0      BIT(0)

#define CIFR_FLVL2      MASK(23, 7)
#define CIFR_FLVL1      MASK(16, 7)
#define CIFR_FLVL0      MASK(8, 8)
#define CIFR_THL_0      MASK(4, 2)
#define CIFR_THL_0_VAL(x) VAL(4, 2, x)
#define CIFR_RESET_F    BIT(3)
#define CIFR_FEN2       BIT(2)
#define CIFR_FEN1       BIT(1)
#define CIFR_FEN0       BIT(0)

/* Prototypes */
int qci_init(uint32_t **buf);
int qci_capture_frame(void);
int qci_output_frame_serial(void);
int qci_get_width(void);
int qci_get_height(void);
#endif
