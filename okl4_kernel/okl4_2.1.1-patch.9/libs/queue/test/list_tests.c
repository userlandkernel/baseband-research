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

#include <stdlib.h>
#include <stdio.h>
#include <queue/list.h>
#include "test_libs_queue.h"

/* 
 * List head initialisation, LIST_FIRST() and LIST_EMPTY()
 */
START_TEST(LIST01)
{
    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    LIST_INIT(&listhead);
    fail_unless(LIST_FIRST(&listhead) == NULL,
                "The list was not initialised properly or LIST_FIRST failed");
    fail_unless(LIST_EMPTY(&listhead) == 1,
                "The list was not initialised properly or LIST_EMPTY failed");
}
END_TEST

/* 
 * Insert at the head. Check LIST_NEXT().
 */
START_TEST(LIST02)
{
    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm1, *elm2;

    LIST_INIT(&listhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm1, links);
    fail_unless(LIST_FIRST(&listhead) == elm1,
                "Element was not inserted at the head of the list");
    fail_unless(LIST_FIRST(&listhead)->data == 1,
                "Wrong value in head of list");
    fail_unless((elm1)->links.le_prev == NULL,
                "Pointer to previous element was not set to NULL");

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    LIST_INSERT_HEAD(&listhead, elm2, links);
    fail_unless(LIST_FIRST(&listhead) == elm2,
                "Element was not inserted at the head of the list");
    fail_unless(LIST_FIRST(&listhead)->data == 2,
                "Wrong value in head of list");
    fail_unless((elm2)->links.le_prev == NULL,
                "Pointer to previous element was not set to NULL");
    fail_unless((elm2)->links.le_next == elm1,
                "Pointer to next element does not point to element1");
    fail_unless(LIST_NEXT(elm2, links) == elm1,
                "LIST_NEXT() returned bad pointer");

    free(elm1);
    free(elm2);
}
END_TEST

/* 
 * Insert after
 */
START_TEST(LIST03)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    LIST_INIT(&listhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    LIST_INSERT_AFTER(&listhead, elm1, elm2, links);
    fail_unless((elm1)->links.le_next == elm2,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.le_prev == elm1,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.le_next == NULL,
                "Element2 pointer to next element was not set to NULL");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    LIST_INSERT_AFTER(&listhead, elm1, elm3, links);
    fail_unless((elm1)->links.le_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.le_prev == elm1,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.le_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = LIST_FIRST(&listhead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.le_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in list");
        fail_unless((elm_tmp)->data == check[iter], "Wrong value in list");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert before
 */
START_TEST(LIST04)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    LIST_INIT(&listhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    LIST_INSERT_AFTER(&listhead, elm1, elm2, links);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    LIST_INSERT_BEFORE(&listhead, elm2, elm3, links);
    fail_unless((elm1)->links.le_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.le_prev == elm1,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.le_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = LIST_FIRST(&listhead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.le_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in list");
        fail_unless((elm_tmp)->data == check[iter], "Wrong value in list");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Forward traversal
 */
START_TEST(LIST05)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm, *elm_var;

    LIST_INIT(&listhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm, links);

    LIST_FOREACH(elm_var, &listhead, links) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in list");
        fail_unless((elm_var)->data == check[iter], "Wrong value in list");
        iter++;
    }
    // printf("\n");

    elm = LIST_FIRST(&listhead);
    while (elm != NULL) {
        elm_var = LIST_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Forward traversal safe
 */
START_TEST(LIST06)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm, *elm_var, *elm_tmp;

    LIST_INIT(&listhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm, links);

    LIST_FOREACH_SAFE(elm_var, &listhead, links, elm_tmp) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in list");
        fail_unless((elm_var)->data == check[iter], "Wrong value in list");
        if ((elm_var)->data == 7) {
            LIST_REMOVE(&listhead, elm_var, links);
            free(elm_var);
        }
        iter++;
    }
    // printf("\n");

    elm = LIST_FIRST(&listhead);
    while (elm != NULL) {
        elm_tmp = LIST_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Element deletion
 */
START_TEST(LIST07)
{
    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    LIST_INIT(&listhead);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    LIST_INSERT_HEAD(&listhead, elm3, links);
    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    LIST_INSERT_HEAD(&listhead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm1, links);

    LIST_REMOVE(&listhead, elm2, links);
    free(elm2);
    fail_if(LIST_FIRST(&listhead) == NULL, NULL);
    fail_if(LIST_FIRST(&listhead)->links.le_next == elm2,
            "Element2 has not been removed from list");
    fail_unless(LIST_FIRST(&listhead)->links.le_next == elm3,
                "Wrong element in list");
    fail_unless((elm3)->links.le_prev == LIST_FIRST(&listhead),
                "Wrong element in list");
    fail_unless(LIST_FIRST(&listhead)->links.le_next->data == 3,
                "Wrong value in list");

    free(elm1);
    free(elm3);
}
END_TEST

/* 
 * List deletion
 */
START_TEST(LIST08)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    LIST_HEAD(listhead_t, node_t) listhead = LIST_HEAD_INITIALIZER(listhead);
    struct node_t {
        int data;
         LIST_ENTRY(node_t) links;
    } *elm;

    LIST_INIT(&listhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    LIST_INSERT_HEAD(&listhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    LIST_INSERT_HEAD(&listhead, elm, links);

    while (!LIST_EMPTY(&listhead)) {
        elm = LIST_FIRST(&listhead);
        if ((elm)->data == check[iter]) {
            iter++;
        }
        LIST_REMOVE(&listhead, elm, links);
        free(elm);
    }
    fail_unless(listhead.lh_first == NULL,
                "List is not empty after removing all");
    fail_unless(check[iter] == -1,
                "List deletion has not removed all elements");
}
END_TEST

TCase *
list_tests()
{
    TCase *tc;

    tc = tcase_create("List tests");

    tcase_add_test(tc, LIST01);
    tcase_add_test(tc, LIST02);
    tcase_add_test(tc, LIST03);
    tcase_add_test(tc, LIST04);
    tcase_add_test(tc, LIST05);
    tcase_add_test(tc, LIST06);
    tcase_add_test(tc, LIST07);
    tcase_add_test(tc, LIST08);

    return tc;
}
