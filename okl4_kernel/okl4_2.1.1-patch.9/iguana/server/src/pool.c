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
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "alloc.h"
#include "physmem.h"
#include "slab_cache.h"
#include "virtmem.h"
#include "vm.h"

struct physpool {
    struct mem_pool alloc;
};

struct virtpool {
    struct mem_pool alloc;
};

struct virtpool *default_directpool;
struct physpool *default_physpool;
struct virtpool *default_virtpool;

struct slab_cache vm_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct virtmem), &vm_cache);

struct slab_cache pm_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct physmem), &pm_cache);

int pool_init(uintptr_t vbase, uintptr_t vend, uintptr_t pbase, uintptr_t pend,
              struct physpool **physpool, struct virtpool **virtpool,
              struct virtpool **directpool);

int
pool_init(uintptr_t vbase, uintptr_t vend, uintptr_t pbase, uintptr_t pend,
          struct physpool **physpool, struct virtpool **virtpool,
          struct virtpool **directpool)
{
    addr_t addr;

    assert(vend - vbase + 1 >= 0x1000);
    assert(pend - pbase + 1 >= 0x1000);

    __mem_pool_init(vbase, pbase, 0x1000);

    default_virtpool = virtpool_create();
    internal_virtpool = &default_virtpool->alloc;
    default_physpool = physpool_create();
    internal_physpool = &default_physpool->alloc;
    default_directpool = virtpool_create();

    if (vbase < vend) {
        DEBUG_PRINT("vbase is 0x%"PRIxPTR", vend is 0x%"PRIxPTR"\n", vbase, vend);
        virtpool_add_memory(default_virtpool, vbase, vend);
        addr = mem_alloc_unboxed(&default_virtpool->alloc, 0x1000);
        DEBUG_PRINT("addr is 0x%"PRIxPTR", vbase is 0x%"PRIxPTR"\n", addr, vbase);
        assert(addr == vbase);
    }

    if (pbase < pend) {
        physpool_add_memory(default_physpool, pbase, pend);
        addr = mem_alloc_unboxed(&default_physpool->alloc, 0x1000);
        assert(addr == pbase);
    }

    *virtpool = default_virtpool;
    *physpool = default_physpool;
    *directpool = default_directpool;
    return 0;
}

struct physpool *
physpool_create(void)
{
    return calloc(1, sizeof(struct physpool));
}

int
physpool_add_memory(struct physpool *pool, uintptr_t base, uintptr_t end)
{
    return mem_add(&pool->alloc, base, end - base + 1);
}

struct physmem *
physpool_add_alloced_memory(struct physpool *pool, uintptr_t base, uintptr_t end)
{
    struct physmem *pm;

    pm = slab_cache_alloc(&pm_cache);
    if (pm != NULL) {
        pm->mem = mem_add_and_alloc(&pool->alloc, base, end - base + 1);
        if (pm->mem == NULL) {
            slab_cache_free(&pm_cache, pm);
            return NULL;
        }
        pm->pool = pool;
        pm->need_scrub = 1;
    }
    return pm;
}


struct virtpool *
virtpool_create(void)
{
    return calloc(1, sizeof(struct virtpool));
}

int
virtpool_add_memory(struct virtpool *pool, uintptr_t base, uintptr_t end)
{
    return mem_add(&pool->alloc, base, end - base + 1);
}

struct virtmem *
virtpool_add_alloced_memory(struct virtpool *pool, uintptr_t base, uintptr_t end)
{
    struct virtmem *vm;

    vm = slab_cache_alloc(&vm_cache);
    if (vm != NULL) {
        vm->mem = mem_add_and_alloc(&pool->alloc, base, end - base + 1);
        if (vm->mem == NULL) {
            slab_cache_free(&vm_cache, vm);
            return NULL;
        }
        vm->pool = pool;
    }
    return vm;
}

struct physmem *
physpool_alloc(struct physpool *pool, size_t size)
{
    struct physmem *pm;

    pm = slab_cache_alloc(&pm_cache);
    if (pm != NULL) {
        pm->mem = mem_alloc(&pool->alloc, size);
        if (pm->mem == NULL) {
            slab_cache_free(&pm_cache, pm);
            return NULL;
        }
        pm->pool = pool;
        pm->need_scrub = 1;
    }
    return pm;
}

struct physmem *
physpool_alloc_fixed(struct physpool *pool, size_t size, uintptr_t addr)
{
    struct memory *mem;
    struct physmem *pm = NULL;

    mem = mem_alloc_fixed(&pool->alloc, size, addr);
    if (mem != NULL) {
        pm = slab_cache_alloc(&pm_cache);
        if (pm == NULL) {
            mem_free(&pool->alloc, mem);
            return NULL;
        }
        pm->mem = mem;
        pm->pool = pool;
        pm->need_scrub = 1;
    }
    return pm;
}

void
physmem_delete(struct physmem *pm)
{
    // DEBUG_PRINT("%s: %p = (%p, %p)\n", __func__, pm, pm->pool, pm->mem);
    mem_free(&pm->pool->alloc, pm->mem);
    slab_cache_free(&pm_cache, pm);
}

struct virtmem *
virtpool_alloc(struct virtpool *pool, size_t size)
{
    struct virtmem *vm;

    vm = slab_cache_alloc(&vm_cache);
    if (vm != NULL) {
        vm->mem = mem_alloc(&pool->alloc, size);
        if (vm->mem == NULL) {
            slab_cache_free(&vm_cache, vm);
            return NULL;
        }
        vm->pool = pool;
    }
    return vm;
}

struct virtmem *
virtpool_alloc_fixed(struct virtpool *pool, size_t size, uintptr_t addr)
{
    struct memory *mem;
    struct virtmem *vm = NULL;

    mem = mem_alloc_fixed(&pool->alloc, size, addr);
    if (mem != NULL) {
        vm = slab_cache_alloc(&pm_cache);
        if (vm == NULL) {
            mem_free(&pool->alloc, mem);
            return NULL;
        }
        vm->mem = mem;
        vm->pool = pool;
    }
    return vm;
}

void
virtpool_delete(struct virtpool *pool)
{
    mem_clear(&pool->alloc);
    free(pool);
}

int
virtpool_remove(struct virtpool *pool, uintptr_t base, uintptr_t end)
{
    return mem_remove(&pool->alloc, base, end - base + 1);
}

void
virtmem_delete(struct virtmem *vm)
{
    mem_free(&vm->pool->alloc, vm->mem);
    slab_cache_free(&vm_cache, vm);
}

#if defined(CONFIG_STATS)
void
pool_stats(size_t malloc_overhead, size_t* pool_size, size_t* pool_mem_size)
{
    *pool_size     = sizeof(struct physpool) + malloc_overhead;
    *pool_mem_size = sizeof(struct memory);
}
#endif
