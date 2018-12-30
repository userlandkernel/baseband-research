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
 * Author: David Mirabito, Nelson Tam
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/kdebug.h>

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/pd.h>
#include <iguana/cap.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/stream.h>

#include <event/event.h>

#include <circular_buffer/cb.h>
#include <range_fl/range_fl.h>

#include <interfaces/vspi_serverdecls.h>


/**
 * SPI_DRIVER is defined in the build system, and indicates the
 * device driver against which we are compiled.
 *
 */
extern struct driver SPI_DRIVER;

#define NUM_PACKETS 20

struct spi_device;

struct memsect_attach
{
    void        *base;
    uintptr_t   size;
};

#define TERMINATED 1
#define COMPLETED 2

struct stream_packet
{
    uintptr_t   next;
    uintptr_t   data_ptr;
    size_t      size;
    size_t      xferred;
    int         status;
};

#define V(x) ((void*) ((uintptr_t) vspi->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define C(x) ((uint8_t*) ((uintptr_t) vspi->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define S(x) ((struct stream_packet*) ((uintptr_t) vspi->memsect_attach[(x & 0xf)].base + (x >> 4)))

struct control_block
{
    volatile uintptr_t tx;
    volatile uintptr_t rx;
};

struct spi_device;

struct virtual_spi
{
    struct virtual_spi      *next;
    struct spi_device       *spi_device;
    L4_ThreadId_t           thread;
    uintptr_t               mask;
    struct memsect_attach   memsect_attach[16];
    struct control_block    *control;

    struct stream_packet    *tx_cur;
    struct stream_packet    *rx_cur;

    /*
     * struct stream_interface datafields for currently
     * connected spi_device
     */
    void *si_start, *si_end, *si_comp_start, *si_comp_end;
};

struct spi_device
{
    struct spi_device       *next;
    struct resource         *resources;
    struct device_interface *di;
    struct stream_interface *tx;
    struct stream_interface *rx;
    struct stream_pkt       *freelist;
    int                     irq;
    struct virtual_spi      *active_virtual;
    struct virtual_spi      *last_virtual;
};

struct spi_device   *spi_devices        = NULL;
struct resource     *global_resources   = NULL;


static L4_ThreadId_t primary_; /**< The thread id of the current
                                  "priamry" wombat instance */
/**
 * The server entry point.
 */
int
main(int argc, char *argv[])
{
    event_init();
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    register_event(thread_l4tid(env_thread(iguana_getenv("MAIN"))), 0x1000, EV_WOMBAT_SWITCH);
    vspi_server_loop();
    return 0;
}

void device_add_resource_impl
(
    L4_ThreadId_t   caller,         /* AUTO */
    uintptr_t       resource,
    uintptr_t       resource2,
    int             type,
    idl4_server_environment *env    /* AUTO */
)
{
    struct resource *new;
    new = (struct resource*)malloc(sizeof(struct resource));

    if (!new) return;

    switch (type)
    {
    case INTERRUPT_RESOURCE:
        {
            new->resource.interrupt = resource;
        }
        break;
    case MEMORY_RESOURCE:
        {
            uintptr_t memory, memory_ptr;
            mem_space_t mem_;
            memory = memsection_create_user(0x1000, &memory_ptr);
            hardware_back_memsection(memory, resource, L4_IOMemory);
            mem_ = (char *) memory_ptr;
            new->resource.mem = mem_;
        }
        break;
    case BUS_RESOURCE:
        {
            DEBUG_PRINT("BUS RESOURCE NOT IMPLEMENYED for spi\n");
        }
    default:
        DEBUG_PRINT("Warning: Unknown resource type: %d\n", type);
        return;
    }

    new->type = (enum resource_type)type;
    new->next = global_resources;
    global_resources = new;
}

/**
 * Create a new device.
 */
cap_t
device_create_impl
(
    CORBA_Object    _caller,        /* AUTO */
    L4_ThreadId_t   *bus,
    uint8_t         deviceid,
    idl4_server_environment * _env  /* AUTO */
)
{
    cap_t cap = { {0}, 0 };
    struct spi_device *spi_device;
    void *device, *stream_packet;
    int i;

    /* Allocate memory for the spi device */
    spi_device = malloc(sizeof(struct spi_device));
    device = malloc(SPI_DRIVER.size);
    stream_packet = malloc(sizeof(struct stream_pkt) * NUM_PACKETS);

    if (spi_device == NULL || device == NULL || stream_packet == NULL)
    {
        free(spi_device);
        free(device);
        free(stream_packet);
        return cap;
    }

    /* Initialise the device state */
    spi_device->next = spi_devices;
    spi_device->resources = global_resources;
    global_resources = NULL;
    spi_devices = spi_device;

    /* Create */
    spi_device->di = setup_device_instance(&SPI_DRIVER, device);

    /* Setup */
    device_setup(spi_device->di, spi_device->resources);

    /* Enable */
    device_enable(device);

    /* Now he's had a chance to program the HW correctly, we enable any interrupts */
    struct resource * r = spi_device->resources;
    for (  ; r ; r = r->next)
    {
        if (r->type == INTERRUPT_RESOURCE)
        {
            hardware_register_interrupt(thread_l4tid(env_thread(iguana_getenv("MAIN"))), r->resource.interrupt);
        }
    }

    /* Get our interfaces up correctly */
    spi_device->tx = (struct stream_interface *) device_get_interface(spi_device->di, 0);
    spi_device->rx = (struct stream_interface *) device_get_interface(spi_device->di, 1);

    /* Create a free list of packet that we can send to the device */
    spi_device->freelist = stream_packet;
    for (i = 0; i < NUM_PACKETS - 1; i++)
    {
        spi_device->freelist[i].next = &spi_device->freelist[i+1];
    }
    spi_device->freelist[i].next = NULL;

    cap.ref.obj = (uintptr_t)spi_device;
    return cap;
}

static void switch_active(void)
{
    struct virtual_spi * this = spi_devices->active_virtual;
    struct virtual_spi * next = this->next;
    struct virtual_spi * last = spi_devices->last_virtual;

    if (next == NULL)
    {
        DEBUG_PRINT("No other virtual spis to make active\n");
        return;
    }

    this->next = NULL;
    spi_devices->active_virtual = next;

    last->next = this;
    spi_devices->last_virtual = this;

    this->si_start      = spi_devices->rx->start;
    this->si_end        = spi_devices->rx->end;
    this->si_comp_end   = spi_devices->rx->completed_end;
    this->si_comp_start = spi_devices->rx->completed_start;

    spi_devices->rx->start           = next->si_start;
    spi_devices->rx->end             = next->si_end;
    spi_devices->rx->completed_end   = next->si_comp_end;
    spi_devices->rx->completed_start = next->si_comp_start;

    DEBUG_PRINT("\nSwitching to Client  0x%lx\n",
           next->thread.raw);
}

volatile int sched_flip = 0;

cap_t
virtual_spi_factory_create_impl
(
    CORBA_Object    _caller,        /* AUTO */
    objref_t        spi_device,
    L4_ThreadId_t   *thread,
    uintptr_t       mask,
    idl4_server_environment * _env  /* AUTO */
)
{
    cap_t cap = { {0}, 0 };
    struct virtual_spi *vspi;
    struct spi_device *dev = (void*)spi_device;

    primary_ = primary_wombat();

    vspi = malloc(sizeof(struct virtual_spi));

    if (vspi == NULL) {
        return cap;
    }

    vspi->spi_device = (void*) spi_device;
    vspi->thread = *thread;
    vspi->mask = mask;
    vspi->next = NULL;

    vspi->si_start = vspi->si_end = NULL;
    vspi->si_comp_end = vspi->si_comp_start = NULL;

    if (dev->active_virtual == NULL) {
        dev->active_virtual = vspi;
        dev->last_virtual = vspi;
    } else {
        dev->last_virtual->next = vspi;
        dev->last_virtual = vspi;
    }

    cap.ref.obj = (uintptr_t)vspi;

    return cap;
}


int
virtual_spi_add_memsection_impl
(
    CORBA_Object    _caller,        /* AUTO */
    objref_t        spi_obj,
    objref_t        memsection,
    uintptr_t       passwd,
    int             idx,
    idl4_server_environment * _env  /* AUTO */
)
{
    struct virtual_spi *vspi = (struct virtual_spi *)spi_obj;
    cap_t cap;
    int ret;

    if (idx > 16) {
        /* ERROR */
        return -1;
    }
    vspi->memsect_attach[idx].base = memsection_base(memsection);
    vspi->memsect_attach[idx].size = memsection_size(memsection);

    cap.ref.obj = memsection;
    cap.passwd = passwd;
    ret = clist_insert(default_clist, cap);

    return ret;
}

static void do_work(struct spi_device *spi_device);

int
virtual_spi_register_control_block_impl
(
    CORBA_Object    _caller,        /* AUTO */
    objref_t        spi_obj,
    uintptr_t       addr,
    idl4_server_environment *_env   /* AUTO */
)
{
    struct virtual_spi *vspi = (struct virtual_spi *)spi_obj;
    void *vaddr;
    /* FIXME: Check valid etc */
    vaddr = (void*) ((uintptr_t) vspi->memsect_attach[(addr & 0xf)].base + (addr >> 4));
    vspi->control = vaddr;

    return 0;
}

static struct stream_pkt *
do_alloc_packet(struct spi_device *spi_device)
{
    struct stream_pkt *retval;
    if (spi_device->freelist == NULL) {
        return NULL;
    }
    retval = spi_device->freelist;
    spi_device->freelist = retval->next;
    return retval;
}

static int
free_tx_packets(struct spi_device *spi_device)
{
    int count = 0;
    int notify = 0;

    while (spi_device->tx->completed_start)
    {
        struct stream_pkt *packet;
        struct stream_pkt *next;

        packet = spi_device->tx->completed_start;
        next = packet->next;

        if (packet->ref)
        {
            struct stream_packet *pkt = (void*)packet->ref;
            pkt->status |= COMPLETED | TERMINATED;
        }

        /* Add to the freelist */
        packet->next = spi_device->freelist;
        spi_device->freelist = packet;

        spi_device->tx->completed_start = next;
        count++;
        notify = 1;
    }

    spi_device->tx->completed_end = NULL;

    if (notify)
    {
        L4_Notify(spi_device->active_virtual->thread, spi_device->active_virtual->mask);
    }
    return count;
}

static int
free_rx_packets(struct spi_device *spi_device)
{
    int count = 0;
    int notify = 0;

    while (spi_device->rx->completed_start)
    {
        struct stream_pkt *packet;
        struct stream_pkt *next;

        packet = spi_device->rx->completed_start;
        next = packet->next;

        if (packet->ref)
        {
            struct stream_packet *pkt = (void*)packet->ref;
            pkt->status |= COMPLETED | TERMINATED;
            pkt->xferred = packet->xferred;
            notify = 1;
        }

        /* Add to the freelist */
        packet->next = spi_device->freelist;
        spi_device->freelist = packet;

        spi_device->rx->completed_start = next;
        count++;
    }

    spi_device->rx->completed_end = NULL;

    /* Notify the receiver */
    if (notify)
    {
            L4_Notify(spi_device->active_virtual->thread, spi_device->active_virtual->mask);
    }
    return count;
}

static void
do_work(struct spi_device *spi_device)
{
    struct virtual_spi *vspi = spi_device->active_virtual;
        if (!vspi) return;

    /* Check for any completed rx packets and send them up */
    free_rx_packets(spi_device);

    /* Add any packets into the driver from the rx things
       being passed down */

#if 0
        DEBUG_PRINT("vspi->rx_cur = %p\n", vspi->rx_cur);
        DEBUG_PRINT("vspi->control = %p\n", vspi->control);
        if (vspi->control)
             DEBUG_PRINT("vspi->control->rx = %u\n", (unsigned)vspi->control->rx);
#endif

    if (vspi->rx_cur == NULL && vspi->control && vspi->control->rx != ~0) {
        /* TX is currently inactive, and we are being asked to add
           some more */
        struct stream_packet *packet;
        packet = S(vspi->control->rx);
        vspi->control->rx = ~0;
        if (packet->next == ~0) {
            packet->status = TERMINATED;
        }
        vspi->rx_cur = packet;
    }

    while(vspi->rx_cur) {
        struct stream_pkt *packet;
        packet = do_alloc_packet(spi_device);

        if (packet == NULL) {
            /* We might have got more packets by now. Unlikely,
               but we have to try, or else we could deadlock */
            if (free_tx_packets(spi_device)) {
                continue;
            }
            break;
        }

        packet->data = C(vspi->rx_cur->data_ptr);
        packet->length = vspi->rx_cur->size;
        packet->xferred = 0;
        packet->ref = vspi->rx_cur;

        if (vspi->rx_cur->status & TERMINATED) {
            /* Latch in the next ring */
            if (vspi->control->rx != ~0) {
                struct stream_packet *pkt = S(vspi->control->rx);
                vspi->control->rx = ~0;
                if (pkt->next == ~0) {
                    pkt->status = TERMINATED;
                }
                vspi->rx_cur = pkt;
            } else {
                vspi->rx_cur = NULL;
            }
        } else {
            struct stream_packet *pkt = S(vspi->rx_cur->next);
            if (pkt->next == ~0) {
                pkt->status = TERMINATED;
            }
            vspi->rx_cur = pkt;
        }
        stream_add_stream_pkt(vspi->spi_device->rx, packet);
    }

    /* TX anything we can */
    free_tx_packets(spi_device);

    if (vspi->tx_cur == NULL && vspi->control && vspi->control->tx != ~0) {
        /* TX is currently inactive, and we are being asked to add
           some more */
        struct stream_packet *packet;
        packet = S(vspi->control->tx);
        vspi->control->tx = ~0;
        if (packet->next == ~0) {
            packet->status = TERMINATED;
        }
        vspi->tx_cur = packet;
    }

    while(vspi->tx_cur) {
        struct stream_pkt *packet;

        packet = do_alloc_packet(spi_device);

        if (packet == NULL) {
            /* We might have got more packets by now. Unlikely,
               but we have to try, or else we could deadlock */
            if (free_tx_packets(spi_device)) {
                continue;
            }
            break;
        }

        packet->data = C(vspi->tx_cur->data_ptr);
        packet->length = vspi->tx_cur->size;
        packet->xferred = 0;
        packet->ref = vspi->tx_cur;

        if (vspi->tx_cur->status & TERMINATED) {
            /* Latch in the next ring */
            if (vspi->control->tx != ~0) {
                struct stream_packet *pkt = S(vspi->control->tx);
                vspi->control->tx = ~0;
                if (pkt->next == ~0) {
                    pkt->status = TERMINATED;
                }
                vspi->tx_cur = pkt;
            } else {
                vspi->tx_cur = NULL;
            }
        } else {
            struct stream_packet *pkt = S(vspi->tx_cur->next);
            if (pkt->next == ~0) {
                pkt->status = TERMINATED;
            }
            vspi->tx_cur = pkt;
        }

        stream_add_stream_pkt(vspi->spi_device->tx, packet);
    }
}


/**
 * Given we use the return value now to indicate chage req, may as well
 * wrap the response.
 */
#define INTERRUPT_AND_NOTIFY(di_,m_) do { \
    if (device_interrupt(di_, m_)) { \
        event_notify(EV_WOMBAT_SWITCH); \
    } \
} while(0)

/**
 * Function called from the server loop when an IRQ is received. See the
 * definition in spi.idl4
 */
void
vspi_irq_handler(L4_ThreadId_t partner)
{
    struct spi_device *device;

    for (device = spi_devices; 
            device != NULL; 
            device = device->next) {
        device_interrupt(device->di, 0);
        do_work(device);
    }
    return;

    /* XXX wtf? */
    switch_active();
}

void
vspi_async_handler(L4_Word_t notify)
{
    struct spi_device *device;

    if (notify & 0x1000){
        L4_ThreadId_t dummy;
        primary_ = primary_wombat();
        for (device = spi_devices; 
             device != NULL; 
             device = device->next) {
            dummy = L4_GlobalId(device->irq, 0);
            vspi_irq_handler(dummy);
        }
    }

    for (device = spi_devices; 
            device != NULL; 
            device = device->next) {
        do_work(device);
    }
}
