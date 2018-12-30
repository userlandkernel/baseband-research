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
 * Author: Ben Leslie, Carl van Schaik
 */
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/cap.h>
#include <iguana/object.h>
#include <iguana/thread.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/stream.h>

#include <event/event.h>
#include <interfaces/vtouch_serverdecls.h>

#include <vtimer/timer.h>
#include <interfaces/devicecore_client.h>
#include <interfaces/vtimer_client.h>

#include <util/trace.h>

#include <l4/misc.h>


/**
 * TOUCH_DRIVER is defined in the build system, and indicates the
 * device driver against which we are compiled.
 *
 */
extern struct driver TOUCH_DRIVER;

#define NUM_PACKETS 20

struct touch_device;

struct memsect_attach {
    void *base;
    uintptr_t size;
};

#define TERMINATED 1
#define COMPLETED 2


struct stream_packet {
    uintptr_t next;
    uintptr_t data_ptr;
    size_t size;
    size_t xferred;
    int status;
};

#define V(x) ((void*) ((uintptr_t) vtouch->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define C(x) ((char*) ((uintptr_t) vtouch->memsect_attach[(x & 0xf)].base + (x >> 4)))
#define S(x) ((struct stream_packet*) ((uintptr_t) vtouch->memsect_attach[(x & 0xf)].base + (x >> 4)))

struct control_block {
    volatile uintptr_t input;
};

typedef int (*tm_callback_t)(struct device_interface *);
void timeout_request(struct device_interface *device, uintptr_t nanosecs, tm_callback_t call);

struct touch_device;

struct virtual_touch {
    struct virtual_touch   *next;
    struct touch_device    *touch_device;
    L4_ThreadId_t         thread;
    uintptr_t             mask;
    struct memsect_attach memsect_attach[16];
    struct control_block *control;
    struct stream_packet *input_cur;
};

struct touch_device {
    struct touch_device     *next;
    struct device_interface *di;
    struct stream_interface *input;
    struct stream_pkt       *freelist;
    int                      irq;
    struct virtual_touch    *active_virtual;
    struct virtual_touch    *last_virtual;
    struct resource         *resources;
    /* timer interface */
    objref_t                 timer_device;
    L4_ThreadId_t            timer_server, device_core;
    tm_callback_t            timer_callback;
};

struct touch_device *touch_devices = NULL;
struct resource *global_resources = NULL;

static L4_ThreadId_t primary_; /**< The thread id of the current
                                  "priamry" wombat instance */

void init_timer_service(struct touch_device *touch);

/**
 * The server entry point.
 */
int
main(void)
{
    event_init();
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    register_event(thread_l4tid(env_thread(iguana_getenv("MAIN"))), 0x1000, EV_WOMBAT_SWITCH);
    vtouch_server_loop();
    return 0;
}

/*
 * XXX auto-generate this code?  Should be mostly the same for all servers.
 */
void
device_add_resource_impl(L4_ThreadId_t caller, 
                         uintptr_t resource, 
                         uintptr_t resource2, 
                         uint32_t type,
                         idl4_server_environment *env)
{
    struct resource *new_r;
    uintptr_t memory, memory_ptr;

    if ((new_r = malloc(sizeof(*new_r))) == NULL)
        return;

    switch (type) {
        case INTERRUPT_RESOURCE:
            new_r->resource.interrupt = resource;
            break;
        case MEMORY_RESOURCE:
            /* XXX - what to do if memsection_create_user() fail? -gl */
            memory = memsection_create_user(0x1000, &memory_ptr);
            hardware_back_memsection(memory, resource, L4_IOMemory);
            new_r->resource.mem = (char *)memory_ptr;
            break;
        case BUS_RESOURCE:
            DEBUG_TRACE(1, "No BUS_RESOURCE yet for vtouch\n");
            return;
        default:
            DEBUG_TRACE(1, "warning: unknown resource type %x\n",
                        (unsigned int)type);
            return;
     }

     new_r->type = type;
     new_r->next = global_resources;
     global_resources = new_r;
}

cap_t
device_create_impl(CORBA_Object _caller,
        L4_ThreadId_t *bus,
        uint8_t deviceid, uintptr_t addr, uintptr_t irq,
        idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct touch_device *touch_device;
    struct resource *r;
    void *device, *stream_packet;

    /* Allocate memory for the touch device */
    touch_device = malloc(sizeof(struct touch_device));
    device = malloc(TOUCH_DRIVER.size);
    stream_packet = malloc(sizeof(struct stream_pkt) * NUM_PACKETS);

    if (touch_device == NULL || device == NULL || stream_packet == NULL) {
        free(touch_device);
        free(device);
        free(stream_packet);
        return cap;
    }

    /* Initialise the device state */
    touch_device->next = touch_devices;
    touch_devices = touch_device;

    touch_device->device_core = _caller;
    touch_device->timer_device = 0;
    touch_device->active_virtual = NULL;

    touch_device->resources = global_resources;
    global_resources = NULL;

    touch_device->irq = irq;   /* XXX still need to store this?  -gl */

    /* Create the device */
    touch_device->di = setup_device_instance(&TOUCH_DRIVER, device);

    device_setup(touch_device->di, touch_device->resources);

    /* Enable the device */
    device_enable(device);

    for (r = touch_device->resources; r != NULL; r = r->next) {
        if (r->type == INTERRUPT_RESOURCE)
            hardware_register_interrupt(thread_l4tid(env_thread(iguana_getenv("MAIN"))), r->resource.interrupt);
    }

    /* Get our interfaces up correctly */
    touch_device->input = (struct stream_interface *) device_get_interface(touch_device->di, 0);

    /* Create a free list of packet that we can send to the device */
    touch_device->freelist = stream_packet;
    int i;
    for (i=0; i < NUM_PACKETS - 1; i++) {
        touch_device->freelist[i].next = &touch_device->freelist[i+1];
    }
    touch_device->freelist[i].next = NULL;

    cap.ref.obj = (uintptr_t)touch_device;
    return cap;
}

cap_t
virtual_touch_factory_create_impl(CORBA_Object _caller, objref_t touch_device,
        L4_ThreadId_t *thread, uintptr_t mask,
        idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct virtual_touch *vtouch;
    struct touch_device *dev = (void*)touch_device;

    primary_ = primary_wombat();

    vtouch = malloc(sizeof(struct virtual_touch));

    if (vtouch == NULL) {
        return cap;
    }

    vtouch->touch_device = (void*) touch_device;
    vtouch->thread = *thread;
    vtouch->mask = mask;
    vtouch->next = NULL;

    if (dev->active_virtual == NULL) {
        dev->active_virtual = vtouch;
        dev->last_virtual = vtouch;
    } else {
        dev->last_virtual->next = vtouch;
        dev->last_virtual = vtouch;
    }

    cap.ref.obj = (uintptr_t)vtouch;

    return cap;
}


int
virtual_touch_add_memsection_impl(CORBA_Object _caller, objref_t touch_obj,
        objref_t memsection, uintptr_t passwd, int idx,
        idl4_server_environment * _env)
{
    struct virtual_touch *vtouch = (struct virtual_touch *)touch_obj;
    cap_t cap;
    int ret;

    if (idx > 16) {
        /* ERROR */
        return -1;
    }
    vtouch->memsect_attach[idx].base = memsection_base(memsection);
    vtouch->memsect_attach[idx].size = memsection_size(memsection);

    cap.ref.obj = memsection;
    cap.passwd = passwd;

    ret = clist_insert(default_clist, cap);

    return ret;
}

int
virtual_touch_register_control_block_impl(CORBA_Object _caller, objref_t touch_obj,
        uintptr_t addr,
        idl4_server_environment *_env)
{
    struct virtual_touch *vtouch = (struct virtual_touch *)touch_obj;
    void *vaddr;

    /* FIXME: Check valid etc */
    vaddr = (void*) ((uintptr_t) vtouch->memsect_attach[(addr & 0xf)].base + (addr >> 4));
    vtouch->control = vaddr;

    /* XXX setup timer - move somwhere better? */
    if (vtouch->touch_device->timer_device == 0) {
        init_timer_service(vtouch->touch_device);
    }

    return 0;
}

static int
free_rx_packets(struct touch_device *touch_device)
{
    int count = 0;
    int notify = 0;
    while (touch_device->input->completed_start) {
        struct stream_pkt *packet;
        struct stream_pkt *next;

        packet = touch_device->input->completed_start;
        next = packet->next;

        if (packet->ref) {
            struct stream_packet *pkt = (void*)packet->ref;
            pkt->status |= COMPLETED | TERMINATED;
            pkt->xferred = packet->xferred;
            //L4_KDB_PrintChar('v');
            notify = 1;
        }

        /* Add to the freelist */
        packet->next = touch_device->freelist;
        touch_device->freelist = packet;

        touch_device->input->completed_start = next;
        count++;
    }
    touch_device->input->completed_end = NULL;
    /* Notify the receiver */
    if (notify) {
        L4_Notify(touch_device->active_virtual->thread, touch_device->active_virtual->mask);
    }

    return count;
}

#if 0
/*
 * XXX changed by gl:
 *
 * Disable switching code until Davidm has interface nutted out. -gl
 */
static
void switch_active(void)
{
    struct virtual_touch * this = touch_devices->active_virtual;
    struct virtual_touch * next = this->next;
    struct virtual_touch * last = touch_devices->last_virtual;

    if (next == NULL){
        DEBUG_TRACE(2, "No other virtual touchs to make active\n");
        return;
    }

    this->next = NULL;
    touch_devices->active_virtual = next;

    last->next = this;
    touch_devices->last_virtual = this;

}
#endif

static struct stream_pkt *
do_alloc_packet(struct touch_device *touch_device)
{
    struct stream_pkt *retval;
    if (touch_device->freelist == NULL) {
        return NULL;
    }
    retval = touch_device->freelist;
    touch_device->freelist = retval->next;
    return retval;
}

static void
do_work(struct touch_device *touch_device)
{
    struct virtual_touch *vtouch = touch_device->active_virtual;
    
    // nothing to do if we have no interested clients
    if (!vtouch) return;    

    /* Check for any completed rx packets and send them up */
    free_rx_packets(touch_device);

    /* Add any packets into the driver from the rx things
       being passed down */
    if (vtouch->input_cur == NULL && vtouch->control->input != ~0) {
        /* TX is currently inactive, and we are being asked to add
           some more */
        struct stream_packet *packet;
        packet = S(vtouch->control->input);
        vtouch->control->input = ~0;
        if (packet->next == ~0) {
            packet->status = TERMINATED;
        }
        vtouch->input_cur = packet;
    }

    while(vtouch->input_cur) {
        struct stream_pkt *packet;
        packet = do_alloc_packet(touch_device);

        if (packet == NULL) {
            /* We might have got more packets by now. Unlikely,
               but we have to try, or else we could deadlock */
            if (free_rx_packets(touch_device)) {
                continue;
            }
            break;
        }

        packet->data = C(vtouch->input_cur->data_ptr);
        packet->length = vtouch->input_cur->size;
        packet->xferred = 0;
        packet->ref = vtouch->input_cur;

        if (vtouch->input_cur->status & TERMINATED) {
            /* Latch in the next ring */
            if (vtouch->control->input != ~0) {
                struct stream_packet *pkt = S(vtouch->control->input);
                vtouch->control->input = ~0;
                if (pkt->next == ~0) {
                    pkt->status = TERMINATED;
                }
                vtouch->input_cur = pkt;
            } else {
                vtouch->input_cur = NULL;
            }
        } else {
            struct stream_packet *pkt = S(vtouch->input_cur->next);
            if (pkt->next == ~0) {
                pkt->status = TERMINATED;
            }
            vtouch->input_cur = pkt;
        }

        stream_add_stream_pkt(vtouch->touch_device->input, packet);
    }
}

volatile int touch_sched_flip = 0;

/*
 * Function called from the server loop when an IRQ is received. See the
 * definition in serial.idl4
 */
void
vtouch_irq_handler(L4_ThreadId_t partner)
{
    struct touch_device *device;
#if 0
    L4_Word_t num;
    L4_StoreMR(1, &num);
#endif

    //DEBUG_TRACE(2, "vtouch: irq - %lx\n", partner.raw);
    for (device = touch_devices;
            device != NULL;
            device = device->next) {
#if 1
/*
 * Changed by gl:
 *
 * Disable the switching code for now until davidm has that nutted out.
 *
 * -gl
 */
        if (device_interrupt(device->di, L4_ThreadNo(partner)))
            do_work(device);
#else
        int irq = device->irq & ~(1<<31);
        if ( (L4_ThreadNo(partner) == device->irq) ||
                ( (device->irq & (1<<31)) && (num==irq) ))
        {
            if (!device->active_virtual)
                continue;
            if ((device->active_virtual->thread.raw != primary_.raw) &&
                    (device->last_virtual != NULL)) {
                touch_sched_flip = 1;
            }

            if (touch_sched_flip) {
                switch_active();
            }

            device_interrupt(device->di, 0);
            touch_sched_flip = 0;
        }
#endif
    }
}

#define TIMER_MASK    0x8000

void
vtouch_async_handler(L4_Word_t notify)
{
    struct touch_device *device;

    if (notify & TIMER_MASK) {
        for (device = touch_devices;
                device != NULL;
                device = device->next)
        {
            // XXX not supported multple drivers - need different async bit?
            if (device->timer_callback) {
                tm_callback_t fn = device->timer_callback;
                device->timer_callback = NULL;

                if (fn(device->di))
                    do_work(device);
            }
        }
        notify &= ~(TIMER_MASK);
    }

    if (notify & 0x1000) {
        L4_ThreadId_t dummy;
        primary_ = primary_wombat();
        for (device = touch_devices;
                device != NULL;
                device = device->next)
        {
            dummy = L4_GlobalId(device->irq, 0);
            vtouch_irq_handler(dummy);
        }
        notify &= ~(0x1000);
    }

    if (notify) {
        for (device = touch_devices;
                device != NULL;
                device = device->next)
        {
            do_work(device);
        }
    }
    return;
}

void
timeout_request(struct device_interface *device, uintptr_t nanosecs, tm_callback_t call)
{
    CORBA_Environment env;
    struct touch_device *touch;

    for (touch = touch_devices;
            touch != NULL;
            touch = touch->next)
    {
        if (touch->di == device) {
            if (touch->timer_server.raw == 0)
                init_timer_service(touch);
            touch->timer_callback = call;
            virtual_timer_request(touch->timer_server, touch->timer_device,
                    nanosecs, TIMER_ONESHOT, &env);
            return;
        }
    }
}

void init_timer_service(struct touch_device *touch)
{
    L4_ThreadId_t me = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    touch->timer_device = device_core_get_vdevice(touch->device_core, 
                                                &(touch->timer_server),
                                                &me,
                                                TIMER_MASK,
                                                "timer", 
                                                NULL);
}

