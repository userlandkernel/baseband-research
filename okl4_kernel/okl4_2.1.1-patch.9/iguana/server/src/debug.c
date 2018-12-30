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
 * Author: Alex Webster, Ben Leslie 
 */
#if !defined(NDEBUG)
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>             /* For abort */
#include <inttypes.h>
#include "debug.h"
#include "memsection.h"
#include "pd.h"
#include "util.h"

static void dump_memsections(struct pd *base, int depth);

#ifdef CONFIG_TEST_IGUANA
extern int __malloc_check(void);
extern void __malloc_dump(void);
#endif

void
iguana_check_state(void)
{
    if (pd_check_state(&iguana_pd) != 0) {
        DEBUG_PRINT(ANSI_RED "Iguana check state failed! " ANSI_NORMAL "\n");
        debug_dump_info();
        abort();
    }
#ifdef CONFIG_TEST_IGUANA
    if (__malloc_check() != 0) {
        DEBUG_PRINT("malloc heap corrupt\n");
        __malloc_dump();
    }
#endif
}

void
debug_dump_info(void)
{

    /* Dump availabel threads */
    DEBUG_PRINT("Thread list:\n");
    // rfl_debug(thread_list, stdout);

    /* Dump memory sections */
    dump_memsections(&iguana_pd, 0);
}

#define print_spaces(n) { int i = (2 * (n)); while (i--) DEBUG_PRINT(" "); }

static void
dump_pd(struct pd *pd)
{
    DEBUG_PRINT("pd(%p)\n", pd);
}

static void
dump_memsection(struct memsection *ms)
{
    DEBUG_PRINT("memsection(%p)", ms);
    DEBUG_PRINT(" <%p:%p>", (void *)ms->base, (void *)ms->end);
    DEBUG_PRINT("\n");
}

static void
dump_thread(struct thread *thread, int spaces)
{
#if defined(CONFIG_SESSION)
    struct session_p_node *sd, *first_sd;
#endif
    int fail;

    fail = (thread_check_state(thread) != 0);

    if (fail) {
        DEBUG_PRINT(ANSI_RED);
    }

    DEBUG_PRINT("thread(%p)", thread);
    DEBUG_PRINT(" #%ld", L4_ThreadNo(thread->id));
    if (fail) {
        DEBUG_PRINT(ANSI_NORMAL);
    }
    DEBUG_PRINT("\n");
    print_spaces(spaces + 1);
#if defined(CONFIG_EAS)
    if (thread->eas == NULL) {
#endif
#if defined(CONFIG_SESSION)
        DEBUG_PRINT("Client sessions: ");
        first_sd = thread->server_sessions.first;
        for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
            DEBUG_PRINT("%p ", sd->data);
        }
        DEBUG_PRINT("\n");
#endif
        print_spaces(spaces + 1);
#if defined(CONFIG_SESSION)
        DEBUG_PRINT("Server sessions: ");
        first_sd = thread->client_sessions.first;
        for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
            DEBUG_PRINT("%p ", sd->data);
        }
        DEBUG_PRINT("\n");
#endif
#if defined(CONFIG_EAS)
    }
#endif
}

#if defined(CONFIG_EAS)
static void
dump_eas(struct eas *eas, int depth)
{
    struct thread_node *td, *first_td = eas->threads.first;

    DEBUG_PRINT("eas(%p)", eas);
    DEBUG_PRINT("\n");

    for (td = first_td; td->next != first_td; td = td->next) {
        print_spaces(depth + 1);
        dump_thread(&td->data, depth + 1);
    }
}
#endif

#if defined(CONFIG_SESSION)
static void
dump_session(struct session *session)
{
    int fail;

    fail = (session_check_state(session) != 0);

    if (fail) {
        DEBUG_PRINT(ANSI_RED);
    }

    DEBUG_PRINT("session(%p)", session);
    DEBUG_PRINT(" Client: #%ld Server: #%ld",
                L4_ThreadNo(session->client->id),
                L4_ThreadNo(session->server->id));
    if (fail) {
        DEBUG_PRINT(ANSI_NORMAL);
    }
    DEBUG_PRINT("\n");
}
#endif

static void
dump_memsections(struct pd *base_pd, int depth)
{
    struct memsection_node *ms, *first_ms = base_pd->memsections.first;
    struct thread_node *td, *first_td = base_pd->threads.first;
#if defined(CONFIG_SESSION)
    struct session_p_node *sd, *first_sd = base_pd->sessions.first;
#endif
    struct pd_node *pd, *first_pd = base_pd->pds.first;
    struct eas_node *ed, *first_ed = base_pd->eass.first;

    print_spaces(depth);
    dump_pd(base_pd);

    for (ms = first_ms; ms->next != first_ms; ms = ms->next) {
        print_spaces(depth + 1);
        dump_memsection(&ms->data);
    }

    for (td = first_td; td->next != first_td; td = td->next) {
        print_spaces(depth + 1);
        dump_thread(&td->data, depth + 1);
    }

#if defined(CONFIG_SESSION)
    for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
        print_spaces(depth + 1);
        dump_session(sd->data);
    }
#endif
    for (ed = first_ed; ed->next != first_ed; ed = ed->next) {
        print_spaces(depth + 1);
#if defined(CONFIG_EAS)
        dump_eas(&ed->data, depth + 1);
#endif
    }

    for (pd = first_pd; pd->next != first_pd; pd = pd->next) {
        dump_memsections(&pd->data, depth + 1);
    }
}
#endif
