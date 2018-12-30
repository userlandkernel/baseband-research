#if defined(CONFIG_ZONE)
/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
#include <string.h>
#include <l4e/map.h>
#include <l4e/misc.h>
#include "util.h"
#include "objtable.h"
#include "pd.h"
#include "pgtable.h"
#include "virtmem.h"
#include "zone.h"

struct slab_cache zone_ent_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct zone_entry), &zone_ent_cache);

struct zone *
zone_create(struct pd *pd, struct virtpool *pool)
{
    struct zone *zone;

    zone = calloc(1, sizeof(*zone));
    if (!zone) {
        return NULL;
    }

    zone->magic = ZONE_MAGIC;
    zone->owner = pd;

    zone->pool = virtpool_create();
    assert(zone->pool);
    zone->morepool = pool;

    zone->phys_pool = physpool_create();
    assert(zone->phys_pool);
    zone->phys_morepool = default_physpool;

    zone->direct_pool = virtpool_create();
    assert(zone->direct_pool);

    TAILQ_INIT(&zone->window_list);
    TAILQ_INIT(&zone->ms_list);

    TAILQ_INSERT_TAIL(&pd->owned_zones, zone, zone_list);
    TAILQ_INIT(&zone->pd_list);

#if defined(ARM_SHARED_DOMAINS)
    zone->space = pd_create(&iguana_pd, 0);
    assert(zone->space);
#endif

    /* Zones are always visible to Iguana. */
    pd_attach_zone(&iguana_pd, zone, L4_FullyAccessible);

    return zone;
}

void
zone_delete(struct zone *zone)
{
    struct pd_entry *ent, *tmp;
    struct memsection *ms, *tmp_ms;
    struct virtmem *vm, *tmp_vm;

    /* Detach the zone from everyone while we clean up. */
    TAILQ_FOREACH_SAFE(ent, &zone->pd_list, pd_list, tmp) {
        pd_detach_zone(ent->pd, zone);
    }

    /* Get rid of all the memsections in the zone. */
    TAILQ_FOREACH_SAFE(ms, &zone->ms_list, zone_list, tmp_ms) {
        memsection_delete(ms);
    }

    /* Clean up the memory pools. */
    virtpool_delete(zone->pool);
    TAILQ_FOREACH_SAFE(vm, &zone->window_list, vm_list, tmp_vm) {
        virtmem_delete(vm);
    }

    /* Notify the owner of the pool. */
    TAILQ_REMOVE(&zone->owner->owned_zones, zone, zone_list);

#if defined(ARM_SHARED_DOMAINS)
    /* Get rid of the underlying space on ARM9. */
    pd_delete(zone->space);
#endif

    /* Free up the zone itself. */
    free(zone);
}

/*
 * The zone itself tracks what PDs have access to it.  This function new PD to
 * that set and marks it with the appropriate access rights.
 */
int
zone_rmap_insert(struct zone *zone, struct pd *pd, int rwx)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
        if (ent->pd == pd) {
            /* PD is already tracked, so update the rights. */
            ent->rwx = rwx;
            return 0;
        }
    }
    ent = slab_cache_alloc(&pd_ent_cache);
    if (ent == NULL) {
        return -1;
    }
    ent->pd = pd;
    ent->rwx = rwx;
    TAILQ_INSERT_TAIL(&zone->pd_list, ent, pd_list);
    return 0;
}

int
zone_rmap_lookup(struct zone *zone, struct pd *pd)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
        if (ent->pd == pd) {
            return ent->rwx;
        }
    }
    return 0;
}

void
zone_rmap_remove(struct zone *zone, struct pd *pd)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
        if (ent->pd == pd)
            break;
    }
    if (ent != NULL) {
        TAILQ_REMOVE(&zone->pd_list, ent, pd_list);
        slab_cache_free(&pd_ent_cache, ent);
    }
}

/*
 * XXX: These definitions should be moved somewhere else, but we really need
 * build system support for choosing which files to build based on config.
 */
#if defined(ARM_SHARED_DOMAINS)
/*
 * On ARM9 we just need to map the memsection into the space representing the
 * zone.
 */
static int
arch_zone_insert_memsection(struct zone *zone, struct memsection *ms)
{
    int r;

    r = pd_insert_memsection(zone->space, ms, L4_Readable);
    if (r != 0) {
        return -1;
    }
    return 0;
}
#else
/*
 * On non-ARM9 targets, we map the memsection into every PD with access to the
 * zone.
 */
static int
arch_zone_insert_memsection(struct zone *zone, struct memsection *ms)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
        pd_insert_memsection(ent->pd, ms, zone_rmap_lookup(zone, ent->pd));
    }
    return 0;
}
#endif

/*
 * Make the memsection visible in the zone.
 */
int
zone_insert_memsection(struct zone *zone, struct memsection *ms)
{
    ms->zone = zone;
    TAILQ_INSERT_TAIL(&zone->ms_list, ms, zone_list);
    return arch_zone_insert_memsection(zone, ms);
}

/*
 * Make the memsection visible in the zone.
 */
int
zone_map_memsection(struct memsection *ms)
{
    return arch_zone_insert_memsection(ms->zone, ms);
}

/*
 * Add free memory in a window to the zone.
 */
int
zone_add_memory(struct zone *zone, uintptr_t base, uintptr_t end)
{
    return virtpool_add_memory(zone->pool, base, end);
}

/*
 * Increase the pool of free memory in the zone (morecore).
 */
int
zone_expand(struct zone *zone, size_t size)
{
    int r;
    struct virtmem *vm;

    /* Try to allocate enough memory to create a new window. */
    size = round_up(size, MIN_WINDOW);
    vm = virtpool_alloc(zone->morepool, size);
    if (vm == NULL) {
        return -1;
    }
    /* If the memory is not suitably aligned, we can't use it. */
    if (vm_base(vm) % MIN_WINDOW) {
        virtmem_delete(vm);
        return -1;
    }

    /* Mark the whole range as available for new memsections. */
    r = virtpool_add_memory(zone->pool, vm_base(vm), vm_end(vm));
    if (r != 0) {
        virtmem_delete(vm);
        return -1;
    }

    return zone_add_window(zone, vm);
}

/*
 * Increase the pool of free direct memory in the zone.
 * This is analogous to morecore for physical, then doing morecore
 * for virtual at the same address.  Since we're getting it off
 * the direct pool, we should be able to get the same (base, size)
 * for both virtual and physical.
 */
int
zone_expand_direct(struct zone *zone, size_t size)
{
    int r;
    struct physmem *pm;
    struct virtmem *vm;

    /* Try to allocate enough memory to create a new window. */
    size = round_up(size, MIN_WINDOW);

    /* Allocate physical */
    pm = physpool_alloc(zone->phys_morepool, size);
    if (pm == NULL) {
        return -1;
    }
    /* If the memory is not suitably aligned, we can't use it. */
    if (pm_base(pm) % MIN_WINDOW) {
        physmem_delete(pm);
        return -1;
    }

    /* Mark the whole range as available for new memsections. */
    r = physpool_add_memory(zone->phys_pool, pm_base(pm), pm_end(pm));
    if (r != 0) {
        physmem_delete(pm);
        return -1;
    }

    /* Allocate virtual *from the direct pool* */
    vm = virtpool_alloc_fixed(default_directpool, size, pm->mem->addr);
    if (vm == NULL) {
        return -1;
    }
    /* If the memory is not suitably aligned, we can't use it. */
    if (vm_base(vm) % MIN_WINDOW) {
        virtmem_delete(vm);
        return -1;
    }

    /* Mark the whole range as available for new memsections. */
    r = virtpool_add_memory(zone->direct_pool, vm_base(vm), vm_end(vm));
    if (r != 0) {
        virtmem_delete(vm);
        return -1;
    }

    return zone_add_window(zone, vm);
}

int zone_add_window(struct zone *zone, struct virtmem *vm)
{
    TAILQ_INSERT_TAIL(&zone->window_list, vm, vm_list);

#if defined(ARM_SHARED_DOMAINS)
    /* On ARM9, we premap the window into all the 'clients' of the zone. */
    {
        uintptr_t addr;
        struct pd_entry *ent;
        int r;

        TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
            for (addr = vm_base(vm); addr <= vm_end(vm); addr += MIN_WINDOW) {
                r = !l4_map_window(pd_l4_space(ent->pd),
                                   pd_l4_space(zone->space),
                                   L4_Fpage(addr, MIN_WINDOW));
                if (r != 0) {
                    return -1;
                }
            }
        }
    }
#endif
    return 0;
}

/*
 * Zone shrink attempts to free up any memory around base (which is typically
 * where the last memsection was just deleted).
 */
int
zone_shrink(struct zone *zone, uintptr_t base)
{
    /* Removed in late testing.  It seems to unmap a window when any
     * memsection within the window is deleted, rather when when all
     * of the memsections are deleted.
     */
#if 0
    int r = -1;
    struct virtmem *vm;

    /* Find the window which covers base (if any). */
    TAILQ_FOREACH(vm, &zone->window_list, vm_list) {
        if (vm_base(vm) <= base && base <= vm_end(vm)) {
            break;
        }
    }
    if (vm == NULL) {
        return -1;
    }

    /* Try to remove the memory from the free pool. */
    TAILQ_REMOVE(&zone->window_list, vm, vm_list);
    r = virtpool_remove(zone->pool, vm_base(vm), vm_end(vm));
    if (r != 0) {
        /* The memory musn't be free, so give up. */
        return -1;
    }

#if defined(ARM_SHARED_DOMAINS)
    {
        uintptr_t addr;
        struct pd_entry *ent;

        TAILQ_FOREACH(ent, &zone->pd_list, pd_list) {
            for (addr = vm_base(vm); addr <= vm_end(vm); addr += MIN_WINDOW) {
                r = !l4_unmap_window(pd_l4_space(ent->pd),
                                     pd_l4_space(zone->space),
                                     L4_Fpage(addr, MIN_WINDOW));
                if (r != 0) {
                    return -1;
                }
            }
        }
    }
#endif

    /* The memory was freed up, so we can return the window to the source. */
    virtmem_delete(vm);

#endif
    return 0;
}
#endif
