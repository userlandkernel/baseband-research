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
 * Author: Ben Leslie, Alex Webster 
 */
#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <hash/hash.h>
#include <compat/c.h>
#include <l4e/map.h>
#include <l4e/misc.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/security.h>
#include <l4/map.h>
#include <l4/config.h>
#include "clist.h"
#include "util.h"
#include "memsection.h"
#include "objtable.h"
#include "pd.h"
#include "pgtable.h"
#include "physmem.h"
#if defined(CONFIG_SESSION)
#include "session.h"
#endif
#include "space.h"
#include "tools.h"
#include "thread.h"
#include "virtmem.h"
#include "vm.h"
#if defined(CONFIG_ZONE)
#include "zone.h"
#endif

ALIGNED(8) struct pd iguana_pd;
int iguana_pd_inited;
static struct hashtable *l4spaceid_to_pd;

#define SPACE_HASHSIZE          256

struct slab_cache ms_ent_cache =
SLAB_CACHE_INITIALIZER(sizeof(struct ms_entry), &ms_ent_cache);

/*
 * Utcb information 
 */
//uintptr_t utcb_size_log2;
uintptr_t utcb_size;

static L4_Fpage_t
setup_utcb_area(struct pd *self, unsigned int threads)
{
    /* Find the size of utcb */
    struct memsection *utcb_obj;
    uintptr_t area_size, fpage_size;

    /*
     * FIXME: (benjl) We should look at the utcb min size and alignment here,
     * but for now we assume that page alignment is good enough 
     */
    area_size = utcb_size * threads;

    /* Find the Fpage size */
    for (fpage_size = 1U; fpage_size < area_size; fpage_size = fpage_size << 1) {
    }
#if defined(ARCH_ARM) && ARCH_VER <= 5
#define ONE_MB (1024 * 1024)
    fpage_size = (((fpage_size - 1) / ONE_MB) + 1) * ONE_MB;
#endif
    utcb_obj = pd_create_memsection(self, fpage_size, 0, NULL, default_virtpool, NULL, MEM_UTCB);
    //assert(utcb_obj); assert((utcb_obj->base % fpage_size) == 0);
    if (!utcb_obj)
        return L4_Nilpage;
    if ((utcb_obj->base % fpage_size))
        return L4_Nilpage;

    return L4_Fpage(utcb_obj->base, fpage_size);
}

#if defined(CONFIG_MEM_PROTECTED)
static int
pd_setup_ext(struct pd *pd)
{
    struct ms_entry *ent;
    L4_SpaceId_t base_space, ext_space;
    int r;
    struct thread *thread;
    L4_Fpage_t utcb_area;

    /* Check if the extensions is already setup. */
    if (!L4_IsNilSpace(pd_ext_l4_space(pd))) {
        return 0;
    }

    /* We don't set up extensions for the Iguana PD for now. */
    if (pd == &iguana_pd) {
        return 0;
    }

    pd->extension = malloc(sizeof(*pd->extension));
    if (pd->extension == NULL) {
        goto error;
    }

    utcb_area = setup_utcb_area(pd, MAX_THREADS_PER_PD_EXT);
    if (L4_IsNilFpage(utcb_area)) {
        goto error;
    }

    // FIXME: We need to cleanup after errors from here on - AGW.
    r = space_setup(pd->extension, PD_EXT_MAGIC, pd, utcb_area, 1, 0);
    if (r != 0) {
        goto error;
    }

    r = hash_insert(l4spaceid_to_pd, L4_SpaceNo(pd->extension->id), pd);
    if (r == -1) {
        goto error;
    }

    base_space = pd_l4_space(pd);
    ext_space = pd_ext_l4_space(pd);

#if defined(ARM_SHARED_DOMAINS)
    r = !l4_share_domain(ext_space, base_space);
#endif

    TAILQ_FOREACH(ent, &pd->ms_list, ms_list) {
        r = pd_sync_range(pd, ent->ms->base, ent->ms->end, ent->ms, 0);
        if (r) {
            goto error;
        }
    }

    /* Enable space switching between base and extension spaces of PD. */
    r = L4_AllowSpaceSwitch(base_space, ext_space);
    if (r != 1) {
        goto error;
    }
    r = L4_AllowSpaceSwitch(ext_space, base_space);
    if (r != 1) {
        goto error;
    }

    thread = thread_list_create_back(&pd->threads);
    if (thread == NULL) {
        goto error;
    }

    thread->owner = pd;

#ifdef NO_UTCB_RELOCATE
    thread->utcb = (void *)-1;
#else
    thread->utcb = (void *)(L4_Address(pd->extension->utcb_area) +
                            L4_GetUtcbSize());
#endif

    r = thread_alloc(thread);
    if (r != 0) {
        goto thread_error;
    }
    r = thread_new(thread, ext_space, L4_rootserver, L4_rootserver, L4_rootserver);

    if (r != 1) {
        goto thread_error;
    }
#if defined(IGUANA_DEBUG)
    L4_KDB_SetThreadName(thread->id, "**EXT**");
#endif
    return 0;

thread_error:
    thread_free(thread->id);

error:
    free(pd->extension);
    pd->extension = NULL;
    return -1;
}

static struct memsection *protected_memsection = NULL;

#endif

void
utcb_init(void)
{
    utcb_size = L4_GetUtcbSize();
}

static struct pd *
pd_setup(struct pd *self, struct pd *parent, unsigned int max_threads)
{
    self->space.magic = PD_MAGIC;
    self->space.owner = parent;
#if defined(CONFIG_MEM_PROTECTED)
    self->extension = NULL;
#endif
    self->local_threadno = bfl_new((uintptr_t)max_threads);
    self->callback_buffer = NULL;
    /*
     * initialize cb_alloc_handler, ignore it will cause problem when doing
     * pd_delete()
     */
    self->cba = NULL;

    thread_list_init(&self->threads);
    pd_list_init(&self->pds);
#if defined(CONFIG_SESSION)
    session_p_list_init(&self->sessions);
#endif
    memsection_list_init(&self->memsections);
#if defined(CONFIG_EAS)
    eas_list_init(&self->eass);
#endif
    clist_list_init(&self->clists);
    TAILQ_INIT(&self->pm_list);
    TAILQ_INIT(&self->ms_list);
    TAILQ_INIT(&self->owned_clists);
#if defined(CONFIG_ZONE)
    TAILQ_INIT(&self->owned_zones);
    TAILQ_INIT(&self->attached_zones);
#endif

    return self;
}

void
pd_init(void)
{
    L4_Fpage_t utcb_area = L4_Nilpage;

    (void)pd_setup(&iguana_pd, &iguana_pd, 10); /* Setup the iguana_pd with
                                                 * itself as parent */

    (void)space_setup(&iguana_pd.space, PD_MAGIC, &iguana_pd, utcb_area, 1, 0);
    l4spaceid_to_pd = hash_init(SPACE_HASHSIZE);
    hash_insert(l4spaceid_to_pd, L4_SpaceNo(L4_rootspace), &iguana_pd);
}

void
pd_setup_callback(struct pd *self, struct memsection *callback_buffer)
{
    int r;

    if (callback_buffer != NULL) {
        self->callback_buffer = callback_buffer;

        /* We don't want to fault on this memory */
        r = pd_attach(&iguana_pd, callback_buffer, L4_ReadWriteOnly);
        assert(r == 0);

        self->cba = cb_new_withmem((void *)callback_buffer->base,
                                   callback_buffer->end -
                                   callback_buffer->base);
    } else {
        self->callback_buffer = NULL;
        self->cba->cb = NULL;
        cb_free(self->cba);
        self->cba = NULL;
    }
}

int
pd_setup_user_map(struct pd *self, struct memsection *phys)
{
    uintptr_t paddr;
    size_t tmp_size;
    struct pd tmp_pd;
    int res;

    memsection_lookup_phys(phys, phys->base, &paddr, &tmp_size, &tmp_pd);

    /*
     * We don't use the returned size value since the underlying memsection is
     * split into contiguous 0x400000 sized segments, we only care about the
     * total size of the memsection
     */
    res = L4_AllowUserMapping(self->space.id, paddr, phys->end - phys->base + 1);
    if (!res) {
        DEBUG_PRINT("Unable to set memsection for user mapping, paddr %" PRIxPTR ", size %zu, pd %p\n",
                paddr, tmp_size, self);
    }

    return res;
}

int
pd_setup_platform_control(struct pd *self)
{
    int res;

    res = L4_AllowPlatformControl(self->space.id);

    if (!res) {
        DEBUG_PRINT("Unable to grant platform control access to pd %p\n", self);
    }

    return res;
}

struct pd *
pd_create(struct pd *self, unsigned int max_threads)
{
    L4_Fpage_t utcb_area;
    L4_Word_t r;
    struct pd *new_pd;

    PD_PRINT("called (%p, %u)\n", self, max_threads);

    if (max_threads >= MAX_THREADS_PER_APD) {
        PD_PRINT("too many threads. Returning NULL\n");
        return NULL;
    }
    if (self == NULL) {
        PD_PRINT("self is NULL. Returning NULL\n");
        return NULL;
    }
    if (max_threads == 0) {
        max_threads = MAX_THREADS_PER_APD;
    }

    new_pd = pd_list_create_back(&self->pds);

    if (new_pd == NULL) {
        PD_PRINT("pd_list_create_back failed. Returning NULL\n");
        return NULL;
    }

    (void)pd_setup(new_pd, self, max_threads);
    utcb_area = setup_utcb_area(new_pd, max_threads);
    if (L4_IsNilFpage(utcb_area)) {
        pd_list_delete(new_pd);
        return NULL;
    }
    r = space_setup(&new_pd->space, PD_MAGIC, self, utcb_area, 1, 0);
    if (r != 0) {
        memsection_delete(objtable_lookup((void *)L4_Address(utcb_area)));
        pd_list_delete(new_pd);
        return NULL;
    }
    assert(hash_lookup(l4spaceid_to_pd, L4_SpaceNo(new_pd->space.id)) == NULL);
    r = hash_insert(l4spaceid_to_pd, L4_SpaceNo(new_pd->space.id), new_pd);
    if (r == -1) {
        pd_delete(new_pd);
        return NULL;
    }
    return new_pd;
}

int
pd_delete(struct pd *pd)
{
    struct thread_node *thread;
    struct thread *dead_thread;
    struct pd_node *pds;
    struct pd *dead_pds;
#if defined(CONFIG_EAS)
    struct eas_node *eas;
    struct eas *dead_eas;
#endif
    struct memsection_node *memsection;
#if defined(CONFIG_SESSION)
    struct session_p_node *session;
    struct session **dead_session;
#endif
    struct ms_entry *ent;
    struct ms_entry *tmp = NULL;
    struct physmem *pm, *tmp_pm;
    struct clist *clist, *tmp_clist;
#if defined(CONFIG_ZONE)
    struct zone *zone;
    struct zone *tmp_zone = NULL;
    struct zone_entry *zent;
    struct zone_entry *ztmp = NULL;
#endif
    hash_remove(l4spaceid_to_pd, L4_SpaceNo(pd->space.id));

    /* Delete callback_buffer */
    if (pd->callback_buffer)
        memsection_delete(pd->callback_buffer);

    /* Delete cba */
    if (pd->cba) {
        pd->cba->cb = NULL;     /* XXX: the buffer is deleted above */
        cb_free(pd->cba);
    }

    TAILQ_FOREACH_SAFE(ent, &pd->ms_list, ms_list, tmp) {
        pd_detach(pd, ent->ms);
    }

    /* Delete explicitly allocated physical memory. */
    TAILQ_FOREACH_SAFE(pm, &pd->pm_list, pm_list, tmp_pm) {
        TAILQ_REMOVE(&pd->pm_list, pm, pm_list);
        physmem_delete(pm);
    }

    /* Delete memsections */
    for (memsection = pd->memsections.first;
         memsection->next != pd->memsections.first;
         memsection = pd->memsections.first) {
        /*
         * FIXME: How can this actually work? 
         */
        /* do not need to remove it from list, memsection_delete will do it by hand. */
        memsection_delete(&(memsection->data));
    }

#if defined(CONFIG_EAS)
    /* Delete all external as's */
    /*
     * FIXME: We really should have a decent iterator 
     */
    for (eas = pd->eass.first; eas->next != pd->eass.first;) {
        dead_eas = &eas->data;
        eas = eas->next;
        eas_delete(dead_eas);
    }
#endif
    
    for (pds = pd->pds.first; pds->next != pd->pds.first;) {
        dead_pds = &pds->data;
        pds = pds->next;
        (void)pd_delete(dead_pds);
    }

#if defined(CONFIG_SESSION)
    /* Delete sessions */
    for (session = pd->sessions.first; session->next != pd->sessions.first;) {
        dead_session = &session->data;
        session = session->next;
        session_delete(*dead_session);
    }
#endif
    /* Delete all threads */
    /*
     * FIXME: We really should have a decent iterator 
     */
    for (thread = pd->threads.first; thread->next != pd->threads.first;) {
        dead_thread = &thread->data;
        thread = thread->next;
        thread_delete(dead_thread);
    }

    TAILQ_FOREACH_SAFE(clist, &pd->owned_clists, clist_list, tmp_clist) {
        server_clist_delete(clist);
    }

    /* Delete local_threadno */
    if (pd->local_threadno)
        free(pd->local_threadno);
#if defined(CONFIG_ZONE)
    /* Clean up any zone mess */
    TAILQ_FOREACH_SAFE(zone, &pd->owned_zones, zone_list, tmp_zone) {
        zone_delete(zone);
    }

    TAILQ_FOREACH_SAFE(zent, &pd->attached_zones, zone_list, ztmp) {
        pd_detach_zone(pd, zent->zone);
    }
#endif
    space_cleanup(&pd->space);

    /* FIXME: Clean up extension if any. */

    pd_list_delete(pd);

    return 0;
}

struct memsection *
pd_create_memsection(struct pd *pd, uintptr_t size, uintptr_t base,
                     struct physpool *physpool, struct virtpool *virtpool,
                     struct zone *zone, uintptr_t flags)
{
    int r = 0;
    struct memsection *ms;
    struct memsection_list *list;
    struct memsection_node *node;
    struct physmem *pm = NULL;
    struct physmem *tmp_pm = NULL;
    struct virtmem *vm = NULL;
    struct virtpool *directpool = default_directpool;
    uintptr_t paddr;
    uintptr_t vaddr;
    unsigned bits;

    PD_PRINT("pd_create_memsection (%p, %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR
             ")\n", pd, size, base, flags);

    base = page_round_down(base);
    size = page_round_up(size);

    node = memsection_new();
    if (node == NULL) {
        PD_PRINT("memsection_new() returned null\n");
        return NULL;
    }

    ms = &(node->data);
    ms->owner = pd;
    ms->flags = flags & ~MEM_INTERNAL;
#if defined(CONFIG_ZONE)
    if (zone) {
        virtpool = zone->pool;
    }
#endif
    /*
     * Set up the virtual side of things...
     */
#if defined(CONFIG_ZONE)
alloc_virt:
#endif
    if (flags & (MEM_NORMAL | MEM_UTCB)) {
        PD_PRINT("MEM_NORMAL | MEM_UTCB : virtpool_alloc\n");
        vm = virtpool_alloc(virtpool, size);
#if defined(CONFIG_ZONE)
        if (vm == NULL && zone) {
            r = zone_expand(zone, size);
            if (!r) {
                goto alloc_virt;
            }
            /*
             * Error occured when trying to expand the zone, suggesting that we
             * really did run out of memory
             */
            else {
                return NULL; /* Not sure if this is the right way to handle ENOMEM - nt */
            }
        }
#endif
    } else if (flags & MEM_FIXED) {
        PD_PRINT("MEM_FIXED: virtpool_alloc_fixed\n");
        vm = virtpool_alloc_fixed(virtpool, size, base);
        if (vm == NULL && flags & MEM_INTERNAL) {
            vm = virtpool_add_alloced_memory(virtpool, base, base + size - 1);
            assert(vm != NULL && vm->mem->addr == base);
        }
    } else if (flags & MEM_DIRECT) {
#if defined(CONFIG_ZONE)
        if (zone) {
            physpool = zone->phys_pool;
            directpool = zone->direct_pool;
        }
#endif
        pm = physpool_alloc(physpool, size);
        if (pm != NULL) {
            vm = virtpool_alloc_fixed(directpool, size, pm->mem->addr);
        }
#if defined(CONFIG_ZONE)
        else if (pm == NULL && zone) {
            r = zone_expand_direct(zone, size);
            /* Really ran out of memory */
            if (r) {
                return NULL;
            }

            pm = physpool_alloc(physpool, size);
            /*
             * zone_expand_direct should have expanded my physpool, otherwise
             * I would have bailed out by now
             */
            if (pm == NULL) {
                return NULL;
            }

            vm = virtpool_alloc_fixed(directpool, size, pm->mem->addr);
            /*
             * zone_expand_direct should have expanded my direct pool, otherwise
             * I would have bailed out by now
             */
            if (vm == NULL) {
                return NULL;
            }
        }
#endif
    }
#if defined(CONFIG_ZONE)
    if (vm == NULL && zone) {
        r = zone_expand(zone, size);
        if (!r) {
            goto alloc_virt;
        }
    }
#endif
    if (vm == NULL) {
        PD_PRINT("failed to alloc from virtpool\n");
        if (pm != NULL) {
            physmem_delete(pm);
        }
        delete_memsection_from_allocator(node);
        return NULL;
    }
    ms->base = vm->mem->addr;
    ms->end = vm->mem->addr + size - 1;
    ms->virt = vm;

    /*
     * ... and now the physical side.
     */
    if ((flags & (MEM_DIRECT | MEM_USER | MEM_UTCB)) == 0) {
        for (vaddr = ms->base; vaddr <= ms->end; vaddr += (1UL << bits)) {
            bits = L4_SizeLog2(l4e_biggest_fpage(vaddr, vaddr, ms->end));
            do {
                pm = physpool_alloc(physpool, 1UL << bits);
            } while (pm == NULL && --bits >= MEM_MIN_BITS);
            if (pm == NULL) {
                PD_PRINT("falled to alloc from physpool\n");
                goto error;
            }
            r = pt_insert(vaddr, bits, pm->mem->addr);
            if (r != 1) {
                PD_PRINT("pt_insert failed\n");
                goto error;
            }
            TAILQ_INSERT_TAIL(&ms->pm_list, pm, pm_list);
        }
    } else if (flags & MEM_DIRECT) {
        for (vaddr = ms->base; vaddr <= ms->end; vaddr += (1UL << bits)) {
            bits = L4_SizeLog2(l4e_biggest_fpage(vaddr, vaddr, ms->end));
            r = pt_insert(vaddr, bits, pm->mem->addr + vaddr - ms->base);
            if (r != 1) {
                PD_PRINT("pt_insert failed\n");
                goto error;
            }
        }
        TAILQ_INSERT_TAIL(&ms->pm_list, pm, pm_list);
    }
    ms->pool = physpool;
#if defined(CONFIG_MEM_PROTECTED)
    if (flags & MEM_PROTECTED) {
        if (protected_memsection != NULL) {
            /* Only one protected memsection supported. */
            goto error;
        }
        protected_memsection = ms;
    }
#endif
    if ((flags & MEM_UTCB) == 0) {
#if defined(CONFIG_ZONE)
        if (zone) {
            r = zone_insert_memsection(zone, ms);
            if (r != 0) {
                goto error;
            }
        } else
#endif
        {
            r = pd_attach(&iguana_pd, ms, L4_ReadWriteOnly);
            if (r != 0) {
                goto error;
            }
        }
        if ((flags & MEM_USER) == 0) {
            memzero((void *)ms->base, ms->end - ms->base + 1);
        }
    }

    r = objtable_insert(ms);
    if (r != 0) {
        PD_PRINT("objtable_insert failed\n");
        goto error;
    }

    ms->owner = pd;
#if defined(CONFIG_MEMLOAD)
    if (flags & MEM_LOAD) {
        ms->load_count = size / l4e_min_pagesize();
        ms->load_map = bfl_new(ms->load_count);
        ms->fault_handler = memload_fault_handler;
    } else {
        ms->fault_handler = default_fault_handler;
    }
#endif

    /* manually append memsection to memsection_list.  */
    list = &pd->memsections;
    node->next = (void *)list;
    list->last->next = node;
    node->prev = list->last;
    list->last = node;

    PD_PRINT("memsection created successfully\n");
    return ms;


error:
    memsection_flush(ms);
    for (vaddr = ms->base; vaddr < ms->end; vaddr += (1UL << bits)) {
        if (!pt_lookup(vaddr, &bits, &paddr)) {
            break;
        }
        pt_remove(vaddr, bits);
    }
    TAILQ_FOREACH_SAFE(pm, &ms->pm_list, pm_list, tmp_pm) {
        TAILQ_REMOVE(&ms->pm_list, pm, pm_list);
        physmem_delete(pm);
    }
    virtmem_delete(ms->virt);
    delete_memsection_from_allocator(node);
    PD_PRINT("memsection create failed!\n");
    return NULL;
}

#if defined(CONFIG_SESSION)
struct session *
pd_create_session(struct pd *self, struct thread *client,
                  struct thread *server, struct clist *clist)
{
    struct session *session = NULL;
    struct pd *other;
    uintptr_t r;

    assert(is_pd(self));
    assert(is_thread(client));
    assert(is_thread(server));
    assert(is_clist(clist));

    other = server->owner;

    /* Create session */
    session = malloc(sizeof(struct session));

    if (session == NULL) {
        return NULL;
    }

    session->owner = self;
    /* Add it to our pd */

    session->owner_node = session_p_list_create_back(&self->sessions);

    if (session->owner_node == NULL) {
        /* We have run out of memory!  */
        free(session);
        return NULL;
    }

    *session->owner_node = session;

    session->magic = SESSION_MAGIC;
    session->call_buf = NULL;
    session->return_buf = NULL;

    /* Setup fields */
    session->client = client;
    session->server = server;
    session->clist = clist;

    /* Add it to client and server */

    session->server_node = session_p_list_create_back(&client->server_sessions);
    if (session->server_node == NULL) {
        /* We have run out of memory!  */
        session_p_list_delete(session->owner_node);
        free(session);
        return NULL;
    }

    *session->server_node = session;

    session->client_node = session_p_list_create_back(&server->client_sessions);

    if (session->client_node == NULL) {
        /* We have run out of memory!  */
        session_p_list_delete(session->owner_node);
        session_p_list_delete(session->server_node);
        free(session);
        return NULL;
    }

    *session->client_node = session;

    r = pd_add_clist(other, clist);

    if (r == 0) {
        /* Need to clean everything up to this point */
        session_p_list_delete(session->owner_node);
        session_p_list_delete(session->client_node);
        session_p_list_delete(session->server_node);
        free(session);
        return NULL;
    }
#if 0
    /*
     * FIXME: Maybe we need to let the server know, maybe not 
     */
    {
        /*
         * Zero timeout ping -- FIXME: This should be handled by idl upcall
         * code, but do this later 
         */
        L4_Msg_t msg;
        uintptr_t *size;
        uintptr_t *addr;

        /* Now notify client and server prot domains */
        size = cb_alloc(other->cba, sizeof(uintptr_t));
        if (size == NULL) {
            ERROR_PRINT("Couldn't do upcall");
            return NULL;        /* Error */
        }
        *size = 2 * sizeof(uintptr_t);
        addr = cb_alloc(other->cba, *size);
        addr[0] = 0x37;
        addr[1] = (uintptr_t)client->id.raw;
        cb_sync_alloc(other->cba);

        L4_MsgClear(&msg);
        L4_MsgLoad(&msg);
        L4_Send_Timeout(other->threads.first->data.id, L4_ZeroTime);
    }
#endif
    return session;
}
#endif

#if defined(CONFIG_EAS)
struct eas *
pd_create_eas(struct pd *self, L4_Fpage_t utcb, int pid, L4_SpaceId_t *l4_id)
{
    struct eas *eas;
    int r;

    eas = eas_list_create_back(&self->eass);
    if (eas != NULL) {
        r = eas_setup(eas, self, utcb, pid, l4_id);
        if (r != 0) {
            eas_list_delete(eas);
            return NULL;
        }
    }
    return eas;
}
#endif

struct thread *
pd_create_thread(struct pd *self, int priority)
{
    struct thread *thread;
    uintptr_t local_tid = 0;

    assert(self != NULL);

    thread = thread_list_create_back(&self->threads);

    if (thread == NULL) {
        return NULL;
    }
    thread->id = L4_nilthread;
    thread->handle = L4_nilthread;

    local_tid = bfl_alloc(self->local_threadno);
    if (local_tid == (uintptr_t)-1) {
        goto thread_error_state_1;
    }
    thread->owner = self;
    thread->utcb =
        (void *)(L4_Address(self->space.utcb_area) +
                 (utcb_size * (unsigned int)local_tid));

    if (thread_setup(thread, priority) != 0) {
        /* Need to clean up */
        goto thread_error_state_2;
    }

    /* thread_setup() could change the utcb on us.  If so free up the local_tid */
    if (thread->utcb == (void *)-1UL) {
        bfl_free(self->local_threadno, local_tid);
    }

    return thread;

  thread_error_state_1:
    thread->id = L4_nilthread;
    thread->handle = L4_nilthread;
    thread_list_delete(thread);
    return NULL;

  thread_error_state_2:
    bfl_free(self->local_threadno, (unsigned int)local_tid);
    thread->id = L4_nilthread;
    thread->handle = L4_nilthread;
    thread_list_delete(thread);
    return NULL;
}

void
pd_delete_thread(struct pd *pd, struct thread *thread)
{
    thread_delete(thread);
}

uintptr_t
pd_add_clist(struct pd *self, struct clist *clist)
{
    struct clist_info *clist_info;

    clist_info = clist_list_create_back(&self->clists);
    if (clist_info == NULL) {
        return 0;
    }
    clist_info->clist = clist;
    return (uintptr_t)clist_info;
}

void
pd_release_clist(const struct pd *self, const struct clist *clist)
{
    struct clist_node *clists;

    ASSERT_PTR(self);

    for (clists = self->clists.first;
         clists->next != self->clists.first; clists = clists->next) {
        if (clist == clists->data.clist) {
            clist_list_delete(&clists->data);
            break;
        }
    }
    return;
}

#if defined(IG_DEBUG_PRINT)
void
pd_print(struct pd *self)
{
    DEBUG_PRINT("PD: %p\n", self);
    DEBUG_PRINT(" Owner: %p\n", self->space.owner);
    /* Print out thread */
    /* Print out pds */
}
#endif

#if !defined(NDEBUG)
int
pd_check_state(const struct pd *base_pd)
{
    /* Check that the PD is valid -- this is a debugging feature */
    int r = 0;

    struct memsection_node *ms, *first_ms = base_pd->memsections.first;
    struct thread_node *td, *first_td = base_pd->threads.first;
#if defined(CONFIG_SESSION)
    struct session_p_node *sd, *first_sd = base_pd->sessions.first;
#endif
    struct pd_node *pd, *first_pd = base_pd->pds.first;

    if (!is_pd(base_pd)) {
        return 1;
    }

    if (!is_pd(base_pd->space.owner)) {
        return 1;
    }

    for (ms = first_ms; ms->next != first_ms; ms = ms->next) {
        ASSERT_PTR(&ms->data);
        r += memsection_check_state(&ms->data);
    }

    for (td = first_td; td->next != first_td; td = td->next) {
        ASSERT_PTR(&td->data);
        r += thread_check_state(&td->data);
    }
#if defined(CONFIG_SESSION)
    for (sd = first_sd; sd->next != first_sd; sd = sd->next) {
        ASSERT_PTR(&sd->data);
        r += session_check_state(sd->data);
    }
#endif
    for (pd = first_pd; pd->next != first_pd; pd = pd->next) {
        ASSERT_PTR(&pd->data);
        r += pd_check_state(&pd->data);
    }

    return r;
}
#endif

int
pd_insert_memsection(struct pd *pd, struct memsection *ms, int rwx)
{
    int r;

    /* If this a protected memsection, set up an extension space. */
#if defined(CONFIG_MEM_PROTECTED)
    if (ms->flags & MEM_PROTECTED) {
        r = pd_setup_ext(pd);
        if (r) {
            assert(0);
            goto error;
        }
    }
#endif
    /* Track the mapping in the PD's map. */
    r = pd_map_insert(pd, ms, rwx);
    if (r) {
        goto error;
    }

    /* Track the reverse mapping in the memsection. */
    r = memsection_rmap_insert(ms, pd, rwx);
    if (r) {
        goto error;
    }

    /* Update the kernel mappings. */
    r = pd_sync_range(pd, ms->base, ms->end, ms, rwx);
    if (r) {
        goto error;
    }

    return 0;
error:
    /* XXX: Maybe we should clean up if we fail part way through. */
    return -1;
}

int
pd_attach(struct pd *pd, struct memsection *ms, int rwx)
{
#if defined(CONFIG_ZONE)
    if (ms->flags & MEM_ZONE) {
        /* Memsections in zones can't be attached directly. */
        return -1;
    }
#endif
    return pd_insert_memsection(pd, ms, rwx);
}

void
pd_detach(struct pd *pd, struct memsection *ms)
{
    /* Remove any hardware mappings. */
    pd_flush_range(pd, ms->base, ms->end);

    /* Update the memsection. */
    memsection_rmap_remove(ms, pd);

    /* Update the PD. */
    pd_map_remove(pd, ms);
}

int
pd_sync_range(struct pd *pd, uintptr_t base, uintptr_t end,
              struct memsection *ms, int rwx)
{
    uintptr_t attr;
    uintptr_t phys;
    int r;
    size_t size;
    L4_SpaceId_t space;
#if defined(CONFIG_MEM_PROTECTED)
    L4_SpaceId_t ext_space;
#endif
    uintptr_t virt;

    /* If a valid memsection wasn't passed in, we look it up. */
    if (!ms) {
        ms = objtable_lookup((void *)base);
        if (!ms) {
            return -1;
        }
    }

    /* Ditto for the access rights. */
    if (!rwx) {
        rwx = pd_map_lookup(pd, ms);
        if (!rwx) {
            return -1;
        }
    }

    /* Walk the address range and update any mappings. */
    attr = ms->attributes;
    space = pd_l4_space(pd);
#if defined(CONFIG_MEM_PROTECTED)
    ext_space = pd_ext_l4_space(pd);
#endif
    for (virt = base; virt <= end; virt += size) {
        r = memsection_lookup_phys(ms, virt, &phys, &size, pd);
        if (!r) {
            /* XXX: We should coalesce adjacent gaps. */
            size = BASE_PAGESIZE;
            l4e_unmap(space, virt, virt + size - 1);
            continue;
        }

        /* XXX: We allow mappings that lie partially outside the range. */
        virt = round_down(virt, size);
#if defined(CONFIG_MEM_PROTECTED)
        if ((ms->flags & MEM_PROTECTED) == 0) {
#endif
            r = l4e_map(space, virt, virt + size - 1, phys, rwx, attr);
            if (r != 1) {
                return -1;
            }
#if defined(CONFIG_MEM_PROTECTED)
        }
#endif
#if defined(CONFIG_MEM_PROTECTED)
#if defined(ARM_SHARED_DOMAINS)
        /* FIXME: resync mappings in extension space? */
        if (!L4_IsNilSpace(ext_space)) {
            if (ms->flags & MEM_PROTECTED) {
                if (ms != protected_memsection) {
                    /* Only one protected memsection supported. */
                    return -1;
                }
                r = l4e_map(ext_space, virt, virt + size - 1, phys, rwx, attr);
                if (r != 1) {
                    assert(0);
                    return -1;
                }
            }
            else {
                uintptr_t addr, limit;

                addr = round_down(virt, MIN_WINDOW);
                limit = round_up(virt + size - 1, MIN_WINDOW) - 1;
                for (; addr <= limit; addr += MIN_WINDOW) {
                    r = !l4_map_window(ext_space, space,
                                       L4_Fpage(addr, MIN_WINDOW));
                    /* XXX: We don't trap which regions are already mapped so
                     * we can end up trying to map over the top of a perfectly
                     * useful window.
                     */
                    if (r != 0 && L4_ErrorCode() != L4_ErrDomainConflict) {
                        assert(0);
                        return -1;
                    }
                }
            }
        }
#else
        if (!L4_IsNilSpace(ext_space)) {
            r = l4e_map(ext_space, virt, virt + size - 1, phys, rwx, attr);
            if (r != 1) {
                return -1;
            }
        }
#endif
#endif
    }
    return 0;
}

void
pd_flush_range(struct pd *pd, uintptr_t base, uintptr_t end)
{
    l4e_unmap(pd_l4_space(pd), base, end);
#if defined(CONFIG_MEM_PROTECTED)
#if !defined(ARM_SHARED_DOMAINS)
    if (!L4_IsNilSpace(pd_ext_l4_space(pd))) {
        l4e_unmap(pd_ext_l4_space(pd), base, end);
    }
#endif
#endif
}

int
pd_map_insert(struct pd *pd, struct memsection *ms, int rwx)
{
    struct ms_entry *ent;

    TAILQ_FOREACH(ent, &pd->ms_list, ms_list) {
        if (ent->ms == ms) {
            ent->rwx = rwx;
            return 0;
        }
    }
    ent = slab_cache_alloc(&ms_ent_cache);
    if (ent == NULL) {
        return -1;
    }
    ent->ms = ms;
    ent->rwx = rwx;
    TAILQ_INSERT_TAIL(&pd->ms_list, ent, ms_list);
    return 0;
}

int
pd_map_lookup(struct pd *pd, struct memsection *ms)
{
    struct ms_entry *ent;

    TAILQ_FOREACH(ent, &pd->ms_list, ms_list) {
        if (ent->ms == ms) {
            return ent->rwx;
        }
    }
    return 0;
}

void
pd_map_remove(struct pd *pd, struct memsection *ms)
{
    struct ms_entry *ent;

    TAILQ_FOREACH(ent, &pd->ms_list, ms_list) {
        if (ent->ms == ms)
            break;
    }
    if (ent != NULL) {
        TAILQ_REMOVE(&pd->ms_list, ent, ms_list);
        slab_cache_free(&ms_ent_cache, ent);
    }
}

struct pd *
pd_lookup(L4_SpaceId_t space)
{
    return hash_lookup(l4spaceid_to_pd, L4_SpaceNo(space));
}

#if defined(CONFIG_ZONE)
struct zone *
pd_create_zone(struct pd *pd, struct virtpool *pool)
{
    return zone_create(pd, pool);
}
#endif

#if defined(CONFIG_ZONE)
#if defined(ARM_SHARED_DOMAINS)
static int
arch_pd_attach_zone(struct pd *pd, struct zone *zone, int rwx)
{
    uintptr_t addr;
    int r;
    struct virtmem *vm;

    if (rwx & L4_Writable) {
        r = !l4_manage_domain(pd_l4_space(pd), pd_l4_space(zone->space));
        if (r != 0) {
            return -1;
        }
    } else {
        r = !l4_share_domain(pd_l4_space(pd), pd_l4_space(zone->space));
        if (r != 0) {
            return -1;
        }
    }

    TAILQ_FOREACH(vm, &zone->window_list, vm_list) {
        for (addr = vm_base(vm); addr <= vm_end(vm); addr += MIN_WINDOW) {
            r = !l4_map_window(pd_l4_space(pd), pd_l4_space(zone->space),
                               L4_Fpage(addr, MIN_WINDOW));
            if (r != 0) {
                return -1;
            }
        }
    }
    return 0;
}
#else
static int
arch_pd_attach_zone(struct pd *pd, struct zone *zone, int rwx)
{
    struct memsection *ms;
    int r;

    TAILQ_FOREACH(ms, &zone->ms_list, zone_list) {
        r = pd_insert_memsection(pd, ms, rwx);
        if (r != 0) {
            return r;
        }
    }
    return 0;
}
#endif
#endif

#if defined(CONFIG_ZONE)
int
pd_attach_zone(struct pd *pd, struct zone *zone, int rwx)
{
    int r;

    /* FIXME: Track it in the PD */
    r = pd_zonemap_insert(pd, zone, rwx);
    if (r) {
        DEBUG_PRINT("fail 0\n");
        goto error;
    }

    r = zone_rmap_insert(zone, pd, rwx);
    if (r) {
        DEBUG_PRINT("fail 1\n");
        goto error;
    }

    return arch_pd_attach_zone(pd, zone, rwx);
error:
    return -1;
}
#endif

#if defined(CONFIG_ZONE)
#if defined(ARM_SHARED_DOMAINS)
static void
arch_pd_detach_zone(struct pd *pd, struct zone *zone)
{
    l4_unshare_domain(pd_l4_space(pd), pd_l4_space(zone->space));
}
#else
static void
arch_pd_detach_zone(struct pd *pd, struct zone *zone)
{
    struct memsection *ms;

    TAILQ_FOREACH(ms, &zone->ms_list, zone_list) {
        pd_detach(pd, ms);
    }
}
#endif
#endif

#if defined(CONFIG_ZONE)
void
pd_detach_zone(struct pd *pd, struct zone *zone)
{
    arch_pd_detach_zone(pd, zone);
    zone_rmap_remove(zone, pd);
    pd_zonemap_remove(pd, zone);
}
#endif

#if defined(CONFIG_ZONE)
int
pd_zonemap_insert(struct pd *pd, struct zone *zone, int rwx)
{
    struct zone_entry *ent;

    TAILQ_FOREACH(ent, &pd->attached_zones, zone_list) {
        if (ent->zone == zone) {
            ent->rwx = rwx;
            return 0;
        }
    }
    ent = slab_cache_alloc(&zone_ent_cache);
    if (ent == NULL) {
        return -1;
    }
    ent->zone = zone;
    ent->rwx = rwx;
    TAILQ_INSERT_TAIL(&pd->attached_zones, ent, zone_list);
    return 0;
}
#endif

#if defined(CONFIG_ZONE)
void
pd_zonemap_remove(struct pd *pd, struct zone *zone)
{
    struct zone_entry *ent;

    TAILQ_FOREACH(ent, &pd->attached_zones, zone_list) {
        if (ent->zone == zone)
            break;
    }
    if (ent != NULL) {
        TAILQ_REMOVE(&pd->attached_zones, ent, zone_list);
        slab_cache_free(&zone_ent_cache, ent);
    }
}
#endif
