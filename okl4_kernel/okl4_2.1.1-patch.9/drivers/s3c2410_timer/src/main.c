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

/*
 * The S3C2410 has 5 timers, timer 0 to timer 4.  Kernel uses timer 4, we
 * shall use timer 0 and 1.
 */

/* Timer specific section */
#include "s3c2410.h"
#include "s3c2410_timer.h"
#include <l4/kdebug.h>

#define TIME_MAX 0xFFFF

#define TIMER_SYSTEM 0x1
#define TIMER_USER   0x2

#if 0
#define dprintf(x...) printf(x)
#else
#define dprintf(x...)
#endif

static void
disable_timeout(struct s3c2410_timer *device)
{
    tcon_set_timer1_enable(0x0);
    tcon_set_timer2_enable(0x0);
}

static void
set_timeout(struct s3c2410_timer *device, uint16_t timeout, int timer, 
            int reload)
{

#if 0
    uint32_t t32 = timeout;
    printf("\t%08lx\n", t32);
    uint32_t orig = tcon_read();
    printf("orig tcon was %08lx\n", orig);

    tcntb1_write(0xffff);

    tcon_set_timer1_enable(0);
    tcon_set_timer1_manual_update(1);
    tcon_set_timer1_autoreload(1);
    tcon_set_timer1_manual_update(0);

    tcon_set_timer1_enable(1);

    int i;
    int a[10];
    for (i = 0; i < 10; i++){
        a[i] = tcnto1_read();
    }
    for (i = 0; i < 10; i++){
        printf("%d %08x\n",i, a[i]);
    }

    return;
#endif

    switch (timer) {
        case 2:
            printf("AGH!\n");
            tcntb2_write(timeout & TIME_MAX);
            tcon_set_timer2_manual_update(0x1);
            /* 
             * The s3c2410 manual says the manual update bit has to 
             * be cleared at next writing.
             */
            tcon_set_timer2_manual_update(0x0);
            if (reload)
                tcon_set_timer2_autoreload(0x1);
            else
                tcon_set_timer2_autoreload(0x0);
            tcon_set_timer2_enable(0x1);
            break;
        case 1:
            tcntb1_write(timeout);

            // This sucks, as we may loose time, but doesnt seem to work otherwise?
            tcon_set_timer1_enable(0);

            /* 
             * The s3c2410 manual says the manual update bit has to 
             * be cleared at next writing.
             */
            tcon_set_timer1_manual_update(1);
            if (reload)
                tcon_set_timer1_autoreload(1);
            else
                tcon_set_timer1_autoreload(0);

            tcon_set_timer1_manual_update(0);
            tcon_set_timer1_enable(1);           
            break;
        default:
            printf("%s: unknown or unimplemented timer %d\n", __func__, timer);
            break;
    }
}

static int
device_setup_impl(struct device_interface *di, struct s3c2410_timer *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("s3c210_timer: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("s3c2410_timer: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }

    device->timer.device = device;
    device->timer.ops = timer_ops;
    dprintf("Set device_ticks to 0 %p\n", device);
    device->ticks = 0;

    // timers->tcfg0 = 0x0707; // 1/8 to match pistachio
    tcfg0_write(0x0707);
    tcfg1_set_mux1(0x00);
    tcfg1_set_mux2(0x00);

    device->state = STATE_DISABLED;

    return DEVICE_SUCCESS;
}

static int
device_enable_impl(struct device_interface *di, struct s3c2410_timer *device)
{
    /*
     * The interrupt shall be automatically enabled because the
     * the set_timeout() function shall start the timer. -gl
     */
    device->state = STATE_TIMEKEEP;
    device->remaining_ticks = 0;

    set_timeout(device, TIME_MAX, TIMER_SYSTEM, 0/*reload*/);
    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_timer *device)
{
    device->state = STATE_DISABLED;
    disable_timeout(device);
    return DEVICE_DISABLED;
}

static uint64_t
timer_set_tick_frequency_impl(struct timer_interface *ti,
                              struct s3c2410_timer *device,
                              uint64_t hz)
{
    return TIMER_RATE;  /* XXX  */
}

static uint64_t
timer_get_tick_frequency_impl(struct timer_interface *ti,
                              struct s3c2410_timer *device)
{
    return TIMER_RATE;  /* XXX */
}

static uint64_t
timer_get_ticks_impl(struct timer_interface *ti, struct s3c2410_timer *device)
{
    uint32_t ticks, read;
    read = tcnto1_read();
    ticks = TIME_MAX - read;

    return (device->ticks + ticks);
}

static int
timer_timeout_impl(struct timer_interface *ti, struct s3c2410_timer *device,
                   uint64_t timeout)
{
    int64_t u6 = timer_get_ticks_impl(ti, device);
    int64_t next = timeout - u6;


    if (next < 0) {
        printf("expired\n");
        return 1;
    }

    if (next < TIME_MAX) {
        /*
         * Account for the currently programmed ticks.
         */
        device->ticks += TIME_MAX - tcnto1_read();
        set_timeout(device, next, TIMER_SYSTEM, 0/*reload*/);
    } else if (device->remaining_ticks) /* timer override */
        set_timeout(device, next, TIMER_SYSTEM, 0/*reload*/);

    device->remaining_ticks = next;

    return 0;
}

static void
timer_set_ticks_impl(struct timer_interface *ti, struct s3c2410_timer *device,
                     uint64_t ticks)
{
    device->ticks = ticks;
}

static int
device_poll_impl(struct device_interface *di, struct s3c2410_timer *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, 
                      struct s3c2410_timer *device,
                      int irq)
{
    uint16_t timeout = TIME_MAX;
    /*
     * Modus operandi:
     *
     * The current one shot timer is stored in the remaining_ticks
     * value in relative time.  This is set in the device_timeout_impl()
     * function.  This timeout can be either larger than the 16 bit counter
     * or smaller.  In the case it is smaller, it can fit into the 
     * decrementer register.  We read from the observation counter to
     * account for the ticks elapsed and then reprogram the timer
     * right away.  Otherwise we reprogram with the value TIME_MAX.
     * 
     * On the interrupt path, we check the remaining ticks.  If remaining
     * ticks is not set then it is regular idle interrupt so we increase
     * the tick count with TIME_MAX.  Otherwise, we check it.
     *
     * If it is smaller than TIME_MAX it must have been reprogrammed at 
     * some point, and that is the number of ticks elapsed.
     *
     * Otherwise, decrement it with TIME_MAX.  If the decrement falls
     * below TIME_MAX then use the new remaining_ticks as the timeout.
     *
     * -gl
     */
    static int next = 0;
    if (device->remaining_ticks) {
        if (device->remaining_ticks < TIME_MAX) {
            device->ticks += device->remaining_ticks;
            device->remaining_ticks = 0;
            next = 1;
        } else {
            device->remaining_ticks -= TIME_MAX + 1;
            if (device->remaining_ticks < 0) printf("THATS BAD!\n");
            if (device->remaining_ticks < TIME_MAX){
                timeout = device->remaining_ticks;
            }
            device->ticks += TIME_MAX + 1;
        }
    } else {
        device->ticks += TIME_MAX + 1;
    }

    set_timeout(device, timeout, TIMER_SYSTEM, 0/*reload*/);

    if (next){
        next = 0;
        return 1;
    }
    return 0;
}

/* XXX support multiple timer source? -gl */
static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct s3c2410_timer *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct s3c2410_timer *device,
                          int interface)
{
    return (struct generic_interface *)&device->timer;
}
