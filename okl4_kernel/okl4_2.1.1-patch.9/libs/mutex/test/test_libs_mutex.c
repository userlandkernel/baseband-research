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

#include "test_libs_mutex.h"
#include <mutex/mutex.h>
#include <l4/thread.h>
#include <check/check.h>

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/env.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <mutex/mutex.h>

#include "utility.h"

#include <interfaces/iguana_client.h>

#include "../src/util.h"

#define NUM_THREADS 10
#define COUNT 5 /* Recursive lock depth. */
#define EXCLUSIVE_ITERATIONS 10000

/* The following is for ipc_reply_cap compliance. */
static L4_ThreadId_t master_tid;

/**
 *  @file Mutex Library's unit tests.
 *
 *  @internal
 *
 *  @todo FIXME: Tests for trylock functionality are missing, tests
 *  for contention are missing - awiggins.
 */

/**
 *  Test that the mutex object is correctly initialised.
 *
 *  @param mutex The mutex object to test the state of.
 */
static void test_mutex_valid_init(okl4_mutex_t mutex);

/**
 *  Test that a mutex object is correctly locked by the caller.
 *
 *  @param mutex The mutex object to test the state of.
 */
static void test_mutex_valid_lock(okl4_mutex_t mutex);

/**
 *  Test that a mutex object is correctly unlocked.
 *
 *  Note this can will only be valid if called when no threads are
 *  trying to acquire it, for example at the end of the tests.
 *
 *  @pram mutex The mutex object to test the state of.
 */
static void test_mutex_valid_unlock(okl4_mutex_t mutex);

#if defined(CONFIG_USER_MUTEXES)

static void
test_mutex_valid_init(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has been initialised in locked state");
    fail_if(mutex->needed != 0, "Mutex has not been initialised properly");
    fail_if(mutex->count != 0, "Mutex has not been initialised properly");
}

static void
test_mutex_valid_lock(okl4_mutex_t mutex)
{
    fail_if(okl4_mutex_is_unlocked(mutex), "Mutex has not been locked");
    fail_if(!okl4_mutex_am_holder(mutex), "This thread doesn't hold the mutex");
}

static void
test_mutex_valid_unlock(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has not been unlocked properly");
    fail_if(mutex->needed != 0, "Mutex has not been unlocked properly");
    fail_if(mutex->count != 0, "Mutex has not been unlocked properly");
}

#elif defined(CONFIG_KERNEL_MUTEXES)

static void
test_mutex_valid_init(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has been initialised in locked state");
    fail_if(mutex->count != 0, "Mutex has not been initialised properly");
}

static void
test_mutex_valid_lock(okl4_mutex_t mutex)
{
    fail_if(L4_TryLock(mutex->id),
            "Kernel object backing mutex was not locked");
    fail_if(okl4_mutex_is_unlocked(mutex), "Mutex has not been locked");
    fail_if(!okl4_mutex_am_holder(mutex), "This thread doesn't hold the mutex");
}

static void
test_mutex_valid_unlock(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has not been unlocked properly");
    fail_if(mutex->count != 0, "Mutex has not been unlocked properly");
}

#elif defined(CONFIG_HYBRID_MUTEXES)

static void
test_mutex_valid_init(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has been initialised in locked state.");
    fail_if(okl4_mutex_is_contended(mutex),
            "Mutex has been initialised in a contended state.");
    fail_if(mutex->count != 0, "Mutex has not been initialised properly");
}

static void
test_mutex_valid_lock(okl4_mutex_t mutex)
{
    fail_if(okl4_mutex_is_unlocked(mutex), "Mutex has not been locked");
    fail_if(!okl4_mutex_am_holder(mutex), "This thread doesn't hold the mutex");
}

static void
test_mutex_valid_unlock(okl4_mutex_t mutex)
{
    fail_if(!okl4_mutex_is_unlocked(mutex),
            "Mutex has not been unlocked properly");
    fail_if(okl4_mutex_is_contended(mutex),
            "Mutex has not been uncontended properly");
    fail_if(mutex->count != 0, "Mutex has not been unlocked properly");
}

#else
#error Mutex type configuration not specified.
#endif

/** Tests non-recursive locking, uncontended. */
START_TEST(test_mutex_lock_uncont)
{
    struct okl4_mutex m;
    okl4_mutex_t mutex = &m;

    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    okl4_mutex_lock(mutex);
    test_mutex_valid_lock(mutex);

    okl4_mutex_unlock(mutex);
    test_mutex_valid_unlock(mutex);

    okl4_mutex_free(mutex);
}
END_TEST

/** Tests recursive locking, uncontended. */
START_TEST(test_mutex_count_lock_uncont)
{
    struct okl4_mutex m;
    okl4_mutex_t mutex = &m;
    int i;

    L4_KDB_SetThreadName(master_tid, "main");

    /* Ensure that we can lock the same mutex multiple times without
     * deadlocking on ourself. */
    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    for(i = 0; i < COUNT; i++) {
        okl4_mutex_count_lock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != i + 1, "Mutex count is wrong");
    }
    for(i = 0; i < COUNT - 1; i++) {
        okl4_mutex_count_unlock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
    }
    okl4_mutex_count_unlock(mutex);
    test_mutex_valid_unlock(mutex);

    okl4_mutex_free(mutex);
}
END_TEST

static struct okl4_mutex m;
static okl4_mutex_t mutex = &m;


static volatile int thread1_done = 0;
static volatile int thread2_done = 0;
static volatile int thread_wait = 1;

static void
locking_thread(void *arg)
{
    L4_ThreadId_t any_thread;
    L4_MsgTag_t tag;
    L4_Word_t num;

    tag = L4_Receive(master_tid);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed");
    num = L4_Label(tag);

    okl4_mutex_lock(mutex);
    test_mutex_valid_lock(mutex);
    if (num == 1) {
        L4_Wait(&any_thread);
        while (thread_wait) ;
    }
    okl4_mutex_unlock(mutex);
    if (num == 1) {
        thread1_done = 1;
    }
    if (num == 3) {
        okl4_mutex_lock(mutex);
        test_mutex_valid_lock(mutex);
        okl4_mutex_unlock(mutex);
        test_mutex_valid_unlock(mutex);
    }
    if (num == 2 || num == 3)
        thread2_done = 1;

    L4_WaitForever();
}

static void
count_locking_thread(void *arg)
{
    L4_ThreadId_t any_thread;
    L4_MsgTag_t tag;
    L4_Word_t num;
    int i;

    tag = L4_Receive(master_tid);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed");
    num = L4_Label(tag);

    if (num == 1) {
        test_mutex_valid_unlock(mutex);
    }
    for(i = 0; i < COUNT; i++) {
        okl4_mutex_count_lock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != i + 1, "Mutex count is wrong");
    }
    if (num == 1) {
        L4_Wait(&any_thread);
        while (thread_wait) ;
    }
    for(i = 0; i < COUNT - 1; i++) {
        okl4_mutex_count_unlock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
    }
    okl4_mutex_count_unlock(mutex);
    if (num == 1) {
        thread1_done = 1;
    }
    if (num == 3) {
        for(i = 0; i < COUNT; i++) {
            okl4_mutex_count_lock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != i + 1, "Mutex count is wrong");
        }
        for(i = 0; i < COUNT - 1; i++) {
            okl4_mutex_count_unlock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
        }
        okl4_mutex_count_unlock(mutex);
    }
    if (num == 2 || num == 3) {
        thread2_done = 1;
    }
    L4_WaitForever();
}

static void
run_mutex_lock_cont(int counted, int add_uncont)
{
    thread_ref_t thrd1, thrd2;
    L4_ThreadId_t tid1, tid2;
    L4_MsgTag_t tag;

    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);
    thread1_done = 0;
    thread2_done = 0;
    thread_wait = 1;

    thrd1 = thread_create_simple(counted ? count_locking_thread : locking_thread,
            0, 100);

    /* setup thread 1 */
    tid1 = thread_l4tid(thrd1);
    tag = L4_Make_MsgTag(1, 0);
    L4_Set_MsgTag(tag);
    L4_Send(tid1);

    L4_LoadMR(0, 0);
    L4_Send(tid1);

    thrd2 = thread_create_simple(counted ? count_locking_thread : locking_thread,
            0, 100);
    tid2 = thread_l4tid(thrd2);
    tag = L4_Make_MsgTag(add_uncont ? 3 : 2, 0);
    L4_Set_MsgTag(tag);
    L4_Send(tid2);
    L4_ThreadSwitch(tid2);
    thread_wait = 0;

    while (!(thread1_done && thread2_done)) {
        L4_Yield();
    }

    printf("All threads done.\n");
    test_mutex_valid_unlock(mutex);
    okl4_mutex_free(mutex);
    thread_delete(tid1);
    thread_delete(tid2);
}

/** Tests non-recursive locking, contended. */
START_TEST(test_mutex_lock_cont)
{
    run_mutex_lock_cont(0, 0);
}
END_TEST

/** Tests recursive locking, contended. */
START_TEST(test_mutex_count_lock_cont)
{
    run_mutex_lock_cont(1, 0);
}
END_TEST

/**
 *  Tests non-recursive locking, contended followed by uncontended
 *  lock/unlock.
 */
START_TEST(test_mutex_lock_cont_uncont)
{
    run_mutex_lock_cont(0, 1);
}
END_TEST

/**
 *  Tests recursive locking, contended followed by uncontended
 *  recursive lock/unlock.
 */
START_TEST(test_mutex_count_lock_cont_uncont)
{
    run_mutex_lock_cont(1, 1);
}
END_TEST

/**
 *  Tests non-recursive locking, contended. Free the mutex and run the
 *  same test again.
 */
START_TEST(test_mutex_lock_cont_2)
{
    run_mutex_lock_cont(0, 0);
    run_mutex_lock_cont(0, 0);
}
END_TEST

/**
 *  Tests recursive locking, contended. Free the mutex and run the
 *  same test again.
 */
START_TEST(test_mutex_count_lock_cont_2)
{
    run_mutex_lock_cont(1, 0);
    run_mutex_lock_cont(1, 0);
}
END_TEST

static volatile unsigned long shared_resource = 0;
static volatile int all_thread_finished = 0;

static void
thread_run(void *arg)
{
    int iter = 0;
    word_t my_handle = okl4_mutex_my_handle();

    /* Give time to main thread to set their handle on SMT systems. */

    L4_KDB_SetThreadName(L4_Myself(), "res_ex");
    all_thread_finished++;

    while (iter < EXCLUSIVE_ITERATIONS) {
        okl4_mutex_lock(mutex);
        test_mutex_valid_lock(mutex);

        shared_resource = my_handle;
        _fail_unless(shared_resource == my_handle, __FILE__, __LINE__,
                "%" PRIxPTR " failed after %d loops\n", my_handle, iter);
        okl4_mutex_unlock(mutex);

        iter++;
    }

    all_thread_finished--;
    L4_WaitForever();
}

static void
count_thread_run(void *arg)
{
    int iter = 0;
    int i;
    word_t my_handle = okl4_mutex_my_handle();
    L4_MsgTag_t tag;

    /* Give time to main thread to set their handle on SMT systems. */

    L4_KDB_SetThreadName(L4_Myself(), "cnt_res_ex");
    tag = L4_Receive(master_tid);
    fail_unless(L4_IpcSucceeded(tag), "Startup IPC failed.");
    all_thread_finished++;

    while (iter < EXCLUSIVE_ITERATIONS) {
        for(i = 0; i < COUNT; i++) {
            okl4_mutex_count_lock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != i + 1, "Mutex count is wrong");
        }

        shared_resource = my_handle;
        _fail_unless(shared_resource == my_handle, __FILE__, __LINE__,
                "%" PRIxPTR " failed after %d loops\n", my_handle, iter);
        for(i = 0; i < COUNT - 1; i++) {
            okl4_mutex_count_unlock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
        }
        okl4_mutex_count_unlock(mutex);

        iter++;
    }

    all_thread_finished--;
    L4_WaitForever();
}

static void
thread_run_forever(void *arg)
{
    int iter = 0;
    word_t my_handle = okl4_mutex_my_handle();

    /* Give time to main thread to set their handle on SMT systems. */

    L4_KDB_SetThreadName(L4_Myself(), "res_ex");
    all_thread_finished++;

    while (iter < EXCLUSIVE_ITERATIONS) {
        okl4_mutex_lock(mutex);
        test_mutex_valid_lock(mutex);

        shared_resource = my_handle;
        _fail_unless(shared_resource == my_handle, __FILE__, __LINE__,
                "%" PRIxPTR " failed after %d loops\n", my_handle, iter);

        if ((int)arg == 0) {
            L4_WaitForever();
        }
        okl4_mutex_unlock(mutex);

        iter++;
    }

    all_thread_finished--;
    L4_WaitForever();
}

static volatile int high_started = 0;
static volatile int high_done = 0;
static volatile int med_done = 0;
static volatile int low_done = 0;
static volatile int low_ready = 0;
static volatile int do_count = 0;

static void
high_priority(void *arg)
{
    int i;

    L4_KDB_SetThreadName(L4_Myself(), "hi");

    /* Grab lock */
    high_started = 1;

    if (do_count) {
        for(i = 0; i < COUNT; i++) {
            okl4_mutex_count_lock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != i + 1, "Mutex count is wrong");
        }
    }
    else {
        okl4_mutex_lock(mutex);
    }
    test_mutex_valid_lock(mutex);
    fail_unless(low_done == 1,
                "Low priority thread did not finish or did not run");
    high_started = 0;
    high_done = 1;
    if (do_count) {
        for(i = 0; i < COUNT - 1; i++) {
            okl4_mutex_count_unlock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
        }
        okl4_mutex_count_unlock(mutex);
    } else {
        okl4_mutex_unlock(mutex);
    }
    printf("High finished\n");
    L4_WaitForever();
}

static void
med_priority(void *arg)
{
    L4_KDB_SetThreadName(L4_Myself(), "med");

    while (!high_done) {
        fail_unless(high_started == 0,
                    "Priority inversion error : Low priority thread should be executed instead of the medium one");
        L4_Yield();
    }
    printf("Medium finished\n");
    med_done = 1;
    L4_WaitForever();
}

static void
low_priority(void *arg)
{
    int i, j = 0;

    L4_KDB_SetThreadName(L4_Myself(), "lo");
    printf("Low priority started\n");

    /* Grab lock */
    if (do_count) {
        for(i = 0; i < COUNT; i++) {
            okl4_mutex_count_lock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != i + 1, "Mutex count is wrong");
        }
    }
    else {
        okl4_mutex_lock(mutex);
    }
    test_mutex_valid_lock(mutex);
    low_ready = 1;
    while (j++ < 20) {
        for (i = 0; i < 99999; i++);
        L4_Yield();
    }
    low_done = 1;
    printf("Low finished\n");
    if (do_count) {
        for(i = 0; i < COUNT - 1; i++) {
            okl4_mutex_count_unlock(mutex);
            test_mutex_valid_lock(mutex);
            fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
        }
        okl4_mutex_count_unlock(mutex);
    } else {
        okl4_mutex_unlock(mutex);
    }
    L4_WaitForever();
}

static void
run_mutex_prio_inheritance(L4_Word_t count)
{
    thread_ref_t low, med, high;
    L4_ThreadId_t tid;

    L4_KDB_SetThreadName(master_tid, "main");
    high_started = high_done = med_done = low_done = low_ready = 0;
    do_count = count;

    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    low = thread_create_simple(low_priority, 0, 89);
    while (!low_ready) {
        L4_ThreadSwitch(thread_l4tid(low));
    }
    med = thread_create_simple(med_priority, 0, 100);
    L4_Yield();
    high = thread_create_simple(high_priority, 0, 105);
    tid = thread_l4tid(high);

    while (!(low_done && high_done && med_done)) {
        L4_Yield();
    }

    printf("All threads done.\n");
    test_mutex_valid_unlock(mutex);
    okl4_mutex_free(mutex);
    thread_delete(thread_l4tid(high));
    thread_delete(thread_l4tid(med));
    thread_delete(thread_l4tid(low));
}

/** Test basic priority inheritance, non-recursive lock. */
START_TEST(test_mutex_prio_inheritance)
{
    run_mutex_prio_inheritance(0);
}
END_TEST

/** Test basic priority inheritance, recursive lock. */
START_TEST(test_mutex_count_prio_inheritance)
{
    run_mutex_prio_inheritance(1);
}
END_TEST

static void
run_mutex_res_exclusivity(int count)
{
    int i;
    thread_ref_t thread[NUM_THREADS];

    L4_KDB_SetThreadName(master_tid, "main");
    shared_resource = all_thread_finished = 0;

    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    for (i = 0; i < NUM_THREADS; i++) {
        thread[i] = thread_create_simple(count ? count_thread_run : thread_run,
                0, 100);
    }
    L4_Yield();

    while (all_thread_finished > 0) {
        L4_Yield();
    }

    printf("All threads done.\n");
    test_mutex_valid_unlock(mutex);
    okl4_mutex_free(mutex);
    for (i = 0; i < NUM_THREADS; i++) {
        thread_delete(thread_l4tid(thread[i]));
    }
}

/** Test basic resource exclusivity, recursive lock. */
START_TEST(test_mutex_count_res_exclusivity)
{
    run_mutex_res_exclusivity(1);
}
END_TEST

static void
run_mutex_thread_delete(int count)
{
    int i;
    thread_ref_t thread[NUM_THREADS];

    L4_KDB_SetThreadName(master_tid, "main");
    shared_resource = all_thread_finished = 0;

    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    for (i = 0; i < NUM_THREADS; i++) {
        thread[i] = thread_create_simple(thread_run_forever, (void *)(i % 2 == 0 ? 0 : 1), 100);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        L4_Yield();
        if ((i % 2) == 0) {
            thread_delete(thread_l4tid(thread[i]));
        }
    }

    while (all_thread_finished > NUM_THREADS / 2) {
        L4_Yield();
    }

    printf("All threads done.\n");
    test_mutex_valid_unlock(mutex);
    okl4_mutex_free(mutex);
    for (i = 0; i < NUM_THREADS; i++) {
        if ((i % 2) != 0) {
            thread_delete(thread_l4tid(thread[i]));
        }
    }
}

/** Test correct behaviour when threads holding locks are deleted. */
START_TEST(test_mutex_thread_delete)
{
    run_mutex_thread_delete(0);
}
END_TEST

#if defined(CONFIG_SMT)

/**
 *  Test basic resource exclusivity, non-recursive lock - concurrent
 *  thread execution.
 */
START_TEST(test_mutex_res_exclusivity_concurrent)
{
    delete_filler_threads();
    run_mutex_res_exclusivity(0);
}
END_TEST

/**
 *  Test basic resource exclusivity, recursive lock - concurrent
 *  thread execution.
 */
START_TEST(test_mutex_count_res_exclusivity_concurrent)
{
    delete_filler_threads();
    run_mutex_res_exclusivity(1);
}
END_TEST

#endif /* CONFIG_SMT */

/** Test basic resource exclusivity, non-recursive lock. */
START_TEST(test_mutex_res_exclusivity)
{
    run_mutex_res_exclusivity(0);
}
END_TEST

#define NUM_CHILDREN 4

#if !defined(CONFIG_USER_MUTEXES)

static void child_locking_thread(void *arg)
{
    L4_Word_t num;
    L4_MsgTag_t tag;

    L4_KDB_SetThreadName(L4_Myself(), "acq_order");
    tag = L4_Receive(master_tid);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed");
    num = L4_Label(tag);
    /* Acquire the mutex. */
    okl4_mutex_lock(mutex);
    test_mutex_valid_lock(mutex);

    /* Inform master thread we have the lock. */
    tag = L4_Make_MsgTag(num, 0);
    L4_Set_MsgTag(tag);
    L4_Send(master_tid);

    /* Release the lock for somebody else to have. */
    okl4_mutex_unlock(mutex);
    test_mutex_valid_unlock(mutex);

    L4_WaitForever();
}

static void child_count_locking_thread(void *arg)
{
    L4_Word_t num;
    L4_MsgTag_t tag;
    int i;

    L4_KDB_SetThreadName(L4_Myself(), "cnt_acq_order");
    tag = L4_Receive(master_tid);
    fail_unless(L4_IpcSucceeded(tag), "IPC failed");
    num = L4_Label(tag);
    /* Acquire the mutex. */
    for(i = 0; i < COUNT; i++) {
        okl4_mutex_count_lock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != i + 1, "Mutex count is wrong");
    }

    /* Inform master thread we have the lock. */
    tag = L4_Make_MsgTag(num, 0);
    L4_Set_MsgTag(tag);
    L4_Send(master_tid);

    /* Release the lock for somebody else to have. */
    for(i = 0; i < COUNT - 1; i++) {
        okl4_mutex_count_unlock(mutex);
        test_mutex_valid_lock(mutex);
        fail_if(mutex->count != COUNT - (i + 1), "Mutex count is wrong");
    }
    okl4_mutex_count_unlock(mutex);
    test_mutex_valid_unlock(mutex);

    L4_WaitForever();
}

static void
setup_thread(void *arg)
{
    while (1) {
        L4_LoadMR(0, 0);
        L4_Send(master_tid);
    }
}

static void
run_mutex_acquire_ordering(int count)
{
    L4_ThreadId_t children[NUM_CHILDREN];
    L4_ThreadId_t from_thread, setup_tid;
    thread_ref_t setup_tref;
    L4_Word_t i;
    L4_MsgTag_t tag;

    /* Create a mutex. */
    okl4_mutex_init(mutex);
    test_mutex_valid_init(mutex);

    /* Lock the mutex. */
    okl4_mutex_lock(mutex);
    test_mutex_valid_lock(mutex);

    setup_tref = thread_create_simple(setup_thread, 0, 1);
    setup_tid = thread_l4tid(setup_tref);
    /* Create our contesting children. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        thread_ref_t child;
        child = thread_create_simple(count ? child_count_locking_thread :
                child_locking_thread, 0, 90);
        /* set up the child */
        children[i] = thread_l4tid(child);
        tag = L4_Make_MsgTag(i + 1, 0);
        L4_Set_MsgTag(tag);
        L4_Send(children[i]);

        /* Let our child run until they hit the lock. */
        L4_Receive(setup_tid);

        /* Give the child an low priority. */
        L4_Set_Priority(children[i], NUM_CHILDREN - i + 1);
    }

    /* Allow children to access the lock. */
    okl4_mutex_unlock(mutex);
    test_mutex_valid_unlock(mutex);

    /* Ensure children get mutex in priority order. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        tag = L4_Wait(&from_thread);
        fail_unless(L4_Label(tag) == i + 1,
                "Threads not released in priority order.");
    }

    /* Delete our children. */
    for (i = 0; i < NUM_CHILDREN; i++) {
        thread_delete(children[i]);
    }
    thread_delete(setup_tid);

    okl4_mutex_free(mutex);
}

/** Tests the order that threads acquire the mutex observes priorities,
 * non-recursive locking. */
START_TEST(test_mutex_acquire_ordering)
{
    run_mutex_acquire_ordering(0);
}
END_TEST

/** Tests the order that threads acquire the mutex observes priorities,
 * recursive locking. */
START_TEST(test_mutex_count_acquire_ordering)
{
    run_mutex_acquire_ordering(1);
}
END_TEST

#endif /* CONFIG_USER_MUTEXES */

static void test_setup(void)
{
    create_filler_threads();
}

static void test_teardown(void)
{
    delete_filler_threads();
}

Suite *
make_test_libs_mutex_suite(void)
{
    Suite *suite;
    TCase *tc;

    master_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    suite = suite_create("mutex tests");
    tc = tcase_create("Core");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, test_mutex_lock_uncont);
    tcase_add_test(tc, test_mutex_count_lock_uncont);
    tcase_add_test(tc, test_mutex_lock_cont);
    tcase_add_test(tc, test_mutex_count_lock_cont);
    tcase_add_test(tc, test_mutex_lock_cont_uncont);
    tcase_add_test(tc, test_mutex_count_lock_cont_uncont);
    tcase_add_test(tc, test_mutex_lock_cont_2);
    tcase_add_test(tc, test_mutex_count_lock_cont_2);
    tcase_add_test(tc, test_mutex_prio_inheritance);
    tcase_add_test(tc, test_mutex_count_prio_inheritance);
#if !defined(CONFIG_USER_MUTEXES)
    tcase_add_test(tc, test_mutex_acquire_ordering);
    tcase_add_test(tc, test_mutex_count_acquire_ordering);
#endif
    tcase_add_test(tc, test_mutex_res_exclusivity);
    tcase_add_test(tc, test_mutex_count_res_exclusivity);
    /* Doesn't work for user mutex */
    if (0) {
    tcase_add_test(tc, test_mutex_thread_delete);
    }
#if defined(CONFIG_SMT)
    tcase_add_test(tc, test_mutex_res_exclusivity_concurrent);
    tcase_add_test(tc, test_mutex_count_res_exclusivity_concurrent);
#endif
    suite_add_tcase(suite, tc);
    return suite;
}

