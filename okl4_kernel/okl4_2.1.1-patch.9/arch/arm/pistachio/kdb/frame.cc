/*
 * Copyright (c) 2003, National ICT Australia (NICTA)
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
 * Description:   Exception frame dumping
 */
#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include <arch/thread.h>
#include <tcb.h>


extern tcb_t * kdb_get_tcb();

void SECTION(SEC_KDEBUG) arm_dump_frame(arm_irq_context_t *context)
{
    printf ("== Stack frame: %p == \n", context);
    printf ("cpsr = %8lx, pc = %8lx, sp  = %8lx, lr  = %8lx\n", context->cpsr, context->pc & ~(1UL), context->sp, context->lr);
    printf ("r0  = %8lx, r1  = %8lx, r2  = %8lx, r3  = %8lx, r4  = %8lx\n", context->r0, context->r1, context->r2, context->r3, context->r4);
    printf ("r5  = %8lx, r6  = %8lx, r7  = %8lx, r8  = %8lx, r9  = %8lx\n", context->r5, context->r6, context->r7, context->r8, context->r9);
    printf ("r10 = %8lx, r11 = %8lx, r12 = %8lx\n", context->r10, context->r11, context->r12);
}

/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
             "show current user exception frame");

CMD (cmd_dump_current_frame, cg)
{
    arm_irq_context_t *frame = (arm_irq_context_t *)(kdb.kdb_param);

    arm_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_dump_frame: show exception frame
 */
DECLARE_CMD (cmd_dump_frame, root, 'F', "dumpframe",
             "show exception frame");


arm_irq_context_t SECTION(SEC_KDEBUG) * get_frame()
{
    space_t * space = get_current_space();
    if (!space) space = get_kernel_space();
    word_t val = get_hex("tcb/tid/addr", (word_t)get_current_tcb(), "current");
    arm_irq_context_t * frame = NULL;
    tcb_t *tcb;

    if (val == ABORT_MAGIC)
        return NULL;

    tcb = get_tcb((addr_t)val);

    if (!tcb && ((val & (~0xfffUL)) != (word_t)get_idle_tcb()))
    {
        threadid_t tid;
        tid.set_raw(val);
        if (tcb = get_root_clist()->lookup_ipc_cap_locked(tid))
        {
            frame = &tcb->arch.context;
            tcb->unlock_read();
        }
    }
    else
    {
        frame = (arm_irq_context_t *)val;
        if (frame == (arm_irq_context_t *)get_tcb((addr_t)val))
        {
            frame = &((tcb_t*)frame)->arch.context;
        }
    }

    return frame;

}

CMD (cmd_dump_frame, cg)
{
    arm_irq_context_t *frame = get_frame();

    if (frame)
        arm_dump_frame(frame);

    return CMD_NOQUIT;
}
