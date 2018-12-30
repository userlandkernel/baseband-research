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
#include <queue/tailq.h>
#include "test_libs_queue.h"

/* 
 * Tail Queue head initialisation. Check TAILQ_FIRST(), TAILQ_LAST() and TAILQ_EMPTY().
 */
START_TEST(TAILQ01)
{
    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    TAILQ_INIT(&tailqhead);
    fail_unless(TAILQ_FIRST(&tailqhead) == NULL,
                "The tail queue was not initialised properly or TAILQ_FIRST failed");
    fail_unless(TAILQ_LAST(&tailqhead) == NULL,
                "The tail queue was not initialised properly or TAILQ_LAST failed");
    fail_unless(TAILQ_EMPTY(&tailqhead) == 1,
                "The tail queue was not initialised properly or TAILQ_EMPTY failed");
}
END_TEST

/* 
 * Insert at the head in an empty tail queue and then insert at the tail. 
 * Check TAILQ_NEXT() and TAILQ_PREV().
 */
START_TEST(TAILQ02)
{
    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    TAILQ_INIT(&tailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm1, links);
    fail_unless(TAILQ_FIRST(&tailqhead) == elm1,
                "Element was not inserted at the head of the tail queue");
    fail_unless(TAILQ_FIRST(&tailqhead)->data == 1,
                "Wrong value in head of tail queue");
    fail_unless((elm1)->links.tqe_prev == NULL,
                "Pointer to previous element was not set to NULL");
    fail_unless(TAILQ_LAST(&tailqhead) == elm1,
                "The tail has not been updated");
    fail_unless(TAILQ_LAST(&tailqhead)->data == 1,
                "Wrong value in tail of tail queue");
    fail_unless((elm1)->links.tqe_next == NULL,
                "Pointer to next element was not set to NULL");

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm2, links);
    fail_unless(TAILQ_FIRST(&tailqhead) == elm2,
                "Element was not inserted at the head of the tail queue");
    fail_unless(TAILQ_FIRST(&tailqhead)->data == 2,
                "Wrong value in head of tail queue");
    fail_unless((elm2)->links.tqe_prev == NULL,
                "Pointer to previous element was not set to NULL");
    fail_unless((elm2)->links.tqe_next == elm1,
                "Pointer to next element does not point to element1");
    fail_unless(TAILQ_NEXT(elm2, links) == elm1,
                "TAILQ_NEXT() returned bad pointer");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    TAILQ_INSERT_TAIL(&tailqhead, elm3, links);
    fail_unless(TAILQ_LAST(&tailqhead) == elm3,
                "Element was not inserted at the tail of the tail queue");
    fail_unless(TAILQ_LAST(&tailqhead)->data == 3,
                "Wrong value in tail of tail queue");
    fail_unless((elm3)->links.tqe_next == NULL,
                "Pointer to next element was not set to NULL");
    fail_unless((elm3)->links.tqe_prev == elm1,
                "Pointer to previous element does not point to element1");
    fail_unless(TAILQ_PREV(elm3, links) == elm1,
                "TAILQ_PREV() returned bad pointer");

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert at the tail in an empty tail queue and then insert at the head. 
 */
START_TEST(TAILQ03)
{
    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    TAILQ_INIT(&tailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    TAILQ_INSERT_TAIL(&tailqhead, elm1, links);
    fail_unless(TAILQ_LAST(&tailqhead) == elm1,
                "Element was not inserted at the tail of the tail queue");
    fail_unless(TAILQ_LAST(&tailqhead)->data == 1,
                "Wrong value in tail of tail queue");
    fail_unless((elm1)->links.tqe_next == NULL,
                "Pointer to next element was not set to NULL");
    fail_unless(TAILQ_FIRST(&tailqhead) == elm1,
                "The head has not been updated");
    fail_unless(TAILQ_FIRST(&tailqhead)->data == 1,
                "Wrong value in head of tail queue");
    fail_unless((elm1)->links.tqe_prev == NULL,
                "Pointer to previous element was not set to NULL");

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    TAILQ_INSERT_TAIL(&tailqhead, elm2, links);
    fail_unless(TAILQ_LAST(&tailqhead) == elm2,
                "Element was not inserted at the tail of the tail queue");
    fail_unless(TAILQ_LAST(&tailqhead)->data == 2, "Wrong value in tail queue");
    fail_unless((elm2)->links.tqe_next == NULL,
                "Pointer to next element was not set to NULL");
    fail_unless((elm2)->links.tqe_prev == elm1,
                "Pointer to previous element does not point to element1");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    TAILQ_INSERT_HEAD(&tailqhead, elm3, links);
    fail_unless(TAILQ_FIRST(&tailqhead) == elm3,
                "Element was not inserted at the head of the tail queue");
    fail_unless(TAILQ_FIRST(&tailqhead)->data == 3,
                "Wrong value in head of tail queue");
    fail_unless((elm3)->links.tqe_prev == NULL,
                "Pointer to previous element was not set to NULL");
    fail_unless((elm3)->links.tqe_next == elm1,
                "Pointer to next element does not point to element1");

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert after
 */
START_TEST(TAILQ04)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    TAILQ_INIT(&tailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    TAILQ_INSERT_AFTER(&tailqhead, elm1, elm2, links);
    fail_unless((elm1)->links.tqe_next == elm2,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.tqe_prev == elm1,
                "Element2 was not inserted properly after element1");
    fail_unless((elm2)->links.tqe_next == NULL,
                "Element2 pointer to next element was not set to NULL");
    fail_unless(TAILQ_LAST(&tailqhead) == elm2,
                "Pointer to last element of the tail queue does not point to element2");

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    TAILQ_INSERT_AFTER(&tailqhead, elm1, elm3, links);
    fail_unless((elm1)->links.tqe_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.tqe_prev == elm1,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.tqe_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = TAILQ_FIRST(&tailqhead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.tqe_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_tmp)->data == check[iter],
                    "Wrong value in tail queue");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Insert before
 */
START_TEST(TAILQ05)
{
    int check[] = { 1, 3, 2, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3, *elm_tmp;

    TAILQ_INIT(&tailqhead);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm1, links);

    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    TAILQ_INSERT_AFTER(&tailqhead, elm1, elm2, links);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    TAILQ_INSERT_BEFORE(&tailqhead, elm2, elm3, links);
    fail_unless((elm1)->links.tqe_next == elm3,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.tqe_prev == elm1,
                "Element3 was not inserted properly after element1");
    fail_unless((elm3)->links.tqe_next == elm2,
                "Element3 was not inserted properly before element2");

    for (elm_tmp = TAILQ_FIRST(&tailqhead); elm_tmp != NULL;
         elm_tmp = (elm_tmp)->links.tqe_next, iter++) {
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_tmp)->data == check[iter],
                    "Wrong value in tail queue");
    }

    free(elm1);
    free(elm2);
    free(elm3);
}
END_TEST

/* 
 * Forward traversal
 */
START_TEST(TAILQ06)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_var;

    TAILQ_INIT(&tailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);

    TAILQ_FOREACH(elm_var, &tailqhead, links) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in tail queue");
        iter++;
    }
    // printf("\n");

    elm = TAILQ_FIRST(&tailqhead);
    while (elm != NULL) {
        elm_var = TAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Forward traversal safe
 */
START_TEST(TAILQ07)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_var, *elm_tmp;

    TAILQ_INIT(&tailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);

    TAILQ_FOREACH_SAFE(elm_var, &tailqhead, links, elm_tmp) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in tail queue");
        if ((elm_var)->data == 7) {
            TAILQ_REMOVE(&tailqhead, elm_var, links);
            free(elm_var);
        }
        iter++;
    }
    // printf("\n");

    elm = TAILQ_FIRST(&tailqhead);
    while (elm != NULL) {
        elm_tmp = TAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Reverse traversal
 */
START_TEST(TAILQ08)
{
    int check[] = { -1, 1, 2, 4, 7, 10, 11, 13 };
    int iter = 7;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_var;

    TAILQ_INIT(&tailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);

    TAILQ_FOREACH_REVERSE(elm_var, &tailqhead, links) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in tail queue");
        iter--;
    }
    // printf("\n");

    elm = TAILQ_FIRST(&tailqhead);
    while (elm != NULL) {
        elm_var = TAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Reverse traversal safe
 */
START_TEST(TAILQ09)
{
    int check[] = { -1, 1, 2, 4, 7, 10, 11, 13 };
    int iter = 7;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_var, *elm_tmp;

    TAILQ_INIT(&tailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);

    TAILQ_FOREACH_REVERSE_SAFE(elm_var, &tailqhead, links, elm_tmp) {
        // printf("%d ", (elm_var)->data);
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm_var)->data == check[iter],
                    "Wrong value in tail queue");
        if ((elm_var)->data == 7) {
            TAILQ_REMOVE(&tailqhead, elm_var, links);
            free(elm_var);
        }
        iter--;
    }
    // printf("\n");

    elm = TAILQ_FIRST(&tailqhead);
    while (elm != NULL) {
        elm_var = TAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_var;
    }
}
END_TEST

/* 
 * Element deletion
 */
START_TEST(TAILQ10)
{
    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm1, *elm2, *elm3;

    TAILQ_INIT(&tailqhead);

    elm3 = malloc(sizeof(struct node_t));
    (elm3)->data = 3;
    TAILQ_INSERT_HEAD(&tailqhead, elm3, links);
    elm2 = malloc(sizeof(struct node_t));
    (elm2)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm2, links);
    elm1 = malloc(sizeof(struct node_t));
    (elm1)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm1, links);

    TAILQ_REMOVE(&tailqhead, elm2, links);
    free(elm2);
    fail_if(TAILQ_FIRST(&tailqhead) == NULL, NULL);
    fail_if(TAILQ_FIRST(&tailqhead)->links.tqe_next == elm2,
            "Element2 has not been removed from tail queue");
    fail_unless(TAILQ_FIRST(&tailqhead)->links.tqe_next == elm3,
                "Wrong element in tail queue");
    fail_unless((elm3)->links.tqe_prev == TAILQ_FIRST(&tailqhead),
                "Wrong element in tail queue");
    fail_unless(TAILQ_FIRST(&tailqhead)->links.tqe_next->data == 3,
                "Wrong value in tail queue");

    free(elm1);
    free(elm3);
}
END_TEST

/* 
 * Tail queue deletion
 */
START_TEST(TAILQ11)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead =
        TAILQ_HEAD_INITIALIZER(tailqhead);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm;

    TAILQ_INIT(&tailqhead);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead, elm, links);

    while (!TAILQ_EMPTY(&tailqhead)) {
        elm = TAILQ_FIRST(&tailqhead);
        if ((elm)->data == check[iter]) {
            iter++;
        }
        TAILQ_REMOVE(&tailqhead, elm, links);
        free(elm);
    }
    fail_unless(tailqhead.tqh_first == NULL,
                "Tail queue is not empty after removing all");
    fail_unless(check[iter] == -1,
                "Tail queue deletion has not removed all elements");
}
END_TEST

/* 
 * Concatenate an empty tail queue to a non empty tail queue.
 */
START_TEST(TAILQ12)
{
    TAILQ_HEAD(tailqhead_t, node_t) tailqhead1 =
        TAILQ_HEAD_INITIALIZER(tailqhead1);
    struct tailqhead_t tailqhead2 = TAILQ_HEAD_INITIALIZER(tailqhead2);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_tmp;

    TAILQ_INIT(&tailqhead1);
    TAILQ_INIT(&tailqhead2);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);

    TAILQ_CONCAT(&tailqhead1, &tailqhead2, links);
    fail_if(TAILQ_FIRST(&tailqhead1) == NULL,
            "Head of destination tail queue is NULL after concatenation");
    fail_unless(TAILQ_FIRST(&tailqhead1)->data == 1,
                "Wrong value in head of destination tail queue");
    fail_if(TAILQ_LAST(&tailqhead1) == NULL,
            "Tail of destination tail queue is NULL after concatenation");
    fail_unless(TAILQ_LAST(&tailqhead1)->data == 4,
                "Wrong value in tail of destination tail queue");

    elm = TAILQ_FIRST(&tailqhead1);
    while (elm != NULL) {
        elm_tmp = TAILQ_NEXT(elm, links);
        free(elm);
        elm = elm_tmp;
    }
}
END_TEST

/* 
 * Concatenate 2 non empty tail queues.
 */
START_TEST(TAILQ13)
{
    int check[] = { 1, 2, 4, 7, 10, 11, 13, -1 };
    int iter = 0;

    TAILQ_HEAD(tailqhead_t, node_t) tailqhead1 =
        TAILQ_HEAD_INITIALIZER(tailqhead1);
    struct tailqhead_t tailqhead2 = TAILQ_HEAD_INITIALIZER(tailqhead2);
    struct node_t {
        int data;
         TAILQ_ENTRY(node_t) links;
    } *elm, *elm_tmp;

    TAILQ_INIT(&tailqhead1);
    TAILQ_INIT(&tailqhead2);

    elm = malloc(sizeof(struct node_t));
    (elm)->data = 4;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 2;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 1;
    TAILQ_INSERT_HEAD(&tailqhead1, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 13;
    TAILQ_INSERT_HEAD(&tailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 11;
    TAILQ_INSERT_HEAD(&tailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 10;
    TAILQ_INSERT_HEAD(&tailqhead2, elm, links);
    elm = malloc(sizeof(struct node_t));
    (elm)->data = 7;
    TAILQ_INSERT_HEAD(&tailqhead2, elm, links);

    TAILQ_CONCAT(&tailqhead1, &tailqhead2, links);
    fail_if(TAILQ_FIRST(&tailqhead1) == NULL,
            "Head of destination tail queue is NULL after concatenation");
    fail_unless(TAILQ_FIRST(&tailqhead1)->data == 1,
                "Wrong value in head of destination tail queue");
    fail_if(TAILQ_LAST(&tailqhead1) == NULL,
            "Tail of destination tail queue is NULL after concatenation");
    fail_unless(TAILQ_LAST(&tailqhead1)->data == 13,
                "Wrong value in tail of destination tail queue");
    fail_unless(TAILQ_EMPTY(&tailqhead2) == 1,
                "Source tail queue is not empty");
    fail_unless(TAILQ_LAST(&tailqhead2) == NULL,
                "Tail of source tail queue was not updated");

    for (elm = TAILQ_FIRST(&tailqhead1); elm != NULL; elm = elm_tmp, iter++) {
        fail_if(check[iter] == -1, "Too much data in tail queue");
        fail_unless((elm)->data == check[iter], "Wrong value in tail queue");
        elm_tmp = TAILQ_NEXT(elm, links);
        free(elm);
    }
}
END_TEST

TCase *
tailq_tests()
{
    TCase *tc;

    tc = tcase_create("Tail queue tests");

    tcase_add_test(tc, TAILQ01);
    tcase_add_test(tc, TAILQ02);
    tcase_add_test(tc, TAILQ03);
    tcase_add_test(tc, TAILQ04);
    tcase_add_test(tc, TAILQ05);
    tcase_add_test(tc, TAILQ06);
    tcase_add_test(tc, TAILQ07);
    tcase_add_test(tc, TAILQ08);
    tcase_add_test(tc, TAILQ09);
    tcase_add_test(tc, TAILQ10);
    tcase_add_test(tc, TAILQ11);
    tcase_add_test(tc, TAILQ12);
    tcase_add_test(tc, TAILQ13);

    return tc;
}
