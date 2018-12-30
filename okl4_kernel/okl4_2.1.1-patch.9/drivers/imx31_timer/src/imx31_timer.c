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
/*
 * Description:   i.MX31 timer driver
 */

#define DBG_ENABLE   0

#if DBG_ENABLE
#define DBG(fmt, va_args...)    printf ("%s: "fmt, __func__, ##va_args)
#else
#define DBG(fmt, va_args...)
#endif

#include <l4/kdebug.h>
#include "imx31_timer.h"
#include "imx31_timer_defs.h"

static uint64_t
timer_get_ticks_impl
(
    struct timer_interface  *ti,
    struct imx31_timer      *device
)
{
    /* XXX what about wrap ? */
    return device->ticks + gptcnt_read();
}

static void
timer_set_ticks_impl
(
    struct timer_interface  *ti,
    struct imx31_timer      *device,
    uint64_t ticks
)
{
    device->ticks = ticks;
}

static uint64_t
timer_get_tick_frequency_impl
(
    struct timer_interface  *ti,
    struct imx31_timer      *device
)
{
    return TIMER_FREQ;
}


static uint64_t
timer_set_tick_frequency_impl
(
    struct timer_interface  *ti,
    struct imx31_timer      *device,
    uint64_t                hz
)
{
    return TIMER_FREQ;
}

static int
timer_timeout_impl
(
    struct timer_interface  *ti,
    struct imx31_timer      *device,
    uint64_t                timeout
)
{
    uint64_t t;
    uint32_t val;

    /* XXX what about wrap? */
    val = gptcnt_read();
    t = device->ticks + val;
    if (timeout < t) {
        device->remaining_ticks = 0;
        DBG("expired\n");
        return 1;
    }
    device->remaining_ticks = timeout - t;
    device->next_rov = MAX_TICKS + 1 - val;
    DBG("timeout 0x%llx\n", timeout);

    /*
     * If the number of ticks can fit the remaining ticks, then 
     * we can simply program the match register.  Otherwise, the rollover
     * interrupt will fix this for us.
     */
    if (device->remaining_ticks < (MAX_TICKS + 1 - val)) {
        /*
         * timeout is relative from "now", so make absolute by adding
         * time elapsed so far.
         */
        gptocr1_write(val + device->remaining_ticks);
        DBG("Set compare val 0x%llx\n", device->remaining_ticks);
        gptir_set_of1ie(0x1);
        /*
         * wrapped while we were trying to program the timer.  That's not
         * cool ...
         */
        if (val > gptcnt_read()) {
            DBG("wrapped and expired\n");
            device->remaining_ticks = 0;
            gptir_set_of1ie(0x0);
            gptsr_set_of1(0x1);
            return 1;
        }
    }
    return 0;
}

static inline void
timer_enable
(
    struct  imx31_timer     *device
)
{
    gptir_set_of1ie(0x0);
    gptir_set_rovie(0x1);
    gptocr1_write(MAX_TICKS);
    gptcr_set_en(EN_ENABLE);
}

static inline void
timer_disable
(
    struct  imx31_timer     *device
)
{
    gptcr_set_en(EN_DISABLE);
    gptir_set_of1ie(0x0);
    gptir_set_rovie(0x0);
}

static void
reset_register_state
(
    struct  imx31_timer     *device
)
{
    gptcr_set_enmode(ENMODE_RESET_AFTER_EN);    /* reset counter after disable -> enable */
    gptcr_set_waiten(0x1);
    gptcr_set_clksrc(CLKSRC_IPG_CLK);           /* use the global clock */
    gptcr_set_frr(FRR_FREERUN);                 /* do not reset after compare event */
    gptsr_write(gptsr_read());
    device->ticks = 0;
}

static int
device_setup_impl
(
    struct device_interface *di,
    struct imx31_timer      *device,
    struct resource         *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++)
    {
        switch (resources->type)
        {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->gpt_space = *resources;
            else
                printf("imx31_timer: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("imx31_timer: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }

    //device->gpt_space    = *resources;
    device->timer.device = device;
    device->timer.ops    = timer_ops;

    reset_register_state(device);
    timer_disable(device);

    DBG("done, timer is disabled\n");
    return DEVICE_SUCCESS;
}

static int
device_enable_impl
(
    struct device_interface *di,
    struct imx31_timer      *device
)
{
    device->state = STATE_ENABLED;
    device->remaining_ticks = 0;

    timer_enable(device);

    DBG("timer is enabled\n");
    return DEVICE_ENABLED;
}

static int
device_disable_impl
(
    struct device_interface *di,
    struct imx31_timer      *device
)
{
    device->state = STATE_DISABLED;

    timer_disable(device);

    DBG("timer is disabled\n");
    return DEVICE_DISABLED;
}

static int
device_poll_impl
(
    struct device_interface *di,
    struct imx31_timer      *device
)
{
    DBG("nothing to do\n");
    return 0;
}

static int
device_interrupt_impl
(
    struct device_interface *di,
    struct imx31_timer      *device,
    int                     irq
)
{
    uint32_t now;
    int intr;

    DBG("imx31.timer got interrupt\n");

    intr = 0;
    now = gptcnt_read();
    
    /*
     * Handle compare match.
     *
     * - nt
     */
    if (gptsr_get_of1()) {
interrupt:
        /*
         * Finished with this timer now, reset the remaining ticks to 0 and
         * turn off the compare interrupt
         */
        gptir_set_of1ie(0x0);
        gptsr_set_of1(0x1);    /* write 1 to clear */
        device->remaining_ticks = 0;
        intr = 1;
    } else if (gptsr_get_rov()) {
        /* rollover */
        DBG("ix31.timer rollover\n");
        gptsr_set_rov(0x1);    /* write 1 to clear */
        device->ticks += MAX_TICKS + 1;
        /*
         * Check if we need to decrement.  The amount to decrement
         * MAX_TICKS + 1 - time-of-program.
         */ 
        if (device->remaining_ticks &&
            device->remaining_ticks > device->next_rov) {
            DBG("decrementing outstanding tick in interrupt path\n");
            device->remaining_ticks -= device->next_rov;
            device->next_rov = MAX_TICKS + 1;
        }
        /* within this tick cycle, set the compare register */
        if (device->remaining_ticks < (MAX_TICKS + 1)) {
            DBG("enabling match register in intr path remaining_ticks "
                "0x%llx\n", device->remaining_ticks);
            /* should be relative from "now" */
            gptocr1_write(now + device->remaining_ticks);
            gptir_set_of1ie(0x1);
            /* check to see if expired already while programming */
            if (now > gptcnt_read())
                goto interrupt;
            intr = 0;
        }
    } else {
        /* can't happen because the compare interrupt has been turned off */
        DBG("imx31.timer: spurious interrupt\n");
    }

    if (intr)
        DBG("timeout occurred\n");
    else
        DBG("timeout not occurred.\n");

    return intr;
}

static int
device_num_interfaces_impl
(
    struct device_interface *di,
    struct imx31_timer      *dev
)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl
(
    struct device_interface *di,
    struct imx31_timer      *device,
    int                     interface
)
{
    return (struct generic_interface *)&device->timer;
}

