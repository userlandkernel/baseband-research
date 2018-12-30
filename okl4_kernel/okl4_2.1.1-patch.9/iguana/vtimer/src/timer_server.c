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
 * Author: Ben Leslie Created: Tue Nov 2 2004
 */
/*
 * Comments:
 *
 * This basically comes down to implementing a priority queue. The is a lot of
 * literature on various types of priority queues. One that looks good is:
 *
 * Calendar Queues: a fast O(1) priority queue implementation for the
 * simulation event set problem (http://portal.acm.org/citation.cfm?id=63045)
 *
 * It seems that the data structure that should be used will (obviously) depend
 * on the type of events being added.
 */
/**
 * The timer server provides virtual timer device to other components
 * and applications.
 */

/**
 * Frequency expressed in Hz. Greatest frequency support is :
 * 18.446 Ghz -> derived from 2**64 / 10**9
 * 
 * Time is expressed in nanoseconds. Greatest time is:
 * 585 years -> derived from 2**64 / 10**9 / 3600 / 24 / 365
 * 
 * Timeouts are expressed in nanoseconds. Greatest timeout is:
 * 31 years -> derived from 10**9 seconds
 *
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

#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/pd.h>

#include <driver/driver.h>
#include <driver/device.h>
#include <driver/timer.h>

#include <circular_buffer/cb.h>
#include <range_fl/range_fl.h>

#include <interfaces/vtimer_serverdecls.h>

#include <compat/c.h>

/** 
 * TIMER_DRIVER is defined in the build system, and indicates
 * which device driver the timer server supports.
 */
extern struct driver TIMER_DRIVER;

#define TIMER_ONESHOT  1
#define TIMER_ABSOLUTE 2
#define TIMER_PERIODIC 4

/**
 * Virtual timers can be in one of three different states.
 */
enum virtual_timer_states { 
    timer_periodic_e, /**< Virtual timer is in periodic mode */
    timer_oneshot_e,  /**< Virtual timer is in oneshot mode */
    timer_cancelled_e /**< Virtual timer is currently cancelled */
};

struct timer_device;

/**
 * Holds the information for each virtual timer.
 */
struct virtual_timer {
    int                 valid_instance;
    L4_ThreadId_t       thread;
    uint32_t            mask;
    uint64_t            timeout;
    int                 flags;
    uint64_t            period;
    
    struct timer_device     *timer_device;
    struct virtual_timer    *next;
    struct virtual_timer    *prev;
    
    enum virtual_timer_states state;
};

/** 
 * The timer device struct holds information for each physical timer
 * device in the server. Most servers only have one timer.
 */
struct timer_device {
    struct timer_device     *next;
    struct resource         *resources;
    
    struct device_interface *di;
    struct timer_interface  *ti;
    
    struct virtual_timer    *active;
    struct virtual_timer    *active_end;
    struct virtual_timer    *inactive;
    
    uint64_t                frequency;
};

/* Forward declarations */
static void insert_inactive(struct virtual_timer *virtual_timer);
static void callback(int, struct timer_device*);
static inline uint64_t ns_to_ticks(uint64_t frequency, uint64_t nanoseconds);

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
static struct virtual_timer virtual_device_instance[4] = { { 1 } }; /* valid but unused instances are ok */

static struct resource      resources[8];
static struct timer_device  timer_device;


#define GIGA 1000000000

static inline 
uint64_t
ns_to_ticks(uint64_t frequency, uint64_t nanoseconds)
{
    uint32_t seconds = nanoseconds / GIGA;
    uint32_t overflow = nanoseconds % GIGA;
    /*
     * Assuming freq < 18.44 GHz (see below), this shouldn't overflow 
     * unless seconds > 10**9 == 31 years
     */
    uint64_t ticks = (frequency * seconds);
    /* 
     * This shouldn't overflow, 
     * unless frequency > (2**64 / 10**9 == 18.44 GHz)
     */
    ticks += ((frequency * overflow) / GIGA);
    return ticks;
}

static inline
uint64_t
ticks_to_ns(uint64_t frequency, uint64_t ticks)
{
    uint64_t seconds = ticks / frequency;
    uint64_t overflow = ticks % frequency;
    uint64_t ns = seconds * GIGA;
    ns += overflow * GIGA / frequency;
    return ns;
}



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
    printf("In vtimer now\n");
    printf("Let's have a look at what I got:\n");
    printf("Physmem:  \n\t1. %p\n\t2. %p\n\t3. %p\n\t4. %p\n",
        iguana_granted_physmem[0], iguana_granted_physmem[1],
        iguana_granted_physmem[2], iguana_granted_physmem[3]);
    printf("Interrupt:\n\t1. 0x%x\n\t2. 0x%x\n\t3. 0x%x\n\t4. 0x%x\n",
        iguana_granted_interrupt[0], iguana_granted_interrupt[1],
        iguana_granted_interrupt[2], iguana_granted_interrupt[3]);
    printf("Size of vd struct %d\n", sizeof(struct virtual_timer));
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
    device = malloc(TIMER_DRIVER.size);
    if (device == NULL) {
        free(device);
    }
    
    /* Initialise the device state */
    {
        timer_device.next = NULL;
        timer_device.active = NULL;
        timer_device.active_end = NULL;
        timer_device.inactive = NULL;
        timer_device.resources = resources;
    }
    
    /* Create and enable the device */
    timer_device.di = setup_device_instance(&TIMER_DRIVER, device);
    device_setup(timer_device.di, timer_device.resources);
    
    timer_device.ti = (struct timer_interface *)device_get_interface(timer_device.di, 0);
    device_enable(timer_device.di);
    
    timer_device.frequency = timer_get_tick_frequency(timer_device.ti);
    
    /*
     * XXX: Was: virtual_timer_factory_create_impl()
     *
     * Clagged from the function.  Should be cleaned up.
     */
    for (i = 0; i < 4; i++)
    {
        if (virtual_device_instance[i].valid_instance)
        {
            virtual_device_instance[i].timer_device = (void *)(&timer_device);
            virtual_device_instance[i].state = timer_cancelled_e;
            // XXX virtual_device_instance[i].thread = *thread;
            // XXX virtual_device_instance[i].mask = mask;
            
            insert_inactive(&virtual_device_instance[i]);
        }
    }
    /* Receives async IRQS's.*/
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0UL);
    
    vtimer_server_loop();
    return 0;

    (void)iguana_granted_physmem;
    (void)iguana_granted_interrupt;
    (void)buffer;
    (void)virtual_device_instance;
}

int
virtual_timer_init_impl(CORBA_Object _caller, device_t handle, L4_Word_t owner, uint32_t mask, idl4_server_environment * _env)
{
    struct virtual_timer *vtimer = virtual_device_instance + handle;

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

    if (!vtimer->valid_instance)
    {
        printf("vtimer: accessing invalid instance (handle: %d)\n", handle);
        return -1;
    }

    /* Re-set the owner if owner argument is different from the existing owner */
    if (!L4_IsThreadEqual(owner_tid, vtimer->thread))
    {
        if (!L4_IsNilThread(vtimer->thread))
        {
            printf
            (
                "vtimer: re-setting owner of instance (old_owner: %lx, new_owner: %lx, handle: %d)\n",
                vtimer->thread.raw, owner_tid.raw, handle
            );
        }

        vtimer->thread = owner_tid;
    }

    vtimer->mask = mask;

    printf
    (
        "vtimer: init done (handle: %d, owner: %lx, mask: %x)\n",
        handle, owner_tid.raw, (unsigned int)mask
    );

    return 0;
}

/**
 * Insert the virtual timer into the timer device's inactive list.
 */
static void
insert_inactive(struct virtual_timer *virtual_timer)
{
    struct timer_device *timer_device = virtual_timer->timer_device;
    if (timer_device->inactive == NULL) {
        /* Empty device list */
        timer_device->inactive = virtual_timer;
        virtual_timer->next = virtual_timer->prev = NULL;
    } else {
        timer_device->inactive->prev = virtual_timer;
        virtual_timer->next = timer_device->inactive;
        virtual_timer->prev = NULL;
        timer_device->inactive = virtual_timer;
    }
}

#if 0
static void
debug_active(struct timer *timer)
{
    struct timer_service *timer_service = timer->timer_service;
    struct timer *next;

    for (next = timer_service->active; next != NULL; next = next->next) {
        DEBUG_TRACE(3, "%" PRId64 " (%p) -- ", next->timeout, next);
    }
    DEBUG_TRACE(3, "\n");
}
#endif

#if 0
static void
delete_inactive(struct timer *timer)
{
    struct timer_service *timer_service = timer->timer_service;

    if (timer->next) {
        timer->next->prev = timer->prev;
    }

    if (timer->prev) {
        timer->prev->next = timer->next;
    }

    if (timer_service->inactive == timer) {
        timer_service->inactive = timer->next;
    }
}
#endif

static int
make_active(struct virtual_timer *vtimer)
{
    struct timer_device *timer_device;
    int in_front = 0;

    assert(vtimer);

    timer_device = vtimer->timer_device;
    assert(timer_device);

    /*
     * If LL head is null, then active_end must be null else active_end
     * must not be null.
     */
    assert((timer_device->active == NULL) ?
           (timer_device->active_end == NULL) :
           (timer_device->active_end != NULL));

    /* Remove from inactive */
    if (vtimer->next != NULL) {
        vtimer->next->prev = vtimer->prev;
    }
    if (vtimer->prev != NULL) {
        vtimer->prev->next = vtimer->next;
    }
    if (timer_device->inactive == vtimer) {
        timer_device->inactive = vtimer->next;
    }

    /* Insert into active list */
    if (timer_device->active_end == NULL) {
        /* 
         *  We can just insert at the head of the empty
         *  list 
         */
        assert(timer_device->active == NULL);
        timer_device->active = vtimer;
        timer_device->active_end = vtimer;
        vtimer->prev = vtimer->next = NULL;
        in_front = 1;
    } else if (vtimer->timeout < timer_device->active->timeout) {
        /* We must insert at the front -- this is the next wakeup!  */
        timer_device->active->prev = vtimer;
        vtimer->next = timer_device->active;
        vtimer->prev = NULL;
        timer_device->active = vtimer;
        in_front = 1;
    } else {
        /*
         * We need to insert in an ordered way. Note: This could be O(n), which
         * may be bad depending on exactly how things work, however we don't
         * expect there to be a lot of virtual timers per device.
         */
        struct virtual_timer *back = timer_device->active_end;

        /*
         * Search backwards until we are behind an existing virtual timer
         */
        while (vtimer->timeout < back->timeout) {
            back = back->prev;
            assert(back != NULL);
            /*
             * This could only happen if we are next, which we can't be because
             * we already checked that!
             */
        }

        /* Now we insert behind back */
        vtimer->next = back->next;
        vtimer->prev = back;
        back->next = vtimer;
        if (vtimer->next) {
            vtimer->next->prev = vtimer;
        }
        if (timer_device->active_end == back) {
            timer_device->active_end = vtimer;
        }
        in_front = 0;
    }

    /* Sanity check our list */
    /*
     * If we have a vtimer after us, then it should point back to
     *  and it should have a timeout after us
     */
    assert(vtimer->next ?
           vtimer->next->prev == vtimer :
           timer_device->active_end == vtimer);
    assert(vtimer->next ?
           vtimer->next->timeout >= vtimer->timeout :
           timer_device->active_end == vtimer);
    /* 
     * If we have a timeout before us, then it should point forward to us
     *  and it should have a timeout earlier than us 
     */
    assert(vtimer->prev ?
           vtimer->prev->next == vtimer :
           timer_device->active == vtimer);
    assert(vtimer->prev ?
           vtimer->prev->timeout <= vtimer->timeout :
           timer_device->active == vtimer);
    
    /* Return whether we were in front of another virtual timer */
    return in_front;
}

static void
deactivate_timer(struct virtual_timer *vtimer)
{
    if (vtimer->next) {
        vtimer->next->prev = vtimer->prev;
    }

    if (vtimer->prev) {
        vtimer->prev->next = vtimer->next;
    }

    if (vtimer->timer_device->active_end == vtimer) {
        vtimer->timer_device->active_end = vtimer->prev;
    }

    if (vtimer->timer_device->active == vtimer) {
        vtimer->timer_device->active = vtimer->next;
    }

    /* Put in the inactive */
    vtimer->state = timer_cancelled_e;

    /*
     * If LL head is not null, then active_end must be non null else active_end
     * must be null
     */
    assert((vtimer->timer_device->active != NULL) ?
           (vtimer->timer_device->active_end != NULL) :
           (vtimer->timer_device->active_end == NULL));

    insert_inactive(vtimer);
}

static void
callback(int reason, struct timer_device *timer_device)
{
    struct timer_interface *ti = timer_device->ti;
    struct virtual_timer *vtimer = timer_device->active;
    struct virtual_timer *next_timer;

    if (vtimer) {

        // Check the interrupt actually means the time has expired
        // (posisble for narrow timers that may need several wraps to
        // arraive at the desired time
        if (  ticks_to_ns(timer_device->frequency, timer_get_ticks(ti)) < vtimer->timeout ){
            // Not yet.
            return;
        }

        do {
            /* Send the notification */

            L4_Notify(vtimer->thread, vtimer->mask);
            /* The timer should be at the start of the list! */
            assert(vtimer->prev == NULL);

            if (vtimer->state == timer_oneshot_e) {
                /* Remove timer from active list */
                next_timer = vtimer->next;
                deactivate_timer(vtimer);
            } else {
                /*
                 * Period timer -- needs to be put straight back in the active
                 * queue
                 */
                vtimer->timeout += vtimer->period;
                if (vtimer->next == NULL || vtimer->next->timeout > vtimer->timeout) {
                    /*
                     * Special case -- we are still at the front of the list so we
                     * don't need to do very much... XXX: Periodic timers should
                     * cost a lot of quota, this could easily DOS a system if
                     * someone request a short periodic timeout
                     */
                    next_timer = vtimer;
                } else {
                    /* Ok, so we are not going to be at the front this time */
                    struct virtual_timer *back = vtimer->timer_device->active_end;

                    next_timer = vtimer->next;
                    vtimer->timer_device->active = next_timer;
                    next_timer->prev = NULL;
                    while (back->timeout > vtimer->timeout) {
                        back = back->prev;
                        assert(back != NULL);
                        /*
                         * This could only happen if we are next, which we can't be
                         * because we already checked that!
                         */
                    }
                    /* Now we insert behind back */
                    vtimer->next = back->next;
                    vtimer->prev = back;
                    back->next = vtimer;
                    if (vtimer->next) {
                        vtimer->next->prev = vtimer;
                    }
                    if (vtimer->timer_device->active_end == back) {
                        vtimer->timer_device->active_end = vtimer;
                    }
                }
            }

            vtimer = NULL;
            if (next_timer) {
                /* We need to add the next timer */
                int r;
                /* FIXME!!! */
                r = timer_timeout(ti, next_timer->timeout);
                if (r) {
                    /* This has already expired, so we loop */
                    //                L4_KDB_PrintChar('%');
                    vtimer = next_timer;
                }
            }
        } while (vtimer);
    }
}

int
virtual_timer_cancel_impl
(
    CORBA_Object    _caller,
    device_t        handle,
    idl4_server_environment * _env
)
{
    /*
     * XXX - have to check for valid handle and tid and blah
     * - nt
     */
    struct virtual_timer    *vtimer = virtual_device_instance + handle;
    struct timer_device     *device = vtimer->timer_device;
    int is_start = (vtimer->prev == NULL);
    struct virtual_timer *next_timer = vtimer->next;

    if (vtimer->state == timer_cancelled_e){
        return 1;
    } else {
        DEBUG_TRACE(3, "Cancelling timer\n");
    }

    assert(is_start ? device->active == vtimer : 1);
    assert(next_timer == NULL ? device->active_end == vtimer : 1);

    /* Inactivate timer */
    deactivate_timer(vtimer);
    if (is_start) {
        if (next_timer) {
            /* Timeout at next registered time */
            if (timer_timeout(device->ti, next_timer->timeout)) {
                callback(0, device);
            }
        } else {
            /* Never timeout -- nothing left in list */
            timer_timeout(device->ti, -1);
        }
    }
    return 0;
}

uint64_t
virtual_timer_request_impl
(
   CORBA_Object     _caller,
   device_t         handle,
   uint64_t         time_nanoseconds,
   int              flags, 
   idl4_server_environment * _env
)
{
    /*
     * XXX - have to check for valid handle and tid and blah
     * - nt
     */
    struct virtual_timer    *vtimer = virtual_device_instance + handle;
    struct timer_interface  *ti = timer_device.ti;
    
    uint64_t ticks, abs_ticks;
    int is_earliest;

    DEBUG_TRACE(4, "Timer request: %p\n", vtimer);
    
    DEBUG_TRACE(4, "Timer_device: %p\n", &timer_device);
    DEBUG_TRACE(4, "Timer_device freq: %lld\n", timer_device.frequency);

    /* Convert nano-seconds to ticks */
    ticks = ns_to_ticks(timer_device.frequency, time_nanoseconds);

    DEBUG_TRACE(4, "Request time: %lld %lld %lld %lld\n", time_nanoseconds, ticks, timer_get_ticks(ti), ticks_to_ns(timer_device.frequency, timer_get_ticks(ti)));

    if (vtimer->state != timer_cancelled_e) {
        DEBUG_TRACE(4, "Cancelling current timer...\n");
        deactivate_timer(vtimer);
    }

    abs_ticks = ticks;

    if (!(flags & TIMER_ABSOLUTE)) {
        /* FIXME */
        uint64_t add = timer_get_ticks(ti);
        abs_ticks += add;
    }

    if (flags & TIMER_PERIODIC) {
        if (flags & TIMER_ABSOLUTE) {
            return -1;
        }
        vtimer->state = timer_periodic_e;
        vtimer->period = ticks;
    } else {
        vtimer->state = timer_oneshot_e;
    }

    vtimer->timeout = abs_ticks;

    DEBUG_TRACE(5, "Making active!\n");

    is_earliest = make_active(vtimer);

#ifdef DEBUG_VTIMER
    debug_active(vtimer);
#endif

    if (is_earliest) {
        /* Set a request for that point in time */
        int r;
        DEBUG_TRACE(3, "Earliest! setting timer timeout\n");
        /* FIXME */
        r = timer_timeout(ti, abs_ticks);
        DEBUG_TRACE(3, "done timeout\n");
        if (r == 1) {
            DEBUG_TRACE(3, "Doing callback...\n");
            /* By the time we set it, it had already passed */
            callback(0, &timer_device);
            DEBUG_TRACE(3, "Done callback...\n");
        }
    }
    /* Convert ticks to ns */
    return ticks_to_ns(timer_device.frequency, abs_ticks);
}

uint64_t
virtual_timer_current_time_impl
(
    CORBA_Object    _caller,
    device_t        handle,
    idl4_server_environment * _env
)
{
    /*
     * XXX - have to check for valid handle and tid and blah
     * - nt
     */
    struct virtual_timer    *vtimer = virtual_device_instance + handle;
    struct timer_device     *timer_device = vtimer->timer_device;
    struct timer_interface *ti = timer_device->ti;

    /* FIXME */
    uint64_t ticks = timer_get_ticks(ti);
    return ticks_to_ns(timer_device->frequency, ticks);
}

int
virtual_timer_delete_impl
(
    CORBA_Object    _caller,
    device_t        handle,
    idl4_server_environment * _env
)
{
#if 0
    /* First it must be cancelled */
    struct timer *timer = (struct timer *)timer_obj;

    if (timer->state != timer_cancelled_e) {
        /* We need to cancel the timer first */
        timer__cancel_impl(_caller, timer_obj, _env);
    }

    /* Now we can remove from inactive */
    delete_inactive(timer);
    free(timer);
#endif
    return 0;
}

void
vtimer_async_handler(L4_Word_t notify)
{
    if (notify & (1UL << 31)) {
        L4_Word_t irq;

        irq = __L4_TCR_PlatformReserved(0);

        //printf("%s: In async handler : %lx : %lu\n", __func__, notify, irq);
        if (device_interrupt(timer_device.di, irq))
        {
            callback(0, &timer_device);
        }

        // ack IRQ
        L4_LoadMR(0, irq);
        L4_AcknowledgeInterrupt(0, 0);
    }
}

