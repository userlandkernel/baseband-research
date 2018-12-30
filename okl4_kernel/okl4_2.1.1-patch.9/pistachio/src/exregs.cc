/*
 * Copyright (c) 2003-2004, Karlsruhe University
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
 * Description:   Iplementation of ExchangeRegisters()
 */
#include <l4.h>
#include <arch/syscalls.h>
#include <smp.h>
#include <schedule.h>

#include <arch/tcb.h>
#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT (SYSCALL_EXCHANGE_REGISTERS);

void handle_ipc_error (void);
void thread_return (void);

CONTINUATION_FUNCTION(finish_exchange_registers);

CONTINUATION_FUNCTION(finish_perform_exreg);

/**
 * Do the actual ExhangeRegisters() syscall.  Separated into a
 * separate function so that it can be invoked on a remote CPU.
 *
 * This is a control function, thus it returns through the
 * continuation argument given.
 *
 * @param source The source TCB - the current tcb on SMT and uniprocessor
 * @param continuation The continuation to activate upon completion
 *
 * The following exreg parameters are stored in the source TCB's
 * misc.exregs structure
 *
 * @param tcb           destination tcb
 * @param control       control parameter (in/out)
 * @param usp           stack pointer (in/out)
 * @param uip           instrunction pointer (in/out)
 * @param uflags        flags (in/out)
 * @param pager         pager (in/out)
 * @param uhandler      user defined handle (in/out)
 */
static void perform_exregs (tcb_t *tcb, word_t *control, word_t *usp,
                            word_t *uip, word_t *uflags, word_t *uhandle,
                            tcb_t *source)
{
    word_t ctrl = *control;

    tcb_t * current = get_current_tcb();

    // Load return values before they are clobbered.
    if (ctrl & EXREGS_CONTROL_DELIVER)
    {
        TCB_SYSDATA_EXREGS(source)->sp          = tcb->get_user_sp();
        TCB_SYSDATA_EXREGS(source)->ip          = tcb->get_user_ip();
        TCB_SYSDATA_EXREGS(source)->flags       = tcb->get_user_flags();
        TCB_SYSDATA_EXREGS(source)->user_handle = tcb->get_user_handle();
        tcb_t *thrd_pager = tcb->get_pager();
        TCB_SYSDATA_EXREGS(source)->pager       = thrd_pager ? thrd_pager->get_global_id() : NILTHREAD;
    }
    TCB_SYSDATA_EXREGS(source)->new_control = 0;

    get_current_scheduler()->pause(tcb);
    tcb->unlock_read();

    if (ctrl & EXREGS_CONTROL_SP)
        tcb->set_user_sp ((addr_t)*usp);

    if (ctrl & EXREGS_CONTROL_IP)
        tcb->set_user_ip ((addr_t)*uip);

    if (ctrl & EXREGS_CONTROL_UHANDLE)
        tcb->set_user_handle (*uhandle);

    if (ctrl & EXREGS_CONTROL_FLAGS)
        tcb->set_user_flags (*uflags);

    if (ctrl & EXREGS_CONTROL_COPY_REGS)
    {
        threadid_t src_id = threadid_t::threadid(ctrl >> EXREGS_CONTROL_THREAD_SHIFT, 1);
        tcb_t *src = get_current_clist()->lookup_thread_cap_locked(src_id);

        tcb->copy_user_regs(src);
        src->unlock_read();
    }

    /*
     * Both EXREGS_CONTROL_TLS and EXREGS_CONTROL_MRS_TO_REGS can use
     * MRs to pass in input arguments, but this is checked earlier so no
     * check need to be done here.
     *
     * Note that set_tls() actually returns an error code but we cannot
     * really handle it.  The reason is ExRegs() is poorly structured,
     * and at this point the code flow assumes no error can happen.  However,
     * the error is basically x86 specific.
     */
    if (ctrl & EXREGS_CONTROL_TLS) {
        (void)tcb->set_tls(&current->get_utcb()->mr[0]);
    }

    /* XXX FIXME This is undocumented (and often unimplemented) */
    if (ctrl & EXREGS_CONTROL_MRS_TO_REGS)
    {
        source->copy_mrs_to_regs(tcb);
    }
    if (ctrl & EXREGS_CONTROL_REGS_TO_MRS)
    {
        source->copy_regs_to_mrs(tcb);
    }

    TCB_SYSDATA_EXREGS(current)->source = source;
    TCB_SYSDATA_EXREGS(source)->exreg_tcb = tcb;
    TCB_SYSDATA_EXREGS(source)->ctrl = ctrl;

    // Check if thread was IPCing
    if (tcb->get_state().is_sending() || tcb->get_state().is_receiving())
    {
        word_t mask;

        if (tcb->get_state().is_sending()) {
            mask = EXREGS_CONTROL_SEND;
        } else {
            mask = EXREGS_CONTROL_RECV;
        }

       TCB_SYSDATA_EXREGS(source)->new_control |= mask;

        if (ctrl & mask) {
            tcb->unwind(tcb->get_partner());

            tcb->notify(handle_ipc_error);

            get_current_scheduler()->activate(tcb, thread_state_t::running);
            /* Unwind might have added more threads to the ready queue. Perform
             * a full schedule. */
            get_current_scheduler()->schedule(current, finish_perform_exreg,
                                              scheduler_t::sched_default);
        }
    }

    /* Should we abort a mutex operation? */
    if (ctrl & (EXREGS_CONTROL_SEND | EXREGS_CONTROL_RECV)) {
        /* Is the thread in the middle of a muetx operation? */
        if (tcb->get_state().is_waiting_mutex()) {
            tcb->unwind(NULL);
            get_current_scheduler()->activate(tcb, thread_state_t::running);
            /* Unwind might have added more threads to the ready queue. Perform
             * a full schedule. */
            get_current_scheduler()->schedule(current, finish_perform_exreg,
                    scheduler_t::sched_default);
        }
    }

    get_current_scheduler()->unpause(tcb);
    ACTIVATE_CONTINUATION(finish_perform_exreg);
}

/*
 * Halt the given thread that is currently in a running state.
 *
 * Return if the thread was halted, unless the thread is the current TCB, in
 * which case perform a full schedule, waking up again at 'cont'.
 */
void halt_thread(tcb_t * tcb, continuation_t cont)
{
    ASSERT(ALWAYS, tcb->get_state().is_running());

    // Halt a running thread
    get_current_scheduler()->pause(tcb);
    if (tcb->get_state().is_runnable()) {
        get_current_scheduler()->deactivate(tcb, thread_state_t::halted);
    } else {
        get_current_scheduler()->update_inactive_state(tcb, thread_state_t::halted);
    }

    /* Perform a full schedule if required. */
    if (tcb == get_current_tcb()) {
        PROFILE_STOP(sys_exchg_regs);
        get_current_scheduler()->schedule(tcb, cont, scheduler_t::sched_default);
    } else {
        tcb->release();
    }
}

/*
 * Resume the given thread from its halted state.
 */
void resume_thread(tcb_t * tcb, continuation_t cont)
{
    ASSERT(ALWAYS, tcb->get_state().is_halted());

    get_current_scheduler()->activate(tcb, thread_state_t::running);
    PROFILE_STOP(sys_exchg_regs);
    get_current_scheduler()->schedule(get_current_tcb(), cont, scheduler_t::sched_default);
}

/*
 * Halt the current TCB.
 *
 * This is called after a thread that the user suspends is in the
 * middle of a system call. When the thread is just about to
 * enter back into userspace this function is called.
 */
static void halt_current_thread(continuation_t cont)
{
    halt_thread(get_current_tcb(), cont);
}

/**
 * finishes up the exreg functions - specifically implements Halting and resuming of threads
 *
 * This is a continuation function so parameters are stored in the TCB
 *
 * @param exreg_source_tcb The source of the exreg call
 * @param perfrom_exreg_continuation The continuation to activate upon completion
 *
 * The following parameters are in the source TCB's misc.exregs structure
 * @param ctrl Exreg control parameter (in)
 * @param new_control Exreg control parameter (out)
 */
CONTINUATION_FUNCTION(finish_perform_exreg)
{
    tcb_t * current = get_current_tcb();
    tcb_syscall_data_t::exregs_t *cur_info = TCB_SYSDATA_EXREGS(current);

    tcb_t * source = cur_info->source;
    tcb_syscall_data_t::exregs_t *src_info = TCB_SYSDATA_EXREGS(source);

    tcb_t * tcb = src_info->exreg_tcb;

    /* Update return control flags. */
    if (tcb->get_state().is_halted())
    {
        src_info->new_control |= EXREGS_CONTROL_HALT;
    }
    if (tcb->is_suspended()) {
        src_info->new_control |= EXREGS_CONTROL_HALT;
        src_info->new_control |= EXREGS_CONTROL_SUSPEND_FLAG;
    }

    /* Check if we should resume a halted/resume a thread. */
    if (src_info->ctrl & EXREGS_CONTROL_HALTFLAG) {
        if (src_info->ctrl & EXREGS_CONTROL_HALT) {
            /* Halt a running thread if necessary. */
            if (tcb->get_state().is_running()) {
                halt_thread(tcb, finish_exchange_registers);
            }
            /* Halting a thread with ongoing kernel operations is not
             * supported. Such functionlity is implemented instead in
             * Suspend/Resume functionality.*/
        } else {
            /* If thread is halted, resume it. */
            if (tcb->get_state().is_halted() && !tcb->is_suspended()) {
                resume_thread(tcb, finish_exchange_registers);
            }
        }
    }

    /*
     * Determine if we should suspend the given thread. Suspending is
     * independent of halting to allow the kernel to be backwards compatible
     * with previous halting behaviour.
     *
     * Suspending a thread prevents a thread from executing any more userspace
     * instructions, but still complete pending system calls.
     */
    if (src_info->ctrl & EXREGS_CONTROL_SUSPEND_FLAG) {
        if (src_info->ctrl & EXREGS_CONTROL_HALT) {
            /* Suspend a running thread if necessary. */
            if (!tcb->is_suspended()) {
                tcb->set_suspended(true);
                tcb->set_post_syscall_callback(halt_current_thread);

                /* If the thread is running now, halt it. Otherwise the
                 * thread will be suspended next time just before it
                 * returns to userspace. */
                if (tcb->get_state().is_running()) {
                    halt_thread(tcb, finish_exchange_registers);
                }
            }
        } else {
            /* If thread is suspended, resume it. */
            if (tcb->is_suspended()) {
                tcb->set_suspended(false);
                tcb->set_post_syscall_callback(NULL);

                /* If the thread has been halted because of the suspend,
                 * resume execution. */
                if (tcb->get_state().is_halted()) {
                    resume_thread(tcb, finish_exchange_registers);
                }
            }
        }
    }

    // FIXME: SMP which continuation do we use here?
    PROFILE_STOP(sys_exchg_regs);
    ACTIVATE_CONTINUATION(finish_exchange_registers);
}

NORETURN SYS_EXCHANGE_REGISTERS (threadid_t dest_tid, word_t control,
                        word_t usp, word_t uip, word_t uflags,
                        word_t uhandle)
{
    PROFILE_START(sys_exchg_regs);
    tcb_t * current = get_current_tcb();
    continuation_t continuation = ASM_CONTINUATION;

    TRACEPOINT_TB (SYSCALL_EXCHANGE_REGISTERS,
                   printf ("SYS_EXCHANGE_REGISTERS: dest=%t, "
                           "control=0x%x [%c%c%c%c%c%c%c%c%c%c%c], "
                           "usp=%p, uip=%p, uflags=%p, "
                           "uhandle=%x\n",
                           TID (dest_tid),
                           control,
                           control & EXREGS_CONTROL_SUSPEND_FLAG ? 'Z' : '~',
                           control & EXREGS_CONTROL_COPY_REGS   ? 'r' : '~',
                           control & EXREGS_CONTROL_DELIVER ? 'd' : '~',
                           control & EXREGS_CONTROL_HALTFLAG? 'h' : '~',
                           //control & EXREGS_CONTROL_PAGER   ? 'p' : '~',
                           control & EXREGS_CONTROL_UHANDLE ? 'u' : '~',
                           control & EXREGS_CONTROL_FLAGS   ? 'f' : '~',
                           control & EXREGS_CONTROL_IP      ? 'i' : '~',
                           control & EXREGS_CONTROL_SP      ? 's' : '~',
                           control & EXREGS_CONTROL_SEND    ? 'S' : '~',
                           control & EXREGS_CONTROL_RECV    ? 'R' : '~',
                           control & EXREGS_CONTROL_HALT    ? 'H' : '~',
                           usp, uip, uflags, uhandle),
                   "sys_exchange_registers (dest=%t, control=%x)",
                   TID(dest_tid), control);

    space_t * space = get_current_space();
    tcb_t *tcb_locked, *pager_tcb;

    if (dest_tid.is_myself()) {
        tcb_locked = acquire_read_lock_tcb(current);
    } else {
        tcb_locked = get_current_clist()->lookup_thread_cap_locked(dest_tid);
    }

    current->sys_data.set_action(tcb_syscall_data_t::action_exregs);

    /* check that thread exists */
    if (EXPECT_FALSE( tcb_locked == NULL ))
    {
        current->set_error_code (EINVALID_THREAD);
        goto error_out;
    }

    pager_tcb = tcb_locked->get_pager();

    /*
     * Only allow exregs on:
     *  - in the same address space as thread's pager
     *    or rootask space
     *    or base space if in extension space
     */
    if (EXPECT_FALSE( (tcb_locked->get_space() != space) && (space != roottask_space) &&
                      ( (pager_tcb != NULL) && (space != pager_tcb->get_space())) &&
                      tcb_locked->get_space() != current->get_base_space()))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out_locked;
    }

    if (control & EXREGS_CONTROL_COPY_REGS)
    {
        threadid_t src_id = threadid_t::threadid(control >> EXREGS_CONTROL_THREAD_SHIFT, 1);
        tcb_t *src_locked = get_current_clist()->lookup_thread_cap_locked(src_id);

        if (EXPECT_FALSE(src_locked == NULL)) {
            current->set_error_code (EINVALID_THREAD);
            goto error_out_locked;
        }
        tcb_t *src_pager = src_locked->get_pager();
        src_locked->unlock_read();

        if (EXPECT_FALSE(src_pager == NULL))
        {
            current->set_error_code (EINVALID_THREAD);
            goto error_out_locked;
        }

        if (EXPECT_FALSE(space != src_pager->get_space()))
        {
            current->set_error_code (ENO_PRIVILEGE);
            goto error_out_locked;
        }
    }

    /*
     * Check for exclusivity: both use MRs to pass in inputs,
     * this can't be possible.
     */
    if ((control & (EXREGS_CONTROL_TLS|EXREGS_CONTROL_MRS_TO_REGS)) ==
        (EXREGS_CONTROL_TLS|EXREGS_CONTROL_MRS_TO_REGS)) {
        current->set_error_code(EINVALID_PARAM);
        goto error_out_locked;
    }

    /* Save the return address for exregs */
    TCB_SYSDATA_EXREGS(current)->exreg_continuation = continuation;

    {
        // Destination thread on same CPU.  Perform operation immediately.
        perform_exregs (tcb_locked, &control, &usp, &uip, &uflags, &uhandle,
                        current);
    }
    while(1);

error_out_locked:
    tcb_locked->unlock_read();
error_out:
    PROFILE_STOP(sys_exchg_regs);
    return_exchange_registers (threadid_t::nilthread(), 0, 0, 0, 0,
            threadid_t::nilthread(), 0, continuation);
}

/**
 * do the exregs return call with the appropriate parameters
 *
 * @param exreg_tcb The tcb that Exregs was performed on
 * @param new_control The return control parameter
 * @param sp The old sp of the thread
 * @param ip The old ip of the thread
 * @param flags The old flags of the thread
 * @param pager The old pager of the thread
 * @param user_handle The old user handle of the thread
 */
CONTINUATION_FUNCTION(finish_exchange_registers)
{
    tcb_t * current = get_current_tcb();
    tcb_syscall_data_t::exregs_t *cur_info = TCB_SYSDATA_EXREGS(current);

    if (cur_info->ctrl & EXREGS_CONTROL_DELIVER)
    {
        return_exchange_registers(cur_info->exreg_tcb->get_global_id (),
                                  cur_info->new_control,
                                  (word_t)cur_info->sp,
                                  (word_t)cur_info->ip,
                                  cur_info->flags,
                                  cur_info->pager,
                                  cur_info->user_handle,
                                  cur_info->exreg_continuation);
    }
    else
    {
        return_exchange_registers(cur_info->exreg_tcb->get_global_id (),
                                  cur_info->new_control,
                                  0,
                                  0,
                                  0,
                                  NILTHREAD,
                                  0,
                                  cur_info->exreg_continuation);
    }
}

