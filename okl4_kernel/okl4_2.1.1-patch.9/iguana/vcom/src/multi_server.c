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
 * Copyright (c) 2004, National ICT Australia
 */

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <interfaces/vmulti_serverdecls.h>

#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/physmem.h>
#include <iguana/cap.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/stream.h>

#include <event/event.h>
#include <driver/fb.h>

#include <l4/misc.h>

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

struct memsect_attach
{
    void        *base;
    uintptr_t   size;
};

struct control_block
{
    void * head;
    void * tail;
    void * end;
    uint8_t data[];
};

struct multi_device;

struct virtual_multi
{
    struct virtual_multi      *next;
    struct multi_device       *multi_device;
    L4_ThreadId_t           thread;
    uintptr_t               mask;
    struct memsect_attach   memsect_attach[16];

    uintptr_t               pass;
    uintptr_t               ms;

    struct control_block    *control;

};

struct multi_device
{
    struct multi_device       *next;
    struct device_interface *di;

    struct virtual_multi      *active_virtual;
    struct virtual_multi      *last_virtual;

};

struct multi_device * multi_devices = NULL;

int
main(void)
{
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    vmulti_server_loop();
    return 0;
}

void 
device_add_resource_impl(L4_ThreadId_t caller, 
        uintptr_t resource, uintptr_t resource2, int type, 
        idl4_server_environment *env)
{
    // do nothing for this driver
}


cap_t
device_create_impl(CORBA_Object _caller,
                   L4_ThreadId_t *bus,
                   uint8_t deviceid,
                   idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct multi_device *multi_device;
    printf("THIS IS THE STATUP OF THE MULTI?\n");

    /* Allocate memory for the device */
    multi_device = malloc(sizeof(struct multi_device));

    if (multi_device == NULL ) {
        free(multi_device);
        return cap;
    }

    /* Initialise the device state */
    multi_device->next = multi_devices;
    multi_devices = multi_device;

    cap.ref.obj = (uintptr_t)multi_device;
    printf("\n\tmulti device is created!\n");

    return cap;
}

int 
virtual_multi_register_control_block_impl(
        L4_ThreadId_t caller, 
        objref_t multi_obj, 
        uintptr_t addr, 
        idl4_server_environment *env)
{
    struct virtual_multi *vm = (struct virtual_multi *)multi_obj;
    void *vaddr;
    printf("vmulti got a conytorol block\n");
    /* FIXME: Check valid etc */
    vaddr = (void*) ((uintptr_t) vm->memsect_attach[(addr & 0xf)].base + (addr >> 4));
    vm->control = vaddr;

    uintptr_t * words = (uintptr_t*)vm->control->head;
    if (vm == multi_devices->active_virtual){
        printf("First multi client attatch. no return for him yet\n");
        words[0] = words[1] = words[3] = words[4] = 0;
    } else {
        printf("He is the seconds guy to register.\n");
        printf("telling him of %08lx:%08lx\n", multi_devices->active_virtual->thread.raw,
                multi_devices->active_virtual->mask);
        words[0] = multi_devices->active_virtual->thread.raw;
        words[1] = multi_devices->active_virtual->mask;
        words[2] = multi_devices->active_virtual->ms;
        words[3] = multi_devices->active_virtual->pass;

        printf("done. Now tell the other guy about him \n");
        words = (uintptr_t*)multi_devices->active_virtual->control->head;
        words[0] = vm->thread.raw;
        words[1] = vm->mask;
        words[2] = vm->ms;
        words[3] = vm->pass;
        L4_Notify(multi_devices->active_virtual->thread, multi_devices->active_virtual->mask);
    }


    return 0;
}

cap_t
virtual_multi_factory_create_impl
(
    CORBA_Object    _caller,
    objref_t        multi_device,
    L4_ThreadId_t   *thread,
    uintptr_t       mask,
    idl4_server_environment * _env
)
{
    cap_t cap = { {0}, 0 };
    struct virtual_multi *vm;
    struct multi_device *dev = (void*)multi_device;

    vm = malloc(sizeof(struct virtual_multi));

    if (vm == NULL) {
        return cap;
    }

    vm->multi_device = (void*) multi_device;
    vm->thread = *thread;
    vm->mask = mask;
    vm->next = NULL;

    if (dev->active_virtual == NULL)
    {
        dev->active_virtual = vm;
        dev->last_virtual = vm;
    }
    else
    {
        dev->last_virtual->next = vm;
        dev->last_virtual = vm;
    }

    cap.ref.obj = (uintptr_t)vm;
    return cap;
}

int
virtual_multi_add_memsection_impl
(
    CORBA_Object    _caller,        /* AUTO */
    objref_t        multi_obj,
    objref_t        memsection,
    uintptr_t       passwd,
    int             idx,
    idl4_server_environment * _env  /* AUTO */
)
{
    struct virtual_multi *vm = (struct virtual_multi *)multi_obj;

    cap_t cap;
    int   ret;

    if (idx > 16) {
        /* ERROR */
        return -1;
    }
    vm->memsect_attach[idx].base = memsection_base(memsection);
    vm->memsect_attach[idx].size = memsection_size(memsection);
    vm->pass = passwd;
    vm->ms = memsection;

    cap.ref.obj = memsection;
    cap.passwd  = passwd;
    ret = clist_insert(default_clist, cap);

    return ret;
}


/* XXX: check to see if interrupts are used */
void
vmulti_async_handler(L4_Word_t notify)
{
    return;
}

