/*
 * Copyright (c) 2006, National ICT Australia
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
 * Author: Malcolm Purvis
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iguana/cap.h>
#include <iguana/object.h>
#include <l4e/map.h>
#include <l4/kdebug.h>
#include <l4/types.h>
#include <l4/security.h>
#include <l4/interrupt.h>
#include <util/bits.h>
#include <bootinfo/bootinfo.h>
#include "bootinfo.h"
#include "util.h"
#include "clist.h"
#include "objtable.h"
#include "pd.h"
#include "pgtable.h"
#include "security.h"
#include "vm.h"
#include "env.h"
#include "tools.h"

/* Simple boolean type. */
typedef int bool;
#define true 1
#define false 0

#define CLIST_MEMORY_SIZE (2 * sizeof(cap_t) * MAX_ELF_SEGMENTS)
#define ARGS_MAX 32

#define NO_OBJ_ENV_BASE NULL

typedef enum {
    TYPE_PD, TYPE_MS, TYPE_THREAD, TYPE_CAP, TYPE_POOL, TYPE_ZONE
} object_type_t;

typedef struct 
{
    object_type_t type; /* Thread? Pd? */
} object_hdr_t;

typedef struct {
    object_hdr_t hdr;

    struct pd *pd;
    struct clist *clist;
    struct environment *env_base;
    envitem_t* free_env;
    char* last_string;
    int env_closed;
} pd_data_t;

typedef struct 
{
    object_hdr_t hdr;

    struct memsection *ms;
} memsection_data_t;

typedef struct {
    object_hdr_t hdr;

    struct thread *thrd;
    pd_data_t *owner;
    uintptr_t ip;
    uintptr_t user_main;
    uintptr_t *stack_base; /* Bottom of the stack memsection. */
    uintptr_t *sp;
    int thread_started;
    uintptr_t argc;
    uintptr_t argv[ARGS_MAX];
} thread_data_t;

/* Structure to map object env names to physical memory pools. */
typedef struct 
{
    object_hdr_t hdr;

    bool is_virtual;
    union 
    {
        struct virtpool *vref;
        struct physpool *pref;
    } u;
} pool_data_t;

typedef struct 
{
    object_hdr_t hdr;

    cap_t cap;
} cap_data_t;

typedef struct 
{
    object_hdr_t hdr;

    struct zone* zone;
} zone_data_t;

typedef struct 
{
    int total_objects;
/* Object array.  Indexes into this array serve as names. */
    void **obj_array;
    int obj_array_size;

    int free_pd;
    pd_data_t* pd_array;
    int free_ms;
    memsection_data_t* ms_array;
    int free_thread;
    thread_data_t* thread_array;
    int free_cap;
    cap_data_t* cap_array;
    int free_pool;
    pool_data_t* pool_array;
    int free_zone;
    zone_data_t* zone_array;
} bi_tracker_t;

static pd_data_t iguana_pd_data;

static void *
get_name(const bi_tracker_t* tr, L4_Word_t name)
{
    if (name > tr->obj_array_size) {
        return NULL;
    } else {
        return tr->obj_array[name];
    }
}

static pd_data_t *
get_pd(const bi_tracker_t* tr, L4_Word_t name)
{
    pd_data_t *pd = get_name(tr, name);

    if (pd == NULL || pd->hdr.type != TYPE_PD ||
        !is_pd(pd->pd)) {
        return NULL;
    } else {
        return pd;
    }
}

static thread_data_t *
get_thread(const bi_tracker_t* tr, L4_Word_t name)
{
    thread_data_t *thread = get_name(tr, name);

    if (thread == NULL || thread->hdr.type != TYPE_THREAD ||
        !is_thread(thread->thrd)) {
        return NULL;
    } else {
        return thread;
    }
}

static L4_ThreadId_t
get_thread_id(const bi_tracker_t* tr, L4_Word_t name)
{
    thread_data_t *thread = get_name(tr, name);

    if (thread == NULL || thread->hdr.type != TYPE_THREAD ||
        !is_thread(thread->thrd)) {
        return L4_nilthread;
    } else {
        return thread->thrd->id;
    }
}

static L4_SpaceId_t
get_thread_space_id(const bi_tracker_t* tr, L4_Word_t name)
{
    thread_data_t *thread = get_name(tr, name);

    if (thread == NULL || thread->hdr.type != TYPE_THREAD) {
        return L4_nilspace;
    } else  {
        return pd_l4_space(thread->owner->pd);
    }
}

static cap_data_t *
get_cap(const bi_tracker_t* tr, L4_Word_t name)
{
    cap_data_t *cap = get_name(tr, name);

    if (cap == NULL || cap->hdr.type != TYPE_CAP) {
        return NULL;
    } else {
        return cap;
    }
}

static pool_data_t *
get_pool(const bi_tracker_t* tr, L4_Word_t name, bool virtual)
{
    pool_data_t *pool = get_name(tr, name);

    if (pool == NULL || pool->hdr.type != TYPE_POOL ||
        pool->is_virtual != virtual) {
        return NULL;
    } else {
        return pool;
    }
}

static memsection_data_t *
get_ms(const bi_tracker_t* tr, L4_Word_t name)
{
    memsection_data_t *ms = get_name(tr, name);

    if (ms == NULL || ms->hdr.type != TYPE_MS ||
        !is_memsection(ms->ms)) {
        return NULL;
    } else {
        return ms;
    }
}

static zone_data_t *
get_zone(const bi_tracker_t* tr, L4_Word_t name)
{
    zone_data_t *zone = get_name(tr, name);

    if (zone == NULL || zone->hdr.type != TYPE_ZONE) {
        return NULL;
    } else {
        return zone;
    }
}

/* Is there room on the stack to push <words> words? */
static inline int
can_push(thread_data_t* td, int words)
{
    return td->sp > td->stack_base + words;
}

static inline void
push_word(thread_data_t* td, uintptr_t word)
{
    /* Stack point to the address the word goes on. */
    td->sp--;
    *td->sp = word;
}

static uintptr_t
push_words(thread_data_t * td, uintptr_t *words, size_t number)
{
    int i;

    if (!can_push(td, number))
        return 0;

    for (i = number - 1; i >= 0; i--)
        push_word(td, words[i]);

    return (uintptr_t)td->sp;
}

static int
close_obj_env(pd_data_t * pdd)
{
    static const char CLIST_KEY[] = "OKL4_CLIST";

    if (!pdd->env_closed) {
        if (pdd->clist != NULL) {
            envitem_t* new_entry;

            /*
             * Firstly make sure that there is enough space to include
             * the new entry and its string.
             */
            if (pdd->last_string - sizeof(CLIST_KEY) <
                (char*) pdd->free_env + sizeof(struct envitem)) {
                return 1;
            }
                    
            pdd->last_string -= sizeof(CLIST_KEY);
            memcpy(pdd->last_string, CLIST_KEY, sizeof(CLIST_KEY));

            new_entry = pdd->free_env;
            new_entry->name = pdd->last_string;
            new_entry->type = ENV_CLIST;
            new_entry->u.clist = security_create_capability((clist_ref_t) pdd->clist);
            pdd->free_env++;
        }

        
        pdd->env_base->version = ENV_VERSION;
        pdd->env_base->len = pdd->free_env - pdd->env_base->item;
        pdd->env_closed = true;
    }

    return 0;
}



int pool_init(uintptr_t vbase, uintptr_t vend, uintptr_t pbase, uintptr_t pend,
              struct physpool **physpool, struct virtpool **virtpool,
              struct virtpool **directpool);


/* Pool structures created by pool_init(). */
struct physpool *physpool   = NULL;
struct virtpool *virtpool   = NULL;
struct virtpool *directpool = NULL;

static int init(const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;

    tr->total_objects = data->total_pds + data->total_mss +
        data->total_threads + data->total_caps + data->total_pools +
        data->total_zones + 1;
    tr->pd_array      = malloc(data->total_pds * sizeof(pd_data_t));
    tr->ms_array      = malloc(data->total_mss * sizeof(memsection_data_t));
    tr->thread_array  = malloc(data->total_threads * sizeof(thread_data_t));
    tr->cap_array     = malloc(data->total_caps * sizeof(cap_data_t));
    tr->pool_array    = malloc(data->total_pools * sizeof(pool_data_t));
    tr->zone_array    = malloc(data->total_zones * sizeof(zone_data_t));
    tr->obj_array     = malloc(tr->total_objects * sizeof(void*));

    if (tr->pd_array  == NULL || tr->thread_array  == NULL ||
        tr->obj_array == NULL || tr->pool_array == NULL ||
        tr->cap_array == NULL || tr->ms_array == NULL ||
        (data->total_zones != 0 && tr->zone_array == NULL)) {
        ERROR_PRINT("Could not allocate memory for bootinfo buffers.\n");

        return 1;
    }

    /* Prime the object array. */
    tr->obj_array[0] = (void *)&iguana_pd_data;
    tr->obj_array_size = 1;

    return 0;
}


static int init_mem(uintptr_t virt_base, uintptr_t virt_end,
                    uintptr_t phys_base, uintptr_t phys_end,
                    const bi_user_data_t* data)
{
    pool_init(virt_base, virt_end, phys_base, phys_end,
              &physpool, &virtpool, &directpool);

    return 0;
}


static int init_mem2(uintptr_t virt_base, uintptr_t virt_end,
                     uintptr_t phys_base, uintptr_t phys_end,
                     const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pool_data_t *poold;

    /* Prime the pool arrays now that they're allocated. */
    poold = &tr->pool_array[tr->free_pool++];
    poold->hdr.type   = TYPE_POOL;
    poold->is_virtual = true;
    poold->u.vref     = virtpool;

    BOOTINFO_PRINT("<--- Created Virtual Pool %d (%p)\n", tr->obj_array_size, virtpool);
    tr->obj_array[tr->obj_array_size++] = poold;

    poold = &tr->pool_array[tr->free_pool++];
    poold->hdr.type   = TYPE_POOL;
    poold->is_virtual = false;
    poold->u.pref     = physpool;

    BOOTINFO_PRINT("<--- Created Physical Pool %d\n", tr->obj_array_size);
    tr->obj_array[tr->obj_array_size++] = poold;

    poold = &tr->pool_array[tr->free_pool++];
    poold->hdr.type   = TYPE_POOL;
    poold->is_virtual = true;
    poold->u.vref     = directpool;

    BOOTINFO_PRINT("<--- Created Direct Pool %d\n", tr->obj_array_size);
    tr->obj_array[tr->obj_array_size++] = poold;

    return 0;
}


static bi_name_t new_pd(bi_name_t bi_owner, const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *owner = get_pd(tr, bi_owner);
    struct pd *pd;

    if (owner == NULL) {
        ERROR_PRINT
            ("Record %d: Failed to create a PD. Invalid owner %d\n",
             data->rec_num, bi_owner);

        return BI_NAME_INVALID;
    }

    pd = pd_create(owner->pd, 0 /* Max threads */ );

    if (pd != NULL) {
        pd_data_t *pdt = &tr->pd_array[tr->free_pd++];

        assert(tr->free_pd <= data->total_pds);

        /* Clear the new structure. */
        pdt->hdr.type    = TYPE_PD;
        pdt->pd          = pd;
        pdt->clist       = NULL;
        pdt->env_closed  = false;
        pdt->env_base    = NULL;
        pdt->free_env    = NULL;
        pdt->last_string = NULL;

        tr->obj_array[tr->obj_array_size] = pdt;
        
        assert(tr->obj_array_size < tr->total_objects);
    } else {
        ERROR_PRINT("Record %d: Failed to create a PD.\n", data->rec_num);

        return BI_NAME_INVALID;
    }

    return tr->obj_array_size++;
}


static bi_name_t new_ms(bi_name_t bi_owner, uintptr_t base, uintptr_t size,
                        uintptr_t flags, uintptr_t attr,
                        bi_name_t bi_physpool, bi_name_t bi_virtpool,
                        bi_name_t bi_zone,
                        const bi_user_data_t* data)
{
    bi_tracker_t* tr       = (bi_tracker_t*) data->user_data;
    pd_data_t *owner       = get_pd(tr, bi_owner);
    pool_data_t* virtpoold = get_pool(tr, bi_virtpool, true);
    pool_data_t* physpoold = get_pool(tr, bi_physpool, false);
    zone_data_t* zd        = get_zone(tr, bi_zone);
    struct zone* zone      = NULL;
    struct virtpool* virtpool = NULL;
    struct memsection *ms;

    flags |= MEM_INTERNAL;

    if (zd != NULL) {
        flags |= MEM_ZONE;
        zone = zd->zone;
    } else {
        virtpool = virtpoold->u.vref;
    }

    ms = pd_create_memsection(owner->pd, size, base,
                              physpoold->u.pref, virtpool,
                              zone, flags);

    if (ms != NULL) {
        memsection_data_t *msd = &tr->ms_array[tr->free_ms++];

        assert(tr->free_ms <= data->total_mss);

        if (attr != L4_DefaultMemory) {
            memsection_set_attributes(ms, attr);
        }

        msd->hdr.type = TYPE_MS;
        msd->ms = ms;

        tr->obj_array[tr->obj_array_size] = msd;

        assert(tr->obj_array_size < tr->total_objects);
    } else {
        ERROR_PRINT("Record %d: Failed to create a memsection\n",
                    data->rec_num);

        return BI_NAME_INVALID;
    }

    return tr->obj_array_size++;
}


static int add_virt_mem(bi_name_t pool, uintptr_t base, uintptr_t end,
                        const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
#if defined(CONFIG_ZONE)
    zone_data_t* zoned = get_zone(tr, pool);
#endif
    int ret;

#if defined(CONFIG_ZONE)
    if (zoned != NULL) {
        ret = zone_add_memory(zoned->zone, base, end);
    } else
#endif
    {
        pool_data_t* poold = get_pool(tr, pool, true);
        ret = virtpool_add_memory(poold->u.vref, base, end);
    }

    if (ret != 0) {
        ERROR_PRINT("Record %d: Failed to add free virtual memory.\n",
                    data->rec_num);
    }
    
    return ret;
}

static int add_phys_mem(bi_name_t pool, uintptr_t base, uintptr_t end,
                        const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pool_data_t* poold = get_pool(tr, pool, false);

    physpool_add_memory(poold->u.pref, base, end);

    return 0;
}

static bi_name_t new_thread(bi_name_t bi_owner, uintptr_t ip,
                            uintptr_t user_main, int priority,
                            char* name, size_t name_len,
                            const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;

    /* Enough space for the arg plus a NUL and word alignment */
    char *value = malloc(name_len + 1);
    pd_data_t *owner;
    struct thread *thread;

    if (value == NULL) {
        ERROR_PRINT("Failed to malloc tmp buffer.\n");

        return BI_NAME_INVALID;
    }

    memcpy(value, name, name_len);
    /* Make sure it's nul terminated.  */
    value[name_len] = '\0';

    owner = get_pd(tr, bi_owner);
    thread = pd_create_thread(owner->pd, priority);

    if (thread != NULL) {
        thread_data_t *td = &tr->thread_array[tr->free_thread++];

        assert(tr->free_thread <= data->total_threads);

        td->hdr.type       = TYPE_THREAD;
        td->thrd           = thread;
        td->owner          = owner;
        td->ip             = ip;
        td->user_main      = user_main;
        td->thread_started = 0;
        td->argc           = 0;
        memzero(td->argv, sizeof(td->argv));

        /* Set the name. */
        L4_KDB_SetThreadName(thread->id, value);

        tr->obj_array[tr->obj_array_size] = td;

        assert(tr->obj_array_size < tr->total_objects);

        free(value);
    } else {
        ERROR_PRINT("Failed to create a thread\n");

        free(value);

        return BI_NAME_INVALID;
    }

    return tr->obj_array_size++;
}


static int run_thread(bi_name_t thread, const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    thread_data_t *td    = get_thread(tr, thread);
    pd_data_t* owner_pdd = td->owner;
    uintptr_t argv;

    if (td->thread_started) {
        ERROR_PRINT("Record %d: Thread is already running.\n",
                    data->rec_num);

        return 1;
    }

    if (close_obj_env(owner_pdd)) {
        ERROR_PRINT("Record %d: Not enough memory to close off the object environment.",
                    data->rec_num);

        return 1;
    }
                
    /*
     * Check that there is room for the final items on the
     * stack.
     */
    if (!can_push(td, ARGS_MAX + 5)) {
        ERROR_PRINT("Record %d: Out of stack space.\n", data->rec_num);

        return 1;
    }

    /* Push the argv array onto the stack. */
    push_words(td, td->argv, ARGS_MAX);

    /* Save the address for later. */
    argv = (uintptr_t) td->sp;

    /* Some architectures require an 8-byte aligned stack. */
    if ((uintptr_t)(td->sp) % 8) {
        push_word(td, 0);
    }

    /* Push the user thread entry point. */
    push_word(td, td->user_main);

    /* Push initial stack frame. */
    push_word(td, argv);
    push_word(td, td->argc);
    push_word(td, (uintptr_t)owner_pdd->env_base);
    /*
     * At this point the stack looks like:
     *
     * |-----------------------|
     * | strings               |<-+
     * |                       |  |
     * |-----------------------|  |
     * | NULL                  |  |
     * | argv[i]               |--+
     * | argv[1]               |
     * | argv[0]               |<-+
     * |-----------------------|  |
     * | user main             |  |
     * |-----------------------|  |
     * | argv                  |--+
     * |-----------------------|
     * | argc                  |
     * |-----------------------|
     * | obj_env_ptr           |<-- stack
     * |-----------------------|
     * |                       |
     * +-----------------------+
     */

    BOOTINFO_PRINT("thread started (ip: 0x%lx, sp: 0x%lx)\n", (long)td->ip,
                   (long)td->sp);
    thread_start(td->thrd, td->ip, (uintptr_t)td->sp);
    td->thread_started = 1;

    return 0;
}


static int map(uintptr_t vaddr, uintptr_t size, uintptr_t paddr,
               int scrub, unsigned mode, const bi_user_data_t* data)
{
    uintptr_t vend        = vaddr + size - 1;
    struct memsection *ms = objtable_lookup((void *)vaddr);
    struct physmem *pm;

    if (ms == NULL) {
        ERROR_PRINT("Record %d: Invalid VADDR\n", data->rec_num);

        return 1;
    }

    pm = physpool_add_alloced_memory(default_physpool, paddr, paddr + size - 1);

    if (pm == NULL) {
        ERROR_PRINT("Record %d: Invalid PADDR\n", data->rec_num);
        return 1;
    }

    assert(pm->mem->addr == paddr);
    pm->need_scrub = scrub;

    if (memsection_map(ms, vaddr - ms->base, pm)) {
        ERROR_PRINT("Record %d: Mapping failed\n", data->rec_num);

        return 1;
    }

    if (!l4e_map(L4_rootspace, vaddr, vend, paddr, 
                 L4_ReadWriteOnly, mode)) {
        ERROR_PRINT("Record %d: Mapping into Iguana\n", data->rec_num);

        return 1;
    }

    return 0;
}


static int attach(bi_name_t pd, bi_name_t ms, int rights,
                  const bi_user_data_t* data)
{
    bi_tracker_t* tr       = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd         = get_pd(tr, pd);
    memsection_data_t* msd = get_ms(tr, ms);

    if (rights != 0) {
        if (msd != NULL) {
            if (pd_attach(pdd->pd, msd->ms, rights) != 0) {
                ERROR_PRINT("Record %d: Attach failed.\n", data->rec_num);

                return 1;
            }
        }
#if defined(CONFIG_ZONE)
        else {
            zone_data_t* zd = get_zone(tr, ms);

            assert(zd != NULL);

            if (pd_attach_zone(pdd->pd, zd->zone, rights) != 0) {
                ERROR_PRINT("Record %d: Attach failed.\n", data->rec_num);

                return 1;
            }
            else
                return 0;
        }
#endif
    }

    /*
     * Give iguana server read/write access to the memsection.
     *
     * Magpie currently passes strings as a pointer to
     * the senders address space.  Sessions grant the
     * necessary permissions between clients, but
     * sessions aren't established between clients and
     * iguana_server.  Therefore give iguana server
     * the necessary rights to every memsection in
     * case one of them contains a string that will be
     * sent.
     */

    if (msd != NULL && /* Not a zone.. */
        pd_attach(&iguana_pd, msd->ms,
                  L4_ReadWriteOnly) != 0) {
        ERROR_PRINT("Record %d: Attach to iguana server failed.\n", data->rec_num);

        return 1;
    }

    return 0;
}


static int grant(bi_name_t pd, bi_name_t obj, int rights,
                 const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd = get_pd(tr, pd);
    cap_t cap = INVALID_CAP;
    int r;

    if (pdd->clist == NULL) {
        pdd->clist = server_clist_create(pdd->pd);

        if (pdd->clist == NULL) {
            ERROR_PRINT("Record %d: Failed to create clist.\n", data->rec_num);

            return 1;
        }

        pd_add_clist(pdd->pd, pdd->clist);
    }

    cap.ref.obj = (objref_t)tr->obj_array[obj];
    r = server_clist_insert(pdd->clist, cap);

    if (r != 0) {
        ERROR_PRINT("Record %d: Failed to insert cap.\n", data->rec_num);

        return 1;
    }

    return 0;
}


static int argv(bi_name_t thread, char* arg, size_t arg_len,
                const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    thread_data_t *td = get_thread(tr, thread);

    /* Enough space for the arg plus a NUL and word alignment */
    char *value = malloc(arg_len + 1);

    if (value == NULL) {
        ERROR_PRINT("Failed to malloc tmp buffer.\n");

        return 1;
    }

    memcpy(value, arg, arg_len);
    /* Make sure it's nul terminated.  */
    value[arg_len] = '\0';

    if (td->argc >= ARGS_MAX) {
        ERROR_PRINT("Record %d: Too many command line arguments supplied.\n",
                    data->rec_num);

        free(value);

        return 1;
    }

    if (can_push(td, (arg_len / sizeof(uintptr_t)) + 1))
        td->argv[td->argc++] =
            push_words(td, (uintptr_t*) value, (arg_len / sizeof(uintptr_t)) + 1);
    else {
        ERROR_PRINT("Record %d: Out of stack space.\n", data->rec_num);

        free(value);

        return 1;
    }

    return 0;
}


static int register_server(bi_name_t thread, bi_name_t ms,
                           const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    thread_data_t *td = get_thread(tr, thread);

    memsection_register_server(get_ms(tr, ms)->ms, td->thrd);

    return 0;
}

static int register_callback(bi_name_t pd, bi_name_t ms,
                             const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd = get_pd(tr, pd);

    pd_setup_callback(pdd->pd, get_ms(tr, ms)->ms);

    return 0;
}

static int register_stack(bi_name_t thread, bi_name_t ms,
                          const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    thread_data_t *td      = get_thread(tr, thread);
    memsection_data_t *msd = get_ms(tr, ms);

    td->stack_base = (uintptr_t *) msd->ms->base;
    td->sp         = (uintptr_t *)(msd->ms->end + 1);

    return 0;
}

static bi_name_t new_cap(bi_name_t bi_obj, bi_cap_rights_t bi_rights,
                         const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    object_hdr_t* obj = get_name(tr, bi_obj);
    objref_t ref;
    cap_data_t *capd;
    uintptr_t rights = -1; /* Initialise to invalid. */

    /*
     * Translate bootinfo bitmask to cap values.
     *
     * The cap values are *not* a bitmask to some guesswork
     * is required.
     */
    if (bi_rights == BI_CAP_MASTER)
        rights = MASTER_IID;
    else if ((bi_rights & BI_CAP_EXECUTE) != 0)
        rights = EXECUTE_IID;
    else if ((bi_rights & BI_CAP_WRITE) != 0)
        rights = WRITE_IID;
    else if ((bi_rights & BI_CAP_READ) != 0)
        rights = READ_IID;

    switch (obj->type) {
    case TYPE_PD:
    {
        pd_data_t* pdd = (pd_data_t*) obj;
        ref = (pd_ref_t) pdd->pd;

        break;
    }

    case TYPE_MS:
    {
        memsection_data_t* msd = (memsection_data_t*) obj;
        ref = (memsection_ref_t) msd->ms;

        break;
    }

    case TYPE_THREAD:
    {
        thread_data_t* td = (thread_data_t*) obj;
        ref = (thread_ref_t) td->thrd;

        break;
    }

    case TYPE_POOL:
    {
        pool_data_t* poold = (pool_data_t*) obj;

        if (poold->is_virtual) {
            ref = (virtpool_ref_t) poold->u.vref;
        } else {
            ref = (physpool_ref_t) poold->u.pref;
        }

        break;
    }

#if defined(CONFIG_ZONE)
    case TYPE_ZONE:
    {
        zone_data_t* zd = (zone_data_t*) obj;
        ref = (zone_ref_t) zd->zone;

        break;
    }
#endif
    case TYPE_CAP:
        ERROR_PRINT("Record %d: Cannot make a cap of a cap\n",
                    data->rec_num);

        return BI_NAME_INVALID;

    default:
        ERROR_PRINT("Record %d: Unknown object type %d\n",
                    data->rec_num, obj->type);

        return BI_NAME_INVALID;
    }

    capd = &tr->cap_array[tr->free_cap++];
    capd->hdr.type = TYPE_CAP;
    capd->cap = security_create_capability(ref | rights);

    tr->obj_array[tr->obj_array_size] = capd;

    return tr->obj_array_size++;
}

static int grant_cap(bi_name_t pd, bi_name_t cap, const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd  = get_pd(tr, pd);
    cap_data_t *capd = get_cap(tr, cap);
    int r;

    if (pdd->clist == NULL) {
        pdd->clist = server_clist_create(pdd->pd);

        if (pdd->clist == NULL) {
            ERROR_PRINT("Record %d: Failed to create clist.\n", data->rec_num);

            return 1;
        }

        pd_add_clist(pdd->pd, pdd->clist);
    }
    
    r = server_clist_insert(pdd->clist, capd->cap);

    if (r != 0) {
        ERROR_PRINT("Record %d: Failed to insert cap.\n", data->rec_num);

        return 1;
    }

    return 0;
}


static int export_object(bi_name_t pd, bi_name_t obj,
                         bi_export_type_t export_type,
                         char* key, size_t key_len,
                         const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd = get_pd(tr, pd);
    envitem_t* new_entry;

    if (pdd->env_base == NULL) {
        ERROR_PRINT("No environment registered with the PD.\n");

        return 1;
    }

    if (pdd->env_closed) {
        ERROR_PRINT("Record %d: Cannot export objects once the "
                    "environment has been closed.\n", data->rec_num);

        return 1;
    }

    /* Make sure that there is enough space to include the
     * new entry and its string. */
    if (pdd->last_string - key_len - 1 <
        (char*) pdd->free_env + sizeof(struct envitem)) {
        ERROR_PRINT("Record %d: Not enough memory for entry.\n",
                    data->rec_num);

        return 1;
    }
                    
    pdd->last_string -= key_len + 1;
    memcpy(pdd->last_string, key, key_len);
    pdd->last_string[key_len] = '\0';

    new_entry = pdd->free_env;
    new_entry->name = pdd->last_string;

    switch (export_type)
    {
    case BI_EXPORT_CONST:   
        new_entry->type = ENV_CONSTANT;
        new_entry->u.consts.a = obj;
        break;

    case BI_EXPORT_BASE:
        new_entry->type = ENV_CONSTANT;
        new_entry->u.consts.a = get_ms(tr, obj)->ms->base;
        break;

    case BI_EXPORT_THREAD_ID:
        new_entry->type = ENV_THREAD_ID;
        new_entry->u.l4_tid = get_thread_id(tr, obj);
        break;

    case BI_EXPORT_MEMSECTION_CAP:
        new_entry->type = ENV_MEMSECTION;
        new_entry->u.memsection = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_THREAD_CAP:
        new_entry->type = ENV_THREAD;
        new_entry->u.thread = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_VIRTPOOL_CAP:
        new_entry->type = ENV_VIRTPOOL;
        new_entry->u.virtpool = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_PHYSPOOL_CAP:
        new_entry->type = ENV_PHYSPOOL;
        new_entry->u.physpool = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_CLIST_CAP:
        new_entry->type = ENV_CLIST;
        new_entry->u.clist = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_PD_CAP:
        new_entry->type = ENV_PD;
        new_entry->u.pd = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_OBJECT:
        new_entry->type = ENV_OBJECT;
        new_entry->u.object = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_ZONE_CAP:
        new_entry->type = ENV_ZONE;
        new_entry->u.zone = get_cap(tr, obj)->cap;
        break;

    case BI_EXPORT_ELF_SEGMENT:
    case BI_EXPORT_ELF_FILE:
        ERROR_PRINT("Record %d: OBJECT_EXPORT does not support structure export type %d.\n",
                    data->rec_num, export_type);

        return 1;

    case BI_EXPORT_PHYSDEV_CAP:
    case BI_EXPORT_VIRTDEV_CAP:
        ERROR_PRINT("Record %d: OBJECT_EXPORT device cap (%d) no yet supported.\n",
                    data->rec_num, export_type);

        return 1;

    }

    pdd->free_env++;

    return 0;
}


static int export_struct(bi_name_t pd, bi_export_type_t export_type,
                         char* key, size_t key_len,
                         uintptr_t first, intptr_t second, uintptr_t third,
                         uintptr_t fourth, uintptr_t fifth, uintptr_t sixth,
                         const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd = get_pd(tr, pd);
    envitem_t* new_entry;

    if (pdd->env_base == NULL) {
        ERROR_PRINT("No environment registered with the PD.\n");

        return 1;
    }

    if (pdd->env_closed) {
        ERROR_PRINT("Record %d: Cannot export objects once the "
                    "environment has been closed.\n", data->rec_num);

        return 1;
    }

    /* Make sure that there is enough space to include the
     * new entry and its string. */
    if (pdd->last_string - key_len - 1 <
        (char*) pdd->free_env + sizeof(struct envitem)) {
        ERROR_PRINT("Record %d: Not enough memory for entry.\n",
                    data->rec_num);

        return 1;
    }
                    
    pdd->last_string -= key_len + 1;
    memcpy(pdd->last_string, key, key_len);
    pdd->last_string[key_len] = '\0';

    new_entry = pdd->free_env;
    new_entry->name = pdd->last_string;

    switch (export_type)
    {
    case BI_EXPORT_ELF_SEGMENT:
        new_entry->type = ENV_ELF_SEGMENT;
        new_entry->u.elf_segment.vaddr  = first;
        new_entry->u.elf_segment.flags  = second;
        new_entry->u.elf_segment.paddr  = third;
        new_entry->u.elf_segment.offset = fourth;
        new_entry->u.elf_segment.filesz = fifth;
        new_entry->u.elf_segment.memsz  = sixth;

        break;

    case BI_EXPORT_ELF_FILE:
        new_entry->type = ENV_ELF_FILE;
        new_entry->u.elf_file.type  = first;
        new_entry->u.elf_file.entry = second;

        break;

    case BI_EXPORT_CONST:   
    case BI_EXPORT_BASE:
    case BI_EXPORT_THREAD_ID:
    case BI_EXPORT_MEMSECTION_CAP:
    case BI_EXPORT_THREAD_CAP:
    case BI_EXPORT_VIRTPOOL_CAP:
    case BI_EXPORT_PHYSPOOL_CAP:
    case BI_EXPORT_CLIST_CAP:
    case BI_EXPORT_PD_CAP:
    case BI_EXPORT_OBJECT:
    case BI_EXPORT_PHYSDEV_CAP:
    case BI_EXPORT_VIRTDEV_CAP:
    case BI_EXPORT_ZONE_CAP:
        ERROR_PRINT("Record %d: STRUCT_EXPORT does not support object export type %d.\n",
                    data->rec_num, export_type);

        return 1;
    }
                
    pdd->free_env++;

    return 0;
}


static int register_env(bi_name_t pd, bi_name_t ms,
                        const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pd_data_t *pdd         = get_pd(tr, pd);
    memsection_data_t *msd = get_ms(tr, ms);

    pdd->env_base    = (struct environment *) msd->ms->base;
    pdd->free_env    = &pdd->env_base->item[0];
    pdd->last_string = (char*) msd->ms->end + 1;

    /* Hook in the root server environment. */
    if (pd == 0)
        __okl4_environ = pdd->env_base;

    return 0;
}


static bi_name_t new_pool(int is_virtual, const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    pool_data_t* poold = &tr->pool_array[tr->free_pool++];

    poold->hdr.type   = TYPE_POOL;
    poold->is_virtual = is_virtual;

    if (is_virtual)
        poold->u.vref = virtpool_create();
    else
        poold->u.pref = physpool_create();

    tr->obj_array[tr->obj_array_size] = poold;

    return tr->obj_array_size++;
}


static int grant_interrupt(bi_name_t thread, L4_Word_t interrupt,
                           const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
    L4_ThreadId_t tid = get_thread_id(tr, thread);
    long          r;
                
    if (L4_IsNilThread(tid)) {
        ERROR_PRINT("Record %d: Failed to look up thread (thread: %d)\n",
                    data->rec_num, (int)thread);
        return 1;
    }

    L4_LoadMR(0, interrupt);
    r = L4_SecurityControl(get_thread_space_id(tr, thread),
                           L4_SecurityCtrl_domain(L4_SecurityControl_IrqAccessDomain) |
                           L4_SecurityCtrl_op(L4_SecurityControl_OpGrant) | 0);
    if (r == 0) {
        ERROR_PRINT("Record %d: Cannot grant interrupt\n",
                    data->rec_num);
        return 1;
    }

    L4_LoadMR(0, interrupt);

    /* Use bit 31 for device server hardware interrupts */
    r = L4_RegisterInterrupt(tid, 31, 0, 0);

    if (r == 0) {
        ERROR_PRINT("Record %d: Cannot register interrupt.\n",
                    data->rec_num);
        return 1;
    }

    return 0;
}

static int security_control(bi_name_t pd, bi_name_t obj,
                            bi_security_control_type_t security_type,
                            const bi_user_data_t* data)
{
   bi_tracker_t* tr = (bi_tracker_t*) data->user_data;
   pd_data_t *pdd   = get_pd(tr, pd);

    switch (security_type) {
    case BI_SECURITY_PLATFORM:
        if (pd_setup_platform_control(pdd->pd) == 0) {
            ERROR_PRINT("Record %d: Unable to grant platform control access.\n",
                        data->rec_num);
            return 1;
        }

        break;

    case BI_SECURITY_SPACE:
        break;

    case BI_SECURITY_INTERRUPT:
        break;

    case BI_SECURITY_MAPPING:
        if (pd_setup_user_map(pdd->pd, get_ms(tr, obj)->ms) == 0) {
            ERROR_PRINT("Record %d: Unable to grant user map access.\n",
                        data->rec_num);

            return 1;
        }

        break;
    }

    return 0;
}

#if defined(CONFIG_ZONE)
static int new_zone(bi_name_t pool, const bi_user_data_t* data)
{
    bi_tracker_t* tr      = (bi_tracker_t*) data->user_data;
    pool_data_t* virtpool = get_pool(tr, pool, true);
    struct zone* zone     = zone_create(&iguana_pd, virtpool->u.vref);

    if (zone != NULL) {
        zone_data_t* zd = &tr->zone_array[tr->free_zone++];

        assert(tr->free_zone <= data->total_zones);

        zd->hdr.type = TYPE_ZONE;
        zd->zone     = zone;

        tr->obj_array[tr->obj_array_size] = zd;

        assert(tr->obj_array_size < tr->total_objects);
    } else {
        ERROR_PRINT("Record %d: Failed to create a Zone.\n", data->rec_num);

        return BI_NAME_INVALID;
    }

    return tr->obj_array_size++;
}

static int add_zone_window(bi_name_t zone, size_t base,
                           const bi_user_data_t* data)
{
    bi_tracker_t* tr   = (bi_tracker_t*) data->user_data;
    zone_data_t* zd    = get_zone(tr, zone);
    struct virtmem *vm =
        virtpool_add_alloced_memory(zd->zone->morepool,
                                    base,
                                    base + MIN_WINDOW - 1);

    if (vm == NULL) {
        ERROR_PRINT("Record %d: Cannot allocate zone window.\n",
                    data->rec_num);

        return 1;
    }

    if (zone_add_window(zd->zone, vm) < 0) {
        ERROR_PRINT("Record %d: Cannot add new zone window.\n",
                    data->rec_num);

        return 1;
    }

    return 0;
}
#endif

word_t kernel_max_spaces; /* Used in space.c */
word_t kernel_max_mutexes; /* Used in mutex.c */

static int kernel_info(int max_spaces, int max_mutexes,
                       int max_root_caps,
                       const bi_user_data_t *data)
{
    kernel_max_spaces = max_spaces;
    kernel_max_mutexes = max_mutexes;
    /* Root caps ignored for now. */

    return 0;
}

static int cleanup(const bi_user_data_t* data)
{
    bi_tracker_t* tr = (bi_tracker_t*) data->user_data;

    if (close_obj_env(&iguana_pd_data)) {
        ERROR_PRINT("Record END: Not enough memory to close off the Iguana Server object environment.");

        return 1;
    }

    /* These aren't needed any more. */
    free(tr->cap_array);
    free(tr->pool_array);
    free(tr->obj_array);
    free(tr->thread_array);
    free(tr->ms_array);
    free(tr->pd_array);
    free(tr->zone_array);

    return 0;
}

static const bi_callbacks_t bi_callbacks = {
    init,
    cleanup,

    init_mem,
    init_mem2,

    new_pd,
    new_ms,
    add_virt_mem,
    add_phys_mem,
    new_thread,
    run_thread,
    map,
    attach,
    grant,
    argv,
    register_server,
    register_callback,
    register_stack,
    new_cap,
    grant_cap,
    export_object,
    export_struct,
    register_env,
    new_pool,
    grant_interrupt,
    security_control,
#if defined(CONFIG_ZONE)
    new_zone,
    add_zone_window,
#else
    NULL,
    NULL,
#endif
    kernel_info
};

int
bi_execute(void * bootinfo)
{
    bi_tracker_t tracker;

    memzero(&tracker, sizeof(tracker));

    /* Initialise the structure describing the iguana PD. */
    iguana_pd_data.hdr.type    = TYPE_PD;
    iguana_pd_data.pd          =  &iguana_pd;
    iguana_pd_data.clist       = NULL;
    iguana_pd_data.env_closed  = false;
    iguana_pd_data.env_base    = NULL;
    iguana_pd_data.free_env    = NULL;
    iguana_pd_data.last_string = NULL;

    return bootinfo_parse(bootinfo, &bi_callbacks, &tracker);
}

