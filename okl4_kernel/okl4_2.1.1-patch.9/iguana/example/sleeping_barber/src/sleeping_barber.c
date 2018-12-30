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
#include <mutex/mutex.h>
#include <iguana/env.h>
#include <rtos/rtos.h>

#define NB_SEATS 10

static volatile int customers = 0;
static volatile okl4_mutex_t customers_mutex;
static volatile okl4_mutex_t barber_mutex;
static volatile int nb_free_seats = NB_SEATS;
static volatile okl4_mutex_t access_seats_mutex;
static volatile L4_ThreadId_t sb_main;
static volatile L4_ThreadId_t barber_tid;
static volatile int libs_ready = 0;

static char *test_name = "Sleeping barber";

void customer(int argc, char **argv);
void barber(int argc, char **argv);

/*
 * The customer when enters the barber shop sits in a vacant chair. If there is
 * no chair available he leaves and comes back later. If he can sit he either awakens the
 * barber if he is sleeping or waits for his turn if the barber is busy with
 * someone else. 
 */
void
customer(int argc, char **argv)
{
    L4_MsgTag_t tag = L4_Niltag;
    L4_Word_t id;
    L4_Msg_t msg;

    while (!libs_ready) ;

    L4_Set_Label(&tag, 0x1);
    L4_Set_MsgTag(tag);
    tag = L4_Call(sb_main);

    L4_MsgStore(tag, &msg);
    id = L4_MsgWord(&msg, 0);

    while (1) {
        okl4_mutex_lock(access_seats_mutex);
        if (nb_free_seats) {
            // Take a seat
            nb_free_seats--;
            okl4_mutex_unlock(access_seats_mutex);
            okl4_mutex_lock(customers_mutex);
            customers++;
            if (customers == 1) {
                // Wake up the barber
                okl4_mutex_unlock(customers_mutex);
                L4_LoadMR(0, 0);
                L4_Send(barber_tid);
            } else
                okl4_mutex_unlock(customers_mutex);
            okl4_mutex_lock(barber_mutex);
            // Has his hair-cut
            L4_LoadMR(0, 0);
            L4_Send(barber_tid);
            okl4_mutex_unlock(barber_mutex);
            L4_MsgClear(&msg);
            L4_Set_MsgLabel(&msg, 0xdead);
            L4_MsgAppendWord(&msg, (L4_Word_t)id);
            L4_MsgLoad(&msg);
            L4_Call(sb_main);
        } else {
            okl4_mutex_unlock(access_seats_mutex);
        }
    }
}

/*
 * The barber cuts one customer's hair at a time. If he has no customers he
 * goes to sleep and waits to be woken by the next coming customer.
 */
void
barber(int argc, char **argv)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag = L4_Niltag;
    L4_MsgTag_t temp = L4_Niltag;
    L4_Msg_t msg;
    L4_Word_t cut = 0;

    while (!libs_ready) ;
    L4_Set_Label(&tag, 0x2);
    L4_Set_MsgTag(tag);
    L4_Call(sb_main);
    while(1) {
        okl4_mutex_lock(customers_mutex);
        if (customers) {
            customers--;
            okl4_mutex_unlock(customers_mutex);
            // The customer to have his hair-cut releases a chair
            okl4_mutex_lock(access_seats_mutex);
            nb_free_seats++;
            okl4_mutex_unlock(access_seats_mutex);
            L4_Wait(&dummy);
            cut++;
            L4_Set_Label(&tag, 0xadd);
            L4_Set_MsgTag(tag);
            L4_Send(sb_main);
        } else {
            okl4_mutex_unlock(customers_mutex);
            // Barber sits in his chair
            okl4_mutex_lock(access_seats_mutex);
            nb_free_seats--;
            okl4_mutex_unlock(access_seats_mutex);
            // Ask for more customers
            L4_Set_Label(&tag, 0xadd);
            L4_Set_MsgTag(tag);
            L4_Send(sb_main);
            // Go to sleep
            L4_MsgClear(&msg);
            L4_Set_MsgMsgTag(&msg, tag);
            L4_Set_MsgLabel(&msg, 0xbed);
            L4_MsgAppendWord(&msg, cut);
            L4_MsgLoad(&msg);
            // Inform main that we are sleeping.
            // CCH: Can't use L4_ReplyWait at main thread
            // may not be ready at the time, and L4_ReplyWait
            // doesn't set send block, it will fail and the
            // msg will lost.
            temp = L4_MsgTag();
            L4_Set_SendBlock(&tag);
            L4_Set_ReceiveBlock(&tag);
            L4_Ipc(sb_main, L4_anythread, temp, &dummy);
        }
    }
}

int
main(int argc, char **argv)
{
    struct okl4_mutex cm, bm, fsm;
    L4_ThreadId_t any_thread, *customers_tids;
    L4_Word_t label;
    int i, j, eg_num, nb_customers, tmp, server_on, dead_id;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    char cap_name[20];

    /*** Initialisation ***/
    sb_main = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    eg_num = nb_customers = server_on = 0;
    if (argc == 3) {
        eg_num = atoi(argv[0]);
        nb_customers = atoi(argv[1]);
        server_on = atoi(argv[2]);
    } else {
        printf("(%s) Error: Argument(s) missing!\n", test_name);
        return 1;
    }
    barber_tid = L4_nilthread;
    customers_mutex = &cm;
    okl4_mutex_init(customers_mutex);
    barber_mutex = &bm;
    okl4_mutex_init(barber_mutex);
    access_seats_mutex = &fsm;
    okl4_mutex_init(access_seats_mutex);

    // Tell other threads that it is safe to use libraries
    libs_ready = 1;

    if (!server_on)
        printf("Start %s test #%d: %d customers\n", test_name, eg_num, nb_customers);
    customers_tids = malloc(nb_customers * sizeof(L4_ThreadId_t));

    // Retrieve customers and barber thread Ids
    for (i = 0, j = 0; i <= nb_customers; i++) {
        tag = L4_Wait(&any_thread);

        if (L4_Label(tag) == 0x1) {
            sprintf(cap_name, "CUSTOMER_%d", i-j);
            customers_tids[i - j] = thread_l4tid(env_thread(iguana_getenv(cap_name)));
        }
        if (L4_Label(tag) == 0x2) {
            sprintf(cap_name, "BARBER_%d", eg_num);
            barber_tid = thread_l4tid(env_thread(iguana_getenv(cap_name)));
            j = 1;
        }
    }
    // Start only the barber and one customer
    L4_LoadMR(0, 0);
    L4_Send(barber_tid);
    j = 0;
    if (nb_customers) {
        L4_MsgClear(&msg);
        L4_MsgAppendWord(&msg, (L4_Word_t)j);
        L4_MsgLoad(&msg);
        L4_Send(customers_tids[j]);
    }

    /*** Start test ***/
    tmp = nb_customers + 1;
    while (tmp) {
        tag = L4_Wait(&any_thread);
        L4_MsgStore(tag, &msg);
        label = L4_Label(tag);
        //printf("Received label: %lx (tmp = %d)\n", L4_Label(tag), tmp);
        if (label == 0xbed) {
            // If barber goes to sleep and is the last thread remaining, then
            // test is done.
            // CCH: We shouldn't use temp == 1 to test if all customer has done,
            // since the last customer send ipc to main after it woke up barber,
            // therefore, barber could send "bed" msg to main before customer
            // send "dead" msg ask main to delete it. As a result, when main
            // get "bed" msg from barber, temp maybe equals 2!. We should use
            // j == nb_customers - 1, since if all customers are invoked and 
            // barber goes to bed means barber has done even though customer
            // is still left (he has cut his hair but still wondering in the
            // shop :-)
            if (j == (nb_customers - 1) && (L4_MsgWord(&msg, 0) == nb_customers)) {
                /*** Test finished ***/
                // If RTOS server is on, report results to it.
                if (server_on) {
                    rtos_init();
                    sleeping_barber_results(eg_num, nb_customers, L4_MsgWord(&msg, 0));
                    rtos_cleanup();
                } else
                    printf("Barber %lx/%lx finished: %lu hair cut done, %d customers\n", 
                            barber_tid.raw, sb_main.raw, L4_MsgWord(&msg, 0), nb_customers);
                thread_delete(barber_tid);
                tmp--;
            }
        }
        if (label == 0xadd) {
            // Start another customer thread if available
            if (j < (nb_customers - 1)) {
                j++;
                L4_MsgClear(&msg);
                L4_MsgAppendWord(&msg, (L4_Word_t)j);
                L4_MsgLoad(&msg);
                L4_Send(customers_tids[j]);
            }
        }
        // If one customer finishes (had his hair cut), we delete it.
        if (label == 0xdead) {
            dead_id = (int)L4_MsgWord(&msg, 0);
            if (dead_id < nb_customers) {
                //printf("Customer %lx/%lx deleted\n", customers_tids[dead_id].raw, sb_main.raw);
                thread_delete(customers_tids[dead_id]);
                customers_tids[dead_id] = L4_nilthread;
                tmp--;
            }
        }
    }
    free(customers_tids);

    if (!server_on)
        printf("%s test #%d finished\n", test_name, eg_num);

    return 0;
}
