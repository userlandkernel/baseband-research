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
 * Description:   ARM specific thread handling functions
 */

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <cpu/syscon.h>

#if defined(CONFIG_ARM_VFP)

#include <cpu/vfp.h>
#include <arch/resources.h>
#include <kdb/tracepoints.h>

DECLARE_KMEM_GROUP (kmem_vfp);
DECLARE_TRACEPOINT(EXCEPTION_IPC_VFP);

#define PC(x)           (x & (~1UL))

/*
 * Define CONFIG_FPU_PRELOAD to enable preloading and enabling
 * the VFP before context switching to a thread previously
 * using it. Only useful for a small class of systems
 */
//#define CONFIG_FPU_PRELOAD  1

static tcb_t * vfp_owner UNIT("cpulocal");

bool
thread_resources_t::arm_vfp_exception(tcb_t * tcb)
{
    /* Thread is attempting to use the FPU */
    word_t fpexc;

    vfp_getsr(FPEXC, fpexc);

    /* If VFP was disabled and thread is allowed to use it */
    if (EXPECT_TRUE(vfp_state && !(fpexc & FPEXC_EN))) {
        arm_vfp_t::enable();

        /* Is fault due to another thread owning VFP? */
        if (EXPECT_TRUE(vfp_owner != tcb)) {
            if (EXPECT_TRUE(vfp_owner != NULL)) {
                arm_vfp_t::save_state(vfp_owner->resources.vfp_state);
#if !defined(CONFIG_FPU_PRELOAD)
                vfp_owner->resource_bits -= VFP;
#endif
            }

            vfp_owner = tcb;
        }

        arm_vfp_t::load_state(vfp_state);

        tcb->resource_bits += VFP;

        tcb->return_from_user_interruption();
    }

    /*
     * Fault not due to no FPU access perms, return such that
     * a FPU exception message can be generated
     */
    return (fpexc & FPEXC_EN) && (fpexc & FPEXC_EXCEPT);
}

void
thread_resources_t::arm_vfp_save(tcb_t * tcb)
{
    arm_vfp_t::disable();
#if !defined(CONFIG_FPU_PRELOAD)
    vfp_owner->resource_bits -= VFP;
#endif
}

void
thread_resources_t::arm_vfp_load(tcb_t * tcb)
{
    arm_vfp_t::enable();

    if (vfp_owner != tcb) {
        if (EXPECT_TRUE(vfp_owner != NULL)) {
            arm_vfp_t::save_state(vfp_owner->resources.vfp_state);
#if !defined(CONFIG_FPU_PRELOAD)
            vfp_owner->resource_bits -= VFP;
#endif
        }

        vfp_owner = tcb;

        arm_vfp_t::load_state(vfp_state);
    }
}

void
thread_resources_t::arm_vfp_free(tcb_t * tcb)
{
    tcb->resource_bits -= VFP;

    kmem.free(kmem_vfp, vfp_state, arm_vfp_t::get_state_size());
    vfp_state = NULL;

    if (vfp_owner == tcb) {
        vfp_owner = NULL;
        arm_vfp_t::disable();
    }
}

bool
thread_resources_t::arm_vfp_control(tcb_t * tcb, bool access)
{
    /*printf("VFP %s for %p\n", access ? "enable" : "disable", tcb);*/

    if (access == true) {
        if (vfp_state == NULL) {
            vfp_state = kmem.alloc(kmem_vfp, arm_vfp_t::get_state_size(), true);
            if (!vfp_state) {
                get_current_tcb()->set_error_code(ENO_MEM);
                return false;
            }
            arm_vfp_t::init();
        }
    } else {
        if (vfp_state) {
            arm_vfp_free(tcb);
        }
    }
    return true;
}

CONTINUATION_FUNCTION(finish_send_vfp_exception_ipc);
NORETURN void
send_vfp_exception_ipc(arm_irq_context_t * context, continuation_t continuation)
{
    tcb_t * current = get_current_tcb();

    if( current->get_exception_handler() == NULL ) {
        ACTIVATE_CONTINUATION(continuation);
    }

    word_t fpexc, fpscr, fpinst, fpinst2;

    vfp_getsr(FPEXC, fpexc);
    vfp_getsr(FPINST, fpinst);
    vfp_getsr(FPINST2, fpinst2);
    vfp_getsr(FPSCR, fpscr);

    /* XXX we should check if the kernel faulted */

    TRACEPOINT(EXCEPTION_IPC_VFP,
               printf("EXCEPTION_IPC_VFP: (%p) ESC = %p INST = %p, IP = %p\n",
                      current, fpexc, fpinst, PC(context->pc)));

    // Save message registers on the stack
    msg_tag_t tag;

    /* We are just about to send an IPC. */
    current->sys_data.set_action(tcb_syscall_data_t::action_ipc);

    // Save message registers.
    current->save_state();

    /* don't allow receiving async */
    current->set_notify_mask(0);

    // Create the message tag.
    tag.set(EXCEPT_IPC_VFP_MR_NUM, EXCEPTION_TAG, true, true);
    current->set_tag(tag);

    // Create the message.
    current->set_mr(EXCEPT_IPC_GEN_MR_IP, PC(context->pc));
    current->set_mr(EXCEPT_IPC_GEN_MR_SP, context->sp);
    current->set_mr(EXCEPT_IPC_GEN_MR_FLAGS, context->cpsr);
    current->set_mr(EXCEPT_IPC_GEN_MR_EXCEPTNO, 8);
    current->set_mr(EXCEPT_IPC_GEN_MR_ERRORCODE, fpexc);
    current->set_mr(EXCEPT_IPC_VFP_MR_FPINST, fpinst);
    current->set_mr(EXCEPT_IPC_VFP_MR_FPSCR, fpscr);
    current->set_mr(EXCEPT_IPC_VFP_MR_FPINST2, fpinst2);

    current->arch.misc.exception.exception_ipc_continuation = continuation;
    // Deliver the exception IPC.
    current->do_ipc(current->get_exception_handler()->get_global_id(),
                    current->get_exception_handler()->get_global_id(),
                    finish_send_vfp_exception_ipc);
}


CONTINUATION_FUNCTION(finish_send_vfp_exception_ipc)
{
    msg_tag_t tag;
    tcb_t * current = get_current_tcb();

    tag.raw = current->get_mr(0);

    // Alter the user context if necessary.
    if (EXPECT_TRUE(!tag.is_error())) {
        /* Force VFP registers to current thread */
        current->resources.arm_vfp_load(current);
        current->resource_bits += VFP;

        current->set_user_ip((addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_IP));
        current->set_user_sp((addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_SP));
        current->set_user_flags(current->get_mr(EXCEPT_IPC_GEN_MR_FLAGS));

        vfp_putsr(FPEXC, current->get_mr(EXCEPT_IPC_GEN_MR_ERRORCODE));
        vfp_putsr(FPSCR, current->get_mr(EXCEPT_IPC_VFP_MR_FPINST2));
    }
    else {
        TRACEF( "Unable to deliver user exception: IPC error.\n" );
    }

    // Clean-up.
    current->restore_state();

    ACTIVATE_CONTINUATION(current->
                          arch.misc.exception.exception_ipc_continuation);
}

#endif
