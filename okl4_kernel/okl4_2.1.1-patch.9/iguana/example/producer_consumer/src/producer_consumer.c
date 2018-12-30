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
#include <l4/thread.h>
#include <mutex/mutex.h>
#include <iguana/env.h>
#include <rtos/rtos.h>

#define MAX_ITEM 16
#define MAX_LOOP 100

static volatile int empty;
static volatile okl4_mutex_t empty_mutex;
static volatile int full;
static volatile okl4_mutex_t full_mutex;
static volatile int total;
static volatile okl4_mutex_t buffer_mutex;
static volatile int buffer[MAX_ITEM];
static volatile L4_ThreadId_t pc_main;
static volatile int libs_ready = 0;

static char *test_name = "Producer consumer";

void producer(int argc, char **argv);
void consumer(int argc, char **argv);
int put_item_into_buffer(int item);
int remove_item_from_buffer(void);
void kill_remaining_threads(L4_ThreadId_t *tlist1, L4_ThreadId_t *tlist2, L4_Word_t nb_threads);
int check_print_data(L4_Word_t *consumer_metrics, int nb_threads);

int
put_item_into_buffer(int item)
{
    int i;

    for (i = 0; i < MAX_ITEM; i++) {
        if (buffer[i] == -1) {
            break;
        }
    }
    if (i == MAX_ITEM) {
        //printf("Producer: Shared buffer corrupted!\n");
        return -1;
    }
    buffer[i] = item;
    //printf("Item %d produced in slot %d\n", item, i);
    return 0;
}

int
remove_item_from_buffer(void)
{
    int i;
    int ret = -1;

    for (i = MAX_ITEM - 1; i >= 0; i--) {
        if (buffer[i] != -1) {
            ret = buffer[i];
            buffer[i] = -1;
            break;
        }
    }
    if (i < 0) {
        //printf("Consumer: Shared buffer corrupted!\n");
        return -1;
    }

    return ret;
}

/*
 * The producer writes data one at a time in a shared buffer. If the 
 * buffer is full it goes to sleep and ask for more consumers. 
 * For each third of its total data produced it asks for more threads.
 * When it has produced all its data it goes to sleep.
 */
void
producer(int argc, char **argv)
{
    int item = 0;
    int ret, written = 0;
    L4_MsgTag_t tag = L4_Niltag;

    while (!libs_ready) ;
    L4_Set_Label(&tag, 0x1);
    L4_Set_MsgTag(tag);
    L4_Call(pc_main);
    while (1) {
        okl4_mutex_count_lock(empty_mutex);
        if (empty) {
            empty--;
            okl4_mutex_count_unlock(empty_mutex);
            written++;
            item = rand();
            okl4_mutex_count_lock(buffer_mutex);
            ret = put_item_into_buffer(item);
            okl4_mutex_count_unlock(buffer_mutex);
            if (ret == -1) {
                written--;
                L4_Set_Label(&tag, 0xdead);
            }
            if (written == MAX_LOOP) {
                //printf("Producer %lx/%lx: Produced item %d\n", me, pc_main.raw, item);
                //printf("Producer %lx/%lx: produced %d items\n", me, pc_main.raw, written);
                okl4_mutex_count_lock(full_mutex);
                full++;
                okl4_mutex_count_unlock(full_mutex);
                L4_Set_Label(&tag, 0xbed);
            }
            if ((ret == -1) || (written == MAX_LOOP)) {
                L4_Set_MsgTag(tag);
                L4_Send(pc_main);
                L4_ThreadSwitch(pc_main);
                L4_Send(L4_myselfconst);
            }
            //printf("Producer %lx/%lx: Produced item %d\n", me, pc_main.raw, item);
            okl4_mutex_count_lock(full_mutex);
            full++;
            okl4_mutex_count_unlock(full_mutex);
            if (written % (MAX_LOOP/3) == 0) {
                L4_Set_Label(&tag, 0xadd);
                L4_Set_MsgTag(tag);
                L4_Send(pc_main);
            }
        } else {
            okl4_mutex_count_unlock(empty_mutex);
        }
    }
}

/*
 * The consumer reads data one at a time in a shared buffer 
 * as long as there are data available.
 * If the buffer is empty, the consumer goes to sleep.
 */
void
consumer(int argc, char **argv)
{
    int item, wait = 0;
    L4_Word_t read = 0;
    L4_MsgTag_t tag = L4_Niltag;
    L4_Msg_t msg;

    while (!libs_ready) ;
    L4_Set_Label(&tag, 0x2);
    L4_Set_MsgTag(tag);
    L4_Call(pc_main);
    while(1) {
        okl4_mutex_count_lock(full_mutex);
        if (full) {
            full--;
            okl4_mutex_count_unlock(full_mutex);
            wait = 0;
            okl4_mutex_count_lock(buffer_mutex);
            item = remove_item_from_buffer();
            okl4_mutex_count_unlock(buffer_mutex);
            if (item == -1) {
                L4_Set_Label(&tag, 0xdead);
                L4_Set_MsgTag(tag);
                L4_Send(pc_main);
                L4_ThreadSwitch(pc_main);
                L4_Send(L4_myselfconst);
            }
            //printf("Consumer %lx/%lx: Consumed item %d\n", me, pc_main.raw, item);
            okl4_mutex_count_lock(empty_mutex);
            empty++;
            total++;
            okl4_mutex_count_unlock(empty_mutex);
            read++;
        } else {
            okl4_mutex_count_unlock(full_mutex);
            wait++;
            if (wait >= 50) {
                L4_MsgClear(&msg);
                L4_Set_MsgMsgTag(&msg, tag);
                L4_Set_MsgLabel(&msg, 0xbed);
                L4_MsgAppendWord(&msg, read);
                L4_MsgLoad(&msg);
                L4_Call(pc_main);
            }
        }
    }
}

void kill_remaining_threads(L4_ThreadId_t *tlist1, L4_ThreadId_t *tlist2, L4_Word_t nb_threads)
{
    int i;

    for (i = 0; i < nb_threads; i++) {
        if (tlist1[i].raw != 0) {
            //printf("Producer %lx killed\n", tlist1[i].raw);
            thread_delete(tlist1[i]);
            tlist1[i] = L4_nilthread;
        }
        if (tlist2[i].raw != 0) {
            //printf("Consumer %lx killed\n", tlist2[i].raw);
            thread_delete(tlist2[i]);
            tlist2[i] = L4_nilthread;
        }
    }
}

int check_print_data(L4_Word_t *consumer_metrics, int nb_threads)
{
    int i, nb_char;
    L4_Word_t total_consumed = 0;
    char *str, *tmp_str;
    int pass = 1;
    
    str = malloc(700 * sizeof(char));
    tmp_str = str;
    nb_char = sprintf(tmp_str, "\nProducer-Consumer %lx results:", pc_main.raw);
    tmp_str += nb_char;
    for (i = 0; i < MAX_ITEM; i++) {
        if (buffer[i] != -1) {
            break;
        }
    }
    if ((empty != MAX_ITEM) || (full != 0) || 
        (total != nb_threads * MAX_LOOP) || (i < MAX_ITEM)) {
        nb_char = sprintf(tmp_str, "\nError! Shared buffer corrupted!");
        tmp_str += nb_char;
        pass = 0;
    }
    if (i < MAX_ITEM) {
        nb_char = sprintf(tmp_str, " -> Unconsumed item remaining in the buffer");
        tmp_str += nb_char;
        pass = 0;
    }
    nb_char = sprintf(tmp_str, "\n* Semaphore \"empty\" = %d\n* Semaphore \"full\" = %d\n* Total items consumed = %d (", empty, full, total);
    tmp_str += nb_char;
    for (i = 0; i < nb_threads; i++) {
        total_consumed += consumer_metrics[i];
        nb_char = sprintf(tmp_str, "%lu", consumer_metrics[i]);
        tmp_str += nb_char;
        if (i == nb_threads - 1) {
            nb_char = sprintf(tmp_str, " = %lu)\n\n", total_consumed);
            tmp_str += nb_char;
        } else {
            nb_char = sprintf(tmp_str, " + ");
            tmp_str += nb_char;
        }
    }
    printf("%s", str);
    free(str);
    return pass;
}

int
main(int argc, char **argv)
{
    int nb_threads, eg_num, tmp, i, j, k, server_on;
    L4_ThreadId_t *producer_tids, *consumer_tids;
    L4_ThreadId_t any_thread;
    L4_Word_t me;
    L4_MsgTag_t tag = L4_Niltag;
    L4_Msg_t msg;
    struct okl4_mutex bm, em, fm;
    L4_Word_t *consumer_metrics;
    pc_results_t pc_metrics;
    int pass;
    
    empty = MAX_ITEM;
    full = total = 0;

    /*** Initialisation ***/
    pc_main = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    me = pc_main.raw;
    buffer_mutex = &bm;
    okl4_mutex_init(buffer_mutex);
    empty_mutex = &em;
    okl4_mutex_init(empty_mutex);
    full_mutex = &fm;
    okl4_mutex_init(full_mutex);
    // Tell other threads that it is safe to use libraries
    libs_ready = 1;
    eg_num = nb_threads = server_on = 0;
    if (argc == 3) {
        eg_num = atoi(argv[0]);
        nb_threads = atoi(argv[1]);
        server_on = atoi(argv[2]);
    } else {
        printf("(%s 0x%lx) Error: Argument(s) missing!\n", test_name, me);
        return 1;
    }
    if (!server_on)
        printf("Start %s test #%d(0x%lx)\n", test_name, eg_num, me);
    for (i = 0; i < MAX_ITEM; i++) {
        buffer[i] = -1;
    }
    producer_tids = malloc(nb_threads * sizeof(L4_ThreadId_t));
    consumer_tids = malloc(nb_threads * sizeof(L4_ThreadId_t));
    consumer_metrics = malloc(nb_threads * sizeof(L4_Word_t));
    // Retrieve consumers and producers thread Ids
    for (i = 0, j = 0, k = 0; i < 2 * nb_threads; i++) {
        tag = L4_Wait(&any_thread);
        if ((L4_Label(tag) == 0x1) && (j < nb_threads)) {
            producer_tids[j] = any_thread;
            consumer_metrics[j++] = 0;
        }
        if ((L4_Label(tag) == 0x2) && (k < nb_threads))
            consumer_tids[k++] = any_thread;
    }
    j = 0;
    L4_LoadMR(0, 0);
    // Start only one producer and one consumer
    L4_Send(producer_tids[j]);
    L4_Send(consumer_tids[j]);

    /*** Start test ***/
    tmp = 2 * nb_threads;
    while (tmp) {
        tag = L4_Wait(&any_thread);
        //printf("Received label: %lx (tmp = %lu)\n", L4_Label(tag), tmp);
        if (L4_Label(tag) == 0xbed) {
            L4_MsgStore(tag, &msg);
            for (i = 0; i < nb_threads; i++) {
                // If a producer goes to sleep, it means it has finished
                // producing all its items.
                if (L4_IsThreadEqual(any_thread, producer_tids[i])) {
                    //printf("Producer %lx/%lx deleted\n", producer_tids[i].raw, pc_main.raw);
                    thread_delete(producer_tids[i]);
                    producer_tids[i] = L4_nilthread;
                    tmp--;
                    break;
                }
                // If a consumer goes to sleep and buffer is not empty or at
                // least one producer remains, then wake up the consumer.
                // Otherwise test is done.
                if (L4_IsThreadEqual(any_thread, consumer_tids[i])) {
                    if (tmp > nb_threads || full) {
                        L4_LoadMR(0, 0);
                        L4_Send(consumer_tids[i]);
                    } else {
                        consumer_metrics[i] = L4_MsgWord(&msg, 0);
                        //printf("Consumer %lx/%lx deleted\n", consumer_tids[i].raw, pc_main.raw);
                        //printf("Consumer %lx/%lx: consumed %lu items\n", consumer_tids[i].raw, pc_main.raw, consumer_metrics[i]);
                        thread_delete(consumer_tids[i]);
                        consumer_tids[i] = L4_nilthread;
                        tmp--;
                    }
                    break;
                }
            }
        }
        if (L4_Label(tag) == 0xadd) {
            // Start another producer and consumer thread if available
            if (j < (nb_threads - 1)) {
                L4_LoadMR(0, 0);
                L4_Send(producer_tids[++j]);
                L4_Send(consumer_tids[j]);
            }
        }
        // If one thread dies the buffer is probably corrupted. We stop the
        // test.
        if (L4_Label(tag) == 0xdead) {
            //printf("Killing all threads\n");
            kill_remaining_threads(producer_tids, consumer_tids, nb_threads);
            tmp = 0;
        }
    }
    /*** Test finished ***/
    free(producer_tids);
    free(consumer_tids);

    /* Clean up mutexes. */
    okl4_mutex_free(buffer_mutex);
    okl4_mutex_free(empty_mutex);
    okl4_mutex_free(full_mutex);

    // If RTOS server is on, report results to it.
    pass = check_print_data(consumer_metrics, nb_threads);
    if (server_on) {
        rtos_init();
        pc_metrics.empty = empty;
        pc_metrics.full = full;
        pc_metrics.total = total;
        pc_metrics.buffer = (int *)buffer;
        pc_metrics.consumer_metrics = consumer_metrics;
        producer_consumer_results(eg_num, nb_threads, MAX_ITEM, MAX_LOOP, &pc_metrics, pass);
        rtos_cleanup();
    } else {
        printf("%s test #%d(0x%lx) finished\n", test_name, eg_num, me);
    }
    free(consumer_metrics);

    return 0;
}
