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
 * Author: Scott McDaid
 */

/*
 * PXA270 I2C Register Set
 *
 * Scott McDaid, 2006
 */

#ifndef __PXA270_I2C_H__
#define __PXA270_I2C_H__

#define I2C_REG(x) *(volatile uint32_t *)((char *)i2c_virt+(x))

extern uint32_t i2c_virt;

#define I2C_BASE 0x40301680

/* Bus Monitor */
#define IBMR            I2C_REG(0x00)
#define IBMR_SDA        (1<<0)
#define IBMR_SCL        (1<<0)

/* Data Buffer */
#define IDBR            I2C_REG(0x08)

/* Control */
#define ICR             I2C_REG(0x10)
#define ICR_START       (1<<0)
#define ICR_STOP        (1<<1)
#define ICR_ACKNAK      (1<<2)
#define ICR_TB          (1<<3)
#define ICR_MA          (1<<4)
#define ICR_SCLE        (1<<5)
#define ICR_IUE         (1<<6)
#define ICR_GCD         (1<<7)
#define ICR_ITEIE       (1<<8)
#define ICR_IRFIE       (1<<9)
#define ICR_BEIE        (1<<10)
#define ICR_SSDIE       (1<<11)
#define ICR_ALDIE       (1<<12)
#define ICR_SADIE       (1<<13)
#define ICR_UR          (1<<14)
#define ICR_FM          (1<<!5)

/* Status */
#define ISR             I2C_REG(0x18)
#define ISR_RWM         (1<<0)
#define ISR_ACKNAK      (1<<1)
#define ISR_UB          (1<<2)
#define ISR_IBB         (1<<3)
#define ISR_SSD         (1<<4)
#define ISR_ALD         (1<<5)
#define ISR_ITE         (1<<6)
#define ISR_IRF         (1<<7)
#define ISR_GCAD        (1<<8)
#define ISR_SAD         (1<<9)
#define ISR_BED         (1<<10)

/* Slave Address */
#define ISAR            I2C_REG(0x20)
#define ISAR_MASK       (0x7F)

int i2c_init(uint8_t addr);
void i2c_write(uint8_t device, uint8_t *data, int len);
void i2c_read(uint8_t device, uint8_t *data, int len);
#endif /* __PXA270_I2C_H__ */
