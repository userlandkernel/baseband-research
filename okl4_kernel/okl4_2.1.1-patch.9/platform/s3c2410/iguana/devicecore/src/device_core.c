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
 * This forms the core of the gta01 (OpenMoko) device system.
 *
 * We know the particular devices we care about, so unlike
 * generic servers, we reference them directly.
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

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/hardware.h>
#include <iguana/object.h>
#include <iguana/env.h>

#include <util/trace.h>

#include <string.h>

/*
 * XXX Should fix: this means that it is not configurable -gl
 */
#include <interfaces/vtimer_client.h>
#include <interfaces/vserial_client.h>
#include <interfaces/vrtc_client.h>
#include <interfaces/vtouch_client.h>
#include <interfaces/vbus_client.h>
#include <interfaces/vlcd_client.h>

/*
 * Lookup these values in the s3c2410 manual, under "Interrupt controller"
 *
 * - nt
 */
#define INT_TC      (32 + 9)
#define INT_ADC     (32 + 10)
#define INT_RXD0    (32 + 0)
#define INT_TXD0    (32 + 1)
#define INT_TIMER1  (11)
#define INT_TIMER2  (12)
#define INT_LCD     (16)
#define INT_SPI1    (29)
#define INT_RTC     (30)

#define LCD_PHYS    0X4d000000
#define UART_PHYS   0x50000000
#define TIMER_PHYS  0x51000000
#define TS_PHYS     0x58000000
#define SPI_PHYS    0x59000000      /* XXX: LCD is on SPI channel 1 */
#define RTC_PHYS    0x57000000

enum device_ids { UART0, TIMER, VTOUCH, SPI1, LCD, RTC };

L4_ThreadId_t timer_server,
              ts_server,
              spi_server,
              lcd_server,
              serial_server,
              rtc_server;

objref_t timer_device,
         ts_device,
         lcd_device,
         spi_device,
         serial_device,
         rtc_device;

#include <interfaces/devicecore_serverdecls.h>

objref_t
device_core_enable_impl(L4_ThreadId_t caller, int device, idl4_server_environment *env)
{
    DEBUG_TRACE(3, "OpenMoko device core enable %d\n", device);
    return 0;
}

static objref_t
get_timer(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_timer;
    *thrd = timer_server;
    /* Create a virtual timer */
    virtual_timer = virtual_timer_factory_create(timer_server, timer_device,
            notify,
            mask, &my_env);
    return virtual_timer.ref.obj;
}

static objref_t
get_rtc(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
        L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_rtc;
    *thrd = rtc_server;
    /* Create a virtual RTC */
    virtual_rtc = virtual_rtc_factory_create(rtc_server, rtc_device,
            notify, mask, &my_env);
    return virtual_rtc.ref.obj;
}

static objref_t
get_touchscreen(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_ts;
    *thrd = ts_server;
    /* Create a virtual touchscreen */
    virtual_ts = virtual_touch_factory_create(ts_server, ts_device,
            notify,
            mask, &my_env);
    return virtual_ts.ref.obj;
}

static objref_t
get_spi(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_spi;
    *thrd = spi_server;
    /* Create a virtual spi */
    virtual_spi = virtual_bus_factory_create(spi_server, spi_device,
            notify,
            mask, &my_env);
    return virtual_spi.ref.obj;
}

static objref_t
get_lcd(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_lcd;
    *thrd = lcd_server;
    /* Create a virtual lcd */
    virtual_lcd = virtual_lcd_factory_create(lcd_server, lcd_device,
            notify,
            mask, &my_env);
    return virtual_lcd.ref.obj;
}

static objref_t
get_serial(L4_ThreadId_t caller, L4_ThreadId_t *thrd,
                            L4_ThreadId_t *notify, L4_Word_t mask)
{
    CORBA_Environment my_env;
    cap_t virtual_serial;
    *thrd = serial_server;
    /* Create a virtual serial */
    virtual_serial = virtual_serial_factory_create(serial_server, serial_device,
            notify,
            mask, &my_env);
    return virtual_serial.ref.obj;
}

objref_t
device_core_get_vdevice_impl(L4_ThreadId_t caller, L4_ThreadId_t *thrd, 
                            L4_ThreadId_t *notify, L4_Word_t mask, char * name,
                            idl4_server_environment *env)
{
    if (strcmp(name, "timer") == 0)
        return get_timer(caller, thrd, notify, mask);
    else if (strcmp(name, "serial") == 0) 
        return get_serial(caller, thrd, notify, mask);
    else if (strcmp(name, "touchscreen") == 0)
        return get_touchscreen(caller, thrd, notify, mask);
    else if (strcmp(name, "rtc") == 0)
        return get_rtc(caller, thrd, notify, mask);
    
//    else if (strcmp(name, "spi") == 0)
//        return get_spi(caller, thrd, notify, mask);
    else if (strcmp(name, "lcd") == 0)
        return get_lcd(caller, thrd, notify, mask);

    DEBUG_TRACE(3, "Uknown device for OpenMoko: %s\n", name);
    return 0;
}


// If the device core needs irq's for any system devices, they go here
void
devicecore_irq_handler(L4_ThreadId_t partner)
{
}

// Go through each device we know is available (or use some plat-specific method to probe)
// and start the servers with appropriate memory addresses and irq numbers

// FIXME: Pass a list of 'resources' rather than a single address+irq number, which may
//        not always be sufficient.
static void
scan_devices(void)
{
    CORBA_Environment env;
    L4_ThreadId_t me = L4_Myself();
    thread_ref_t server_;
    cap_t device;

    /* timer device */
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VTIMER_SERVER")),
                      &server_);
    timer_server = thread_l4tid(server_);
    device_add_resource(timer_server, TIMER_PHYS, 0, MEMORY_RESOURCE, &env);

    /* XXX only use timer1? -gl */
    device_add_resource(timer_server, INT_TIMER1, 0, INTERRUPT_RESOURCE, &env);
    device_add_resource(timer_server, INT_TIMER2, 0, INTERRUPT_RESOURCE, &env);
    device = device_create(timer_server, &me, TIMER, &env);
    timer_device = device.ref.obj;

    /* rtc device */
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VRTC_SERVER")),
                      &server_);
    rtc_server = thread_l4tid(server_);
    device_add_resource(rtc_server, RTC_PHYS, 0, MEMORY_RESOURCE, &env);
    device_add_resource(rtc_server, INT_RTC, 0, INTERRUPT_RESOURCE, &env);
    device = device_create(rtc_server, &me, RTC, &env);
    rtc_device = device.ref.obj;

    /* This is an example of how to setup a software rtc. */
#if 0    
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VRTC_SERVER")),
            &server_);
    rtc_server = thread_l4tid(server_);

    /* Create virtual timer and pass to rtc_device as a resource.*/ 
    cap_t virtual_timer = virtual_timer_factory_create(timer_server,
            timer_device,
            &rtc_server, 
            0x3, NULL);
    device_add_resource(rtc_server, timer_server.raw, virtual_timer.ref.obj, TIMER_RESOURCE, &env);
    device = device_create(rtc_server, &me, RTC, &env);
    rtc_device = device.ref.obj;
#endif

    /* serial device */
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VSERIAL_SERVER")),
                      &server_);
    serial_server = thread_l4tid(server_);
    device_add_resource(serial_server, UART_PHYS, 0, MEMORY_RESOURCE, &env);
    device_add_resource(serial_server, INT_RXD0, 0, INTERRUPT_RESOURCE, &env);
    device_add_resource(serial_server, INT_TXD0, 0, INTERRUPT_RESOURCE, &env);
    device = device_create(serial_server, &me, UART0, &env);
    serial_device = device.ref.obj;

    /* touchscreen device */
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VTOUCH_SERVER")),
                      &server_);
    ts_server = thread_l4tid(server_);
    device_add_resource(ts_server, TS_PHYS, 0, MEMORY_RESOURCE, &env);
    device_add_resource(ts_server, INT_TC, 0, INTERRUPT_RESOURCE, &env);
    device_add_resource(ts_server, INT_ADC, 0, INTERRUPT_RESOURCE, &env);
    device = device_create(ts_server, &me, VTOUCH, &env);
    ts_device = device.ref.obj;

    /* spi device */
    memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VBUS_SERVER")),
                      &server_);
    spi_server = thread_l4tid(server_);
    device_add_resource(spi_server, SPI_PHYS, 0, MEMORY_RESOURCE, &env);
    device_add_resource(spi_server, INT_SPI1, 0, INTERRUPT_RESOURCE, &env);
    device = device_create(spi_server, &me, SPI1, &env);
    spi_device = device.ref.obj;

    /* get a bus handle to provide to the lcd driver */
    cap_t virtual_spi =  virtual_bus_factory_create(spi_server, spi_device,
            &lcd_server, 0, NULL);

    /* lcd device */
    // TEMP: REMOVED (jmatthews): causes NPE
/*     memsection_lookup((objref_t) env_memsection_base(iguana_getenv("OKL4_VLCD_SERVER")), */
/*                       &server_); */
/*     lcd_server = thread_l4tid(server_); */
/*     device_add_resource(lcd_server, LCD_PHYS, 0, MEMORY_RESOURCE, &env); */
/*     //device_add_resource(lcd_server, spi_server.raw, virtual_spi.ref.obj, BUS_RESOURCE, &env); */
/*     device_add_resource(lcd_server, spi_server.raw, 0, BUS_RESOURCE, &env); */
/*     device_add_resource(lcd_server, INT_LCD, 0, INTERRUPT_RESOURCE, &env); */
/*     device = device_create(lcd_server, &me, LCD, &env); */
/*     lcd_device = device.ref.obj; */

    // And any other devices you care to add ...
    return;
     (void)get_spi;
     (void) virtual_spi;
}

int
main(void)
{
    scan_devices();
    devicecore_server_loop();
}
