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
 * Authors: Carl van Schaik, Geoffrey Lee
 */

#include "uart.h"
#include <l4/kdebug.h>
#include <util/trace.h>

enum parity { PARITY_NONE, PARITY_EVEN, PARITY_ODD /* xxx mark/space */};
//#define BAUD_RATE_CONSTANT 3686400 /* Frequency of the crystal on the SA1100 */

#define DEFAULT_BAUD 115200
#define DEFAULT_SIZE 8
#define DEFAULT_PARITY PARITY_NONE
#define DEFAULT_STOP 1

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

/* fw decl */
static int serial_set_params(struct s3c2410_uart *device, 
                  unsigned baud,
                  int data_size,
                  enum parity parity,
                  int stop_bits);
static void do_xmit_work(struct s3c2410_uart *device);
static int do_rx_work(struct s3c2410_uart *device);


static int
device_setup_impl(struct device_interface *di, struct s3c2410_uart *device,
                  struct resource *resources)
{
    int i, n_mem = 0;
    for (i = 0; i < 8; i++) {
        switch (resources->type) {
        case MEMORY_RESOURCE:
            if (n_mem == 0)
                device->main = *resources;
            else
                printf("s3c2410_uart: got more memory than expected!\n");
            n_mem++;
            break;
            
        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;
            
        default:
            printf("s3c2410_uart: Invalid resource type %d!\n", resources->type);
            break;
        }
        resources++;
    }

    device->tx.device = device;
    device->rx.device = device;
    device->tx.ops = stream_ops;
    device->rx.ops = stream_ops;

    device->txen = 0;
    device->writec = 0;

    device->rxen = 0;
    device->readc = 0;

    if (serial_set_params(device, DEFAULT_BAUD, DEFAULT_SIZE, DEFAULT_PARITY,
        DEFAULT_STOP) != 0)
        return 0;
    
    dprintf("%s: done\n", __func__);
    return DEVICE_SUCCESS;
}

static int
serial_set_params(struct s3c2410_uart *device, 
                  unsigned baud,
                  int data_size,
                  enum parity parity,
                  int stop_bits)
{
    ulcon_read();
    ucon_read();
    ufcon_read();
    umcon_read();

    if (data_size < 5 || data_size > 8)
        return -1;
    if (stop_bits < 1 || stop_bits > 2)
        return -1;

    ulcon_set_word_len(data_size - 5);
    ulcon_set_stop_bits(stop_bits - 1);
    ulcon_set_parity(parity == PARITY_NONE ? 0 :
            parity == PARITY_ODD ? 0x4 : 0x5);
    ulcon_set_ir_mode(0);

    /* ... set baud rate */

    ucon_set_rx_mode(1); // enable irq mode
    ucon_set_tx_mode(0); // disable irq mode
    ucon_set_loopback(0);
    ucon_set_rx_err_irqen(0);
    ucon_set_rx_to_irqen(1); // rx timeout irq
    ucon_set_rx_irq_type(0); // edge
    ucon_set_tx_irq_type(0); // edge

    ufcon_set_rx_fifo_rst(1);
    ufcon_set_tx_fifo_rst(1);
    ufcon_set_fifo_en(1);
    ufcon_set_rx_fifo_level(0);  // 4 bytes
    ufcon_set_tx_fifo_level(0);  // 0 bytes

    return 0;
}

static int
device_enable_impl(struct device_interface *di, struct s3c2410_uart *device)
{
    device->state = STATE_ENABLED;
    dprintf("%s: done\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_uart *device)
{
    device->state = STATE_DISABLED;
    dprintf("%s: done\n", __func__);

    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct s3c2410_uart *device)
{
    dprintf("%s: called.\n", __func__);
    return 0;
}

/*
 * XXX clagged from imx21 serial driver: should be autogenerated.
 */
static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct s3c2410_uart *device)
{
    dprintf("%s: called.\n", __func__);

    return 2;    /* XXX: in reality there are more UARTs than this. -gl */
}

/*
 * XXX clagged from imx21 serial driver: should be autogenerated.
 */
static struct generic_interface *
device_get_interface_impl(struct device_interface *di, 
                          struct s3c2410_uart *device, int i)
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
                      struct s3c2410_uart *device,
                      int irq)
{
    int status = 0;

//    printf("ufstat is %08lx\n", ufstat_read());
//    printf("uerstat is %08lx\n", uerstat_read());
//    printf("utrstat is %08lx\n", utrstat_read());

    dprintf("%s: in interrupt.\n", __func__);

    /*
     * Interrupt handling.  The TX interrupt means TX done so we check
     * to see if the TX buffer is empty.  If so we do xmit.
     *
     * The RX interrupt means there is data in the receive buffer so we
     * read it off and pass it to the user.
     *
     * -gl
     */
    if (ufstat_get_tx_fifo_cnt() == 0 && !ufstat_get_tx_fifo_full()) {
        device->fifodepth = 1;    /* XXX */
        do_xmit_work(device);
        ucon_set_tx_mode(1);   // disable irq mode
    }

    if (ufstat_get_rx_fifo_full() || ufstat_get_rx_fifo_cnt() > 0)  {
        status = do_rx_work(device);
    }

    dprintf("%s: interrupt done status %d.\n", __func__, status);
    return status;
}

static void
do_xmit_work(struct s3c2410_uart *device)
{
    struct stream_interface *si = &device->tx;
    struct stream_pkt *packet = stream_get_head(si);

    dprintf("%s: called  FIFO depth = %d.\n", __func__, device->fifodepth);

    if (packet == NULL)
        return;

    while (device->fifodepth) {
        device->fifodepth--;
        /* Place character on the UART FIFO */
        assert(packet->data);
        dprintf("Transmitting character ASCII %d\n",
            packet->data[packet->xferred]);

        // Turn on the transmitter
        ucon_set_tx_mode(1);

        utxh_write(packet->data[packet->xferred++]);
        assert(packet->xferred <= packet->length);
        if (packet->xferred == packet->length) {
            /* Finished this packet */
            packet = stream_switch_head(si);
            if (packet == NULL)
                break;
        }
    }

    if (device->fifodepth == 0){
        ; // leave the transmitter on
    } else
        ucon_set_tx_mode(0); // turn it off to avoid interrupts

    dprintf("%s: done.\n", __func__);
}

static int
do_rx_work(struct s3c2410_uart *device)
{
    struct stream_interface *si = &device->rx;
    struct stream_pkt *packet = stream_get_head(si);
    uint16_t data;

    while (ufstat_get_rx_fifo_cnt()) {
        data = urxhh_get_rbr();
        if ((data & 0xff) == 0xb) { /* Ctrl-k */
            L4_KDB_Enter("breakin");
            continue;
        }
        if (!packet) continue;
        packet->data[packet->xferred++] = (data & 0xff);
        if (packet->xferred == packet->length) {
            if ((packet = stream_switch_head(si)) == NULL) {
                /*
                 * Empty the FIFO by dropping the characters if we have
                 * have space to put them.
                 */
                while (ufstat_get_rx_fifo_cnt()) {
                    if ((urxhh_get_rbr() & 0xff) == 0xb) 
                         L4_KDB_Enter("breakin");
                }
                break;
            }
        }
    }

    if (packet) {
        if (packet->xferred)
            packet = stream_switch_head(si);
    }

    dprintf("%s: done.\n", __func__);

    return 0;
}

static int
stream_sync_impl(struct stream_interface *si, struct s3c2410_uart *device)
{
    int retval = 0;

    dprintf("%s: stream %p dev %p\n", __func__, si, device);

    if (si == &device->tx)
        do_xmit_work(device);
    if (si == &device->rx)
        retval = do_rx_work(device);

    dprintf("%s: done.\n", __func__);

    return retval;
}
