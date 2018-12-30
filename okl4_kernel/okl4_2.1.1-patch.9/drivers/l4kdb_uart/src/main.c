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
#include <iguana/env.h>

#include "l4kdb_uart.h"

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

#define TIMER_HACK

#ifdef TIMER_HACK
/*
 * Put these into the device structure if possible.
 */
L4_ThreadId_t poll_tid, main_tid;
extern L4_ThreadId_t devicecore_tid;
/*
 * XXX timer_hack_char is racy.
 */
char timer_hack_char;
#define IRQ_LABEL (((~(0UL)) >> 20 << 20) >> 16)

#include <interfaces/vtimer_client.h>
#include <vtimer/timer.h>
#include <stdlib.h>                    /* abort() */

/*
 * Polling hack by ported from driverv1 by glee
 *
 * -gl
 */
static void
pollthrd(void *arg)
{
#ifndef NDEBUG
    L4_ThreadId_t timer_server, dummy;
    device_t timer_handle;
    struct l4kdb_uart *device = (struct l4kdb_uart *)arg;
    (void)device;

    L4_Receive(main_tid);

    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(0x1);

    timer_server = env_thread_id(iguana_getenv("VTIMER_TID"));
    timer_handle = env_const(iguana_getenv("VTIMER_HANDLE"));

    virtual_timer_init(timer_server, timer_handle, poll_tid.raw, 0x1, NULL);
    virtual_timer_request(timer_server, timer_handle, MILLISECS(10), TIMER_PERIODIC, NULL);

    for (;;) {
        timer_hack_char = L4_KDB_ReadChar();
        if (timer_hack_char != -1) {
            L4_Notify(main_tid, 1UL << 31);
        }
        if (L4_IpcFailed(L4_Wait(&dummy))) {
            dprintf("%s: failed ipc.\n", __func__);
            continue;
        }
    } 
#else    /*NDEBUG*/
    L4_WaitForever();
#endif   /*NDEBUG*/
}

static void
device_start_pollthrd(struct l4kdb_uart *device)
{
    poll_tid = thread_l4tid(thread_create_simple(pollthrd, device, 239/*XXX*/));
    /* Make poll thread wait until poll_tid has a valid value */
    L4_Send(poll_tid);
    L4_KDB_SetThreadName(poll_tid, "kdb_poll");
}

#endif

static void
do_tx_work(struct l4kdb_uart *device)
{
    struct stream_interface *si = &device->tx;
    struct stream_pkt *packet = stream_get_head(si);

    if (packet == NULL)
        return;

    while (packet->xferred < packet->length) {
        L4_KDB_PrintChar(packet->data[packet->xferred++]);
        if (packet->xferred == packet->length) {
            if ((packet = stream_switch_head(si)) == NULL)
                break;
        }
    }

    dprintf("%s: done.\n", __func__);
}

static int
do_rx_work(struct l4kdb_uart *device)
{
    struct stream_interface *si = &device->rx;
    struct stream_pkt *packet = stream_get_head(si);
    char data;

    data = -1;
    /*
     * First character has already been consumed unfortunately.  :-(
     */
    for (;;) {
        if (data == -1) {
            data = timer_hack_char;
            timer_hack_char = -1;
        } else
            data = L4_KDB_ReadChar();
        if (data == 0xb) { /* Ctrl-k */
            L4_KDB_Enter("breakin");
            continue;
        }

        if (data == -1)
            break;
        if (!packet)
            continue;
         packet->data[packet->xferred++] = data;
         if (packet->xferred == packet->length) {
             /*
              * Normally we would flush the FIFO and drop chars but
              * not in this !
              */
             if ((packet = stream_switch_head(si)) == NULL) 
                 break;
         } 
    }

    if (packet && packet->xferred)
        packet = stream_switch_head(si);

    return 0;
}

static int
device_setup_impl(struct device_interface *di, struct l4kdb_uart *device,
                  struct resource *resources)
{
    device->tx.device = device;
    device->rx.device = device;
    device->tx.ops = stream_ops;
    device->rx.ops = stream_ops;

    device->txen = 0;
    device->writec = 0;

    device->rxen = 0;
    device->readc = 0;

    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    device_start_pollthrd(device);

    dprintf("%s: done\n", __func__);

    return DEVICE_SUCCESS;
}

static int
device_enable_impl(struct device_interface *di, struct l4kdb_uart *device)
{
    device->state = STATE_ENABLED;
#ifndef TIMER_HACK
    virtual_timer_request(device->timer_server, device->timer_device, MILLISECS(10), TIMER_PERIODIC, NULL);
#endif
    dprintf("%s: done.\n", __func__);
    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct l4kdb_uart *device)
{
    device->state = STATE_DISABLED;
#ifndef TIMER_HACK
    virtual_timer_cancel(device->timer_server, device->timer_device, NULL);
#endif
    dprintf("%s: done.\n", __func__);
    return DEVICE_DISABLED;
}

/*
 * We just pretend to be interrupt-driven when we only have polling
 * I/O.  Yet, we do not support poll method.
 */
static int
device_poll_impl(struct device_interface *di, struct l4kdb_uart *device)
{
    dprintf("%s: called.\n", __func__);
    return 0;
}

static int
device_num_interfaces_impl(struct device_interface *di,
                           struct l4kdb_uart *device)
{
    dprintf("%s: called.\n", __func__);
    return 2;
}

/*
 * XXX clagged from imx21 serial driver: should be autogenerated.
 */
static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct l4kdb_uart *device, int i)
{
    switch (i) {
        case 0:
            return (struct generic_interface *)&device->tx;
        case 1:
            return (struct generic_interface *)&device->rx;
        default:
            return NULL;
    }
}

static int
device_interrupt_impl(struct device_interface *di,
                      struct l4kdb_uart *device,
                      int irq)
{
    /*
     * With our cheesy emulation this must be an rx "interrupt".
     * So, dispatch the work to do_rx_work().
     */
    return do_rx_work(device);
}

static int
stream_sync_impl(struct stream_interface *si, struct l4kdb_uart *device)
{
    int retval = 0;

    dprintf("%s: stream %p dev %p\n", __func__, si, device);

    if (si == &device->tx)
        do_tx_work(device);
    if (si == &device->rx)
        retval = do_rx_work(device);

    dprintf("%s: done.\n", __func__);

    return retval;
}

