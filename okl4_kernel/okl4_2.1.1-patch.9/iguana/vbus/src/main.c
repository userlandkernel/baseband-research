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

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/pd.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/stream.h>
#include <driver/cmd.h>

#include <util/trace.h>

#include <interfaces/vbus_serverdecls.h>
#include <interfaces/devicecore_client.h>

struct bus_device;

struct virtual_bus {
    struct virtual_bus *next;
    struct bus_device * device;

    L4_ThreadId_t thread;
    uintptr_t mask;

};


struct bus_device {
    struct bus_device *next;
    struct virtual_bus * virtuals;

    struct device_interface *di;
    struct cmd_interface *ci;

    struct resource * resources;

};


struct resource * global_resources = NULL;
struct bus_device *bus_devices = NULL;


/**
 * The server entry point.
 */
int
main(void)
{
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    vbus_server_loop();
    return 0;
}

extern struct driver BUS_DRIVER;


cap_t
virtual_bus_factory_create_impl(CORBA_Object _caller, objref_t bus_device,
        L4_ThreadId_t *thread, uintptr_t mask,
        idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };

    struct virtual_bus *vbus;
    struct bus_device *dev = (void*)bus_device;

    vbus = (struct virtual_bus*)malloc(sizeof(struct virtual_bus));
    if (vbus == NULL)
        return cap;

    vbus->device = (void*)dev;
    vbus->thread = *thread;
    vbus->mask   = mask;

    if (dev->virtuals == NULL){
        dev->virtuals = vbus;
    }else{
        dev->virtuals->next = vbus;
    }

    cap.ref.obj = (uintptr_t)vbus;
    return cap;
}

void device_add_resource_impl(L4_ThreadId_t caller, uintptr_t resource, uintptr_t resource2, uint32_t type,
                idl4_server_environment *env)
{
    struct resource *new;
    new = (struct resource*)malloc(sizeof(struct resource));

    if (!new) return;

    switch(type){
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
            new->resource.mem= mem_;
        }
        break;
    case BUS_RESOURCE:
        {
            DEBUG_TRACE(1, "BUS RESOURCE NOT IMPLEMENYED for timer\n");
        }
    default:
        return;
    }

    new->type = (enum resource_type)type;
    new->next = global_resources;
    global_resources = new;
}



cap_t
device_create_impl(CORBA_Object _caller, 
        L4_ThreadId_t *bus,
        uint8_t deviceid, 
        idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct bus_device *bus_device;
    void *device;


    /* Allocate memory for the bus device */
    bus_device = malloc(sizeof(struct bus_device));
    device = malloc(BUS_DRIVER.size);

    if (bus_device == NULL || device == NULL ) {
        free(bus_device);
        free(device);
        return cap;
    }

    /* Initialise the device state */
    bus_device->next = bus_devices;
    bus_devices = bus_device;
    bus_devices->resources = global_resources;
    global_resources = NULL;

    /* Create the device */
    bus_device->di = setup_device_instance(&BUS_DRIVER, device);

    device_setup(bus_device->di, bus_device->resources);

    /* Enable the device */
    device_enable(device);

    /* Get our interfaces up correctly */
    bus_device->ci = (struct cmd_interface *) device_get_interface(bus_device->di, 0);


    cap.ref.obj = (uintptr_t)bus_device;
    return cap;

}

int
virtual_bus_add_memsection_impl(L4_ThreadId_t caller,
        objref_t virtual_bus, objref_t memsection, int idx,
        idl4_server_environment *env)
{
    //    void * base = (void*)memsection_base(memsection);
    //    struct virtual_bus * vbus = (void*)virtual_bus;


    //    vbus->ms = memsection;
    //    vbus->cmd_base = base;
    //    if (!base) return -1;

    //    if (idx != 0) DEBUG_TRACE(2, "WARNING: index is ignored, suggest explicit 0\n");

    DEBUG_TRACE(1, "ADDING ms to bbus?\n");
    return 0;
}

int virtual_bus_register_read8_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset, uint8_t *val, idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA READ 8!\n");
    return 0;
}
int
virtual_bus_register_read16_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset, uint16_t * val,
        idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA READ 16!\n");
    return 0;
}
int
virtual_bus_register_read32_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset,  uint32_t * val,
        idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA READ 16!\n");
    return 0;
}

int
virtual_bus_register_write32_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset, uint32_t val, idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA WRITE 32\n");
    return 0;
}
int
virtual_bus_register_write16_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset, uint16_t val, idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA WRITE 16\n");
    return 0;
}
int
virtual_bus_register_write8_impl(L4_ThreadId_t caller, objref_t virtual_i2c, int offset, uint8_t val, idl4_server_environment *env)
{
    DEBUG_TRACE(4, "WANNA WRITE 8\n");
    return 0;
}

/*
 * Function called from the server loop when an IRQ is received. See the
 * definition in net.idl4
 */
void
vbus_irq_handler(L4_ThreadId_t partner)
{
    DEBUG_TRACE(4, "Got IRQ from %08lx\n", partner.raw);
}

void
vbus_async_handler(L4_Word_t notify)
{
    DEBUG_TRACE(4, "Got ANYNC of %08lx\n", notify);
}
