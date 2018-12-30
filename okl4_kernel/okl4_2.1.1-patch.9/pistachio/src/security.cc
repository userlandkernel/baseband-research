/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description:   Security control implementation
 */

#include <l4.h>
#include <config.h>
#include <tcb.h>
#include <thread.h>
#include <security.h>

#include <kdb/tracepoints.h>
#include <arch/platform.h>

DECLARE_TRACEPOINT(SYSCALL_SECURITY_CONTROL);

static space_t* valid_space(spaceid_t space_id)
{
    space_t *space;

    // Check for valid space id
    space = get_space_list()->lookup_space(space_id);

    if (EXPECT_FALSE (space == NULL))
    {
        get_current_tcb ()->set_error_code (EINVALID_SPACE);
    }
    return space;
}


SYS_SECURITY_CONTROL (spaceid_t space_id, word_t control)
{
    PROFILE_START(sys_security_ctrl);
    continuation_t continuation = ASM_CONTINUATION;
    security_control_t ctrl;
    ctrl.set_raw(control);
    NULL_CHECK(continuation);
    tcb_t * current = get_current_tcb();
    word_t last_item = ctrl.highest_item();
    word_t i;

    TRACEPOINT (SYSCALL_SECURITY_CONTROL,
                printf ("SYSCALL_SECURITY_CONTROL: current=%p control=%lx, space=%d\n",
                        current, ctrl.get_raw(), space_id.get_spaceno());
            );

    // Check privilege
    if (EXPECT_FALSE (! is_privileged_space(get_current_space())))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out;
    }

    /*
     * Check number of items and supported domains (0,1,2 currently)
     */
    if (EXPECT_FALSE(last_item >= IPC_NUM_MR))
    {
        current->set_error_code (EINVALID_PARAM);
        goto error_out;
    }

    space_t * target_space;

    if (space_id.is_nilspace())
    {
        target_space = get_current_space();
    }
    else
    {
        // Check for valid space id
        target_space = valid_space(space_id);

        if (EXPECT_FALSE (target_space == NULL)) {
            goto error_out;
        }
    }

    i = 0;
    switch (ctrl.domain())
    {
    case security_control_t::dom_platctrl:
        if (last_item != 0) {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }

        if (ctrl.is_grant()) {
            target_space->allow_plat_control();
        } else if (ctrl.is_revoke()) {
            target_space->restrict_plat_control();
        } else {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }
        break;
    case security_control_t::dom_spcswitch:
        if (last_item != 0) {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }

        if (ctrl.is_grant()) {
            spaceid_t dest_space_id;
            dest_space_id.set_raw(current->get_mr(0));

            /* Check for valid space id */
            space_t * dest_space = valid_space(dest_space_id);
            if (EXPECT_FALSE( dest_space == NULL ))
                goto error_out;

            target_space->allow_space_switch(dest_space_id);
        } else if (ctrl.is_revoke()) {
            target_space->restrict_space_switch();
        } else {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }
        break;
#if defined(CONFIG_ARM_V5)
    case security_control_t::dom_sharedomain:
        do {
            space_t * dest;
            spaceid_t dest_sid;
            bool r = false;

            //printf("sec-control - share domain\n");

            dest_sid.set_raw(current->get_mr(i));

            dest = get_space_list()->lookup_space(dest_sid);
            // Check for valid space id
            if (EXPECT_FALSE(dest == NULL))
            {
                printf("sec-control - shr-domain on invalid spc\n");
                get_current_tcb ()->set_error_code(EINVALID_SPACE);
                goto error_out;
            }

            if (ctrl.is_manager()) {
                // printf("sec: manage: %S access to %S\n", target_space, dest);
                r = target_space->add_shared_domain(dest, true);
            } else if (ctrl.is_grant()) {
                // printf("sec: shared: %S access to %S\n", target_space, dest);
                r = target_space->add_shared_domain(dest, false);
            } else if (ctrl.is_revoke()) {
                // printf("sec: unshare: %S access to %S\n", target_space, dest);
                r = target_space->remove_shared_domain(dest);
            } else {
                printf("sec-control - shr-domain on invalid op\n");
                get_current_tcb ()->set_error_code (EINVALID_PARAM);
                goto error_out;
            }

            if (r == false) {
                /* Error code set in add/remove_shared_domain() */
                goto error_out;
            }
            i++;
        } while (i <= last_item);
        break;
#endif
    case security_control_t::dom_irqaccess:
        {
            word_t int_control = last_item;
            word_t ret;

            if (ctrl.is_grant()) {
                int_control |= (0 << 16);
            } else if (ctrl.is_revoke()) {
                int_control |= (1 << 16);
            } else {
                get_current_tcb ()->set_error_code (EINVALID_PARAM);
                goto error_out;
            }

            /* Call platform to configure interrupt security */
            ret = Platform::security_control_interrupt(
                    (Platform::irq_desc_t*)&current->get_utcb()->mr[0],
                    target_space, int_control);
            if (ret) {
                get_current_tcb()->set_error_code(ret);
                goto error_out;
            }
        }
        break;
    case security_control_t::dom_usermap:
        if (last_item != 1) {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }

        if (ctrl.is_grant()) {
            target_space->add_user_map_area(current->get_mr(0), current->get_mr(1));
        } else if (ctrl.is_revoke()) {
            if (!target_space->remove_user_map_area(current->get_mr(0), current->get_mr(1))) {
                get_current_tcb ()->set_error_code (EINVALID_PARAM);
                goto error_out;
            }
        } else {
            get_current_tcb ()->set_error_code (EINVALID_PARAM);
            goto error_out;
        }
        break;
    default:
        current->set_error_code (EINVALID_PARAM);
        goto error_out;
    }

    PROFILE_STOP(sys_security_ctrl);
    return_security_control(1, continuation);

error_out:
    PROFILE_STOP(sys_security_ctrl);
    return_security_control(0, continuation);
}
