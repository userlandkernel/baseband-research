/*
 * Copyright (c) 2002, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2005, National ICT Australia (NICTA)
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
 * Description:  Interrupt handling
 */

#include <l4.h>
#include <debug.h>
#include <kdb/tracepoints.h>
#include <tcb.h>
#include <space.h>
#include <interrupt.h>
#include <schedule.h>
#include <smp.h>
#include <atomic_ops/atomic_ops.h>

#include <arch/platform.h>

DECLARE_TRACEPOINT(INTERRUPT);
DECLARE_TRACEPOINT(SYSCALL_INTERRUPT_CONTROL);

namespace PlatformSupport {

/*
 * Deliver notify bits to a handler thread.
 * If cont is NON-NULL, only one interrupt descriptors is pending
 * and only one handler will be woken up.
 */
bool deliver_notify(void *handler, word_t notifybits, continuation_t cont)
{
    tcb_t *handler_tcb = (tcb_t*)handler;

    word_t bits = handler_tcb->add_notify_bits(notifybits);

    TRACEPOINT_TB(INTERRUPT, printf("IRQ notify %T:%lx\n", handler_tcb, notifybits),
            "irq %lx, %lx", handler_tcb->get_global_id().get_raw(), notifybits);

async_wakeup_check:
    if (EXPECT_TRUE( (bits & handler_tcb->get_notify_mask()) &&
                ( handler_tcb->get_state().is_waiting_notify() ||
                  (handler_tcb->get_state().is_waiting() &&
                   ((word_t)handler_tcb->get_partner() == ~0UL)
                     ) ) ))
    {
        if (!handler_tcb->grab()) {
            okl4_atomic_barrier_smp();
            goto async_wakeup_check;
        }
        //printf(" - irq wakeup (to=%t)\n", handler_tcb);

        handler_tcb->sent_from = NILTHREAD;
        handler_tcb->set_tag(msg_tag_t::nil_tag()); /* clear tag of tcb */

        if (EXPECT_TRUE(cont)) {
            get_current_scheduler()->activate_sched(
                    handler_tcb, thread_state_t::running,
                    get_current_tcb(), cont,
                    scheduler_t::preempting_thread);
            NOTREACHED();
        } else {
            get_current_scheduler()->activate(handler_tcb, thread_state_t::running);
            return true;
        }
    }
    /* Do not wakeup destination */
    return false;
}

}

INLINE void setup_notify_return(tcb_t *tcb)
{
    word_t mask = tcb->get_notify_mask();
    word_t bits = tcb->sub_notify_bits(mask);

    tcb->set_tag(msg_tag_t::notify_tag());
    tcb->set_mr(1, bits & mask);
}

extern "C" CONTINUATION_FUNCTION(check_async_irq)
{
    tcb_t * current = get_current_tcb();

    setup_notify_return(current);

    PROFILE_STOP(sys_interrupt_ctrl);
    return_interrupt_control(1, TCB_SYSDATA_IPC(current)->ipc_return_continuation);
}

SYS_INTERRUPT_CONTROL(threadid_t tid, irq_control_t control)
{
    PROFILE_START(sys_interrupt_ctrl);
    continuation_t continuation = ASM_CONTINUATION;

    tcb_t *handler;
    tcb_t *current = get_current_tcb();
    utcb_t *utcb;
    word_t ret;

    TRACEPOINT (SYSCALL_INTERRUPT_CONTROL,
                printf ("SYS_INTERRUPT_CONTROL: tid=%t, "
                        "control=%x (count=%d , op=%d , req=%x, notify_bit=%d)\n",
                        TID (tid), control.get_raw(),
                        control.get_count(),
                        control.get_op(),
                        control.get_request(),
                        control.get_notify_bit()));

    if (tid.is_nilthread()) {
        handler = acquire_read_lock_tcb(current);
    } else {
        handler = get_current_clist()->lookup_thread_cap_locked(tid);
    }

    /*
     * handler does not exist
     */
    if (EXPECT_FALSE(handler == NULL))
    {
        current->set_error_code(EINVALID_THREAD);
        goto error_out;
    }

    /* Check privilege */
    if ( EXPECT_FALSE( handler->get_space() != get_current_space() &&
                !is_privileged_space(get_current_space()) ))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out_locked;
    }

    utcb = handler->get_utcb();

    /* Call platform code to configure interrupts */
    ret = Platform::config_interrupt(
                    (Platform::irq_desc_t*)&current->get_utcb()->mr[0],
                    handler, control, utcb);

    if (EXPECT_FALSE(ret)) {
        current->set_error_code(ret);
        goto error_out_locked;
    }

    handler->unlock_read();

    /* Determine if the user has requested to sleep until our next interrupt
     * arrives. */
    if (EXPECT_TRUE(control.get_op() == irq_control_t::op_ack_wait)) {
#if defined(CONFIG_MUNITS)
        /* Eagerly set state to avoid races with other threads attempting to
         * contact us. */
        current->set_state(thread_state_t::waiting_notify);
        okl4_atomic_barrier_smp();
#endif

        /* Check to see if we have any pending notify bits: if so, we return
         * immeadiatly instead of sleeping. */
        if (EXPECT_TRUE( current->get_notify_bits() & current->get_notify_mask() ))
        {
            //printf("notify recieve (curr=%t)\n", current);
            setup_notify_return(current);

            PROFILE_STOP(sys_interrupt_ctrl);
#if defined(CONFIG_MUNITS)
            /* Restore previous state. */
            current->set_state(thread_state_t::running);
#endif
            return_interrupt_control(1, continuation);
        }

        /* Otherwise, sleep until we get our next interrupt. */
        TCB_SYSDATA_IPC(current)->ipc_return_continuation = continuation;

        get_current_scheduler()->
            deactivate_sched(current, thread_state_t::waiting_notify,
                    current, check_async_irq,
                    scheduler_t::sched_default);
    }

    /* No further operations to perform. */
    PROFILE_STOP(sys_interrupt_ctrl);
    return_interrupt_control(1, continuation);

error_out_locked:
    handler->unlock_read();
error_out:
    PROFILE_STOP(sys_interrupt_ctrl);
    return_interrupt_control(0, continuation);
}

