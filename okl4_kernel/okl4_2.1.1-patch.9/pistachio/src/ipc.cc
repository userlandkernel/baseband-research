/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   generic IPC path
 */
#include <l4.h>
#include <debug.h>
#include <kdb/tracepoints.h>

#define DO_TRACE_IPC(x...) do { printf(x); } while(0)

#if 1
#define TRACE_IPC(x...)
#define TRACE_XIPC(x...)
#define TRACE_NOTIFY(x...)
#else

#define TRACE_IPC       DO_TRACE_IPC
#define TRACE_XIPC      DO_TRACE_IPC
#define TRACE_NOTIFY    DO_TRACE_IPC
#endif

#include <tcb.h>
#include <schedule.h>
#include <ipc.h>
#include <syscalls.h>
#include <smp.h>
#include <arch/syscalls.h>
#include <profile.h>


DECLARE_TRACEPOINT(SYSCALL_IPC);
DECLARE_TRACEPOINT(IPC_TRANSFER);


INLINE bool transfer_message(tcb_t * src, tcb_t * dst)
{
    TRACEPOINT (IPC_TRANSFER,
                printf("IPC transfer message: src=%t, dst=%t\n", src, dst));

    msg_tag_t tag = src->get_tag();

    // clear all flags
    tag.clear_receive_flags();

    // we set the sender space id here
    dst->set_sender_space(src->get_space_id());

    /*
     * Any errors here will be reported as a message overflow error.
     * For the "exception" IPCs this is misleading as more than just overflow
     * is checked.  A new error type should be defined sometime.
     */
    if (EXPECT_FALSE(src->in_exception_ipc())) {
        if (! src->copy_exception_mrs_from_frame(dst)) {
            goto error;
        }
    } else if (EXPECT_FALSE(dst->in_exception_ipc())) {
        if (! src->copy_exception_mrs_to_frame(dst)) {
            goto error;
        }
    } else if (EXPECT_TRUE(tag.get_untyped())){
        if (EXPECT_FALSE(!src->copy_mrs(dst, 1, tag.get_untyped()))) {
            goto error;
        }
    }

    dst->set_tag(tag);
    return true;

error:
    // Report message overflow error
    dst->set_error_code (IPC_RCV_ERROR (ERR_IPC_MSG_OVERFLOW));
    src->set_error_code (IPC_SND_ERROR (ERR_IPC_MSG_OVERFLOW));

    tag.set_error();
    dst->set_tag(tag);
    return false;
}

INLINE void setup_notify_return(tcb_t *tcb)
{
    word_t mask = tcb->get_notify_mask();
    word_t bits = tcb->sub_notify_bits(mask);

    tcb->set_tag(msg_tag_t::notify_tag());
    tcb->set_mr(1, bits & mask);
}

/*
 * Enqueue 'to_tcb' (if it exists) and return from the IPC.
 */
NORETURN static void
enqueue_tcb_and_return(tcb_t *current, tcb_t *to_tcb,
        continuation_t continuation)
{
#if defined(CONFIG_MUNITS)
    /* FIXME : Mothra Issue #2123 : Only the scheduler should be using
     * set_state(). */
    current->set_state(thread_state_t::running);
#endif

    scheduler_t * scheduler = get_current_scheduler();
    if (to_tcb) {
        scheduler->activate_sched(to_tcb, thread_state_t::running,
                                  current, continuation,
                                  scheduler_t::sched_default);
    } else {
        ACTIVATE_CONTINUATION(continuation);
    }
}

/**
 * Restart the IPC operation as destination thread is now ready to receive
 * Threads that block on sending (in polling state) have their continuation
 * set to this function
 *
 * This is a continuation function so it's parameters are in the TCB
 *
 * @param ipc_to The to thread id of the IPC
 * @param ipc_from The from thread id of the IPC
 */
static CONTINUATION_FUNCTION(restart_ipc)
{
    tcb_t * current = get_current_tcb();

    SYS_IPC_RESTART(TCB_SYSDATA_IPC(current)->to_tid,
                    TCB_SYSDATA_IPC(current)->from_tid,
                    TCB_SYSDATA_IPC(current)->ipc_restart_continuation);
}


/**
 * This function checks whether the received message was an async IPC
 * and does the appropriate set up for return if it was.
 *
 * It then returns through TCB_SYSDATA_IPC(current)->ipc_return_continuation function
 */
extern "C" CONTINUATION_FUNCTION(check_async_ipc)
{
    tcb_t * current = get_current_tcb();
    if (EXPECT_TRUE(current->sent_from.is_nilthread() &&
                    (!current->get_tag().is_error())))
    {
        TRACE_IPC("Asynch notify wakeup\n");
        setup_notify_return(current);
        PROFILE_STOP(sys_ipc_e);
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);
    }

    TRACE_IPC("%t received msg\n", current);

    /* XXX VU: restructure switching code so that dequeueing
     * from wakeup is removed from critical path */
    PROFILE_STOP(sys_ipc_e);
    ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);
}

/**
 * Handle IPC errors
 */
static void NORETURN
return_ipc_error_send(tcb_t *current, tcb_t *to_tcb, tcb_t* from_tcb, word_t error)
{
    current->set_error_code(IPC_SND_ERROR(error));
    current->set_tag(msg_tag_t::error_tag());
    current->sent_from = NILTHREAD;

    if (to_tcb) { to_tcb->unlock_read(); }
    if (from_tcb) { from_tcb->unlock_read(); }
    PROFILE_STOP(sys_ipc_e);
    return_ipc();
}

/**
 * Handle IPC errors with wakeup to_tcb
 */
static void NORETURN
return_ipc_error_recv(tcb_t *current, tcb_t *to_tcb, tcb_t* from_tcb, word_t error)
{
    current->set_error_code(IPC_RCV_ERROR(error));
    current->set_tag(msg_tag_t::error_tag());
    current->sent_from = NILTHREAD;

    if (from_tcb) { from_tcb->unlock_read(); }
    PROFILE_STOP(sys_ipc_e);
    enqueue_tcb_and_return(current, to_tcb,
            TCB_SYSDATA_IPC(current)->ipc_return_continuation);
    NOTREACHED();
}

 /**********************************************************************
 *
 *                          IPC syscall
 *
 **********************************************************************/
SYS_IPC (threadid_t to_tid, threadid_t from_tid)
{
    PROFILE_START(sys_ipc_e);

    tcb_t * to_tcb = NULL;
    tcb_t * from_tcb = NULL;

    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();
    bool recv_blocks;
    continuation_t continuation = ASM_CONTINUATION;
    current->sys_data.set_action(tcb_syscall_data_t::action_ipc);
    TCB_SYSDATA_IPC(current)->ipc_return_continuation = continuation;

    TRACEPOINT_TB (SYSCALL_IPC,
                   msg_tag_t tag = current->get_tag ();
                   printf ("SYS_IPC: current: %t, to_tid: %t, "
                           "from_tid: %t, "
                           "tag: 0x%x (label=0x%x, %c%c%c, u=%d)\n",
                           TID(current->get_global_id()), TID(to_tid),
                           TID(from_tid), tag.raw, tag.get_label (),
                           tag.send_blocks() ? 'S' : 's',
                           tag.recv_blocks() ? 'R' : 'r',
                           tag.is_notify() ? 'N' : '~',
                           tag.get_untyped ()
                           ),
                   "sys_ipc %t => %t (recvfrom=%t)",
                   TID(current->get_global_id()),
                   TID(to_tid), TID(from_tid));


    /*
     * sending phase of ipc that uses thread handle,
     * to-thread MUST be waiting for me.
     */
    if (to_tid.is_threadhandle())
    {
        TRACE_IPC("IPC get thread handle 0x%lx\n", to_tid.get_raw());
        to_tcb = lookup_tcb_by_handle_locked(to_tid.get_raw());

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
            TRACE_NOTIFY("invalid send thread handle, %lx\n",
                         to_tid.get_raw() );
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }

        /* Check that if the to_thread is waiting for me. */
        if (EXPECT_FALSE(!to_tcb->get_state().is_waiting()
                         || !to_tcb->is_partner_valid()
                         || to_tcb->get_partner() != current))
        {
            TRACE_IPC ("dest not ready or it's partner isn't me (%t, is_wt=%d, state=%d, to_tcb->partner=0x%lx)\n",
                    to_tcb, to_tcb->get_state().is_waiting(), (word_t) to_tcb->get_state(), to_tcb->get_partner());
            return_ipc_error_send(current, to_tcb, NULL, ERR_IPC_NOPARTNER);
            NOTREACHED();
        }
        to_tid = to_tcb->get_global_id();
    }
    else if (to_tid.is_myself()) {
        to_tcb = acquire_read_lock_tcb(current);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* someone else is likely trying to delete us */
            TRACE_IPC("pending delete, TCB %p\n", current);
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }
    }
    else if (EXPECT_TRUE(!to_tid.is_nilthread()))
    {
        to_tcb = get_current_clist()->lookup_ipc_cap_locked(to_tid);

        if (EXPECT_FALSE(to_tcb == NULL))
        {
            /* specified thread id invalid */
            TRACE_IPC("invalid send tid, %t\n", to_tid.get_raw() );
            return_ipc_error_send(current, NULL, NULL, ERR_IPC_NON_EXISTING);
            NOTREACHED();
        }
    }

    /* --- send phase --------------------------------------------------- */

    /* FIXME: Async IPC assumes to_tcb is in same domain */
    if (EXPECT_TRUE(!to_tid.is_nilthread()))
    {
        ASSERT(DEBUG, to_tcb != NULL);
        msg_tag_t tag = current->get_tag ();

        recv_blocks = tag.recv_blocks();

        /* --- check for asynchronous notification -------------------------- */
        if (EXPECT_FALSE( tag.is_notify() ))
        {
            TRACE_NOTIFY("notify send phase curr=%t, to=%t\n", current, to_tcb);

            acceptor_t acceptor = to_tcb->get_acceptor();

            if ( EXPECT_TRUE(acceptor.accept_notify()) )
            {
                word_t bits = to_tcb->add_notify_bits(current->get_mr(1));
                /* I am fairly sure we don't need a barrier here as the one after set_notify_wait
                 * already eliminates the conditions leading to a race */
                /* okl4_atomic_barrier_smp(); */

                /* check if we need to wakeup the destination */

async_wakeup_check:
                okl4_atomic_barrier_smp();
                if (EXPECT_TRUE( (bits & to_tcb->get_notify_mask()) &&
                            ( to_tcb->get_state().is_waiting_notify() ||
                              ( to_tcb->get_state().is_waiting() &&
                                ((word_t)to_tcb->get_partner() == ~0UL)
                                 ) ) ))
                {
                    if (!to_tcb->grab()) {
                        goto async_wakeup_check;
                    }
                    to_tcb->unlock_read();

                    TRACE_NOTIFY("notify wakeup (to=%t)\n", to_tcb);

                    to_tcb->sent_from = NILTHREAD;
                    to_tcb->set_tag(msg_tag_t::nil_tag()); /* clear tag of tcb */
                }
                else
                {
                    /* Do no wakeup destination */
                    to_tcb->unlock_read();
                    to_tcb = NULL;
                    to_tid = NILTHREAD;
                }
            }
            else
            {
                /* destination thread not accepting notifications */
                return_ipc_error_send(current, to_tcb, NULL, ERR_IPC_NOT_ACCEPTED);
                NOTREACHED();
            }
            goto receive_phase;
        }

        TRACE_IPC("send phase curr=%t, to=%t\n", current, to_tcb);

        threadid_t sender_handle = threadhandle(current->tcb_idx);

check_waiting:
        okl4_atomic_barrier_smp();
        // not waiting || (not waiting for me && not waiting for any)
        // optimized for receive and wait any
        if (EXPECT_FALSE(
                    !to_tcb->get_state().is_waiting()
                    || (to_tcb->get_partner() != current &&
                      ((word_t)to_tcb->get_partner() != ~0UL)) ))
        {
            TRACE_IPC ("dest not ready (%t, is_wt=%d, state=%d, partner=%p)\n",
                    to_tcb, to_tcb->get_state().is_waiting(), (word_t) to_tcb->get_state(),
                    to_tcb->get_partner());
            //enter_kdebug("blocking send");

            /* thread is not receiving */
            if (EXPECT_FALSE( !tag.send_blocks() ))
            {
                TRACE_IPC ("non-blocking send returns\n");
                return_ipc_error_send(current, to_tcb, NULL, ERR_IPC_NOPARTNER);
                NOTREACHED();
            }
            // eagerly set these to allow atomic locking
            current->set_partner(to_tcb);
            to_tcb->get_endpoint()->enqueue_send(current);
#if defined (CONFIG_MUNITS)
            /**
             *  @todo FIXME: Mothra Issue #2123 - davidg.
             *  Only the scheduler should be using set_state().
             */
            current->set_state(thread_state_t::polling);
            // recheck state in case receiver has changed state 
            // whilst we were (avoid races)
            okl4_atomic_barrier_smp();
            if (EXPECT_FALSE(!(!to_tcb->get_state().is_waiting()
                               || (to_tcb->get_partner() != current
                                   && ((word_t)to_tcb->get_partner() != ~0UL)))
                            ))
            {
                to_tcb->get_endpoint()->dequeue_send(current);
                current->set_state(thread_state_t::running);
                goto check_waiting;
            }
#endif

            TCB_SYSDATA_IPC(current)->from_tid = from_tid;
            TCB_SYSDATA_IPC(current)->to_tid = to_tid;
            TCB_SYSDATA_IPC(current)->ipc_restart_continuation = continuation;

            /* Schedule somebody else while we wait for the receiver to become
             * ready. */
            to_tcb->unlock_read();
            PROFILE_STOP(sys_ipc_e);
            scheduler->
                deactivate_sched(current, thread_state_t::polling,
                                 current, restart_ipc,
                                 scheduler_t::sched_default);
            NOTREACHED();
        }
        else
        {
            /* The partner must be told who the IPC originated from. */
            if (!to_tcb->grab()) {
                goto check_waiting;
            }
            to_tcb->unlock_read();
            to_tcb->remove_dependency();

            /* Mark our partner as an invalid value, to ensure that our partner
             * can't send to us twice with a reply cap. */
            to_tcb->set_partner((tcb_t*)INVALID_RAW);

            /* set sent_from to be thread handle of the sender. */
            TRACE_IPC("set sent_from of tcb(tid) 0x%lx(0x%lx) to handle of tcb %lx which is 0x%lx\n",
                to_tcb, to_tid.get_raw(), current, sender_handle.get_raw());
            to_tcb->sent_from = sender_handle;
        }

        if (EXPECT_FALSE(!transfer_message(current, to_tcb)))
        {
            /*
             * Error transferring messages across. Reactivate our partner
             * and return with an error. */
            if (EXPECT_FALSE(!to_tcb->is_local_domain())) {
                /* Unimplemented for SMP */
                UNIMPLEMENTED();
            }

            current->set_tag(to_tcb->get_tag());
            current->set_partner(to_tcb);

            PROFILE_STOP(sys_ipc_e);
            scheduler->update_active_state(current, thread_state_t::running);
            scheduler->activate_sched(to_tcb, thread_state_t::running,
                                      current, TCB_SYSDATA_IPC(current)->ipc_return_continuation,
                                      scheduler_t::sched_default);
        }
    }
    else        /* to_tid == NILTHREAD */
    {
        recv_blocks = current->get_tag().recv_blocks();
        current->set_tag(msgtag(0));
    }

    /* --- send finished ------------------------------------------------ */
receive_phase:
    okl4_atomic_barrier_smp();
    if (EXPECT_FALSE(from_tid.is_nilthread())) {
        /* this case is entered on:
         *   - send-only case
         *   - both descriptors set to nil id
         * in the SMP case to_tcb is always NULL! */

        if (EXPECT_FALSE(to_tcb == NULL)) {
            /* Both from_tid and to_tid are NULL. No work to perform. */
            ASSERT(DEBUG, current->get_state() == thread_state_t::running);
            current->set_tag(msg_tag_t::nil_tag());
            current->sent_from = NILTHREAD;
            PROFILE_STOP(sys_ipc_e);
            return_ipc();
        }

        /* IPC send without receive. Set tags up and return. */
        current->set_tag(msg_tag_t::nil_tag());
        current->set_partner(NULL);

        PROFILE_STOP(sys_ipc_e);
        scheduler->update_active_state(current, thread_state_t::running);
        scheduler->activate_sched(to_tcb, thread_state_t::running,
                                  current, TCB_SYSDATA_IPC(current)->ipc_return_continuation,
                                  scheduler_t::sched_default);
        NOTREACHED();
    }
    /* --- receive phase ------------------------------------------------ */
    else /* ! from_tid.is_nilthread() */
    {
        /* Is thread blocking on its self? */
        if (EXPECT_FALSE(from_tid.is_myself())) {
            current->set_partner(current);

            PROFILE_STOP(sys_ipc_e);
            scheduler->
                deactivate_sched(current, thread_state_t::waiting_forever,
                        current, check_async_ipc,
                        scheduler_t::sched_default);
            NOTREACHED();
        }

        from_tcb = get_current_clist()->lookup_ipc_cap_locked(from_tid);

        /*
         * Optimisation for Call(), i.e., when to == from.
         *
         * This allows us to avoid a whole number of checks, because we
         * know that from_tid is not waiting for us. This block of code
         * does not affect correctness, but gives a performance boost
         * for the Call() case.
         */
        if (to_tcb && (to_tcb == from_tcb) && recv_blocks) {
            current->set_partner(from_tcb);

            to_tcb->get_endpoint()->enqueue_recv(current);

            from_tcb->unlock_read();
            scheduler->deactivate_activate_sched(current, to_tcb,
                    thread_state_t::waiting_forever, thread_state_t::running,
                    current, check_async_ipc,
                    scheduler_t::sched_default);
            NOTREACHED();
        }

#ifdef CONFIG_MUNITS
        /* Eager state setting to ensure that sending thread and
         * receiving thread do not miss each other
         */
        if (recv_blocks) {
            current->set_partner(from_tid.is_anythread() ? (tcb_t*)~0UL : from_tcb);
            /* FIXME : Mothra Issue #2123 : Only the scheduler should be using
             * set_state(). */
            current->set_state(from_tid.is_waitnotify()?
                    thread_state_t::waiting_notify : 
                    thread_state_t::waiting_forever);
            /* ensure memory ordering */
            okl4_atomic_barrier_smp();
        }
#endif

        acceptor_t acceptor;
        acceptor.clear();

        /* VU: optimize for common case -- any, closed */
        if (from_tid.is_anythread())
        {
retry_get_head:
            okl4_atomic_barrier_smp();
            from_tcb = current->get_endpoint()->get_send_head();

            if (EXPECT_TRUE(from_tcb)) {
                if (EXPECT_FALSE(!from_tcb->try_lock_read())) {
                    okl4_atomic_barrier_smp();
                    goto retry_get_head;
                }
            }

            /* only accept notify bits if waiting from anythread */
            acceptor = current->get_acceptor();
        }
        else if (from_tid.is_waitnotify())
        {
            /* Wait on asynch notify only */

            /* check if we have any pending notify bits */
            if (EXPECT_TRUE( current->get_notify_bits() & current->get_notify_mask() ))
            {
                TRACE_NOTIFY("notify recieve (curr=%t)\n", current);
                setup_notify_return(current);
                current->sent_from = NILTHREAD;

                PROFILE_STOP(sys_ipc_e);
                enqueue_tcb_and_return(current, to_tcb,
                        TCB_SYSDATA_IPC(current)->ipc_return_continuation);
                NOTREACHED();
            }

            TRACE_NOTIFY("notify blocking receive (curr=%t)\n", current);

            acceptor = current->get_acceptor();

            /* current thread not accepting notifications? */
            /*
             * Should this check be before checking the bits? should 
             * wait_notify ever work if there is no acceptor?
             * Leaving for the moment as acceptors are soon to be deprecated 
             */
            if ( !acceptor.accept_notify() )
            {
                return_ipc_error_recv(current, to_tcb, NULL, ERR_IPC_NOT_ACCEPTED);
                NOTREACHED();
            }

            /* no pending bits and non-blocking */
            if (EXPECT_FALSE( !recv_blocks ))
            {
                /* prepare the IPC error */
                return_ipc_error_recv(current, to_tcb, NULL, ERR_IPC_NOPARTNER);
                NOTREACHED();
            }

            current->set_tag(msg_tag_t::notify_tag());

            /* FIXME: is it ok to use check_async_ipc here? Matthew Warton */
            /* Nothing more we can do except wait. Perform a schedule. */
            if (EXPECT_TRUE(to_tcb != NULL)) {
                scheduler->
                    deactivate_activate_sched(current, to_tcb,
                            thread_state_t::waiting_notify,
                            thread_state_t::running,
                            current, check_async_ipc,
                            scheduler_t::sched_default);
            } else {
                scheduler->
                    deactivate_sched(current, thread_state_t::waiting_notify,
                            current, check_async_ipc,
                            scheduler_t::sched_default);
            }
        }
        else {
            /* closed wait */
            ASSERT(DEBUG, from_tid.is_global());

            TRACE_IPC("closed wait from %t, current=%t, current_space=%p\n",
                      from_tcb, current, current->get_space());

            if (EXPECT_FALSE( from_tcb == NULL ))
            {
                /* wrong receiver id */
                TRACE_IPC("invalid from tid, %t\n",
                          from_tid.get_raw() );
                return_ipc_error_recv(current, to_tcb, NULL, ERR_IPC_NON_EXISTING);
                NOTREACHED();
            }
        }

        TRACE_IPC("receive phase curr=%t, from=%t\n", current, from_tcb);

        /*
         * no partner || partner is not polling ||
         * partner doesn't poll on me
         */
        if( EXPECT_TRUE ( (from_tcb == NULL) ||
                          (!from_tcb->get_state().is_polling()) ||
                          (from_tcb->get_partner() != current)))
        {
            /* If no partner is ready to send, check if we have any pending notify bits */
            if ( acceptor.accept_notify() && (current->get_notify_bits() & current->get_notify_mask()) )
            {
                TRACE_NOTIFY("notify recieve (curr=%t)\n", current);
                setup_notify_return(current);
                current->sent_from = NILTHREAD;
                ASSERT(DEBUG, current->get_state().is_runnable());
                scheduler->update_active_state(current,
                                               thread_state_t::running);
                if (from_tcb) { from_tcb->unlock_read(); }
                PROFILE_STOP(sys_ipc_e);
                enqueue_tcb_and_return(current, to_tcb,
                        TCB_SYSDATA_IPC(current)->ipc_return_continuation);
                NOTREACHED();
            }

            TRACE_IPC("blocking receive (curr=%t, from=%t)\n", current, from_tcb);

            /* Should we block waiting for the thread to be ready? */
            if (EXPECT_FALSE(!recv_blocks))
            {
                /* Don't block waiting. Instead, just return an error. */
                return_ipc_error_recv(current, to_tcb, from_tcb, ERR_IPC_NOPARTNER);
                NOTREACHED();
            }

            /* Update schedule inheritance dependancy on closed waits. */
            if (from_tcb != NULL) {
                from_tcb->get_endpoint()->enqueue_recv(current);
            }

#ifndef CONFIG_MUNITS
            current->set_partner(from_tid.is_anythread() ? (tcb_t*)~0UL : from_tcb);
#endif
            if (from_tcb) { from_tcb->unlock_read(); }
            /* Find somebody else to schedule. */
            if (EXPECT_TRUE(to_tcb != NULL))
            {
                scheduler->
                    deactivate_activate_sched(current, to_tcb,
                                              thread_state_t::waiting_forever,
                                              thread_state_t::running,
                                              current, check_async_ipc,
                                              scheduler_t::sched_default);
                NOTREACHED();
            }
            else
            {
                PROFILE_STOP(sys_ipc_e);
                scheduler->
                    deactivate_sched(current, thread_state_t::waiting_forever,
                                     current, check_async_ipc,
                                     scheduler_t::sched_default);
                NOTREACHED();
            }
        }
        else
        {
            if (!from_tcb->grab()) {
                from_tcb->unlock_read();
                goto receive_phase;
            }
            from_tcb->unlock_read();

            TRACE_IPC("perform receive from %t\n", from_tcb);
            //enter_kdebug("do receive");

            // both threads on the same CPU?
            {
                current->set_partner(from_tid.is_anythread() ? (tcb_t*)~0UL : from_tcb);
                current->get_endpoint()->dequeue_send(from_tcb);

                /* If we just sent an IPC, enqueue the receiver. */
                if (to_tcb != NULL) {
                    scheduler->activate(to_tcb, thread_state_t::running);
                }

                /* We need to perform a conext switch here, and not a schedule.
                 * This is because 'from_tcb' is responsible for finishing the
                 * copy, and we (a potentially high priority thread) can not be
                 * scheduled until that is done. 'from_tcb' will perform a
                 * schedule when the copy is done, ensuring that the scheduler
                 * is still in control. */
                PROFILE_STOP(sys_ipc_e);
                scheduler->context_switch(current, from_tcb, thread_state_t::waiting_forever,
                        thread_state_t::running,
                        TCB_SYSDATA_IPC(current)->ipc_return_continuation);
                NOTREACHED();
            }
        }
        NOTREACHED();
    }
    NOTREACHED();
}
