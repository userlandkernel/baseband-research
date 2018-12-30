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

#include "s3c2410.h"
#include "s3c2410_rtc.h"
#include "s3c2410_util.h"

#include <l4/kdebug.h>

#define dprintf(arg...) printf(arg)

static uint64_t
rtc_get_time_impl(struct rtc_interface *rtci, struct s3c2410_rtc *device)
{
    uint64_t unix_time;
    struct rtc_time tm;

    int reads = 0;
    tm.sec = 0;

    /* If BCDSEC is zero then we read during an update, reread. */
    while (tm.sec == 0 && reads <= 1) {
        tm.sec  = BCD2BIN(bcdsec_get_secdata());
        tm.min  = BCD2BIN(bcdmin_get_mindata());
        tm.hour = BCD2BIN(bcdhour_get_hourdata());
        tm.date = BCD2BIN(bcddate_get_datedata());
        tm.mon  = BCD2BIN(bcdmon_get_mondata());
        /* s3c410 rtc only has two digits for year, assumes 21st century. */
        tm.year = bcdyear_get_yeardata();

        reads++;
    }

    if (rtc_to_unix(&unix_time, &tm) != 0) {
        printf("%s: WARNING RTC returned invalid time value\n", __func__);
    }

    //printf("%s: time is: unix_time=%llu ... year=%d, mon=%d, date=%d, hour=%d, min=%d, sec=%d\n", __func__, unix_time, tm.year, tm.mon, tm.date, tm.hour, tm.min, tm.sec);


    return unix_time;
}

static void
rtc_set_time_impl(struct rtc_interface *rtci, struct s3c2410_rtc *device,
                  uint64_t time)
{
    struct rtc_time tm;

    /* Convert time back into s3c2410 rtc format and write out. */
    if (unix_to_rtc(time, &tm) != 0) {
        printf("%s: WARNING RTC does not support given time value\n", __func__);
    }

    bcdsec_set_secdata(BIN2BCD(tm.sec));
    bcdmin_set_mindata(BIN2BCD(tm.min));
    bcdhour_set_hourdata(BIN2BCD(tm.hour));
    bcddate_set_datedata(BIN2BCD(tm.date));
    bcdmon_set_mondata(BIN2BCD(tm.mon));
    bcdyear_set_yeardata(tm.year);
}

static uint64_t
rtc_get_alarm_impl(struct rtc_interface *ti,
                   struct s3c2410_rtc *device)
{
    uint64_t unix_time;
    struct rtc_time tm;

    tm.sec  = BCD2BIN(almsec_get_secdata());
    tm.min  = BCD2BIN(almmin_get_mindata());
    tm.hour = BCD2BIN(almhour_get_hourdata());
    tm.date = BCD2BIN(almdate_get_datedata());
    tm.mon  = BCD2BIN(almmon_get_mondata());
    tm.year = almyear_get_yeardata();

    if (rtc_to_unix(&unix_time, &tm) != 0) {
        printf("%s: WARNING RTC returned invalid time value\n", __func__);
    }

    return unix_time;
}

static int
rtc_set_alarm_impl(struct rtc_interface *ti,
                   struct s3c2410_rtc *device, uint64_t time)
{
    uint64_t curr_time;
    struct rtc_time tm;

    if (unix_to_rtc(time, &tm) != 0) {
        printf("%s: WARNING RTC does not support given time value\n", __func__);
    }

    almsec_set_secdata(BIN2BCD(tm.sec));
    almmin_set_mindata(BIN2BCD(tm.min));
    almhour_set_hourdata(BIN2BCD(tm.hour));
    almdate_set_datedata(BIN2BCD(tm.date));
    almmon_set_mondata(BIN2BCD(tm.mon));
    almyear_set_yeardata(tm.year);

    rtcalm_set_secen(0x1);
    rtcalm_set_minen(0x1);
    rtcalm_set_houren(0x1);
    rtcalm_set_dateen(0x1);
    rtcalm_set_monren(0x1);
    rtcalm_set_yearen(0x1);
    rtcalm_set_almen(0x1);

    /* Has alarm expired? */
    curr_time = rtc_get_time_impl(ti, device);
    if (time < curr_time) {
        printf("%s: Alarm expired\n", __func__);
        return 1;
    }

    return 0;
}

static void
rtc_cancel_alarm_impl(struct rtc_interface *ti, struct s3c2410_rtc *device)
{
    /* Disable alarm. */
    rtcalm_set_almen(0x0);
}

static int
device_setup_impl(struct device_interface *di, struct s3c2410_rtc *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("s3c2410_rtc: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("s3c2410_rtc: Invalid resource type %d!\n", resources->type);
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
device_enable_impl(struct device_interface *di, struct s3c2410_rtc *device)
{
    /* Enables read/write to RTC. */
    rtccon_set_rtcen(0x1);

    /* Remove BCD clock count reset and clock select bits.*/
    rtccon_set_clkrst(0x0);
    rtccon_set_cntsel(0x0);

    dprintf("%s: done\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_rtc *device)
{
   rtcalm_set_almen(0x0);

    /* Disable read/write to RTC. */
    rtccon_set_rtcen(0x0);

    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct s3c2410_rtc *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, 
                      struct s3c2410_rtc *device,
                      int irq)
{
    printf("%s: Got alarm interrupt", __func__);

    /* Disable alarm if an alarm interrupt occured. This is necessary. If not
     * disabled we could receive unexpected alarms when set_time is called with
     * some time before the current alarm, or when the time rolls over. */
    rtcalm_set_almen(0x0);

    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct s3c2410_rtc *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct s3c2410_rtc *device,
                          int interface)
{
    return (struct generic_interface *)&device->rtc;
}

