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
 * Author: David Mirabito
 */
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <util/trace.h>

#include <interfaces/event_serverdecls.h>

#include <iguana/memsection.h>

#include <event/event.h>

#include <l4/misc.h>


/*
 * Event Types
 */
struct listener {
    L4_ThreadId_t tid;
    L4_Word_t mask;
    struct listener*next;
};

struct event {
    struct listener * l;
} events[MAX_EVENTS];

/*
 * Wombat management Types
 */

struct wombat_instance {
    L4_ThreadId_t notif_thread;
    struct wombat_instance *next;
};

static struct wombat_instance *whead;
static struct wombat_instance *wtail;

static void handle_wombat_switch(void);

/*
 * Entry point
 */
int
main(int argc, char * argv[])
{
    int i;

    for (i = 0; i < MAX_EVENTS; i++)
        events[i].l = NULL;

    whead = wtail = NULL; /* OK, probably not necessary; I like it */
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(~0);

    event_server_loop();
}


/*
 * Event section
 */

int 
event_register_interest_impl(L4_ThreadId_t caller, L4_ThreadId_t *tid, L4_Word_t mask, 
        int event, idl4_server_environment *env){
    struct listener * l;

    if (event >= MAX_EVENTS) return -2;
    l = (struct listener * )malloc(sizeof (struct listener));
    if (!l) return -1;

    l->tid = *tid;
    l->mask = mask;
    l->next = events[event].l;
    
    events[event].l = l;

    return 0;

}

static void
signal_event(int i){
    struct listener * l;
    
    for (l = events[i].l ; l; l = l->next){
        DEBUG_TRACE(2,
                    "SERVER: pinging %08lx on %08lx\n",
                    l->tid.raw,
                    l->mask);
        L4_Notify(l->tid, l->mask);
    }
}


void
event_async_handler(L4_Word_t notify){
    int i;
    if (notify & (1<<EV_WOMBAT_SWITCH)){
        handle_wombat_switch();
    }
    for (i = 0; i < MAX_EVENTS; i++){
        if (notify & (1<<i)){
            DEBUG_TRACE(2, 
                        "SERVR: will signal for %d\n",
                        i);
            signal_event(i);
        }
    }
}


/*
 * Wombat mangement section
 */


static void
handle_wombat_switch(void)
{
    /* At this point, all events lead to one action, so not checking */

    /* Rotate list of instances so that whead goes to wtail and whead->next
       becomes whead */
    struct wombat_instance *old_head = whead;
    struct wombat_instance *old_tail = wtail;

    if (whead == wtail) {
        /* Either 0 or 1 registered wombat instances; no swapping possible */
        return;
    }
    whead = old_head->next;
    old_head->next = NULL;
    old_tail->next = old_head;
    wtail = old_head;

    DEBUG_TRACE(1,
                "handle_self_event.  Main tid is now 0x%lx\n",
                whead->notif_thread.raw);
}


int
wombat_threads_new_wombat_impl(L4_ThreadId_t caller,
                               L4_ThreadId_t *tid,
                               L4_ThreadId_t *instance,
                               idl4_server_environment *env)
{
    int retval = 0;
    struct wombat_instance *inst = 
        (struct wombat_instance *)malloc(sizeof(struct wombat_instance));

    inst->notif_thread = *instance;
    inst->next = NULL;

    if (whead == NULL) {
        whead = wtail = inst;
        retval = 1;
    } else {
        wtail->next = inst;
        wtail = inst;
    }

    return retval;
}


L4_ThreadId_t
wombat_threads_get_primary_impl(L4_ThreadId_t caller,
                                L4_ThreadId_t *tid,
                                idl4_server_environment *env)
{
    return L4_nilthread;
    return whead->notif_thread;
}
