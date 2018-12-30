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
 * Memory Pools
 * ============
 *
 * This file implements a low level memory allocator.  The functionality here
 * shouldn't be exposed directly to the user.  These functions should only
 * really be accessed by the PhysMem and VirtMem objects.
 *
 * This allocator implements a segregated fit algorithm.  In contrast to
 * traditional segregated fit allocators, memory is stored in the power of two
 * sized list corresponding the largest aligned block.
 *
 * These functions are not especially optimised, but should perform
 * reasonably.  We defer coalescing till an allocation fails.  Some papers say
 * that this improves overall performance, but we'd need benchmarks to verify
 * that.  In any case deferring coalescing makes things relatively simpler --
 * we would need more complicated data structures to coalesce on the fly.
 *
 * The most obvious optimisation I can see would be to speed up free list
 * searches.  At present free lists are ordered linked lists, and they are
 * scanned linearly to find a suitable block.  In general this will hopefully
 * not be an issue, since there is a free list per super page size.  These
 * free lists are optimised to deal with aligned allocations, so allocating a
 * power of two sized region can just pop the head of the appropriate list.
 *
 * The allocator implemented here is the descendent of the old binary buddy
 * Fpage allocator.  Using segregated fit has resulted in slightly more
 * complicated computation to find the best block, but has drastically reduced
 * the intermodule complexity.
 */
#include "alloc.h"
#include "slab_cache.h"
#include "util.h"

/*
 * FIXME: This should probably be moved to <compat/bitops.h> or similar.
 */
#if defined(_lint)
int __clzl(size_t x);
#elif defined(__GNUC__) 
#if __GNUC__ > 2 && __GNUC_MINOR__ < 4  /* __builtin_clz() is gcc-3.4 or better */
#if defined(__i386__)
static inline int
__clzl(size_t x)
{
    int zeroes = 0;
    __asm__("\t"
            "bsrl %1, %0\n\t"
            "xorl $31, %0\n\n"
            : "=r" (zeroes)
            : "r" (x), "0"(zeroes)
            : "0");
    return  zeroes;

}
#else
#error "no __clzl defined for this architecture"
#endif /* defined(__i386__) */

#else
#define __clzl(x)               __builtin_clzl(x)
#endif /* gcc-3.4 or better */
#elif defined(__RVCT__) || defined(__RVCT_GNU__) || defined(__ADS__)
static inline int
__clzl(size_t x)
{
    int r;

    __asm {
        clz r, x
    }
    
    return r;
}
#else
#error "No __clzl defined for this architecture and compiler"
#endif /* __RVCT__ __RVCT_GNU__ __ADS__ */

#define ALIGN_UP(addr, size)    (((addr) + (size) - 1) & ~((size) - 1))
#define BEGIN(mem)              ((mem)->addr)
#define END(mem)                ((mem)->addr + (mem)->size - 1)
#define FREE_LIST(pool, idx)    (&(pool)->free_list[idx])
#define SIZE(mem)               ((mem)->size)

struct mem_pool *internal_physpool;
struct mem_pool *internal_virtpool;

static struct slab_cache mem_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct memory), &mem_cache);

static inline size_t
idx_to_size(unsigned idx)
{
    return 1UL << (idx + MEM_MIN_BITS);
}

static inline unsigned
size_to_idx(size_t size)
{
    return MEM_MAX_IDX - __clzl(size) - 1;
}

static inline uintptr_t
best_addr(size_t size, struct memory *mem)
{
    addr_t addr;
    size_t rsize;
    unsigned idx;

    idx = size_to_idx(size);
    addr = ALIGN_UP(mem->addr, idx_to_size(idx));
    rsize = END(mem) - addr + 1;

    if (rsize < size) {
        addr -= size - rsize;
    }
    return addr;
}

static inline unsigned
best_idx(struct memory *mem)
{
    addr_t addr;
    size_t size;
    unsigned idx;

    idx = size_to_idx(mem->size);
    size = idx_to_size(idx);
    addr = ALIGN_UP(mem->addr, size);

    if (addr + size - 1 > END(mem)) {
        idx--;
    }
    return idx;
}

static void
mem_unfree(struct mem_pool *pool, struct memory *ent)
{
    unsigned idx;

    idx = best_idx(ent);
    TAILQ_REMOVE(FREE_LIST(pool, idx), ent, mem_list);
}

static struct memory *
trim_mem(struct mem_pool *pool, struct memory *mem, addr_t addr, size_t size)
{
    size_t rest;
    struct memory *tmp;

    if (mem->addr < addr) {
        rest = mem->size - (addr - mem->addr);
        mem->size = addr - mem->addr;
        mem_free(pool, mem);
        tmp = mem;
        mem = slab_cache_alloc(&mem_cache);
        if (mem == NULL) {
            mem_unfree(pool, tmp);
            tmp->size += rest;
            mem_free(pool, tmp);
            return NULL;
        }
        mem->addr = addr;
        mem->size = rest;
    }
    if (SIZE(mem) > size) {
        mem->addr = addr + size;
        mem->size = mem->size - size;
        mem_free(pool, mem);
        tmp = mem;
        mem = slab_cache_alloc(&mem_cache);
        if (mem == NULL) {
            mem_unfree(pool, tmp);
            tmp->size += size;
            mem_free(pool, tmp);
            return NULL;
        }
        mem->addr = addr;
        mem->size = size;
    }
    return mem;
}

struct memory *
mem_alloc(struct mem_pool *pool, size_t size)
{
    addr_t addr;
    struct memory *ent = NULL;
    unsigned i, idx;

    size = ALIGN_UP(size, MEM_MIN_SIZE);

    idx = size_to_idx(size);
    for (i = idx; i <= MEM_MAX_IDX; i++) {
        TAILQ_FOREACH(ent, FREE_LIST(pool, i), mem_list) {
            if (ent->size >= size) {
                break;
            }
        }
        if (ent != NULL) {
            break;
        }
    }
    if (ent == NULL) {
        mem_coalesce(pool);
        for (i = idx; i <= MEM_MAX_IDX; i++) {
            TAILQ_FOREACH(ent, FREE_LIST(pool, i), mem_list) {
                if (ent->size >= size) {
                    break;
                }
            }
            if (ent != NULL) {
                break;
            }
        }
    }
    if (ent == NULL) {
        return NULL;
    }
    TAILQ_REMOVE(FREE_LIST(pool, i), ent, mem_list);

    addr = best_addr(size, ent);
    ent = trim_mem(pool, ent, addr, size);
    return ent;
}

struct memory *
mem_alloc_fixed(struct mem_pool *pool, size_t size, addr_t addr)
{
    struct memory *ent;
    unsigned idx;

    size = ALIGN_UP(size, MEM_MIN_SIZE);

    mem_coalesce(pool);
    for (idx = 0; idx <= MEM_MAX_IDX; idx++) {
        TAILQ_FOREACH(ent, FREE_LIST(pool, idx), mem_list) {
            if (ent->addr <= addr && addr <= END(ent)) {
                break;
            }
        }
        if (ent != NULL) {
            break;
        }
    }
    if (ent == NULL) {
        return NULL;
    }
    TAILQ_REMOVE(FREE_LIST(pool, idx), ent, mem_list);
    ent = trim_mem(pool, ent, addr, size);
    return ent;
}

void
mem_free(struct mem_pool *pool, struct memory *mem)
{
    struct memory *tmp;
    unsigned idx;

    pool->coalesced = 0;
    idx = best_idx(mem);
    if (TAILQ_EMPTY(FREE_LIST(pool, idx))) {
        TAILQ_INSERT_HEAD(FREE_LIST(pool, idx), mem, mem_list);
        return;
    }
    TAILQ_FOREACH(tmp, FREE_LIST(pool, idx), mem_list) {
        if (mem->size < tmp->size) {
            TAILQ_INSERT_BEFORE(FREE_LIST(pool, idx), tmp, mem, mem_list);
            return;
        }
    }
    TAILQ_INSERT_TAIL(FREE_LIST(pool, idx), mem, mem_list);
}

int
mem_add(struct mem_pool *pool, addr_t addr, size_t size)
{
    struct memory *ent;

    ent = mem_add_and_alloc(pool, addr, size);
    if (ent == NULL) {
        return -1;
    }
    mem_free(pool, ent);
    return 0;
}

struct memory *
mem_add_and_alloc(struct mem_pool *pool, addr_t addr, size_t size)
{
    struct memory *ent;

    /* FIXME: We should not be adding more memory than is passed in. Bootinfo
     * needs to be fixed so that it rounds up small sizes to added. */
    size = ALIGN_UP(size, MEM_MIN_SIZE);

    ent = slab_cache_alloc(&mem_cache);
    if (ent == NULL) {
        return NULL;
    }
    ent->addr = addr;
    ent->size = size;
    return ent;
}

uintptr_t
mem_alloc_unboxed(struct mem_pool *pool, size_t size)
{
    addr_t addr;
    struct memory *ent;
    unsigned i;

    if (pool != internal_virtpool && pool != internal_physpool) {
        return INVALID_ADDR;
    }
    if (size != MEM_MIN_SIZE) {
        return INVALID_ADDR;
    }

    for (i = 0; i <= MEM_MAX_IDX; i++) {
        if (!TAILQ_EMPTY(FREE_LIST(pool, i))) {
            break;
        }
    }
    if (i > MEM_MAX_IDX) {
        return INVALID_ADDR;
    }
    ent = TAILQ_FIRST(FREE_LIST(pool, i));
    addr = ent->addr;
    if (size < ent->size) {
        ent->addr += size;
        ent->size -= size;
    } else {
        TAILQ_REMOVE(FREE_LIST(pool, i), ent, mem_list);
        slab_cache_free(&mem_cache, ent);
    }

    return addr;
}

int
mem_remove(struct mem_pool *pool, addr_t addr, size_t size)
{
    struct memory *ent;

    ent = mem_alloc_fixed(pool, size, addr);
    if (ent == NULL) {
        return -1;
    }
    slab_cache_free(&mem_cache, ent);
    return 0;
}

/*
 * This function traverses all the memory in the pool and joins adjacent
 * blocks.  This is rather unoptimised at the moment, but not ridiculously so.
 * The implementation is intended to be simple.  This function would probably
 * benefit most from tweaking the data structures.
 */
void
mem_coalesce(struct mem_pool *pool)
{
    struct mem_list tmplist;
    struct memory *ent, *mem, *tmp;
    unsigned idx;

    if (pool->coalesced) {
        return;
    }
    /*
     * First we take all of the memory out of the lists...
     */
    TAILQ_INIT(&tmplist);
    for (idx = 0; idx <= MEM_MAX_IDX; idx++) {
        TAILQ_FOREACH_SAFE(ent, FREE_LIST(pool, idx), mem_list, tmp) {
            TAILQ_REMOVE(FREE_LIST(pool, idx), ent, mem_list);
            TAILQ_INSERT_TAIL(&tmplist, ent, mem_list);
        }
    }

    /*
     * Then we scan the list merging adjacent blocks, and adding the coalesced
     * blocks back into the free lists.
     */
    while (!TAILQ_EMPTY(&tmplist)) {
        mem = TAILQ_FIRST(&tmplist);
        TAILQ_REMOVE(&tmplist, mem, mem_list);

        TAILQ_FOREACH_SAFE(ent, &tmplist, mem_list, tmp) {
            if (END(mem) + 1 == ent->addr || END(ent) + 1 == mem->addr) {
                TAILQ_REMOVE(&tmplist, ent, mem_list);
                mem->addr = min(mem->addr, ent->addr);
                mem->size += ent->size;
                slab_cache_free(&mem_cache, ent);
            }
        }
        mem_free(pool, mem);
    }
    pool->coalesced = 1;
}

/*
 * Check whether a pool has any memory in it or not.  Returns a bool.
 */
int
mem_empty(struct mem_pool *pool)
{
    unsigned i;

    for (i = 0; i <= MEM_MAX_IDX; i++) {
        if (!TAILQ_EMPTY(FREE_LIST(pool, i))) {
            return 0;
        }
    }
    return 1;
}

/*
 * This function bootstraps the whole system.  This should only be called
 * during start up.
 */
void
__mem_pool_init(uintptr_t vbase, uintptr_t pbase, size_t size)
{
    __slab_cache_add(&mem_cache, vbase, pbase, size);
}

/*
 * Empty the pool.
 */
void
mem_clear(struct mem_pool *pool)
{
    struct memory *ent, *tmp;
    int idx;

    for (idx = 0; idx <= MEM_MAX_IDX; idx++) {
        TAILQ_FOREACH_SAFE(ent, FREE_LIST(pool, idx), mem_list, tmp) {
            TAILQ_REMOVE(FREE_LIST(pool, idx), ent, mem_list);
            slab_cache_free(&mem_cache, ent);
        }
    }
}
