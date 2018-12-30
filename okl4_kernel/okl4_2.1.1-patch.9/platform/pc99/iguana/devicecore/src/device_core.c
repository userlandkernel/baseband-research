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
 * This is the device core of the pc99 platform.
 *
 * This file was based on the devicecore of the PXA platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/kdebug.h>
#include <l4/message.h>
#include <driver/driver.h>
#include <driver/device.h>

#include <util/trace.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/env.h>

#include <string.h>


#include <interfaces/vtimer_client.h>
#include <interfaces/vserial_client.h>
#include <interfaces/vrtc_client.h>
#include <interfaces/vbus_client.h>



enum device_ids { UART1, TIMER, RTC };


L4_ThreadId_t timer_server, 
              serial_server,
              rtc_server;
              
objref_t timer_device, 
         serial_device,
         rtc_device;

#include <interfaces/devicecore_serverdecls.h>

objref_t
device_core_enable_impl(L4_ThreadId_t caller, int device, idl4_server_environment *env)
{
    DEBUG_TRACE(1, "pc99 device core enable %d\n", device);
    return 0;
}

static objref_t
get_timer(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_timer;
    *thrd = timer_server;
    /* Create a virtual keypad */
    virtual_timer = virtual_timer_factory_create(timer_server, timer_device,
            notify,
            mask, &my_env);
    DEBUG_TRACE(1, "get_timer: server 0x%lx obj = 0x%lx\n", 
        timer_server.raw, virtual_timer.ref.obj);
    return virtual_timer.ref.obj;
}


static objref_t
get_serial(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_serial;
    *thrd = serial_server;
    /* Create a virtual keypad */
    virtual_serial = virtual_serial_factory_create(serial_server, serial_device,
            notify,
            mask, &my_env);
    return virtual_serial.ref.obj;
}

static objref_t
get_rtc(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_rtc;
    *thrd = rtc_server;
    /* Create a virtual keypad */
    virtual_rtc = virtual_rtc_factory_create(rtc_server, rtc_device,
            notify,
            mask, &my_env);
    return virtual_rtc.ref.obj;
}

objref_t
device_core_get_vdevice_impl(L4_ThreadId_t caller, L4_ThreadId_t *thrd, 
                            L4_ThreadId_t *notify, L4_Word_t mask, char * name,
                            idl4_server_environment *env)
{
    if (strcmp(name, "timer") == 0) return get_timer(caller, thrd, notify, mask);
    else if  (strcmp(name, "serial") == 0)  return get_serial(caller, thrd, notify, mask);
    else if  (strcmp(name, "rtc") == 0) return get_rtc(caller, thrd, notify, mask);
    
    thrd->raw = 0;
    return 0;
}


// If the device core needs irq's for any system devices, they go here
void
devicecore_irq_handler(L4_ThreadId_t partner)
{
}




// Go through each device we know is available (or use some plat-specific method to probe)
// and start the servers with appropriate memory addresses and irq numbers

/*
 * Note: the beige box is fairly configurable, usually this sort of stuff
 * needs to be detected at runtime.  For Iguana, we support timer only.
 * The pc99 has traditionally used the KDB serial driver.
 *
 * -gl
 */
static void
scan_devices(void)
{
    CORBA_Environment env;
    L4_ThreadId_t me = L4_Myself();
    thread_ref_t server_;
    cap_t device;

    /* timer device */
    //memsection_lookup(iguana_getenv(IGUANA_GETENV_VTIMER_SERVER), &server_);
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VTIMER_SERVER")),
                      &server_);
    timer_server = thread_l4tid(server_);
    device_add_resource(timer_server, 0/*irq*/, 0, INTERRUPT_RESOURCE, &env);
    device_add_resource(timer_server, 0x40, 0, X86_IOPORT_RESOURCE, &env);
    device = device_create(timer_server, &me, TIMER, &env);
    timer_device = device.ref.obj;

    /* serial device */
    //memsection_lookup(iguana_getenv(IGUANA_GETENV_VSERIAL_SERVER), &server_);
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VSERIAL_SERVER")),
                      &server_);
    serial_server = thread_l4tid(server_);
    device = device_create(serial_server, &me, UART1, &env);
    serial_device = device.ref.obj;

    /* rtc device */
    //memsection_lookup(iguana_getenv(IGUANA_GETENV_VRTC_SERVER), &server_);
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VRTC_SERVER")),
                      &server_);
    rtc_server = thread_l4tid(server_);
    /* IRQ 8 used by pistachio so it fails to register */
    //device_add_resource(rtc_server, 8, 0, INTERRUPT_RESOURCE, &env);
    device_add_resource(rtc_server, 0x70, 0, X86_IOPORT_RESOURCE, &env);
    device = device_create(rtc_server, &me, RTC, &env);
    rtc_device = device.ref.obj;
    

    // And any other devices you care to add ...
}

int
main(int argc, char *argv[])
{
    

    /* Link to event server */
//    memsection_lookup(iguana_getenv(0x90b), &server_);
//    event_server = thread_l4tid(server_);

    //Set up any system drivers that we may link against

    scan_devices();
    devicecore_server_loop();
}
