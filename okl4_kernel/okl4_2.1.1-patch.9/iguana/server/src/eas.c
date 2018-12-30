#if defined(CONFIG_EAS)
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
#include <assert.h>
#include <stdio.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4e/map.h>
#include <inttypes.h>
#include "pd.h"
#include "eas.h"
#include "util.h"
#include "objtable.h"
#include "thread.h"

int
eas_setup(struct eas *eas, struct pd* owner, L4_Fpage_t utcb, int pid, L4_SpaceId_t *l4_id)
{
    L4_Word_t r;
#if !defined(ARM_PID_RELOC)
    int pid = 0;
#endif

    r = space_setup_eas(&eas->space, EAS_MAGIC, owner, utcb, 0, pid);

    if (r != 0) {
        return -1;
    }
    *l4_id = eas->space.id;
    thread_list_init(&eas->threads);

    return 0;
}

void
eas_delete(struct eas *eas)
{
    struct thread_node *thread;
    struct thread *dead_thread;

    /* Delete all threads */
    for (thread = eas->threads.first; thread->next != eas->threads.first;) {
        dead_thread = &thread->data;
        thread = thread->next;
        thread_delete(dead_thread);
    }

    space_cleanup(&eas->space);
    eas_list_delete(eas);
}

/*
 * FIXME add exception_handler 
 */
struct thread *
eas_create_thread(struct eas *self, L4_ThreadId_t pager,
                  L4_ThreadId_t scheduler, uintptr_t utcb)
{
    struct thread *thread = NULL;
    L4_Word_t r;
    int r1;

    thread = thread_list_create_back(&self->threads);

    if (thread == NULL) {
        return NULL;
    }
    thread->id = L4_nilthread;
    thread->handle = L4_nilthread;

    r1 = thread_alloc(thread);      /* Allocate a thread id */
    if (r1 != 0) {
        thread->id = L4_nilthread;
        thread->handle = L4_nilthread;
        thread_list_delete(thread);
        return NULL;
    }
    thread->magic = THREAD_MAGIC;
    thread->eas = self;
    thread->owner = self->space.owner;
    thread->utcb = (void *)utcb;

    r = thread_new(thread, self->space.id, scheduler, pager, pager);
    
    if (r == 1)
    {
        return thread;
    }

    if (L4_ErrorCode() == L4_ErrNoMem || L4_ErrorCode() == L4_ErrUtcbArea ||
        L4_ErrorCode() == L4_ErrNonExist)
    {
        /*
         * L4 has run out of memory... this is probably very bad, but we want
         * to keep going for as long as we can 
         */
        thread_free(thread->id);
        thread->id = L4_nilthread;
        thread->handle = L4_nilthread;
        thread_list_delete(thread);

    } else {
        ERROR_PRINT("Thread control failed. %lx\n", L4_ErrorCode());
        assert(!"This shouldn't happen");
    }
    return NULL;
}

void
eas_delete_thread(struct eas *self, struct thread *thread)
{
    thread_delete(thread);
}

int
eas_map(struct eas *eas, L4_Fpage_t src_fpage, uintptr_t dst_addr,
        uintptr_t attrib)
{
    uintptr_t base, end, phys, virt;
    struct memsection *ms;
    size_t size;
    int r, rwx;

    base = L4_Address(src_fpage);
    end = base + L4_Size(src_fpage) - 1;
    ms = objtable_lookup((void *)base);

    /*
     * XXX: we need to check the attribs and rights 
     */
    attrib &= ms->attributes;
    rwx = L4_Rights(src_fpage);

    /*
     * XXX: we map 4k fpages even when bigger mappings are possible 
     */
    for (virt = base; virt < end; virt += BASE_PAGESIZE) {
        if (!memsection_lookup_phys(ms, virt, &phys, &size, &iguana_pd))
            continue;
        if (size > BASE_PAGESIZE)
            phys += virt % size;
        r = l4e_map(eas_l4_space(eas), dst_addr, dst_addr + BASE_PAGESIZE - 1,
                    phys, rwx, attrib);
        if (r != 0)
            return r;
        dst_addr += BASE_PAGESIZE;
    }
    return 0;
}

void
eas_unmap(struct eas *eas, L4_Fpage_t dst_fpage)
{
    l4e_unmap_fpage(eas_l4_space(eas), dst_fpage);
}

#if defined(ARM_SHARED_DOMAINS)
int eas_share_domain(struct eas *eas)
{
    int r;
    L4_LoadMR(0, eas_l4_space(eas).raw);

    r = L4_SecurityControl(pd_l4_space(eas->space.owner),
            L4_SecurityCtrl_domain(L4_SecurityControl_DomainShareDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant) | 0);

    return (r == 0);
}

void eas_unshare_domain(struct eas *eas)
{
    int r;
    L4_LoadMR(0, eas_l4_space(eas).raw);

    r = L4_SecurityControl(pd_l4_space(eas->space.owner),
            L4_SecurityCtrl_domain(L4_SecurityControl_DomainShareDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpRevoke) | 0);
}
#endif

#if defined(ARM_PID_RELOC)
uintptr_t eas_modify(struct eas *eas, int pid)
{
    int r;

    r = L4_SpaceControl(eas_l4_space(eas), L4_SpaceCtrl_resources,
            L4_rootclist, L4_Nilpage, pid, NULL);

    // XXX setup window
#if defined(ARM_SHARED_DOMAINS)
    if (r) {
        L4_Fpage_t src_fpage;

        src_fpage = L4_FpageLog2((uintptr_t)pid * (32<<20), 25);
        L4_Set_Rights(&src_fpage, 7);
        L4_Set_Meta(&src_fpage);    // fault via callback

        L4_LoadMR(1, src_fpage.raw);

        L4_LoadMR(0, eas_l4_space(eas).raw);
        r = L4_MapControl(pd_l4_space(eas->space.owner), L4_MapCtrl_MapWindow);
    }
#endif

    return (r == 0);
}
#endif
#endif
