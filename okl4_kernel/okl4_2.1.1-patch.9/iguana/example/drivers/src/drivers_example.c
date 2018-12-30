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
 * Created: Tue Nov 9 2004 
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <l4/thread.h>
#include <l4/misc.h>

#include <vtimer/timer.h>
#include <vrtc/rtc.h>

#include <iguana/thread.h>
#include <iguana/physmem.h>
#include <iguana/memsection.h>
#include <iguana/env.h>

#include <driver/types.h>

/*
 * XXX hardcoded: this means that it is not configurable -gl
 *
 * What we can do is make the build system append a CFLAG for
 * every type of vdevice included in the build.  Then we write
 * a top-level vdevice_client.h that hides all the #ifdef ugliness
 *
 * - nt
 */
#include <interfaces/vtimer_client.h>
#include <interfaces/vserial_client.h>
#include <interfaces/vrtc_client.h>

enum supported_devices {
    TIMER,
    SERIAL,
    RTC
};

#define NOTIFY_MASK(x) (1 << x)

/*
 * If you ever write a new device, start by looking at the timer
 */

struct timer {
    server_t    server;
    device_t    handle;
};

struct rtc {
    server_t server;
    device_t handle;
};

/*
 * The following is used for the more complicated serial device
 */
#define BUFFER_SIZE 0x1000
#define PACKET_SIZE 16

#define TERMINATED 1
#define COMPLETED 2
/* 
 * This is the format used by the device server. 
 * The definition really belongs in a header
 */
struct serial_control_block {
    volatile uintptr_t tx;
    volatile uintptr_t rx;
};

struct stream_packet {
    uintptr_t       next;
    uintptr_t       data_ptr;
    size_t          size;
    size_t          xferred;
    volatile int    status;
    char            data[PACKET_SIZE];
};

struct serial {
    server_t    server;
    device_t    handle;
    
    memsection_ref_t    ms;
    uintptr_t           ms_base;
    
    struct serial_control_block *control;
    struct stream_packet        *free_list;

    struct stream_packet *rx_complete;
    struct stream_packet *rx_last;
};

static inline uintptr_t
vaddr_to_memsect(uintptr_t base, void *vaddr)
{
    return ((uintptr_t) vaddr - base) << 4;
}

static inline void *
memsect_to_vaddr(uintptr_t base, uintptr_t memsect)
{
    return (void *) (base + (memsect >> 4));
}

static int 
init_timer(struct timer *timer)
{
    /* Find the timer server and handle */
    timer->server = env_thread_id(iguana_getenv("VTIMER_TID"));
    timer->handle = env_const(iguana_getenv("VTIMER_HANDLE"));

    return 0;
}

static int
init_rtc(struct rtc *rtc)
{
    rtc->server = env_thread_id(iguana_getenv("VRTC_TID"));
    rtc->handle = env_const(iguana_getenv("VRTC_HANDLE"));
}

static int 
init_serial(struct serial *serial)
{
    /* Find the serial server and handle */
    serial->server = env_thread_id(iguana_getenv("VSERIAL_TID"));
    serial->handle = env_const(iguana_getenv("VSERIAL_HANDLE"));

    /* Any shared mem gets initialiesd here */
    serial->ms = memsection_create(0x1000, &(serial->ms_base));

    /* Give the ms to the server. Note that the password is zero by default */
    virtual_serial_add_memsection(serial->server, serial->handle, serial->ms, 0, 0, NULL);

    /* Initialise the control block */
    serial->control = (struct serial_control_block*)serial->ms_base;
    serial->control->tx = ~0;
    serial->control->rx = ~0;

    serial->free_list = (struct stream_packet*)(serial->ms_base + sizeof(struct serial_control_block));

    /* Register the control block with the server */
    virtual_serial_register_control_block(serial->server,
            serial->handle,
            vaddr_to_memsect(serial->ms_base, serial->control), NULL);

    /* Initialise the free list of packets */
    int i;
    for( i = 0; i < (BUFFER_SIZE- sizeof(struct serial_control_block))/ sizeof(struct stream_packet); i++){
        serial->free_list[i].next = (uintptr_t) &serial->free_list[i+1];
        serial->free_list[i].data_ptr = vaddr_to_memsect(serial->ms_base, &serial->free_list[i].data);
        // serial->available += PACKET_SIZE;
    }
    serial->free_list[i-1].next = 0;

    serial->rx_last = NULL;
    serial->rx_complete = NULL;

    /* Now feed some packets to the server to it may RX into them */
    for (i = 0; i < 10; i++){
        struct stream_packet *packet = packet = serial->free_list;
        assert(packet);

        if (serial->rx_complete == NULL) {
            serial->rx_complete = packet;
        }

        serial->free_list = (void*) packet->next;
        packet->next = ~0;
        packet->size = PACKET_SIZE;
        packet->xferred = 0;
        packet->status = 0;

        if (serial->rx_last != NULL) {
            serial->rx_last->next = vaddr_to_memsect(serial->ms_base, packet);
        } 
        else {
            serial->control->rx = vaddr_to_memsect(serial->ms_base, packet);
        }
        serial->rx_last = packet;
    }

    L4_Notify(serial->server, 0x1);

    return 0;
}

int
main(int argc, char **argv)
{
    struct timer timer;
    struct serial serial;
    struct rtc rtc;
    L4_Word_t my_tid = thread_l4tid(env_thread(iguana_getenv("MAIN"))).raw;

    init_serial(&serial);
    init_timer(&timer);
    init_rtc(&rtc);

    printf("Found serial server (tid: %08lx, handle: %d)\n", serial.server.raw, serial.handle);
    printf("Found timer server (tid: %08lx, handle: %d)\n", timer.server.raw, timer.handle);
    printf("Found rtc server (tid: %08lx, handle: %d)\n", rtc.server.raw, rtc.handle);

    /*
     * Disable the timer example for now -- until I get the timer wraparound
     * bug sorted out.
     *
     * -gl
     */
    virtual_timer_init(timer.server, timer.handle, my_tid, NOTIFY_MASK(TIMER), NULL);
    virtual_timer_request(timer.server, timer.handle, MILLISECS(10), TIMER_PERIODIC, NULL);

    virtual_rtc_init(rtc.server, rtc.handle, my_tid, NOTIFY_MASK(RTC), NULL);

    uint64_t rtc_time = virtual_rtc_get_time(rtc.server, rtc.handle, NULL);
    printf("Get RTC time is %llu\n", rtc_time);

    virtual_rtc_set_time(rtc.server, rtc.handle, 1241395200, NULL);
    // one shot
    //printf("Setting RTC alarm oneshot for 5 seconds\n");
    //virtual_rtc_request_alarm(rtc.server, rtc.obj, 5, RTC_ALARM_ONESHOT, NULL);

    // periodic
    //printf("Setting RTC alarm periodic timer for every 1 seconds");
    //virtual_rtc_request_alarm(rtc.server, rtc.handle, 1, RTC_ALARM_PERIODIC, NULL);

    virtual_serial_init(serial.server, serial.handle, my_tid, NOTIFY_MASK(SERIAL), NULL);

    L4_ThreadId_t sender;
    L4_Set_NotifyMask(0xffffffff);
    L4_Accept(L4_NotifyMsgAcceptor);

    uint64_t last = 0;
    uint64_t current = 0, new_current;

    while(1)
    {
        L4_Msg_t msg;
        L4_MsgTag_t tag = L4_Wait(&sender);
        L4_MsgStore(tag, &msg);
        L4_Word_t num = L4_MsgWord(&msg, 0);

        printf("%s: Got message from %lx (msg: %lx)\n", __FUNCTION__, sender.raw, num);

        if (num & NOTIFY_MASK(RTC)) {
            uint64_t rtc_time, alarm;

            rtc_time = virtual_rtc_get_time(rtc.server, rtc.handle, NULL);
            alarm = virtual_rtc_get_alarm(rtc.server, rtc.handle, NULL);
            printf("Received RTC alarm notification at %llu next at %llu\n", rtc_time, alarm);
        }
        if (num & NOTIFY_MASK(TIMER))
        {
            current = virtual_timer_current_time(timer.server, timer.handle, NULL);
            new_current = virtual_timer_current_time(timer.server, timer.handle, NULL);
            printf("time is %llu\t", current);
            printf("time is %llu\t", new_current);

            printf("diff = %08lld\n", current-last);

            uint64_t rtc_time = virtual_rtc_get_time(rtc.server, rtc.handle, NULL);
            printf("rtc = %lld\n", rtc_time);

/*                 if (rtc_secs++ == 20) { */
/*                     rtc_secs = 0; */
/*                     printf("adding 20 secs to rtc\n"); */
/*                     virtual_rtc_set_time(rtc.server, rtc.obj, rtc_time + 20, NULL); */
/*                 } */
            last = current;
        }
        if (num & NOTIFY_MASK(SERIAL))
        {
            struct stream_packet *next, *packet = serial.rx_complete;
            int new_q = 0;

            while(packet)
            {
                new_q = 0;
                next  = (void*)packet->next;
                if (!(packet->status & COMPLETED)) {
                    break;
                }

                // deal with the recieved chars somehow
                int i;
                for (i = 0; i < packet->xferred; i++)
                {
                    printf("%c", packet->data[i]);
                }

                // free the packet they were delivered in
                packet->next = ~0;
                packet->size = PACKET_SIZE;
                packet->xferred = 0;
                packet->status = 0;

                if (serial.rx_last != NULL) {
                    serial.rx_last->next = vaddr_to_memsect(serial.ms_base, packet);
                    if (serial.rx_last->status & TERMINATED) {
                        new_q = 1;
                    }
                } else {
                    new_q = 1;
                }
                if (new_q) {
                    serial.control->rx = vaddr_to_memsect(serial.ms_base, packet);
                }
                serial.rx_last = packet;

                if ((uintptr_t)next == ~0) {
                    packet = NULL;
                } else {
                    packet = memsect_to_vaddr(serial.ms_base, (uintptr_t)next);
                }
            }
            // notify the server of the new empty packet if necessary
            if (new_q) L4_Notify(serial.server, 0x1);
            serial.rx_complete = packet;
        }
    }

    return 0;
}

