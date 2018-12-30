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
 * Author: Ben Leslie Created: Wed Jul 7 2004
 * 
 * Describes the iguana external address space 
 */

#ifndef _IGUANA_EAS_H_
#define _IGUANA_EAS_H_

#include <l4/types.h>
#include <stdint.h>
#include "space.h"
#include "thread.h"

#ifdef ARM_PID_RELOC
#define IF_PID(x) x
#define eas_setup(a,b,c,d,e)  eas_setup(a,b,c,d,e)
#else
#define IF_PID(x)
#define eas_setup(a,b,c,d,e)  eas_setup(a,b,c,e)
#endif

#define EAS_MAGIC 0x9fffefff

struct eas {
    struct space space;
    struct thread_list threads;
};

#define eas_l4_space(eas) (eas->space.id)

struct eas_list {
    struct eas_node *first;
    struct eas_node *last;
};

struct eas_node {
    struct eas_node *next;
    struct eas_node *prev;
    struct eas data;
};

static inline struct eas *
eas_list_create_front(struct eas_list *list)
{
    return (struct eas *)dl_list_create_front((struct double_list *)list,
                                              sizeof(struct eas));
}

static inline struct eas *
eas_list_create_back(struct eas_list *list)
{
    return (struct eas *)dl_list_create_back((struct double_list *)list,
                                             sizeof(struct eas));
}

static inline void
eas_list_delete(struct eas *data)
{
    (void)ll_delete((struct ll *)((void **)data - 2));
}

static inline void
eas_list_init(struct eas_list *list)
{
    dl_list_init((struct double_list *)list);
}

static inline void
eas_list_clear(struct eas_list *list)
{
    dl_list_clear((struct double_list *)list);
}

 /* Called to initialise the thread structure */
int eas_setup(struct eas *eas, struct pd* owner, L4_Fpage_t utcb, int pid, L4_SpaceId_t *l4_id);
void eas_delete(struct eas *eas);
/* Called to initialise the thread structure */
struct thread *eas_create_thread(struct eas *eas, L4_ThreadId_t pager, L4_ThreadId_t scheduler, uintptr_t utcb);
void eas_delete_thread(struct eas *eas, struct thread *thread);

int eas_map(struct eas *eas, L4_Fpage_t src_fpage, uintptr_t dst_addr,
            uintptr_t attributes);
void eas_unmap(struct eas *eas, L4_Fpage_t dst_fpage);

#if defined(ARM_SHARED_DOMAINS)
int eas_share_domain(struct eas *eas);
void eas_unshare_domain(struct eas *eas);
#endif

#if defined(ARM_PID_RELOC)
uintptr_t eas_modify(struct eas *eas, int pid);
#endif

#endif /* _IGUANA_EAS_H_ */
