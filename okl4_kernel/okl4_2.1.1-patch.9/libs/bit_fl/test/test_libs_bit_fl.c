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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <bit_fl/bit_fl.h>
#include "test_libs_bit_fl.h"

START_TEST(test_bfl_new)
{
    uintptr_t size = 8;
    bfl_t bfl = bfl_new(size);

    fail_if(bfl == NULL, "NULL bfl returned");
    fail_if(bfl->bitarray[0] != 0xff, "bfl not initialised properly");
    free(bfl);

}
END_TEST

START_TEST(test_bfl_small)
{
    uintptr_t size = 8, i, val;
    bfl_t bfl = bfl_new(size);

    fail_if(bfl == NULL, "newly allocate bfl is NULL");

    for (i = 0; i < size; i++) {
        val = bfl_alloc(bfl);
        fail_if(i != val, "allocated bit is out of order");
    }
    val = bfl_alloc(bfl);
    fail_if(val != -1, "allocated bit when the free list should be full");

    for (i = 0; i < size; i++) {
        bfl_free(bfl, i);
    }

    free(bfl);
}
END_TEST

START_TEST(test_bfl_medium)
{
    uintptr_t size = sizeof(bfl_word), i, val;
    bfl_t bfl = bfl_new(size);

    fail_if(bfl == NULL, "newly allocate bfl is NULL");

    for (i = 0; i < size; i++) {
        val = bfl_alloc(bfl);
        fail_if(i != val, "allocated bit is out of order");
    }
    val = bfl_alloc(bfl);
    fail_if(val != -1, "allocated bit when the free list should be full");

    for (i = 0; i < size; i++) {
        bfl_free(bfl, i);
    }

    free(bfl);
}
END_TEST

START_TEST(test_bfl_large)
{
    uintptr_t size = 4 * sizeof(bfl_word), i, val;
    bfl_t bfl = bfl_new(size);

    fail_if(bfl == NULL, "newly allocate bfl is NULL");

    for (i = 0; i < size; i++) {
        val = bfl_alloc(bfl);
        fail_if(i != val, "allocated bit is out of order");
    }
    val = bfl_alloc(bfl);
    fail_if(val != -1, "allocated bit when the free list should be full");

    for (i = 0; i < size; i++) {
        bfl_free(bfl, i);
    }

    free(bfl);
}
END_TEST

START_TEST(test_bfl_free)
{
    uintptr_t size = 32;
    uintptr_t a, b, c, d;

    bfl_t bfl = bfl_new(size);

    fail_if(bfl == NULL, "NULL bfl returned");
    fail_if(bfl->bitarray[0] != 0xffffffff, "bfl not initialised properly");

    a = bfl_alloc(bfl);
    b = bfl_alloc(bfl);
    c = bfl_alloc(bfl);
    d = bfl_alloc(bfl);

    bfl_free(bfl, c);
    fail_if(bfl->bitarray[0] != 0xfffffff4, "value not freed properly");
    bfl_free(bfl, a);
    fail_if(bfl->bitarray[0] != 0xfffffff5, "value not freed properly");
    bfl_free(bfl, b);
    fail_if(bfl->bitarray[0] != 0xfffffff7, "value not freed properly");
    bfl_free(bfl, d);
    fail_if(bfl->bitarray[0] != 0xffffffff, "value not freed properly");

    free(bfl);

}
END_TEST 

Suite *
make_test_libs_bit_fl_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("BFL tests");

    tc = tcase_create("bfl_new tests");
    tcase_add_test(tc, test_bfl_new);
    tcase_add_test(tc, test_bfl_small);
    tcase_add_test(tc, test_bfl_medium);
    tcase_add_test(tc, test_bfl_large);
    tcase_add_test(tc, test_bfl_free);
    suite_add_tcase(suite, tc);

    return suite;
}
