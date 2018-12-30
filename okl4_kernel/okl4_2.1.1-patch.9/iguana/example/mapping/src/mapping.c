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
#include <string.h>
#include <assert.h>

#include <iguana/thread.h>
#include <iguana/pd.h>
#include <iguana/memsection.h>
#include <iguana/env.h>
#include <l4/kdebug.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/config.h>
#include <rtos/rtos.h>

/** @todo: FIXME: Mothra Issue #2693 - nhannah */
#define MAX_LOOP 40

static volatile L4_ThreadId_t map_main;
static volatile int libs_ready = 0;
static volatile int nb_mapped_memsection = 0;
static volatile int nb_corrupted_memsection = 0;
static volatile int nb_creation_failure = 0;
static volatile int total_size_mapped = 0; // in bytes

static char *test_name = "Mapping";

void mapping_thread(int argc, char **argv);

/*
 * The mapping thread maps memsection of variable size, writes to it, tries to read the data back
 * and then deletes the mesection.
 */
void
mapping_thread(int argc, char **argv)
{
    memsection_ref_t mem;
    uintptr_t base, rand_val, *w, *r;
    int loop = MAX_LOOP;
    L4_Word_t page_size, page_size_mask;
    L4_MsgTag_t tag = L4_Niltag;

    while (!libs_ready) ;
    // Notify main thread and then wait to be woken up
    L4_LoadMR(0, 0);
    L4_Call(map_main);
    page_size_mask = L4_GetPageMask();
    page_size = 0x400;
    while (loop) {
        if (page_size >= ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)))
            page_size = 0x400;
        // If size is not valid we skip it.
        if (!(page_size & page_size_mask)) {
            page_size <<= 1;
            continue;
        }
        mem = memsection_create(page_size, &base);
        if (mem && base) {
            // Write a random value and try to read it back.
            w = (uintptr_t *)base;
            rand_val = (uintptr_t)rand();
            *w = rand_val;
            r = (uintptr_t *)base;
            if (*r != rand_val) {
                //printf("Mapping thread %lx/%lx: Memsection corrupted!\n", L4_Myself().raw, map_main.raw);
                nb_corrupted_memsection++;
            }
            memsection_delete(mem);
            loop--;
            nb_mapped_memsection++;
            total_size_mapped += page_size;
            page_size <<= 1;
            if (loop % (MAX_LOOP/3) == 0) {
                //printf("Mapping thread %lx/%lx created and deleted %d memsections\n", L4_Myself().raw, map_main.raw, MAX_LOOP - loop);
                // Tell main to start another mapping thread.
                L4_Set_Label(&tag, 0xadd);
                L4_Set_MsgTag(tag);
                L4_Send(map_main);
            }
        } else {
            //printf("Mapping thread %lx/%lx: Failed to create memsection!\n", L4_Myself().raw, map_main.raw);
            nb_creation_failure++;
        }
    }

    // Notify main thread we are done.
    L4_Set_Label(&tag, 0xdead);
    L4_Set_MsgTag(tag);
    L4_Call(map_main);
}

int
main(int argc, char **argv)
{
    L4_ThreadId_t any_thread, *mapping_threads_tids;
    L4_Word_t tmp, label, me;
    int i, j, eg_num, max_threads, server_on;
    L4_MsgTag_t tag = L4_Niltag;

    /*** Initialisation ***/
    map_main = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    me = map_main.raw;
    // Tell other threads that it is safe to use libraries
    libs_ready = 1;
    eg_num = max_threads = server_on = 0;
    if (argc == 3) {
        eg_num = atoi(argv[0]);
        max_threads = atoi(argv[1]);
        server_on = atoi(argv[2]);
    } else {
        printf("(%s 0x%lx) Error: Argument(s) missing!\n", test_name, me);
        return 1;
    }

    if (!server_on)
        printf("Start %s test #%d(0x%lx)\n", test_name, eg_num, me);
    // Retrieve mapping thread Ids
    mapping_threads_tids = malloc(max_threads * sizeof(L4_ThreadId_t));
    for (i = 0; i < max_threads; i++) {
        tag = L4_Wait(&any_thread);
        mapping_threads_tids[i] = any_thread;
    }
    // Start only the first mapping thread
    j = 0;
    if (max_threads) {
        L4_LoadMR(0, 0);
        L4_Send(mapping_threads_tids[j]);
    }

    /*** Start test ***/
    tmp = max_threads;
    while (tmp) {
        tag = L4_Wait(&any_thread);
        label = L4_Label(tag);
        if (label == 0xdead) {
            for (i = 0; i < max_threads; i++) {
                if (L4_IsThreadEqual(any_thread, mapping_threads_tids[i])) {
                    thread_delete(mapping_threads_tids[i]);
                    mapping_threads_tids[i] = L4_nilthread;
                    tmp--;
                    break;
                }
            }
        } else if (label == 0xadd) {
            // Start another mapping thread if available
            if (j < (max_threads - 1)) {
                L4_LoadMR(0, 0);
                L4_Send(mapping_threads_tids[++j]);
                L4_Send(mapping_threads_tids[++j]);
            }
        }
    }
    /*** Test finished ***/
    free(mapping_threads_tids);

    // If RTOS server is on, report results to it.
    if (server_on) {
        rtos_init();
        mapping_results(eg_num, max_threads, nb_mapped_memsection, 
                        nb_corrupted_memsection, nb_creation_failure, total_size_mapped); 
        rtos_cleanup();
    } else
        printf("%s test #%d(0x%lx) finished\n", test_name, eg_num, me);

    return 0;
}
