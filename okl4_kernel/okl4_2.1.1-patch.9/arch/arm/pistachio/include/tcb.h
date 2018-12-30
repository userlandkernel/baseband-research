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
 * Description:   ARM TCB handling functions
 */
#ifndef __ARM__TCB_H__
#define __ARM__TCB_H__

#include <kernel/arch/asm.h>

#ifndef __TCB_H__
#error not for stand-alone inclusion
#endif

extern stack_t __stack;


/**
 * Return current tcb pointer.
 */
PURE INLINE tcb_t *
get_current_tcb (void)
{
    return get_arm_globals()->current_tcb;
}

INLINE clist_t* get_current_clist(void) PURE;

INLINE clist_t* get_current_clist(void)
{
    return get_arm_globals()->current_clist;
}


/* Define Stack and Continuation handling functions */

#if defined (__GNUC__)
#define ACTIVATE_CONTINUATION(continuation)     \
    do {                \
        __asm__ __volatile__ (                                          \
                "       orr     sp,     sp,     %1              \n"     \
                "       mov     pc,     %0                      \n"     \
                :                                                       \
                : "r" ((word_t)(continuation)),                         \
                  "i" (STACK_TOP)                                       \
                : "sp", "memory"                                        \
                );                                                      \
        while(1);                                                       \
    } while(false)


/* call a function with 2 arguments + a continuation so that the continuation can be retrieved by ASM_CONTINUATION */
NORETURN INLINE void call_with_asm_continuation(word_t argument0, word_t argument1, word_t continuation, word_t function)
{
    register word_t arg0    ASM_REG("r0") = argument0;
    register word_t arg1    ASM_REG("r1") = argument1;
    register word_t lr      ASM_REG("r14") = continuation;

    __asm__ __volatile__ (
        CHECK_ARG("r0", "%0")
        CHECK_ARG("r1", "%1")
        CHECK_ARG("lr", "%2")
        "       mov     pc,     %3      \n"
        :: "r" (arg0), "r" (arg1), "r" (lr),
        "r" (function)
    );
    while (1);
}

#elif defined (__RVCT_GNU__)

NORETURN void jump_and_set_sp(word_t ret, addr_t context);
NORETURN void call_with_asm_continuation(word_t argument0, word_t argument1, word_t continuation, word_t function);

#define TOP_STACK       ((tcb_t **)(__current_sp() | STACK_TOP))

#define ACTIVATE_CONTINUATION(continuation)     \
    jump_and_set_sp((word_t)(continuation), (addr_t)TOP_STACK)


#elif defined(_lint)
void call_with_asm_continuation(word_t argument0, word_t argument1, word_t continuation, word_t function);
#define TOP_STACK       ((tcb_t **)0)
#define ACTIVATE_CONTINUATION(continuation) ((continuation_t) continuation)()
INLINE tcb_t * get_current_tcb (void)
{
}
#else
#error "Unknown compiler"
#endif
INLINE void tcb_t::enable_preempt_recover(continuation_t continuation)
{
    /* can't jump straight to the continuation as will have invalid stack pointer */
    preemption_continuation = continuation;
}

INLINE void tcb_t::disable_preempt_recover()
{
    preemption_continuation = 0;
}

/* End - stack/continuation functions. */

#include <kernel/syscalls.h>           /* for sys_ipc */
#include <kernel/arch/thread.h>
#include <kernel/cpu/cache.h>
#include <kernel/arch/resource_functions.h>
#include <kernel/cpu/syscon.h>

/* include ARM version specific implementations */
#include <kernel/arch/ver/tcb.h>

INLINE word_t tcb_t::set_tls(word_t *mr)
{
    get_utcb()->kernel_reserved[0] = mr[0];
    return EOK;
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    return get_utcb()->mr[index];
}

/**
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    get_utcb()->mr[index] = value;
}

/**
 * read value of the acceptor
 */
INLINE acceptor_t tcb_t::get_acceptor(void)
{
    return get_utcb()->acceptor;
}

/**
 * set the value of the acceptor register
 * @param value value to set
 */
INLINE void tcb_t::set_acceptor(const acceptor_t value)
{
    get_utcb()->acceptor = value;
}

/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 * @return whether operation succeeded
 */
INLINE bool tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    word_t *dest_mr = &dest->get_utcb()->mr[start];
    word_t *src_mr = &get_utcb()->mr[start];

    if ((start + count) > IPC_NUM_MR)
        return false;

    /* This will always copy at least 1 register,
     * assuming IPCs with 0 MRs are rare.
     */
#if defined(__GNUC__)
    word_t temp1, temp2;
    __asm__ __volatile__ (
        "1:                             \n"
        "ldr    %[t1],  [%[src]], #4    \n"
        "ldr    %[t2],  [%[src]], #4    \n"
        "subs   %[num], %[num], #2      \n"
        "str    %[t1],  [%[dst]], #4    \n"
        "strpl  %[t2],  [%[dst]], #4    \n"
        "bgt    1b                      \n"
        : [t1] "=r" (temp1),
          [t2] "=r" (temp2),
          [src] "+r" (src_mr),
          [dst] "+r" (dest_mr),
          [num] "+r" (count)
    );
#else
    do {
            *dest_mr++ = *src_mr++;
    } while(--count > 0);
#endif
    return true;
}

INLINE bool tcb_t::copy_exception_mrs_from_frame(tcb_t *dest)
{
    arm_irq_context_t *context = &arch.context;

    word_t * RESTRICT mr = &dest->get_utcb()->mr[0];

    // Create the message.
    if ((resource_bits & (1UL << EXCEPTIONFP)) == 0) {
        mr[EXCEPT_IPC_GEN_MR_IP] = PC(context->pc);
        mr[EXCEPT_IPC_GEN_MR_SP] = context->sp;
        mr[EXCEPT_IPC_GEN_MR_FLAGS] = context->cpsr;
        mr[EXCEPT_IPC_GEN_MR_EXCEPTNO] = arch.exc_num;
        mr[EXCEPT_IPC_GEN_MR_ERRORCODE] = arch.exc_code;
    } else {
        mr[EXCEPT_IPC_SYS_MR_PC] = PC(context->pc);
        mr[EXCEPT_IPC_SYS_MR_R0] = context->r0;
        mr[EXCEPT_IPC_SYS_MR_R1] = context->r1;
        mr[EXCEPT_IPC_SYS_MR_R2] = context->r2;
        mr[EXCEPT_IPC_SYS_MR_R3] = context->r3;
        mr[EXCEPT_IPC_SYS_MR_R4] = context->r4;
        mr[EXCEPT_IPC_SYS_MR_R5] = context->r5;
        mr[EXCEPT_IPC_SYS_MR_R6] = context->r6;
        mr[EXCEPT_IPC_SYS_MR_R7] = context->r7;
        mr[EXCEPT_IPC_SYS_MR_SP] = context->sp;
        mr[EXCEPT_IPC_SYS_MR_LR] = context->lr;
#ifdef CONFIG_ARM_THUMB_SUPPORT
        if (context->cpsr & CPSR_THUMB_BIT)
            mr[EXCEPT_IPC_SYS_MR_SYSCALL] = *(u16_t *)(PC(context->pc)-2);
        else
#endif
            mr[EXCEPT_IPC_SYS_MR_SYSCALL] = *(word_t *)(PC(context->pc)-4);

        mr[EXCEPT_IPC_SYS_MR_FLAGS] = context->cpsr;
    }

    return true;
}

INLINE bool tcb_t::copy_exception_mrs_to_frame(tcb_t *dest)
{
    if ((dest->resource_bits & (1UL << EXCEPTIONFP)) == 0) {
        dest->set_user_ip( (addr_t)get_mr(EXCEPT_IPC_GEN_MR_IP) );
        dest->set_user_sp( (addr_t)get_mr(EXCEPT_IPC_GEN_MR_SP) );
        dest->set_user_flags( get_mr(EXCEPT_IPC_GEN_MR_FLAGS) );
    } else {
        arm_irq_context_t *context = &dest->arch.context;
        word_t * RESTRICT mr = &this->get_utcb()->mr[0];

        dest->set_user_ip( (addr_t)get_mr(EXCEPT_IPC_SYS_MR_PC) );
        dest->set_user_sp( (addr_t)get_mr(EXCEPT_IPC_SYS_MR_SP) );
        dest->set_user_flags( get_mr(EXCEPT_IPC_SYS_MR_FLAGS) );

        context->r0 = mr[EXCEPT_IPC_SYS_MR_R0];
        context->r1 = mr[EXCEPT_IPC_SYS_MR_R1];
        context->r2 = mr[EXCEPT_IPC_SYS_MR_R2];
        context->r3 = mr[EXCEPT_IPC_SYS_MR_R3];
        context->r4 = mr[EXCEPT_IPC_SYS_MR_R4];
        context->r5 = mr[EXCEPT_IPC_SYS_MR_R5];
        context->r6 = mr[EXCEPT_IPC_SYS_MR_R6];
        context->r7 = mr[EXCEPT_IPC_SYS_MR_R7];
        context->lr = mr[EXCEPT_IPC_SYS_MR_LR];
    }

    return true;
}

INLINE bool tcb_t::in_exception_ipc(void)
{
    return (resource_bits & (1UL << EXCEPTIONFP)) || (arch.exc_num != 0);
}

INLINE void tcb_t::clear_exception_ipc(void)
{
    arch.exc_num = 0;
    resource_bits & ~(1UL << EXCEPTIONFP);
}

/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * new_space)
{
    this->space = new_space;

    if (EXPECT_TRUE(new_space)) {
        this->space_id = new_space->get_space_id();
        this->page_directory = new_space->pgent(0);
    } else {
        this->space_id = spaceid_t::nilspace();
        this->page_directory = NULL;
    }
}

/**
 * set the global thread ID in a TCB
 * @param tid   new thread ID
 */
INLINE void tcb_t::set_global_id(threadid_t tid)
{
    myself_global = tid;
}

/**
 * set the mutex thread handle in a UTCB
 * @param handle mutex thread handle
 */
INLINE void tcb_t::set_mutex_thread_handle(threadid_t handle)
{
    get_utcb()->mutex_thread_handle = handle;
}

/*
 * Return back to user_land when an IPC is aborted
 * We short circuit the restoration of any saved registers/state
 */
INLINE void tcb_t::return_from_ipc (void)
{
    extern void * vector_ipc_syscall_return;

    ACTIVATE_CONTINUATION(&vector_ipc_syscall_return);
}


/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption(void)
{
    extern void * vector_arm_abort_return;

#ifdef CONFIG_IPC_FASTPATH
    tcb_t * current = get_current_tcb();
    current->resources.clear_kernel_ipc( current );
    current->resources.clear_except_fp( current );
#endif

    ACTIVATE_CONTINUATION(&vector_arm_abort_return);
    // NOT REACHED
}


/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    /* Clear the exception context */
    arm_irq_context_t *context = &arch.context;
    word_t *t, *end;
    word_t size = sizeof(arch.context);

    /* clear whole context */
    t = (word_t*)context;
    end = (word_t*)((word_t)context + (size & ~(15)));

    do {
        *t++ = 0;
        *t++ = 0;
        *t++ = 0;
        *t++ = 0;
    } while (t < end);

    while (t < (word_t*)(context+1))
        *t++ = 0;

#if CONFIG_ARM_VER <= 5
    context->pc = 1;    /* Make it look like an exception context. */
#endif
}



/**********************************************************************
 *
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level stack pointer
 * @return      the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    arm_irq_context_t * context = &arch.context;

    return (addr_t) (context)->sp;
}

/**
 * set the user-level stack pointer
 * @param sp    new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    arm_irq_context_t *context = &arch.context;

    context->sp = (word_t)sp;
}

INLINE void tcb_t::copy_user_regs(tcb_t *src)
{
    arm_irq_context_t * RESTRICT to = &arch.context;
    arm_irq_context_t * RESTRICT from = &(src->arch.context);

    to->r0 = from->r0;
    to->r1 = from->r1;
    to->r2 = from->r2;
    to->r3 = from->r3;
    to->r4 = from->r4;
    to->r5 = from->r5;
    to->r6 = from->r6;
    to->r7 = from->r7;
    to->r8 = from->r8;
    to->r9 = from->r9;
    to->r10 = from->r10;
    to->r11 = from->r11;
    to->r12 = from->r12;
    to->lr = from->lr;
    to->cpsr = from->cpsr;
}

INLINE void tcb_t::copy_regs_to_mrs(tcb_t *src)
{
    arm_irq_context_t * RESTRICT from = &(src->arch.context);
    word_t * RESTRICT mr = &this->get_utcb()->mr[0];

    mr[0] = from->r0;
    mr[1] = from->r1;
    mr[2] = from->r2;
    mr[3] = from->r3;
    mr[4] = from->r4;
    mr[5] = from->r5;
    mr[6] = from->r6;
    mr[7] = from->r7;
    mr[8] = from->r8;
    mr[9] = from->r9;
    mr[10] = from->r10;
    mr[11] = from->r11;
    mr[12] = from->r12;
    mr[13] = from->sp;
    mr[14] = from->lr;
    mr[15] = (word_t)src->get_user_ip();
    mr[16] = from->cpsr;
}

INLINE void tcb_t::copy_mrs_to_regs(tcb_t *dest)
{
    arm_irq_context_t * RESTRICT from = &(dest->arch.context);
    word_t * RESTRICT mr = &this->get_utcb()->mr[0];

    from->r0  = mr[0];
    from->r1  = mr[1];
    from->r2  = mr[2];
    from->r3  = mr[3];
    from->r4  = mr[4];
    from->r5  = mr[5];
    from->r6  = mr[6];
    from->r7  = mr[7];
    from->r8  = mr[8];
    from->r9  = mr[9];
    from->r10 = mr[10];
    from->r11 = mr[11];
    from->r12 = mr[12];
    from->sp  = mr[13];
    from->lr  = mr[14];
    dest->set_user_flags(mr[16]);
    dest->set_user_ip((addr_t)mr[15]);
}

INLINE word_t tcb_t::get_utcb_location()
{
    return (word_t)this->utcb_location;
}

INLINE void tcb_t::set_utcb_location(word_t location)
{
    utcb_location = location;
}


/**
 * read the user-level flags (one word)
 * @return      the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    arm_irq_context_t * context = &(arch.context);

    return (word_t) (context)->cpsr & ARM_USER_FLAGS_MASK;
}

/**
 * set the user-level flags
 * @param flags new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    arm_irq_context_t *context = &(arch.context);

    context->cpsr = (context->cpsr & ~ARM_USER_FLAGS_MASK) |
            ((word_t)flags & ARM_USER_FLAGS_MASK);
}

/**********************************************************************
 *
 *                       preemption callback function
 *
 **********************************************************************/

/**
 * set the address where preemption occured
 */
INLINE void tcb_t::set_preempted_ip(addr_t ip)
{
#ifdef CONFIG_ARM_THUMB_SUPPORT
    arm_irq_context_t *context = &(arch.context);

    /* CPU has thumb support, fix ip if needed */
    if (context->cpsr & CPSR_THUMB_BIT)
        ip = (addr_t)((word_t)ip | 1);
#endif
    get_utcb()->preempted_ip = (word_t)ip;
}

INLINE addr_t tcb_t::get_preempted_ip()
{
    return (addr_t)get_utcb()->preempted_ip;
}

/**
 * get the preemption callback ip
 */
INLINE addr_t tcb_t::get_preempt_callback_ip()
{
    return (addr_t)get_utcb()->preempt_callback_ip;
}

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

/**
 * invoke an IPC from within the kernel
 *
 * @param to_tid destination thread id
 * @param from_tid from specifier
 */
void do_ipc_helper(void);

INLINE void
tcb_t::do_ipc(threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
#if defined(CONFIG_IPC_FASTPATH)
    tcb_t *current = get_current_tcb();
    /* For fast path, we need to indicate that we are doing ipc from the
     * kernel. */
    current->resources.set_kernel_ipc(current);
#endif

    /* Keep track of where we need to come back to. */
    TCB_SYSDATA_IPC(this)->do_ipc_continuation = continuation;

    /* Call sys_ipc with a custom continuation as the return address. */
#if defined(CONFIG_IPC_C_FASTPATH)
    call_with_asm_continuation(to_tid.get_raw(), from_tid.get_raw(),
            (word_t)do_ipc_helper, (word_t)sys_ipc_c_fastpath);
#else
    call_with_asm_continuation(to_tid.get_raw(), from_tid.get_raw(),
            (word_t)do_ipc_helper, (word_t)sys_ipc);
#endif
}

NORETURN INLINE void
SYS_IPC_SLOW(threadid_t to_tid, threadid_t from_tid,
        continuation_t continuation)
{
    call_with_asm_continuation(to_tid.get_raw(), from_tid.get_raw(),
            (word_t)continuation,(word_t) sys_ipc);
}

NORETURN INLINE void
SYS_IPC_RESTART(threadid_t to_tid, threadid_t from_tid,
        continuation_t continuation)
{
    /* Call sys_ipc with continuation as the return address. */
    call_with_asm_continuation(to_tid.get_raw(), from_tid.get_raw(),
            (word_t)continuation, (word_t)sys_ipc);
}

#endif /* !__ARM__TCB_H__ */
