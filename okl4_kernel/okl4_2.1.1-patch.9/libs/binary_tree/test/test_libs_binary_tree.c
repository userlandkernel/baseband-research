/*
 * Copyright (c) 2004, National ICT Australia
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
/*
 * Author: Ben Leslie
 * Created: Tue Nov 9 2004 
 */
#include <stddef.h>
#include <string.h>
#include <check/check.h>
#include "test_libs_binary_tree.h"

static struct bin_tree *
setup_balanced_tree(void)
{
    struct bin_tree *bin_tree;
    static int data[16];
    int i;

    for (i = 0; i < 16; i++)
        data[i] = i;

    bin_tree = binary_tree_new();
    fail_unless(binary_tree_insert(bin_tree, "h", &data[8]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "d", &data[4]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "l", &data[12]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "b", &data[2]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "f", &data[6]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "j", &data[10]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "n", &data[14]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "a", &data[1]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "c", &data[3]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "e", &data[5]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "g", &data[7]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "i", &data[9]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "k", &data[11]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "m", &data[13]) == 0,
                "Couldn't insert");
    fail_unless(binary_tree_insert(bin_tree, "o", &data[15]) == 0,
                "Couldn't insert");

    return bin_tree;
}

void
check_balanced_tree(struct bin_tree *bin_tree, char *chars)
{
    char key[2];
    int i;

    key[1] = '\0';
    for (i = 0; i < strlen(chars); i++) {
        key[0] = chars[i];
        fail_unless(*((int *)binary_tree_lookup(bin_tree, key)) ==
                    chars[i] - 'a' + 1, "Didn't lookup right value");
    }
}

void
check_notin_tree(struct bin_tree *bin_tree, char *chars)
{
    static char key[2];
    int i;

    key[1] = '\0';
    for (i = 0; i < strlen(chars); i++) {
        key[0] = chars[i];
        fail_unless(binary_tree_lookup(bin_tree, key) == 0,
                    "Found key that shouldn't be in tree");
    }
}

START_TEST(test_binary_tree_remove)
{
    struct bin_tree *bin_tree;
    int data = 1;

    bin_tree = binary_tree_new();

    /*
     * Simple case 
     */
    fail_unless(binary_tree_insert(bin_tree, "foo", &data) == 0,
                "Couldn't insert foo");
    fail_unless(*((int *)binary_tree_lookup(bin_tree, "foo")) == 1,
                "Didn't lookup right value");
    fail_unless(binary_tree_remove(bin_tree, "foo") == 0,
                "Didn't remove entry");
    fail_unless(binary_tree_lookup(bin_tree, "foo") == 0,
                "Still looked up value after remove");

    bin_tree = setup_balanced_tree();

    fail_unless(binary_tree_remove(bin_tree, "l") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "abcdefghijkmno");
    check_notin_tree(bin_tree, "l");

    fail_unless(binary_tree_remove(bin_tree, "k") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "abcdefghijmno");
    check_notin_tree(bin_tree, "lk");

    fail_unless(binary_tree_remove(bin_tree, "j") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "abcdefghimno");
    check_notin_tree(bin_tree, "lkj");

    fail_unless(binary_tree_remove(bin_tree, "i") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "abcdefghmno");
    check_notin_tree(bin_tree, "lkji");

    fail_unless(binary_tree_remove(bin_tree, "b") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "acdefghmno");
    check_notin_tree(bin_tree, "lkjib");

    fail_unless(binary_tree_remove(bin_tree, "h") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "acdefgmno");
    check_notin_tree(bin_tree, "lkjibh");

    fail_unless(binary_tree_remove(bin_tree, "d") == 0, "Didn't remove entry");
    check_balanced_tree(bin_tree, "acefgmno");
    check_notin_tree(bin_tree, "lkjibhd");
}
END_TEST

START_TEST(test_binary_tree_insert_duplicate)
{
    struct bin_tree *bin_tree;
    int data = 1;

    bin_tree = binary_tree_new();

    fail_unless(binary_tree_insert(bin_tree, "foo", &data) == 0,
                "Couldn't insert foo");
    fail_unless(binary_tree_insert(bin_tree, "foo", &data) != 0,
                "Could insert foo, shouldn't be able to");
}
END_TEST

START_TEST(test_binary_tree_insert)
{
    struct bin_tree *bin_tree;
    int data[4], i;

    for (i = 0; i < 4; i++)
        data[i] = i;

    bin_tree = binary_tree_new();

    fail_unless(binary_tree_insert(bin_tree, "foo", &data[1]) == 0,
                "Couldn't insert foo");
    fail_unless(binary_tree_insert(bin_tree, "bar", &data[2]) == 0,
                "Couldn't insert bar");
    fail_unless(binary_tree_insert(bin_tree, "baz", &data[3]) == 0,
                "Couldn't insert baz");

    fail_unless(*((int *)binary_tree_lookup(bin_tree, "foo")) == 1,
                "Didn't lookup right value");
    fail_unless(*((int *)binary_tree_lookup(bin_tree, "bar")) == 2,
                "Didn't lookup right value");
    fail_unless(*((int *)binary_tree_lookup(bin_tree, "baz")) == 3,
                "Didn't lookup right value");

}
END_TEST

START_TEST(test_binary_tree_basic)
{
    /*
     * TODO: Put your own test code here. 
     */
    fail_unless(1 == 1, "Reality is on the blink again.");
}
END_TEST 

Suite *
make_test_libs_binary_tree_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("binary_tree tests");
    tc = tcase_create("Core");
    tcase_add_test(tc, test_binary_tree_basic);
    /*
     * TODO: add other tests here 
     */
    /*
     * tcase_add_test(tc, test_binary_tree_new_test); 
     */
    tcase_add_test(tc, test_binary_tree_insert);
    tcase_add_test(tc, test_binary_tree_insert_duplicate);
    tcase_add_test(tc, test_binary_tree_remove);

    suite_add_tcase(suite, tc);
    return suite;
}
