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
#include <queue/slist.h>
#include "test_libs_queue.h"

/* 
 * Singly-linked list head initialisation, SLIST_FIRST() and SLIST_EMPTY()
 */
START_TEST(SLIST01)
{
    SLIST_HEAD(listhead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    SLIST_INIT(&slisthead);
    fail_unless(SLIST_FIRST(&slisthead) == NULL,
                "The singly-linked list was not initialised properly or SLIST_FIRST() failed");
    fail_unless(SLIST_EMPTY(&slisthead) == 1,
                "The singly-linked list was not initialised properly or SLIST_EMPTY() failed");
}
END_TEST

/* 
 * Insert at the head. Check SLIST_NEXT().
 */
START_TEST(SLIST02)
{
    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm1, *elm2;

    SLIST_INIT(&slisthead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm1, links);
    fail_unless(SLIST_FIRST(&slisthead) == elm1,
                "Element was not inserted at the head of the singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead)->data == 1,
                "Wrong value in head of singly-linked list");
    fail_unless((elm1)->links.sle_next == NULL,
                "Pointer to next element was not set to NULL");

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm2, links);
    fail_unless(SLIST_FIRST(&slisthead) == elm2,
                "Element was not inserted at the head of the singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead)->data == 2,
                "Wrong value in head of singly-linked list");
    fail_unless((elm2)->links.sle_next == elm1,
                "Pointer to next element does not point to element1");
    fail_unless(SLIST_NEXT(elm2, links) == elm1,
                "SLIST_NEXT() returned bad pointer");

    free(elm1);
    free(elm2);
}
END_TEST

/* 
 * Insert after
 */
START_TEST(SLIST03)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    SLIST_INIT(&slisthead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    SLIST_INSERT_AFTER(&slisthead, elm1, elm2, links);
    fail_unless((elm1)->links.sle_next == elm2,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.sle_next == NULL,
                "Element2 pointer to next element was not set to NULL");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    SLIST_INSERT_AFTER(&slisthead, elm1, elm3, links);
    fail_unless((elm1)->links.sle_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.sle_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = SLIST_FIRST(&slisthead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.sle_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in singly-linked list");
        fail_unless((elm_tmp)->data == check[iter],
                    "Wrong value in singly-linked list");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Forward traversal
 */
START_TEST(SLIST04)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm, *elm_var;

    SLIST_INIT(&slisthead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm, links);

    SLIST_FOREACH(elm_var, &slisthead, links) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in singly-linked list");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in singly-linked list");
        iter++;
    }
    // printf("\n");

    elm = SLIST_FIRST(&slisthead);
    while (elm != NULL) {
        elm_var = SLIST_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Forward traversal safe
 */
START_TEST(SLIST05)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm, *elm_var, *elm_tmp;

    SLIST_INIT(&slisthead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm, links);

    SLIST_FOREACH_SAFE(elm_var, &slisthead, links, elm_tmp) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in singly-linked list");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in singly-linked list");
        if ((elm_var)->data == 7) {
            SLIST_REMOVE(&slisthead, elm_var, node_t, links);
            free(elm_var);
        }
        iter++;
    }
    // printf("\n");

    elm = SLIST_FIRST(&slisthead);
    while (elm != NULL) {
        elm_tmp = SLIST_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Deletion from the head
 */
START_TEST(SLIST06)
{
    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm;

    SLIST_INIT(&slisthead);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm1, links);

    elm = SLIST_FIRST(&slisthead);
    SLIST_REMOVE_HEAD(&slisthead, links);
    free(elm);

    fail_if(SLIST_FIRST(&slisthead) == elm1,
            "Element1 has not been removed from head of the singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead) == elm2,
                "Element was not properly removed from head of the singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead)->data == 2,
                "Wrong value in singly-linked list");
}
END_TEST

/* 
 * Element deletion
 */
START_TEST(SLIST07)
{
    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    SLIST_INIT(&slisthead);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    SLIST_INSERT_HEAD(&slisthead, elm3, links);
    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm1, links);

    SLIST_REMOVE(&slisthead, elm2, node_t, links);
    free(elm2);
    fail_if(SLIST_FIRST(&slisthead) == NULL, NULL);
    fail_if(SLIST_FIRST(&slisthead)->links.sle_next == elm2,
            "Element2 has not been removed from singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead)->links.sle_next == elm3,
                "Wrong element in singly-linked list");
    fail_unless(SLIST_FIRST(&slisthead)->links.sle_next->data == 3,
                "Wrong value in singly-linked list");

    free(elm1);
    free(elm3);
}
END_TEST

/* 
 * Singly-linked list deletion
 */
START_TEST(SLIST08)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    SLIST_HEAD(slisthead_t, node_t) slisthead = SLIST_HEAD_INITIALIZER(slisthead);
    struct node_t {
        int data;
         SLIST_ENTRY(node_t) links;
    } *elm;

    SLIST_INIT(&slisthead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    SLIST_INSERT_HEAD(&slisthead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    SLIST_INSERT_HEAD(&slisthead, elm, links);

    while (!SLIST_EMPTY(&slisthead)) {
        elm = SLIST_FIRST(&slisthead);
        if ((elm)->data == check[iter]) {
            iter++;
        }
        SLIST_REMOVE(&slisthead, elm, node_t, links);
        free(elm);
    }
    fail_unless(slisthead.slh_first == NULL,
                "Singly-linked list is not empty after removing all");
    fail_unless(check[iter] == -1,
                "Singly-linked list deletion has not removed all elements");
}
END_TEST

TCase *
slist_tests()
{
    TCase *tc;

    tc = tcase_create("Singly-linked list tests");

    tcase_add_test(tc, SLIST01);
    tcase_add_test(tc, SLIST02);
    tcase_add_test(tc, SLIST03);
    tcase_add_test(tc, SLIST04);
    tcase_add_test(tc, SLIST05);
    tcase_add_test(tc, SLIST06);
    tcase_add_test(tc, SLIST07);
    tcase_add_test(tc, SLIST08);

    return tc;
}
