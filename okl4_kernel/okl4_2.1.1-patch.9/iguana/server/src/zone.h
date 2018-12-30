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

#ifndef IGUANA_SERVER_SRC_ZONE_H
#define IGUANA_SERVER_SRC_ZONE_H

#include <queue/tailq.h>
#include "memsection.h"
#include "pd.h"
#include "virtmem.h"

#define MIN_WINDOW              (1024 * 1024)

#define ZONE_MAGIC              0x2011e
struct zone {
    uintptr_t magic;
    struct pd *owner;

    struct virtpool *pool;              /* Available memory */
    struct virtpool *morepool;          /* Where to go for more memory */
    struct physpool *phys_pool;         /* Available physical memory, only used
                                         * for direct memsections for now */
    struct physpool *phys_morepool;     /* Where to go for more physical memory */
    struct virtpool *direct_pool;       /* Starts out empty, gets filled when it gets
                                         * allocations from a virtpool _due_to_a_
                                         * directpool alloc.  We want to keep this
                                         * separate from the zone's pool (i.e.
                                         * the virtpool).  I'm sorry if this
                                         * unfortunate overload of terms is confusing,
                                         * please talk to Nelson or Malcolm */
    struct vm_list window_list;         /* All memory managed by zone */
    TAILQ_HEAD(, memsection) ms_list;   /* Memsections that live here */

    TAILQ_ENTRY(zone) zone_list;        /* Entry in pd->owned_zones list */
    struct pd_head pd_list;             /* PDs with access to the zone */

#if defined(ARM_SHARED_DOMAINS)
    struct pd *space;                   /* ...and a space for write access. */
#endif
};

struct zone_entry {
    TAILQ_ENTRY(zone_entry) zone_list;
    struct zone *zone;
    int rwx;
};

extern struct slab_cache zone_ent_cache;

static inline int
is_zone(const struct zone* ptr)
{
    return ptr != NULL && ptr->magic == ZONE_MAGIC;
}

struct zone *zone_create(struct pd *pd, struct virtpool *pool);
void zone_delete(struct zone *zone);

int zone_insert_memsection(struct zone *zone, struct memsection *ms);
int zone_map_memsection(struct memsection *ms);
int zone_add_memory(struct zone *zone, uintptr_t base, uintptr_t end);
int zone_expand(struct zone *zone, size_t size);
int zone_expand_direct(struct zone *zone, size_t size);
int zone_add_window(struct zone *zone, struct virtmem *vm);
int zone_shrink(struct zone *zone, uintptr_t base);
int zone_rmap_insert(struct zone *zone, struct pd *pd, int rwx);
int zone_rmap_lookup(struct zone *zone, struct pd *pd);
void zone_rmap_remove(struct zone *zone, struct pd *pd);

#endif
