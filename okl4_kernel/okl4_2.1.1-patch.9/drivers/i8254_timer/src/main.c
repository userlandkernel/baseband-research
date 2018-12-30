/*
 * Copyright (c) 2004, National ICT Australia
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

#include <l4/kdebug.h>

#include "i8254_defs.h"
#include "i8254_timer.h"

#if 0
#define dprintf(x...) printf(x)
#else
#define dprintf(x...) do { } while (0/*CONSTCOND*/)
#endif

static
void delay(int delay)
{
    volatile int i;
    for (i = 0; i < delay; i++);
}

static void
reset_device(struct i8254_timer *device)
{
    /* Setup 8254 timer */
    pit_mcr_write(STROBE_SW_MODE);
    delay(1000);
    pit_cnt_0_write(0);
    delay(1);
    pit_cnt_0_write(0);
}

static uint16_t
read_timer(struct i8254_timer *device)
{
    uint16_t value;

    /* Latch the count value */
    pit_mcr_write(0x0);
    delay(1);
    value = pit_cnt_0_read();
    delay(1);
    value |= pit_cnt_0_read() << 8;

    return value;
}

static int
device_setup_impl(struct device_interface *di, struct i8254_timer *device,
                  struct resource *resources)
{
    int i;
    for(i = 0; i < 8; i++){
        switch (resources->type){
        case MEMORY_RESOURCE:
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
        default:
            printf("i8254_timer: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }
    
    device->timer.device = device;
    device->timer.ops = timer_ops;
    device->ticks = 0;

    reset_device(device);
    dprintf("%s: i8254 done.\n", __func__);

    return DEVICE_SUCCESS;
}

static void
timeout(struct i8254_timer *device, uint16_t timeout)
{
    /* Setup the timer */
    pit_mcr_write(STROBE_SW_MODE);
    delay(1);
    pit_cnt_0_write(timeout & 0xff);
    delay(1);
    pit_cnt_0_write(timeout >> 8);
    device->timeout_value = timeout;
}

static int
device_enable_impl(struct device_interface *di, struct i8254_timer *device)
{
    device->state = STATE_TIMEKEEP;
    /*
     * Reset the remaining ticks and keep track of the current
     * number of ticks.
     */

    device->remaining_ticks = 0;
    device->timeout_value = 0;
    device->ticks += read_timer(device);
    timeout(device, TIMER_MAX);
    dprintf("%s: i8254 done.\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct i8254_timer *device)
{
    device->state = STATE_DISABLED;
    reset_device(device);
    dprintf("%s: i8254 done.\n", __func__);
    return DEVICE_DISABLED;
}

static uint64_t
timer_set_tick_frequency_impl(struct timer_interface *ti, 
                              struct i8254_timer *device,
                              uint64_t hz)
{
    return TIMER_RATE;    /* XXX */
}

static uint64_t
timer_get_ticks_impl(struct timer_interface *ti, struct i8254_timer *device)
{
    uint16_t value;
    uint32_t diff;

    /* Latch the count value */
    pit_mcr_write(0x0);
    delay(1);
    value = pit_cnt_0_read();
    delay(1);
    value |= pit_cnt_0_read() << 8;

    /*
     * This code was in the legacy v1 driver.  What does it do???
     * I would have assumed that the current tick is the last time   
     * tick value + difference where difference is
     *
     * initial timeout value - current value in timer
     * In any case the code seems to work, so I will leave it in 
     * there for now ..
     *
     * -gl
     */
    if (value <= device->timeout_value)
        diff = device->timeout_value - value;
    else
        diff = (TIMER_MAX + 1) + device->timeout_value - value;

    dprintf("ticks 0x%llx diff 0x%llx total 0x%llx\n", 
        device->ticks, (uint64_t)diff,
        device->ticks + diff);

    return device->ticks + diff;
}

static uint64_t
timer_get_tick_frequency_impl(struct timer_interface *ti, 
                              struct i8254_timer *device)
{
    return TIMER_RATE;    /* XXX */
}

static void
timer_set_ticks_impl(struct timer_interface *ti, struct i8254_timer *device,
                     uint64_t ticks)
{
    device->ticks = ticks;
}

static int
timer_timeout_impl(struct timer_interface *ti, struct i8254_timer *device,
                   uint64_t my_timeout)
{
    int64_t u6 = timer_get_ticks_impl(ti, device);
    int64_t next = my_timeout - u6;

    if (next < 0)  {
        dprintf("%s: expired\n", __func__);
        return 1;
    }

    if (next < TIMER_MAX) {
        /*
         * Account for the currently programmed ticks.
         */
        device->ticks += TIMER_MAX - read_timer(device);
        timeout(device, next);
    }
#if 0
    /*
     * this is in the s3c2410 driver, but why?  Next variable should
     * always be less than TIMER_MAX, and doesn't matter if there is
     * a timer override,  because v2 framework takes care of this for us
     * and only programs the head of the timer list? -gl
     */
    } else if (device->remaining_ticks) /* timer override */
        timeout(device, next);
#endif

    device->remaining_ticks = next;

    return 0;
}

static int
device_interrupt_impl(struct device_interface *di,
                      struct i8254_timer *device,
                      int irq)
{
    uint16_t my_timeout;
    int expired = 0;

    dprintf("%s: i8254 in interrupt.\n", __func__);
    /*
     * Current one shot timer stored in the remaining ticks in relative
     * time.  This is set in the device_timeout_impl().
     *
     * The timeout can be larger or smaller than the 16bit counter.
     * If smaller, it will fit into the decrement register.  Otherwise
     * we program TIMER_MAX.
     *
     * On interrupt we check remaining ticks.  If not set then it is idle
     * so we increase tick count with TIMER_MAX.  If smaller than it must
     * have been programmed at some point and the number of ticks has elapsed.
     * Otherwise decrement with TIMER_MAX.  If it falls below TIMER_MAX then
     * use remaining_ticks as the timeout.
     *
     * This piece of code uses the same logic as the s3c2410 timer, perhaps
     * this stuff should be auto-generated.
     *
     * -gl
     */

    my_timeout = TIMER_MAX;

    if (device->remaining_ticks) {
        if (device->remaining_ticks < TIMER_MAX) {
            device->ticks += device->remaining_ticks;
            device->remaining_ticks = 0;
            expired = 1;
        } else {
            device->remaining_ticks -= TIMER_MAX + 1;
            if (device->remaining_ticks < 0)
                printf("%s: error: remaining ticks < 0\n", __func__);
            if (device->remaining_ticks < TIMER_MAX)
                my_timeout = device->remaining_ticks;
            device->ticks += TIMER_MAX + 1;
        }
    } else {
        device->ticks += TIMER_MAX + 1;
    }

    timeout(device, my_timeout);
    dprintf("%s: i8254 done expired = %d.\n", __func__, expired);
    return expired;
}

static int
device_num_interfaces_impl(struct device_interface *di,
                          struct i8254_timer *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct i8254_timer *device,
                          int interface)
{
    return (struct generic_interface *)&device->timer;
}

static int
device_poll_impl(struct device_interface *di, struct i8254_timer *device)
{
    return 0;
}

