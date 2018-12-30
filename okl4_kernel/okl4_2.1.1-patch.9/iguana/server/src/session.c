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
 * Authors: Ben Leslie Created: Thu Aug 12 2004 
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <l4/ipc.h>
#include "clist.h"
#include "util.h"
#include "memsection.h"
#include "pd.h"
#include "session.h"

void
session_add_buffer(struct session *session, void *call_buf, void *return_buf)
{
    assert(session->call_buf == NULL);
    assert(session->return_buf == NULL);
    session->call_buf = call_buf;
    session->return_buf = return_buf;

    /* Now tell the server about this development */
    {
        /*
         * Zero timeout ping -- FIXME: This should be handled by idl upcall
         * code, but do this later 
         */
        struct pd *other = session->server->owner;
        L4_Msg_t msg;
        uintptr_t *size;
        uintptr_t *addr;

        /* Now notify client and server prot domains */
        size = cb_alloc(other->cba, sizeof(uintptr_t));
        if (size == NULL) {
            ERROR_PRINT("Couldn't do upcall");
            return;
        }
        *size = 3 * sizeof(uintptr_t);
        addr = cb_alloc(other->cba, *size);
        addr[0] = 0x38;
        addr[1] = (uintptr_t)call_buf;
        addr[2] = (uintptr_t)return_buf;
        cb_sync_alloc(other->cba);

        L4_MsgClear(&msg);
        L4_MsgLoad(&msg);
        (void)L4_Reply(other->threads.first->data.id);
    }

}

void
session_delete(struct session *session)
{
    /* Tell the other side */
    ASSERT_PTR(session);
    ASSERT_PTR(session->server);
    ASSERT_PTR(session->server->owner);
    assert(is_thread(session->server));

    pd_release_clist(session->server->owner, session->clist);

    session_p_list_delete(session->owner_node);
    session_p_list_delete(session->server_node);
    session_p_list_delete(session->client_node);
    /* Then nuke everything */
    session->magic = 0;
    free(session);
}

#if !defined(NDEBUG)
static int
session_check_basic_state(const struct session *self)
{
    if (!is_session(self)) {
        return 1;
    }

    return 0;
}

int
session_check_state(const struct session *self)
{
    int r = 0;

    if (session_check_basic_state(self) != 0) {
        return 1;
    }

    r += thread_check_state(self->client);
    r += thread_check_state(self->server);

    return r;
}

int
session_check_state_server(const struct session *self,
                           const struct thread *server)
{
    int r = 0;

    if (session_check_basic_state(self) != 0) {
        ERROR_PRINT("check server client: %p\n", self);
        return 1;
    }

    if (self->server != server) {
        r++;
    }

    return r;
}

int
session_check_state_client(const struct session *self,
                           const struct thread *client)
{
    int r = 0;

    if (session_check_basic_state(self) != 0) {
        ERROR_PRINT("check stat client: %p\n", self);
        return 1;
    }

    if (self->client != client) {
        r++;
    }

    return r;
}
#endif
#else
/* This is only here because ADS is silly. */
#if defined (__RVCT__) || defined(__RVCT_GNU__) || defined (__ADS__)
void __this_is_never_used_sessions(void);
void __this_is_never_used_sessions(void) {
}
#endif
#endif
