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

#include "s3c2410_lcd.h"
#include <iguana/memsection.h>
#include <string.h>

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

#define X_RES               480
#define Y_RES               640

#define BITS_PER_PIXEL      16
#define BITS_PER_HWORD      16

#define OFFSET_AFTER_LINE   0

static void fb_set_buffer_base(struct s3c2410_lcd * device, uint32_t addr)
{
    //uint32_t    offsize   = OFFSET_AFTER_LINE;
//    uint32_t    pagewidth = X_RES * BITS_PER_PIXEL / BITS_PER_HWORD;
    //uint32_t    lcdbank   = addr >> 22 & ((1 << 9) - 1);
//    uint32_t    lcdbaseu  = (addr & ((1 << 22) - 1)) >> 1;
   // uint32_t    lcdbasel  = lcdbaseu + (pagewidth + offsize) * Y_RES;

    /*
     * XXX: should read previous bit value and restore that, instead
     *      assuming it should be set afterwards - nt
     */
    lcdcon1_set_envid(0x0);     /* clear ENVID */

//    lcdsaddr1_set_lcdbank(lcdbank);
//    lcdsaddr1_set_lcdbaseu(lcdbaseu);

    lcdsaddr1_write(addr>>1);

    lcdsaddr2_set_lcdbasel(
            (lcdsaddr1_read() & 0x1fffff) + 640*480
             );
//      lcdsaddr2_set_lcdbasel(   (addr+648*480*2)>>1);

//    lcdsaddr3_set_offsize(offsize);
    lcdsaddr3_set_pagewidth(480);


    lcdcon1_set_envid(0x1);     /* set ENVID */
}

static void fb_set_buffer_impl(struct fb_interface *fbi, struct s3c2410_lcd *device, uintptr_t buff)
{
    fb_set_buffer_base(device, buff);
}

static uintptr_t fb_get_buffer_impl(struct fb_interface *fbi, struct s3c2410_lcd *device)
{
    dprintf("%s [ lcd] : This shouldn't be called - server knows better than I do!\n", __func__);
    return 0;
}

static void fb_get_mode_impl(struct fb_interface *fbi, struct s3c2410_lcd *device, uint32_t * xres, uint32_t * yres, uint32_t * bpp)
{

    *xres = X_RES;
    *yres = Y_RES;
    *bpp  = BITS_PER_PIXEL;

    return;
}

static int fb_set_mode_impl(struct fb_interface *fbi, struct s3c2410_lcd *device, uint32_t xres, uint32_t yres, uint32_t bpp)
{
    dprintf("%s [ lcd] : Not supported!\n", __func__);
    return 0;
}

/*
 * Device interface
 *
 * int setup(struct resource *);
 * int enable();
 * int disable();
 * int poll();
 * int interrupt(int);
 * int num_interfaces();
 * struct generic_interface * get_interface(int);
 */

static int
device_setup_impl(struct device_interface *di, struct s3c2410_lcd *device,
                struct resource *resources)
{
//    int have_s3 = 0;

    for (; resources ; resources = resources->next){
        if (resources->type == BUS_RESOURCE || resources->type == MEMORY_RESOURCE){
//            if (!have_s3) {
//                device->jbt_space = *resources;
//                have_s3 = 1;
//            }
//            else {
                device->s3c_space = *resources;
//                break;
//            }
        }
    }

    device->fb.device = device;
    device->fb.ops = fb_ops;

    /* TODO: clear interrupts and mask, LUTs, unsued registers, framebuffer addr */
    return 1;
}

static int
device_enable_impl(struct device_interface *di, struct s3c2410_lcd *device)
{

#if 0
    printf("lcdcon1 was : %08lx\n", lcdcon1_read());
    printf("lcdcon2 was : %08lx\n", lcdcon2_read());
    printf("lcdcon3 was : %08lx\n", lcdcon3_read());
    printf("lcdcon4 was : %08lx\n", lcdcon4_read());
    printf("lcdcon5 was : %08lx\n\n", lcdcon5_read());


    lcdcon2_write (
        LCDCON2_VSPW(0x1) |     /* 2 */
        LCDCON2_VFPD(0xf) |     /* 16 */
        LCDCON2_LINEVAL(639) |  /* 640 */
        LCDCON2_VBPD(0x1)       /* 2 */
    );

    lcdcon3_write (
        LCDCON3_HFPD(0x67) |    /* 104 */
        LCDCON3_HOZVAL(479) |   /* 480 */
        LCDCON3_HBPD(0x7)       /* 8 */
    );

    lcdcon4_write (
        LCDCON4_HSPW(0x7) |     /* 8 */
        LCDCON4_MVAL(0x0)
    );

    lcdcon5_write (
        LCDCON5_HWSWP | LCDCON5_PWREN | LCDCON5_INVVFRAM | LCDCON5_INVVLINE |
        LCDCON5_INVVCLK | LCDCON5_FRM565
    );

    lpcsel_write(0x0);

    lcdcon1_write (
        LCDCON1_ENVID |         /* enable the device */
        LCDCON1_BPPMODE(0xc) |  /* 16bpp for TFT */
        LCDCON1_PNRMODE(0x3) |  /* TFT LCD */
        LCDCON1_CLKVAL(0x1)     /* VCLK = HCLK / 4 */
    );


    printf("lcdcon1 is : %08lx\n", lcdcon1_read());
    printf("lcdcon2 is : %08lx\n", lcdcon2_read());
    printf("lcdcon3 is : %08lx\n", lcdcon3_read());
    printf("lcdcon4 is : %08lx\n", lcdcon4_read());
    printf("lcdcon5 is : %08lx\n\n", lcdcon5_read());
#else
    lcdcon1_write(0x00000178);
    lcdcon2_write(0x019fc3c1);
    lcdcon3_write(0x0039df67);
    lcdcon4_write(0x00000007);
    lcdcon5_write(0x0001cf09);
    lpcsel_write(0x0);
#endif

    device->state = STATE_NORMAL;

    dprintf("%s: done\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_lcd *device)
{
    lcdcon1_set_envid(0x0);     /* clear ENVID */

    device->state = STATE_SLEEP;

    dprintf("%s: done\n", __func__);

    return DEVICE_DISABLED;
}

static int
device_interrupt_impl(struct device_interface *si, struct s3c2410_lcd *device, int irq)
{
    dprintf("%s [ lcd]: done\n", __func__);

    return 0;
}

static int
device_poll_impl(struct device_interface *si, struct s3c2410_lcd *device)
{
    dprintf("%s [ lcd]: done\n", __func__);

    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di, struct s3c2410_lcd *device)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di, struct s3c2410_lcd *device, int i)
{
    /* FIXME: This can definately be autogenerated */
    switch (i)
    {
    case 0:
        return (struct generic_interface *) &device->fb;
    default:
        return NULL;
    }
}

