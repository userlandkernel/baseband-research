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

#include <kernel/l4.h>
#include <kernel/platform_support.h>
#include <kernel/plat/timer.h>
#include <kernel/plat/kernel_counter.h>
#include <kernel/platform.h>

#include <kernel/debug.h>
#include <kernel/generic/lib.h>

#ifdef KERNEL_COUNTER_EXAMPLE

namespace Platform {

word_t do_platform_control(void* current, plat_control_t control, word_t param1,
                         word_t param2, word_t param3, continuation_t cont)
{
    tcb_t *tcb;
    word_t ret;

    switch (control.request()) {
        case CURRENT | KERNEL_COUNTER:
            ret = kernel_counter_current_value((tcb_t*)current);
            return ret;

        case REQUEST | KERNEL_COUNTER:
            tcb = PlatformSupport::lookup_tcb_locked(threadid(param2));
            if (EXPECT_FALSE(tcb == NULL))
            {
                get_current_tcb()->set_error_code(EINVALID_PARAM);
                return 0;
            }
            ret = kernel_counter_request(param1, (word_t *)tcb, param3);
            tcb->unlock_read();
            return ret;

        case INCREMENT | KERNEL_COUNTER:
            return kernel_counter_increment(param1, cont);

        default:
            get_current_tcb()->set_error_code(EINVALID_PARAM);
            return 0;
    }
}

}

/********************* PXA Kernel counter **************************************/


volatile word_t counter = 0;

struct cnt_request {
    word_t *tcb;
    word_t timeout;
    word_t notifybits;
};

typedef struct cnt_request request_t;

struct ll {
    struct ll *next;
    struct ll *prev;
    request_t data;
};

struct ll *req_list = NULL;

struct ll *ll_alloc(void);
void ll_destroy(void);
struct ll *ll_insert_before(struct ll *ll, request_t *data);
struct ll * ll_insert_after(struct ll *ll, request_t *data);
struct ll *ll_delete(struct ll *ll);


bool
handle_kernel_counter_interrupt(void)
{
    struct ll *tmp = req_list ? req_list->next : NULL;
    bool do_schedule = false;

    //printf("Handle counter interrupt: ");
    counter++;
    //printf("%lu\n", counter);
    /* Handle any notification required by registered threads */
    //if (tmp)
    //    printf("handle kernel counter interrupt: tmp=%lx, timeout=%lu\n", (word_t)tmp, tmp->data.timeout);
    while (tmp && tmp->data.timeout <= counter) {
        //printf("Notify thread %lx, notifybits=%lu\n", (word_t)tmp->data.tcb, tmp->data.notifybits);
        if (PlatformSupport::deliver_notify(tmp->data.tcb, tmp->data.notifybits, NULL) == true) {
            do_schedule = true;
        }
        tmp = ll_delete(tmp);
    }
    return do_schedule;
}

word_t
kernel_counter_current_value(tcb_t *tcb)
{
    tcb->set_mr(0, counter);

    return 1;
}

word_t
kernel_counter_request(word_t value, word_t *tcb, word_t notifybits)
{
    request_t new_req;
    struct ll *tmp;

    if (value <= counter) {
        PlatformSupport::deliver_notify(tcb, notifybits, NULL);
        return 1;
    }
    /* Register request for given thread */
    new_req.tcb = tcb;
    new_req.timeout = value;
    new_req.notifybits = notifybits;
    if (req_list == NULL) {
        req_list = ll_alloc();
        tmp = ll_insert_after(req_list, &new_req);
    } else {
        tmp = req_list;
        do {
            if (!tmp->next) {
                tmp = ll_insert_after(tmp, &new_req);
                //printf("Registered timeout=%lu for tcb=%lx, current counter=%lu\n", tmp->data.timeout, (word_t)tmp->data.tcb, counter);
                return 1;
            }
            tmp = tmp->next;
        } while (tmp && tmp->data.timeout < new_req.timeout);
        tmp = ll_insert_before(tmp, &new_req);
    }
    //printf("Registered timeout=%lu for tcb=%lx, current counter=%lu\n", tmp->data.timeout, (word_t)tmp->data.tcb, counter);

    return 1;
}

word_t
kernel_counter_increment(word_t value, continuation_t cont)
{
    if (handle_kernel_counter_interrupt()) {
        PlatformSupport::schedule(cont);
    }

    return 1;
}

/*--------------------------------------------------------------------------------------*/


/********************* Simple linked list ***********************************************/

#define SIZE 0x400

struct ll_head {
    struct ll *next_free;
    word_t size;
};

struct ll_head *head = NULL;

void
ll_init(void)
{
    struct ll *tmp;

    for (tmp = head->next_free; ((word_t)tmp + sizeof(struct ll)) <= ((word_t)head + head->size); tmp = tmp->next) {
        tmp->next = tmp + 1;
        tmp->prev = NULL;
    }
    tmp->next = NULL;
}

struct ll*
ll_alloc(void)
{
    void *new_block;
    struct ll *free_node;
    
    if (head && head->next_free) {
        free_node = head->next_free;
    } else {
        new_block = PlatformSupport::mem_alloc(SIZE, true);
        if (head == NULL) {
            head = (struct ll_head*)new_block;
            head->next_free = (struct ll*)((word_t)head + sizeof(struct ll_head));
            head->size = SIZE;
            free_node = head->next_free;
        } else {
            head->next_free = (struct ll*)new_block;
            free_node = (struct ll*)new_block;
            head->size += SIZE;
        }
        ll_init();
    }
    head->next_free = free_node->next;
    free_node->next = NULL;

    return free_node;
}

void
ll_destroy(void)
{
    PlatformSupport::mem_free((void *)head, head->size);
}

void
ll_free(struct ll *ll)
{
    ll->prev = NULL;
    if (head->next_free) {
        ll->next = head->next_free->next;
    } else {
        ll->next = NULL;
    }
    head->next_free = ll;
}

struct ll *
ll_insert_before(struct ll *ll, request_t *data)
{
    struct ll *new_node;

    new_node = ll_alloc();
    if (new_node != NULL) {
        new_node->next = ll;
        if (ll->prev) 
            ll->prev->next = new_node;
        new_node->prev = ll->prev;
        ll->prev = new_node;
        new_node->data = *data;
    }
    return new_node;
}

struct ll *
ll_insert_after(struct ll *ll, request_t *data)
{
    struct ll *new_node;

    new_node = ll_alloc();
    if (new_node != NULL) {
        new_node->prev = ll;
        if (ll->next)
            ll->next->prev = new_node;
        new_node->next = ll->next;
        ll->next = new_node;
        new_node->data = *data;
    }
    return new_node;
}

struct ll *
ll_delete(struct ll *ll)
{
    struct ll *next = ll->next;

    if (ll->next)
        ll->next->prev = ll->prev;
    if (ll->prev)
        ll->prev->next = ll->next;
    ll_free(ll);

    return next;
}

#endif /* defined KERNEL_COUNTER_EXAMPLE */
