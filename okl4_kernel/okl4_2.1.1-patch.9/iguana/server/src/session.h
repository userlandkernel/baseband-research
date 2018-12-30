#if defined(CONFIG_SESSION)
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
 * Author: Ben Leslie Created: Wed Jul 28 2004 
 */

#ifndef _IGUANA_SESSION_H_
#define _IGUANA_SESSION_H_

#include <stdint.h>
#include <ll/ll.h>

#define SESSION_MAGIC 0x5e551011

#define is_session(ptr) ((ptr != NULL) && (ptr->magic == SESSION_MAGIC))

struct session;

struct session {
    uintptr_t magic;
    struct pd *owner;
    struct thread *client;
    struct thread *server;
    void *call_buf;
    void *return_buf;
    struct clist *clist;
    struct session **owner_node;
    struct session **server_node;
    struct session **client_node;
};

struct session_p_list {
    struct session_p_node *first;
    struct session_p_node *last;
};

struct session_p_node {
    struct session_p_node *next;
    struct session_p_node *prev;
    struct session *data;
};

static inline struct session **
session_p_list_create_front(struct session_p_list *list)
{
    return (struct session **)dl_list_create_front((struct double_list *)list,
                                                   sizeof(struct session *));
}

static inline struct session **
session_p_list_create_back(struct session_p_list *list)
{
    return (struct session **)dl_list_create_back((struct double_list *)list,
                                                  sizeof(struct session *));
}

static inline void
session_p_list_delete(struct session **data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
session_p_list_init(struct session_p_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
session_p_list_clear(struct session_p_list *list)
{
    dl_list_clear((struct double_list *)list);
}

void session_add_buffer(struct session *session,
                        void *call_buf, void *return_buf);

void session_delete(struct session *session);
int session_check_state(const struct session *self);
int session_check_state_server(const struct session *self,
                               const struct thread *server);
int session_check_state_client(const struct session *self,
                               const struct thread *client);

#endif /* _IGUANA_SESSION_H_ */
#endif
