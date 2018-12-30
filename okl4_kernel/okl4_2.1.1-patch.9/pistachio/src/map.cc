/*
 * Copyright (c) 2005-2006, National ICT Australia (NICTA)
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
 * Description:   Mapping implementation
 */

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <kdb/tracepoints.h>
#include <map.h>
#include <tcb.h>
#include <syscalls.h>


DECLARE_TRACEPOINT (SYSCALL_MAP_CONTROL);

SYS_MAP_CONTROL (spaceid_t space_id, word_t control)
{
    PROFILE_START(sys_map_ctrl);
    continuation_t continuation = ASM_CONTINUATION;
    NULL_CHECK(continuation);
    map_control_t ctrl;
    tcb_t *current;
    space_t *space, *curr_space;
    word_t count;

    ctrl = control;

    TRACEPOINT (SYSCALL_MAP_CONTROL,
                printf("SYS_MAP_CONTROL: space=%d, control=%p "
                       "(n = %d, %c%c)\n", space_id.get_spaceno(),
                       control, ctrl.highest_item(),
                       ctrl.is_modify() ? 'M' : '~',
                       ctrl.is_query() ? 'Q' : '~'));

    current = get_current_tcb();

    curr_space = get_current_space();
    space = get_space_list()->lookup_space(space_id);
    /*
     * Check for valid space id and that either the caller is the root task
     * or the destination space has the current space as its pager
     */
    if (EXPECT_FALSE (space == NULL))
    {
        current->set_error_code (EINVALID_SPACE);
        goto error_out;
    }

    if (EXPECT_FALSE(!is_privileged_space(curr_space) &&
            space->get_space_pager() != curr_space->get_space_id()))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out;
    }

    count = ctrl.highest_item()+1;

    /* check for message overflow !! */
    if (EXPECT_FALSE((count*2) > IPC_NUM_MR))
    {
        current->set_error_code (EINVALID_PARAM);
        goto error_out;
    }

    for (word_t i = 0; i < count; i++)
    {
        phys_desc_t tmp_base;
        perm_desc_t tmp_perm;

#if defined(CONFIG_ARM_V5)
        if (ctrl.is_window())
        {
            fpage_t fpg;
            space_t * target;
            spaceid_t target_sid;
            bool r;

            target_sid.set_raw(current->get_mr(i*2));
            fpg.raw = current->get_mr(i*2+1);

            target = get_space_list()->lookup_space(target_sid);
            // Check for valid space id
            if (EXPECT_FALSE (target == NULL))
            {
                get_current_tcb ()->set_error_code (EINVALID_SPACE);
                goto error_out;
            }

            if (space->is_sharing_domain(target)) {
                // printf("map: map window : %S -> %S - fpg: %lx\n", space, target, fpg.raw);
                r = space->window_share_fpage(target, fpg);

                if (r == false) {
                    /* Error code set in window_share_fpage() */
                    goto error_out;
                }
            } else {
                printf("No permission - domain not shared!!\n");
                get_current_tcb ()->set_error_code (ENO_PRIVILEGE);
                goto error_out;
            }

            continue;
        }
#endif

        // Read the existing pagetable
        if (ctrl.is_query())
        {
            fpage_t fpg;

            tmp_perm.clear();
            fpg.raw = current->get_mr(i*2+1);

            space->read_fpage(fpg, &tmp_base, &tmp_perm);
        }

        // Modify the page table
        if (ctrl.is_modify())
        {
            phys_desc_t base;
            fpage_t fpg;

            base = current->get_mr(i*2);
            fpg.raw = current->get_mr(i*2+1);

            /*
             * Check for valid fpage
             */
            if (EXPECT_FALSE(fpg.get_address() != fpg.get_base()))
            {
                get_current_tcb ()->set_error_code (EINVALID_PARAM);
                goto error_out;
            }

            if (fpg.get_rwx() == 0)
            {
                space->unmap_fpage(fpg, false);
            }
            else
            {
                /*
                 * Check that the physical backing is within the allowed
                 * mapping range
                 */
                if (EXPECT_FALSE(!is_privileged_space(curr_space) &&
                            (!curr_space->is_user_map_area(base.get_base(), fpg.get_size()))))
                {
                    current->set_error_code (ENO_PRIVILEGE);
                    goto error_out;
                }

                if (EXPECT_FALSE(!space->map_fpage(base, fpg)))
                {
                    /* Error code set in map_fpage */
                    goto error_out;
                }
            }
        }

        // Write back data to MRs
        if (ctrl.is_query())
        {
            current->set_mr(i*2, tmp_base.get_raw());
            current->set_mr(i*2+1, tmp_perm.get_raw());
        }

    }

    PROFILE_STOP(sys_map_ctrl);
    return_map_control(1, continuation);

error_out:
    PROFILE_STOP(sys_map_ctrl);
    return_map_control(0, continuation);
}
