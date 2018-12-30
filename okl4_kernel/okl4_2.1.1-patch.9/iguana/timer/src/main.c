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

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/kdebug.h>

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/object.h>
#include <iguana/pd.h>
#include <iguana/env.h>

#include <driver/driver.h>
#include <driver/driver_ops.h>
#include <driver/timer_ops.h>

#include <naming/naming.h>

#include <circular_buffer/cb.h>
#include <range_fl/range_fl.h>

#include <interfaces/timer_serverdecls.h>

#define IRQ_LABEL (((~(0UL)) << 20) >> 16)      /* This should be somewhere
                                                 * better */

extern struct driver TIMER_DRIVER;

extern void *__callback_buffer;
struct cb_get_handle *iguana_cb_handle;

struct timer_service;

struct driver *global_device;
struct timer_service *global_timer_service;

#define TIMER_ONESHOT  1
#define TIMER_ABSOLUTE 2
#define TIMER_PERIODIC 4

enum timer_states { timer_periodic_e, timer_oneshot_e, timer_cancelled_e };

struct timer {
    struct timer *next;
    struct timer *prev;
    struct timer_service *timer_service;
    uint64_t timeout;
    int flags;
    L4_ThreadId_t thread;
    uintptr_t mask;
    enum timer_states state;
    uint64_t period;
};

struct timer_service {
    struct timer *active;
    struct timer *active_end;
    struct timer *inactive;
    struct driver *device;
};

#ifdef ENABLE_PM_THREAD
uintptr_t pm_stack[1000];
thread_ref_t pm_thread;

static void
power_management_thread(void)
{
    uint64_t last_timeout = 0;

    while (1) {
        uint64_t next_timeout;

        if (global_timer_service->active) {
            next_timeout = global_timer_service->active->timeout;
        } else {
            next_timeout = 0;
        }
        if (next_timeout != last_timeout) {
            last_timeout = next_timeout;
#if 0
            if (last_timeout != 0) {
                DEBUG_PRINT("   ====> PowerMgt: next wakeup .. %" PRId64 " <====\n",
                       last_timeout - timer_current_time(global_device));
            } else {
                DEBUG_PRINT("   ====> PowerMgt: no wakups <====\n");
            }
#endif
        }
    }
}
#endif

static L4_ThreadId_t main_tid;

int
main(int argc, char **argv)
{
    struct driver_ops *ops;
    objref_t obj;

    /*
     * XXX: Should we be inserting the timer device into naming here?
     */
    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    obj = device_create_impl(main_tid, 0UL, NULL);
    naming_insert("timer", obj);

    ops = TIMER_DRIVER.ops.d_ops;

    iguana_cb_handle = cb_attach(__callback_buffer);

#ifdef ENABLE_PM_THREAD
    {
        L4_ThreadId_t unused;

        pm_thread = thread_create_priority(1, &unused);
        L4_KDB_SetThreadName(unused, "PwrMgmt");
    }
#endif
    L4_Accept(L4_AsynchItemsAcceptor);
    L4_Set_NotifyMask(1UL << 31);
    timer_server_loop();
    assert(!"Should reach here\n");

    return 0;
}

objref_t
device_create_impl(CORBA_Object _caller, objref_t driver,
                   idl4_server_environment * _env)
{
    struct driver_ops *ops;
    struct timer_service *timer_service;
    memsection_ref_t memsection;
    thread_ref_t myself, unused;

    ops = TIMER_DRIVER.ops.d_ops;

    /* Let us actually allocate the backed memory.. */

    global_timer_service = timer_service = malloc(sizeof(struct timer_service));

#ifdef ENABLE_PM_THREAD
    thread_start(pm_thread, (uintptr_t)power_management_thread,
                 (uintptr_t)&pm_stack[1000 - 1]);
#endif

    if (timer_service == NULL) {
        return 0;
    }
    timer_service->active = timer_service->active_end = NULL;
    timer_service->inactive = NULL;
    global_device = timer_service->device = ops->setup(0, NULL, NULL, NULL);

    if (timer_service->device == NULL) {
        free(timer_service);
        return 0;
    }

    driver_enable(timer_service->device);

    memsection = memsection_lookup((objref_t)timer_service, &unused);
    myself = thread_id(main_tid);
    memsection_register_server(memsection, myself);

    return (objref_t)timer_service;
}

#if 0
static void
debug_active(struct timer *timer)
{
    struct timer_service *timer_service = timer->timer_service;
    struct timer *next;

    for (next = timer_service->active; next != NULL; next = next->next) {
        DEBUG_PRINT("%" PRId64 " (%p) -- ", next->timeout, next);
    }
    DEBUG_PRINT("\n");
}
#endif

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

static void
insert_inactive(struct timer *timer)
{
    struct timer_service *timer_service = timer->timer_service;

    if (timer_service->inactive == NULL) {
        timer_service->inactive = timer;
        timer->next = timer->prev = NULL;
    } else {
        timer_service->inactive->prev = timer;
        timer->next = timer_service->inactive;
        timer->prev = NULL;
        timer_service->inactive = timer;
    }
}

static int
make_active(struct timer *timer)
{
    struct timer_service *timer_service;
    int in_front = 0;

    assert(timer);

    timer_service = timer->timer_service;
    assert(timer_service);

    /*
     * If LL head is not null, then active_end must be non null else active_end
     * must be null
     */
    assert((timer->timer_service->active != NULL) ?
           (timer->timer_service->active_end != NULL) :
           (timer->timer_service->active_end == NULL));

    /* Remove from inactive */
    if (timer->next != NULL) {
        timer->next->prev = timer->prev;
    }
    if (timer->prev != NULL) {
        timer->prev->next = timer->next;
    }
    if (timer_service->inactive == timer) {
        timer->timer_service->inactive = timer->next;
    }

    /* Insert into active list */
    if (timer_service->active_end == NULL) {
        assert(timer_service->active == NULL);
        /* We can just insert!  */
        timer_service->active = timer;
        timer_service->active_end = timer;
        timer->prev = timer->next = NULL;
        in_front = 1;
    } else if (timer->timeout < timer_service->active->timeout) {
        /* We must insert at the front -- this is the next wakeup!  */
        timer_service->active->prev = timer;
        timer->next = timer_service->active;
        timer->prev = NULL;
        timer_service->active = timer;
        in_front = 1;
    } else {
        /*
         * We need to insert in an ordered way. Note: This could be O(n), which
         * may be bad depending on exactly how things work
         */
        struct timer *back = timer_service->active_end;

        while (back->timeout > timer->timeout) {
            back = back->prev;
            assert(back != NULL);
            /*
             * This could only happen if we are next, which we can't be because
             * we already checked that!
             */
        }

        /* Now we insert behind back */
        timer->next = back->next;
        timer->prev = back;
        back->next = timer;
        if (timer->next) {
            timer->next->prev = timer;
        }
        if (timer_service->active_end == back) {
            timer_service->active_end = timer;
        }
        in_front = 0;
    }

    assert(timer->next ? timer->next->prev ==
           timer : timer_service->active_end == timer);
    assert(timer->prev ? timer->prev->next == timer : timer_service->active ==
           timer);
    assert(timer->next ? timer->next->timeout >=
           timer->timeout : timer_service->active_end == timer);
    assert(timer->prev ? timer->prev->timeout <=
           timer->timeout : timer_service->active == timer);

    return in_front;
}

static void
deactivate_timer(struct timer *timer)
{
    if (timer->next) {
        timer->next->prev = timer->prev;
    }

    if (timer->prev) {
        timer->prev->next = timer->next;
    }

    if (timer->timer_service->active_end == timer) {
        timer->timer_service->active_end = timer->prev;
    }

    if (timer->timer_service->active == timer) {
        timer->timer_service->active = timer->next;
    }

    /* Put in the inactive */
    timer->state = timer_cancelled_e;

    /*
     * If LL head is not null, then active_end must be non null else active_end
     * must be null
     */
    assert((timer->timer_service->active != NULL) ?
           (timer->timer_service->active_end != NULL) :
           (timer->timer_service->active_end == NULL));

    insert_inactive(timer);
}

L4_ThreadId_t recv_thread;
static void
timer_callback(callback_data_t data, int ignored, struct data *ignored_data,
               uintptr_t key)
{
    struct timer *timer = (struct timer *)key;
    struct timer *next_timer;
    struct driver *device = timer->timer_service->device;

    do {
        L4_Notify(timer->thread, timer->mask);

        /* The timer should be at the start of the list! */
        assert(timer->timer_service->active == timer);
        assert(timer->prev == NULL);

        if (timer->state == timer_oneshot_e) {
            /* Remove timer from active list */
            next_timer = timer->next;
            deactivate_timer(timer);
        } else {
            /*
             * Period timer -- needs to be put straight back in the active
             * queue
             */
            timer->timeout += timer->period;
            if (timer->next == NULL || timer->next->timeout > timer->timeout) {
                /*
                 * Special case -- we are still at the front of the list so we
                 * don't need to do very much... XXX: Periodic timers should
                 * cost a lot of quota, this could easily DOS a system if
                 * someone request a short periodic timeout
                 */
                next_timer = timer;
            } else {
                /* Ok, so we are not going to be at the front this time */
                struct timer *back = timer->timer_service->active_end;

                next_timer = timer->next;
                timer->timer_service->active = next_timer;
                next_timer->prev = NULL;
                while (back->timeout > timer->timeout) {
                    back = back->prev;
                    assert(back != NULL);
                    /*
                     * This could only happen if we are next, which we can't be
                     * because we already checked that!
                     */
                }
                /* Now we insert behind back */
                timer->next = back->next;
                timer->prev = back;
                back->next = timer;
                if (timer->next) {
                    timer->next->prev = timer;
                }
                if (timer->timer_service->active_end == back) {
                    timer->timer_service->active_end = timer;
                }
            }
        }

        timer = NULL;
        if (next_timer) {
            /* We need to add the next timer */
            if (timer_timeout
                (device, next_timer->timeout, timer_callback, 0,
                 (uintptr_t)next_timer)) {
                timer = next_timer;
            }
        }
    } while (timer);
}

/*
 * Create a new timer object
 */
cap_t
timer_server_create_impl(CORBA_Object _caller, objref_t timer_service,
                         L4_ThreadId_t *thread, uintptr_t mask,
                         idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    L4_ThreadId_t subject_thread;
    struct timer *timer;
    timer = malloc(sizeof(struct timer));

    assert(thread->raw != L4_myselfconst.raw);
    subject_thread = *thread;
    if (timer == NULL) {
        return cap;
    }

    /* XXX: We always use the global_timer_service. */
    timer->timer_service = global_timer_service;
    timer->state = timer_cancelled_e;
    timer->thread = subject_thread;
    timer->mask = mask;
    insert_inactive(timer);

    cap.ref.obj = (uintptr_t)timer;
    /*
     * FIXME: set up a password
     */
    return cap;
}

int
timer__cancel_impl(CORBA_Object _caller, objref_t timer_obj,
                   idl4_server_environment * _env)
{
    struct timer *timer = (struct timer *)timer_obj;
    struct driver *device = timer->timer_service->device;
    int is_start = (timer->prev == NULL);
    struct timer *next_timer = timer->next;

    if (timer->state == timer_cancelled_e)
        return 1;

    assert(is_start ? timer->timer_service->active == timer : 1);
    assert(next_timer == NULL ? timer->timer_service->active_end == timer : 1);

    /* Inactivate timer */
    deactivate_timer(timer);
    if (is_start) {
        if (next_timer) {
            /* Timeout at next registered time */
            if (timer_timeout
                (device, next_timer->timeout, timer_callback, 0,
                 (uintptr_t)next_timer)) {
                timer_callback(0, 0, NULL, (uintptr_t)timer);
            }
        } else {
            /* Never timeout -- nothing left in list */
            timer_timeout(device, -1, NULL, 0, (uintptr_t)0);
        }
    }

    return 0;
}

uint64_t
timer__request_impl(CORBA_Object _caller, objref_t timer_obj,
                    uint64_t time, int flags, idl4_server_environment * _env)
{
    struct timer *timer = (struct timer *)timer_obj;
    struct driver *device = timer->timer_service->device;
    uint64_t abs_time;
    int is_earliest;

    if (timer->state != timer_cancelled_e) {
        /* We need to cancel the timer first */
        timer__cancel_impl(_caller, timer_obj, _env);
    }

    abs_time = time;

    if (!(flags & TIMER_ABSOLUTE)) {
        abs_time += timer_current_time(device);
    }

    if (flags & TIMER_PERIODIC) {
        if (flags & TIMER_ABSOLUTE) {
            return -1;
        }
        timer->state = timer_periodic_e;
        timer->period = time;
    } else {
        timer->state = timer_oneshot_e;
    }

    timer->timeout = abs_time;
    is_earliest = make_active(timer);
    // debug_active(timer);

    /* Set a request for that point in time */
    if (is_earliest) {
        if (timer_timeout
            (device, abs_time, timer_callback, 0, (uintptr_t)timer)) {
            timer_callback(0, 0, NULL, (uintptr_t)timer);
        }
    }

    return abs_time;
}

uint64_t
timer__current_time_impl(CORBA_Object _caller, idl4_server_environment * _env)
{
    return timer_current_time(global_timer_service->device);
}

int
timer__delete_impl(CORBA_Object _caller, objref_t timer_obj,
                   idl4_server_environment * _env)
{
    /* First it must be cancelled */
    struct timer *timer = (struct timer *)timer_obj;

    if (timer->state != timer_cancelled_e) {
        /* We need to cancel the timer first */
        timer__cancel_impl(_caller, timer_obj, _env);
    }

    /* Now we can remove from inactive */
    delete_inactive(timer);
    free(timer);

    return 0;
}

/*
 * check_async called from server loop before every message - see timer.idl4
 */
void
check_async(void)
{
    uintptr_t *size;

    /*
     * Now check async queues -- we check this everytime, which is a bit silly,
     * would be nice if we could detect a failed ipc to us
     */
    size = cb_get(iguana_cb_handle, sizeof(uintptr_t));
    if (size) {
        // uintptr_t *data =
        cb_get(iguana_cb_handle, *size);
        // DEBUG_PRINT("Got stuff: %" PRIdPTR "\n", *size);
        // DEBUG_PRINT("Got stuff: %" PRIxPTR " %" PRIxPTR "\n", data[0], data[1]);
    }
}

void
timer_async_handler(L4_Word_t bits)
{
    if((bits & (1UL << 31)) != 0)
        driver_interrupt(global_device, -1);
}

#if 0
void
device_server(void)
{
    L4_ThreadId_t partner;
    L4_MsgTag_t msgtag;
    idl4_msgbuf_t msgbuf;
    long cnt;
    uintptr_t *size;

    partner = L4_nilthread;
    msgtag.raw = 0;
    cnt = 0;

    while (1) {
        /*
         * Now check async queues -- we check this everytime, which is a bit
         * silly, would be nice if we could detect a failed ipc to us
         */
        size = cb_get(iguana_cb_handle, sizeof(uintptr_t));
        if (size) {
            // uintptr_t *data =
            cb_get(iguana_cb_handle, *size);
            // DEBUG_PRINT("Got stuff: %" PRIdPTR "\n", *size);
            // DEBUG_PRINT("Got stuff: %" PRIxPTR " %" PRIxPTR "\n", data[0],
            // data[1]);
        }
        /* Wait for message */

        idl4_msgbuf_sync(&msgbuf);
        idl4_reply_and_wait(&partner, &msgtag, &msgbuf, &cnt);

        if (idl4_is_error(&msgtag)) {
            DEBUG_PRINT("timer:device_server(%d) Error %" PRIxPTR
                   " -- replying_to: %" PRIxPTR "\n", __LINE__, L4_ErrorCode(),
                   partner.raw);
            partner = L4_nilthread;
            msgtag.raw = 0;
            cnt = 0;
            continue;
        }

        if (msgtag.X.label == IRQ_LABEL) {
            driver_interrupt(global_device, partner.global.X.thread_no);

            continue;
        }
        switch (magpie_get_interface_bigid(&msgbuf)) {
        case 37:
            idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
                                 device_vtable[idl4_get_function_id(&msgtag) &
                                               DEVICE_FID_MASK]);
            break;
        case 40:
            idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
                                 timer_server_vtable[idl4_get_function_id
                                                     (&msgtag) &
                                                     TIMER_SERVER_FID_MASK]);
            break;
        case 41:
            idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
                                 timer__vtable[idl4_get_function_id(&msgtag) &
                                               TIMER__FID_MASK]);
            break;
        default:
            DEBUG_PRINT("timer: device_server: Bad label! (0x%lx)\n",
                   magpie_get_interface_bigid(&msgbuf));
            partner = L4_nilthread;
        }
    }
}
#endif
