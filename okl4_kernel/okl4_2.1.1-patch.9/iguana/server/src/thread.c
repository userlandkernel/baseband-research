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
 * Authors: Ben Leslie 
 */

#include "thread.h"
#include "pd.h"

#include <l4/schedule.h>
#include <l4/kdebug.h>

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <l4/config.h>

#include <hash/hash.h>

#include "util.h"

#ifdef __ARMv__
#if (__ARMv__ == 5) || (__ARMv__ == 4)
#define NO_UTCB_RELOCATE
#endif
#endif /* __ARMv__ */

#define THREAD_PD_HASHSIZE 1024

rfl_t thread_list;
static struct hashtable *l4tid_to_thread;
static L4_Word_t min_threadno, max_threadno;

/*
 * This initialise data structures for managing threads. 
 *
 * Called from: main (1)
 */
void
thread_init(void)
{
    /* Work out the thread ids that are available to us */
    int r;

    min_threadno = L4_ThreadNo(L4_rootserver) + 2;        /* +1 is the callback
                                                         * thread */
    max_threadno = 1UL << L4_GetThreadBits();

    thread_list = rfl_new();
    assert(thread_list != NULL);

#ifdef CONFIG_TEST_FEW_THREADS
    /*
     * This tests iguana behaviour when the pool of available thread ids is
     * exhausted 
     */
    max_threadno = from + 30;
#endif
    r = rfl_insert_range(thread_list, min_threadno, max_threadno);
    assert(r == 0);

    l4tid_to_thread = hash_init(THREAD_PD_HASHSIZE);
}

void
thread_free(L4_ThreadId_t thread)
{
    int r;
    struct thread * dead;

    dead = hash_lookup(l4tid_to_thread, thread.raw);

    if (dead != NULL)
    {
        /* Removed both mappings */
        hash_remove(l4tid_to_thread, dead->id.raw);
        hash_remove(l4tid_to_thread, dead->handle.raw);

        /* Add thread back to free pool */
        r = rfl_free(thread_list, L4_ThreadNo(dead->id));
        assert(r == RFL_SUCCESS);
    }
}

int
thread_alloc(struct thread *thread)
{
    unsigned int thread_no;
    int r;

    thread_no = rfl_alloc(thread_list);

    if (thread_no == -1U) {
        /* Run out of threads */
        return 1;
    }

    assert(thread_no > 0);

    thread->id = L4_GlobalId((unsigned long)thread_no, 1);

    assert(hash_lookup(l4tid_to_thread, thread->id.raw) == NULL);
    r = hash_insert(l4tid_to_thread, thread->id.raw, thread);

    if (r == -1) {
        r = rfl_free(thread_list, (unsigned long)thread_no);
        /* We just removed it before, so it should be possible to reinsert */
        assert(r == RFL_SUCCESS);
        return -1;
    }

    return 0;
}

int
thread_new
(
    struct thread   * thread,
    L4_SpaceId_t    space,
    L4_ThreadId_t   scheduler,
    L4_ThreadId_t   pager,
    L4_ThreadId_t   exception
)
{
    int     rv;

    rv =
        L4_ThreadControl
        (
            thread->id, space, scheduler, pager, exception, 0, (void *)thread->utcb
        );

    if (rv == 1)
    {
        L4_Word_t handle;
        L4_StoreMR(0, &handle);

        thread->handle.raw = handle;

        assert(hash_lookup(l4tid_to_thread, handle) == NULL);

        /* Guaranteed to work because hash_lookup was just performed */
        hash_insert(l4tid_to_thread, handle, thread);
    } else {
        /* Initialise the handle to an invalid valid. */
        thread->handle = L4_nilthread;
    }
    
    return rv;
}

int
thread_setup(struct thread *self, int _priority)
{
    int ret = 1;
    int r;
    unsigned long r_l;
    struct pd *pd;
    unsigned long priority = 100;

    assert(self != NULL);

    pd = self->owner;

    /* Note: These don't allocate any resources */
#if defined(CONFIG_SESSION)
    session_p_list_init(&self->client_sessions);
    session_p_list_init(&self->server_sessions);
#endif
    if (_priority != -1) {
        priority = (unsigned long)_priority;
    }

    self->magic = THREAD_MAGIC;
#if defined(CONFIG_EAS)
    self->eas = NULL;
#endif
    r = thread_alloc(self); /* Allocate a thread id, ALLOC #1 */
    if (r != 0) {
        /* Can't allocate a new thread ID */
        return 1;
    }

    /* Activate new thread */
    {
#ifdef NO_UTCB_RELOCATE
        self->utcb = (void *)-1UL;
#endif
        r_l = thread_new(self, pd_l4_space(pd), self->id, L4_rootserver, L4_rootserver);

        if (r_l != 1) {
            if (L4_ErrorCode() == L4_ErrNoMem || 
                L4_ErrorCode() == L4_ErrUtcbArea) {
                /*
                 * L4 has run out of memory... this is probably very bad, but
                 * we want to keep going for as long as we can 
                 */
                goto thread_error_state;
            } else {
                ERROR_PRINT_L4;
                assert(!"This shouldn't happen");
            }
        } else {
            ret = 0;
        }
        /* Set Priority */
        r = L4_Set_Priority(self->id, priority);
        assert(r != 0);
    }

    return ret;

  thread_error_state:
    /* Here we clean up anything we have allocated */
    thread_free(self->id);
    return 1;
}

void
thread_delete(struct thread *thread)
{
    struct pd *pd;
#if defined(CONFIG_SESSION)
    struct session_p_node *sd;
#endif
    pd = thread->owner;
    (void)L4_ThreadControl(thread->id,
                           L4_nilspace,
                           L4_nilthread, L4_nilthread, L4_nilthread, 0, NULL);
    thread_free(thread->id);

    if (
#if defined(CONFIG_EAS)
        thread->eas == NULL &&
#endif
        thread->utcb != (void *)-1UL) {
        /* free local thread no. */
        bfl_free(pd->local_threadno,
                 ((uintptr_t)thread->utcb -
                  L4_Address(pd->space.utcb_area)) >> L4_GetUtcbBits());
    }
#if defined(CONFIG_SESSION)
    /*
     * Now I need to go and delete any session that we are currently involved
     * with. Note: EAS threads don't have sessions. 
     */
#if defined(CONFIG_EAS)
    if (thread->eas == NULL) {
#endif
        for (sd = thread->client_sessions.first;
             sd->next != thread->client_sessions.first;
             sd = thread->client_sessions.first) {
            assert(is_session(sd->data));
            session_delete(sd->data);
        }

        for (sd = thread->server_sessions.first;
             sd->next != thread->server_sessions.first;
             sd = thread->server_sessions.first) {
            session_delete(sd->data);
        }
#if defined(CONFIG_EAS)
    }
#endif
#endif
    thread->id = L4_nilthread;
    thread->handle = L4_nilthread;
    thread_list_delete(thread);
}

struct thread *
thread_lookup(L4_ThreadId_t thread)
{
    return hash_lookup(l4tid_to_thread, thread.raw);
}

int
thread_start(const struct thread *self, uintptr_t ip, uintptr_t sp)
{
    L4_Start_SpIp(self->id, sp, ip);
    return 0;
}

int
thread_stop(const struct thread *self, uintptr_t *ip, uintptr_t *sp)
{
    L4_Word_t flags;

    /* YUCK: The casts are needed to stop the ADS compiler from
     * complaining.
     */
    L4_Stop_SpIpFlags(self->id, (L4_Word_t*) sp, (L4_Word_t*) ip, &flags);
    return 0;
}

#if defined(IG_DEBUG_PRINT)
void
thread_print(struct thread *self)
{
    DEBUG_PRINT("Thread: %p (Valid: %d -- %" PRIxPTR ")\n", self,
                (self->magic == THREAD_MAGIC), self->magic);
    DEBUG_PRINT(" Owner   : %p\n", self->owner);
    DEBUG_PRINT(" ThreadNo: %ld\n", L4_ThreadNo(self->id));
#if defined(CONFIG_EAS)
    DEBUG_PRINT(" External: %p\n", self->eas);
#endif
    DEBUG_PRINT(" Utcb    : %p\n", self->utcb);
}
#endif

void
thread_raise_exception(const struct thread *thread, uintptr_t exc, uintptr_t x)
{
    L4_Word_t old_sp, old_ip, old_flags;
    L4_Word_t new_sp, new_ip;
    uintptr_t *stack;

    (void)L4_Stop_SpIpFlags(thread->id, &old_sp, &old_ip, &old_flags);

    stack = (uintptr_t *)thread->exception[exc].sp;
    if (!stack)
        stack = (uintptr_t *)old_sp;
    stack--;
    *stack = old_ip;
    stack--;
    *stack = old_sp;
    stack--;
    *stack = old_flags;
    stack--;
    *stack = x;

    new_sp = (uintptr_t)stack;
    new_ip = thread->exception[exc].ip;

    assert(new_ip && new_sp);
    L4_Start_SpIp(thread->id, new_sp, new_ip);
}

#if !defined(NDEBUG)
int
thread_check_state(struct thread *thread)
{
    int r = 0;
#if defined(CONFIG_SESSION)
    struct session_p_node *sd, *first_sd;
#endif
    if (!is_thread(thread)) {
        return 1;
    }

    if (L4_ThreadNo(thread->id) > max_threadno) {
        return 1;
    }

    if (L4_ThreadNo(thread->id) < min_threadno) {
        return 1;
    }

    if (thread_lookup(thread->id) != thread) {
        return 1;
    }
#if defined(CONFIG_SESSION)
#if defined(CONFIG_EAS)
    if (thread->eas == NULL)
#endif
    {
        first_sd = thread->client_sessions.first;
        for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
            r += session_check_state_server(sd->data, thread);
        }
        if (r > 0)
            ERROR_PRINT("thread: %p client failed: %d\n", thread, r);

        first_sd = thread->server_sessions.first;
        for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
            r += session_check_state_client(sd->data, thread);
        }
        if (r > 0)
            ERROR_PRINT("thread: %p failed: %d\n", thread, r);
    }
#endif
    return r;
}
#endif
