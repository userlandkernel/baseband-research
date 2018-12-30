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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <l4/ipc.h>
#include <l4/config.h>
#include <l4/kdebug.h>
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4/types.h>

#include <iguana/cap.h>
#if defined(CONFIG_EAS)
#include <iguana/eas.h>
#endif
#include <iguana/debug.h>
#include <iguana/memsection.h>
#include <iguana/object.h>
#include <iguana/pd.h>
#include <iguana/session.h>
#include <iguana/types.h>
#include <iguana/thread.h>
#include <iguana/env.h>

#include "test_libs_iguana.h"

#include <interfaces/iguana_client.h>

START_TEST(THRD0100)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = thread_create(&l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(THRD0200)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd, l4_tid;

    iguana_thrd = thread_create(&l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    l4_tid = thread_l4tid(iguana_thrd);
    fail_if(l4_tid.raw != l4_thrd.raw, "not right l4 thread ID");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(THRD0300)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = thread_create(&l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    fail_if(thread_id(l4_thrd) != iguana_thrd, "incorrect thread ref");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(THRD0400)
{
    thread_ref_t iguana_thrd = 0, my_thrd;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = thread_create(&l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    my_thrd = thread_myself();
    fail_if(my_thrd == iguana_thrd, "not right iguana thread ID");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(THRD0500)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = thread_create_priority(100, &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(THRD0600)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = thread_create(&l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    thread_delete(l4_thrd);
}
END_TEST

START_TEST(THRD0700)
{
    /* no way to check it becuase idl4_set_no_response invoked in this case */
}

END_TEST

START_TEST(THRD1100)
{
    /* how to get the IP and SP?  */

}

END_TEST

START_TEST(THRD0800)
{
    /* need the description for this function */

}
END_TEST

static void
die(void *arg)
{
}

START_TEST(THRD0900)
{
    thread_ref_t thrd;
    int i;

    thrd = thread_create_simple(die, NULL, 100);

    /* give the thread time to die */
    for (i = 0; i < 100; i++)
        L4_Yield();

    thread_delete(thread_l4tid(thrd));
}
END_TEST

START_TEST(THRD1000)
{
    thread_ref_t *threads = NULL;
    L4_ThreadId_t ignore;
    int max_threads;
    int i = 0, cursize = 100;

    printf("Thread create many\n");

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
        threads[i] = thread_create(&ignore);
    } while (threads[i++] != 0);

    max_threads = i - 1;

    printf("\nCreated %d threads\n", max_threads);

    for (i = 0; i < max_threads; i++) {
        iguana_thread_delete(IGUANA_PAGER, threads[i], NULL);
    }

    free(threads);

    fail_unless(max_threads > 0, "Unable to any threads");

    for (i = 0; i < max_threads * 3; i++) {
        thread_ref_t thrd;

        thrd = thread_create(&ignore);
        fail_if(thrd == 0, "Unable to create thread");
        if (thrd == 0) {
            break;
        }
        thread_delete(ignore);
    }
}
END_TEST

#define N_REPS 3
#define N_THREADS 20
#define STACK_SIZE 4096
#define FINISHED (N_THREADS+7)
// THREAD_DELETE_FROM_MAIN: enable to avoid race in deleting threads
#define THREAD_DELETE_FROM_MAIN
extern void __thread_stub(void);
static void worker(void *arg_p);

static void
worker(void *arg_p)
{
    int i;
    uint8_t data[2048];
    volatile int *arg = (int *)arg_p;

    // printf("thread %d running\n", *arg);
    for (i = 0; i < sizeof(data); i++) {
        data[i] = i % 256;
    }
    for (i = 0; i < sizeof(data); i++) {
        if (data[i] != (i % 256)) {
            assert(!"data mismatch\n");
        }
    }
    // printf("thread %d finished\n", *arg);
    *arg = FINISHED;

#ifdef THREAD_DELETE_FROM_MAIN
    L4_WaitForever();
#endif
}

START_TEST(THRD1200)
{
    int i, r;
    void **stack_top;
    thread_ref_t thr_ref;
    thread_ref_t thread[N_THREADS];
    volatile int args[N_THREADS];
    L4_ThreadId_t ignore;
    memsection_ref_t stack[N_THREADS];

    for (r = 0; r < N_REPS; r++) {
        /* create a bunch of threads */
        for (i = 0; i < N_THREADS; i++) {
            args[i] = i;
            stack[i] = memsection_create(STACK_SIZE, (void *)&stack_top);
            if (stack[i] == 0) {
                assert(!"error in creating stack\n");
            }
            stack_top = (void **)((uintptr_t)stack_top + STACK_SIZE);
            stack_top--;
            *stack_top = (void *)worker;
            stack_top--;
            *stack_top = (void *)&args[i];
            thread[i] = thr_ref = thread_create_priority(100, &ignore);
            thread_start(thr_ref, (uintptr_t)__thread_stub,
                         (uintptr_t)stack_top);
        }

        /* wait for all threads to exit */
        i = 0;
        while (i < N_THREADS) {
            if ((volatile int)args[i] == FINISHED) {
                i++;
            }
        }

#ifdef THREAD_DELETE_FROM_MAIN
        for (i = 0; i < N_THREADS; i++) {
            thread_delete(thread_l4tid(thread[i]));
        }
#endif

        /* delete thread user stacks */
        for (i = 0; i < N_THREADS; i++) {
            memsection_delete(stack[i]);
        }
        // printf("iteration %d passed\n", r);
    }

    // printf("test passed\n");
}
END_TEST

#define MAX_THREADS 400
#define MAX_TESTS 50
#define TS2_MAJOR_ITERATIONS 20
#define TS2_MINOR_ITERATIONS 5

L4_ThreadId_t tids[MAX_THREADS];
static volatile L4_ThreadId_t master_tid;
static thread_ref_t rthread[MAX_THREADS];
static volatile uint64_t counter[MAX_THREADS];
static volatile int thread_stop;

static void
thread_simple_spinner(void *arg)
{
    uintptr_t thrd_num = (uintptr_t)arg;

    while (!thread_stop) {
        if (counter[thrd_num]++ % 50000 == 0) {
        }
    }
    L4_Call(master_tid);
}

#if defined(CONFIG_EAS)
START_TEST(THRD1300)
{
    eas_ref_t eas;
    L4_ThreadId_t tid, ignored;
    L4_Fpage_t utcb_area;
    uintptr_t utcb;
    int i, test;
    L4_ThreadId_t me;
    
    utcb_area = L4_FpageLog2(0x0, 16);  /* 16 bits worth of threads! Lots ! */

    /* If a utcb is needed then calculate the address. */
#if defined(NO_UTCB_RELOCATE)
    utcb = -1;
#else
    utcb = 0;
#endif

    /* Create external adress space */
#if defined(ARM_PID_RELOC)
    eas = eas_create(utcb_area, 0, NULL);
#else
    eas = eas_create(utcb_area, NULL);
#endif
    me = thread_l4tid(thread_myself());

    /* Simple test */
    tid = eas_create_thread(eas, me, me, (void *) utcb, &ignored);

    thread_delete(tid);
    tid = eas_create_thread(eas, me, me, (void *) utcb, &ignored);

    printf("SIMPLE TEST COMPLETE\n");

    /* Complex test */
    for (test = 0; test < MAX_TESTS; test++) {
        /* Create lots of threads */
        for (i = 0; i < MAX_THREADS; i++) {
            tids[i] = eas_create_thread(eas, me, me, (void *) utcb, &ignored);

            fail_unless(tids[i].raw != L4_nilthread.raw,
                        "Failed to create thread");
            return;
        }

        /* Delete lots of threads */
        for (i = 0; i < MAX_THREADS; i++) {
            thread_delete(tids[i]);
        }
        printf("Test %d complete\n", test);
    }
    printf("COMPLEX TEST COMPLETE\n");

    eas_delete(eas);
}
END_TEST
#endif

START_TEST(THRD1400)
{
    int i, j;

    printf("COMPLEX TEST 2\n");
    master_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    for (j = 0; j < TS2_MAJOR_ITERATIONS; j++) {
        thread_stop = 0;
        for (i = 0; i < TS2_MINOR_ITERATIONS; i++) {
            rthread[i] = thread_create_simple(thread_simple_spinner,
                                              (void *)(uintptr_t)i, 100);
            fail_unless(rthread[i] != 0, "Failed to create thread");
            fail_unless((thread_l4tid(rthread[i])).raw != L4_nilthread.raw,
                        "Failed to create thread");
        }
        thread_stop = 1;
        for (i = 0; i < TS2_MINOR_ITERATIONS; i++) {
            L4_Receive(thread_l4tid(rthread[i]));
            thread_delete(thread_l4tid(rthread[i]));
            fail_unless((thread_l4tid(rthread[i])).raw == L4_nilthread.raw,
                        NULL);
        }

        printf("Test %d complete\n", j);
    }

    printf("COMPLEX TEST 2 COMPLETE\n");
}
END_TEST

TCase *
thread_tests()
{
    TCase *tc;

    tc = tcase_create("Thread tests");

    tcase_add_test(tc, THRD0100);
    tcase_add_test(tc, THRD0200);
    tcase_add_test(tc, THRD0300);
    tcase_add_test(tc, THRD0400);
    tcase_add_test(tc, THRD0500);
    tcase_add_test(tc, THRD0600);
    tcase_add_test(tc, THRD0700);
    tcase_add_test(tc, THRD0800);
    tcase_add_test(tc, THRD0900);
    tcase_add_test(tc, THRD1000);
    tcase_add_test(tc, THRD1100);
    tcase_add_test(tc, THRD1200);
#if defined(CONFIG_EAS)
    tcase_add_test(tc, THRD1300);
#endif
    tcase_add_test(tc, THRD1400);

    return tc;
}
