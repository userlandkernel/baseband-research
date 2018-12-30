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
 * Author: Ben Leslie Created: Mon Jul 5 2004
 * 
 * Describes the iguana thread 
 */

#ifndef _IGUANA_THREAD_H_
#define _IGUANA_THREAD_H_

#include <l4/thread.h>
#include <ll/ll.h>
#include <stdint.h>
#include <range_fl/range_fl.h>

#include "exception.h"
#if defined(CONFIG_SESSION)
#include "session.h"
#endif
extern const uintptr_t thread_magic;
#define THREAD_MAGIC thread_magic

#define is_thread(ptr) ((ptr != NULL) && (ptr->magic == THREAD_MAGIC))

struct thread {
    uintptr_t magic;
    struct pd *owner;           /* Our owner */
    void *utcb;
#if defined(CONFIG_EAS)
    struct eas *eas;            /* If not NULL, indicates an external thread,
                                 * in a different space */
#endif
    L4_ThreadId_t id;
    L4_ThreadId_t handle;
#if defined(CONFIG_SESSION)
    /*
     * Note that these are a list of session list *, since sessions will be in 
     * two linked lists 
     */
    struct session_p_list client_sessions;
    struct session_p_list server_sessions;
#endif
    struct exception exception[2];
};

struct thread_list {
    struct thread_node *first;
    struct thread_node *last;
};

struct thread_node {
    struct thread_node *next;
    struct thread_node *prev;
    struct thread data;
};

static inline struct thread *
thread_list_create_front(struct thread_list *list)
{
    return (struct thread *)dl_list_create_front((struct double_list *)list,
                                                 sizeof(struct thread));
}

static inline struct thread *
thread_list_create_back(struct thread_list *list)
{
    return (struct thread *)dl_list_create_back((struct double_list *)list,
                                                sizeof(struct thread));
}

static inline void
thread_list_delete(struct thread *data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
thread_list_init(struct thread_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
thread_list_clear(struct thread_list *list)
{
    dl_list_clear((struct double_list *)list);
}

void thread_init(void);

/* Given an L4 thread ID return its associated thread */
struct thread *thread_lookup(L4_ThreadId_t thrd);
/* Called to initialise the thread structure */
int thread_setup(struct thread *thread, int priority);
int thread_start(const struct thread *thread, uintptr_t ip, uintptr_t sp);
int thread_stop(const struct thread *thread, uintptr_t *ip, uintptr_t *sp);
void thread_delete(struct thread *thread);
int thread_new(struct thread *thread, L4_SpaceId_t space, L4_ThreadId_t scheduler, L4_ThreadId_t pager, L4_ThreadId_t exception);
void thread_free(L4_ThreadId_t thread);

int thread_alloc(struct thread *thread);
void thread_print(struct thread *thread);

void thread_raise_exception(const struct thread *thread, uintptr_t exc,
                            uintptr_t x);

int thread_check_state(struct thread *thread);

extern rfl_t thread_list;

#endif /* _IGUANA_THREAD_H_ */
