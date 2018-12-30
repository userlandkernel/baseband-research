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
 * Authors: Ben Leslie, Alex Webster 
 * Created: Tue Jul 6 2004
 * 
 * Describes the iguana memory section. 
 */
#ifndef _IGUANA_MEMSECTION_H_
#define _IGUANA_MEMSECTION_H_

#include <stdint.h>
#include <bit_fl/bit_fl.h>
#include <l4/types.h>
#include <ll/ll.h>
#include <queue/tailq.h>
#include "physmem.h"
#include "slab_cache.h"
#include "session.h"
#include "virtmem.h"

extern const uintptr_t memsection_magic;
#define MEMSECTION_MAGIC memsection_magic

#define MEM_NORMAL      0x1
#define MEM_FIXED       0x2
#define MEM_DIRECT      0x4
#define MEM_UTCB        0x8
#define MEM_USER        0x10
#define MEM_INTERNAL    0x20
#if defined(CONFIG_MEMLOAD)
#define MEM_LOAD        0x40
#endif
#define MEM_PROTECTED   0x80
#define MEM_ZONE        0x100

#define MEM_VALID_USER_FLAGS ( MEM_NORMAL | MEM_FIXED | MEM_DIRECT | MEM_UTCB | MEM_USER )


struct pd_entry {
    TAILQ_ENTRY(pd_entry) pd_list;
    struct pd *pd;
    int rwx;
};

extern struct slab_cache pd_ent_cache;

/*
 * Description of the iguana memory sections 
 */
struct memsection {
    uintptr_t magic;
    struct pd *owner;           /* Our owner */
    uintptr_t base;
    uintptr_t end;
    uintptr_t flags;
    struct physpool *pool;
    uintptr_t attributes;
    struct virtmem *virt;
    struct pm_list pm_list;
    struct thread *server;
    TAILQ_HEAD(pd_head, pd_entry) pd_list;
    unsigned load_count;        /* count of pages yet to be loaded      */
    bfl_t load_map;             /* bitmap of loaded pages               */
#if defined(CONFIG_MEMLOAD)
    int (*fault_handler)(struct memsection *, uintptr_t, uintptr_t, int,
                         struct pd *, struct thread *);
#endif
    struct zone *zone;
    TAILQ_ENTRY(memsection) zone_list;
    /* Binary tree pointers */
    struct memsection *left;
    struct memsection *right;
};

static inline int
is_memsection(const struct memsection* m)
{
    return m != NULL && m->magic == MEMSECTION_MAGIC;
}

struct memsection_list {
    struct memsection_node *first;
    struct memsection_node *last;
};

struct memsection_node {
    struct memsection_node *next;
    struct memsection_node *prev;
    struct memsection data;
};

static inline struct memsection *
memsection_list_create_front(struct memsection_list *list)
{
    return (struct memsection *)dl_list_create_front((struct double_list *)list,
                                                     sizeof(struct memsection));
}

static inline struct memsection *
memsection_list_create_back(struct memsection_list *list)
{
    return (struct memsection *)dl_list_create_back((struct double_list *)list,
                                                    sizeof(struct memsection));
}

static inline void
memsection_list_delete(struct memsection *data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
memsection_list_init(struct memsection_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
memsection_list_clear(struct memsection_list *list)
{
    dl_list_clear((struct double_list *)list);
}

extern struct memsection_list internal_memsections;

struct memsection_node *memsection_new(void);
void memsection_print(struct memsection *self);
void memsection_register_server(struct memsection *self, struct thread *server);
void memsection_delete(struct memsection *memsect);
int memsection_set_attributes(struct memsection *memsect, uintptr_t attributes);
void delete_memsection_from_allocator(struct memsection_node *node);
int memsection_page_map(struct memsection *self, L4_Fpage_t from_page,
                        L4_Fpage_t to_page);
int memsection_map(struct memsection *ms, uintptr_t offset, struct physmem *pm);
int memsection_unmap(struct memsection *ms, uintptr_t offset, uintptr_t size);
int memsection_check_state(struct memsection *self);
void memsection_flush(struct memsection *memsection);
struct memsection *memsection_create_cache(struct slab_cache *sc);

int memsection_lookup_phys(struct memsection *memsection, uintptr_t addr,
                           uintptr_t *paddr, size_t *size, struct pd *pd);
int memsection_list_is_valid(struct memsection_list *list);

struct memsection *iguana_memsection_create(size_t size, uintptr_t *ret_base,
                                            uintptr_t *ret_size);

int memsection_rmap_insert(struct memsection *ms, struct pd *pd, int rwx);
void memsection_rmap_remove(struct memsection *ms, struct pd *pd);

#define HANDLE_PAGE_FAULT(ms, addr, ip, rwx, pd, thread) \
    (ms)->fault_handler(ms, addr, ip, rwx, pd, thread)

int default_fault_handler(struct memsection *ms, uintptr_t addr, uintptr_t ip,
                          int rwx, struct pd *pd, struct thread *thread);
int memload_fault_handler(struct memsection *ms, uintptr_t addr, uintptr_t ip,
                          int rwx, struct pd *pd, struct thread *thread);

#endif /* _IGUANA_MEMSECTION_H_ */
