/*
 * Copyright (c) 2005, National ICT Australia
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
 * Author: Alex Webster <Alex.Webster@nicta.com.au> 
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <l4/thread.h>
#include <l4e/map.h>
#include <queue/tailq.h>
#include "alloc.h"
#include "util.h"
#include "memsection.h"
#include "pd.h"
#include "objtable.h"
#include "slab_cache.h"
#include "tools.h"

struct chunk {
    TAILQ_ENTRY(chunk) chunk_list;
};

struct slab {
    TAILQ_HEAD(chunk_head, chunk) chunks;
    TAILQ_ENTRY(slab) slab_list;
};

void *
slab_cache_alloc(struct slab_cache *cache)
{
    struct chunk *chunk;
    struct slab *slab;

    TAILQ_FOREACH(slab, &cache->slabs, slab_list) {
        if (!TAILQ_EMPTY(&slab->chunks))
            break;
    }

    if (slab == NULL) {
        if (mem_empty(internal_virtpool)) {
            return NULL;
        }
        if (mem_empty(internal_physpool)) {
            return NULL;
        }
        slab = __slab_cache_add(cache, 
                                mem_alloc_unboxed(internal_virtpool, 0x1000),
                                mem_alloc_unboxed(internal_physpool, 0x1000),
                                0x1000);
    }

    if (slab == NULL) {
        /* We really can't allocate any memory. */
        return NULL;
    }

    chunk = TAILQ_FIRST(&slab->chunks);
    TAILQ_REMOVE(&slab->chunks, chunk, chunk_list);
    memzero(chunk, cache->size);
    return chunk;
}

void
slab_cache_free(struct slab_cache *cache, void *ptr)
{
    struct chunk *chunk;
    struct slab *slab;

    chunk = ptr;
    slab = (struct slab *)((uintptr_t)ptr >> 12 << 12);
    TAILQ_INSERT_TAIL(&slab->chunks, chunk, chunk_list);
}

/* 8-byte align the size. */
static uintptr_t align8(uintptr_t size)
{
    uintptr_t ovr = size % 8;
    if (ovr) {
        size = size + 8 - ovr;
    }

    return size;
}


struct slab *
__slab_cache_add(struct slab_cache *cache, uintptr_t vbase, uintptr_t pbase,
                 size_t size)
{
    int i, n, r;
    struct chunk *chunk;
    struct slab *slab;
    /* Align objects on an 8-byte boundary. */
    uintptr_t aligned_size = align8(cache->size);

    r = l4e_map(pd_l4_space(&iguana_pd), vbase, vbase + size - 1, pbase, 
                L4_ReadWriteOnly, L4_DefaultMemory);

    slab = (struct slab *)vbase;
    chunk = (struct chunk *)(vbase + max(sizeof(struct slab), aligned_size));

    n = (vbase + size - (uintptr_t)chunk) / aligned_size;
    TAILQ_INIT(&slab->chunks);

    assert(n > 0);
    for (i = 0; i < n; i++) {
        TAILQ_INSERT_TAIL(&slab->chunks, chunk, chunk_list);
        chunk = (struct chunk *)((uintptr_t)chunk + aligned_size);
    }
    TAILQ_INSERT_TAIL(&cache->slabs, slab, slab_list);
    return slab;
}
