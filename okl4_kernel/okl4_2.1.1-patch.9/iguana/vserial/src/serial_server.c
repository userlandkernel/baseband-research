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
 * Author: Ben Leslie
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/kdebug.h>
#include <l4/interrupt.h>

#include <util/trace.h>

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

#include <interfaces/vserial_serverdecls.h>

#include <compat/c.h>

/**
 * SERIAL_DRIVER is defined in the build system, and indicates the
 * device driver against which we are compiled.
 *
 */
extern struct driver SERIAL_DRIVER;

#define NUM_PACKETS 20

struct serial_device;

struct memsect_attach {
    void *base;
    uintptr_t size;
};

#define TERMINATED 1
#define COMPLETED 2

#define MEMSECT_MAX 16

/* A "virtual" packet from consumers interacting with vserial devices
   stream_pkt, conversely, is a "real" packet when interacting with the underlying device */
struct stream_packet {
    uintptr_t next;
    uintptr_t data_ptr;
    size_t size;
    size_t xferred;
    int status;
};

/* Get a packet from the context's vserial's memsection at position x
   and cast to a void (V), char (C), or stream packet (S) */
#define V(x) ((void*) ((uintptr_t) vserial->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define C(x) ((uint8_t*) ((uintptr_t) vserial->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define S(x) ((struct stream_packet*) ((uintptr_t) vserial->memsect_attach[(x & 0xf)].base + (x >> 4)))

struct control_block {
    volatile uintptr_t tx;
    volatile uintptr_t rx;
};

struct virtual_serial {
    int                 valid_instance;
    L4_ThreadId_t       thread;
    uintptr_t           mask;

    struct serial_device    *serial_device;
    struct virtual_serial   *next;

    struct memsect_attach   memsect_attach[MEMSECT_MAX];
    struct control_block    *control;

    struct stream_packet    *tx_cur;
    struct stream_packet    *rx_cur;
    
    void *si_start, *si_end, *si_comp_start, *si_comp_end;
};

struct serial_device {
    struct serial_device    *next;
    struct resource         *resources;
    
    struct device_interface *di;
    struct stream_interface *tx;
    struct stream_interface *rx;
    struct stream_pkt       *freelist;
    
    struct virtual_serial   *active_virtual;
    struct virtual_serial   *last_virtual;
    
    int                     irq;
};

/* Statically allocated memory */
/*
 * NOTES:
 *
 * So the following variables need to be in the .data section.  By default,
 * these will go into .bss because they're uninitialised.
 *
 * You can force them to go into .data by using the SECTION macro (defined in
 * libs/compat).  Unforunately it only works on the gnu and rvct_gnu toolchains,
 * but not plain rvct.
 *
 * So a more hacky way is used where these variables get initialised with
 * bogus, non-zero data.
 *
 * A better way to do it would be to define a macro ELFWEAVER_PATCHED and
 * force them into the data section depending on the toolchain used.  For
 * plain rvct, use #pragma's to achieve this.
 *
 * Finally, given that these variables are shared across all device servers,
 * they should be defined in a library.
 *
 * - nt
 */
static int         buffer[8] = { 0x01234567 };
static mem_space_t iguana_granted_physmem[4] = { (mem_space_t)0xdeadbeef }; /* XXX: will not work if no physmem is granted */
static int         iguana_granted_interrupt[4] = { 0xffffffff }; /* will always work because irq -1 is invalid */
static struct virtual_serial virtual_device_instance[4] = { { 1 } }; /* valid but unused instances are ok */

static struct resource          resources[8];
static struct serial_device     serial_device;

static struct stream_pkt        stream_packet[NUM_PACKETS];

/**
 * The server entry point.
 */
int
main(int argc, char *argv[])
{
#if 0
    /*
     * XXX: Debugging section.  Remove later.
     *
     * Move me to a library.  Use as diagnosis
     */
    printf("In vserial now\n");
    printf("Let's have a look at what I got:\n");
    printf("Physmem:  \n\t1. %p\n\t2. %p\n\t3. %p\n\t4. %p\n",
        iguana_granted_physmem[0], iguana_granted_physmem[1],
        iguana_granted_physmem[2], iguana_granted_physmem[3]);
    printf("Interrupt:\n\t1. 0x%x\n\t2. 0x%x\n\t3. 0x%x\n\t4. 0x%x\n",
        iguana_granted_interrupt[0], iguana_granted_interrupt[1],
        iguana_granted_interrupt[2], iguana_granted_interrupt[3]);
    printf("Virtual devices: %d %d %d %d\n",
        virtual_device_instance[0].valid_instance,
        virtual_device_instance[1].valid_instance,
        virtual_device_instance[2].valid_instance,
        virtual_device_instance[3].valid_instance);
#endif
    int i, r;
    void *device;

    /*
     * XXX: Was: device_add_resource_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */
    for (i = 0, r = 0; i < 4; i++)
    {
        if (iguana_granted_physmem[i] != NULL)
        {
            resources[r].type = MEMORY_RESOURCE;
            resources[r++].resource.mem = iguana_granted_physmem[i];
        }
        if (iguana_granted_interrupt[i] != -1)
        {
            resources[r].type = INTERRUPT_RESOURCE;
            resources[r++].resource.interrupt = iguana_granted_interrupt[i];
        }
    }
    for (/*none*/; r < 8; r++)
    {
        resources[r].type = NO_RESOURCE;
    }
        
    /*
     * XXX: Was: device_create_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */
    device = malloc(SERIAL_DRIVER.size);
    
    if (device == NULL) {
        free(device);
    }    
    
    /* Initialise the device state */
    {
        serial_device.next = NULL;
        serial_device.resources = resources;
    }
    
    /* Create and enable the device */
    serial_device.di = setup_device_instance(&SERIAL_DRIVER, device);
    device_setup(serial_device.di, serial_device.resources);
    
    serial_device.tx = (struct stream_interface *) device_get_interface(serial_device.di, 0);
    serial_device.rx = (struct stream_interface *) device_get_interface(serial_device.di, 1);
    device_enable(serial_device.di);
    
    /* Create a free list of packet that we can send to the device */
    serial_device.freelist = stream_packet;
    for (i = 0; i < NUM_PACKETS - 1; i++)
    {
        serial_device.freelist[i].next = &serial_device.freelist[i+1];
    }
    serial_device.freelist[i].next = NULL;
        
    /*
     * XXX: Was: virtual_serial_factory_create_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */
    for (i = 0; i < 4; i++)
    {
        if (virtual_device_instance[i].valid_instance)
        {
            virtual_device_instance[i].serial_device = (void *)(&serial_device);
            // XXX virtual_device_instance[i].thread = *thread;
            // XXX virtual_device_instance[i].mask = mask;
            
            virtual_device_instance[i].si_start = virtual_device_instance[i].si_end = NULL;
            virtual_device_instance[i].si_comp_end = virtual_device_instance[i].si_comp_start = NULL;

            if (serial_device.active_virtual == NULL)
            {
                serial_device.active_virtual    = &virtual_device_instance[i];
                serial_device.last_virtual      = &virtual_device_instance[i];
            }
            else
            {
                serial_device.last_virtual->next = &virtual_device_instance[i];
                serial_device.last_virtual       = &virtual_device_instance[i];
            }
        }
    }
        
    event_init();
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    vserial_server_loop();
    return 0;

    (void)iguana_granted_physmem;
    (void)iguana_granted_interrupt;
    (void)buffer;
    (void)virtual_device_instance;
}

#define TIMER_HACK

#ifdef TIMER_HACK
L4_ThreadId_t devicecore_tid;
#endif

static void switch_active(void)
{
    struct virtual_serial * this = serial_device.active_virtual;
    struct virtual_serial * next = this->next;
    struct virtual_serial * last = serial_device.last_virtual;

    if (next == NULL){
        DEBUG_TRACE(2, "No other virtual serials to make active\n");
        return;
    }

    this->next = NULL;
    serial_device.active_virtual = next;
    
    last->next = this;
    serial_device.last_virtual = this;

    this->si_start      = serial_device.rx->start;
    this->si_end        = serial_device.rx->end;
    this->si_comp_end   = serial_device.rx->completed_end;
    this->si_comp_start = serial_device.rx->completed_start;


    serial_device.rx->start           = next->si_start;
    serial_device.rx->end             = next->si_end;
    serial_device.rx->completed_end   = next->si_comp_end;
    serial_device.rx->completed_start = next->si_comp_start;

    DEBUG_TRACE(2, "\nSwitching to Client  0x%lx\n",
           next->thread.raw);
}

volatile int sched_flip = 0;

int
virtual_serial_add_memsection_impl(CORBA_Object _caller, device_t handle, objref_t memsection, uintptr_t passwd, int idx, idl4_server_environment * _env)
{
    struct virtual_serial *vserial = virtual_device_instance + handle;
    cap_t cap;
    int ret;

    if (idx > MEMSECT_MAX) {
        /* ERROR */
        return -1;
    }
    vserial->memsect_attach[idx].base = memsection_base(memsection);
    vserial->memsect_attach[idx].size = memsection_size(memsection);

    cap.ref.obj = memsection;
    cap.passwd = passwd;
    ret = clist_insert(default_clist, cap);

    return ret;
}

static void do_work(struct serial_device *serial_device);

int
virtual_serial_register_control_block_impl(CORBA_Object _caller, device_t handle, uintptr_t addr, idl4_server_environment *_env)
{
    struct virtual_serial *vserial = virtual_device_instance + handle;
    void *vaddr;
    
    /* FIXME: Check valid etc */
    vaddr = (void*) ((uintptr_t) vserial->memsect_attach[(addr & 0xf)].base + (addr >> 4));
    vserial->control = vaddr;

    return 0;
}

/* Shutdown a serial device.
   To be called from an application on sys_exit to notify us that the serial device will
   no longer be used, and that the shared memsection is no longer valid.
   Send all remaining packets, release the shared memsection, then free resources */
void
virtual_serial_shutdown_impl(CORBA_Object _caller, device_t handle, idl4_server_environment *_env)
{
    /* Obtain the vserial instance and the underlying serial device */
    struct virtual_serial *vserial = virtual_device_instance + handle;
    struct serial_device *serial_device = vserial->serial_device;

    struct stream_pkt *real_pkt, *last_pkt, *next_pkt;
    int found, memsect_idx;
    struct virtual_serial *previous, *current;

    /* Transmit everything available and sync the stream */
    do_work(serial_device);
    stream_sync(serial_device->tx);

    /* Release any RX packets.
       Look at each RX packet in the stream and see if it is within
       any of our memsections. */
    real_pkt = stream_get_head(serial_device->rx);
    last_pkt = NULL; /* The last "good" head of the stream's list */
    next_pkt = NULL;
    while (real_pkt != NULL) {

        /* Determine if the rx packet was in one of our memsection */
        // TODO: possibly extend stream_packet to reference the owning vserial
        // we can get stream_packet from stream_pkt by stream_pkt->ref
        found = 0;
        for (memsect_idx = 0; memsect_idx < MEMSECT_MAX; memsect_idx++) {
            struct memsect_attach memsect = vserial->memsect_attach[memsect_idx];
            char* real_pkt_data = (char*)real_pkt->data;
            char* memsect_data_start = (char*)memsect.base;
            char* memsect_data_end = (char*)memsect.base + memsect.size;
            if (real_pkt_data >= memsect_data_start && real_pkt_data < memsect_data_end) {
                found = 1;
                break;
            }
        }

        /* If it is one of our packets, remove it from the stream's list and destroy it.
           We cache real_pkt->next here because stream_switch_head nulls it out */
        next_pkt = real_pkt->next;
        if (found != 0) {
            if (last_pkt == NULL) {
                stream_switch_head(serial_device->rx);
            } else {
                last_pkt->next = real_pkt->next;
            }
        } else {
            last_pkt = real_pkt;
        }
        real_pkt = next_pkt;
    }

    /* Remove the vserial from the list */
    previous = NULL;
    current = serial_device->active_virtual;
    while (current != NULL) {
        if (current == vserial) {
            if (previous != NULL) {
                previous->next = current->next;
            } else {
                serial_device->active_virtual = NULL;
            }
            if (current->next == NULL) {
                serial_device->last_virtual = previous;
            }
            break;
        } else {
            previous = current;
            current = current->next;
        }
    }
}

int
virtual_serial_init_impl(CORBA_Object _caller, device_t handle, L4_Word_t owner, uint32_t mask, idl4_server_environment * _env)
{
    struct virtual_serial *vserial = virtual_device_instance + handle;

    /*
     * Owner will be the thread that is notified when an event occurs
     * on the vserial handle.  Therefore 'owner' must always be a
     * global tid, never myselfconst nor thread handle.  IPC endpoints
     * are ok.
     *
     * The 'caller' argument is always going to be a thread handle, so
     * it is useless in this function.
     */
    L4_ThreadId_t   owner_tid;
    owner_tid.raw = owner;

    assert(!L4_IsThreadEqual(owner_tid, L4_nilthread));
    assert(!L4_IsThreadEqual(owner_tid, L4_myselfconst));
    /* assert owner is not thread handle? */

    if (!vserial->valid_instance)
    {
        printf("vserial: accessing invalid instance (handle: %d)\n", handle);
        return -1;
    }

    /* Re-set the owner if owner argument is different from the existing owner */
    if (!L4_IsThreadEqual(owner_tid, vserial->thread))
    {
        if (!L4_IsNilThread(vserial->thread))
        {
            printf
            (
                "vserial: re-setting owner of instance (old_owner: %lx, new_owner: %lx, handle: %d)\n",
                vserial->thread.raw, owner_tid.raw, handle
            );
        }

        vserial->thread = owner_tid;
    }

    vserial->mask = mask;

    printf
    (
        "vserial: init done (handle: %d, owner: %lx, mask: %x)\n",
        handle, owner_tid.raw, (unsigned int)mask
    );

    return 0;
}

/* Pop a packet off the free list for the underlying device */
static struct stream_pkt *
do_alloc_packet(struct serial_device *serial_device)
{
    struct stream_pkt *retval;
    if (serial_device->freelist == NULL) {
        return NULL;
    }
    retval = serial_device->freelist;
    serial_device->freelist = retval->next;
    return retval;
}

static int
free_packets(struct serial_device *serial_device)
{
    int count = 0;
    int notify = 0;
    while (serial_device->tx->completed_start) {
        struct stream_pkt *packet;
        struct stream_pkt *next;

        //L4_KDB_PrintChar('^');

        packet = serial_device->tx->completed_start;
        next = packet->next;

        if (packet->ref) {
            struct stream_packet *pkt = (void*)packet->ref;
            pkt->status |= COMPLETED | TERMINATED;
        }

        /* Add to the freelist */
        packet->next = serial_device->freelist;
        serial_device->freelist = packet;

        serial_device->tx->completed_start = next;
        count++;
        notify = 1;
    }
    serial_device->tx->completed_end = NULL;
    if (notify) {
        L4_Notify(serial_device->active_virtual->thread, serial_device->active_virtual->mask);
    }
    return count;
}

static int
free_rx_packets(struct serial_device *serial_device)
{
    int count = 0;
    int notify = 0;
    while (serial_device->rx->completed_start) {
        struct stream_pkt *packet;
        struct stream_pkt *next;

        //L4_KDB_PrintChar('^');

        packet = serial_device->rx->completed_start;
        next = packet->next;

        if (packet->ref) {
            struct stream_packet *pkt = (void*)packet->ref;
            pkt->status |= COMPLETED | TERMINATED;
            pkt->xferred = packet->xferred;
            notify = 1;
        } 

        /* Add to the freelist */
        packet->next = serial_device->freelist;
        serial_device->freelist = packet;

        serial_device->rx->completed_start = next;
        count++;
    }
    serial_device->rx->completed_end = NULL;
    /* Notify the receiver */
    if (notify) {
        L4_Notify(serial_device->active_virtual->thread, serial_device->active_virtual->mask);
    }

    return count;
}

static void
do_work(struct serial_device *serial_device)
{
    struct virtual_serial *vserial = serial_device->active_virtual;
    if (!vserial) return;

    /* Receiving */

    /* Check for any completed rx packets and send them up */
    free_rx_packets(serial_device);

    /* Add any packets into the driver from the rx things
       being passed down */

#if 0
    DEBUG_TRACE(2, "vserial->rx_cur = %p\n", vserial->rx_cur);
    DEBUG_TRACE(2, "vserial->control = %p\n", vserial->control);
    if (vserial->control)
        DEBUG_TRACE(2, "vserial->control->rx = %u\n", (unsigned)vserial->control->rx);
#endif

    if (vserial->rx_cur == NULL && vserial->control &&
            vserial->control->rx != ~0) {
        /* TX is currently inactive, and we are being asked to add
           some more */
        struct stream_packet *packet;
        packet = S(vserial->control->rx);
        vserial->control->rx = ~0;
        if (packet->next == ~0) {
            packet->status = TERMINATED;
        }
        vserial->rx_cur = packet;
    }

    while(vserial->rx_cur) {
        struct stream_pkt *packet;
        packet = do_alloc_packet(serial_device);

        if (packet == NULL) {
            /* We might have got more packets by now. Unlikely,
               but we have to try, or else we could deadlock */
            if (free_packets(serial_device)) {
                continue;
            }
            break;
        }

        packet->data = C(vserial->rx_cur->data_ptr);
        packet->length = vserial->rx_cur->size;
        packet->xferred = 0;
        packet->ref = vserial->rx_cur;

        if (vserial->rx_cur->status & TERMINATED) {
            /* Latch in the next ring */
            if (vserial->control->rx != ~0) {
                struct stream_packet *pkt = S(vserial->control->rx);
                vserial->control->rx = ~0;
                if (pkt->next == ~0) {
                    pkt->status = TERMINATED;
                }
                vserial->rx_cur = pkt;
            } else {
                vserial->rx_cur = NULL;
            }
        } else {
            struct stream_packet *pkt = S(vserial->rx_cur->next);
            if (pkt->next == ~0) {
                pkt->status = TERMINATED;
            }
            vserial->rx_cur = pkt;
        }
        stream_add_stream_pkt(vserial->serial_device->rx, packet);
    }

    /* TX anything we can */
    free_packets(serial_device);

    /* If we don't currently have anything to transmit for this
       vserial, see if we can obtain something from the control block
       Remember the control block is at the start of the shared memsection */
    if (vserial->tx_cur == NULL && vserial->control &&
            vserial->control->tx != ~0) {
        /* TX is currently inactive, and we are being asked to add
           some more */

        /* Obtain the packet at the start of the control block's tx */
        struct stream_packet *packet;
        packet = S(vserial->control->tx);

        /* Null out the tx in the control block */
        vserial->control->tx = ~0;

        /* If there is no next, set it to terminated */
        if (packet->next == ~0) {
            packet->status = TERMINATED;
        }

        /* Set the current tx packet to this packet */
        vserial->tx_cur = packet;
    }

    /* While we have packets to transmit */
    while(vserial->tx_cur) {

        /* Obtain a packet from the free list of the REAL device
           stream_pkt is a packet in the real device, whereas
           stream_packet is a packet in the virtual device */
        struct stream_pkt *packet;
        packet = do_alloc_packet(serial_device);

        /* If we couldn't obtain a packet from the free list of the REAL device
           see if we can free some. If so, re-start this loop */
        if (packet == NULL) {
            /* We might have got more packets by now. Unlikely,
               but we have to try, or else we could deadlock */
            if (free_packets(serial_device)) {
                continue;
            }
            break;
        }

        /* Set the attributes of this real packet from the virtual packet */
        packet->data = C(vserial->tx_cur->data_ptr);
        packet->length = vserial->tx_cur->size;
        packet->xferred = 0;
        packet->ref = vserial->tx_cur;

        /* If the virtual packet is terminated (ie: it was the last packet in this list)
           "latch in the next ring" (ie: get the next list of packets to send from the control
           block, if there is one. if there isn't, quit the loop)
           If the virtual packet is not terminated, obtain the next virtual packet in this list of packets */
        if (vserial->tx_cur->status & TERMINATED) {
            /* Latch in the next ring */
            if (vserial->control->tx != ~0) {
                //L4_KDB_PrintChar(':');
                struct stream_packet *pkt = S(vserial->control->tx);
                vserial->control->tx = ~0;
                if (pkt->next == ~0) {
                    pkt->status = TERMINATED;
                }
                vserial->tx_cur = pkt;
            } else {
                //L4_KDB_PrintChar(';');
                vserial->tx_cur = NULL;
            }
        } else {
            //L4_KDB_PrintChar('\\');
            struct stream_packet *pkt = S(vserial->tx_cur->next);
            if (pkt->next == ~0) {
                pkt->status = TERMINATED;
            }
            vserial->tx_cur = pkt;
        }
        stream_add_stream_pkt(vserial->serial_device->tx, packet);

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
 * The irq_handler function is not needed anymore as an interface function but is
 * still called from async_handler.  The best solution is to take it out of the
 * IDL4 interface and let it remain as a static function. - nt
 */
static void vserial_irq_handler(L4_ThreadId_t partner);

static void
vserial_irq_handler(L4_ThreadId_t partner)
{
    // XXX: fix the irq storm that happens when this is uncommented
    // printf("vserial: handling irq %lx\n", (L4_Word_t)partner.raw);
    
    device_interrupt(serial_device.di, L4_ThreadNo(partner));
    do_work(&serial_device);
    
//        if (L4_ThreadNo(partner) == device->irq) {
//            if ((device->active_virtual->thread.raw != primary_.raw) &&
//                    (device->last_virtual != NULL)) {
//                sched_flip = 1;
//            }
//            INTERRUPT_AND_NOTIFY(device->di, 0);
//            while (sched_flip){
//                do_work(device);
//                switch_active();
//                sched_flip = 0;
//                INTERRUPT_AND_NOTIFY(device->di, 0);
//            }
//            do_work(device);
//        }
    
    return;
    
    switch_active();
}

void
vserial_async_handler(L4_Word_t notify)
{
    if (notify & (1UL << 31)) {
        L4_Word_t irq;

        irq = __L4_TCR_PlatformReserved(0);

        //printf("%s: In async handler : %lx : %lu\n", __func__, notify, irq);
        device_interrupt(serial_device.di, irq);

        // ack IRQ
        L4_LoadMR(0, irq);
        L4_AcknowledgeInterrupt(0, 0);
    }

    if (notify & 0x1000)
    {
        L4_ThreadId_t dummy;
        dummy = L4_GlobalId(serial_device.irq, 0);
        vserial_irq_handler(dummy);

        // primary_ = primary_wombat();
    }

    do_work(&serial_device);
}
