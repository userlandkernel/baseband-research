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
 * Description: Bit array freelist.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au>
 * 
 */

#include <stdlib.h>
#include <stddef.h>
#include <bit_fl/bit_fl.h>
#include <string.h>
#include <assert.h>

#define BITS_PER_LONG (sizeof(bfl_word)*8)

static int find_first_set(bfl_word x);

/*
 * FIXME: (benjl) Find a better home for this 
 */
static int
find_first_set(bfl_word x)
{
    int ret;

    for (ret = 0; ret < BITS_PER_LONG; ret++) {
        if (x & 1UL) {
            return ret;
        }
        x >>= 1UL;
    }
    assert(!"no bit found");
    return -1;
}

bfl_t
bfl_new(uintptr_t size)
{
    int i;
    int array_size = (size / BITS_PER_LONG) + 1;

    /* Allocate enough space for the header and the bit array needed */
    bfl_t bfl = malloc(sizeof(struct bfl) + (array_size) * sizeof(bfl_word));

    if (bfl == NULL) {
        return NULL;
    }

    bfl->curpos = 0;
    bfl->len = array_size;
    /* Set all as allocated */
    memset(bfl->bitarray, 0, (array_size) * sizeof(bfl_word));
    /* Now free the ones we have */
    /*
     * FIXME: (benjl) This is a terribly ineffecient way to do this 
     */
    for (i = 0; i < size; i++)
        bfl_free(bfl, i);
    return bfl;
}

void
bfl_free(bfl_t bfl, uintptr_t val)
{
    int idx = val / BITS_PER_LONG;

    /* Set the bit */
    assert((val / BITS_PER_LONG) < bfl->len);
    bfl->bitarray[idx] |= 1UL << (val % BITS_PER_LONG);
}

uintptr_t
bfl_alloc(bfl_t bfl)
{
    /* Find empty int */
    int found = 0;
    int pos, i;

    for (i = bfl->curpos; i < bfl->len; i++) {
        if (bfl->bitarray[i] != 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        for (i = 0; i < bfl->curpos; i++) {
            if (bfl->bitarray[i] != 0) {
                found = 1;
                break;
            }
        }
    }

    if (found == 0) {
        return -1UL;
    }

    pos = find_first_set(bfl->bitarray[i]);
    /* Unset the bit */
    bfl->bitarray[i] &= ~(1UL << pos);

    if (bfl->bitarray == 0)
        bfl->curpos = (bfl->curpos + 1) % bfl->len;
    return (i * BITS_PER_LONG) + pos;
}

void
bfl_destroy(bfl_t bfl)
{
    free(bfl);
}

int
bfl_is_free(bfl_t bfl, uintptr_t bit)
{
    int idx = bit / BITS_PER_LONG;

    assert((bit / BITS_PER_LONG) < bfl->len);
    return bfl->bitarray[idx] & (1UL << (bit % BITS_PER_LONG));
}

uintptr_t
bfl_alloc_bit(bfl_t bfl, uintptr_t bit)
{
    int idx = bit / BITS_PER_LONG;

    assert((bit / BITS_PER_LONG) < bfl->len);
    return bfl->bitarray[idx] &= ~(1UL << (bit % BITS_PER_LONG));
}
