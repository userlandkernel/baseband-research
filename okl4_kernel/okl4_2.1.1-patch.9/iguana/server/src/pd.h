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
 * Authors: Ben Leslie 
 * Created: Mon Jul 5 2004
 * 
 * Describes the the iguana protection domain. 
 */

#ifndef _IGUANA_PD_H_
#define _IGUANA_PD_H_

#include <l4/thread.h>
#include <bit_fl/bit_fl.h>
#include <circular_buffer/cb.h>
#include <iguana/cap.h>
#include <iguana/types.h>
#include "eas.h"
#include "memsection.h"
#include "physmem.h"
#if defined(CONFIG_SESSION)
#include "session.h"
#endif
#include "thread.h"
#include "virtmem.h"
#include "zone.h"

/* This is the size of the Utcb area */
#define MAX_THREADS_PER_APD 1000
#define MAX_THREADS_PER_PD_EXT 1000
/* FIXME: (benjl) This should be more easily configurable */

#define CLIST_SIZE 20
/* 
 * FIXME: Dynamic clist size, although array is negligble cf. other costs of a 
 * pd.
 */
extern const uintptr_t pd_magic;
extern const uintptr_t pd_ext_magic;

#define PD_MAGIC pd_magic
#define PD_EXT_MAGIC pd_ext_magic

/*
 * Declare a doubly linked list of PDs 
 */
struct pd_list {
    struct pd_node *first;
    struct pd_node *last;
};

struct clist_list {
    struct clist_node *first;
    struct clist_node *last;
};

struct clist_info {
    struct clist *clist;
    unsigned int type;
    uintptr_t length;
};

struct clist_node {
    struct clist_node *next;
    struct clist_node *prev;
    struct clist_info data;
};

static inline struct clist_info *
clist_list_create_front(struct clist_list *list)
{
    return (struct clist_info *)dl_list_create_front((struct double_list *)list,
                                                     sizeof(struct clist_info));
}

static inline struct clist_info *
clist_list_create_back(struct clist_list *list)
{
    return (struct clist_info *)dl_list_create_back((struct double_list *)list,
                                                    sizeof(struct clist_info));
}

static inline void
clist_list_delete(struct clist_info *data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
clist_list_init(struct clist_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
clist_list_clear(struct clist_list *list)
{
    dl_list_clear((struct double_list *)list);
}

struct ms_entry {
    TAILQ_ENTRY(ms_entry) ms_list;
    struct memsection *ms;
    int rwx;
};

extern struct slab_cache ms_ent_cache;

struct pd {
    struct space space;
#if defined(CONFIG_MEM_PROTECTED)
    struct space *extension;
#endif
    struct memsection *callback_buffer;
    struct cb_alloc_handle *cba;

    /*
     * Thread Info 
     */
    struct thread_list threads;
    bfl_t local_threadno;       /* Freelist for utcbs */

    /*
     * PD info 
     */
    struct pd_list pds;
    struct memsection_list memsections;
    struct eas_list eass;
#if defined(CONFIG_SESSION)
    struct session_p_list sessions;     /* Note that these are a list of
                                         * session list *, since sessions will
                                         * be in two linked lists */
#endif
    TAILQ_HEAD(clist_head, clist) owned_clists;

    TAILQ_HEAD(zone_head, zone) owned_zones;
    TAILQ_HEAD(zone_ent_head, zone_entry) attached_zones;

    /*
     * Clist info 
     */
    struct clist_list clists;

    /*
     * Explicitly allocated PhysMem objects.
     */
    struct pm_list pm_list;

    TAILQ_HEAD(ms_head, ms_entry) ms_list;
    long padding;       /* FIXME: Make 8 byte aligned for caps - alexw. */
};

static inline int
is_pd(const struct pd* ptr)
{
    return ptr != NULL && ptr->space.magic == PD_MAGIC;
}

struct pd_node {
    struct pd_node *next;
    struct pd_node *prev;
    struct pd data;
};

static inline struct pd *
pd_list_create_front(struct pd_list *list)
{
    return (struct pd *)dl_list_create_front((struct double_list *)list,
                                             sizeof(struct pd));
}

static inline struct pd *
pd_list_create_back(struct pd_list *list)
{
    return (struct pd *)dl_list_create_back((struct double_list *)list,
                                            sizeof(struct pd));
}

static inline void
pd_list_delete(struct pd *data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
pd_list_init(struct pd_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
pd_list_clear(struct pd_list *list)
{
    dl_list_clear((struct double_list *)list);
}

extern struct pd iguana_pd;
extern int iguana_pd_inited;
void pd_init(void);
void utcb_init(void);

/* FIXME: more options */
struct pd *pd_create(struct pd *pd, unsigned int max_thread);
void pd_setup_callback(struct pd *pd, struct memsection *callback_buffer);
int pd_setup_user_map(struct pd *self, struct memsection *phys);
int pd_setup_platform_control(struct pd *self);
struct thread *pd_create_thread(struct pd *pd, int priority);
void pd_delete_thread(struct pd *pd, struct thread *thread);
void pd_delete_clist(struct pd *self, cap_t *clist);

#define pd_l4_space(pd)         ((pd)->space.id)
#if defined(CONFIG_MEM_PROTECTED)
#define pd_ext_l4_space(pd)     ((pd)->extension ?      \
                                 (pd)->extension->id : L4_nilspace)
#define pd_ext_l4_utcb(pd)      ((pd)->extension ?      \
                                 (pd)->extension->utcb_area : NULL)
#endif

#if defined(CONFIG_SESSION)
/**
 * Create a new session.
 *
 * @param pd Owner of the session
 * @param client Client thread
 * @param server Server thread
 * @param clist A capability list which is address to the server's protection 
 * domain.
 * 
 * \return The new session. NULL on failure
 */
struct session *pd_create_session(struct pd *pd, struct thread *client,
                                  struct thread *server,
                                  struct clist *clist);
#endif

struct memsection *pd_create_memsection(struct pd *pd, uintptr_t size,
                                        uintptr_t base,
                                        struct physpool *physpool,
                                        struct virtpool *virtpool,
                                        struct zone *zone,
                                        uintptr_t flags);

/**
 * Create an external address space owned by a given protection domain
 *
 * @param pd The owner of the 
 *
 * @param kip The kernel information page for the external address space
 *
 * @param utcb 
 *
 * \return The newly created external address space. NULL returned in case of
 * error.
 */
#ifdef ARM_PID_RELOC
#define pd_create_eas(a,b,c,d)  pd_create_eas(a,b,c,d)
#else
#define pd_create_eas(a,b,c,d)  pd_create_eas(a,b,d)
#endif

struct eas *pd_create_eas(struct pd *pd, L4_Fpage_t utcb,
                          int pid, L4_SpaceId_t *l4_id);

/**
 * Delete an external address space owned by a given protection domain
 *
 * @param pd The owner of the 
 *
 * @param eas The external address space to delete
 *
 */
void pd_delete_eas(struct pd *pd, struct eas *eas);

/**
 * Add a clist to a given protection domain. This will add the
 * clist in the first free slot.
 *
 * @param pd protection domain to add the clist to.
 * @param clist capability list to add to protection domain.
 */
uintptr_t pd_add_clist(struct pd *pd, struct clist *clist);

/**
 * Release a clist from a given protection domain. This will remove the
 * clist, and free the slot.
 *
 * @param pd protection domain to add the clist to.
 * @param clist capability list to add to protection domain.
 */
void pd_release_clist(const struct pd *pd, const struct clist *clist);

/**
 * Delete a protection domain, (and all the items it owns)
 *
 * @param pd The protection domain to delete
 *
 * \return 0 on succes, -1 on failure.
 */
int pd_delete(struct pd *pd);

/**
 * Pretty print a protection domain structure for debugging
 * purposes.
 *
 * @param pd The protection domain to print
 */
void pd_print(struct pd *pd);

/**
 * Check that the state of a protection domain is valid. This will recursively
 * check any other data structures owned by the PD. This allows the entire system
 * state to be checked by calling pd_check_state() on the root pd.
 *
 * @param base_pd The protection domain to check
 *
 * \return 0 on success, > 0 for each failure
 */
int pd_check_state(const struct pd *base_pd);

int pd_insert_memsection(struct pd *pd, struct memsection *ms, int rwx);

int pd_attach(struct pd *pd, struct memsection *ms, int rwx);
void pd_detach(struct pd *pd, struct memsection *ms);
int pd_sync_range(struct pd *pd, uintptr_t base, uintptr_t end,
                  struct memsection *ms, int rwx);
void pd_flush_range(struct pd *pd, uintptr_t base, uintptr_t end);

int pd_map_insert(struct pd *pd, struct memsection *ms, int rwx);
int pd_map_lookup(struct pd *pd, struct memsection *ms);
void pd_map_remove(struct pd *pd, struct memsection *ms);

struct pd *pd_lookup(L4_SpaceId_t space);

struct zone *pd_create_zone(struct pd *pd, struct virtpool *pool);
int pd_attach_zone(struct pd *pd, struct zone *zone, int rwx);
void pd_detach_zone(struct pd *pd, struct zone *zone);

int pd_zonemap_insert(struct pd *pd, struct zone *zone, int rwx);
void pd_zonemap_remove(struct pd *pd, struct zone *zone);

#endif /* _IGUANA_PD_H_ */
