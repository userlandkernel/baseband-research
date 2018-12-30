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

#include "mc146818_rtc.h"
#include "mc146818_util.h"

#include <l4/kdebug.h>

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0)
#endif

#if 0
static struct alm_time{
    int date;
    int mon;
    int year;
}alarm_time;
#endif

#define SECONDS 0x0
#define MINUTES 0x2
#define HOURS 0x4
#define DATEOFMONTH 0x7
#define MONTH 0x8
#define YEAR 0x9
#define ALARM_SECONDS 0x1
#define ALARM_MINUTES 0x3
#define ALARM_HOURS   0x5
#define REG_A   0xa
#define REG_B   0xb
#define REG_C   0xc
#define REG_D   0xd

#define SET_BIT 7
#define AIE_BIT 5
#define UIE_BIT 4
#define DM_BIT 2
#define HR_BIT 1
#define IRQF 0x80
#define AF 0x20
#define UF 0x10

#define UIP 0x80

#define SET 1
#define UNSET 0

static inline uint8_t
read_reg(struct mc146818_rtc *device, unsigned int addr)
{
    address_write(addr);
    return value_read();
}

static inline void
write_reg(struct mc146818_rtc *device, unsigned char addr, uint8_t value)
{
    address_write(addr);
    value_write(value);
}

static inline void
set_reg_bit(struct mc146818_rtc *device, int reg, int pos, int val)
{
    uint8_t tmp = read_reg(device, reg);
    tmp &= ~ (0x1 << pos);
    tmp |= ((val & 0x1) << pos);
    write_reg(device, reg, tmp);
}

static uint64_t
rtc_get_time_impl(struct rtc_interface *rtci, struct mc146818_rtc *device)
{
    struct rtc_time tm;
    uint64_t unix_time;

    /* If we get pre-empted here, we may get dropped into an update and read garbage */
    /* Read the time from the chip. If the UIP bit is set before any reads,
     * then wait for it to clear and restart the read. */
    while(1) {
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.year = read_reg(device, YEAR);
            dprintf("Year=%d, ",tm.year);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.mon = read_reg(device, MONTH);
            dprintf("Month=%d, ",tm.mon);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.date = read_reg(device, DATEOFMONTH);
            dprintf("DayOfMonth=%d, ",tm.date);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.hour = read_reg(device, HOURS);
            dprintf("Hour=%d, ",tm.hour);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.min = read_reg(device, MINUTES);
            dprintf("Min=%d, ",tm.min);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.sec = read_reg(device, SECONDS);
            dprintf("Second=%d\n",tm.sec);
            /* We successfully read the time, break */
            break;
        }
    }

    if (rtc_to_unix(&unix_time, &tm) != 0) {
        printf("%s: WARNING RTC returned invalid time value\n", __func__);
        return 0;
    }
        
    return unix_time;
}

static void
rtc_set_time_impl(struct rtc_interface *rtci, struct mc146818_rtc *device,
                  uint64_t time)
{
    struct rtc_time tm;

    if (unix_to_rtc(time, &tm) != 0) {
        printf("%s: WARNING RTC does not support given time value\n", __func__);
    }

    set_reg_bit(device, REG_B, SET_BIT, SET);

    write_reg(device, SECONDS, tm.sec);
    write_reg(device, MINUTES, tm.min);
    write_reg(device, HOURS, tm.hour);
    write_reg(device, DATEOFMONTH, tm.date);
    write_reg(device, MONTH, tm.mon);
    write_reg(device, YEAR, tm.year);

    set_reg_bit(device, REG_B, SET_BIT, UNSET);
}

static uint64_t
rtc_get_alarm_impl(struct rtc_interface *ti,
                   struct mc146818_rtc *device)
{
    /*
     * Alarms don't work as pistachio uses IRQ8
     */
#if 0
    uint64_t unix_time;
    struct rtc_time tm;

    /* Read the time from the chip. If the UIP bit is set before any reads,
     * then wait for it to clear and restart the read. */
    while(1) {
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.hour = read_reg(device, ALARM_HOURS);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.min = read_reg(device, ALARM_MINUTES);
        }
        if (read_reg(device, REG_A) & UIP){
            continue;
        } else {
            tm.sec = read_reg(device, ALARM_SECONDS);
            /* We successfully read the time, break */
            break;
        }
    }
    tm.year = alarm_time.year;
    tm.mon = alarm_time.mon;
    tm.date = alarm_time.date;
    
    if (rtc_to_unix(&unix_time, &tm) != 0) {
        printf("%s: WARNING RTC returned invalid time value\n", __func__);
        return 0;
    }
    return unix_time;
#else
    return 0;
#endif  
}

static int
rtc_set_alarm_impl(struct rtc_interface *ti, 
                   struct mc146818_rtc *device, uint64_t time)
{
    /*
     * Alarms don't work as pistachio uses IRQ 8
     */
#if 0
    struct rtc_time tm;

    if (unix_to_rtc(time, &tm) != 0) {
        printf("%s: WARNING RTC does not support given time value\n", __func__);
    }

    alarm_time.year = tm.year;
    alarm_time.mon = tm.mon;
    alarm_time.date = tm.date;
    while(1){
        if (read_reg(device, REG_A) & UIP){
            continue;
        }
        write_reg(device, ALARM_HOURS, tm.hour);
        write_reg(device, ALARM_MINUTES, tm.min);
        write_reg(device, ALARM_SECONDS, tm.sec);
        /* Make sure sure we can read back what we wrote */
        if (read_reg(device, ALARM_HOURS) != tm.hour ||
            read_reg(device, ALARM_MINUTES) != tm.min ||
            read_reg(device, ALARM_SECONDS) != tm.sec){
            continue;
        } else {
            break;
        }
    }

    set_reg_bit(device, REG_B, AIE_BIT, SET);
#endif
    return 0;
}

static void
rtc_cancel_alarm_impl(struct rtc_interface *ti, struct mc146818_rtc *device)
{
    set_reg_bit(device, REG_B, AIE_BIT, UNSET);
}

static int
device_setup_impl(struct device_interface *di, struct mc146818_rtc *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("mc146818_rtc: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("mc146818_rtc: Invalid resource type %d!\n", resources->type);
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
device_enable_impl(struct device_interface *di, struct mc146818_rtc *device)
{
    set_reg_bit(device, REG_B, DM_BIT, SET);
    set_reg_bit(device, REG_B, HR_BIT, SET);

    /* XXX : set up interrupt to avoid busy waiting */
    
    while (!(read_reg(device, REG_A) & UIP))
        continue;
    while ((read_reg(device, REG_A) & UIP))
        continue;

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct mc146818_rtc *device)
{
    set_reg_bit(device, REG_B, AIE_BIT, UNSET);
    
    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct mc146818_rtc *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, 
                      struct mc146818_rtc *device,
                      int irq)
{
/*
 * This won't work as pistachio uses IRQ8 and so we never get it
 */

#if 0
    uint8_t tmp;
    struct alm_time tm;
        
    tmp = read_reg(device, REG_C);

    /*
     * Risk if we get pre-empted here that we will read garbage.
     */
    if (tmp & AF) {
        while(1) {
            if (read_reg(device, REG_A) & UIP){
                continue;
            } else {
                tm.year = read_reg(device, YEAR);
            }
            if (read_reg(device, REG_A) & UIP){
                continue;
            } else {
                tm.mon = read_reg(device, MONTH);
            }
            if (read_reg(device, REG_A) & UIP){
                continue;
            } else {
                tm.date = read_reg(device, DATEOFMONTH);
                break;
            }
        }
        
        if (tm.date == alarm_time.date &&
            tm.mon == alarm_time.mon &&
            tm.year == alarm_time.year){
            set_reg_bit(device, REG_B, AIE_BIT, UNSET);
        } else {
            /* wrong day */
            return 1;
        }
    } else {
        /* wrong interrupt type */
        return 1;
    }
#endif
    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct mc146818_rtc *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct mc146818_rtc *device,
                          int interface)
{
    return (struct generic_interface *)&device->rtc;
}
