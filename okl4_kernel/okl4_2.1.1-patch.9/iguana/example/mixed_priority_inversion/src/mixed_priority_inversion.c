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
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <iguana/thread.h>
#include <l4/kdebug.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <mutex/mutex.h>
#include <iguana/env.h>
#include <rtos/rtos.h>

#define LIMIT 100000

static volatile okl4_mutex_t resource_mutex;
static volatile L4_ThreadId_t pi_main;
static volatile L4_ThreadId_t high_prio_thread;
static volatile L4_ThreadId_t medium1_prio_thread;
static volatile L4_ThreadId_t medium2_prio_thread;
static volatile L4_ThreadId_t medium3_prio_thread;
static volatile L4_ThreadId_t low_prio_thread;
static volatile int flag1 = 0;
static volatile int flag3 = 0;
static volatile int cnt_h = 0;
static volatile int cnt_m1 = 0;
static volatile int cnt_m2 = 0;
static volatile int cnt_l = 0;
static volatile int cnt_i1 = 0;
static volatile int cnt_i2 = 0;
static volatile int stop_spinning = 1;
static volatile int libs_ready = 0;

static volatile int scenario1 = 0;
static volatile int scenario2 = 0;
static char *test_name = "Mixed priority inversion";

void mixed_pi_high(int argc, char **argv);
void mixed_pi_medium(int argc, char **argv);
void mixed_pi_low(int argc, char **argv);
void mixed_pi_intermediate(int argc, char **argv);

/*
 * The highest priority thread blocks sending to medium thread in the
 * first scenario. It increments a counter on success.
 * It tries to acquire a mutex in the second scenario. It increments a counter on success.
 */
void
mixed_pi_high(int argc, char **argv)
{
    L4_MsgTag_t tag;

    while (!libs_ready) ;
    L4_Receive(pi_main);

    //printf("High thread %lx/%lx starts PI test\n", me, pi_main.raw);
    while (1) {
        while (stop_spinning) ;
        if (scenario1) {
            L4_LoadMR(0, 0);
            tag = L4_Send(medium2_prio_thread);
            if (L4_IpcSucceeded(tag))
                cnt_h++;
        } else if (scenario2) {
            //printf("High thread %lx/%lx acquiring mutex for resource, current holder: 0x%lx\n", me, pi_main.raw, resource_mutex->holder);
            okl4_mutex_count_lock(resource_mutex);
            //printf("High thread %lx/%lx acquired mutex for resource\n", me, pi_main.raw);
            cnt_h++;
        }
        // If intermediate threads have run, then increment respective counter.
        if (flag1) 
            cnt_i1++;
        if (flag3) 
            cnt_i2++;
        if (scenario2)
            okl4_mutex_count_unlock(resource_mutex);
        // Tell main thread iteration is done.
        L4_LoadMR(0, 0);
        L4_Call(pi_main);
        // Re-initialise.
        if (flag1) {
            L4_Receive_Nonblocking(medium1_prio_thread);
            flag1 = 0;
        }
        if (flag3) {
            L4_Receive_Nonblocking(medium3_prio_thread);
            flag3 = 0;
        }
    }
}

static void
wait_a_bit(int counter)
{
    int i;

    for (i = 0; i < counter; i++) ;
}

/*
 * The medium thread tries to acquire a mutex, releases it once acquired and then 
 * receives from highest priority thread in scenario 1. It increments 2 counters 
 * on success, one for each operation.
 * It acquires a mutex at initialisation time and waits for any thread before
 * releasing the mutex in scenario 2. It increments 2 counters on success, 
 * one for each operation.
 */
void
mixed_pi_medium(int argc, char **argv)
{
    L4_ThreadId_t any_thread, tid;
    L4_MsgTag_t tag;
    int counter = 10000;

    while (!libs_ready) ;
    while (medium1_prio_thread.raw == 0) ;
    tid = medium1_prio_thread;

    //printf("Middle thread %lx/%lx starts PI test\n", me, pi_main.raw);
    while (1) {
        // Initialisation
        if (scenario2) {
            //printf("Middle thread %lx/%lx acquires mutex for resource\n", me, pi_main.raw);
            okl4_mutex_count_lock(resource_mutex);
            cnt_m1++;
        }
        //printf("(Initialisation) Medium thread %lx/%lx blocks open receiving\n", me, pi_main.raw);
        L4_Wait(&any_thread);
        //printf("(Initialisation) Medium thread %lx/%lx received from 0x%lx\n", me, pi_main.raw, any_thread.raw);
        L4_LoadMR(0, 0);
        L4_Send(tid);
        L4_Yield();
        /*** Start test ***/
        while (stop_spinning) ;
        wait_a_bit(counter);
        if (scenario1) {
            okl4_mutex_count_lock(resource_mutex);
            cnt_m1++;
            wait_a_bit(counter);
        } else if (scenario2) {
            //printf("Middle thread %lx/%lx blocks open receiving\n", me, pi_main.raw);
            tag = L4_Wait(&any_thread);
            if (L4_IpcSucceeded(tag) && (any_thread.raw == low_prio_thread.raw))
                cnt_m2++;
            wait_a_bit(counter);
        }
        okl4_mutex_count_unlock(resource_mutex);
        if (!flag1 && (counter < LIMIT)) {
            counter *= 2;
        }
        if (scenario1) {
            cnt_m2++;
            tag = L4_Receive(high_prio_thread);
            if (L4_IpcFailed(tag))
                cnt_m2--;
        }
        L4_Yield();
        tid = pi_main;
    }
}

/*
 * The lowest priority thread acquires a mutex at initialisation time, releases
 * it the next it is run in scenario 1. It increments a counter on success.
 * It sends IPC to medium thread in scenario 2. It increments a counter on success.
 */
void
mixed_pi_low(int argc, char **argv)
{
    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    int counter = 10000;

    while (!libs_ready) ;
    while (medium3_prio_thread.raw == 0) ;
    tid = medium3_prio_thread;

    //printf("Low thread %lx/%lx starts PI test\n", me, pi_main.raw);
    while (1) {
        // Initalisation
        if (scenario1) {
            //printf("Low thread %lx/%lx acquires mutex for resource\n", me, pi_main.raw);
            okl4_mutex_count_lock(resource_mutex);
            cnt_l++;
        }
        //printf("(Initialisation) Low thread %lx/%lx blocks sending to Medium thread\n", me, pi_main.raw);
        L4_LoadMR(0, 0);
        L4_Send(tid);
        //printf("(Initialisation) Low thread %lx/%lx sent to Medium thread\n", me, pi_main.raw);
        L4_Yield();
        /*** Start test ***/
        while (stop_spinning) ;
        wait_a_bit(counter);
        if (!flag1 && (counter < LIMIT)) {
            counter *= 2;
        }
        if (scenario1) {
            okl4_mutex_count_unlock(resource_mutex);
        } else if (scenario2) {
            cnt_l++;
            //printf("Low thread %lx/%lx blocks sending to Medium thread\n", me, pi_main.raw);
            L4_LoadMR(0, 0);
            tag = L4_Send(medium2_prio_thread);
            if (L4_IpcFailed(tag))
                cnt_l--;
        }
        L4_Yield();
        tid = medium2_prio_thread;
    }
}

/*
 * Intermediate thread 1 has priority in between highest and medium thread.
 * Intermediate thread 2 has priority in between medium and lowest thread.
 * They set their flag each time they run during the PI test.
 */
void
mixed_pi_intermediate(int argc, char **argv)
{
    int num = 0;
    L4_ThreadId_t any_thread;
    L4_MsgTag_t tag = L4_Niltag;

    while (!libs_ready) ;
    if (argc) {
        num = atoi(argv[0]);
    } else {
        printf("(%s Intermediate Thread) Error: Argument(s) missing!\n", test_name);
        L4_Set_Label(&tag, 0xdead);
        L4_Set_MsgTag(tag);
        L4_Call(pi_main);
    }
    // Initialisation
    if (num == 1) {
        L4_Wait(&any_thread);
        L4_LoadMR(0, 0);
        L4_Send(pi_main);
    } else if (num == 2) {
        L4_Wait(&any_thread);
        while (medium2_prio_thread.raw == 0) ;
        L4_LoadMR(0, 0);
        L4_Send(medium2_prio_thread);
    }
    L4_Yield();

    // Thread is now ready to set the flag the next time it is run.
    while(1) {
        if (num == 1) {
            flag1 = 1;
        } else if (num == 2) {
            flag3 = 1;
        }
        L4_LoadMR(0, 0);
        L4_Send(high_prio_thread);
    }
}

static void
print_metrics(int iter)
{
    int nb_char;
    char *str, *tmp_str;

    str = malloc(800 * sizeof(char));
    tmp_str = str;
    if (scenario1) {
        nb_char = sprintf(tmp_str, "\n%s (0x%lx) results on %d iterations: Close IPC", test_name, pi_main.raw, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* High thread - messages sent successfully to Medium thread: %d/%d", cnt_h, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* Medium thread acquired mutex: %d/%d; messages received successfully from High thread: %d/%d", cnt_m1, iter, cnt_m2, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* Low thread acquired mutex: %d/%d", cnt_l, iter);
        tmp_str += nb_char;
    } else if (scenario2) {
        nb_char = sprintf(tmp_str, "\n%s (0x%lx) results on %d iterations: Open IPC", test_name, pi_main.raw, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* High thread acquired mutex: %d/%d", cnt_h, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* Medium thread acquired mutex: %d/%d; messages received successfully from Low thread: %d/%d", cnt_m1, iter, cnt_m2, iter);
        tmp_str += nb_char;
        nb_char = sprintf(tmp_str, "\n* Low thread - messages sent successfully to Medium thread: %d/%d", cnt_l, iter);
        tmp_str += nb_char;
    }
    if (cnt_i1) {
        nb_char = sprintf(tmp_str, "\n* Intermediate thread 1 ran %d times, ", cnt_i1);
        tmp_str += nb_char;
    } else {
        nb_char = sprintf(tmp_str, "\n* Intermediate thread 1 did not run, ");
        tmp_str += nb_char;
    }
    if (cnt_i2) {
        nb_char = sprintf(tmp_str, "Intermediate thread 2 ran %d times\n\n", cnt_i2);
        tmp_str += nb_char;
    } else {
        nb_char = sprintf(tmp_str, "Intermediate thread 2 did not run\n\n");
        tmp_str += nb_char;
    }
    printf("%s", str);
    free(str);
}

int
main(int argc, char **argv)
{
    struct okl4_mutex rm;
    L4_ThreadId_t tid;
    int i, max_iteration, eg_num, server_on;
    L4_Word_t me;
    L4_MsgTag_t tag = L4_Niltag;

    /*** Initialisation ***/
    pi_main = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    me = pi_main.raw;
    eg_num = max_iteration = server_on = 0;
    if (argc == 3) {
        eg_num = atoi(argv[0]);
        max_iteration = atoi(argv[1]);
        server_on = atoi(argv[2]);
    } else {
        printf("(%s 0x%lx) Error: Argument(s) missing!\n", test_name, me);
        return 1;
    }
    resource_mutex = &rm;
    okl4_mutex_init(resource_mutex);
    high_prio_thread = medium1_prio_thread = medium2_prio_thread = medium3_prio_thread = low_prio_thread = L4_nilthread;

    high_prio_thread = thread_l4tid(env_thread(iguana_getenv("MIXED_PI_HIGH")));
    medium3_prio_thread = thread_l4tid(env_thread(iguana_getenv("MIXED_PI_INTERMEDIATE_2")));
    medium2_prio_thread = thread_l4tid(env_thread(iguana_getenv("MIXED_PI_MEDIUM")));
    medium1_prio_thread = thread_l4tid(env_thread(iguana_getenv("MIXED_PI_INTERMEDIATE_1")));
    low_prio_thread = thread_l4tid(env_thread(iguana_getenv("MIXED_PI_LOW")));

    // Tell other threads that it is safe to use libraries
    libs_ready = 1;

    if (!server_on)
        printf("Start %s test #%d(0x%lx)\n", test_name, eg_num, me);
    /*** Start test ***/
    scenario1 = 1;
    for (i = 0; i < 2 * max_iteration; i++) {
        // Wait for threads to be ready
        tag = L4_Wait(&tid);
        // If one thread had a problem while initialisation, then stop the test and notify
        // server that the test is dead.
        if (L4_Label(tag) == 0xdead) {
            rtos_init();
            test_died(test_name, eg_num);
            rtos_cleanup();
            return 1;
        }
        // Tell high prio thread to start the next iteration.
        L4_LoadMR(0, 0);
        tag = L4_Send(high_prio_thread);
        stop_spinning = 0;
        // Wait for the iteration to finish.
        L4_Receive(high_prio_thread);
        stop_spinning = 1;
        // If end of iterations for scenario1, then report results to RTOS server if server is on.
        if (i == (max_iteration - 1)) {
            if (server_on) {
                rtos_init();
                mixed_priority_inversion_results(eg_num, 1, max_iteration, cnt_h, cnt_m1, cnt_m2, cnt_l, cnt_i1, cnt_i2);
            } else 
                print_metrics(max_iteration);
            // Start scenario2
            cnt_h = cnt_m1 = cnt_m2 = cnt_l = cnt_i1 = cnt_i2 = 0;
            scenario1 = 0;
            scenario2 = 1;
        }
    }
    /*** Test finished ***/
    thread_delete(medium1_prio_thread);
    thread_delete(medium3_prio_thread);
    thread_delete(high_prio_thread);
    thread_delete(medium2_prio_thread);
    thread_delete(low_prio_thread);

    /* Clean up allocated mutexes. */
    okl4_mutex_free(resource_mutex);

    // If RTOS server is on, report results to it.
    if (server_on) {
        mixed_priority_inversion_results(eg_num, 2, max_iteration, cnt_h, cnt_m1, cnt_m2, cnt_l, cnt_i1, cnt_i2);
        rtos_cleanup();
    } else {
        print_metrics(max_iteration);
        printf("%s test #%d(0x%lx) finished\n", test_name, eg_num, me);
    }

    return 0;
}
