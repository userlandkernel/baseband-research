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

#ifndef __PXA_GPIO_H__
#define __PXA_GPIO_H__

#define GPIO_REG(x) (*(volatile uint32_t*)((char*)gpio_virt + x))

extern uint32_t gpio_virt;

#define GPIO_BASE   (0x40e00000)

#define GPLR0       GPIO_REG(0x0000)
#define GPLR1       GPIO_REG(0x0004)
#define GPLR2       GPIO_REG(0x0008)
#define GPLR3       GPIO_REG(0x0100)

#define GPDR0       GPIO_REG(0x000C)
#define GPDR1       GPIO_REG(0x0010)
#define GPDR2       GPIO_REG(0x0014)
#define GPDR3       GPIO_REG(0x010C)

#define GPSR0       GPIO_REG(0x0018)
#define GPSR1       GPIO_REG(0x001c)
#define GPSR2       GPIO_REG(0x0020)
#define GPSR3       GPIO_REG(0x0118)

#define GPCR0       GPIO_REG(0x0024)
#define GPCR1       GPIO_REG(0x0028)
#define GPCR2       GPIO_REG(0x002c)
#define GPCR3       GPIO_REG(0x0124)

#define GRER0       GPIO_REG(0x0030)
#define GRER1       GPIO_REG(0x0034)
#define GRER2       GPIO_REG(0x0038)
#define GRER3       GPIO_REG(0x0130)

#define GFER0       GPIO_REG(0x003C)
#define GFER1       GPIO_REG(0x0040)
#define GFER2       GPIO_REG(0x0044)
#define GFER3       GPIO_REG(0x013C)

#define GEDR0       GPIO_REG(0x0048)
#define GEDR1       GPIO_REG(0x004C)
#define GEDR2       GPIO_REG(0x0050)
#define GEDR3       GPIO_REG(0x0148)

#define GAFR0_L     GPIO_REG(0x0054)
#define GAFR0_U     GPIO_REG(0x0048)
#define GAFR1_L     GPIO_REG(0x005C)
#define GAFR1_U     GPIO_REG(0x0060)
#define GAFR2_L     GPIO_REG(0x0064)
#define GAFR2_U     GPIO_REG(0x0068)
#define GAFR3_L     GPIO_REG(0x006C)
#define GAFR3_U     GPIO_REG(0x0070)

#define GPIO_REG_OFFSET(x) (((x) & 0x60) >> 3)

/* We could implement a slightly nicer version of these for PXA25x and PXA26x */

#define _GPLR(x)        GPIO_REG(0x0000 + GPIO_REG_OFFSET(x))
#define _GPDR(x)        GPIO_REG(0x000C + GPIO_REG_OFFSET(x))
#define _GPSR(x)        GPIO_REG(0x0018 + GPIO_REG_OFFSET(x))
#define _GPCR(x)        GPIO_REG(0x0024 + GPIO_REG_OFFSET(x))
#define _GRER(x)        GPIO_REG(0x0030 + GPIO_REG_OFFSET(x))
#define _GFER(x)        GPIO_REG(0x003C + GPIO_REG_OFFSET(x))
#define _GEDR(x)        GPIO_REG(0x0048 + GPIO_REG_OFFSET(x))

#define GPLR(x)         (*((((x) & 0x7f) < 96) ? &_GPLR(x) : &GPLR3))
#define GPDR(x)         (*((((x) & 0x7f) < 96) ? &_GPDR(x) : &GPDR3))
#define GPSR(x)         (*((((x) & 0x7f) < 96) ? &_GPSR(x) : &GPSR3))
#define GPCR(x)         (*((((x) & 0x7f) < 96) ? &_GPCR(x) : &GPCR3))
#define GRER(x)         (*((((x) & 0x7f) < 96) ? &_GRER(x) : &GRER3))
#define GFER(x)         (*((((x) & 0x7f) < 96) ? &_GFER(x) : &GFER3))
#define GEDR(x)         (*((((x) & 0x7f) < 96) ? &_GEDR(x) : &GEDR3))

#define GAFR(x)         GPIO_REG(0x0054 + (((x) & 0x70) >> 2))

#define GPIO_BIT(x) (1 << ((x) & (32 - 1)))     /* Mask off the bottom 32 bits */
#define GPIO_GAFR_MODE(mode, gpio)   ((mode) << ((gpio << 1) & (32 - 1)))
#define GPIO_GAFR_MASK(gpio) ((0x03) << ((gpio << 1) & (32 - 1)))

#define GPIO_IN     0
#define GPIO_OUT    1

#define GPIO_MODE   0
#define ALT1_MODE   1
#define ALT2_MODE   2
#define ALT3_MODE   3

/* GPIO names */
#define I2C_SCL    117
#define I2C_SDA    118

#endif
