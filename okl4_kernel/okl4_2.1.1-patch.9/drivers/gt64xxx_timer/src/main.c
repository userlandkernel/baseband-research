/*
 * Copyright (c) 2004, National ICT Australia (NICTA)
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

#include <driver/driver.h>
#include <driver/timer_ops.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <l4/types.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/misc.h>
#include <iguana/thread.h>
#include <iguana/env.h>

/* Timer specific section */

#include <iguana/memsection.h>  /* memsection_create */
#include <iguana/hardware.h>    /* hardware_back_memsection() */
#include "gt64xxx.h"

uintptr_t GT_TIMERS_VADDR;

static int
map_gt64xxx_timer(void)
{
        size_t length = 0x1000;
        uintptr_t offset = GT_TIMERS_PHYS; /* physical address of GT timer registers */
        memsection_ref_t ms;
        int r;

        printf("GT64xxx timer driver starting...\n");

        ms = memsection_create_user(length, &GT_TIMERS_VADDR);

        if (ms == 0)
            return 0;

        r = hardware_back_memsection(ms, offset, L4_IOMemory);
        if (r != 0)
        {
                printf("Error backing memory section: (%d)\n", r);
                return 0;
        }
        return 1;
}

/* Driver specific section */

static void *setup(int spacec, bus_space_t *spacev, dma_handle_t dma, bus_space_t pciconf);
static void  enable(void *device);
static void  cleanup(void *device);
static void  interrupt(void *device, int irq);
static int timeout(void *device, uint64_t time, callback_t callback, 
                   callback_data_t callback_data, uintptr_t key);
static uint64_t current_time(void *device);

struct gt64xxx_dev {
        struct driver_instance generic;
        int enabled;
};
                
static struct timer_ops ops = {
        /* Driver ops */
        { setup,
          enable,
          cleanup,
          interrupt },
        /* Timer ops */
        timeout,
        current_time
};

struct driver gt64xxx_timer = 
{
        .name = "GT64xxx timer",
        .class = timer_device_e,
        .ops.t_ops = &ops
};

static void *
setup(int spacec, bus_space_t *spacev, dma_handle_t dma, bus_space_t pciconf)
{
        struct gt64xxx_dev *drv;
        L4_Word_t irq = IRQ_GT;
        L4_ThreadId_t main_tid;

        if (!map_gt64xxx_timer())
            return NULL;

        drv = malloc(sizeof(struct gt64xxx_dev));
        if (drv == 0) return 0;
        memset(drv, 0, sizeof(*drv));

        drv->generic.classp = &gt64xxx_timer;

        /* Create a thread */
        main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

        /* Register timer interrupt */
        hardware_register_interrupt(main_tid, irq);

        /* Start the timer ticks */
        *GT_TIMER_CONTROL = (0ul<<24);
        *GT_TIMER_0 = 0xffffffff;
        *GT_TIMER_CONTROL = (1ul<<24);

        /* Enable timer0 interrupt */
        gt_write(GT_INTERRUPT_MASK, 1<<8);

        timeout(NULL, -1, NULL, NULL, 0);

        return drv;
}

static void
enable(void *device)
{
}
static void
cleanup(void *device)
{
}

static callback_t timer_callback;
static callback_data_t timer_data;
static uintptr_t timer_key;

static uint64_t next_timeout, ticks = 0;
static uint32_t count_from = 0xffffffff;

static void
interrupt(void *device, int irq)
{
        ticks += count_from;
        *GT_INTERRUPT_CAUSE = 0;

        if (next_timeout != -1ull)
        {
                if (count_from == 0xffffffff)
                {
                        next_timeout -= 0xffffffff;
                        if (next_timeout)
                        {
                                count_from = next_timeout >= (0xffffffff) ? (0xffffffff) : next_timeout;

                                /* Setup the timer */
                                *GT_TIMER_CONTROL = (0ul<<24);
                                gt_write(GT_TIMER_0, count_from);
                                *GT_TIMER_CONTROL = (1ul<<24);

                                return;
                        }
                }

                /* Reset the timer downcount */
                *GT_TIMER_CONTROL = (0ul<<24);
                *GT_TIMER_0 = 0xffffffff;
                *GT_TIMER_CONTROL = (1ul<<24);
                count_from = 0xffffffff;

                timer_callback(timer_data, 0, NULL, timer_key);
        }
}


static int
timeout(void *device, uint64_t time, callback_t callback, callback_data_t callback_data, uintptr_t key)
{
        next_timeout = time;
        timer_callback = callback;
        timer_data = callback_data;
        timer_key = key;

        uint32_t value = gt_read(GT_TIMER_0);

//      if (count_from != 0xffffffff)
                ticks += count_from - value;

        if (next_timeout != -1) {
                next_timeout = US_TO_TICKS(next_timeout);

                if (next_timeout > ticks)
                {
                        next_timeout -= ticks;

                        count_from = next_timeout >= (0xffffffff) ? (0xffffffff) : next_timeout;

                        /* Setup the timer */
                        *GT_TIMER_CONTROL = (0ul<<24);
                        gt_write(GT_TIMER_0, count_from);
                        *GT_TIMER_CONTROL = (1ul<<24);

                } else {
                        return 1;
                }
        }
        return 0;
}

static uint64_t
current_time(void *device)
{
        /* Get the counter value */
        uint32_t value = gt_read(GT_TIMER_0);

        value = count_from - value;

        return TICKS_TO_US(ticks + value);
}

