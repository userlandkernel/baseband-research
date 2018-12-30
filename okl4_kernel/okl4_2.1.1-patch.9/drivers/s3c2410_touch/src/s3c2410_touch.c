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

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <util/format_bin.h>

#include <l4/kdebug.h>
#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4/misc.h>

#include <vtimer/timer.h>

#include <iguana/memsection.h>    /* XXX - nuke when GPIO move out -gl */
#include <iguana/hardware.h>    /* XXX - nuke when GPIO move out -gl */

#include "s3c2410_touch.h"

static int pen_press;
static int use_sleep;

/*
 * XXX probably make configurable? -gl
 */
struct s3c2410_ts_info {
    int delay;
    int presc;
    int oversampling_shift;
} ts_info = { 10000, 49, 2 };

struct press_info {
    int x;
    int y;
    int pressure;
};

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while(0/*CONSTCOND*/)
#endif

typedef int (*tm_callback_t)(struct device_interface*);
void timeout_request(struct device_interface *device, uintptr_t nanosecs, tm_callback_t call);

static void s3c2410_touch_config_ts_gpio(struct s3c2410_touch *device);

/* Simple delay function for scanning */
/* XXX makes use of L4 primatives */
static void delay(struct s3c2410_touch *device, uintptr_t time)
{
#define TIMER_MASK    0x8000        // XXX
    uintptr_t mask;

    L4_Set_NotifyMask(TIMER_MASK);

    if (use_sleep) {
        // delay in uints of 10 microsecs
        timeout_request(&device->device, MICROSECS(time*10), NULL);
        L4_WaitNotify(&mask);
    } else {
        volatile int i;
        for(i = 0; i < time; i++) {
            L4_Yield();
        }
    }
    L4_Set_NotifyMask(~0);

#undef TIMER_MASK
}

/*
 * XXX HACK XXX
 *
 * Should move to GPIO server -gl
 */
#define S3C2410_GPIO_PHYS      (0x56000000)

#define S3C2410_GPIO_BANKG     (32 * 6)
#define S3C2410_GPIO(bank, offset)    ((bank) + (offset))
#define S3C2410_GPIO_OFFSET(pin)      ((pin) & 31)

#define S3C2410_GPG12         S3C2410_GPIO(S3C2410_GPIO_BANKG, 12)
#define S3C2410_GPG13         S3C2410_GPIO(S3C2410_GPIO_BANKG, 13)
#define S3C2410_GPG14         S3C2410_GPIO(S3C2410_GPIO_BANKG, 14)
#define S3C2410_GPG15         S3C2410_GPIO(S3C2410_GPIO_BANKG, 15)

#define S3C2410_GPG12_XMON    (0x03 << 24)
#define S3C2410_GPG13_nXPON   (0x03 << 26)
#define S3C2410_GPG14_YMON    (0x03 << 28)
#define S3C2410_GPG15_nYPON   (0x03 << 30)

static void
s3c2410_touch_config_ts_gpio(struct s3c2410_touch *device)
{
    uintptr_t ms, p_ms;
    uint32_t mask, cfg;
    volatile uint32_t *addr;
    (void)device;

    /* XXX what to do if fail? -gl */
    ms = memsection_create_user(0x1000, &p_ms);
    hardware_back_memsection(ms, S3C2410_GPIO_PHYS, L4_IOMemory);
    addr = (volatile uint32_t *)(p_ms + 0x60/*bankg*/);

#define CONFIG(pin, func)                     \
    mask = 3 << (S3C2410_GPIO_OFFSET(pin) * 2); \
    cfg = (*addr & ~mask) | func;             \
    *addr = cfg;

    CONFIG(S3C2410_GPG12, S3C2410_GPG12_XMON);
    CONFIG(S3C2410_GPG13, S3C2410_GPG13_nXPON);
    CONFIG(S3C2410_GPG14, S3C2410_GPG14_YMON);
    CONFIG(S3C2410_GPG15, S3C2410_GPG15_nYPON);

#undef CONFIG
}

static void touch_enable(struct s3c2410_touch *device)
{
    adccon_set_stdbm(0x0);
    delay(device, 1);
}

static void touch_disable(struct s3c2410_touch *device)
{
    adccon_set_stdbm(0x1);
}

/*
 * Connect the touchscreen so the output is wired up to the touchscreen
 * and not the external source.  In theory we can make this more generic
 * since this is just an ADC converter but will do for now.
 */
static void touch_ts_connect(struct s3c2410_touch *device)
{
    adctsc_set_ym_sen(0x1);   /* GND */
    adctsc_set_yp_sen(0x1);   /* AIN[5] */
    adctsc_set_xm_sen(0x0);   /* EXT voltage */
    adctsc_set_xp_sen(0x1);   /* AIN[7] */
    adctsc_set_pull_up(0x0);   /* Pull-up disable */

    /* 
     * Set the GPIO pins.
     * 
     * XXX move to GPIO server -gl
     */
    s3c2410_touch_config_ts_gpio(device);
}

static void touch_irq_enable(struct s3c2410_touch *device)
{
    adctsc_set_xy_pst(0);
    adctsc_set_auto_pst(0);

    adctsc_set_xy_pst(0x3);    /* 11 = waiting for interrupt mode */
}

static void touch_irq_disable(struct s3c2410_touch *device)
{
    adctsc_set_xy_pst(0x00);    /* 0x0 = no operation */
}

static int touch_pen_pressed(struct s3c2410_touch *device)
{
    /*
     * If the x updown and the y updown are both down then we assume
     * the stylus is down.
     */
    return (adcdat0_get_updown() == 0x0/*down*/ &&
        adcdat1_get_updown() == 0x0/*down*/);
}

static void touch_read_adc(struct s3c2410_touch *device, uint32_t *x, uint32_t *y)
{
    *x = adcdat0_get_xpdata();
    *y = adcdat1_get_ypdata();

    dprintf(" - x = %ld, y = %ld\n", *x, *y);
}

static void
touch_set_prescale(struct s3c2410_touch *device)
{
    if ((ts_info.presc & 0xff) > 0) {
        adccon_set_prscen(0x1);
        adccon_set_prscvl(ts_info.presc & 0xff);
    }
}

static void
touch_set_delay(struct s3c2410_touch *device)
{
    if ((ts_info.delay & 0xffff) > 0)
        adcdly_set_delay(ts_info.delay & 0xffff);
}

static int
device_setup_impl(struct device_interface *di, struct s3c2410_touch *device,
                  struct resource *resources)
{
    use_sleep = 0;

    for (/*null*/; resources && (resources->type != BUS_RESOURCE && resources->type != MEMORY_RESOURCE); 
        resources = resources->next);

    if (!resources)
        return (0);

    device->main = *resources;

    device->input.device = device;
    device->input.ops = stream_ops;

    touch_irq_disable(device);
    touch_disable(device);
    touch_ts_connect(device);

    touch_set_prescale(device);
    touch_set_delay(device);

    // wait for A/D converter to stabilise
    delay(device, 100);

    pen_press = 0;

    dprintf("%s: done.\n", __func__);

    return 0;
}

static int
device_enable_impl(struct device_interface *di, struct s3c2410_touch *device)
{
    touch_irq_enable(device);

    device->state = STATE_ENABLED;

    touch_enable(device);

    use_sleep = 1;

    dprintf("%s: done.\n", __func__);

    return DEVICE_ENABLED;
}

static void
packet_append(struct s3c2410_touch *device, int x, int y, int p)
{
    struct stream_interface *si = &device->input;
    struct stream_pkt *packet = stream_get_head(si);
    struct press_info *pi;

    if (packet == NULL) {
        dprintf("%s: no packet, returning.\n", __func__);
        return;
    }

    pi = (struct press_info*)packet->data;

    pi->x = x;
    pi->y = y;
    pi->pressure = p;
    packet->xferred = sizeof(struct press_info);

    packet = stream_switch_head(si);
}

static int do_input_work(struct s3c2410_touch *device);

static int
touch_timer_callback(struct device_interface* di)
{
    struct s3c2410_touch *device = (struct s3c2410_touch*)di->device;
    return do_input_work(device);
}

static int
do_input_work(struct s3c2410_touch *device)
{
    uint32_t x, y;
    int pressed;

    dprintf("%s: called.\n", __func__);

    pressed = touch_pen_pressed(device);
    /*
     * If state is enabled then we have pressed the touchpad for the
     * first time.  Otherwise, we must have gotten an INT_TC.
     */
    if (device->state == STATE_ENABLED && pressed) {
        dprintf(" -- pen-down\n");
        device->state = STATE_PENDOWN;
    } else {
        if (pressed) {
            touch_read_adc(device, &x, &y);
            device->x += x;
            device->y += y;
            device->samplecnt++;
        } else {
            char *state;
            /* assume pen up */
            dprintf(" -- pen-up\n");
            switch (device->state) {
                case STATE_ENABLED:
                    state = "enabled";
                    break;
                case STATE_DISABLED:
                    state = "disabled";
                    break;
                case STATE_PENDOWN:
                    state = "pendown";
                    break;
                default:
                    state = "unknown";
            }
            dprintf(" -- state %s pressed %d cnt %lu\n", 
                state, pressed, device->samplecnt);
            device->x = device->y = device->samplecnt = 0;
            device->state = STATE_ENABLED;
            packet_append(device, 0, 0, 0);
            touch_irq_enable(device);
            return 0;
        }
    }

    /*
     * Check to see if we have sampled enough.  If it is less than
     * than the oversampling shift, then setup the ADC for the next
     * sample.
     */
    if (device->samplecnt < (1 << ts_info.oversampling_shift)) {
        /*
         * Program the adctsc register to use auto conversion whe
         */
        adctsc_set_ym_sen(0x1);   /* GND */
        adctsc_set_yp_sen(0x1);   /* AIN[5] */
        adctsc_set_xm_sen(0x0);   /* EXT voltage */
        adctsc_set_xp_sen(0x1);   /* AIN[7] */
        adctsc_set_pull_up(0x0);   /* Pull-up disable */
        adctsc_set_xy_pst(0x0);   /* no operation */
        adctsc_set_auto_pst(0x1); /* auto conversion */
    
        adccon_set_enable_start(0x1);   /* enable the conversion */
        /* now, wait for the INT_ADC */
    } else {
        /* 
         * Sampled enough already.  Append the packet to
         * the client.
         */
         device->x /= device->samplecnt;
         device->y /= device->samplecnt;

         dprintf("Sending to client - x = %ld, y = %ld\n", 
             device->x, device->y);
         packet_append(device, device->x, device->y, 1);
         device->samplecnt = device->x = device->y = 0;
         device->state = STATE_ENABLED;

        timeout_request(&device->device, MILLISECS(100), touch_timer_callback);
         touch_irq_enable(device);
         return 1;
    }
    return 0;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_touch *device)
{
    touch_irq_disable(device);
    touch_disable(device);

    device->state = STATE_DISABLED;

    use_sleep = 0;
    dprintf("%s: done.\n", __func__);
    return DEVICE_DISABLED;
}

static int
stream_sync_impl(struct stream_interface *si, struct s3c2410_touch *device)
{
    dprintf("%s: done.\n", __func__);
    return 0;
}

extern volatile int touch_sched_flip;

static int
device_interrupt_impl(struct device_interface *di, struct s3c2410_touch *device, int irq)
{
    dprintf("%s: invoked.\n", __func__ );

    if (touch_sched_flip) {
        //printf("touch_sched_flip set before device_interrupt_impl called\n");
        (void)stream_switch_head(&device->input);
    }

    /*
     * do_input_work() will handle both INT_TC (pendown) and INT_ADC
     * (conversion done) interrupts.
     */
    int ret = do_input_work(device);

    dprintf("%s: done.\n", __func__);

    return ret;
}


static int
device_poll_impl(struct device_interface *di, struct s3c2410_touch *device)
{
    dprintf("%s: done.\n", __func__);
    return 0;
}

/*
 * FIXME: From here on is code that will be auto-generated in later versions.
 */
/* FIXME: This can be autogenerated! */
static int
device_num_interfaces_impl(struct device_interface *di, struct s3c2410_touch *device)
{
    return 1;
}

/* FIXME: This can definately be autogenerated */
static struct generic_interface *
device_get_interface_impl(struct device_interface *di, struct s3c2410_touch *device, int i)
{
    switch (i) {
        case 0:
            return (struct generic_interface *) &device->input;
        default:
            return NULL;
    }
}

