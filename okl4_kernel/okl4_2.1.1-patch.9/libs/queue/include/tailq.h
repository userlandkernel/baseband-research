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
 * Author: Alex Webster
 */
#ifndef TAILQ_H
#define TAILQ_H

#include <stddef.h>

#define TAILQ_CONCAT(head1, head2, name) \
    do { \
            if ((head2)->tqh_first != NULL) { \
                (head1)->tqh_last->name.tqe_next = (head2)->tqh_first; \
                (head2)->tqh_first->name.tqe_prev = (head1)->tqh_last; \
                (head1)->tqh_last = (head2)->tqh_last; \
                (head2)->tqh_first = NULL; \
                (head2)->tqh_last = NULL; \
        } \
    } while (0)

#define TAILQ_EMPTY(head) \
    ((head)->tqh_first == NULL)

#define TAILQ_ENTRY(type) \
    struct { \
            struct type *tqe_next; \
            struct type *tqe_prev; \
    }

#define TAILQ_FIRST(head) \
    ((head)->tqh_first)

#define TAILQ_FOREACH(var, head, name) \
    for ((var) = (head)->tqh_first; (var); (var) = (var)->name.tqe_next)

#define TAILQ_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_first; (tmp) = (var) ? (var)->name.tqe_next : NULL, (var); (var) = (tmp))

#define TAILQ_FOREACH_REVERSE(var, head, name) \
    for ((var) = (head)->tqh_last; (var); (var) = (var)->name.tqe_prev)

#define TAILQ_FOREACH_REVERSE_SAFE(var, head, name, tmp) \
    for ((var) = (head)->tqh_last; (tmp) = (var) ? (var)->name.tqe_prev : NULL, (var); (var) = (tmp))

#define TAILQ_HEAD(headname, type) \
    struct headname { \
            struct type *tqh_first; \
            struct type *tqh_last; \
    }

#define TAILQ_HEAD_INITIALIZER(head) \
    { NULL, NULL }

#define TAILQ_INIT(head) \
    do { \
            (head)->tqh_first = NULL; \
            (head)->tqh_last = NULL; \
    } while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, name) \
    do { \
            (elm)->name.tqe_next = (listelm)->name.tqe_next; \
            (elm)->name.tqe_prev = (listelm); \
            (listelm)->name.tqe_next = (elm); \
            if ((elm)->name.tqe_next != NULL) { \
                    (elm)->name.tqe_next->name.tqe_prev = (elm); \
            } else { \
                    (head)->tqh_last = (elm); \
            } \
    } while (0)

#define TAILQ_INSERT_BEFORE(head, listelm, elm, name) \
    do { \
        (elm)->name.tqe_next = (listelm); \
        (elm)->name.tqe_prev = (listelm)->name.tqe_prev; \
        (listelm)->name.tqe_prev = (elm); \
        if ((elm)->name.tqe_prev != NULL) { \
                (elm)->name.tqe_prev->name.tqe_next = (elm); \
        } else { \
                (head)->tqh_first = (elm); \
        } \
    } while (0)

#define TAILQ_INSERT_HEAD(head, elm, name) \
    do { \
            (elm)->name.tqe_next = (head)->tqh_first; \
            (elm)->name.tqe_prev = NULL; \
            if ((head)->tqh_first != NULL) { \
                    (head)->tqh_first->name.tqe_prev = (elm); \
            } \
            if ((elm)->name.tqe_next == NULL) { \
                    (head)->tqh_last = (elm); \
            } \
            (head)->tqh_first = (elm); \
    } while (0)

#define TAILQ_INSERT_TAIL(head, elm, name) \
    do { \
            (elm)->name.tqe_next = NULL; \
            (elm)->name.tqe_prev = (head)->tqh_last; \
            if ((head)->tqh_last != NULL) { \
                    (head)->tqh_last->name.tqe_next = (elm); \
            } \
            if ((elm)->name.tqe_prev == NULL) { \
                    (head)->tqh_first = (elm); \
            } \
            (head)->tqh_last = (elm); \
    } while (0)

#define TAILQ_LAST(head) \
    ((head)->tqh_last)

#define TAILQ_NEXT(elm, name) \
    ((elm)->name.tqe_next)

#define TAILQ_PREV(elm, name) \
    ((elm)->name.tqe_prev)

#define TAILQ_REMOVE(head, elm, name) \
    do { \
            if ((elm)->name.tqe_next != NULL) { \
                    (elm)->name.tqe_next->name.tqe_prev = (elm)->name.tqe_prev; \
            } else { \
                    (head)->tqh_last = (elm)->name.tqe_prev; \
            } \
            if ((elm)->name.tqe_prev != NULL) { \
                    (elm)->name.tqe_prev->name.tqe_next = (elm)->name.tqe_next; \
            } else { \
                    (head)->tqh_first = (elm)->name.tqe_next; \
            } \
    } while (0)

#endif
