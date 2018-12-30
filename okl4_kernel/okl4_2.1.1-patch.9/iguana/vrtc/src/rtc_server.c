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

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <l4/ipc.h>
#include <l4/misc.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/interrupt.h>

#include <util/trace.h>

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/pd.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/rtc.h>

#include <circular_buffer/cb.h>
#include <range_fl/range_fl.h>

#include <interfaces/vrtc_serverdecls.h>

#define TIMER_TICK_MASK 0x3

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

/** 
 * RTC_DRIVER is defined in the build system, and indicates
 * which device driver the rtc server supports.
 */
extern struct driver RTC_DRIVER;

// Forward declaration
struct rtc_device;

/**
 * Virtual rtc alarms can be in one of three different states.
 if one-shot, can either be set relatively (default) or absolutely
*/
#define RTC_ALARM_ONESHOT  1
#define RTC_ALARM_ABSOLUTE 2
#define RTC_ALARM_PERIODIC 4
enum virtual_rtc_alarm_state { 
    rtc_alarm_periodic_e,
    rtc_alarm_oneshot_e,
    rtc_alarm_cancelled_e
};

/**
 * Holds the information for each virtual rtc.
 * RTCs are kept in a list per device in order to search
 * for expired alarms on alarm interrupt
 */
struct virtual_rtc {
    int valid_instance;
    // underlying physical device
    struct rtc_device *rtc_device; 

    L4_ThreadId_t thread;
    uintptr_t mask;
    int flags;

    // private offset for time setting
    int64_t offset;

    // alarms
    enum virtual_rtc_alarm_state alarm_state;
    uint64_t alarm_timeout;
    uint64_t alarm_period;
    struct virtual_rtc *alarm_next;
    struct virtual_rtc *alarm_prev;

};

/** 
 * The rtc device struct holds information for each physical rtc
 * device in the server. Most servers only have one rtc.
 */
struct rtc_device {
    struct resource *resources;
    struct device_interface *di;
    struct rtc_interface *ri;

    // alarm lists
    struct virtual_rtc *active;
    struct virtual_rtc *active_end;
    struct virtual_rtc *inactive;
};

/* Statically allocated device structures */
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
static struct virtual_rtc virtual_device_instance[4] = { { 1 } }; /* valid but unused instances are ok */

static struct resource resources[8];
static struct rtc_device rtc_device;

// Forward declarations of alarm operations
static void alarm_callback(struct rtc_device *rtc_device);
static void alarm_insert_inactive(struct virtual_rtc *virtual_rtc);
static void alarm_remove_from_list(struct virtual_rtc *virtual_rtc);
static int alarm_activate(struct virtual_rtc * virtual_rtc);
static void alarm_inactivate(struct virtual_rtc * virtual_rtc);
static int alarm_cancel(struct virtual_rtc * vrtc_alarm);
static uint64_t alarm_create(struct virtual_rtc * virtual_rtc, 
                             uint64_t alarm_time_seconds, int flags,
                             uint64_t current_time);

// Converts the time to be relative to the RTC time, given any offsets on the RTC
static inline uint64_t get_relative_time(struct virtual_rtc *vrtc, uint64_t time)
{
    // Apply internal offset if one exists
    if (vrtc->offset != 0) {

        // Obtain the time as a signed int
        int64_t stime = (int64_t)time;

        // If the offset is negative and is larger than the time, zero it
        if (vrtc->offset < 0 && vrtc->offset > stime) {
            time = 0;
        } else {
            // Apply the offset
            time = (uint64_t)(stime + vrtc->offset);
        }
    }
    return time;
}

/****************************************/
/*        Server Entry Point            */
/****************************************/

int
main(int argc, char *argv[])
{
#if 0
    /*
     * XXX: Debugging section.  Remove later.
     *
     * Move me to a library.  Use as diagnosis
     */
    printf("In vrtc now\n");
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
    for (i = 0, r = 0; i < 4; i++) {
        if (iguana_granted_physmem[i] != NULL) {
            resources[r].type = MEMORY_RESOURCE;
            resources[r++].resource.mem = iguana_granted_physmem[i];
        }
        if (iguana_granted_interrupt[i] != -1) {
            resources[r].type = INTERRUPT_RESOURCE;
            resources[r++].resource.interrupt = iguana_granted_interrupt[i];
        }
    }
    while (r < 8) {
        resources[r++].type = NO_RESOURCE;
    }

    /*
     * XXX: Was: device_create_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */

    // Allocate memory for the RTC device
    device = malloc(RTC_DRIVER.size);

    if (device == NULL) {
        free(device);
    }

    // Setup the RTC device
    rtc_device.resources = resources;

    // Create the device
    rtc_device.di = setup_device_instance(&RTC_DRIVER, device);
    dprintf("%s: calling device_setup\n", __func__);
    device_setup(rtc_device.di, rtc_device.resources);
    rtc_device.ri = (struct rtc_interface *) device_get_interface(rtc_device.di, 0);
    
    // Enable the device and return the capability
    dprintf("%s: calling device_enable\n", __func__);
    device_enable(rtc_device.di);

    /*
     * XXX: Was: virtual_serial_factory_create_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */

    for (i = 0; i < 4; i++) {
        if (virtual_device_instance[i].valid_instance) {
            // Give it the underlying device, its thread, and its mask
            virtual_device_instance[i].rtc_device = &rtc_device;

            // Set its offset to 0
            virtual_device_instance[i].offset = 0;

            // Set alarm state
            virtual_device_instance[i].alarm_state = rtc_alarm_cancelled_e;
            virtual_device_instance[i].alarm_timeout = 0;
            virtual_device_instance[i].alarm_period = 0;

            // Add to inactive list
            alarm_insert_inactive(&virtual_device_instance[i]);
        }
    }
    
    /* Receives async IPC's from vtimer when using a software rtc.*/
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);

    vrtc_server_loop(); // Generated by Magpie
    return 0;

    (void)buffer;
}

uint64_t
virtual_rtc_get_time_impl(CORBA_Object _caller, device_t handle, 
                          idl4_server_environment * _env)
{
    // Obtain the vrtc, the device, and the interface
    struct rtc_device *rtc_device;
    struct virtual_rtc *vrtc = virtual_device_instance + handle;
    struct rtc_interface *ri; 
    uint64_t time;

    rtc_device = vrtc->rtc_device;
    if (!rtc_device) {
        return 0;
    }
    ri = rtc_device->ri;

    // Obtain the time from the interface and get the relative time
    time = rtc_get_time(ri);
    time = get_relative_time(vrtc, time);

    return time;
}

int
virtual_rtc_set_time_impl(CORBA_Object _caller, device_t handle, uint64_t time,
                          idl4_server_environment * _env)
{
    struct virtual_rtc *vrtc = virtual_device_instance + handle;
    struct rtc_device *rtc_device;
    struct rtc_interface *ri;

    uint64_t relative_timeout;
    uint64_t ctime;

    int i, num_vrtc = 0;

    // Obtain the virtual RTC, the device, and the interface
    rtc_device = vrtc->rtc_device;
    ri = rtc_device->ri;

    // Get the alarm relative to its current time
    relative_timeout = get_relative_time(vrtc, vrtc->alarm_timeout);

    // Per the feature doc, if we have only a single rtc,
    // we should persist the time and set the offset to 0
    for (i = 0; i < 4; i++) {
        num_vrtc += virtual_device_instance[i].valid_instance;
    }
    if (num_vrtc == 1) {
        ctime = time;
        rtc_set_time(ri, time);
        vrtc->offset = 0;
    } else {
        // Otherwise, obtain the current time and set an offset
        ctime = rtc_get_time(ri);
        vrtc->offset = time - ctime;
    }

    // If the alarm is oneshot and has now expired, cancel it
    // If the alarm is periodic, reset it to the next period
    if (vrtc->alarm_state == rtc_alarm_oneshot_e && 
        relative_timeout < get_relative_time(vrtc, rtc_get_time(ri))) {
            alarm_cancel(vrtc);

    } else if (vrtc->alarm_state == rtc_alarm_periodic_e) {
        alarm_create(vrtc, vrtc->alarm_period, RTC_ALARM_PERIODIC, ctime);
    }

    return 0;
}

int
virtual_rtc_delete_impl(CORBA_Object _caller, device_t handle,
                        idl4_server_environment * _env)
{
    // Ensure the alarm is cancelled, if it existed
    struct virtual_rtc *vrtc = virtual_device_instance + handle;
    if (vrtc->alarm_state != rtc_alarm_cancelled_e) {
        alarm_cancel(vrtc);
    }

    // Delete it
    alarm_remove_from_list(vrtc);

    // Decrement count
    //vrtc_count--;
    return 0;
}

/***************************************/
/*   Alarm Operations Implementation   */
/***************************************/

static void
alarm_callback(struct rtc_device *rtc_device)
{
    struct virtual_rtc *virtual_rtc;
    struct rtc_interface *ri;
    struct virtual_rtc *alarm_next;

    // Obtain the active virtual rtc and the device interface
    virtual_rtc = rtc_device->active;
    ri = rtc_device->ri;

    // We may not have an active alarm, check
    if (virtual_rtc){

        do {

            // Send the notification, message num is mask we were given
            L4_Notify(virtual_rtc->thread, virtual_rtc->mask);

            // The timer should be at the start of the list!
            assert(virtual_rtc->alarm_prev == NULL);

            // If this was a one shot alarm, remove it
            if (virtual_rtc->alarm_state == rtc_alarm_oneshot_e) {
                alarm_next = virtual_rtc->alarm_next;
                alarm_inactivate(virtual_rtc);

                // Otherwise, it is alarm_periodic: put it back in the queue
            } else {

                // Set alarm_timeout to alarm_next alarm_timeout
                virtual_rtc->alarm_timeout += virtual_rtc->alarm_period;

                // Re-activate the alarm
                alarm_activate(virtual_rtc);

                // alarm_next is the current active
                alarm_next = rtc_device->active;
            }

            // If we have a alarm_next alarm, set it to alarm_timeout
            // If it times out immediately, loop with it
            virtual_rtc = NULL;
            if (alarm_next) {
                if (rtc_set_alarm(ri, alarm_next->alarm_timeout)) {
                    virtual_rtc = alarm_next;
                }
            }
        } while (virtual_rtc);
    }
}

static void
alarm_insert_inactive(struct virtual_rtc *virtual_rtc)
{
    struct rtc_device *rtc_device = virtual_rtc->rtc_device;
    if (rtc_device->inactive == NULL) {
        rtc_device->inactive = virtual_rtc;
        virtual_rtc->alarm_next = virtual_rtc->alarm_prev = NULL;
    } else {
        rtc_device->inactive->alarm_prev = virtual_rtc;
        virtual_rtc->alarm_next = rtc_device->inactive;
        virtual_rtc->alarm_prev = NULL;
        rtc_device->inactive = virtual_rtc;
    }
}

// Removes the alarm from whatever list it is in
static void
alarm_remove_from_list(struct virtual_rtc *virtual_rtc)
{
    struct rtc_device *rtc_device = virtual_rtc->rtc_device;
    if (virtual_rtc->alarm_next) {
        virtual_rtc->alarm_next->alarm_prev = virtual_rtc->alarm_prev;
    }
    if (virtual_rtc->alarm_prev) {
        virtual_rtc->alarm_prev->alarm_next = virtual_rtc->alarm_next;
    }
    if (rtc_device->inactive == virtual_rtc) {
        rtc_device->inactive = virtual_rtc->alarm_next;
    } else {
        if (rtc_device->active == virtual_rtc) {
            rtc_device->active = virtual_rtc->alarm_next;
        }
        if (rtc_device->active_end == virtual_rtc) {
            rtc_device->active_end = virtual_rtc->alarm_prev;
        }
    }
}

static int
alarm_activate(struct virtual_rtc *virtual_rtc)
{
    struct rtc_device *rtc_device;
    int in_front = 0;

    rtc_device = virtual_rtc->rtc_device;
    
    // If active head is null, then active_end must be null else active_end
    // must not be null
    assert((rtc_device->active == NULL) ?
           (rtc_device->active_end == NULL) :
           (rtc_device->active_end != NULL));

    // Remove from list
    alarm_remove_from_list(virtual_rtc);

    // Insert into active list
    // Insert into head if no head already
    if (rtc_device->active_end == NULL) {
        assert(rtc_device->active == NULL);
        rtc_device->active = virtual_rtc;
        rtc_device->active_end = virtual_rtc;
        virtual_rtc->alarm_prev = virtual_rtc->alarm_next = NULL;
        in_front = 1;

        // Insert at front if alarm_timeout is less than the current front
    } else if (virtual_rtc->alarm_timeout < rtc_device->active->alarm_timeout) {
        rtc_device->active->alarm_prev = virtual_rtc;
        virtual_rtc->alarm_next = rtc_device->active;
        virtual_rtc->alarm_prev = NULL;
        rtc_device->active = virtual_rtc;
        in_front = 1;

        // Otherwise, search backwards until we are behind an existing timer
        // Note: this is O(n) in the worst case - we don't expect there
        // to be too many alarms
    } else {

        struct virtual_rtc *back = rtc_device->active_end;
        while (virtual_rtc->alarm_timeout < back->alarm_timeout) {
            back = back->alarm_prev;

            // Back can't be null - this could only happen if we are alarm_next,
            // which we can't be because we already checked that!
            assert(back != NULL);
        }

        // Insert behind the back
        virtual_rtc->alarm_next = back->alarm_next;
        virtual_rtc->alarm_prev = back;
        back->alarm_next = virtual_rtc;
        if (virtual_rtc->alarm_next) {
            virtual_rtc->alarm_next->alarm_prev = virtual_rtc;
        }
        if (rtc_device->active_end == back) {
            rtc_device->active_end = virtual_rtc;
        }
        in_front = 0;
    }

    /* Sanity check our list */
    /*
     * If we have a virtual_rtc after us, then it should point back to
     *  and it should have a alarm_timeout after us
     */
    assert(virtual_rtc->alarm_next ?
           virtual_rtc->alarm_next->alarm_prev == virtual_rtc :
           rtc_device->active_end == virtual_rtc);
    assert(virtual_rtc->alarm_next ?
           virtual_rtc->alarm_next->alarm_timeout >= virtual_rtc->alarm_timeout :
           rtc_device->active_end == virtual_rtc);
    /* 
     * If we have a alarm_timeout before us, then it should point forward to us
     *  and it should have a alarm_timeout earlier than us 
     */
    assert(virtual_rtc->alarm_prev ?
           virtual_rtc->alarm_prev->alarm_next == virtual_rtc :
           rtc_device->active == virtual_rtc);
    assert(virtual_rtc->alarm_prev ?
           virtual_rtc->alarm_prev->alarm_timeout <= virtual_rtc->alarm_timeout :
           rtc_device->active == virtual_rtc);
    
    // Return whether we were in front of another virtual timer
    return in_front;
}

static void
alarm_inactivate(struct virtual_rtc *virtual_rtc)
{
    // Remove from active list
    alarm_remove_from_list(virtual_rtc);

    // Place in cancelled state
    virtual_rtc->alarm_state = rtc_alarm_cancelled_e;

    // Set timeout to 0
    virtual_rtc->alarm_timeout = 0;

    // If active head is not null, then active_end must be non null else active_end
    // must be null
    assert((virtual_rtc->rtc_device->active != NULL) ?
           (virtual_rtc->rtc_device->active_end != NULL) :
           (virtual_rtc->rtc_device->active_end == NULL));
    
    // Insert into active list
    alarm_insert_inactive(virtual_rtc);
}

static int
alarm_cancel(struct virtual_rtc *virtual_rtc)
{
    struct rtc_device *rtc_device;
    struct rtc_interface *ri;
    struct virtual_rtc *alarm_next;
    int is_start;

    // Obtain the device, and the device interface
    rtc_device = virtual_rtc->rtc_device;
    ri = rtc_device->ri;

    // Determine if this is the first alarm for the rtc_device, and obtain the next one
    is_start = (virtual_rtc->alarm_prev == NULL);
    alarm_next = virtual_rtc->alarm_next;

    // Ensure it's not already cancelled
    if (virtual_rtc->alarm_state == rtc_alarm_cancelled_e){
        return 1;
    }

    // Sanity checks
    assert(is_start ? rtc_device->active == virtual_rtc : 1);
    assert(alarm_next == NULL ? rtc_device->active_end == virtual_rtc : 1);

    // De-activate alarm
    alarm_inactivate(virtual_rtc);

    // If this was the first, set the next alarm to alarm_next's alarm_timeout
    // If there is no alarm_next, cancel it on the device
    if (is_start) {
        if (alarm_next) {
            if (rtc_set_alarm(ri, alarm_next->alarm_timeout)) {
                alarm_callback(alarm_next->rtc_device);
            }
        } else {
            rtc_cancel_alarm(ri);
        }
    }
    return 0;
}

static uint64_t
alarm_create(struct virtual_rtc *virtual_rtc, uint64_t alarm_time_seconds, int flags, uint64_t current_time)
{
    struct rtc_device *rtc_device; 
    struct rtc_interface *ri; 
    int is_earliest;

    // Obtain the device and the device interface
    rtc_device = virtual_rtc->rtc_device;
    ri = rtc_device->ri;

    // De-activate the alarm if it is not cancelled
    if (virtual_rtc->alarm_state != rtc_alarm_cancelled_e)
        alarm_inactivate(virtual_rtc);

    // Set the alarm state
    if (flags & RTC_ALARM_PERIODIC) {

        // Can't be absolutely periodic
        if (flags & RTC_ALARM_ABSOLUTE) {
            return -1;
        }
        virtual_rtc->alarm_state = rtc_alarm_periodic_e;
        virtual_rtc->alarm_period = alarm_time_seconds;

    } else { // one-shot
        virtual_rtc->alarm_state = rtc_alarm_oneshot_e;
        virtual_rtc->alarm_period = 0;
    }

    // Set the alarm time
    // if not absolute, add the time to the current real time + the alarm_period
    // if absolute, subtract offset for rtc_device, if any
    // We end up with a real time alarm_timeout
    if (!(flags & RTC_ALARM_ABSOLUTE)) {
        virtual_rtc->alarm_timeout = current_time + alarm_time_seconds;
    } else {
        virtual_rtc->alarm_timeout = alarm_time_seconds - virtual_rtc->offset;
    }

    // Make the alarm active and determine if it is now the first
    is_earliest = alarm_activate(virtual_rtc);
    if (is_earliest) {
        
        // If the first, set the underlying device
        // If, by the time we set it, it had already passed, do the callback
        if (rtc_set_alarm(ri, virtual_rtc->alarm_timeout)) {
            alarm_callback(rtc_device);
        }
    }

    // Return the alarm_timeout, relative to the rtc_device
    return get_relative_time(virtual_rtc, virtual_rtc->alarm_timeout);
}


/***************************************/
/*   Alarm Interface Implementation    */
/***************************************/

uint64_t
virtual_rtc_request_alarm_impl(CORBA_Object _caller, device_t handle,
                               uint64_t alarm_time_seconds, int flags,
                               idl4_server_environment * _env)
{
    struct virtual_rtc *virtual_rtc = virtual_device_instance + handle;

    // We obtain the current time here and pass it in to enable re-use of the function
    // from set time, which needs to give it its own time
    return alarm_create(virtual_rtc, alarm_time_seconds, flags, rtc_get_time(virtual_rtc->rtc_device->ri));
}

int
virtual_rtc_cancel_alarm_impl(CORBA_Object _caller, device_t handle,
                              idl4_server_environment * _env)
{
    struct virtual_rtc *virtual_rtc = virtual_device_instance + handle;
    return alarm_cancel(virtual_rtc);
}

uint64_t
virtual_rtc_get_alarm_impl(CORBA_Object _caller, device_t handle,
                          idl4_server_environment * _env)
{
    // Obtain the vrtc from the obj
    struct virtual_rtc *vrtc = virtual_device_instance + handle;

    // Obtain the alarm timeout
    uint64_t time = vrtc->alarm_timeout;

    // Apply internal offset if one exists
    time = get_relative_time(vrtc, time);
    return time;
}

void
vrtc_async_handler(L4_Word_t notify)
{
    dprintf("%s: In async handler\n", __func__);

    if (notify & (1UL << 31)) {
        L4_Word_t irq;
        struct rtc_device *device;
        device = &rtc_device;

        irq = L4_GetUtcbBase()[__L4_TCR_PLATFORM_OFFSET];

        printf("%s: In async handler : %lx : %lu\n", __func__, notify, irq);
        if (device_interrupt(device->di, irq))
        {
            alarm_callback(device);
        }

        // ack IRQ
        L4_LoadMR(0, irq);
        L4_AcknowledgeInterrupt(0, 0);
    }
//    struct rtc_device *device;

    /* Search through devices and see if any of them registered a
     * TIMER_RESOURCE, and so may be expecting a notification. Will have to be
     * fixed if vrtc ever accepts more than one type of timer message. Ideally
     * the mask should be included in the timer resource struct. */
/*    if (notify & TIMER_TICK_MASK) {

        for (device = rtc_devices; device != NULL; device = device->next) {

            struct resource *resources;
            for (resources = device->resources;
                 resources && (resources->type != TIMER_RESOURCE);
                 resources = resources->next);

            if (!resources) {
                continue;
            }

            // Notify the device
            if (device_interrupt(device->di, L4_ThreadNo(resources->resource.timer.tid)) == 0) {
                // Perform the callback
                alarm_callback(device);
            }
        }
    }*/
}

int
virtual_rtc_init_impl(CORBA_Object _caller, device_t handle, L4_Word_t owner, uint32_t mask, idl4_server_environment * _env)
{
    struct virtual_rtc *vrtc = virtual_device_instance + handle;

    /*
     * Owner will be the thread that is notified when an event occurs
     * on the vrtc handle.  Therefore 'owner' must always be a
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

    if (!vrtc->valid_instance)
    {
        printf("vrtc: accessing invalid instance (handle: %d)\n", handle);
        return -1;
    }

    /* Re-set the owner if owner argument is different from the existing owner */
    if (!L4_IsThreadEqual(owner_tid, vrtc->thread))
    {
        if (!L4_IsNilThread(vrtc->thread))
        {
            printf
            (
                "vrtc: re-setting owner of instance (old_owner: %lx, new_owner: %lx, handle: %d)\n",
                vrtc->thread.raw, owner_tid.raw, handle
            );
        }

        vrtc->thread = owner_tid;
    }

    vrtc->mask = mask;

    printf
    (
        "vrtc: init done (handle: %d, owner: %lx, mask: %x)\n",
        handle, owner_tid.raw, (unsigned int)mask
    );

    return 0;
}

