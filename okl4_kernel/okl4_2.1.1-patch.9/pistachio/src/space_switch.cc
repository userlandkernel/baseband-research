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
 * Description: Space switch implementation.
 */

#include <l4.h>
#include <schedule.h>
#include <space.h>
#include <syscalls.h>
#include <tcb.h>
#include <thread.h>
#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYSCALL_SPACE_SWITCH);

SYS_SPACE_SWITCH(threadid_t tid, spaceid_t space_id, word_t utcb_location)
{
    PROFILE_START(sys_spaceswitch);
    continuation_t continuation = ASM_CONTINUATION;
    word_t retval = 0;

    TRACEPOINT(SYSCALL_SPACE_SWITCH,
               printf("SYS_SPACE_SWITCH: tid=%t, space=%d, utcb_location=%x\n",
                      TID(tid), space_id.get_spaceno(), utcb_location));

    tcb_t *current = get_current_tcb();

    space_t *new_space, *old_space;
    utcb_t *old_utcb;
    word_t old_utcb_location;
    utcb_t *new_utcb;
    tcb_t *tcb;

    if (tid.is_myself()) {
        tcb = acquire_read_lock_tcb(current);
    } else {
        tcb = get_current_clist()->lookup_thread_cap_locked(tid);
    }

    /* Check for valid thread id. */
    if (EXPECT_FALSE(tcb == NULL)) {
        current->set_error_code(EINVALID_THREAD);
        goto out1;
    }

    new_space = get_space_list()->lookup_space(space_id);
    /* Check for valid space id. */
    if (EXPECT_FALSE(new_space == NULL))
    {
        current->set_error_code(EINVALID_SPACE);
        goto out1_locked;
    }

    old_space = tcb->get_space();

    if (!is_privileged_space(current->get_space()))
    {
        /* if invoked from non-previledged thread, only the thread
         * from the same space and has the capability set can perform
         * the op.
         */
        if (EXPECT_FALSE(current->get_space() != old_space))
        {
            current->set_error_code(ENO_PRIVILEGE);
            goto out1_locked;
        }

        /* check security */
        if (EXPECT_FALSE(!old_space->may_space_switch(space_id)))
        {
            current->set_error_code(ENO_PRIVILEGE);
            goto out1_locked;
        }
    }

    /* Check for valid UTCB location. */
    if (EXPECT_FALSE(!new_space->check_utcb_location(utcb_location)))
    {
        current->set_error_code(EUTCB_AREA);
        goto out1_locked;
    }

    /* Work out where we're coming from. */
    get_current_scheduler()->pause(tcb);
    tcb->unlock_read();

    old_utcb = tcb->get_utcb();
    old_utcb_location = tcb->get_utcb_location();

    /* Try allocate new UTCB in new space */
    tcb->set_utcb_location(utcb_location);
    new_utcb = new_space->allocate_utcb(tcb);

    if (EXPECT_FALSE(new_utcb == NULL)) {
        /* Back out any changes we've made. */
        tcb->set_utcb_location(old_utcb_location);
        current->set_error_code(ENO_MEM);
        goto out2;
    }

    /* Fix up the thread's space. */
    old_space->remove_tcb(tcb);
    tcb->set_space(new_space);
    new_space->add_tcb(tcb);

    /* Fix up the UTCB and copy across the old contents.  */
    tcb->set_utcb(new_utcb);
    new_utcb->copy(old_utcb);
    old_space->free_utcb(old_utcb);

    /* Store the base space if switching to the extended space.
     * This allows the extended space to perform actions on the base space */
    if (!tcb->has_base_space()) {
        tcb->set_base_space(old_space);
    } else {
        tcb->clear_base_space();
    }
    
    /* If the target is the current thread, switch to the new space now.  */
    if (tcb == current) {
        new_space->activate(tcb);
    }
    retval = 1;

out2:
    get_current_scheduler()->unpause(tcb);
    PROFILE_STOP(sys_spaceswitch);
    return_space_switch(retval, continuation);

out1_locked:
    tcb->unlock_read();
out1:
    PROFILE_STOP(sys_spaceswitch);
    return_space_switch(retval, continuation);
}

/* TCB space_switch helper functions */

space_t *
tcb_t::get_base_space()
{
    return this->base_space;
}

void
tcb_t::set_base_space(space_t * space)
{
    this->base_space = space;
}

void
tcb_t::clear_base_space(void)
{
    this->base_space = NULL;
}

bool
tcb_t::has_base_space(void)
{
    return (this->base_space != NULL);
}
