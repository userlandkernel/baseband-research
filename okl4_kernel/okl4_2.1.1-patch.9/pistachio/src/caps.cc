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
/*
 * Description: Capability management
 */

#include <kernel/l4.h>
#include <kernel/debug.h>
#include <kernel/tcb.h>
#include <kernel/caps.h>
#include <kernel/clist.h>
#include <kernel/kmemory.h>
#include <kernel/kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYSCALL_CAP_CONTROL);
DECLARE_KMEM_GROUP (kmem_clistids);
DECLARE_KMEM_GROUP (kmem_clist);

/* Table containing mappings from clistid_t to clist_t* */
clistid_lookup_t clist_lookup;

#define CONFIG_MAX_CLISTS   16
void SECTION(SEC_INIT) init_clistids(void)
{
    void * new_table;
    word_t entries = CONFIG_MAX_CLISTS;
    TRACE_INIT("Init clistids for %d clists\n", entries);
    ASSERT(ALWAYS, entries > 0);

    new_table = kmem.alloc(kmem_clistids, sizeof(clist_t*) * entries, true);
    ASSERT(ALWAYS, new_table);

    clist_lookup.init(new_table, entries);
}


SYS_CAP_CONTROL(clistid_t clist_id, cap_control_t control)
{
    PROFILE_START(sys_cap_ctrl);
    continuation_t continuation = ASM_CONTINUATION;

    TRACEPOINT (SYSCALL_CAP_CONTROL,
                printf("SYS_CAP_CONTROL: clist=%d, control=%p\n",
                    clist_id.get_clistno(), control.get_raw()));

    tcb_t *current = get_current_tcb();
    tcb_t *tcb_locked;
    clist_t * clist;

    // Check privilege
    if (EXPECT_FALSE (! is_privileged_space(get_current_space())))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out;
    }

    clist = get_clist_list()->lookup_clist(clist_id);

    switch (control.get_op())
    {
    case cap_control_t::op_create_clist:
        {
            word_t entries, alloc_size;

            /* Check for not existing clist id. */
            if (EXPECT_FALSE(clist != NULL) || !get_clist_list()->is_valid(clist_id))
            {
                current->set_error_code(EINVALID_CLIST);
                goto error_out;
            }

            entries = current->get_mr(0);
            alloc_size = (word_t)addr_align_up((addr_t)(sizeof(cap_t) * entries + sizeof(clist_t)), KMEM_CHUNKSIZE);

            clist_t* new_list = (clist_t*)kmem.alloc(kmem_clist, alloc_size, true);
            if (new_list == NULL)
            {
                current->set_error_code(ENO_MEM);
                goto error_out;
            }

            new_list->init(entries);

            get_clist_list()->add_clist(clist_id, new_list);
            //printf("Create clist id: %ld size: %ld\n", clist_id.get_raw(), entries);
        }
        break;
    case cap_control_t::op_delete_clist:
        {
            word_t entries, alloc_size;

            /* Check for valid clist id. */
            if (EXPECT_FALSE(clist == NULL))
            {
                current->set_error_code(EINVALID_CLIST);
                goto error_out;
            }

            /* Check for empty clist. */
            if (EXPECT_FALSE(clist->get_space_count() != 0))
            {
                current->set_error_code(ECLIST_NOT_EMPTY);
                goto error_out;
            }

            if (EXPECT_FALSE(!clist->is_empty()))
            {
                current->set_error_code(ECLIST_NOT_EMPTY);
                goto error_out;
            }

            entries = clist->num_entries();

            get_clist_list()->remove_clist(clist_id);

            alloc_size = (word_t)addr_align_up((addr_t)(sizeof(cap_t) * entries + sizeof(clist_t)), KMEM_CHUNKSIZE);
            kmem.free(kmem_clist, clist, alloc_size);
            //printf("Deleted clist: %ld\n", clist_id.get_raw());
        }
        break;
    case cap_control_t::op_copy_cap:
        {
            threadid_t thread_id = threadid(current->get_mr(0));
            clistid_t target_list_id = clistid(current->get_mr(1));
            threadid_t target_cap = threadid(current->get_mr(2));
            clist_t *target_list;

            /* Check for valid clist id. */
            if (EXPECT_FALSE(clist == NULL))
            {
                current->set_error_code(EINVALID_CLIST);
                goto error_out;
            }

            if (EXPECT_FALSE(!(get_clist_list()->is_valid(target_list_id) &&
                            (target_list = get_clist_list()->lookup_clist(target_list_id)) )))
            {
                current->set_error_code(EINVALID_CLIST);
                goto error_out;
            }

            tcb_locked = clist->lookup_thread_cap_locked(thread_id);
            if (EXPECT_FALSE(tcb_locked == NULL)) {
                current->set_error_code(EINVALID_CAP);
                goto error_out;
            }

            /* ensure target cap is valid */
            if (EXPECT_FALSE(!target_list->is_valid(target_cap))) {
                current->set_error_code(EINVALID_CAP);
                goto error_out_locked;
            }

            /* try to insert the cap */
            if (!target_list->add_ipc_cap(target_cap, tcb_locked)) {
                current->set_error_code(EINVALID_CAP);
                goto error_out_locked;
            }
            //printf("Add IPC cap: %ld to list %ld   - (copy from %ld  in list %ld)\n", target_cap.get_threadno(), target_list_id.get_raw(), thread_id.get_threadno(), clist_id.get_raw());
            tcb_locked->unlock_read();
        }
        break;
    case cap_control_t::op_delete_cap:
        {
            threadid_t thread_id = threadid(current->get_mr(0));

            /* Check for valid clist id. */
            if (EXPECT_FALSE(clist == NULL))
            {
                current->set_error_code(EINVALID_CLIST);
                goto error_out;
            }

            if (EXPECT_FALSE(!clist->is_valid(thread_id))) {
                current->set_error_code(EINVALID_CAP);
                goto error_out;
            }

            if (!clist->remove_ipc_cap(thread_id)) {
                current->set_error_code(EINVALID_CAP);
                goto error_out;
            }

            //printf("Del IPC cap: %ld in list %ld\n", thread_id.get_threadno(), clist_id.get_raw());
        }
        break;
    default:
        current->set_error_code(EINVALID_PARAM);
        goto error_out;
    }

    PROFILE_STOP(sys_cap_ctrl);
    return_cap_control(1, continuation);

error_out_locked:
    tcb_locked->unlock_read();
error_out:
    PROFILE_STOP(sys_cap_ctrl);
    return_cap_control(0, continuation);
}

