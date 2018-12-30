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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <l4/types.h>

#include <driver/driver.h>
#include <driver/mem_space.h>
#include <event/event.h>

struct com_device;

struct control_block {
    void *head;
};

struct memsect_attach {
    void *base;
    uintptr_t size;
};

struct virtual_com {
    struct virtual_com *next;
    struct com_device *com_device;
    L4_ThreadId_t thread;
    uintptr_t mask;
    struct memsect_attach memsect_attach[16];

    uintptr_t pass;
    uintptr_t ms;

    struct control_block *control;
};

struct com_device {
    struct com_device *next;
    struct resource *resources;
    
    struct device_interface *di;

    struct virtual_com *active_virtual;
    struct virtual_com *last_virtual;

    int irq;
};

struct com_device *com_devices = NULL;


/* Statically allocated memory */

static int buffer[8] = { 0x01234567 };
static mem_space_t iguana_granted_physmem[4] = { (mem_space_t)0xdeadbeef } 
static int iguana_granted_interrupt[4] = { 0xffffffff };
static struct virtual_com virtual_device_instance[4] = { { 1 } };

static struct resource resources[8];
static struct com_device com_device;

int
main(int argc, char **argv)
{
    int i, r;
    void *device;

    for (i=0, r=0; i<4; i++) {
        if (iguana_granted_physmem[i] != NULL) {
            resources[r].type = MEMORY_RESOURCE;
            resources[r++].resource.mem = iguana_granted_physmem[i];
        }
        if (iguana_granted_interrupt[i] != -1) {
            resources[r].type = INTERRUPT_RESOURCE;
            resources[r++].resource.interrupt = iguana_granted_interrupt[i];
        }
    }
    for (/* none */; r<8; r++) {
        resources[r].type = NO_RESOURCE;
    }

    com_device.next = NULL;
    com_device.resources = resources;
    com_devices = &com_device;

    for (i=0; i<4; i++) {
        if (virtual_device_instance[i].valid_instance) {
            virtual_device_instance[i].com_device = (void *)(&com_device);

            if (com_device.active_virtual == NULL) {
                com_device.active_virtual = &virtual_device_instance[i];
                com_device.last_virtual = &virtual_device_instance[i];
            } else {
                com_device.last_virtual->next = &virtual_device_instance[i];
                com_device.last_virtual = &virtual_device_instance[i];
            }
        }
    }

    event_init();
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    register_event(thread_l4tid(thread_myself()), 0x1000, EV_WOMBAT_SWITCH);
    vcom_server_loop();

    return 0;
}

static void switch_active(void)
{
    return 0;
}

int virtual_com_add_memsection_impl(CORBA_Object _caller, device_t handle, objref_t memsection,
                                    uintptr_t passwd, int idx, idl4_server_environment *_env)
{

}
