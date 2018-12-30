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
 * Author: Guillaume Mulko
 * Created: Sun Oct 14 2007 
 */

#include <rtos/rtos.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iguana/object.h>
#include <iguana/env.h>

#include <interfaces/rtos_client.h>

object_t *rtos_obj;

void
rtos_init(void)
{
    objref_t rtos_ref;

    rtos_ref = (objref_t) env_memsection_base(iguana_getenv("OKL4_RTOS_TESTS_SERVER"));

    if (rtos_ref == 0UL || rtos_ref == -1UL) {
        assert(!"Could not find ig_rtos.");
    }

    rtos_obj = object_get_interface(rtos_ref);

    assert(rtos_obj != NULL);
}

void
rtos_cleanup(void)
{
}

void
test_died(char *test_name, int eg_num)
{
    rtos_notify_test_died(rtos_obj->server,
                          test_name, eg_num, NULL);
}

void
dining_philosophers_results(int eg_num, int nb_philosophers, int nb_dinners)
{
    rtos_report_dining_philosophers_results(rtos_obj->server,
                                            eg_num, nb_philosophers, nb_dinners, NULL);
}

void mapping_results(int eg_num, int nb_threads, int nb_mapped_memsection, 
                     int nb_corrupted_memsection, int nb_creation_failure, int total_size_mapped)
{
    rtos_report_mapping_results(rtos_obj->server, eg_num, nb_threads,
                                nb_mapped_memsection, nb_corrupted_memsection, 
                                nb_creation_failure, total_size_mapped, NULL);
}

void mapping_nomem_results(int eg_num, int nb_threads, int nb_mapped_memsection, int nb_corrupted_memsection, 
                           int nb_creation_failure, int nb_pd_creation_failure, int total_size_mapped)
{
    rtos_report_mapping_nomem_results(rtos_obj->server, eg_num, nb_threads,nb_mapped_memsection, 
                                nb_corrupted_memsection, nb_creation_failure, nb_pd_creation_failure, 
                                total_size_mapped, NULL);
}

void
ipc_priority_inversion_results(int eg_num, int nb_iterations, int cnt_h, int cnt_m1, 
                               int cnt_m2, int cnt_l, int cnt_i1, int cnt_i2)
{
    rtos_report_ipc_priority_inversion_results(rtos_obj->server, eg_num, nb_iterations,
                                               cnt_h, cnt_m1, cnt_m2, cnt_l, cnt_i1, cnt_i2, NULL);
}

void
mutex_priority_inversion_results(int eg_num, int nb_iterations, int cnt_h, int cnt_m1, 
                                 int cnt_m2, int cnt_l, int cnt_i1, int cnt_i2)
{
    rtos_report_mutex_priority_inversion_results(rtos_obj->server, eg_num, nb_iterations,
                                                 cnt_h, cnt_m1, cnt_m2, cnt_l, cnt_i1, cnt_i2, NULL);
}

void 
mixed_priority_inversion_results(int eg_num, int scenario, int nb_iterations,
                                 int cnt_h, int cnt_m1, int cnt_m2, int cnt_l, 
                                 int cnt_i1, int cnt_i2)
{
    rtos_report_mixed_priority_inversion_results(rtos_obj->server, eg_num, scenario, nb_iterations, 
                                                 cnt_h, cnt_m1, cnt_m2, cnt_l, cnt_i1, cnt_i2, NULL);
}

void
sleeping_barber_results(int eg_num, int nb_customers, L4_Word_t nb_hair_cut)
{
    rtos_report_sleeping_barber_results(rtos_obj->server, eg_num, nb_customers, nb_hair_cut, NULL);
}

void 
producer_consumer_results(int eg_num, int nb_threads, int nb_item, int nb_loop,
                          pc_results_t *pc_metrics, int pass)
{
    rtos_report_producer_consumer_results(rtos_obj->server, eg_num, nb_threads, nb_item, 
                                          nb_loop, pc_metrics, pass, NULL);
}
