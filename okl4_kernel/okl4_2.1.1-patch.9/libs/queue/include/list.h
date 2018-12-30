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
#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define LIST_EMPTY(head) \
    ((head)->lh_first == NULL)

#define LIST_ENTRY(type) \
    struct { \
            struct type *le_next; \
            struct type *le_prev; \
    }

#define LIST_FIRST(head) \
    ((head)->lh_first)

#define LIST_FOREACH(var, head, name) \
    for ((var) = (head)->lh_first; (var); (var) = (var)->name.le_next)

#define LIST_FOREACH_SAFE(var, head, name, tmp) \
    for ((var) = (head)->lh_first; (tmp) = (var) ? (var)->name.le_next : NULL, (var); (var) = (tmp))

#define LIST_HEAD(headname, type) \
    struct headname { \
            struct type *lh_first; \
    }

#define LIST_HEAD_INITIALIZER(head) \
    { NULL }

#define LIST_INIT(head) \
    do { \
            (head)->lh_first = NULL; \
    } while (0)

#define LIST_INSERT_AFTER(head, listelm, elm, name) \
    do { \
            (elm)->name.le_next = (listelm)->name.le_next; \
            (elm)->name.le_prev = (listelm); \
            (listelm)->name.le_next = (elm); \
            if ((elm)->name.le_next != NULL) { \
                    (elm)->name.le_next->name.le_prev = (elm); \
            } \
    } while (0)

#define LIST_INSERT_BEFORE(head, listelm, elm, name) \
    do { \
        (elm)->name.le_next = (listelm); \
        (elm)->name.le_prev = (listelm)->name.le_prev; \
        (listelm)->name.le_prev = (elm); \
        if ((elm)->name.le_prev != NULL) { \
                (elm)->name.le_prev->name.le_next = (elm); \
        } \
    } while (0)

#define LIST_INSERT_HEAD(head, elm, name) \
    do { \
            (elm)->name.le_next = (head)->lh_first; \
            (elm)->name.le_prev = NULL; \
            if ((head)->lh_first != NULL) { \
                    (head)->lh_first->name.le_prev = (elm); \
            } \
            (head)->lh_first = (elm); \
    } while (0)

#define LIST_NEXT(elm, name) \
    ((elm)->name.le_next)

#define LIST_REMOVE(head, elm, name) \
    do { \
            if ((elm)->name.le_next != NULL) { \
                    (elm)->name.le_next->name.le_prev = (elm)->name.le_prev; \
            } \
            if ((elm)->name.le_prev != NULL) { \
                    (elm)->name.le_prev->name.le_next = (elm)->name.le_next; \
            } else { \
                    (head)->lh_first = (elm)->name.le_next; \
            } \
    } while (0)

#endif
