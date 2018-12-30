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
 * Author: Scott McDaid
 */

#include <stdio.h>
#include <assert.h>

#include <iguana/memsection.h>
#include <iguana/hardware.h>

#include <l4/misc.h>

#include <pxa/pxa_i2c.h>
#include <pxa/pxa.h>

/*
 * addr is slave address, presently unused
 */
int
i2c_init(uint8_t addr)
{
    volatile uint32_t counter;

    /* Set the appropriate GPIOs up */
    /* Both I2C_SCL and I2C_SDA need their alternate functions */
    pxa_gpio_func(I2C_SCL, 1, GPIO_OUT);
    pxa_gpio_func(I2C_SDA, 1, GPIO_OUT);

    /* Enable clock */
    pxa_set_cken(CKEN14_I2C, 1);

    /* Set 7bit slave address */
    ISAR = addr & ISAR_MASK;

    ICR |= ICR_MA;

    ICR = ICR_UR;
    ISR = 0x7FF;                // Clear ISR as per datsheet
    ICR &= ~ICR_UR;

    ISAR = addr & ISAR_MASK;

    ICR |= ICR_SCLE | ICR_GCD;

    ICR |= ICR_IUE;

    counter = 0xFFFFFFUL;
    while (counter--);

    return 0;
}

void
i2c_write(uint8_t device, uint8_t *data, int len)
{
    /* Set target addr and WR mode */
    IDBR = ((device & 0x7f) << 1);
    /* Start write */
    ICR = (ICR & ~(ICR_STOP | ICR_ALDIE)) | ICR_START | ICR_TB;
    /* Spin till written */
    while (~ISR & ISR_ITE);
    if (ISR & ISR_ACKNAK) {
        printf("We got NAK'ed, retrying\n");
        ISR = 0x7F;
        IDBR = ((device & 0x7f) << 1);
        ICR = (ICR & ~(ICR_STOP | ICR_ALDIE)) | ICR_START | ICR_TB;
        while (~ISR & ISR_ITE);
    }

    /* Clear Interrupts */
    ISR |= ISR_ITE | ISR_ALD;

    ICR &= ~ICR_START;
    while (len > 1) {
        /* Load data and xmit */
        IDBR = *data++;

        ICR |= ICR_ALDIE | ICR_TB;

        while (~ISR & ISR_ITE);

        /* Clear Interrupts */
        ISR |= ISR_ITE | ISR_ALD;
        len--;
    }
    /* Load data and xmit */
    IDBR = *data++;

    /* Send stop mark */
    ICR |= ICR_STOP | ICR_ALDIE | ICR_TB;
    while (~ISR & ISR_ITE);

    /* Clear Interrupts */
    ISR |= ISR_ITE | ISR_ALD;

    ICR &= ~ICR_STOP;
}

void
i2c_read(uint8_t device, uint8_t *data, int len)
{
    /* Set target addr and WR mode */
    IDBR = (device << 1) | 0x01;
    /* Start write */
    ICR = (ICR & ~(ICR_STOP | ICR_ALDIE)) | ICR_START | ICR_TB;

    /* Spin till written */
    while (~ISR & ISR_ITE);
    /* Clear Interrupts */
    ISR |= ISR_ITE | ISR_ALD;

    ICR &= ~ICR_START;

    /* Read Phase */
    while (len) {
        ICR |= ICR_ALDIE | ICR_ACKNAK | ICR_TB;
        if (len == 1)
            ICR |= ICR_STOP;

        while (~ISR & ISR_IRF);

        /* Clear Interrupts */
        ISR |= ISR_IRF | ISR_ALD;

        *data++ = IDBR;

        len--;
    }

    ICR &= ~(ICR_STOP | ICR_ACKNAK);
}
