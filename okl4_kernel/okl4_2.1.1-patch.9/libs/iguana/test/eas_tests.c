#if defined(CONFIG_EAS)
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
#include <l4/config.h>
#include <l4/kdebug.h>
#include <l4/config.h>
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

START_TEST(EAS0100)
{
    eas_ref_t eas;
    L4_Fpage_t utcb_area
;
    utcb_area = L4_FpageLog2(0x0, 16);
#if defined(ARM_PID_RELOC)
    eas = eas_create(utcb_area, 0, NULL);
#else
    eas = eas_create(utcb_area, NULL);
#endif

    fail_if(eas == 0, "NULL eas returned");
}
END_TEST

START_TEST(EAS0200)
{
    eas_ref_t eas;
    L4_Fpage_t utcb_area;

    utcb_area = L4_FpageLog2(0x0, 16);
#if defined(ARM_PID_RELOC)
    eas = eas_create(utcb_area, 0, NULL);
#else
    eas = eas_create(utcb_area, NULL);
#endif
    fail_if(eas == 0, "NULL eas returned");
    eas_delete(eas);
}
END_TEST

START_TEST(EAS0300)
{
    eas_ref_t eas;
    L4_ThreadId_t tid, ignored;
    L4_Fpage_t utcb_area;
    uintptr_t utcb;
    L4_ThreadId_t me;

    utcb_area = L4_FpageLog2(0x0, 16);  /* 16 bits worth of threads! Lots ! */
    me = thread_l4tid(thread_myself());


    /* If a utcb is needed then calculate the address. */
#if defined(NO_UTCB_RELOCATE)
    utcb = -1;
#else
    utcb = 0;
#endif

#if defined(ARM_PID_RELOC)
    eas = eas_create(utcb_area, 0, NULL);
#else
    eas = eas_create(utcb_area, NULL);
#endif
    fail_if(eas == 0, "NULL eas returned");

    tid = eas_create_thread(eas, me, me, (void *) utcb, &ignored);
    fail_if(tid.raw == 0, "NULL eas thread created");
    eas_delete(eas);
}
END_TEST

START_TEST(EAS0400)
{
    eas_ref_t eas;
    L4_Fpage_t utcb_area;
    uintptr_t utcb;
    thread_ref_t *threads = NULL;
    L4_ThreadId_t ignore;
    L4_ThreadId_t myself = thread_l4tid(thread_myself());
    int max_threads;
    int i = 0, cursize = 100;

    utcb_area = L4_FpageLog2(0x0, 16);  /* 16 bits worth of threads! Lots ! */
#if defined(ARM_PID_RELOC)
    eas = eas_create(utcb_area, 0, NULL);
#else
    eas = eas_create(utcb_area, NULL);
#endif

    /* If a utcb is needed then calculate the address. */
#if defined(NO_UTCB_RELOCATE)
    utcb = -1;
#else
    utcb = 0;
#endif

    printf("EAS thread create many\n");

    threads = malloc(sizeof(struct thread *) * cursize);

    do {
        printf(".");
        if (i >= cursize) {
            cursize += 100;
            threads = realloc(threads, sizeof(struct thread *) * cursize);
            if (threads == NULL) {
                fail("Unable to allocate memory");
                break;
            }
        }
        threads[i] =
            iguana_eas_create_thread(IGUANA_PAGER, eas, &myself, &myself,
                                     (uintptr_t) utcb, default_clist, &ignore, &ignore, NULL).ref.thread;
    } while (threads[i++] != 0);

    max_threads = i - 1;

    printf("\nCreated %d EAS threads\n", max_threads);

    for (i = 0; i < max_threads; i++) {
        iguana_thread_delete(IGUANA_PAGER, threads[i], NULL);
    }

    free(threads);

    fail_unless(max_threads > 0, "Unable to create any threads");

    for (i = 0; i < max_threads * 3; i++) {
        thread_ref_t thrd;

        thrd = thread_create(&ignore);
        fail_if(thrd == 0, "Unable to create thread");
        if (thrd == 0) {
            break;
        }
        thread_delete(ignore);
    }

    eas_delete(eas);
}
END_TEST

TCase *
eas_tests()
{
    TCase *tc;

    tc = tcase_create("EAS Sections");
    tcase_add_test(tc, EAS0100);
    tcase_add_test(tc, EAS0200);
    tcase_add_test(tc, EAS0300);
    tcase_add_test(tc, EAS0400);

    return tc;
}
#endif
