/*
 * Copyright (c) 2005, National ICT Australia
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
#include <stdlib.h>
#include <l4/kdebug.h>
#include <l4/profile.h>
#include <l4/schedule.h>
#include <compat/c.h>
#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <l4test/bootinfo.h>

extern char * arch_excludes [];

/* Put any tests that crash the kernel in here to avoid running them
 * in regression */
/* to run these tests either remove them from this list or comment out
 * the line that passes the list to lib check below */
char * hanging_tests [] = {
    /*"kmem02",*/
//    "FUZZ0100",
//    "FUZZ0201",
//    "FUZZ0500",
//    "FUZZ0701",
//    "FUZZ0801",
//    "FUZZ0802",
    NULL};

#define TCASE(x) suite_add_tcase(suite, make_ ## x ## _tcase())

static Suite *
make_suite(void)
{
    Suite * suite;

    suite = suite_create("L4 Test suite");

    TCASE(kdb);
    TCASE(mapcontrol);
    TCASE(spacecontrol);
    TCASE(ipc);
    TCASE(xas_ipc);
    TCASE(aipc);
    TCASE(xas_aipc);
    TCASE(ipc_cpx);
    TCASE(thread);
    TCASE(cache);
    TCASE(caps);
    TCASE(virtual_register);
    TCASE(thread_id);
    TCASE(threadcontrol);
    TCASE(exchange_registers);
    TCASE(spaceswitch);
    TCASE(kmem);
    TCASE(fuzz);
    TCASE(schedule);
    TCASE(preempt);
    TCASE(cust);
    TCASE(mutex);
    TCASE(arch);
#if defined(CONFIG_SCHEDULE_INHERITANCE)
    TCASE(ipc_schedule_inheritance);
    TCASE(mutex_schedule_inheritance);
    TCASE(schedule_inheritance_graph);
#endif
    TCASE(interrupt_control);
#if defined(CONFIG_REMOTE_MEMORY_COPY)
    TCASE(remote_memcpy);
#endif

    return suite;
}

int
main(void)
{
    int n, ht_size=0, ax_size=0;
    SRunner * sr;
    char ** excluded_tests;
    int result;

    L4_Profiling_Reset();

    L4_KDB_SetSpaceName(L4_rootspace, "l4test");

    result = init_boot_params();
    if (result) {
        printf("Couldn't find boot parameters\n");
        exit(-1);
    }

    /* Lower our priority to 254, allowing system threads to override
     * us if necessary. */
    L4_Set_Priority(L4_Myself(), 254);

    /* Architecture-specific setup. */
    l4test_arch_setup();

    /* Construct a list of tests to be excluded */ 
    while (hanging_tests[ht_size++]);
    while (arch_excludes[ax_size++]);
    excluded_tests = malloc((ht_size + ax_size - 1) * sizeof(char *));
    for (n = 0; n < ht_size - 1; n++) {
        excluded_tests[n] = hanging_tests[n];
    }
    for (; n < ht_size + ax_size - 2; n++) {
        excluded_tests[n] = arch_excludes[n - ht_size + 1];
    }
    excluded_tests[n] = NULL;

    sr = srunner_create(make_suite());
    srunner_exceptions(sr, excluded_tests);
    srunner_set_fork_status(sr, CK_WITHPAGER);

    L4_Profiling_Start();

    srunner_run_all(sr, CK_VERBOSE);

    L4_Profiling_Stop();

    n = srunner_ntests_failed(sr);
    srunner_free(sr);
    free(excluded_tests);

    L4_Profiling_Print();

    L4_KDB_Enter("L4Test Done");
}

