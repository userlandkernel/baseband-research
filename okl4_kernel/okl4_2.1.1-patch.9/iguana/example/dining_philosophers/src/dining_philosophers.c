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


static volatile int meal_served = 0;
static volatile okl4_mutex_t * chopstick;
static volatile L4_ThreadId_t dp_main;
static volatile int libs_ready = 0;

static char *test_name = "Dining philosophers";

void philosopher(int argc, char **argv);

/*
 * The Philosopher thread tries to take the 2 chopsticks closest to him 
 * (according to its number and the number of each chopstick) in order to eat.
 */
void
philosopher(int argc, char **argv)
{
    int left, right, first, second, mynum = 0;
    L4_Word_t me = L4_Myself().raw;
    L4_MsgTag_t tag = L4_Niltag;

    while (!libs_ready) ;
    L4_LoadMR(0, 0);
    L4_Send(dp_main);
    if (argc) {
        mynum = atoi(argv[0]);
    } else {
        printf("(Philosopher 0x%lx) Error: Argument(s) missing!\n", me);
        L4_Receive(dp_main);
        L4_Set_Label(&tag, 0xdead);
        L4_Set_MsgTag(tag);
        L4_Call(dp_main);
    }

    // Have to take chopsticks according to "my number"
    left = mynum;
    if (chopstick[mynum + 1]) {
        right = mynum + 1;
    } else {
        right = 0;
    }
    // This should allow to avoid deadlock when trying to grab the chopsticks.
    first = left < right ? left : right;
    second = first == left ? right : left;
    //printf("Philosopher %d: %lx, managed by %lx\n", mynum, me, dp_main.raw);
    //printf("Philosopher %d: chopsticks %d and %d\n", mynum, left, right);
    L4_Receive(dp_main);
    while (meal_served == 0) ;
    okl4_mutex_count_lock(chopstick[first]);
    //printf("Philosopher %d/%lx holds chopstick %d/%lx\n", mynum, me, first, chopstick[first]);
    okl4_mutex_count_lock(chopstick[second]);
    //printf("Philosopher %d/%lx holds chopstick %d/%lx\n", mynum, me, second, chopstick[second]);
    //printf("Philosopher %d/%lx is eating\n", mynum, me);

    okl4_mutex_count_unlock(chopstick[second]);
    okl4_mutex_count_unlock(chopstick[first]);
    //printf("Philosopher %d: %lx/%lx finished eating\n", mynum, me, dp_main.raw);

    L4_Set_Label(&tag, 0xfed);
    L4_Set_MsgTag(tag);
    L4_Send(dp_main);
    L4_Send(L4_Myself());
}

int
main(int argc, char **argv)
{
    int i, tmp, eg_num, nb_philosophers, nb_dinners, server_on;
    L4_ThreadId_t *philo_tids;
    L4_ThreadId_t tid, any_thread;
    L4_MsgTag_t tag = L4_Niltag;

    /*** Initialisation ***/
    dp_main = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    eg_num = nb_philosophers = nb_dinners = server_on = 0;
    // Tell other threads that it is safe to use libraries
    libs_ready = 1;
    if (argc == 3) {
        eg_num = atoi(argv[0]);
        nb_philosophers = atoi(argv[1]);
        server_on = atoi(argv[2]);
    } else {
        printf("(%s 0x%lx) Error: Argument(s) missing!\n", test_name, dp_main.raw);
        return 1;
    }
    if (!server_on)
        printf("Start %s test #%d(0x%lx): %d philosophers\n", test_name, eg_num, dp_main.raw, nb_philosophers);
    chopstick = malloc((nb_philosophers + 1) * sizeof(okl4_mutex_t));
    philo_tids = malloc(nb_philosophers * sizeof(L4_ThreadId_t));
    for (i = 0; i < nb_philosophers; i++) {
        chopstick[i] = malloc (sizeof(struct okl4_mutex));
        okl4_mutex_init(chopstick[i]);
    }
    chopstick[nb_philosophers] = NULL;

    // Retrieve philosophers thread Ids
    for (i = 0; i < nb_philosophers; i++) {
        L4_Wait(&tid);
        philo_tids[i] = tid;
    }
    // Give them a go
    for (i = 0; i < nb_philosophers; i++) {
        L4_Send(philo_tids[i]);
    }

    /*** Start test ***/
    meal_served = 1;
    tmp = nb_philosophers;
    while (tmp) {
        tag = L4_Wait(&any_thread);
        for (i = 0; i < nb_philosophers; i++) {
            if (any_thread.raw == philo_tids[i].raw) {
                thread_delete(any_thread);
                tmp--;
                if (L4_Label(tag) == 0xfed) {
                    // Philosopher finished eating
                    nb_dinners++;
                }
                break;
            }
        }
    }

    /*** Test finished ***/
    free(philo_tids);
    for (i = 0; i < nb_philosophers; i++) {
        okl4_mutex_free(chopstick[i]);
        free(chopstick[i]);
    }
    //free(*chopstick);
    // If RTOS server is on, report results to it.
    if (server_on) {
        rtos_init();
        dining_philosophers_results(eg_num, nb_philosophers, nb_dinners);
        rtos_cleanup();
    }
    else
        printf("%s test #%d(0x%lx) finished\n", test_name, eg_num, dp_main.raw);

    return 0;
}
