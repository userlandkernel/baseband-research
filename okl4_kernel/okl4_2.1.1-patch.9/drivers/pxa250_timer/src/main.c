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

/* Timer specific section */
#include "pxa250_timer.h"
#include "pxa250.h"
#include <l4/kdebug.h>

#define TIME_MAX 0xffffffff

static int
device_setup_impl(struct device_interface *di, struct pxa250_timer *device,
                          struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++)
    {
        switch (resources->type)
        {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("pxa250_timer: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("pxa250_timer: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }
    
    device->timer.device = device;
    device->timer.ops = timer_ops;

    device->ticks = 0;

    /* Set match register in the past */
    mr2_write(-1);
    cr_write(-1);
    mr1_write(0);       /* Set match value on wrap around */

    device->state = STATE_DISABLED;
    return DEVICE_SUCCESS;
}

/**
 * Ideally this would be done atomically, or at least with knowledge of
 * pre-emption.
 */
static inline void
set_timeout(struct pxa250_timer *device, uint32_t timeout)
{
    mr2_write(timeout);
}


static int
timer_timeout_impl(struct timer_interface *ti, struct pxa250_timer *device, uint64_t timeout)
{
    /* Any previous timeouts are deleted as of right now */
    int64_t u6 = timer_get_ticks_impl(ti, device);
    int64_t next = timeout - u6; //timer_get_ticks_impl(ti, device);

    if (next < 0) {
        printf("expired\n");
        return 1;
    }

    if (next > TIME_MAX) {
        // XXX fixme
        L4_KDB_PrintChar('*');
        next = TIME_MAX;
    }

    // printf("pxa250_timer: %s (timeout: %llx, next: %llx)\n", __FUNCTION__, timeout, next);

    set_timeout(device, timeout);
    device->state = STATE_SHOT_SHORT;

    return 0;
}

static int
device_enable_impl(struct device_interface *di, struct pxa250_timer *device)
{  
    /* Ack possible pending hardware interrupts */
    sr_write(0x04); sr_write(0x02);

    set_timeout(device, TIME_MAX);
    device->state = STATE_TIMEKEEP;

    /* Enable timer interrupts */
    ier_write(0x07);       /* Enabled = 0, 1, 2. Disabled = 3 */

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct pxa250_timer *device)
{
    device->state = STATE_DISABLED;

    /* Disable the timer */
    ier_write(0x08); // FIXME: do i put 0x8 or 0x0 here?

    return DEVICE_DISABLED;
}

static uint64_t
timer_set_tick_frequency_impl(struct timer_interface *ti, struct pxa250_timer *device, uint64_t hertz)
{
    // Dont support changing frequency?
    return TIMER_RATE;
}

static uint64_t
timer_get_tick_frequency_impl(struct timer_interface *ti, struct pxa250_timer *device )
{
    return TIMER_RATE;
}


static uint64_t
timer_get_ticks_impl(struct timer_interface *ti, struct pxa250_timer *device)
{
    uint32_t ticks;
    ticks = cr_read();

    return (device->ticks + ticks);
}

static void
timer_set_ticks_impl(struct timer_interface *ti, struct pxa250_timer *device, uint64_t ticks)
{
    device->ticks = ticks;
}

static int
device_poll_impl(struct device_interface *di, struct pxa250_timer *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, struct pxa250_timer *device, int irq)
{
    int ret = 0;
    uint32_t mask = sr_read();

    // XXX: fix the irq storm that happens when this is uncommented
    // printf("pxa250_timer: irq: %d, mask: %lx\n", irq, mask);

    // we only care about user timers
    // mask &= (0x2 | 0x4);

    if (mask & 0x2){
        device->ticks += 0x100000000ll;
    }
    if (mask & 0x4){
        ret = 1;
    }
    // Ack the interrupt
    sr_write(mask);
    return ret;
}

static int
device_num_interfaces_impl(struct device_interface *di, struct pxa250_timer *device)
{
    return 1;
}   
    
static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct pxa250_timer *device,
                          int interface)
{
    return (struct generic_interface *) &device->timer;
} 

