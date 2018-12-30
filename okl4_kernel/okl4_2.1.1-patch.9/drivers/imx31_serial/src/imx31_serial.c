/*
 * Copyright (c) 2005 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   i.MX31 serial driver
 */

/*
 * Set this to enable debug.
 * USER BEWARE: this flag is DANGEROUS and DOES NOT WORK because 
 * the uart this serial driver use doubles up as the debug console, 
 * it gets very confused if you so ever slightly place the DBG() 
 * in the wrong place.
 *
 * -gl
 */
#define DBG_ENABLE   0

/*
 * Define this to 1 if kernel does not contain uart kernel driver.
 *
 * If this is set to 0 then we are stealing the kernel uart so 
 * we do not try to do anything which may disable tx / rx or 
 * reset the uart chip state.
 */
#define KERNEL_UART_DISABLED 0

/*
 * Set to 1 if you want TX interrupts be used
 */
#define USE_TX_INT 1

#if DBG_ENABLE
#define DBG(fmt, va_args...)    printf("%s: "fmt, __func__, ##va_args)
#else
#define DBG(fmt, va_args...)
#endif

#include <l4/kdebug.h>
#include "imx31_serial.h"
#include "imx31_serial_defs.h"

#define tx_ready(device)               (!uts1_get_txfull())
#define rx_ready(device)               (!uts1_get_rxempty())
#define tx_interrupt(device)           (usr1_get_trdy())
#define rx_interrupt(device)           (usr1_get_rrdy())
#define rx_mask_interrupt(device)      (ucr1_set_rrdyen(0x0))
#define tx_mask_interrupt(device)      (ucr1_set_trdyen(0x0))
#define rx_unmask_interrupt(device)    (ucr1_set_rrdyen(0x1))
#define tx_unmask_interrupt(device)    (ucr1_set_trdyen(0x1))

static void
do_tx_work
(
    struct imx31_serial *device
)
{
    struct stream_interface *si = &device->tx;
    struct stream_pkt *packet = stream_get_head(si);

#if !KERNEL_UART_DISABLED
    if (device->state == STATE_DISABLED)
        return;
#endif
    if (packet == NULL) 
        goto out;

    /*
     * In v1 driver it limits the rate.  Do we need to do the same? 
     * -gl
     */
    while (packet->xferred < packet->length) {
        assert(packet->data);
        while (!tx_ready());
        utxd_write(packet->data[packet->xferred++]);
        assert(packet->xferred <=packet->length);
        if (packet->xferred == packet->length &&
            (packet = stream_switch_head(si)) == NULL) 
                break;
    }
out:
    return;
}

static int
do_rx_work
(
    struct imx31_serial     *device
)
{
    struct stream_interface *si = &device->rx;
    struct stream_pkt *packet = stream_get_head(si);
    uint8_t data;

#if !KERNEL_UART_DISABLED
    if (device->state == STATE_DISABLED)
        return 0;
#endif
    while (rx_ready(device)) {
        DBG("trying to get a character\n");
        
        data = urxd_get_data();
        DBG("got character %d\n", data);
        if (data == 0xb) {    /* Ctrl-k */
            L4_KDB_Enter("breakin");
            continue;
        }
        if (!packet)
            continue;
        packet->data[packet->xferred++] = data;
        if (packet->xferred == packet->length) {
            /*
             * We have to start dropping characters.  :-(
             */
            if ((packet = stream_switch_head(si)) == NULL) {
                while (urxd_get_data() == 0xb)
                    L4_KDB_Enter("breakin");
            }
        }
    }

    if (packet && packet->xferred)
        packet = stream_switch_head(si);

    return 0;
}

static int
stream_sync_impl
(
    struct stream_interface *si,
    struct imx31_serial     *device
)
{
    /*
     * This code is clagged from other serial drivers
     */
    int retval = 0;

    DBG("stream %p dev %p\n", si, device);
    if (si == &device->tx)
        do_tx_work(device);
    else if (si == &device->rx)
        retval = do_rx_work(device);

    DBG("done\n");
    return retval;
}

static inline void
uart_enable
(
    struct imx31_serial     *device
)
{
    /*
     * Enable uart interrupt, and tell me when there are rx interrupts
     * Yes, we have to turn these on ourselves.
     */
#if KERNEL_UART_DISABLED
    ucr1_set_uarten(UARTEN_ENABLE);
    ucr2_set_rxen(0x1);
    ucr2_set_txen(0x1);
#endif
    rx_unmask_interrupt(device);
    device->state = STATE_ENABLED;
}

static inline void
uart_disable
(
    struct imx31_serial     *device
)
{
    /*
     * Disable the UART if we are exclusive user.  If so then turn off
     * the tx and rx.
     */
#if KERNEL_UART_DISABLED
    ucr1_set_uarten(UARTEN_DISABLE);
    ucr2_set_rxen(0x0);
    ucr2_set_txen(0x0);
    rx_mask_interrupt();
    tx_mask_interrupt();
#endif
    device->state = STATE_DISABLED;
}

static void
reset_register_state
(
    struct imx31_serial     *device
)
{
#if KERNEL_UART_DISABLED
    ucr1_set_trdyen(TX_RDY_IRQ_ON);
    ucr1_set_rrdyen(RX_RDY_IRQ_ON);

    ucr2_set_pren(PARITY_NONE);
    ucr2_set_stpb(STOP_BITS_1);
    ucr2_set_ws(WORD_SIZE_8_BITS);
    ucr2_set_txen(TXEN_ENABLE);
    ucr2_set_rxen(RXEN_ENABLE);
#endif
}

static int
device_setup_impl
(
    struct device_interface *di,
    struct imx31_serial     *device,
    struct resource         *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++)
    {
        switch (resources->type)
        {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->uart_space = *resources;
            else
                printf("imx31_serial: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("imx31_serial: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }

    //device->uart_space = *resources;
    device->tx.device = device;
    device->rx.device = device;
    device->tx.ops = stream_ops;
    device->rx.ops = stream_ops;

    reset_register_state(device);
    uart_disable(device);

    DBG("done, uart is in disabled state\n");
    return DEVICE_SUCCESS;
}

static int
device_enable_impl
(
    struct device_interface *di,
    struct imx31_serial     *device
)
{
    device->state = STATE_ENABLED;

    reset_register_state(device);
    uart_enable(device);

    DBG("uart is enabled\n");
    return DEVICE_ENABLED;
}

static int
device_disable_impl
(
    struct device_interface *di,
    struct imx31_serial     *device
)
{
    device->state = STATE_DISABLED;

    reset_register_state(device);
    uart_disable(device);

    DBG("uart is enabled\n");
    return DEVICE_DISABLED;
}

static int
device_poll_impl
(
    struct device_interface *di,
    struct imx31_serial     *device
)
{
    DBG("nothing to do\n");
    return 0;
}

static int
device_interrupt_impl
(
    struct device_interface *di,
    struct imx31_serial     *device,
    int                     irq
)
{
    /*
     * Already in do_tx_work() and do_rx_work() but here also
     * to avoid checking status register of uart.
     */
#if !KERNEL_UART_DISABLED
    if (device->state == STATE_DISABLED)
        return 0;
#endif

    if (tx_interrupt(device)) {
        DBG("tx intr\n");
#if !(USE_TX_INT)
        assert(!"TX interrupts are disabled");
#endif
        do_tx_work(device);
    }

    if (rx_interrupt(device)) {
        DBG("rx intr\n");
        rx_mask_interrupt(device);
        do_rx_work(device);
        rx_unmask_interrupt(device);
    }
    return 0;
}

static int
device_num_interfaces_impl
(
    struct device_interface *di,
    struct imx31_serial     *dev
)
{
    return 2;
}

static struct generic_interface *
device_get_interface_impl
(
    struct device_interface *di,
    struct imx31_serial     *device,
    int                     interface
)
{
    switch (interface) {
        case 0:
            return (struct generic_interface *)&device->tx;
        case 1:
            return (struct generic_interface *)&device->rx;
        default:
            return NULL;
    }
}
