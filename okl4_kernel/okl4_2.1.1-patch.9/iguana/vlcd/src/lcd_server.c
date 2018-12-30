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
 * Author: Nelson Tam
 */
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <interfaces/vlcd_serverdecls.h>

#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/physmem.h>
#include <iguana/cap.h>
#include <iguana/env.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/stream.h>

#include <event/event.h>
#include <driver/fb.h>

#include <util/trace.h>

#include <l4/misc.h>

#if 0
#define dDEBUG_PRINT(arg...) DEBUG_TRACE(3, arg)
#else
#define dDEBUG_PRINT(arg...) do { } while (0/*CONSTCOND*/);
#endif

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

/**
 * LCD_DRIVER is defined in the build system, and indicates the
 * device driver against which we are compiled.
 *
 */
extern struct driver LCD_DRIVER;

struct memsect_attach
{
    void        *base;
    uintptr_t   size;
};

struct control_block
{
    uint32_t            xres;
    uint32_t            yres;
    uint32_t            bpp;
};

struct lcd_device;

struct virtual_lcd
{
    int                     valid_instance;
    struct virtual_lcd      *next;
    struct lcd_device       *lcd_device;
    L4_ThreadId_t           thread;
    uintptr_t               mask;
    struct memsect_attach   memsect_attach[16];

    struct control_block    *control;
};

struct lcd_device
{
    struct resource         *resources;
    struct device_interface *di;
    struct fb_interface     *fb;

    struct virtual_lcd      *active_virtual;
    struct virtual_lcd      *last_virtual;

    uintptr_t               fb_vbase;
    uintptr_t               fb_pbase;
    physmem_ref_t           fb_physmem;
    memsection_ref_t        fb_memsect;
};

// HACK: Linking the granted sections doesn't work at the start of the text
// section...
static int buffer[8] SECTION(".data.");

/* Statically allocated driver resources */
static mem_space_t iguana_granted_physmem[4] SECTION(".data.");
static int iguana_granted_interrupt[4] SECTION(".data.");

static struct virtual_lcd virtual_device_instance[4] SECTION(".data.");
static struct resource resources[8];
static struct lcd_device lcd_device;

static L4_ThreadId_t primary_; /* The thread id of current "primary" wombat instance */
int
main(void)
{
#if 0
    /*
     * XXX: Debugging section.  Remove later.
     *
     * Move me to a library.  Use as diagnosis
     */
    printf("In vlcd now\n");
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

    /*
     * Setup the lcd.  Based on old add_resource and device_impl functions as
     * well as he code from vtimer
     */
    int i, r;
    for (i = 0, r = 0; i < 4; i++) {
        if (iguana_granted_physmem[i] != NULL) {
            dprintf("Adding memory resource at %p\n", iguana_granted_physmem[i]);
            resources[r].type = MEMORY_RESOURCE;
            resources[r++].resource.mem = iguana_granted_physmem[i];
        }
        if (iguana_granted_interrupt[i] != 0) {
            dprintf("Adding interrupt resource %d interrupt %d\n", r,
                    iguana_granted_interrupt[i]);
            resources[r].type = INTERRUPT_RESOURCE;
            resources[r++].resource.interrupt = iguana_granted_interrupt[i];
        }
    }
    while (r < 8) {
        dprintf("Setting resource %d to no resource\n", r);
        resources[r++].type = NO_RESOURCE;
    }

    /*
     * Copied from device_create function
     */
    void *device;

    /* Allocate memory for the lcd device */
    device = malloc(LCD_DRIVER.size);

    if (device == NULL) {
        free(device);
    }

    /* Initialise the device state */
    lcd_device.resources = resources;

    /* Create */
    lcd_device.di = setup_device_instance(&LCD_DRIVER, device);

    /* Setup */
    device_setup(lcd_device.di, lcd_device.resources);

    /* Enable */
    device_enable(device);


    /* Connect up the interfaces */
    lcd_device.fb = (struct fb_interface *) device_get_interface(lcd_device.di, 0);
    lcd_device.fb_vbase = (uintptr_t)NULL;
    lcd_device.fb_pbase = (uintptr_t)NULL;
    lcd_device.fb_memsect = (memsection_ref_t)NULL;
    lcd_device.fb_physmem = (physmem_ref_t)NULL;


    /*
     * Copied from factory_create function
     */
    primary_ = primary_wombat();

    for (int i = 0; i < 4; i++) {
        if (virtual_device_instance[i].valid_instance) {
            virtual_device_instance[i].lcd_device = &lcd_device;
            //vlcd->thread = *thread;
            //vlcd->mask = mask;
            if (lcd_device.active_virtual == NULL) {
                lcd_device.active_virtual = &virtual_device_instance[i];
                lcd_device.last_virtual = &virtual_device_instance[i];
            } else {
                lcd_device.last_virtual->next = &virtual_device_instance[i];
                lcd_device.last_virtual = &virtual_device_instance[i];
            }
        }
    }


    event_init();
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);
    register_event(thread_l4tid(env_thread(iguana_getenv("MAIN"))), 0x1000, EV_WOMBAT_SWITCH);
    vlcd_server_loop();
    return 0;

    (void)buffer;
    (void)resources;
    (void)virtual_device_instance;
    (void)lcd_device;
}

/*void device_add_resource_impl
(
    L4_ThreadId_t   caller,
    uintptr_t       resource,
    uintptr_t       resource2,
    int             type,
    idl4_server_environment *env
)
{
    struct resource *new;
    new = (struct resource*)malloc(sizeof(struct resource));

    if (!new) return;

    switch(type)
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
            DEBUG_TRACE(3, "adding mem %p to %p\n", mem_, new);
        }
        break;
    case BUS_RESOURCE:
        {
            new->resource.bus.tid.raw = resource;
            new->resource.bus.obj = resource2;
            DEBUG_TRACE(3, "adding %08lx %08lx to %p\n", resource, resource2, new);
        }
        break;
    default:
        DEBUG_TRACE(3, "WWTF arning: Unknown resource type: %d\n", type);
        return;
    }

    new->type = (enum resource_type)type;
//    new->next = global_resources;
//    global_resources = new;
}*/

/*cap_t
device_create_impl(CORBA_Object _caller,
                   L4_ThreadId_t *bus,
                   uint8_t deviceid,
                   idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct lcd_device *lcd_device;
    void *device;
*/
    /* Allocate memory for the lcd device */
/*    lcd_device = malloc(sizeof(struct lcd_device));
    device = malloc(LCD_DRIVER.size);

    if (lcd_device == NULL || device == NULL) {
        free(lcd_device);
        free(device);
        return cap;
    }
*/
    /* Initialise the device state */
    //lcd_device->next = lcd_devices;
    //lcd_device->resources = global_resources;
    //global_resources = NULL;
    //lcd_devices = lcd_device;

    /* Create */
    //lcd_device->di = setup_device_instance(&LCD_DRIVER, device);

    /* Setup */
    //device_setup(lcd_device->di, lcd_device->resources);

    /* Enable */
    //device_enable(device);

    /* Now he's had a chance to program the HW correctly, we enable any interrupts */
/*    struct resource * r = lcd_device->resources;
    for ( ; r ; r = r->next)
    {
        if (r->type == INTERRUPT_RESOURCE)
        {
            hardware_register_interrupt(L4_Myself(), r->resource.interrupt);
        }
    }
*/
    /* Connect up the interfaces */
/*    lcd_device->fb = (struct fb_interface *) device_get_interface(lcd_device->di, 0);

    lcd_device->fb_vbase = (uintptr_t)NULL;
    lcd_device->fb_pbase = (uintptr_t)NULL;
    lcd_device->fb_memsect = (memsection_ref_t)NULL;
    lcd_device->fb_physmem = (physmem_ref_t)NULL;

    cap.ref.obj = (uintptr_t)lcd_device;

    return cap;
}*/

int
virtual_lcd_register_control_block_impl
(
    CORBA_Object    _caller,        /* AUTO */
    device_t        handle,
    uintptr_t       addr,
    idl4_server_environment *_env   /* AUTO */
)
{
    struct virtual_lcd *vlcd = virtual_device_instance + handle;
    void *vaddr;
    /* FIXME: Check valid etc */
    vaddr = (void*) ((uintptr_t) vlcd->memsect_attach[(addr & 0xf)].base + (addr >> 4));
    vlcd->control = vaddr;

    return 0;
}

int
virtual_lcd_add_memsection_impl
(
    CORBA_Object    _caller,        /* AUTO */
    device_t        handle,
    objref_t        memsection,
    uintptr_t       passwd,
    int             idx,
    idl4_server_environment * _env  /* AUTO */
)
{
    struct virtual_lcd *vlcd = virtual_device_instance + handle;

    cap_t cap;
    int   ret;

    if (idx > 16) {
        /* ERROR */
        return -1;
    }
    vlcd->memsect_attach[idx].base = memsection_base(memsection);
    vlcd->memsect_attach[idx].size = memsection_size(memsection);

    cap.ref.obj = memsection;
    cap.passwd  = passwd;
    ret = clist_insert(default_clist, cap);

    return ret;
}

void
virtual_lcd_get_mode_impl
(
    L4_ThreadId_t   caller,
    device_t        handle,
    uint32_t        *xres,
    uint32_t        *yres,
    uint32_t        *bpp,
    idl4_server_environment *env
)
{
    struct virtual_lcd *vdev = virtual_device_instance + handle;

    dDEBUG_PRINT("%s done\n", __func__);

    fb_get_mode(vdev->lcd_device->fb, xres, yres, bpp);
}

int
virtual_lcd_set_mode_impl
(
    L4_ThreadId_t   caller,
    device_t        handle,
    uint32_t        xres,
    uint32_t        yres,
    uint32_t        bpp,
    idl4_server_environment *env
)
{
    struct virtual_lcd *vdev = virtual_device_instance + handle;

    return fb_set_mode(vdev->lcd_device->fb, xres, yres, bpp);
}

int
virtual_lcd_get_fb_impl
(
    CORBA_Object    _caller,        /* AUTO */
    device_t        handle,
    uintptr_t       *vaddr,
    idl4_server_environment *_env   /* AUTO */
)
{
    struct virtual_lcd *vlcd = virtual_device_instance + handle;
    struct lcd_device  *lcd  = vlcd->lcd_device;

    *vaddr = lcd->fb_vbase;

    return 0;
}

int
virtual_lcd_set_fb_impl
(
    CORBA_Object    _caller,        /* AUTO */
    device_t        handle,
    uintptr_t       new_vaddr,
    idl4_server_environment *_env   /* AUTO */
)
{
    enum errno_e { none, error_vaddr, error_paddr, warning_fbsize };
    enum errno_e errno = none;

    struct virtual_lcd *vlcd = virtual_device_instance + handle;
    struct lcd_device  *lcd  = vlcd->lcd_device;

    /*
     * If the new fb vaddr is sane and is backed by a memsection, then
     * copy the contents over, and destroy the old memsection.
     *
     * Server expects that the memsection for the new fb is allocated
     * and created by the caller
     *
     * Checks:
     *
     * 1. new_vaddr is backed by valid memsection
     * 2. new_fb_memsect is sufficient size
     *
     * - nt
     */
    uintptr_t   old_size;
    size_t      new_size;
    uintptr_t   dummy;
    uintptr_t   new_paddr;

    dDEBUG_PRINT("%s [vlcd]: vaddr = 0x%lx\n", __func__, new_vaddr);

    /* See if given vaddr is really backed, and get its size */
    new_paddr = memsection_virt_to_phys(new_vaddr, &new_size);

    dDEBUG_PRINT("%s [vlcd]: paddr = 0x%lx, size = 0x%lx\n", __func__, new_paddr, (uint32_t)new_size);

    /* Check info of current fb */
    if (lcd->fb_physmem)
        physmem_info(lcd->fb_physmem, &dummy, &old_size);
    else
        old_size = 0;

    if (new_vaddr == (uintptr_t)NULL)
        errno = error_vaddr;

    else if (new_paddr == (uintptr_t)NULL)
        errno = error_paddr;

    else if (new_size < (size_t)old_size)
        errno =  warning_fbsize;

    /* Phew!  Everyone is happy */
    else
    {
        dDEBUG_PRINT("all normal\n");

        /* TODO: might want to do scaling of some sort? - nt */
        if (lcd->fb_vbase)
            memcpy((void*)new_vaddr, (void*)lcd->fb_vbase, old_size);

        if (lcd->fb_memsect && lcd->fb_physmem)
            memsection_delete_dma(lcd->fb_memsect, lcd->fb_physmem);

        lcd->fb_vbase   = new_vaddr;
        lcd->fb_pbase   = new_paddr;
        lcd->fb_memsect = memsection_lookup((objref_t)new_vaddr, NULL);
        lcd->fb_physmem = (physmem_ref_t)new_paddr;

        dDEBUG_PRINT("all done!\n");
    }

    /* XXX: what does the next line do? - nt */
    // vlcd->lcd_device->fb_pbase += ((uintptr_t)addr & (size-1));

    switch (errno)
    {
    default:
        DEBUG_TRACE(3, "%s: wtf how did you get here\n", __func__);
        return -1;

    case error_vaddr:
    case error_paddr:
    case warning_fbsize:
        DEBUG_TRACE(3, "%s: PANIC handle these errors please\n", __func__);
        return -1;

    case none:
        fb_set_buffer(lcd->fb, new_paddr);
        return 0;
    }
}

