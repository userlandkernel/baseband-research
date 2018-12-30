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

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#include <l4e/misc.h>

#include <iguana/session.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/pd.h>
#include <iguana/cap.h>
#include <interfaces/iguana_client.h>

#include <stdio.h>

#include <inttypes.h>

int
_session_create(objref_t object, clist_ref_t clist,
                L4_ThreadId_t *server_tid, struct session *session)
{
    cap_t cap;
    thread_ref_t server;
    memsection_ref_t memsection;

    if (object == 0 || object == INVALID_ADDR) {
        return -1;
    }
    /* Find memsection */
    memsection = memsection_lookup(object, &server);
    if (memsection == 0 || memsection == INVALID_ADDR) {
        return -1;
    }

    /* TODO: Search for existing session with that server */

    cap = iguana_pd_create_session(IGUANA_PAGER,
                                   pd_myself(), thread_myself(), server,
                                   clist, default_clist, NULL);
    if (cap.ref.session == 0) {
        return -1;
    }
    *server_tid = thread_l4tid(server);
    session->ref = cap.ref.session;
    session->clist = clist;
    session->own_clist = 0;
    return 0;
}

struct session *
session_create_with_clist(objref_t object, clist_ref_t clist,
                          L4_ThreadId_t *server_tid)
{
    struct session *session;
    int r;

    session = malloc(sizeof(struct session));

    if (session == NULL)
        return NULL;

    r = _session_create(object, clist, server_tid, session);

    if (r != 0) {
        free(session);
        session = NULL;
    }

    return session;
}

struct session *
session_create(objref_t object, L4_ThreadId_t *server_tid)
{
    clist_ref_t clist;
    struct session *session;

    clist = clist_create();
    if (clist == 0) {
        return 0;
    }
    session = session_create_with_clist(object, clist, server_tid);

    if (session == NULL) {
        clist_delete(clist);
        return NULL;
    }

    session->own_clist = 1;

    return session;
}

struct session *
session_create_full_share(objref_t object, L4_ThreadId_t *server_tid)
{
    return session_create_with_clist(object, default_clist, server_tid);
}

void
session_add_async(struct session *session, objref_t call_buf,
                  objref_t return_buf)
{
    /* Add async buffers to a session */
    iguana_session_add_buffer(IGUANA_PAGER, session->ref, call_buf, return_buf,
                              NULL);
}

int
session_provide_access(struct session *session, objref_t object, int interface)
{
    /* Search for the capability in our clists -- either the master or exact */
    cap_t cap = INVALID_CAP;

    assert(session != NULL);

    cap = iguana_get_cap(object, interface);
    if (IS_VALID_CAP(cap) == 0) {
        return 0;
    }
    return clist_insert(session->clist, cap) == 0;
}

int
session_revoke_access(struct session * session, objref_t object, int interface)
{
    cap_t cap;
    int r;

    r = clist_lookup(session->clist, object, interface, &cap);
    if (r != 0) {
        return 0;
    }
    iguana_session_revoke(IGUANA_PAGER, session->ref, &cap, NULL);
    return clist_remove(session->clist, cap) == 0;
}

void
session_delete(struct session *session)
{
    /* Delete the clist */
    if (session->own_clist) {
        clist_delete(session->clist);
    }

    /* Delete the session */
    iguana_session_delete(IGUANA_PAGER, session->ref, NULL);
    free(session);
}

#else

/* This is only here because ADS is silly. */
#if defined (__RVCT__) || defined(__RVCT_GNU__) || defined (__ADS__)
void __this_is_never_used_sessions(void);
void __this_is_never_used_sessions(void) {}
#endif

#endif  /* CONFIG_SESSION */
