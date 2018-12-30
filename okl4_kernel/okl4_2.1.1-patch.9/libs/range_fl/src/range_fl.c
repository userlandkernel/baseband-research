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
 * Description: range_fl implements a free list. The external interface is
 * basically alloc() and free(). A simple implementation would just be a linked 
 * list.
 * 
 * This implementation tries to improve on space needed to hold the free list.
 * We have a list of ranges. This means if we have a contiguous set of items
 * the space required is very small. As long as the list isn't too fragmented
 * this is a win. In the fragmented case we degenerate to a linked list.
 * 
 * The overhead with this is that on a free it must make sure the list remains
 * sorted, and also perform any range coalescing. E.g: If you have a list 1-3,
 * 5-6, and then free '4' you want to get 1-6.
 * 
 * Allocation is fairly straightforward, just take the front of the list and if 
 * you exhaust a range, then remove it from the linked list.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au>
 * 
 */

#include <ll/ll.h>
#include <stdlib.h>
#include <stddef.h>
#include <range_fl/range_fl.h>
#include <stdio.h>
#include <inttypes.h>

rfl_t
rfl_new(void)
{
    return ll_new();
}

int
rfl_insert_range(rfl_t rfl, uintptr_t from, uintptr_t to)
{
    struct ll *temp;
    struct range *range, *next_range;
    int added = 0;

#if defined(CONFIG_DEBUG)
    if (from > to) {
        /* Can't have a range like this */
        return E_RFL_INV_RANGE;
    }
#endif
    /* See we can append to existing */
    for (temp = rfl->next; temp != rfl; temp = temp->next) {
        range = temp->data;
#if defined(CONFIG_DEBUG)
        /* Check that the new range doesn't overlap with this existing range */
        if ((from >= range->from && from <= range->to) ||
            (to >= range->from && to <= range->to)) {
            return E_RFL_OVERLAP;
        }
#endif
        if (range->from == to + 1) {
            /* In this case can add to the start of this range */
            range->from = from;
            added = 1;
            break;
        }
        if (range->to == from - 1) {
            /* In this case we can add to the end of this range */
            next_range = temp->next->data;
            range->to = to;
            if (next_range != NULL && range->to + 1 == next_range->from) {
                /* Merge with next range */
                range->to = next_range->to;
                /* Now delete the next range */
                free(next_range);
                ll_delete(temp->next);
            }
            added = 1;
            break;
        }
        if (range->from > to) {
            /*
             * In this case we need to insert it before the existing range, so
             * we break now and let the logic at the end add it. 
             */
            break;
        }
    }

    if (added == 0) {
        /*
         * Couldn't extend an existing range, lets add a new range before the
         * current one. This keeps the list sorted. 
         */
        struct range *new_range = malloc(sizeof(struct range));

        if (new_range == NULL) {
            return E_RFL_NOMEM; /* Failure! */
        }
        ll_insert_before(temp, new_range);
        new_range->from = from;
        new_range->to = to;
    }

    return RFL_SUCCESS;
}

int
rfl_free(rfl_t rfl, uintptr_t val)
{
    return rfl_insert_range(rfl, val, val);
}

uintptr_t
rfl_alloc(rfl_t rfl)
{
    struct range *range;
    uintptr_t retval;

    if (rfl->next == rfl) {
        /* This is the no items left case */
        return -1UL;
    }
    range = rfl->next->data;
    retval = range->from;
    if (range->from == range->to) {
        /* None left in this range now, free resources */
        free(range);
        ll_delete(rfl->next);
    } else {
        /* There are more left in the range, just increment the from value */
        range->from++;
    }
    return retval;
}

#if 0
void
rfl_debug(rfl_t rfl, FILE *out)
{
    struct ll *temp;
    struct range *range;

    for (temp = rfl->next; temp != rfl; temp = temp->next) {
        range = temp->data;
        printf("from: %" PRIdPTR " to: %" PRIdPTR "\n", range->from, range->to);
    }
}
#endif
