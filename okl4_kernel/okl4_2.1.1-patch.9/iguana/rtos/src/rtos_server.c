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
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <rtos/rtos.h>
#include <interfaces/rtos_serverdecls.h>


static volatile int remaining_tests, nb_tests, nb_passed, nb_failed, nb_errors;
static volatile int mixed_pi_failure = 0;

static void print_end(void);
void dummy_thread(int argc, char **argv);

static void
print_end(void)
{
    printf("\n===> RTOS tests finished! Tests:%d, Passed:%d, Failed:%d, Errors:%d <===\n", nb_tests, nb_passed, nb_failed, nb_errors);
}

void
rtos_report_dining_philosophers_results_impl(CORBA_Object _caller, int eg_num, int nb_philosophers, 
                                             int nb_dinners, idl4_server_environment * _env)
{
    printf("\nDining philosophers test #%d finished: %d/%d philosopher(s) had dinner", eg_num, nb_dinners, nb_philosophers);
    if (nb_philosophers == nb_dinners)
        nb_passed++;
    else {
        nb_failed++;
        printf(" --> Failed!");
    }
    printf("\n");

    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_mapping_results_impl(CORBA_Object _caller, int eg_num, int nb_threads,
                                 int nb_mapped_memsection, int nb_corrupted_memsection, 
                                 int nb_creation_failure, int total_size_mapped, idl4_server_environment * _env)
{
    printf("\nMapping test #%d finished: %d thread(s) successfully mapped %d memsections", eg_num, nb_threads, nb_mapped_memsection);
    printf("\n* Corrupted memsections: %d", nb_corrupted_memsection);
    if (nb_corrupted_memsection) {
        nb_failed++;
        printf(" --> Failed!");
    } else
        nb_passed++;
    printf("\n* Memsection creation failures: %d", nb_creation_failure);
    printf("\n* Total size mapped: %d Ko\n", total_size_mapped);

    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_mapping_nomem_results_impl(CORBA_Object _caller, int eg_num, int nb_threads, int nb_mapped_memsection, 
                                       int nb_corrupted_memsection, int nb_creation_failure, int nb_pd_creation_failure, 
                                       int total_size_mapped, idl4_server_environment * _env)
{
    int failure = 0;

    printf("\nMapping until no memory test #%d finished: %d thread(s) successfully mapped %d memsections", eg_num, nb_threads, nb_mapped_memsection);
    printf("\n* Corrupted memsections: %d", nb_corrupted_memsection);
    if (nb_corrupted_memsection) {
        failure = 1;
        printf(" --> Failed!");
    }
    printf("\n* Memsection creation failures: %d", nb_creation_failure);
    printf("\n* PD creation failures: %d", nb_pd_creation_failure);
    if (nb_pd_creation_failure) {
        failure = 1;
        printf(" --> Failed!");
    }
    printf("\n* Total size mapped: %d Ko\n", total_size_mapped);
    if (failure)
        nb_failed++;
    else
        nb_passed++;

    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_ipc_priority_inversion_results_impl(CORBA_Object _caller, int eg_num, int nb_iterations,
                                                int cnt_h, int cnt_m1, int cnt_m2, int cnt_l, 
                                                int cnt_i1, int cnt_i2, idl4_server_environment * _env)
{
    int failure = 0;

    /* Determine what type of scheduler is in use. */
#ifdef CONFIG_STRICT_SCHEDULING
    int has_strict_ipc_priorities = 1;
#else
    int has_strict_ipc_priorities = 0;
#endif
#ifdef CONFIG_SCHEDULE_INHERITANCE
    int has_schedule_inheritance = 1;
#else
    int has_schedule_inheritance = 0;
#endif
    int has_strict_scheduling = has_strict_ipc_priorities &&
            !has_schedule_inheritance;

    printf("\nIPC priority inversion test #%d finished: %d iteration(s):", eg_num, nb_iterations);
    printf("\n* High thread - successfull IPC with Medium thread (2 per iteration): %d/%d", cnt_h, 2*nb_iterations);
    printf("\n* Medium thread - successfull IPC with Low thread (2 per iteration): %d/%d, with High thread (2 per iteration): %d/%d", 
           cnt_m1, 2*nb_iterations, cnt_m2, 2*nb_iterations);
    printf("\n* Low thread - successfull IPC with Medium thread (2 per iteration): %d/%d", cnt_l, 2*nb_iterations);
    printf("\n* Intermediate thread 1 ran %d time(s)", cnt_i1);
    printf("\n* Intermediate thread 2 ran %d time(s)", cnt_i2);

    /* Ensure each thread ran the correct number of times. */
    if ((cnt_h < 2 * nb_iterations) || (cnt_m1 < 2 * nb_iterations) ||
            (cnt_m2 < 2 * nb_iterations) || (cnt_l < 2 * nb_iterations)) {
        failure = 1;
    }

    /* Ensure that the intermediate threads ran an appropriate number of
     * times, which changes depending on the scheduler in use. */
    if (has_schedule_inheritance) {
        /* Schedule inheritance is enabled. Intermediate priority threads
         * should not be run. Rather, the lower priority threads should
         * inherit the schedule of the higher threads. */
        if (cnt_i1 > 0 || cnt_i2 > 0) {
            failure = 1;
        }
    } else if (has_strict_scheduling) {
        /* Strict IPC priorities are enabled. Intermediate threads should
         * always be scheduled when sending an IPC down from a high thread
         * to a low. */
        if (cnt_i1 != 2 * nb_iterations || cnt_i2 != 2 * nb_iterations) {
            failure = 1;
        }
    }

    /* Display results. */
    if (failure) {
        nb_failed++;
        printf("\n--> Test failed!");
    }
    else
        nb_passed++;
    printf("\n");
    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_mutex_priority_inversion_results_impl(CORBA_Object _caller, int eg_num, int nb_iterations,
                                                  int cnt_h, int cnt_m1, int cnt_m2, int cnt_l, 
                                                  int cnt_i1, int cnt_i2, idl4_server_environment * _env)
{
    printf("\nMutex priority inversion test #%d finished: %d iteration(s)", eg_num, nb_iterations);
    printf("\n* High thread acquired mutex_1: %d/%d", cnt_h, nb_iterations);
    printf("\n* Medium thread acquired mutex_1: %d/%d, mutex_2: %d/%d", cnt_m1, nb_iterations, cnt_m2, nb_iterations);
    printf("\n* Low thread acquired mutex_2: %d/%d", cnt_l, nb_iterations);
    printf("\n* Intermediate thread 1 ran %d time(s)", cnt_i1);
    printf("\n* Intermediate thread 2 ran %d time(s)", cnt_i2);
    if ((cnt_h < nb_iterations) || (cnt_m1 < nb_iterations) || 
        (cnt_m2 < nb_iterations) || (cnt_l < nb_iterations) || cnt_i1 || cnt_i2) {
        nb_failed++;
        printf("\n--> Test failed!");
    }
    else
        nb_passed++;
    printf("\n");
    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_mixed_priority_inversion_results_impl(CORBA_Object _caller, int eg_num, int scenario, int nb_iterations,
                                                  int cnt_h, int cnt_m1, int cnt_m2, int cnt_l, int cnt_i1, 
                                                  int cnt_i2, idl4_server_environment * _env)
{
    if (scenario == 1) {
        printf("\nMixed priority inversion test #%d - 1st scenario (%d iteration(s)): Close IPC", eg_num, nb_iterations);
        printf("\n* High thread - messages sent successfully to Medium thread: %d/%d", cnt_h, nb_iterations);
        printf("\n* Medium thread acquired mutex: %d/%d; messages received successfully from High thread: %d/%d", cnt_m1, nb_iterations, cnt_m2, nb_iterations);
        printf("\n* Low thread acquired mutex: %d/%d", cnt_l, nb_iterations);
        if ((cnt_h < nb_iterations) || (cnt_m1 < nb_iterations) || 
            (cnt_m2 < nb_iterations) || (cnt_l < nb_iterations))
            mixed_pi_failure = 1;

#if defined(CONFIG_STRICT_SCHEDULING) || defined(CONFIG_SCHEDULE_INHERITANCE)
        if (cnt_i1 < nb_iterations)
            mixed_pi_failure = 1;
#endif
        if (mixed_pi_failure)
            printf("\n--> \"Close IPC\" scenario failed!");
        printf("\n");
    }
    if (scenario == 2) {
        printf("\nMixed priority inversion test #%d - 2nd scenario (%d iteration(s)): Open IPC", eg_num, nb_iterations);
        printf("\n* High thread acquired mutex: %d/%d", cnt_h, nb_iterations);
        printf("\n* Medium thread acquired mutex: %d/%d; messages received successfully from Low thread: %d/%d", cnt_m1, nb_iterations, cnt_m2, nb_iterations);
        printf("\n* Low thread - messages sent successfully to Medium thread: %d/%d", cnt_l, nb_iterations);
        if ((cnt_h < nb_iterations) || (cnt_m1 < nb_iterations) || 
            (cnt_m2 < nb_iterations) || (cnt_l < nb_iterations))
            mixed_pi_failure++;
#if defined(CONFIG_STRICT_SCHEDULING) || defined(CONFIG_SCHEDULE_INHERITANCE)
        if (cnt_i2 < nb_iterations)
            mixed_pi_failure++;
#endif
        if (mixed_pi_failure >= 2)
            printf("\n--> \"Open IPC\" scenario failed!");
        printf("\n");
        if (mixed_pi_failure)
            nb_failed++;

        remaining_tests--;
        if (remaining_tests == 0)
            print_end();
    }
}

void
rtos_report_sleeping_barber_results_impl(CORBA_Object _caller, int eg_num, int nb_customers, L4_Word_t nb_hair_cut,
                                         idl4_server_environment * _env)
{
    printf("\nSleeping barber test #%d finished: %d customer(s), %lu hair cut done", eg_num, nb_customers, nb_hair_cut);
    if (nb_customers == nb_hair_cut)
        nb_passed++;
    else {
        nb_failed++;
        printf("\n--> Test failed!");
    }
    printf("\n");

    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_report_producer_consumer_results_impl(CORBA_Object _caller, int eg_num, int nb_threads, int nb_item,
                                           int nb_loop, pc_results_t *pc_metrics, int pass, idl4_server_environment * _env)
{
    printf("\nProducer consumer test #%d finished (2 * %d thread(s)):", eg_num, nb_threads);
    printf("\n* Semaphore \"empty\" = %d\n* Semaphore \"full\" = %d\n* Total items consumed = %d, ", pc_metrics->empty, pc_metrics->full, pc_metrics->total);
    if (pass) {
        nb_passed++;
        printf("\n");
    } else {
        nb_failed++;
        printf("\n--> Test failed!");
    }
    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void
rtos_notify_test_died_impl(CORBA_Object _caller, char *test_name, int eg_num, 
                           idl4_server_environment * _env)
{
    printf("Error: %s test #%d died!\n", test_name, eg_num);
    nb_errors++;
    remaining_tests--;
    if (remaining_tests == 0)
        print_end();
}

void 
dummy_thread(int argc, char **argv)
{
    while (1) ;
}

int
main(int argc, char **argv)
{
    nb_passed = nb_failed = nb_errors = 0;
    if (argc) {
        nb_tests = atoi(argv[0]);
        remaining_tests = nb_tests;
    } else
        assert(!"rtos server error: argument missing!\n");
    printf("rtos server running: %d tests running\n", nb_tests);
    L4_KDB_SetThreadName(L4_Myself(), "rtoserv");
    rtos_server_loop();
    assert(!"Shouldn't reach here\n");
}
