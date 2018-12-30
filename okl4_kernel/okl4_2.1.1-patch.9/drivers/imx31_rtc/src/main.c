/*
 * Copyright (c) 2005, National ICT Australia
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

#include "imx31.h"
#include "imx31_rtc.h"
#include "imx31_util.h"

#include <l4/kdebug.h>

#define dprintf(arg...) printf(arg)

static uint64_t
rtc_get_time_impl(struct rtc_interface *rtci, struct imx31_rtc *device)
{
    struct rtc_time tm;
    uint64_t unix_time;

    tm.days = dayr_get_days();
    tm.hour = hourmin_get_hour();
    tm.min  = hourmin_get_minutes();
    tm.sec  = seconds_get_seconds();

    /* Convert from imx31 rtc internal representation and unix time. */
    rtc_to_unix(&unix_time, &tm);

    return unix_time;
}

static void
rtc_set_time_impl(struct rtc_interface *rtci, struct imx31_rtc *device,
                  uint64_t time)
{
    struct rtc_time tm;

    /* Convert from unix time to imx31 rtc internal representation. */
    unix_to_rtc(time, &tm);

    dayr_set_days(tm.days);
    hourmin_set_hour(tm.hour);
    hourmin_set_minutes(tm.min);
    seconds_set_seconds(tm.sec);
}

static uint64_t
rtc_get_alarm_impl(struct rtc_interface *ti,
                   struct imx31_rtc *device)
{
    struct rtc_time tm;
    uint64_t unix_time;

    tm.days = dayalarm_get_daysal();
    tm.hour = alrm_hm_get_hour();
    tm.min  = alrm_hm_get_minutes();
    tm.sec  = alrm_sec_get_seconds();

    rtc_to_unix(&unix_time, &tm);

    return unix_time;
}

static int
rtc_set_alarm_impl(struct rtc_interface *ti, 
                   struct imx31_rtc *device, uint64_t time)
{
    struct rtc_time tm;
    uint64_t curr_time;

    unix_to_rtc(time, &tm);

    dayalarm_set_daysal(tm.days);
    alrm_hm_set_hour(tm.hour);
    alrm_hm_set_minutes(tm.min);
    alrm_sec_set_seconds(tm.sec);

    /* Has alarm expired? */
    curr_time = rtc_get_time_impl(ti, device);
    if(time < curr_time) {
        return 1;
     }

    /* Enable alarm interrupt. Disabled in interrupt routine. */
    rtcienr_set_alm(0x1);

    return 0;
}

static void
rtc_cancel_alarm_impl(struct rtc_interface *ti, struct imx31_rtc *device)
{
    /* Disabled alarm interrupt. */
    rtcienr_set_alm(0x0);
}

static int
device_setup_impl(struct device_interface *di, struct imx31_rtc *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("imx31_rtc: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("imx31_rtc: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }

    device->rtc.ops = rtc_ops;
    device->rtc.device = device;

    device->state = STATE_DISABLED;

    return DEVICE_SUCCESS;
}

static int
device_enable_impl(struct device_interface *di, struct imx31_rtc *device)
{

    dprintf("%s Start\n", __func__);

    /* Enable real-time clock */
    rtcctl_set_en(0x1);

    dprintf("%s Done\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct imx31_rtc *device)
{
    /* Disable alarm interrupt. */
    rtcienr_set_alm(0x0);

    /* Disable real-time clock */
    rtcctl_set_en(0x0);

    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct imx31_rtc *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, 
                      struct imx31_rtc *device,
                      int irq)
{
   /* If an alarm occured acknowledge alarm and disable alarm interrupt. */
    if (rtcisr_get_alm()) {
        rtcisr_set_alm(0x1);
        rtcienr_set_alm(0x0);
    }

    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct imx31_rtc *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct imx31_rtc *device,
                          int interface)
{
    return (struct generic_interface *)&device->rtc;
}

