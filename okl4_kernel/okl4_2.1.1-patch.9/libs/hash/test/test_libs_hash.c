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

#include <stdlib.h>
#include <stdio.h>
#include <hash/hash.h>
#include <check/check.h>
#include "test_libs_hash.h"

struct teststruct {
    int key;
    void *value;
};

struct teststruct simple_test_data[] = {
    {1, (void *)123},
    {2, (void *)456},
    {1024, (void *)1024},
    {8192, (void *)8192},
    {4962, (void *)929384},
    {0, 0}                      /* sentinel */
};

START_TEST(simple_hash_test)
{
    int counter, res;
    struct hashtable *table;

    table = hash_init(1024);
    fail_if(table == NULL, "hash_init return NULL");

    /*
     * Insert some numbers and read them back. 
     */
    for (counter = 0; simple_test_data[counter].key != 0; counter++) {
        res =
            hash_insert(table, simple_test_data[counter].key,
                        simple_test_data[counter].value);
        fail_if(res == -1, "hash_insert failed");
    }
    for (counter = 0; simple_test_data[counter].key != 0; counter++) {
        fail_unless(hash_lookup(table, simple_test_data[counter].key) ==
                    simple_test_data[counter].value,
                    "hash_lookup failed to find entry");
    }
    hash_free(table);
}
END_TEST

START_TEST(stress_test)
{
    uintptr_t counter;
    struct hashtable *table;
    int res;

    table = hash_init(1024);
    fail_if(table == NULL, "hash_init return NULL");
    /*
     * Insert heaps of numbers and ensure we can read them all back 
     */
    for (counter = 0; counter < 10240; counter++) {
        res = hash_insert(table, counter, (void *)~counter);
        if (res == -1) {
            /*
             * We have exhausted the heap so time to clean up 
             */
            break;
        }
        fail_unless(hash_lookup(table, counter) == (void *)~counter,
                    "hash_lookup failed in stress test");
    }
    hash_free(table);
}
END_TEST 

Suite *
make_test_libs_hash_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("Hash tests");
    tc = tcase_create("Core");
    tcase_add_test(tc, simple_hash_test);
    tcase_add_test(tc, stress_test);
    suite_add_tcase(suite, tc);
    return suite;
}
