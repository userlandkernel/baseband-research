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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "clist.h"

#define MIN_CLIST_SLOTS         16
static int clist_grow(struct clist *clist);
static int clist_shrink(struct clist *clist);

static int
capcmp(const void *x, const void *y)
{
    uintptr_t r1;
    uintptr_t r2;

    r1 = *(const uintptr_t *)x;
    r2 = *(const uintptr_t *)y;
    return r1 - r2;
}


struct clist *
server_clist_create(struct pd *pd)
{
    struct clist *clist;

    clist = malloc(sizeof(*clist));
    if (clist != NULL) {
        clist->magic = CLIST_MAGIC;
        clist->owner = pd;
        clist->used = 0;
        clist->size = MIN_CLIST_SLOTS;
        clist->cap = malloc(clist->size * sizeof(cap_t));
        if (clist->cap == NULL) {
            free(clist);
            return NULL;
        }
        TAILQ_INSERT_TAIL(&pd->owned_clists, clist, clist_list);
    }
    return clist;
}

void
server_clist_delete(struct clist *clist)
{
    TAILQ_REMOVE(&clist->owner->owned_clists, clist, clist_list);
    free(clist->cap);
    free(clist);
}

int
server_clist_insert(struct clist *clist, cap_t cap)
{
    int i;
    int r;

    if (clist->used == clist->size) {
        r = clist_grow(clist);
        if (r != 0) {
            return -1;
        }
    }
    clist->used++;
    for (i = clist->used - 2; i >= 0; i--) {
        if (clist->cap[i].ref.obj < cap.ref.obj) {
            break;
        }
        clist->cap[i + 1] = clist->cap[i];
    }
    clist->cap[i + 1] = cap;
    return 0;
}

cap_t
server_clist_lookup(struct clist *clist, uintptr_t obj, unsigned interface)
{
    cap_t cap;
    cap_t *p;

    cap.ref.obj = obj | interface;
    p = bsearch(&cap, clist->cap, clist->used, sizeof(cap_t), capcmp);
    if (p == NULL) {
        return invalid_cap;
    }
    return *p;
}

int
server_clist_remove(struct clist *clist, cap_t cap)
{
    int i;
    cap_t *p;

    p = bsearch(&cap, clist->cap, clist->used, sizeof(cap_t), capcmp);
    if (p == NULL) {
        return -1;
    }
    for (i = p - clist->cap; i < clist->used - 1; i++) {
        clist->cap[i] = clist->cap[i + 1];
    }
    clist->used--;
    if (clist->used < clist->size / 4) {
        (void)clist_shrink(clist);
    }
    return 0;
}

static int
clist_grow(struct clist *clist)
{
    cap_t *new_array;
    size_t new_size;

    new_size = clist->size * 2;
    new_array = realloc(clist->cap, new_size * sizeof(cap_t));
    if (new_array == NULL) {
        return -1;
    }
    clist->size = new_size;
    clist->cap = new_array;
    return 0;
}

static int
clist_shrink(struct clist *clist)
{
    cap_t *new_array;
    size_t new_size;

    if (clist->size == MIN_CLIST_SLOTS) {
        return 0;
    }
    new_size = clist->size / 2;
    new_array = realloc(clist->cap, new_size * sizeof(cap_t));
    if (new_array == NULL) {
        return -1;
    }
    clist->size = new_size;
    clist->cap = new_array;
    return 0;
}
