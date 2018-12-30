/*
 * Copyright (c) 2002, University of New South Wales
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
 * Copyright (c) 2004-2006, National ICT Australia (NICTA)
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

#include <arch/context.h>
#include <tcb.h>

#if defined(L4_32BIT)
#define     REG     "8lx"
#else
#define     REG     "16lx"
#endif

extern tcb_t * kdb_get_tcb();

void SECTION(SEC_KDEBUG) mips64_dump_frame(mips_irq_context_t *context)
{
    printf ("== Stack frame: %p == \n", context);
    printf ("== STATUS: %8x == CAUSE: %"REG" == EPC: %"REG"\n", context->status, context->cause, context->epc);
    printf ("at = %"REG", v0 = %"REG", v1 = %"REG", sp = %"REG"\n", context->at, context->v0, context->v1, context->sp);
    printf ("a0 = %"REG", a1 = %"REG", a2 = %"REG", a3 = %"REG"\n", context->a0, context->a1, context->a2, context->a3);
    printf ("t0 = %"REG", t1 = %"REG", t2 = %"REG", t3 = %"REG"\n", context->t0, context->t1, context->t2, context->t3);
    printf ("t4 = %"REG", t5 = %"REG", t6 = %"REG", t7 = %"REG"\n", context->t4, context->t5, context->t6, context->t7);
    printf ("s0 = %"REG", s1 = %"REG", s2 = %"REG", s3 = %"REG"\n", context->s0, context->s1, context->s2, context->s3);
    printf ("s4 = %"REG", s5 = %"REG", s6 = %"REG", s7 = %"REG"\n", context->s4, context->s5, context->s6, context->s7);
    printf ("t8 = %"REG", t9 = %"REG", s8 = %"REG", gp = %"REG"\n", context->t8, context->t9, context->s8, context->gp);
    printf ("ra = %"REG", hi = %"REG", lo = %"REG"\n", context->ra, context->hi, context->lo);
}

/* XXX Fix fpu for mips32 */
void SECTION(SEC_KDEBUG) dump_fprs( tcb_t *tcb )
{
    struct fpu_state_t * state = (struct fpu_state_t*)&tcb->resources;

    if (state) {
        for (int i = 0; i < 32; i++)
            printf ("f%d\t= %16lx\n", i, state->fpu_gprs[i]);
        printf("FPCSR\t= %16lx\n", state->fpu_fpcsr);
    } else {
        printf("Thread does not have FPU resources\n");
    }
}


/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
             "show current user exception frame");

CMD (cmd_dump_current_frame, cg)
{
    mips_irq_context_t *frame = (mips_irq_context_t *)(kdb.kdb_param);

    mips64_dump_frame(frame);

//    printf("tcb = %p\n", space_t::get_tcb((addr_t)kdb.kdb_param));
//    printf("stack = %p\n", space_t::get_tcb((addr_t)kdb.kdb_param)->stack);

    return CMD_NOQUIT;
}


/**
 * cmd_dump_frame: show exception frame
 */
DECLARE_CMD (cmd_dump_frame, root, 'F', "dumpframe",
             "show exception frame");


mips_irq_context_t SECTION(SEC_KDEBUG) * get_frame()
{
    space_t * space = get_current_space();
    if (!space) space = get_kernel_space();
    word_t val = get_hex("tcb/tid/addr", (word_t)get_tcb(kdb.kdb_param), "current");
    mips_irq_context_t * frame;

    if (val == ABORT_MAGIC)
        return NULL;

    if (!get_tcb((addr_t)val) &&
        ((val & (~0xfffUL)) != (word_t)get_idle_tcb()))
    {
        tcb_t *tcb;
        threadid_t tid;
        tid.set_raw(val);
        tcb = get_root_clist()->lookup_ipc_cap_tcb(tid);
        if (tcb)
        {
            frame = &(tcb->arch.context);
        }
    }
    else
    {
        frame = (mips_irq_context_t *)val;
        if (frame == (mips_irq_context_t *)get_tcb((addr_t)val))
        {
            frame = &(((tcb_t *)frame)->arch.context);
        }
    }
    return frame;

}

CMD (cmd_dump_frame, cg)
{
    mips_irq_context_t *frame = get_frame();

    if (frame)
        mips64_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_find_frame: search for an exception frame
 */
DECLARE_CMD (cmd_find_frame, root, 's', "findframe",
             "search for an exception frame");

CMD (cmd_find_frame, cg)
{
    word_t i;
    mips_irq_context_t *frame;
    clist_t *clist = get_root_clist();
    for (i = 0; i < (1 << VALID_THREADNO_BITS); i++)
    {
        tcb_t *tcb = clist->lookup_ipc_cap_tcb(threadid_t::threadid(i, 0));
        if (tcb != 0)
        {
            frame = &tcb->arch.context;
            if ((frame->status & ~0x3ff1fULL) == 0x40000e0ULL)
            {
                printf("== TCB: ox%lx, tid: 0x%lx =\n", tcb, TID(tcb->get_global_id()));
                mips64_dump_frame(frame);
            }
        }
    }

    return CMD_NOQUIT;
}

/**
 * cmd_dump_fprs: dump floating point registers
 */
DECLARE_CMD (cmd_dump_fprs, root, 'f', "fpr",
             "show floating point registers");


CMD (cmd_dump_fprs, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb) {
        tcb->resources.purge( tcb );
        dump_fprs( tcb );
    }

    return CMD_NOQUIT;
}

