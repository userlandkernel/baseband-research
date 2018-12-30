/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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

#include "c8250_serial.h"
#include <l4/kdebug.h>

// FIXME: make framework deal with other variants of the 8250

enum parity { PARITY_NONE, PARITY_EVEN, PARITY_ODD, PARITY_MARK, PARITY_SPACE };


#define DEFAULT_BAUD            115200
#define DEFAULT_SIZE            8
#define DEFAULT_PARITY          PARITY_NONE
#define DEFAULT_STOP            1
#define BAUD_RATE_CONSTANT      115200
#define CAP_INT_OUT2            (1 << 3)
#define tx_ready()              (!lsr_get_trdy())
#define rx_ready()              (lsr_get_dr())

/* Seven or eight bits for parity */
/* One or two stop bits */
int serial_set_params(struct device_interface *di, struct c8250_serial *self, unsigned baud, int data_size,
                      enum parity parity, int stop_bits);

static int
device_setup_impl(struct device_interface *di, struct c8250_serial *device,
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
                printf("vtimer: got more memory than expected!\n");
            n_mem++;
            break;

        case INTERRUPT_RESOURCE:
        case BUS_RESOURCE:
        case NO_RESOURCE:
            /* do nothing */
            break;

        default:
            printf("vtimer: Invalid resource type %d!\n", resources->type);
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

    serial_set_params(di, device, DEFAULT_BAUD, DEFAULT_SIZE, DEFAULT_PARITY,
                     DEFAULT_STOP);
    return DEVICE_SUCCESS;
}

/* Seven or eight bits for parity */
/* One or two stop bits */
int
serial_set_params(struct device_interface *di, struct c8250_serial *device, unsigned baud, int data_size,
                  enum parity parity, int stop_bits /* , int mode */)
{

    if (data_size < 5 || data_size > 8) {
        /* Invalid data_size */
        return -1;
    }

    if (stop_bits < 1 || stop_bits > 2) {
        /* Invalid # of stop bits */
        return -1;
    }
    /* Baud Rate is already set by the kernel and we don't 
     * require to reset it */
#if 0
       lcr_set_dlab(0x1);
    /* The LSB of Baud Rate divisor goes in DLL and the MSB goes in DLH */
        dll_set_lsb((115200/ baud) & 0xFF);
        dlh_set_msb(((115200 / baud)>> 8) & 0xFF);

        if(dll_get_lsb() == 0 && dlh_get_msb() == 0) {
                printf("Error: The Baud Rate divisor is set to zero \n");
        }

    /*Resetting the DLAB bit to 0 */
    lcr_set_dlab(0x0);
#endif
   /* Initializing the UART */
    if ( data_size == 8) 
        lcr_set_dwl(0x03);

    if ( parity == PARITY_NONE) 
        lcr_set_par(0x0);

    if (stop_bits == 0) 
        lcr_set_stop(0x0);

    /* Disable Interrupts */
    ier_write(0x0);
    /* Enable Data Receive Available interrupt */
    ier_set_rdie(0x1);
    /*Disable Transmit Data request interrupt */
    ier_set_txie(0x0);

    /* Acknowledge any pending ints */
    lsr_read();
    rbr_read();
    iir_read();
    msr_read();
    ier_read();
    mcr_read();


    /* Disable loopback */
    mcr_set_loop(0x0);

    if (CAP_INT_OUT2) {
        /* Enable UART interrupt to CPU  */
        mcr_set_out2(0x1);
    }
    return 0;
}

static int
device_enable_impl(struct device_interface *di, struct c8250_serial *device)
{
    device->state = STATE_ENABLED;
    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct c8250_serial *device)
{
    /* We don't actually turn off the UART here, as the kernel may still
       be using it */
    device->state = STATE_DISABLED;
    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct c8250_serial *device)
{
    return 0;
}


static void
do_xmit_work(struct c8250_serial *device)
{
    struct stream_interface *si = &device->tx;
    struct stream_pkt *packet = stream_get_head(si);

    if (packet == NULL) {
        return;
    }

    assert(packet->data);
    while (!tx_ready()) {
        thr_write(packet->data[packet->xferred++]);
        assert(packet->xferred <= packet->length);
        if (packet->xferred == packet->length) {
            packet = stream_switch_head(si);
            if (packet == NULL) {
                break;
           }
        }
    }

    ier_set_txie(1);
    return;
}


static int 
do_rx_work(struct c8250_serial *device)
{  
    uint8_t data;
    struct stream_interface *si = &device->rx;
    struct stream_pkt *packet = stream_get_head(si);

    while (rx_ready()) {
        data = rbr_get_data();
        /* 11 or 0xb is Ctrl-k */ 
        if ((data & 0xff) == 0xb){
            L4_KDB_Enter("breakin");
            // Dont want to pass the C-k to the client
            continue;
        }

       if(!packet)
           continue;

       packet->data[packet->xferred] = (data & 0xFF);
       packet->xferred++;
       while(rx_ready())
           rbr_get_data();
       if (packet->xferred == packet->length) {
           if ((packet = stream_switch_head(si)) == NULL) {
               break;
           }
       }

   }

   if (packet){
       if (packet->xferred)
           packet = stream_switch_head(si);
   }
 return 0;
}

/*
 * FIXME: dd_dsl.py should generate these.. 
 */

static int
device_interrupt_impl(struct device_interface *di, struct c8250_serial *device, int irq)
{
    /* This UART is interrupt driven.We're using COM1 with base address as 0x3F8 and
     * IRQ 4.This IRQ number has to be registered with the kernel in machines.py file 
     */
    uint8_t r;
    int status=0;
    r = iir_read();

    if (!r&1) {
        printf("No Interrupt is pending\n");
    } else {
        if (!r&1) {
            printf(" Multiple Interrupts detected\n");
        }
   }
    r = lsr_read();

    if (r&0x1){ // RX
       /* lsr DR is set which means that an incoming character has been received and transfered into
        * the Receive Buffer Register.
        */
        status = do_rx_work(device);
    }
    if ((r&0x60) == 0){
       /* This means that a character has been transfered from THR into Transmit Shift Register */
        ier_set_txie(0);   /* stop the interrupt */
        do_xmit_work(device);
    }

    return status;
}

static int
stream_sync_impl(struct stream_interface *si, struct c8250_serial *device)
{
    int retval = 0;

    if (si == &device->tx) {
        do_xmit_work(device);
    }
    if (si == &device->rx) {
        retval = do_rx_work(device);
    }
    return retval;
}

/* FIXME: This can be autogenerated! */
static int
device_num_interfaces_impl(struct device_interface *di, struct c8250_serial *device)
{
    return 2;
}

/* FIXME: This can definately be autogenerated */
static struct generic_interface *
device_get_interface_impl(struct device_interface *di, struct c8250_serial *device, int interface_num)
{
    switch (interface_num) {
    case 0:
        return (struct generic_interface *) &device->tx;
    case 1:
        return (struct generic_interface *) &device->rx;
    default:
        return NULL;
    }
}

