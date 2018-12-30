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
 * Description: C IPC Fastpath
 */

#if defined(CONFIG_IPC_C_FASTPATH)

#ifdef CONFIG_SCHEDULE_INHERITANCE
#error "C IPC fastpath does not work with inheritance"
#endif

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <ipc.h>
#include <interrupt.h>
#include <syscalls.h>
#include <smp.h>
#include <arch/syscalls.h>

#if defined(CONFIG_SCHEDULE_INHERITANCE)
/* Throw an error if we attempt to compile the C fastpath with schedule
 * inheritance. */
#error "No current support for schedule inheritance in the C fastpath."
#endif

/* Return to the slowpath. */
#define SLOWPATH() \
        SYS_IPC_SLOW(to_tid, from_tid, continuation)

extern "C" CONTINUATION_FUNCTION(check_async_ipc);

/* This fast path is intended to handle only Call() and ReplyWait()
 * All other cases proceed to the slow path */
SYS_IPC_FAST(threadid_t to_tid, threadid_t from_tid)
{
    tcb_t * to_tcb = NULL;
    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();

    continuation_t continuation = ASM_CONTINUATION;
    current->sys_data.set_action(tcb_syscall_data_t::action_ipc);
    TCB_SYSDATA_IPC(current)->ipc_return_continuation = continuation;

    if (EXPECT_FALSE(to_tid.is_threadhandle()))
    {
        SLOWPATH();
    }

    if (EXPECT_FALSE(to_tid.is_nilthread() || from_tid.is_nilthread())) {
        SLOWPATH();
    }

    msg_tag_t tag = current->get_tag();

    if (EXPECT_FALSE(tag.is_notify())) {
        SLOWPATH();
    }

    to_tcb = get_current_clist()->lookup_ipc_cap_tcb(to_tid);

    if (EXPECT_FALSE(to_tcb == NULL)) {
        SLOWPATH();
    }

    /* Check that IPC Control hasn't stopped communications. */
    if (EXPECT_FALSE (to_tcb->get_global_id() != to_tid || current->cannot_send_ipc(to_tcb))) {
        SLOWPATH();
    }

    /* Should we block waiting for the thread to be ready? */
    if (EXPECT_FALSE(tag.get_untyped() > IPC_NUM_MR - 1 || !tag.recv_blocks())) {
        SLOWPATH();
    }
 
    threadid_t sender_id = current->get_global_id();

    /* not waiting || (not waiting for me && not waiting for any)
     * optimized for receive and wait any */
    if (EXPECT_FALSE((!to_tcb->get_state().is_waiting()
                      || (to_tcb->get_partner() != sender_id &&
                          !to_tcb->get_partner().is_anythread())))) {
        SLOWPATH();
    }

    to_tcb->sent_from = threadhandle(current->tcb_idx);
    to_tcb->set_partner((tcb_t*)INVALID_RAW);

    /* we set the sender space id here */
    to_tcb->set_sender_space(current->get_space_id());

    if (current->in_exception_ipc()) {
        current->copy_exception_mrs_from_frame(to_tcb);
    } else if (to_tcb->in_exception_ipc()) {
        current->copy_exception_mrs_to_frame(to_tcb);
    } else if (tag.get_untyped()) {
        current->copy_mrs(to_tcb, 1, tag.get_untyped());
    }

    /* clear all flags except propagation */
    tag.clear_receive_flags();
    to_tcb->set_tag(tag);

    /* Send finished, received phase */

    /**********************
     *     CALL CASE
     **********************/

    if (from_tid == to_tid) {

        current->set_partner(from_tid);

        scheduler->deactivate_activate_sched(current, to_tcb,
                                             thread_state_t::waiting_forever,
                                             thread_state_t::running,
                                             current, check_async_ipc,
                                             scheduler_t::sched_default);
        NOTREACHED();
    }


    /**********************
     *   OPEN WAIT CASE
     **********************/

    /* Jump to slow path if we're being polled */
    if (EXPECT_FALSE(!(from_tid.is_anythread() && !current->get_endpoint()->get_send_head()))) {
        SLOWPATH();
    }

    /* If no partner is ready to send, check if we have any pending notify bits */
    acceptor_t acceptor;
    acceptor = current->get_acceptor();
    if ( acceptor.accept_notify() && (current->get_notify_bits() & current->get_notify_mask()) )
    {
        /* Setup notify and return */
        word_t mask = current->get_notify_mask();
        word_t bits = current->sub_notify_bits(mask);
        current->set_tag(msg_tag_t::notify_tag());
        current->set_mr(1, bits & mask);
        current->set_partner(NILTHREAD);

        /* Enqueue TCB and return */
        scheduler->update_active_state(current, thread_state_t::running);
        scheduler->activate_sched(to_tcb, thread_state_t::running,
                                  current, TCB_SYSDATA_IPC(current)->ipc_return_continuation,
                                  scheduler_t::sched_default);
        NOTREACHED();
    }

    current->set_partner(from_tid);
    scheduler->deactivate_activate_sched(current, to_tcb,
                                         thread_state_t::waiting_forever,
                                         thread_state_t::running,
                                         current, check_async_ipc,
                                         scheduler_t::sched_default);
    NOTREACHED();
}

#endif /* CONFIG_IPC_C_FASTPATH */
