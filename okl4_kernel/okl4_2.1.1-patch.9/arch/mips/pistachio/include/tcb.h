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
 * Description:   Mips specific thread support functions
 * Author: Carl van Schaik
 */

#ifndef __MIPS__TCB_H__
#define __MIPS__TCB_H__

#ifndef __TCB_H__
#error not for stand-alone inclusion
#endif

#include <kernel/debug.h>

#include <kernel/arch/types.h>
#include <kernel/syscalls.h>
#include <kernel/arch/resource_functions.h>

extern word_t K_CURRENT_TCB;
extern word_t K_CURRENT_CLIST;

#if defined(__GNUC__)
#define ACTIVATE_CONTINUATION(continuation)             \
    do {                                                \
        __asm__ __volatile__ (                          \
                "       .set    push            \n"     \
                "       .set    noreorder       \n"     \
                "       jr      %[cont]         \n"     \
                "       ori     $29, $29, %0    \n"     \
                "       .set    pop             \n"     \
                :: "i" (STACK_TOP),                     \
                   [cont] "r" (continuation)            \
                );                                      \
        while (1);                                      \
    } while(false)
#elif defined(_lint)
#define ACTIVATE_CONTINUATION(continuation) ((continuation_t) continuation)()
#else
#define "Unknown compiler"
#endif


/**
 * Locate current TCB and return it.
 */
INLINE tcb_t * get_current_tcb (void) PURE;

INLINE tcb_t * get_current_tcb (void)
{
    return (tcb_t *) K_CURRENT_TCB;
}

INLINE clist_t* get_current_clist(void) PURE;

INLINE clist_t* get_current_clist(void)
{
    return (clist_t *) K_CURRENT_CLIST;
}

/*
 * Not yet implemented.
 */
INLINE word_t tcb_t::set_tls(word_t *mr)
{
    return EINVALID_PARAM;
}

INLINE void tcb_t::enable_preempt_recover(continuation_t continuation)
{
    preemption_continuation = continuation;
}

INLINE void tcb_t::disable_preempt_recover()
{
    preemption_continuation = 0;
}

#include <kernel/arch/cache.h>

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
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */

INLINE bool tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    word_t *dest_mr = &dest->get_utcb()->mr[start];
    word_t *src_mr = &get_utcb()->mr[start];

    if (start + count > IPC_NUM_MR)
        return false;

    do {
        *dest_mr++ = *src_mr++;
    } while(--count > 0);

    return true;
}

INLINE bool tcb_t::copy_exception_mrs_from_frame(tcb_t *dest) {
    UNIMPLEMENTED();
    return false;
}

INLINE bool tcb_t::copy_exception_mrs_to_frame(tcb_t *dest) {
    UNIMPLEMENTED();
    return false;
}

INLINE bool tcb_t::in_exception_ipc(void)
{
    return false;
}

INLINE void tcb_t::clear_exception_ipc(void)
{
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
 * Exchange registers register access/copy functions
 */

INLINE void tcb_t::copy_user_regs(tcb_t *src)
{
    mips_irq_context_t * RESTRICT to = &arch.context;
    mips_irq_context_t * RESTRICT from = &src->arch.context;

    to->v0 = from->v0;
    to->v1 = from->v1;
    to->a0 = from->a0;
    to->a1 = from->a1;
    to->a2 = from->a2;
    to->a3 = from->a3;
    to->t0 = from->t0;
    to->t1 = from->t1;
    to->t2 = from->t2;
    to->t3 = from->t3;
    to->t4 = from->t4;
    to->t5 = from->t5;
    to->t6 = from->t6;
    to->t7 = from->t7;
    to->s0 = from->s0;
    to->s1 = from->s1;
    to->s2 = from->s2;
    to->s3 = from->s3;
    to->s4 = from->s4;
    to->s5 = from->s5;
    to->s6 = from->s6;
    to->s7 = from->s7;
    to->gp = from->gp;
    //to->sp = from->sp;
    to->at = from->at;
    to->t8 = from->t8;
    to->t9 = from->t9;
    to->s8 = from->s8;
    to->ra = from->ra;
    to->hi = from->hi;
    to->lo = from->lo;
    to->status = from->status;
}

INLINE void tcb_t::copy_mrs_to_regs(tcb_t *dest)
{
    mips_irq_context_t * RESTRICT from = &dest->arch.context;
    word_t * RESTRICT mr = &this->get_utcb()->mr[0];

    from->at = mr[0];
    from->v0 = mr[1];
    from->v1 = mr[2];
    from->a0 = mr[3];
    from->a1 = mr[4];
    from->a2 = mr[5];
    from->a3 = mr[6];
    from->t0 = mr[7];
    from->t1 = mr[8];
    from->t2 = mr[9];
    from->t3 = mr[10];
    from->t4 = mr[11];
    from->t5 = mr[12];
    from->t6 = mr[13];
    from->t7 = mr[14];
    from->s0 = mr[15];
    from->s1 = mr[16];
    from->s2 = mr[17];
    from->s3 = mr[18];
    from->s4 = mr[19];
    from->s5 = mr[20];
    from->s6 = mr[21];
    from->s7 = mr[22];
    from->t8 = mr[23];
    from->t9 = mr[24];
    from->gp = mr[25];
    from->sp = mr[26];
    from->s8 = mr[27];
    from->ra = mr[28];
    from->hi = mr[29];
    from->lo = mr[30];
    dest->set_user_flags(mr[32]);
    from->epc = mr[31];
}

INLINE void tcb_t::copy_regs_to_mrs(tcb_t *src)
{
    mips_irq_context_t * RESTRICT from = &src->arch.context;
    word_t * RESTRICT mr = &this->get_utcb()->mr[0];

    mr[0] = from->at;
    mr[1] = from->v0;
    mr[2] = from->v1;
    mr[3] = from->a0;
    mr[4] = from->a1;
    mr[5] = from->a2;
    mr[6] = from->a3;
    mr[7] = from->t0;
    mr[8] = from->t1;
    mr[9] = from->t2;
    mr[10] = from->t3;
    mr[11] = from->t4;
    mr[12] = from->t5;
    mr[13] = from->t6;
    mr[14] = from->t7;
    mr[15] = from->s0;
    mr[16] = from->s1;
    mr[17] = from->s2;
    mr[18] = from->s3;
    mr[19] = from->s4;
    mr[20] = from->s5;
    mr[21] = from->s6;
    mr[22] = from->s7;
    mr[23] = from->t8;
    mr[24] = from->t9;
    mr[25] = from->gp;
    mr[26] = from->sp;
    mr[27] = from->s8;
    mr[28] = from->ra;
    mr[29] = from->hi;
    mr[30] = from->lo;
    mr[31] = from->epc;
    mr[32] = src->get_user_flags();
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
    } else {
        this->space_id = spaceid_t::nilspace();
    }
    // sometimes it might be desirable to use a pdir cache,
    // like in cases where it's not cheap to derive the page
    // directory from the space
    //this->pdir_cache = (word_t)space->get_pdir();
}


/**
 * set the global thread ID in a TCB
 * @param tid   new thread ID
 */
INLINE void tcb_t::set_global_id(threadid_t tid)
{
    /* FIXME */

    myself_global = tid;
}

/**
 * set the mutex thread handle in a UTCB
 * @param handle   mutex thread handle
 */
INLINE void tcb_t::set_mutex_thread_handle(threadid_t handle)
{
    get_utcb()->mutex_thread_handle = handle;
}

INLINE void tcb_t::return_from_ipc (void)
{
    extern char _mips_l4sysipc_return[];
    ACTIVATE_CONTINUATION(&_mips_l4sysipc_return);
    // NOT REACHED
}

/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption (void)
{
    extern char _mips_restore_user[];
    ACTIVATE_CONTINUATION(&_mips_restore_user);
    // NOT REACHED
}

/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/


#define mips64_initial_switch_to(d_stack, d_asid, d_space, d_cont)      \
__asm__ __volatile__ (          \
    "dmtc0   %[asid], "STR(CP0_ENTRYHI)"        \n\t" /* Set new ASID */                \
    "move    $29, %[stack]                      \n\t" /* Install the new stack */       \
    ".set noat;                                 \n\t" \
    "dsll    $1, %[space], 32                   \n\t" \
    "dmtc0   $1, "STR(CP0_CONTEXT)"             \n\t" /* Save current Page Table */     \
    ".set at;                                   \n\t" \
    \
    "jr      %[cont]                            \n\t" \
    "0:                                         \n\t" /* Return Address */   \
    : : [stack] "r" (d_stack),                  \
        [asid]  "r" (d_asid),                   \
        [space] "r" (d_space),                  \
        [cont] "r" (d_cont):                    \
        "$1", "$31"                             \
    );  \
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );      \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );       \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

#define mips32_initial_switch_to(d_stack, d_asid, d_space, d_cont)      \
__asm__ __volatile__ (          \
    "mtc0    %[asid], "STR(CP0_ENTRYHI)"        \n\t" /* Set new ASID */                \
    "move    $29, %[stack]                      \n\t" /* Install the new stack */       \
    "sw      %[space], 0(%[saved_pgtable])      \n\t" /* Save current Page Table */     \
    \
    "jr      %[cont]                            \n\t" \
    "0:                                         \n\t" /* Return Address */   \
    : : [stack] "r" (d_stack),                  \
        [asid]  "r" (d_asid),                   \
        [space] "r" (d_space),                  \
        [saved_pgtable] "r" (&K_SAVED_PGTABLE), \
        [cont] "r" (d_cont):                    \
        "$1", "$31"                             \
    );  \
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );      \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );       \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

#if defined(L4_32BIT)
extern space_t* K_SAVED_PGTABLE;
#define mips_initial_switch_to  mips32_initial_switch_to
#elif defined(L4_64BIT)
#define mips_initial_switch_to  mips64_initial_switch_to
#endif

extern stack_t __stack;
/**
 * switch to initial thread
 * @param tcb TCB of initial thread
 *
 * Initializes context of initial thread and switches to it.  The
 * context (e.g., instruction pointer) has been generated by inserting
 * a notify procedure context on the stack.  We simply restore this
 * context.
 */
INLINE void NORETURN initial_switch_to (tcb_t * tcb)
{
    word_t stack_top = (((word_t)((&__stack)+1))-sizeof(word_t));
    hw_asid_t new_asid = get_kernel_space()->get_asid()->get(get_kernel_space());

    cache_t::flush_cache_all();
    K_CURRENT_TCB = (word_t)tcb;
    K_CURRENT_CLIST = (word_t)get_kernel_space()->get_clist();

    mips_initial_switch_to(stack_top, new_asid, (word_t)tcb->get_space(), tcb->cont);

    ASSERT(ALWAYS, !"We shouldn't get here!");
    while(true) {}
}

#define mips64_switch_to(d_asid, d_space, d_cont)       \
__asm__ __volatile__ (              \
    "dmtc0      %[asid], "STR(CP0_ENTRYHI)"     \n\t" /* Set new ASID */\
    "ori        $29, $29, %[st]                 \n\t"   \
    ".set noat;                                 \n\t"   \
    "dsll       $1, %[space], 32                \n\t"   \
    "dmtc0      $1, "STR(CP0_CONTEXT)"          \n\t" /* Save current Page Table */\
    ".set at;                                   \n\t"   \
    "jr         %[cont]                         \n\t"   \
    : : [asid]  "r" (d_asid),   \
        [space] "r" (d_space),  \
        [cont] "r" (d_cont),    \
        [st] "i" (STACK_TOP):   \
        "$1", "$31", "memory"   \
    );  \
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );          \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );      \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );           \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

#define mips32_switch_to(d_asid, d_space, d_cont)       \
__asm__ __volatile__ (              \
    "mtc0       %[asid], "STR(CP0_ENTRYHI)"     \n\t" /* Set new ASID */\
    "ori        $29, $29, %[st]                 \n\t"   \
    "sw         %[space], 0(%[saved_pgtable])   \n\t"   /* Save current Page Table */ \
    "jr         %[cont]                         \n\t"   \
    : : [asid]  "r" (d_asid),   \
        [space] "r" (d_space),  \
        [cont] "r" (d_cont),    \
        [saved_pgtable] "r" (&K_SAVED_PGTABLE), \
        [st] "i" (STACK_TOP):   \
        "$1", "$31", "memory"   \
    );  \
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );      \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );       \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

#if defined(L4_32BIT)
#define mips_switch_to  mips32_switch_to
#elif defined(L4_64BIT)
#define mips_switch_to  mips64_switch_to
#endif

/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    /* Clear whole context */
    for (word_t * t = (word_t *) &(arch.context); t < ((word_t *)(&arch.context+1)); t++)
        *t = 0;

    //TRACEF("[%p] stack = %p\n", this, stack);
}

/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level instruction pointer
 * @return      the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    mips_irq_context_t * context = &arch.context;
    return (addr_t) (context)->epc;
}

/**
 * read the user-level stack pointer
 * @return      the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    mips_irq_context_t * context = &arch.context;
    return (addr_t) (context)->sp;
}


/**
 * set the user-level instruction pointer
 * @param ip    new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    mips_irq_context_t * context =&arch.context;
    context->epc = (word_t)ip;
}

/**
 * set the user-level stack pointer
 * @param sp    new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    mips_irq_context_t * context = &arch.context;
    context->sp = (word_t)sp;
}

INLINE word_t tcb_t::get_utcb_location()
{
    return utcb_location;
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
    mips_irq_context_t * context = &arch.context;
    return (context->status & USER_FLAG_READ_MASK);
}

/**
 * set the user-level flags
 * @param flags new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    mips_irq_context_t * context = &arch.context;
    context->status = (context->status & ~USER_FLAG_WRITE_MASK) |
                      (flags & USER_FLAG_WRITE_MASK);
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
 * @return IPC message tag (MR0)
 */

void do_ipc_helper(void);

INLINE void tcb_t::do_ipc (threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
#ifdef CONFIG_IPC_FASTPATH
    tcb_t *current = get_current_tcb();
    // For fast path, we need to indicate that we are doing ipc from the kernel
    current->resources.set_kernel_ipc( current );
#endif

    TCB_SYSDATA_IPC(this)->do_ipc_continuation = continuation;

    register word_t to          ASM_REG("$4") = to_tid.get_raw();   /* a0 */
    register word_t from        ASM_REG("$5") = from_tid.get_raw(); /* a1 */

    __asm__ __volatile__ (
        "       .set    push                    \n"
        "       .set    noreorder               \n"
        "       jr      %[ipc]                  \n"
        "       move    $31, %[cont]            \n"     /* delay slot */
        "       .set    pop                     \n"
        :: "r" (to), "r" (from), [cont] "r" ((word_t)do_ipc_helper),
           [ipc] "r" (sys_ipc)
    );
    while (1);
}

INLINE void SYS_IPC_RESTART(threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
    register word_t to          ASM_REG("$4") = to_tid.get_raw();   /* a0 */
    register word_t from        ASM_REG("$5") = from_tid.get_raw(); /* a1 */

    __asm__ __volatile__ (
        "       .set    push                    \n"
        "       .set    noreorder               \n"
        "       jr      %[ipc]                  \n"
        "       move    $31, %[cont]            \n"     /* delay slot */
        "       .set    pop                     \n"
        :: "r" (to), "r" (from), [cont] "r" (continuation),
           [ipc] "r" (sys_ipc)
    );
    while (1);
}

#endif /* !__MIPS4__TCB_H__ */
