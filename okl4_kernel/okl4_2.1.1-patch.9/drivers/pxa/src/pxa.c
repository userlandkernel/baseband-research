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

#include <stdio.h>
#include <assert.h>
#include <naming/naming.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/misc.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>

#include <pxa/pxa.h>

uint32_t ssp1_virt;
uint32_t dma_virt, dmah_virt;
uint32_t gpio_virt;
uint32_t clocks_virt;
uint32_t pwm0_virt, pwm1_virt;
uint32_t i2s_virt;
uint32_t i2c_virt;

uintptr_t ssp1_obj;
uintptr_t dma_obj, dmah_obj;
uintptr_t gpio_obj;
uintptr_t clocks_obj;
uintptr_t pwm0_obj, pwm1_obj;
uintptr_t i2s_obj;
uintptr_t i2c_obj;

#ifdef CONFIG_PXA270
uint32_t qci_virt;
uintptr_t qci_obj;
#endif

int
pxa_init(void)
{
    int r;

    ssp1_obj = dma_obj = dmah_obj = gpio_obj =
        clocks_obj = pwm0_obj = pwm1_obj = 0;

#ifdef CONFIG_PXA270
    qci_obj = 0;
#endif

    /* Map in the SSP registers */
    ssp1_obj = memsection_create_user(0x1000, &ssp1_virt);
    assert(ssp1_obj != 0);

    r = hardware_back_memsection(ssp1_obj, SSP1_BASE, L4_IOMemory);
    assert(r == 0);

    /* Map in the DMA registers */
    dma_obj = memsection_create_user(0x1000, &dma_virt);
    assert(dma_obj != 0);

    r = hardware_back_memsection(dma_obj, DMA_BASE, L4_IOMemory);
    assert(r == 0);

    dmah_obj = memsection_create_user(0x1000, &dmah_virt);
    assert(dmah_obj != 0);

    r = hardware_back_memsection(dmah_obj, DMAH_BASE, L4_IOMemory);
    assert(r == 0);

    /* Map in the GPIO registers */
    gpio_obj = memsection_create_user(0x1000, &gpio_virt);
    assert(gpio_obj != 0);

    r = hardware_back_memsection(gpio_obj, GPIO_BASE, L4_IOMemory);
    assert(r == 0);

    /* Map in the Clock registers */
    clocks_obj = memsection_create_user(0x1000, &clocks_virt);
    assert(clocks_obj != 0);

    r = hardware_back_memsection(clocks_obj, CLOCKS_BASE, L4_IOMemory);
    assert(r == 0);

    /* Map in the PWM registers */
    pwm0_obj = memsection_create_user(0x1000, &pwm0_virt);
    assert(pwm0_obj != 0);

    r = hardware_back_memsection(pwm0_obj, PWM0_BASE, L4_IOMemory);
    assert(r == 0);

    pwm1_obj = memsection_create_user(0x1000, &pwm1_virt);
    assert(pwm1_obj != 0);

    r = hardware_back_memsection(pwm1_obj, PWM1_BASE, L4_IOMemory);
    assert(r == 0);

#ifdef CONFIG_PXA270
    /* Map in the QCI registers */
    qci_obj = memsection_create_user(0x1000, &qci_virt);
    assert(qci_obj != 0);

    r = hardware_back_memsection(qci_obj, QCI_BASE, L4_IOMemory);
    assert(r == 0);
#endif

    /* Map in the I2S registers */
    i2s_obj = memsection_create_user(0x1000, &i2s_virt);
    assert(i2s_obj != 0);

    r = hardware_back_memsection(i2s_obj, I2S_BASE, L4_IOMemory);
    assert(r == 0);

    /* Map in the I2C registers */
    i2c_obj = memsection_create_user(0x1000, &i2c_virt);
    assert(i2c_obj != 0);
    r = hardware_back_memsection(i2c_obj, I2C_BASE & 0xFFFFF000, L4_IOMemory);
    assert(r == 0);
    i2c_virt += 0x680;          /* Not page aligned start */

    // printf("clocks_virt: 0x%lx\n", clocks_virt);
    // printf("dma_virt: 0x%lx\n", dma_virt);
    // printf("ssp1_virt: 0x%lx\n", ssp1_virt);
    // printf("gpio_virt: 0x%lx\n", gpio_virt);
    // printf("pwm0_virt: 0x%lx\n", pwm0_virt);
    // printf("pwm1_virt: 0x%lx\n", pwm1_virt);
    // printf("qci_virt: 0x%lx\n", qci_virt);
    // printf("i2s_virt: 0x%lx\n", i2s_virt);

    return 0;
}

void
pxa_destroy(void)
{
    if (ssp1_obj)
        memsection_delete(ssp1_obj);
    if (gpio_obj)
        memsection_delete(gpio_obj);
    if (clocks_obj)
        memsection_delete(clocks_obj);
    if (dma_obj)
        memsection_delete(dma_obj);
    if (dmah_obj)
        memsection_delete(dmah_obj);
    if (pwm0_obj)
        memsection_delete(pwm0_obj);
    if (pwm1_obj)
        memsection_delete(pwm1_obj);
#ifdef CONFIG_PXA270
    if (qci_obj)
        memsection_delete(qci_obj);
#endif
    if (i2s_obj)
        memsection_delete(i2s_obj);
    if (i2c_obj)
        memsection_delete(i2c_obj);
}

/*
 * Set bits in CKEN - emulates a linux function of the same name.
 *
 * FIXME Note: this is not atomic, and could break if multiple threads are
 * accessing CKEN concurrently.
 */
void
pxa_set_cken(int cken_mask, int on)
{
    if (on)
        CKEN |= cken_mask;
    else
        CKEN &= ~(cken_mask);
}

void
pxa_gpio_func(int gpio, int mode, int dir)
{
    if (dir)
        GPDR(gpio) |= GPIO_BIT(gpio);
    else
        GPDR(gpio) &= ~GPIO_BIT(gpio);

    /* Set the alternate function register */
    GAFR(gpio) = (GAFR(gpio) & ~GPIO_GAFR_MASK(gpio)) |
        GPIO_GAFR_MODE(mode, gpio);

}

void
pxa_gpio_set(int gpio, int on)
{
    if (on)
        GPSR(gpio) = GPIO_BIT(gpio);
    else
        GPCR(gpio) = GPIO_BIT(gpio);
}

/*
 * FIXME: This function is board specific
 */
int
pxa_pwm_init(int pwm)
{
    /* Set the control register to divide by 2 */
    PWMCR0 = 0x01;
    PWMCR1 = 0x01;

    /* Set the duty cycle */
    PWMDCR0 = 0x06;
    PWMDCR1 = 0x06;

    /* Set the period */
    PWMPCR0 = 0x0A;
    PWMPCR1 = 0x0A;

    pxa_set_cken(CKEN0_PWM0, 1);
    pxa_set_cken(CKEN1_PWM1, 1);

    return 0;
}
