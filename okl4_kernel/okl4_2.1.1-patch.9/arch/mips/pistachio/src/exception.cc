/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Description:   Mips exception handling
 */                

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <kernelinterface.h>
#include <syscalls.h>
#include <arch/context.h>
#include <arch/syscalls.h>
#include <arch/exception.h>
#include <linear_ptab.h>

#include <kdb/names.h>
#include <kdb/tracepoints.h>
#ifdef CONFIG_DEBUG
#include <kdb/console.h>
#endif

DECLARE_TRACEPOINT(EXCEPTION_IPC_SYSCALL);
DECLARE_TRACEPOINT(EXCEPTION_IPC_GENERAL);

extern "C" void mips_exception(mips_irq_context_t *context);

static void finish_exception_ipc (void);

static void send_exception_ipc( word_t exc_no, word_t exc_code,
        mips_irq_context_t *context, continuation_t continuation)
{
    tcb_t *current = get_current_tcb();
    if (current->get_exception_handler() == NULL) {
        TRACEF("Unable to deliver user exception: %p no exception handler.\n",
                current);
        return;
    }

    TRACEPOINT (EXCEPTION_IPC_GENERAL,
                printf ("EXCEPTION_IPC_GENERAL: (%p) exc_no %d, exc_code %016lx, IP = %p\n",
                        current, exc_no, exc_code, context->epc));

    current->arch.exc_code = exc_code;
    current->arch.exc_num = exc_no;

    // Save message registers on the stack
    msg_tag_t tag;

    /* We are just about to send an IPC. */
    current->sys_data.set_action(tcb_syscall_data_t::action_ipc);

    // Save message registers.
    current->save_state(1);

    /* don't allow receiving async */
    current->set_notify_mask(0);

    // Create the message tag.
    tag.set(EXCEPT_IPC_GEN_MR_NUM, EXCEPTION_TAG, true, true);
    current->set_tag(tag);

    // Deliver the exception IPC.
    current->arch.exception_continuation = continuation;
    current->do_ipc(current->get_exception_handler()->get_global_id(),
                    current->get_exception_handler()->get_global_id(), finish_exception_ipc);
}

static void
finish_exception_ipc(void)
{
    tcb_t * current = get_current_tcb();
    msg_tag_t tag;
    tag.raw = current->get_mr(0);

    current->resources.clear_exception_ipc(current);

    /* Alter the user context if necessary. */
    if (EXPECT_TRUE(!tag.is_error())) {
        current->set_user_ip((addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_IP));
        current->set_user_sp((addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_SP));
        current->set_user_flags(current->get_mr(EXCEPT_IPC_GEN_MR_FLAGS));
    }
    else {
        printf("Unable to deliver user exception: IPC error.\n");
    }

    /* Clean-up. */
    current->restore_state(1);
    current->clear_exception_ipc();

    ACTIVATE_CONTINUATION(current->arch.exception_continuation);
}

extern "C" bool handle_slow_syscall(mips_irq_context_t *context)
{
    if (context->at == MAGIC_KIP_REQUEST)
    {
        //TRACEF("KernelInterface() at %p\n", context->epc);
        space_t * space = get_current_space ();

        context->t0 = (word_t) space->get_kip_area ().get_base ();
        context->t1 = api_version_raw_get(get_kip()->api_version);
        context->t2 = api_flags_raw_get(get_kip()->api_flags);
        context->t3 = (NULL != get_kip()->kernel_desc_ptr) ?
            *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr) : 0;

        context->epc += 4;
        return true;        // Succesfully handled
    }
    return false; // Not handled
}

extern "C" bool mips64_break(mips_irq_context_t *context)
{
#if defined(CONFIG_DEBUG)
    /* Unless the $2 (v0) register is set to the magic KDB value,
     * treat this as a standard break instruction. */
    if (context->v0 != MAGIC_KDB_REQUEST) {
        return false;
    } else {
        /* Reset v0, so it is not left for future (non-KDB call)
         * break calls. */
        context->v0 = 0;
    }

    /*lint -e30 */
    switch (context->at) {
    /* Debug functions */
    case L4_TRAP_KPUTC:     putc((char)context->a0);    break;
    case L4_TRAP_KGETC:     context->v0 = getc(true);   break;
    case L4_TRAP_KGETC_NB:  context->v0 = getc(false);  break;
    case L4_TRAP_KDEBUG: {
            space_t *space = get_current_space();
            char *string = (char *)context->v1;
            char c;

            if (EXPECT_TRUE(get_kip()->kdebug_entry == NULL)) {
                return false;
            }
            if (space == NULL) {
                space = get_kernel_space();
            }
            printf( TXT_BRIGHT "--- KD# " );
            word_t user = (context->status & ST_KSU);
            if (!user) {
                printf( "<L4> " );
            }
            while (readmem(space, string, &c) && (c != 0)) {
                putc(c);
                string++;
            }
            if (c != 0) {
                printf("[not mapped]");
            }
            printf( " ---\n" TXT_NORMAL );
            kdebug_entry_t kdebug_entry = kip_kdebug_entry_get();
            kdebug_entry(context);
            break;
        }
#ifdef CONFIG_CPU_MIPS64_SB1
    case L4_TRAP_READ_PERF:
        context->a0 = mips_cpu::get_cp0_perf_control0();
        context->a1 = mips_cpu::get_cp0_perf_counter0();
        context->a2 = mips_cpu::get_cp0_perf_control1();
        context->a3 = mips_cpu::get_cp0_perf_counter1();
        context->t0 = mips_cpu::get_cp0_perf_control2();
        context->t1 = mips_cpu::get_cp0_perf_counter2();
        context->t2 = mips_cpu::get_cp0_perf_control3();
        context->t3 = mips_cpu::get_cp0_perf_counter3();
        break;
    case L4_TRAP_WRITE_PERF:
        switch(context->a0) {
        case 0: mips_cpu::set_cp0_perf_control0(context->a1); break;
        case 1: mips_cpu::set_cp0_perf_counter0(context->a1); break;
        case 2: mips_cpu::set_cp0_perf_control1(context->a1); break;
        case 3: mips_cpu::set_cp0_perf_counter1(context->a1); break;
        case 4: mips_cpu::set_cp0_perf_control2(context->a1); break;
        case 5: mips_cpu::set_cp0_perf_counter2(context->a1); break;
        case 6: mips_cpu::set_cp0_perf_control3(context->a1); break;
        case 7: mips_cpu::set_cp0_perf_counter3(context->a1); break;
        default:
            return false;
        }
        break;
#endif
    case L4_TRAP_KSET_OBJECT_NAME:
#if defined(CONFIG_THREAD_NAMES)       \
        || defined(CONFIG_SPACE_NAMES) \
        || defined(CONFIG_MUTEX_NAMES)
        {
            word_t type = context->a0;
            word_t id = context->a1;
            char name[MAX_DEBUG_NAME_LENGTH];
            word_t *name_words = (word_t *)name;

            /* Copy the name string from registers. */
            name_words[0] = context->a2;
            name_words[1] = context->a3;
#if defined(L4_32BIT)
            name_words[2] = context->t0;
            name_words[3] = context->t1;
#endif
            name[MAX_DEBUG_NAME_LENGTH - 1] = '\0';

            /* Set the object name. */
            (void)kdb_set_object_name((object_type_e)type, id, name);
        }
#endif
        break;
    default:
        return false;
    }
    ASSERT(NORMAL, !(context->cause & CAUSE_BD));

    context->epc += 4;
    return true;            // Succesfully handled
#else /* !CONFIG_DEBUG */
    /* If KDB is disabled, all 'break' calls are illegal instructions. */
    return false;
#endif /* !CONFIG_DEBUG */
}

static word_t get_reg(mips_irq_context_t *context, word_t num)
{
    switch (num)
    {
    case 0: return 0;
    case 1: return context->at; case 2: return context->v0;
    case 3: return context->v1; case 4: return context->a0;
    case 5: return context->a1; case 6: return context->a2;
    case 7: return context->a3; case 8: return context->t0;
    case 9: return context->t1; case 10: return context->t2;
    case 11: return context->t3; case 12: return context->t4;
    case 13: return context->t5; case 14: return context->t6;
    case 15: return context->t7; case 16: return context->s0;
    case 17: return context->s1; case 18: return context->s2;
    case 19: return context->s3; case 20: return context->s4;
    case 21: return context->s5; case 22: return context->s6;
    case 23: return context->s7; case 24: return context->t8;
    case 25: return context->t9; case 28: return context->gp;
    case 29: return context->sp; case 30: return context->s8;
    case 31: return context->ra;
    default:
           printf("Read k-register\n"); 
    }
    return 0;
}

static void set_reg(mips_irq_context_t *context, word_t num, word_t val)
{
    switch (num)
    {
    case 0: return;
    case 1: context->at = val; break; case 2: context->v0 = val; break;
    case 3: context->v1 = val; break; case 4: context->a0 = val; break;
    case 5: context->a1 = val; break; case 6: context->a2 = val; break;
    case 7: context->a3 = val; break; case 8: context->t0 = val; break;
    case 9: context->t1 = val; break; case 10: context->t2 = val; break;
    case 11: context->t3 = val; break; case 12: context->t4 = val; break;
    case 13: context->t5 = val; break; case 14: context->t6 = val; break;
    case 15: context->t7 = val; break; case 16: context->s0 = val; break;
    case 17: context->s1 = val; break; case 18: context->s2 = val; break;
    case 19: context->s3 = val; break; case 20: context->s4 = val; break;
    case 21: context->s5 = val; break; case 22: context->s6 = val; break;
    case 23: context->s7 = val; break; case 24: context->t8 = val; break;
    case 25: context->t9 = val; break; case 28: context->gp = val; break;
    case 29: context->sp = val; break; case 30: context->s8 = val; break;
    case 31: context->ra = val; break;
    default:
        printf("Write k-register\n");
    }
}

typedef struct {
    union {
        u32_t raw;
        struct {
            s32_t offset : 16;
            u32_t rt : 5;
            u32_t base : 5;
            u32_t op : 6;
        } x;
    };
} load_instr;

static struct {
    word_t address;
    word_t space;
} llval;

extern "C" bool mips64_illegal(mips_irq_context_t *context, word_t* code)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;

    space_t::access_e access = space_t::read;

    /* Check for emulated instructions */
    load_instr op = *(load_instr *)context->epc;
    *code = *(u32_t *)context->epc;

    /* Wait instruction used for SLOW syscalls */
    if (op.raw == 0x42000020)
    {
        if (handle_slow_syscall(context)) {
            return true;
        }
    }

    switch(op.x.op)    /* Get instruction Op */
    {
        case 0x38:  /* SC   Store Conditional Word      */
        case 0x3c:  /* SCD  Store Conditional Double    */
            access = space_t::write;
            break;
        case 0x30:  /* LL   Load Linked Word            */
        case 0x34:  /* LLD  Load Linked Double          */
            break;
        default:
            return false;
    }

    word_t address = get_reg(context, op.x.base) + op.x.offset;

    space_t * space = get_current_tcb ()->get_space();
    if (space == NULL)
        space = get_kernel_space();

    // Check if address exists in page table and is writeable if needed
    while (!space->lookup_mapping ((addr_t)address, &pg, &pgsize) ||
            ((access == space_t::write) && (!pg->is_writable(space, pgsize))))
    {
        panic("Please implement me!");
        // FIXME: make this compatible with single stack
        // space->handle_pagefault ((addr_t)address, (addr_t)context->epc, access, false);
    }

    addr_t paddr = pg->address (space, pgsize);
    paddr = addr_offset (paddr, address & page_mask (pgsize));

    word_t align = 0x3;
    // Word access is properly aligned?
    switch(op.x.op)    /* Get instruction Op */
    {
        case 0x34:  /* LLD  Load Linked Double          */
        case 0x3c:  /* SCD  Store Conditional Double    */
            align = 0x7;
            break;
    }
    if (address & align)
    {
        /* Fake an exception frame */
        mips_irq_context_t *old = context;
        context--;
        /*lint -e506 FIXME: This look slike a real error */
        context->cause = (old->cause & ~CAUSE_EXCCODE) | (access ? (5<<2) : (4<<2));
        context->epc = old->epc;
        context->badvaddr = address;
        mips_exception(context);
        context++;
    }

    switch(op.x.op)    /* Get instruction Op */
    {
        case 0x30:  /* LL   Load Linked Word            */
            llval.address = (word_t)paddr; llval.space = (word_t)space;
            set_reg(context, op.x.rt, *(s32_t *)address); break;
        case 0x34:  /* LLD  Load Linked Double          */
            llval.address = (word_t)paddr; llval.space = (word_t)space;
            set_reg(context, op.x.rt, *(word_t *)address); break;
        case 0x38:  /* SC   Store Conditional Word      */
            if ((llval.address == (word_t)paddr)&&(llval.space == (word_t)space))
            {
                *(u32_t *)address = (u32_t)get_reg(context, op.x.rt);
                set_reg(context, op.x.rt, 1);
            } else
            {
                set_reg(context, op.x.rt, 0);
            }
            break;
        case 0x3c:  /* SCD  Store Conditional Double    */
            if ((llval.address == (word_t)paddr)&&(llval.space == (word_t)space))
            {
                *(word_t *)address = get_reg(context, op.x.rt);
                set_reg(context, op.x.rt, 1);
            } else
            {
                set_reg(context, op.x.rt, 0);
            }
            break;
    }
    ASSERT(NORMAL, !(context->cause & CAUSE_BD));

    context->epc += 4;
    return true;            // Succesfully handled
}

extern "C" bool mips64_watch(mips_irq_context_t *context, word_t *code)
{
    /* Kernel should check if kdb wants this exception */
    return false;
}

extern "C" bool mips64_cpu_unavail(mips_irq_context_t *context, word_t *code)
{
    switch (CAUSE_CE_NUM(context->cause)) {
    case 0: /* COP0 unavailable */
        u32_t instr = *(u32_t*)context->epc;
        /* Wait instruction used for SLOW syscalls */
        if (instr == 0x42000020)
        {
            if (handle_slow_syscall(context)) {
                return true;
            }
        }
        break;
    case 1: /* COP1 unavailable */
        /* Floating Point unavailable exception */
        {
            tcb_t *current_tcb = get_current_tcb();
            ASSERT(NORMAL, context->status & ST_KSU);

            return current_tcb->resources.mips_fpu_unavail_exception( current_tcb, context );
        }
    case 2: /* COP2 unavailable */
        break;
    case 3: /* COP3 unavailable */
        break;
    default:
        break;
    }

    *code = CAUSE_CE_NUM(context->cause);
    return false;
}

static void exception_error_check(void);

extern "C" void mips_exception(mips_irq_context_t *context)
{
    char * exception = NULL;
    bool result = false;
    word_t exc_no = CAUSE_EXCCODE_NUM(context->cause);
    word_t exc_code = 0;
    tcb_t * current = get_current_tcb();

    switch (exc_no) {
        case 4:  exception = "Address error (load/execute)";
                 exc_code = context->badvaddr;                  break;
        case 5:  exception = "Address error (store)";
                 exc_code = context->badvaddr;                  break;
        case 6:  exception = "Bus error (instruction fetch)";   break;
        case 7:  exception = "Bus error (data load/store)";     break;
        case 9:  exception = "Breakpoint";
                 result = mips64_break(context);                break;
        case 10: exception = "Illegal instruction";
                 result = mips64_illegal(context, &exc_code);   break;
        case 11: exception = "Coprocessor unavailable";
                 result = mips64_cpu_unavail(context, &exc_code); break;
        case 12: exception = "Arithmetic overflow";             break;
        case 13: exception = "Trap exception";                  break;
        case 14: exception = "Virtual coherency exception (instruction)"; break;
        case 15: exception = "Floating point exception";        break;
        case 18: exception = "Coprocessor 2 exception";         break;
        case 22: exception = "MDMX Unusable";                   break;
        case 23: exception = "Watchpoint";
                 result = mips64_watch(context, &exc_code);     break;
        case 24: exception = "Machine check";                   break;
        case 30: exception = "Cache error in debug mode";       break;
        case 31: exception = "Virtual coherency exception (data)"; break;
        default: break;
    }

    if (EXPECT_FALSE(result == false)) {
        current->arch.exception = exception;
        current->arch.error_context = context;
        send_exception_ipc(exc_no, exc_code, context, ASM_CONTINUATION);
    }
}

static void
exception_error_check(void)
{
    tcb_t * current = get_current_tcb();
    msg_tag_t tag;
    tag.raw = current->get_mr(0);

    if (EXPECT_FALSE(tag.is_error())) {
        kdebug_entry_t kdebug_entry = kip_kdebug_entry_get();
        if(EXPECT_FALSE(kdebug_entry != NULL)) {
            printf( TXT_BRIGHT "--- KD# Unhandled Exception [");
            if (current->arch.exception) {
                printf("%s", current->arch.exception);
            }
            else {
                printf("%d", current->arch.exc_num);
            }
            printf("] ---\n" TXT_NORMAL);

            kdebug_entry(current->arch.error_context);
        }
        get_current_scheduler()->
            deactivate_sched(current, thread_state_t::halted,
                             current, current->arch.exception_continuation,
                             scheduler_t::sched_default);
    }
    ACTIVATE_CONTINUATION(current->arch.exception_continuation);
}

static void finish_syscall_ipc (void);

static void send_syscall_ipc(mips_irq_context_t *_ctx,
                             continuation_t continuation)
{
    tcb_t * RESTRICT current = get_current_tcb();
    mips_irq_context_t * RESTRICT context = _ctx;

    if (current->get_exception_handler().is_nilthread()) {
        TRACEF("Unable to deliver syscall exception: %p no exception handler.\n",
                current);
        return;
    }

    TRACEPOINT (EXCEPTION_IPC_SYSCALL,
                printf ("EXCEPTION_IPC_SYSCALL: (%p) IP = %p, v0 = 0x%016lx\n",
                        current, (word_t)current->get_user_ip(), context->v0));

    msg_tag_t tag;

    // Save message registers.
    current->save_state();

    // Create the message tag.
    tag.set(EXCEPT_IPC_SYS_MR_NUM, EXCEPTION_TAG, true, true);
    current->set_tag( tag );

    // Create the message.
    {
        utcb_t * RESTRICT utcb = current->get_utcb();
        utcb->mr[EXCEPT_IPC_SYS_MR_V0] = context->v0;
        utcb->mr[EXCEPT_IPC_SYS_MR_V1] = context->v1;
        utcb->mr[EXCEPT_IPC_SYS_MR_A0] = context->a0;
        utcb->mr[EXCEPT_IPC_SYS_MR_A1] = context->a1;
        utcb->mr[EXCEPT_IPC_SYS_MR_A2] = context->a2;
        utcb->mr[EXCEPT_IPC_SYS_MR_A3] = context->a3;
        utcb->mr[EXCEPT_IPC_SYS_MR_A4] = context->t0;
        utcb->mr[EXCEPT_IPC_SYS_MR_A5] = context->t1;
        utcb->mr[EXCEPT_IPC_SYS_MR_A6] = context->t2;
        utcb->mr[EXCEPT_IPC_SYS_MR_A7] = context->t3;

        utcb->mr[EXCEPT_IPC_SYS_MR_IP] = (word_t)current->get_user_ip();
        utcb->mr[EXCEPT_IPC_SYS_MR_SP] = (word_t)current->get_user_sp();
        utcb->mr[EXCEPT_IPC_SYS_MR_FLAGS] = (word_t)current->get_user_flags() |
                (context->cause & CAUSE_BD ? 1 : 0);
    }

    // For fast path, we need to indicate that we are doing exception ipc
    current->resources.set_exception_ipc( current );

    // Deliver the exception IPC.
    current->arch.exception_continuation = continuation;
    current->arch.error_context = context;
    current->do_ipc(current->get_exception_handler(),
                    current->get_exception_handler(), finish_syscall_ipc);
}

static
void finish_syscall_ipc (void)
{
    tcb_t * RESTRICT current = get_current_tcb();
    msg_tag_t tag;

    tag.raw = current->get_mr(0);

    current->resources.clear_exception_ipc(current);

    // Alter the user context if necessary.
    if (EXPECT_FALSE(!tag.is_error())) {
        current->set_user_ip((addr_t)current->get_mr(EXCEPT_IPC_SYS_MR_IP));
        current->set_user_sp((addr_t)current->get_mr(EXCEPT_IPC_SYS_MR_SP));
        current->set_user_flags(current->get_mr(EXCEPT_IPC_SYS_MR_FLAGS));

        /* XXX cleanup this SKS stuff */
        current->arch.error_context->v0 = current->get_mr(EXCEPT_IPC_SYS_MR_V0);
        current->arch.error_context->v1 = current->get_mr(EXCEPT_IPC_SYS_MR_V1);
        current->arch.error_context->a0 = current->get_mr(EXCEPT_IPC_SYS_MR_A0);
        current->arch.error_context->a1 = current->get_mr(EXCEPT_IPC_SYS_MR_A1);
        current->arch.error_context->a2 = current->get_mr(EXCEPT_IPC_SYS_MR_A2);
        current->arch.error_context->a3 = current->get_mr(EXCEPT_IPC_SYS_MR_A3);
        current->arch.error_context->t0 = current->get_mr(EXCEPT_IPC_SYS_MR_A4);
        current->arch.error_context->t1 = current->get_mr(EXCEPT_IPC_SYS_MR_A5);
        current->arch.error_context->t2 = current->get_mr(EXCEPT_IPC_SYS_MR_A6);
        current->arch.error_context->t3 = current->get_mr(EXCEPT_IPC_SYS_MR_A7);
    }
    else {
        printf( "Unable to deliver user exception: IPC error.\n" );
    }

    // Clean-up.
    current->restore_state();

    ACTIVATE_CONTINUATION(current->arch.exception_continuation);
}

static void finish_syscall_exception (void);

extern "C" void syscall_exception(mips_irq_context_t *context)
{
    tcb_t * current = get_current_tcb();
    /* XXX cleanup this SKS stuff */
    current->arch.error_context = context;
    send_syscall_ipc(context, ASM_CONTINUATION);
}

static void finish_syscall_exception(void)
{
    tcb_t * current = get_current_tcb();
    msg_tag_t tag;
    tag.raw = current->get_mr(0);
    if(tag.is_error()) {
        kdebug_entry_t kdebug_entry = kip_kdebug_entry_get();
        if (EXPECT_FALSE(kdebug_entry != NULL)) {
            printf( TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL,
                    "Unhandled User SYSCALL" );

            kdebug_entry(current->arch.error_context);
        }
        get_current_scheduler()->
            deactivate_sched(current, thread_state_t::halted,
                             current, current->arch.exception_continuation,
                             scheduler_t::sched_default);
    }
    ACTIVATE_CONTINUATION(current->arch.exception_continuation);
}
