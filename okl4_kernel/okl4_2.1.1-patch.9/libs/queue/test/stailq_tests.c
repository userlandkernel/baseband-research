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
#include <queue/stailq.h>
#include "test_libs_queue.h"

/* 
 * Singly-linked tail queue head initialisation. Check STAILQ_FIRST(), STAILQ_LAST() and STAILQ_EMPTY().
 */
START_TEST(STAILQ01)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    STAILQ_INIT(&stailqhead);
    fail_unless(STAILQ_FIRST(&stailqhead) == NULL,
                "The singly-linked tail queue was not initialised properly or STAILQ_FIRST failed");
    fail_unless(STAILQ_LAST(&stailqhead) == NULL,
                "The singly-linked tail queue was not initialised properly or STAILQ_LAST failed");
    fail_unless(STAILQ_EMPTY(&stailqhead) == 1,
                "The singly-linked tail queue was not initialised properly or STAILQ_EMPTY failed");
}
END_TEST

/* 
 * Insert at the head in an empty singly-linked tail queue and then insert at the tail. 
 * Check STAILQ_NEXT().
 */
START_TEST(STAILQ02)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    STAILQ_INIT(&stailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm1, links);
    fail_unless(STAILQ_FIRST(&stailqhead) == elm1,
                "Element was not inserted at the head of the singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->data == 1,
                "Wrong value in head of singly-linked tail queue");
    fail_unless(STAILQ_LAST(&stailqhead) == elm1,
                "The tail has not been updated");
    fail_unless(STAILQ_LAST(&stailqhead)->data == 1,
                "Wrong value in tail of singly-linked tail queue");
    fail_unless((elm1)->links.stqe_next == NULL,
                "Pointer to next element was not set to NULL");

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm2, links);
    fail_unless(STAILQ_FIRST(&stailqhead) == elm2,
                "Element was not inserted at the head of the singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->data == 2,
                "Wrong value in head of singly-linked tail queue");
    fail_unless((elm2)->links.stqe_next == elm1,
                "Pointer to next element does not point to element1");
    fail_unless(STAILQ_NEXT(elm2, links) == elm1,
                "STAILQ_NEXT() returned bad pointer");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    STAILQ_INSERT_TAIL(&stailqhead, elm3, links);
    fail_unless(STAILQ_LAST(&stailqhead) == elm3,
                "Element was not inserted at the tail of the singly-linked tail queue");
    fail_unless(STAILQ_LAST(&stailqhead)->data == 3,
                "Wrong value in tail of singly-linked tail queue");
    fail_unless((elm3)->links.stqe_next == NULL,
                "Pointer to next element was not set to NULL");

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert at the tail in an empty singly-linked tail queue and then insert at the head. 
 */
START_TEST(STAILQ03)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    STAILQ_INIT(&stailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    STAILQ_INSERT_TAIL(&stailqhead, elm1, links);
    fail_unless(STAILQ_LAST(&stailqhead) == elm1,
                "Element was not inserted at the tail of the singly-linked tail queue");
    fail_unless(STAILQ_LAST(&stailqhead)->data == 1,
                "Wrong value in tail of singly-linked tail queue");
    fail_unless((elm1)->links.stqe_next == NULL,
                "Pointer to next element was not set to NULL");
    fail_unless(STAILQ_FIRST(&stailqhead) == elm1,
                "The head has not been updated");
    if (STAILQ_FIRST(&stailqhead) != NULL) {
        fail_unless(STAILQ_FIRST(&stailqhead)->data == 1,
                    "Wrong value in head of singly-linked tail queue");
    }

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    STAILQ_INSERT_TAIL(&stailqhead, elm2, links);
    fail_unless(STAILQ_LAST(&stailqhead) == elm2,
                "Element was not inserted at the tail of the singly-linked tail queue");
    fail_unless(STAILQ_LAST(&stailqhead)->data == 2,
                "Wrong value in singly-linked tail queue");
    fail_unless((elm2)->links.stqe_next == NULL,
                "Pointer to next element was not set to NULL");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    STAILQ_INSERT_HEAD(&stailqhead, elm3, links);
    fail_unless(STAILQ_FIRST(&stailqhead) == elm3,
                "Element was not inserted at the head of the singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->data == 3,
                "Wrong value in head of singly-linked tail queue");
    fail_unless((elm3)->links.stqe_next == elm1,
                "Pointer to next element does not point to element1");

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert after
 */
START_TEST(STAILQ04)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    STAILQ_INIT(&stailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    STAILQ_INSERT_AFTER(&stailqhead, elm1, elm2, links);
    fail_unless((elm1)->links.stqe_next == elm2,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.stqe_next == NULL,
                "Element2 pointer to next element was not set to NULL");
    fail_unless(STAILQ_LAST(&stailqhead) == elm2,
                "Pointer to last element of the singly-linked tail queue does not point to element2");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    STAILQ_INSERT_AFTER(&stailqhead, elm1, elm3, links);
    fail_unless((elm1)->links.stqe_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.stqe_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = STAILQ_FIRST(&stailqhead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.stqe_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in singly-linked tail queue");
        fail_unless((elm_tmp)->data == check[iter],
                    "Wrong value in singly-linked tail queue");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Forward traversal
 */
START_TEST(STAILQ05)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm, *elm_var;

    STAILQ_INIT(&stailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);

    STAILQ_FOREACH(elm_var, &stailqhead, links) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in singly-linked tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in singly-linked tail queue");
        iter++;
    }
    // printf("\n");

    elm = STAILQ_FIRST(&stailqhead);
    while (elm != NULL) {
        elm_var = STAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Forward traversal safe
 */
START_TEST(STAILQ06)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm, *elm_var, *elm_tmp;

    STAILQ_INIT(&stailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);

    STAILQ_FOREACH_SAFE(elm_var, &stailqhead, links, elm_tmp) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in singly-linked tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in singly-linked tail queue");
        if ((elm_var)->data == 7) {
            STAILQ_REMOVE(&stailqhead, elm_var, node_t, links);
            free(elm_var);
        }
        iter++;
    }
    // printf("\n");

    elm = STAILQ_FIRST(&stailqhead);
    while (elm != NULL) {
        elm_tmp = STAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Deletion from the head
 */
START_TEST(STAILQ07)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm;

    STAILQ_INIT(&stailqhead);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm1, links);

    elm = STAILQ_FIRST(&stailqhead);
    STAILQ_REMOVE_HEAD(&stailqhead, links);
    free(elm);

    fail_if(STAILQ_FIRST(&stailqhead) == elm1,
            "Element1 has not been removed from head of the singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead) == elm2,
                "Element was not properly removed from head of the singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->data == 2,
                "Wrong value in singly-linked tail queue");

    free(elm2);
}
END_TEST

/* 
 * Element deletion
 */
START_TEST(STAILQ08)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    STAILQ_INIT(&stailqhead);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    STAILQ_INSERT_HEAD(&stailqhead, elm3, links);
    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm1, links);

    STAILQ_REMOVE(&stailqhead, elm2, node_t, links);
    free(elm2);
    fail_if(STAILQ_FIRST(&stailqhead) == NULL, NULL);
    fail_if(STAILQ_FIRST(&stailqhead)->links.stqe_next == elm2,
            "Element2 has not been removed from singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->links.stqe_next == elm3,
                "Wrong element in singly-linked tail queue");
    fail_unless(STAILQ_FIRST(&stailqhead)->links.stqe_next->data == 3,
                "Wrong value in singly-linked tail queue");

    free(elm1);
    free(elm3);
}
END_TEST

/* 
 * Singly-linked tail queue deletion
 */
START_TEST(STAILQ09)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    STAILQ_HEAD(stailqhead_t, node_t) stailqhead =
        STAILQ_HEAD_INITIALIZER(stailqhead);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm;

    STAILQ_INIT(&stailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead, elm, links);

    while (!STAILQ_EMPTY(&stailqhead)) {
        elm = STAILQ_FIRST(&stailqhead);
        if ((elm)->data == check[iter]) {
            iter++;
        }
        STAILQ_REMOVE(&stailqhead, elm, node_t, links);
        free(elm);
    }
    fail_unless(stailqhead.stqh_first == NULL,
                "Singly-linked tail queue is not empty after removing all");
    fail_unless(check[iter] == -1,
                "Singly-linked tail queue deletion has not removed all elements");
}
END_TEST

/* 
 * Concatenate an empty singly-linked tail queue to a non empty singly-linked tail queue.
 */
START_TEST(STAILQ10)
{
    STAILQ_HEAD(stailqhead_t, node_t) stailqhead1 =
        STAILQ_HEAD_INITIALIZER(stailqhead1);
    struct stailqhead_t stailqhead2 = STAILQ_HEAD_INITIALIZER(stailqhead2);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm, *elm_tmp;

    STAILQ_INIT(&stailqhead1);
    STAILQ_INIT(&stailqhead2);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);

    STAILQ_CONCAT(&stailqhead1, &stailqhead2, links);
    fail_if(STAILQ_FIRST(&stailqhead1) == NULL,
            "Head of destination singly-linked tail queue is NULL after concatenation");
    fail_unless(STAILQ_FIRST(&stailqhead1)->data == 1,
                "Wrong value in head of destination singly-linked tail queue");
    fail_if(STAILQ_LAST(&stailqhead1) == NULL,
            "Tail of destination singly-linked tail queue is NULL after concatenation");
    fail_unless(STAILQ_LAST(&stailqhead1)->data == 4,
                "Wrong value in tail of destination singly-linked tail queue");

    elm = STAILQ_FIRST(&stailqhead1);
    while (elm != NULL) {
        elm_tmp = STAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Concatenate 2 non empty singly-linked tail queues.
 */
START_TEST(STAILQ11)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    STAILQ_HEAD(stailqhead_t, node_t) stailqhead1 =
        STAILQ_HEAD_INITIALIZER(stailqhead1);
    struct stailqhead_t stailqhead2 = STAILQ_HEAD_INITIALIZER(stailqhead2);
    struct node_t {
        int data;
         STAILQ_ENTRY(node_t) links;
    } *elm, *elm_tmp;

    STAILQ_INIT(&stailqhead1);
    STAILQ_INIT(&stailqhead2);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    STAILQ_INSERT_HEAD(&stailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    STAILQ_INSERT_HEAD(&stailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    STAILQ_INSERT_HEAD(&stailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    STAILQ_INSERT_HEAD(&stailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    STAILQ_INSERT_HEAD(&stailqhead2, elm, links);

    STAILQ_CONCAT(&stailqhead1, &stailqhead2, links);
    fail_if(STAILQ_FIRST(&stailqhead1) == NULL,
            "Head of destination singly-linked tail queue is NULL after concatenation");
    fail_unless(STAILQ_FIRST(&stailqhead1)->data == 1,
                "Wrong value in head of destination singly-linked tail queue");
    fail_if(STAILQ_LAST(&stailqhead1) == NULL,
            "Tail of destination singly-linked tail queue is NULL after concatenation");
    fail_unless(STAILQ_LAST(&stailqhead1)->data == 13,
                "Wrong value in tail of destination singly-linked tail queue");
    fail_unless(STAILQ_EMPTY(&stailqhead2) == 1,
                "Source singly-linked tail queue is not empty");
    fail_unless(STAILQ_LAST(&stailqhead2) == NULL,
                "Tail of source singly-linked tail queue was not updated");

    for (elm = STAILQ_FIRST(&stailqhead1); elm != NULL; elm = elm_tmp, iter++) {
        fail_if(check[iter] == -1, "Too much data in singly-linked tail queue");
        fail_unless((elm)->data == check[iter],
                    "Wrong value in singly-linked tail queue");
        elm_tmp = STAILQ_NEXT(elm, links);
        free(elm);
    }
}
END_TEST

TCase *
stailq_tests()
{
    TCase *tc;

    tc = tcase_create("Singly-linked tail queue tests");
    tcase_add_test(tc, STAILQ01);
    tcase_add_test(tc, STAILQ02);
    tcase_add_test(tc, STAILQ03);
    tcase_add_test(tc, STAILQ04);
    tcase_add_test(tc, STAILQ05);
    tcase_add_test(tc, STAILQ06);
    tcase_add_test(tc, STAILQ07);
    tcase_add_test(tc, STAILQ08);
    tcase_add_test(tc, STAILQ09);
    tcase_add_test(tc, STAILQ10);
    tcase_add_test(tc, STAILQ11);

    return tc;
}
