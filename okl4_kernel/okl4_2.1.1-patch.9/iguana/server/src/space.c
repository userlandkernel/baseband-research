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
#include <range_fl/range_fl.h>
#include <l4/ipc.h>
#include <l4/caps.h>
#include "util.h"
#include "memsection.h"
#include "pd.h"
#include "space.h"
#include "thread.h"
#include <l4/config.h>

static rfl_t space_list;
static L4_Word_t min_spaceno, max_spaceno;
static L4_ClistId_t eas_clist;

extern L4_Word_t kernel_max_spaces;

void
space_init(void)
{
    int r;

    space_list = rfl_new();

    min_spaceno = L4_SpaceNo(L4_rootspace) + 1;
    max_spaceno = kernel_max_spaces - 1;

    r = rfl_insert_range(space_list, min_spaceno, max_spaceno);
    assert(r == 0);

    /* Create an empty clist for use by eas's */
    eas_clist = L4_ClistId(1);
    r = L4_CreateClist(eas_clist, 64); /* Size of 64 */
    assert(r != 0);
}

static L4_SpaceId_t
space_alloc(void)
{
    L4_Word_t spaceno;

    spaceno = rfl_alloc(space_list);
    if (spaceno == -1UL) {
        return L4_nilspace;
    }
    return L4_SpaceId(spaceno);
}

static void
space_free(L4_SpaceId_t space)
{
    int r;

    r = rfl_free(space_list, L4_SpaceNo(space));
    assert(r == RFL_SUCCESS);
}

static int
do_space_setup(struct space *space, uintptr_t magic, struct pd *owner,
            L4_Fpage_t utcb_area, int vspace, int pid, int eas)
{
    L4_Word_t ctl, res;
    int r;

#if defined(NO_UTCB_RELOCATE)
    utcb_area = L4_Nilpage;
#else
    vspace = 0;
#endif

#if !defined(ARM_PID_RELOC)
    pid = 0;
#endif

    space->magic = magic;
    space->owner = owner;
    space->utcb_area = utcb_area;
    if ((struct pd *)space == &iguana_pd) {
        space->id = L4_rootspace;
        return 0;
    }
    space->id = space_alloc();
    assert(L4_SpaceNo(space->id) != -1UL);

    res = (vspace << 16) | pid;
    // While creating the space, set the space pager to the owner
    ctl = L4_SpaceCtrl_new | (res ? L4_SpaceCtrl_resources : 0) |
        L4_SpaceCtrl_space_pager;
    L4_LoadMR(0, pd_l4_space(owner).raw);
    r = L4_SpaceControl(space->id, ctl, (eas ? eas_clist : L4_rootclist), utcb_area, res, NULL);
    if (r != 1) {
        ERROR_PRINT_L4;
        space_free(space->id);
        return -1;
    }
    return 0;
}

int
space_setup(struct space *space, uintptr_t magic, struct pd *owner,
            L4_Fpage_t utcb_area, int vspace, int pid)
{
    return do_space_setup(space, magic, owner, utcb_area, vspace, pid, 0);
}

int
space_setup_eas(struct space *space, uintptr_t magic, struct pd *owner,
            L4_Fpage_t utcb_area, int vspace, int pid)
{
    static int init = 0;

    /* Hack for now, give eas threads access to server threads */
    if (init == 0) {
        struct thread_node *thread;
        struct thread *server_thread;
        int r;

        init = 1;
        for (thread = owner->threads.first; thread->next != owner->threads.first;) {
            server_thread = &thread->data;
            thread = thread->next;
            //printf("-- setup eas: owner: %lx\n", server_thread->id.raw);
            r = L4_CreateIpcCap(server_thread->id, L4_rootclist, server_thread->id, eas_clist);
            assert(r != 0);
        }
    }
    return do_space_setup(space, magic, owner, utcb_area, vspace, pid, 1);
}

void
space_cleanup(struct space *space)
{
    int r;

    r = L4_SpaceControl(space->id, L4_SpaceCtrl_delete,
                        L4_rootclist, L4_Nilpage, 0, NULL);
    assert(r == 1);
    space_free(space->id);
}
