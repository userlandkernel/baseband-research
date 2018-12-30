#if defined(CONFIG_SESSION)
/*
 * Copyright (c) 2006, National ICT Australia
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <iguana/debug.h>
#include <iguana/memsection.h>
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/session.h>
#include <iguana/cap.h>
#include <iguana/eas.h>
#include <iguana/object.h>
#include "test_libs_iguana.h"

#include <interfaces/iguana_client.h>

// extern size_t __malloc_instrumented_allocated;

START_TEST(SESS0100)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    struct session *sess;
    clist_ref_t clist = 0;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");

    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);

    clist = clist_create();
    fail_if(clist == 0, "reference not zero");

    sess = session_create_with_clist(base, clist, &l4_thrd);
    fail_if(sess == 0, "NULL session returned");

    session_delete(sess);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
    clist_delete(clist);
}
END_TEST

START_TEST(SESS0200)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    struct session *sess;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);
    sess = session_create(base, &l4_thrd);
    fail_if(sess == 0, "NULL session returned");
    session_delete(sess);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);

}
END_TEST

START_TEST(SESS0300)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    struct session *sess;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);
    sess = session_create_full_share(base, &l4_thrd);
    fail_if(sess == 0, "NULL session returned");
    session_delete(sess);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
}
END_TEST

#if 0
START_TEST(test_session_add_async)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0, callback, retcb;
    objref_t callback_base, return_base;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    struct session *sess;
    pd_ref_t mypd = 0;

    mypd = pd_create();
    fail_if(mypd == 0, "NULL pd returned");
    callback = pd_create_memsection(mypd, MEM_SIZE, &callback_base);
    pd_set_callback(mypd, callback);
    retcb = pd_create_memsection(mypd, MEM_SIZE, &return_base);
    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);
    sess = session_create_full_share(base, &l4_thrd);
    fail_if(sess == 0, "NULL session returned");
    session_add_async(sess, callback_base, return_base);
    session_delete(sess);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
    pd_delete(mypd);

}
END_TEST
#endif

START_TEST(SESS0500)
{
    static char *mystring = "be in touch!";
    uintptr_t ignore;
    struct session *sess;
    pd_ref_t subpd1 = 0;
    uintptr_t str1_addr;
    memsection_ref_t mem_ref1;
    thread_ref_t thrd1 = 0;
    L4_ThreadId_t l4_thrd1;

    subpd1 = pd_create();
    fail_if(subpd1 == 0, "NULL pd returned");
    mem_ref1 = pd_create_memsection(subpd1, MEM_SIZE, &str1_addr);
    thrd1 = pd_create_thread(subpd1, &l4_thrd1);
    fail_if(thrd1 == 0, "NULL thread returned");
    memsection_register_server(mem_ref1, thrd1);
    sess = session_create_full_share((objref_t)str1_addr, &l4_thrd1);
    fail_if(sess == 0, "NULL session returned");
    if (sess == NULL) {
        return;
    }
    session_provide_access(sess,
                           memsection_lookup((uintptr_t)mystring, &ignore),
                           MASTER_IID);
    session_delete(sess);
    memsection_delete(mem_ref1);
    thread_delete(l4_thrd1);
    pd_delete(subpd1);
}
END_TEST

#ifdef CONFIG_LOWMEMORY
START_TEST(SESS0400)
{
    struct session **sessions = NULL;
    L4_ThreadId_t ignore;
    int max_sessions;
    int i = 0, cursize = 100;
    memsection_ref_t local_control_mem_section;
    uintptr_t mem_section_ptr;

    local_control_mem_section = memsection_create(4096, &mem_section_ptr);
    memsection_register_server(local_control_mem_section, thread_myself());

    sessions = malloc(sizeof(struct session *) * cursize);

    do {
        DEBUG_PRINT(".");
        if (i >= cursize) {
            cursize += 100;
            sessions = realloc(sessions, sizeof(struct session *) * cursize);
            if (sessions == NULL) {
                fail("Unable to allocate memory");
                break;
            }
        }
        sessions[i] = session_create((objref_t)mem_section_ptr, &ignore);
    } while (sessions[i++] != 0);

    max_sessions = i - 1;
    DEBUG_PRINT("Created %d sessions\n", max_sessions);

    for (i = 0; i < max_sessions; i++) {
        session_delete(sessions[i]);
    }

    free(sessions);

    fail_unless(max_sessions > 0, "Unable to create and direct section");

    for (i = 0; i < max_sessions * 3; i++) {
        struct session *sess;

        DEBUG_PRINT(",");
        // DEBUG_PRINT("session_create: %d\n", __malloc_instrumented_allocated);
        sess = session_create((objref_t)mem_section_ptr, &ignore);
        // DEBUG_PRINT("done session create: %d\n",
        // __malloc_instrumented_allocated);
        // DEBUG_PRINT("Doing failif: %d\n", __malloc_instrumented_allocated);
        fail_if(sess == NULL, "Unable to create session");
        // DEBUG_PRINT("Done failif: %d\n", __malloc_instrumented_allocated);
        if (sess == NULL) {
            DEBUG_PRINT("Couldn't create session\n");
            break;
        }
        // DEBUG_PRINT("doing session delete: %d\n",
        // __malloc_instrumented_allocated);
        session_delete(sess);
        // DEBUG_PRINT("done session delete: %d\n",
        // __malloc_instrumented_allocated);
    }
    memsection_delete(local_control_mem_section);
    DEBUG_PRINT("finished\n");
}
END_TEST
#endif

TCase *
session_tests()
{
    TCase *tc;

    tc = tcase_create("Session Sections");
    tcase_add_test(tc, SESS0100);
    tcase_add_test(tc, SESS0200);
    tcase_add_test(tc, SESS0300);
#ifdef CONFIG_LOWMEMORY
    tcase_add_test(tc, SESS0400);
#endif
    // tcase_add_test(tc, test_session_add_async);

    tcase_add_test(tc, SESS0500);

    return tc;
}

#else

/* This is only here because ADS is silly. */
#if defined (__RVCT__) || defined(__RVCT_GNU__) || defined (__ADS__)
void __this_is_never_used_session_tests(void);
void __this_is_never_used_session_tests(void) {}
#endif

#endif
