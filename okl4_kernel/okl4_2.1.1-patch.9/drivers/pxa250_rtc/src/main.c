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

#include "pxa250.h"
#include "pxa250_rtc.h"

#include <l4/kdebug.h>

#define dprintf(arg...) printf(arg)

#define CLOCK_DIVISOR_HW  0x00007fff
#define CLOCK_DIVISOR_SIM 0x00000bcc

static uint64_t
rtc_get_time_impl(struct rtc_interface *rtci, struct pxa250_rtc *device)
{
    return rcnr_get_rcv();
}

static void
rtc_set_time_impl(struct rtc_interface *rtci, struct pxa250_rtc *device,
                  uint64_t time)
{
    rcnr_set_rcv(time);
}

static uint64_t
rtc_get_alarm_impl(struct rtc_interface *ti,
                   struct pxa250_rtc *device)
{
    return rtar_get_rtmv();
}

static int
rtc_set_alarm_impl(struct rtc_interface *ti, 
                   struct pxa250_rtc *device, uint64_t time)
{
    uint64_t curr_time;

    rtar_set_rtmv(time);

    /* Enable alarm interrupt. Disabled in interrupt routine. */
    rtsr_set_ale(0x1);

    /* Has alarm expired? */
    curr_time = rcnr_get_rcv();
    if (time < curr_time) {
        return 1;
    }

    return 0;
}

static void
rtc_cancel_alarm_impl(struct rtc_interface *ti, struct pxa250_rtc *device)
{
    /* Disable alarm interrupt. */
    rtsr_set_ale(0x0);
}

static int
device_setup_impl(struct device_interface *di, struct pxa250_rtc *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("pxa250_rtc: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("pxa250_rtc: Invalid resource type %d!\n", resources->type);
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
device_enable_impl(struct device_interface *di, struct pxa250_rtc *device)
{
    /* set clock divisor to get 1Hz resolution. */ 
    //rttr_set_ck_div(CLOCK_DIVISOR_HW);

    /* If simulating, set the following instead: */
    rttr_set_ck_div(CLOCK_DIVISOR_SIM);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct pxa250_rtc *device)
{
    /* Disable alarm. */
    rtsr_set_ale(0x0);

    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct pxa250_rtc *device)
{
    return 0;
}

static int
device_interrupt_impl(struct device_interface *di, 
                      struct pxa250_rtc *device,
                      int irq)
{
    uint32_t al = rtsr_get_al(); 

    /* Acknowledge interrupt, clears interrupt sources. */
    rtsr_set_al(0x1);
    rtsr_set_hz(0x1);

    /* Disable alarm, if one has occured. */
    if (al) {
        rtsr_set_ale(0x0);
    }

    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct pxa250_rtc *dev)
{
    return 1;
}

static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct pxa250_rtc *device,
                          int interface)
{
    return (struct generic_interface *)&device->rtc;
}

