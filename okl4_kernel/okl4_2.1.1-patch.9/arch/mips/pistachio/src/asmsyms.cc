/*
 * Copyright (c) 2002-2003, University of New South Wales
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
 * Description:   Various asm definitions for mips
 */
                
#include <l4.h>
#include <mkasmsym.h>

#include <tcb.h>
#include <arch/space.h>
#include <arch/config.h>
#include <arch/context.h>

MKASMSYM_START  /* Do not remove */

#define cpp_offsetof(type, field) ((unsigned long) &(((type *) 4)->field) - 4)

MKASMSYM (MIPS_SWITCH_STACK_SIZE, sizeof(mips_switch_stack_t));

MKASMSYM (UTCB_MR_OFFSET, cpp_offsetof(utcb_t, mr));
MKASMSYM (TSTATE_RUNNING, (word_t) thread_state_t::running);
MKASMSYM (TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever);
MKASMSYM (TSTATE_POLLING, (word_t) thread_state_t::polling);

MKASMSYM (SPACE_ASID_OFFSET, cpp_offsetof(space_t, asid));
MKASMSYM (OFS_TCB_ARCH_CONTEXT, cpp_offsetof(tcb_t, arch.context));
MKASMSYM (OFS_CAP_TCB, cpp_offsetof(cap_t, raw));
MKASMSYM (CLIST_CAPENTRY_SHIFT, (word_t) 2);

MKASMSYM (PT_SIZE,      sizeof(mips_irq_context_t));
MKASMSYM (PT_EPC        , (word_t) &(((mips_irq_context_t *) 0)->epc));
MKASMSYM (PT_SP         , (word_t) &(((mips_irq_context_t *) 0)->sp));
MKASMSYM (PT_STATUS     , (word_t) &(((mips_irq_context_t *) 0)->status));
MKASMSYM (PT_RA         , (word_t) &(((mips_irq_context_t *) 0)->ra));
MKASMSYM (PT_V0         , (word_t) &(((mips_irq_context_t *) 0)->v0));
MKASMSYM (PT_V1         , (word_t) &(((mips_irq_context_t *) 0)->v1));
MKASMSYM (PT_A1         , (word_t) &(((mips_irq_context_t *) 0)->a1));
MKASMSYM (PT_CAUSE      , (word_t) &(((mips_irq_context_t *) 0)->cause));
MKASMSYM (PT_A2         , (word_t) &(((mips_irq_context_t *) 0)->a2));
MKASMSYM (PT_A3         , (word_t) &(((mips_irq_context_t *) 0)->a3));
MKASMSYM (PT_T9         , (word_t) &(((mips_irq_context_t *) 0)->t9));
MKASMSYM (PT_A0         , (word_t) &(((mips_irq_context_t *) 0)->a0));
MKASMSYM (PT_GP         , (word_t) &(((mips_irq_context_t *) 0)->gp));
MKASMSYM (PT_AT         , (word_t) &(((mips_irq_context_t *) 0)->at));
MKASMSYM (PT_T0         , (word_t) &(((mips_irq_context_t *) 0)->t0));
MKASMSYM (PT_T1         , (word_t) &(((mips_irq_context_t *) 0)->t1));
MKASMSYM (PT_HI         , (word_t) &(((mips_irq_context_t *) 0)->hi));
MKASMSYM (PT_T2         , (word_t) &(((mips_irq_context_t *) 0)->t2));
MKASMSYM (PT_T3         , (word_t) &(((mips_irq_context_t *) 0)->t3));
MKASMSYM (PT_LO         , (word_t) &(((mips_irq_context_t *) 0)->lo));
MKASMSYM (PT_T4         , (word_t) &(((mips_irq_context_t *) 0)->t4));
MKASMSYM (PT_T5         , (word_t) &(((mips_irq_context_t *) 0)->t5));
MKASMSYM (PT_T6         , (word_t) &(((mips_irq_context_t *) 0)->t6));
MKASMSYM (PT_T7         , (word_t) &(((mips_irq_context_t *) 0)->t7));
MKASMSYM (PT_T8         , (word_t) &(((mips_irq_context_t *) 0)->t8));
MKASMSYM (PT_S0         , (word_t) &(((mips_irq_context_t *) 0)->s0));
MKASMSYM (PT_S1         , (word_t) &(((mips_irq_context_t *) 0)->s1));
MKASMSYM (PT_S2         , (word_t) &(((mips_irq_context_t *) 0)->s2));
MKASMSYM (PT_S3         , (word_t) &(((mips_irq_context_t *) 0)->s3));
MKASMSYM (PT_S4         , (word_t) &(((mips_irq_context_t *) 0)->s4));
MKASMSYM (PT_S5         , (word_t) &(((mips_irq_context_t *) 0)->s5));
MKASMSYM (PT_S6         , (word_t) &(((mips_irq_context_t *) 0)->s6));
MKASMSYM (PT_S7         , (word_t) &(((mips_irq_context_t *) 0)->s7));
MKASMSYM (PT_S8         , (word_t) &(((mips_irq_context_t *) 0)->s8));
MKASMSYM (PT_BADVADDR   , (word_t) &(((mips_irq_context_t *) 0)->badvaddr));
MKASMSYM (PT_X1         , (word_t) &(((mips_irq_context_t *) 0)->x1));

MKASMSYM_END    /* Do not remove */
