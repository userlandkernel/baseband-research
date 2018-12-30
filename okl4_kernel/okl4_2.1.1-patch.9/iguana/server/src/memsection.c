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
 * Authors: Ben Leslie, Alex Webster Created: Tue Jul 6 2004 
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <l4/space.h>
#include <l4e/map.h>
#include <l4e/misc.h>
#include "util.h"
#include "memsection.h"
#include "objtable.h"
#include "pd.h"
#include "pgtable.h"
#include "slab_cache.h"
#include "tools.h"
#include "vm.h"

struct memsection_list internal_memsections =
    { (void *)&internal_memsections, (void *)&internal_memsections };

static struct slab_cache ms_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct memsection_node), &ms_cache);

struct memsection_node *
memsection_new(void)
{
    struct memsection_node *node;

    MEMSECTION_PRINT("memsection_new_called\n");
    node = slab_cache_alloc(&ms_cache);

    if (node == NULL) {
        MEMSECTION_PRINT("called slab cache alloc (fail)\n");
        return NULL;
    }

    memzero(node, sizeof(struct memsection_node));
    node->data.magic = MEMSECTION_MAGIC;
    return node;
}

/*
 * XXX: Should this go in another file? 
 */
void
memsection_delete(struct memsection *ms)
{
    int r;
    struct memsection_node *node;
    struct physmem *pm, *tmp_pm;
    uintptr_t paddr, vaddr;
    unsigned bits;

    assert(ms);
    node = (struct memsection_node *)((uintptr_t)ms - 2 * sizeof(void *));

    /*
     * Clean up virtual memory.
     */
    memsection_flush(ms);
    for (vaddr = ms->base; vaddr < ms->end; vaddr += (1UL << bits)) {
        if (!pt_lookup(vaddr, &bits, &paddr)) {
            bits = BASE_LOG2_PAGESIZE;
            continue;
        }
        pt_remove(vaddr, bits);
    }
    virtmem_delete(ms->virt);

    /*
     * Clean up any physical memory.
     */
    TAILQ_FOREACH_SAFE(pm, &ms->pm_list, pm_list, tmp_pm) {
        TAILQ_REMOVE(&ms->pm_list, pm, pm_list);
        physmem_delete(pm);
    }

    r = objtable_delete(ms);
    assert(r == 0);

#if defined(CONFIG_ZONE)
    /* XXX: Move elsewhere? */
    if (ms->zone) {
        TAILQ_REMOVE(&ms->zone->ms_list, ms, zone_list);
        zone_shrink(ms->zone, ms->base);
    }
#endif
    /* manually delete it from memsection_list */
    node->next->prev = node->prev;
    node->prev->next = node->next;

    /* free memsect data structure. */
    slab_cache_free(&ms_cache, node);
}

/* this should only be called when objtable_add() fails */
void
delete_memsection_from_allocator(struct memsection_node *node)
{
    assert(node);
    slab_cache_free(&ms_cache, node);
}

int
memsection_set_attributes(struct memsection *memsect, uintptr_t attributes)
{
    memsect->attributes = attributes;
    memsection_flush(memsect);
    pd_attach(&iguana_pd, memsect, L4_ReadWriteOnly);
    return 0;
}

#if defined(IG_DEBUG_PRINT)
void
memsection_print(struct memsection *self)
{
    DEBUG_PRINT("Memsection: %p\n", self);
    if (self == NULL) {
        return;
    }
    DEBUG_PRINT(" Owner:  %p\n", self->owner);
    DEBUG_PRINT(" Extent: <%p:%p>\n", (void *)self->base, (void *)self->end);
}
#endif

void
memsection_register_server(struct memsection *self, struct thread *server)
{
    assert(self);
    self->server = server;
}

#if defined(CONFIG_MEMLOAD)
static int
load_page(uintptr_t vaddr)
{
    DEBUG_PRINT("%s(%p): UNIMPLEMENTED!\n", __func__, (void *)vaddr);
    return 0;
}
#endif

int
memsection_lookup_phys(struct memsection *memsection, uintptr_t addr,
                       uintptr_t *paddr, size_t *size, struct pd *pd)
{
#if defined(CONFIG_ZONE)
    unsigned bit;
    unsigned end;
#endif
    unsigned bits;
    uintptr_t phys;
    int r;

    addr = round_down(addr, BASE_LOG2_PAGESIZE);

    r = pt_lookup(addr, &bits, &phys);
    if (r == 0) {
        return 0;
    }

    /*
     * If this is a MEM_LOAD memsection, we need to make sure the data is ready.
     */
#if defined(CONFIG_MEMLOAD)
    if (memsection->flags & MEM_LOAD && pd != &iguana_pd) {
        do {
            bit = (round_down(addr, 1UL << bits) - memsection->base) /
                BASE_PAGESIZE;
            end = bit + (1UL << (bits - BASE_LOG2_PAGESIZE));

            while (bit < end) {
                if (bfl_is_free(memsection->load_map, bit)) {
                    break;
                }
                bit++;
            }
            if (bit == end) {
                break;
            }

            bits--;
            phys += round_down(addr, 1UL << bits) % (1UL << (bits + 1));
        } while (bits >= BASE_LOG2_PAGESIZE);
        if (bits < BASE_LOG2_PAGESIZE) {
            return 0;
        }
    }
#endif
    if (paddr)
        *paddr = phys;
    if (size)
        *size = 1UL << bits;
    return 1;
}

int
memsection_page_map(struct memsection *self, L4_Fpage_t from_page,
                    L4_Fpage_t to_page)
{
    uintptr_t from_base, from_end, to_base, to_end;
    uintptr_t offset, phys, size;
    struct memsection *src;
    uintptr_t _size;

    MEMSECTION_PRINT("page_map(<%p:%p>, <%p:%p>)\n",
                     (void *)L4_Address(from_page),
                     (void *)(L4_Address(from_page) + L4_Size(from_page) - 1),
                     (void *)L4_Address(to_page),
                     (void *)(L4_Address(to_page) + L4_Size(to_page) - 1));

    if (!(self->flags & MEM_USER))
        return -1;
    if (L4_SizeLog2(from_page) != L4_SizeLog2(to_page))
        return -1;

    MEMSECTION_PRINT("seems ok...\n");

    _size = (1 << L4_SizeLog2(from_page)) - 1;
    from_base = L4_Address(from_page);
    from_end = from_base + _size;
    to_base = L4_Address(to_page);
    to_end = to_base + _size;

    src = objtable_lookup((void *)from_base);

    if (!src || src->flags & MEM_USER)  /* can't map from user-paged ms */
        return -1;
    if (src->flags & MEM_INTERNAL)
        return -1;
    if (to_base < self->base || to_end > self->end)
        return -1;

    size = from_end - from_base + 1;
    /*
     * XXX: we map 4k fpages even when bigger mappings are possible 
     */
    for (offset = 0; offset < size; offset += BASE_PAGESIZE) {
        size_t psize;
        int r;
        struct pd_entry *ent;

        r = memsection_lookup_phys(src, from_base + offset, &phys, &psize,
                                   &iguana_pd);
        if (!r) {
            return -1;
        }

        if (psize > BASE_PAGESIZE)
            phys += (from_base + offset) % psize;

        r = pt_insert(to_base + offset, BASE_LOG2_PAGESIZE, phys);
        if (!r) {
            return -1;
        }
        TAILQ_FOREACH(ent, &self->pd_list, pd_list) {
            r = pd_sync_range(ent->pd, to_base + offset,
                              to_base + offset + BASE_PAGESIZE - 1, NULL, 0);
            if (r != 0) {
                return -1;
            }
        }
    }

    MEMSECTION_PRINT("done...\n");
    return 0;
}

int
memsection_map(struct memsection *ms, uintptr_t offset, struct physmem *pm)
{
    L4_Fpage_t ppage, vpage;
    int r;
    struct pd_entry *ent;
    const uintptr_t pm_mem_size = pm->mem->size;
    const uintptr_t ms_base_offset = ms->base + offset;
    uintptr_t vbase = ms_base_offset;
    const uintptr_t vend = vbase + pm_mem_size - 1;
    uintptr_t pbase = pm->mem->addr;
    const uintptr_t pend = pbase + pm_mem_size - 1;
    uintptr_t vpage_size;
    
    if (!(ms->flags & MEM_USER)) {
        return -1;
    }
    if (ms_base_offset > ms->end) {
        return -1;
    }
    if (vend > ms->end) {
        return -1;
    }
    if (offset % BASE_PAGESIZE != 0) {
        return -1;
    }

    while (vbase < vend) {
        vpage = l4e_biggest_fpage(vbase, vbase, vend);
        ppage = l4e_biggest_fpage(pbase, pbase, pend);
        if (L4_SizeLog2(vpage) > L4_SizeLog2(ppage)) {
            vpage = L4_FpageLog2(vbase, L4_SizeLog2(ppage));
        } else {
            ppage = L4_FpageLog2(pbase, L4_SizeLog2(vpage));
        }
        r = pt_insert(vbase, L4_SizeLog2(vpage), pbase);
        if (!r) {
            return -1;
        }
        vpage_size = 1 << L4_SizeLog2(vpage);
        vbase += vpage_size;
        pbase += vpage_size;
    }

#if defined(CONFIG_ZONE)
    /*
     * If the memsection is in a zone, then push the mapping out to
     * all of the PDs that use the zone.
     */
    if (ms->zone != NULL) {
        if (zone_map_memsection(ms) != 0) {
            return -1;
        }
    }
#endif
    
    /*
     * Zero out the memsection if it is needed.
     */
    if (pm->need_scrub) {
        /*
         * Map the memsection into the iguana server.  All zones are
         * already attached to the iguana server, so no additional
         * work is needed in that case.
         */
#if defined(CONFIG_ZONE)
        if (ms->zone == NULL) {
#endif
            r = pd_sync_range(&iguana_pd, ms_base_offset, vend, ms,
                              L4_ReadWriteOnly);
            if (r != 0) {
                return -1;
            }
#if defined(CONFIG_ZONE)
        }
#endif
        memset((void *)(ms_base_offset), 0, pm_mem_size);
        pm->need_scrub = 0;
    }

    /*
     * Push the mapping out to all of the PDs that are attached to the
     * memsection.
     */
#if defined(CONFIG_ZONE)
    if (ms->zone == NULL) {
#endif
        TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
            r = pd_sync_range(ent->pd, ms_base_offset, vend, NULL, 0);
            if (r != 0) {
                return -1;
            }
        }
#if defined(CONFIG_ZONE)
    }
#endif
    return 0;
}

int
memsection_unmap(struct memsection *ms, uintptr_t offset, uintptr_t size)
{
    L4_Fpage_t fpage;
    struct pd_entry *ent;
    uintptr_t addr;
    uintptr_t sz;

    /*
     * FIXME: We don't deal with unaligned unmaps.  Also, we need to remove
     * any physical memory objects from the pm_list in the memsection.
     */
    if (offset % BASE_PAGESIZE != 0) {
        return -1;
    }
    if (size % BASE_PAGESIZE != 0) {
        return -1;
    }
    if (!(ms->flags & MEM_USER)) {
        return -1;
    }
    if (ms->base + offset + size - 1 > ms->end) {
        return -1;
    }

    addr = ms->base + offset;
    TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
        pd_flush_range(ent->pd, addr, addr + size - 1);
    }
    while (size > 0) {
        fpage = l4e_biggest_fpage(addr, addr, addr + size - 1);
        (void)pt_remove(addr, L4_SizeLog2(fpage));
        sz = 1 << L4_SizeLog2(fpage);
        addr += sz;
        size -= sz;
    }
    return 0;
}

#if !defined(NDEBUG)
int
memsection_check_state(struct memsection *self)
{
    return !is_memsection(self);
}
#endif

void
memsection_flush(struct memsection *ms)
{
    struct pd_entry *ent, *tmp = NULL;

    TAILQ_FOREACH_SAFE(ent, &ms->pd_list, pd_list, tmp) {
        pd_detach(ent->pd, ms);
    }
}

struct slab_cache pd_ent_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct pd_entry), &pd_ent_cache);

int
memsection_list_is_valid(struct memsection_list *list)
{
    struct memsection_node *ms, *first_ms = list->first;

    ms = first_ms;
    do {
        if (!is_memsection(&ms->data))
            return 0;
        ms = ms->next;
    } while (ms->next != first_ms);
    return 1;
}

struct memsection *
iguana_memsection_create(size_t size, uintptr_t *ret_base, uintptr_t *ret_size)
{
    struct memsection *ms;

    ms = pd_create_memsection(&iguana_pd, size, 0, default_physpool,
                              default_virtpool, NULL, MEM_NORMAL);
    if (ms) {
        *ret_base = ms->base;
        *ret_size = ms->end - ms->base + 1;
    }
    return ms;
}

int
memsection_rmap_insert(struct memsection *ms, struct pd *pd, int rwx)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
        if (ent->pd == pd) {
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
    TAILQ_INSERT_TAIL(&ms->pd_list, ent, pd_list);
    return 0;
}

void
memsection_rmap_remove(struct memsection *ms, struct pd *pd)
{
    struct pd_entry *ent;

    TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
        if (ent->pd == pd)
            break;
    }
    if (ent != NULL) {
        TAILQ_REMOVE(&ms->pd_list, ent, pd_list);
        slab_cache_free(&pd_ent_cache, ent);
    }
}

int
default_fault_handler(struct memsection *ms, uintptr_t addr, uintptr_t ip,
                      int rwx, struct pd *pd, struct thread *thread)
{
    return 0;
}

#if defined(CONFIG_MEMLOAD)
int
memload_fault_handler(struct memsection *ms, uintptr_t addr, uintptr_t ip,
                      int rwx, struct pd *pd, struct thread *thread)
{
    uintptr_t bit = (addr - ms->base) / BASE_PAGESIZE;
    int r;
    struct pd_entry *ent;

    /* Check whether the data has been loaded. */
    if (bfl_is_free(ms->load_map, bit)) {
        r = load_page(addr);
        if (r) {
            return -1;
        }
        bfl_alloc_bit(ms->load_map, bit);
        ms->load_count--;
        addr = round_down(addr, BASE_PAGESIZE);
        TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
            r = pd_sync_range(ent->pd, addr, addr + BASE_PAGESIZE - 1, NULL, 0);
            if (r != 0) {
                return -1;
            }
        }
    }

    /* If the whole section is loaded, then we revert to normal. */
    if (ms->load_count == 0) {
        ms->flags ^= MEM_LOAD;
        ms->fault_handler = NULL;
        bfl_destroy(ms->load_map);
        TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
            r = pd_sync_range(ent->pd, ms->base, ms->end, NULL, 0);
            if (r != 0) {
                return -1;
            }
        }
    }
    return 0;
}
#endif
