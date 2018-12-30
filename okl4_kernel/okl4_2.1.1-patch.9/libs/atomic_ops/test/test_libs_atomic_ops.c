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
 * Author: Malcolm Purvis <malcolmp@ok-labs.com>
 *
 *
 * Allow some tests to run on machines that don't have the necessary
 * support by using the generic code.
 */

#include "test_libs_atomic_ops.h"
#include <atomic_ops/atomic_ops.h>
#include <stdio.h>

//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(...)
#endif

/* Check that behaviour is correct in a single thread. */
START_TEST(test_okl4_atomic_ops_semantics)
{
//  DEBUG_PRINT("Here\n");

    /* Mask with top and bottom bits of a word set. */
    static L4_Word_t top_bottom_bits;

    int i;
    int is_set;
    okl4_atomic_word_t data;
    okl4_atomic_word_t array[2];
    L4_Word_t result;
    static L4_Word_t constants[] = {
        0, 1, 2, 16, 64, 1024, 1024*1024
    };

    okl4_atomic_init(&data, 0);
    okl4_atomic_init(&array[0], 0);
    okl4_atomic_init(&array[1], 0);
    top_bottom_bits = ((1UL << (L4_BITS_PER_WORD - 1)) | 1UL);

    DEBUG_PRINT("Basic set and read.\n"); 
    for (i = 0; i < 7; i++) {
        okl4_atomic_set(&data, constants[i]);
        result = okl4_atomic_read(&data);

        _fail_unless(result == constants[i], __FILE__, __LINE__, 
                     "Set or Read failed: (was %ld, expected %ld)",
                     (long) result, (long) constants[i]);
    }

    DEBUG_PRINT("Bitwise operations.\n");
    okl4_atomic_set(&data, 255);
    okl4_atomic_and(&data, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_and failed (was %ld, expected %ld)", (long) result, 1L);
    
    okl4_atomic_set(&data, 2);
    okl4_atomic_and(&data, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_and failed (was %ld, expected %ld)", (long) result, 0L);

    okl4_atomic_set(&data, 0);
    okl4_atomic_or(&data, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_or failed (was %ld, expected %ld)", (long) result, 1L);
    
    okl4_atomic_or(&data, 2);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 3, __FILE__, __LINE__,
                 "okl4_atomic_or failed (was %ld, expected %ld)", (long) result, 3L);
    
    okl4_atomic_xor(&data, 3);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_xor failed (was %ld, expected %ld)", (long) result, 0L);
    

    DEBUG_PRINT("okl4_atomic_X_return: Test that the returned value is the same "
                "as the stored value.\n");
     
    okl4_atomic_set(&data, 255);
    result = okl4_atomic_and_return(&data, 1);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_and_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_and_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    
    okl4_atomic_set(&data, 2);
    result = okl4_atomic_and_return(&data, 1);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_and_return failed (was %ld, expected %ld)",
                 (long) result, 0L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_and_return failed (was %ld, expected %ld)",
                 (long) result, 0L);
    
    okl4_atomic_set(&data, 0);
    result = okl4_atomic_or_return(&data, 1);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_or_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_or_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    
    result = okl4_atomic_or_return(&data, 2);
    _fail_unless(result == 3, __FILE__, __LINE__,
                 "okl4_atomic_or_return failed (was %ld, expected %ld)",
                 (long) result, 3L);
    result = okl4_atomic_read(&data);
    DEBUG_PRINT("%d\n", 13);
    _fail_unless(result == 3, __FILE__, __LINE__,
                 "okl4_atomic_or_return failed (was %ld, expected %ld)",
                 (long) result, 3L);
    
    result = okl4_atomic_xor_return(&data, 3);
    DEBUG_PRINT("%d\n", 14);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_xor_return failed (was %ld, expected %ld)",
                 (long) result, 0L);
    result = okl4_atomic_read(&data);
    DEBUG_PRINT("%d\n", 15);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_xor_return failed (was %ld, expected %ld)",
                 (long) result, 0L);

    DEBUG_PRINT("Bitfield operations.\n");
    okl4_atomic_set(&array[0], 0);
    okl4_atomic_set(&array[1], 0);

    
    DEBUG_PRINT("Turn on the top and bottom bit of each word to check "
                "boundary conditions.\n");

    okl4_atomic_set_bit(array, 0);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 1L);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0L);

    okl4_atomic_set_bit(array, L4_BITS_PER_WORD - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 1L);

    okl4_atomic_set_bit(array, L4_BITS_PER_WORD);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
    /*             (long) result, 1L); */
                 (long) result, L4_BITS_PER_WORD);

    okl4_atomic_set_bit(array, (2 * L4_BITS_PER_WORD) - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_set_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    DEBUG_PRINT("Turn off the top and bottom bit of each word to check "
                "boundary conditions.\n");

    okl4_atomic_clear_bit(array, 0);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == (1UL << (L4_BITS_PER_WORD - 1)), __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, (1UL << (L4_BITS_PER_WORD - 1)));
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    okl4_atomic_clear_bit(array, L4_BITS_PER_WORD - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    okl4_atomic_clear_bit(array, L4_BITS_PER_WORD);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == (1UL << (L4_BITS_PER_WORD - 1)), __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, (1UL << (L4_BITS_PER_WORD - 1)));

    okl4_atomic_clear_bit(array, (2 * L4_BITS_PER_WORD) - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_clear_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);


    
    DEBUG_PRINT("Turn on the top and bottom bit of each word to check "
                "boundary conditions.\n");
     
    okl4_atomic_change_bit(array, 0);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 1L);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0L);

    okl4_atomic_change_bit(array, L4_BITS_PER_WORD - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 1L);

    okl4_atomic_change_bit(array, L4_BITS_PER_WORD);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 1L);

    okl4_atomic_change_bit(array, (2 * L4_BITS_PER_WORD) - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    
    DEBUG_PRINT("Turn off the top and bottom bit of each word to check "
                "boundary conditions.\n");
    
    okl4_atomic_change_bit(array, 0);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == (1UL << (L4_BITS_PER_WORD - 1)), __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, (1UL << (L4_BITS_PER_WORD - 1)));
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    okl4_atomic_change_bit(array, L4_BITS_PER_WORD - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == top_bottom_bits, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, top_bottom_bits);

    okl4_atomic_change_bit(array, L4_BITS_PER_WORD);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == (1UL << (L4_BITS_PER_WORD - 1)), __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, (1UL << (L4_BITS_PER_WORD - 1)));

    okl4_atomic_change_bit(array, (2 * L4_BITS_PER_WORD) - 1);
    result = okl4_atomic_read(&array[0]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);
    result = okl4_atomic_read(&array[1]);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_change_bit failed (was 0x%lx, expected 0x%lx)",
                 (long) result, 0);


    DEBUG_PRINT("Arithmetic operations.\n"); 

    okl4_atomic_set(&data, 0);
    okl4_atomic_add(&data, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_add failed (was %ld, expected %ld)", (long) result, 1L);
    
    okl4_atomic_sub(&data, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub failed (was %ld, expected %ld)", (long) result, 0L);

    DEBUG_PRINT("Add a value that does not fit as an ARM immediate value.\n"); 

    okl4_atomic_set(&data, 0);
    okl4_atomic_add(&data, 65535);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 65535, __FILE__, __LINE__,
                 "okl4_atomic_add failed (was %ld, expected %ld)", (long) result, 65535L);
    
    okl4_atomic_sub(&data, 65535);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub failed (was %ld, expected %ld)", (long) result, 0L);

    
    DEBUG_PRINT("okl4_atomic_X_return: Test that the returned value is the same "
                "as the stored value.\n");

    okl4_atomic_set(&data, 0);
    result = okl4_atomic_add_return(&data, 1);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_add_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_add_return failed (was %ld, expected %ld)",
                 (long) result, 1L);
    
    result = okl4_atomic_sub_return(&data, 1);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub_return failed (was %ld, expected %ld)",
                 (long) result, 0L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub_return failed (was %ld, expected %ld)",
                 (long) result, 0L);

    okl4_atomic_set(&data, 0);
    result = okl4_atomic_add_return(&data, 65535);
    _fail_unless(result == 65535, __FILE__, __LINE__,
                 "okl4_atomic_add_return failed (was %ld, expected %ld)",
                 (long) result, 65535L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 65535, __FILE__, __LINE__,
                 "okl4_atomic_add_return failed (was %ld, expected %ld)",
                 (long) result, 65535L);
    
    result = okl4_atomic_sub_return(&data, 65535);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub_return failed (was %ld, expected %ld)",
                 (long) result, 0L);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 0, __FILE__, __LINE__,
                 "okl4_atomic_sub_return failed (was %ld, expected %ld)",
                 (long) result, 0L);

    DEBUG_PRINT("General Operations.\n");

    okl4_atomic_set(&data, 0);

    is_set = okl4_atomic_compare_and_set(&data, 0, 1);
    _fail_unless(is_set, __FILE__, __LINE__,
                 "okl4_atomic_compare_and_set failed (was %d, expected %d)",
                 is_set, 1);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_compare_and_set failed (was %ld, expected %ld)",
                 (long) result, 1L);

    is_set = okl4_atomic_compare_and_set(&data, 0, 2);
    _fail_unless(!is_set, __FILE__, __LINE__,
                 "okl4_atomic_compare_and_set failed (was %d, expected %d)",
                 is_set, 0);
    result = okl4_atomic_read(&data);
    _fail_unless(result == 1, __FILE__, __LINE__,
                 "okl4_atomic_compare_and_set failed (was %ld, expected %ld)",
                 (long) result, 1L);

    
    DEBUG_PRINT("Memory barrier tests.\n");
    
    /* 
     * Since these don't return a value they're impossible to test
     * in a single thread, so mark a point before each call just
     * in case they crash.
     */
    mark_point();
    okl4_atomic_compiler_barrier();
    mark_point();
    okl4_atomic_barrier_write();
    mark_point();
    okl4_atomic_barrier_write_smp();
    mark_point();
    okl4_atomic_barrier_read();
    mark_point();
    okl4_atomic_barrier_read_smp();
    mark_point();
    mark_point();
    okl4_atomic_barrier();
    mark_point();
    okl4_atomic_barrier_smp();
}
END_TEST 

Suite *
make_test_libs_atomic_ops_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("okl4_atomic_ops tests");
    tc = tcase_create("Core");
    tcase_add_test(tc, test_okl4_atomic_ops_semantics);

    suite_add_tcase(suite, tc);
    return suite;
}
