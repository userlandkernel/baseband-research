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
 * Description:   specific implementations
 */
#ifndef __IA32__TCB_H__
#define __IA32__TCB_H__

#include <kernel/kdb/tracepoints.h>
#include <kernel/arch/tss.h>
#include <kernel/arch/ldt.h>
#include <kernel/arch/context.h>
#include <kernel/syscalls.h>
#include <kernel/generic/lib.h>
#include <kernel/arch/resource_functions.h>
#include <arch/syscalls.h>

#define TESTIFINEXREG

#if defined(__GNUC__)

#define ACTIVATE_CONTINUATION(cont) ACTIVATE_CONTINUATION_ECX(cont)

#define ACTIVATE_CONTINUATION_RESULT(continuation, result)        \
    do {                                                \
        asm("orl %[st_mask], %%esp   \n"                \
            "jmp *%%ecx              \n"                \
            ::                                          \
            "c"((word_t)(continuation)),                \
             "a" (result),                              \
            [st_mask] "i" (STACK_TOP));                 \
        while(1);                                       \
    } while(false);

#define ACTIVATE_CONTINUATION_RESULT2(continuation, result, result2)    \
    do {                                                \
        asm("orl %[st_mask], %%esp   \n"                \
            "jmp *%%ecx              \n"                \
            ::                                          \
            "c"((word_t)(continuation)),                \
             "a" (result),                              \
             "S" (result2),                             \
            [st_mask] "i" (STACK_TOP));                 \
        while(1);                                       \
    } while(false);

#define ACTIVATE_CONTINUATION_RESULT_EXREG(continuation, result,control, sp, ip, flags, pager, handle)    \
    do {                                                \
        utcb_t *utcb= get_current_tcb()->get_utcb();    \
        utcb->syscall_args[0] = pager.get_raw();        \
        utcb->syscall_args[1] = handle;                 \
                                                        \
        asm("movl %%edx, %%ebp\n"                       \
            "orl %[st_mask], %%esp   \n"                \
            "jmp *%%ecx              \n"                \
            ::                                          \
            "c"((word_t)(continuation)),                \
             "a" (result),                              \
             "S" (control),                             \
             "D" (sp),                                  \
             "b" (ip),                                  \
             "d" (flags),                               \
            [st_mask] "i" (STACK_TOP));                 \
        while(1);                                       \
    } while(false);

#define ACTIVATE_CONTINUATION_ECX(continuation)         \
    do {                                                \
        asm("orl %[st_mask], %%esp   \n"                \
            "jmp *%%ecx              \n"                \
            ::                                          \
            "c"((word_t)(continuation)),                \
            [st_mask] "i" (STACK_TOP));                 \
        while(1);                                       \
    } while(false);

#elif defined(_lint)
#define ACTIVATE_CONTINUATION(continuation) ((continuation_t) continuation)()
#else
#define "Unknown compiler"
#endif

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

INLINE tcb_t * get_current_tcb()
{
    word_t tcb;
    asm (
        "mov    %%esp, %0 \n"
        "orl    %1, %0\n"
        "mov    (%0), %0\n"
        :"=r" (tcb): "i"(STACK_TOP)
    );

    return (tcb_t *) tcb;
}

INLINE clist_t* get_current_clist(void) PURE;

INLINE clist_t* get_current_clist(void)
{
    /* XXX not SMP safe */
    extern word_t ia32_current_clist;
    return (clist_t *) ia32_current_clist;
}

extern void * preempt_recover;
extern ia32_tss_t tss;
extern void * stack_top;

INLINE void tcb_t::enable_preempt_recover(continuation_t continuation)
{
    preemption_continuation = continuation;
}

INLINE void tcb_t::disable_preempt_recover()
{
    preemption_continuation = 0;
}

#ifdef CONFIG_SMP
INLINE cpuid_t get_current_cpu()
{
    return get_idle_tcb()->get_cpu();
}
#endif

/**
 * initial_switch_to: switch to first thread
 */
INLINE void __attribute__((noreturn)) initial_switch_to(tcb_t * tcb)
{
    extern word_t ia32_current_clist;

    *((tcb_t **) stack_top) = tcb;
    ia32_current_clist = (word_t)get_kernel_space()->get_clist();

    asm(" movl %[stack_top], %%esp    \n"
        " jmp *%[cont]      \n"
        :
        : [stack_top] "r"(stack_top),
          [cont] "r" (tcb->cont));

    /* do nothing */;
    while(1);
}

INLINE void tcb_t::set_utcb_location(word_t location)
{
    utcb_t * dummy = (utcb_t*)NULL;
    utcb_location = location + ((word_t)&dummy->mr[0]);
}

INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t * dummy = (utcb_t*)NULL;
    return utcb_location - ((word_t)&dummy->mr[0]);
}

INLINE void tcb_t::set_global_id(threadid_t tid)
{
    myself_global = tid;
}

INLINE void tcb_t::set_mutex_thread_handle(threadid_t handle)
{
    get_utcb()->mutex_thread_handle = handle;
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
 * tcb_t::get_mr: returns value of message register
 * @index: number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    ASSERT(NORMAL, index < IPC_NUM_MR);
    return get_utcb()->mr[index];
}

/**
 * tcb_t::set_mr: sets the value of a message register
 * @index: number of message register
 * @value: value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    ASSERT(NORMAL, index < IPC_NUM_MR);
    get_utcb()->mr[index] = value;
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
    ASSERT(DEBUG, start + count <= IPC_NUM_MR);
    ASSERT(DEBUG, count > 0);
    word_t dummy;

    if (start + count > IPC_NUM_MR)
        return false;


    /* use optimized IA32 copy loop -- uses complete cacheline
       transfers */
    __asm__ __volatile__ (
        "cld                   \n"
        "rep                   \n"  /* apple_cc doesn't like rep mov?! */
        "movsl (%%esi), (%%edi)\n"
        : /* output */
        "=S"(dummy), "=D"(dummy), "=c"(dummy)
        : /* input */
        "c"(count), "S"(&get_utcb()->mr[start]),
        "D"(&dest->get_utcb()->mr[start]));

    return true;
}

/*
 * Set the LDT.  This assumes that the appropriate checks
 * have already been done.
 */
INLINE word_t tcb_t::set_tls(word_t *mr)
{
    ia32_segdesc_t seg;

    seg.set_raw(mr[1], mr[2]);
    return set_ldt(this, mr[0], seg);
}

INLINE void tcb_t::copy_mrs_to_regs(tcb_t *dest)
{
    dest->arch.context.eip = get_mr(0);
    dest->set_user_flags(get_mr(1));
    dest->arch.context.edi = get_mr(2);
    dest->arch.context.esi = get_mr(3);
    dest->arch.context.ebp = get_mr(4);
    dest->arch.context.esp = get_mr(5);
    dest->arch.context.ebx = get_mr(6);
    dest->arch.context.edx = get_mr(7);
    dest->arch.context.ecx = get_mr(8);
    dest->arch.context.eax = get_mr(9);
}

INLINE void tcb_t::copy_regs_to_mrs(tcb_t *src)
{
    set_mr(0, src->arch.context.eip);
    set_mr(1, src->arch.context.eflags);
    set_mr(2, src->arch.context.edi);
    set_mr(3, src->arch.context.esi);
    set_mr(4, src->arch.context.ebp);
    set_mr(5, src->arch.context.esp);
    set_mr(6, src->arch.context.ebx);
    set_mr(7, src->arch.context.edx);
    set_mr(8, src->arch.context.ecx);
    set_mr(9, src->arch.context.eax);
}



INLINE bool tcb_t::copy_exception_mrs_from_frame(tcb_t *dest)
{
    msg_tag_t tag = get_tag();
    if (tag.get_untyped() != 12)
        return false;

    ia32_exceptionframe_t *frame = &arch.context;

    dest->set_mr(1, frame->eip);
    dest->set_mr(2, frame->eflags);
    dest->set_mr(3, arch.exc_code - 2);
    dest->set_mr(4, frame->error);
    dest->set_mr(5, frame->edi);
    dest->set_mr(6, frame->esi);
    dest->set_mr(7, frame->ebp);
    dest->set_mr(8, frame->esp);
    dest->set_mr(9, frame->ebx);
    dest->set_mr(10, frame->edx);
    dest->set_mr(11, frame->ecx);
    dest->set_mr(12, frame->eax);

    return true;
}

INLINE bool tcb_t::copy_exception_mrs_to_frame(tcb_t *dest)
{
    msg_tag_t tag = get_tag();
    if (tag.get_untyped() != 12){
        return false;
    }
    dest->arch.context.eip = get_mr(1);
    dest->set_user_flags(get_mr(2));
    /* skip exception type and error */
    dest->arch.context.edi = get_mr(5);
    dest->arch.context.esi = get_mr(6);
    dest->arch.context.ebp = get_mr(7);
    dest->arch.context.esp = get_mr(8);
    dest->arch.context.ebx = get_mr(9);
    dest->arch.context.edx = get_mr(10);
    dest->arch.context.ecx = get_mr(11);
    dest->arch.context.eax = get_mr(12);

    return true;
}

INLINE bool tcb_t::in_exception_ipc(void)
{
    return this->arch.exc_code != 0;
}

INLINE void tcb_t::clear_exception_ipc(void)
{
    this->arch.exc_code = 0;
}

INLINE void tcb_t::set_space(space_t * new_space)
{
    this->space = new_space;
    if(space) {
        this->space_id = new_space->get_space_id();
#ifdef CONFIG_MDOMAINS
        this->page_directory = space->pgent(0, context.domain);
#else
        this->page_directory = space->pgent(0);
#endif
    } else {
        this->space_id = spaceid_t::nilspace();
        this->page_directory = NULL;
    }
}

INLINE void tcb_t::init_stack()
{
    //TRACE("stack = %p\n", stack);
}



/**********************************************************************
 *
 *                        in-kernel IPC invocation
 *
 **********************************************************************/

/**
 * tcb_t::do_ipc: invokes an in-kernel IPC
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @return IPC message tag (MR0)
 */
extern void finish_do_ipc(void);

INLINE void tcb_t::do_ipc(threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
    // Use three register parameters (to_tid = EAX, from_tid = EDX).
    word_t dummy;
    TCB_SYSDATA_IPC(this)->do_ipc_continuation = continuation;

    __asm__ __volatile__(
        "push   %%ecx           \n"
#ifdef CONFIG_IPC_C_FASTPATH
        "jmp    sys_ipc_c_fastpath \n"
#else
        "jmp    sys_ipc \n"
#endif
        : "=a" (dummy),
          "=d" (dummy)
        : "a" (to_tid.get_raw()),
          "d" (from_tid.get_raw()),
          "c" (continuation)
        : "edi", "esi", "ebx", "memory"
    );
    /* returns to continuation */
    while (1);
}

NORETURN INLINE void SYS_IPC_RESTART(threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
    // Use three register parameters (to_tid = EAX, from_tid = EDX)
    word_t dummy;

    __asm__ __volatile__(
        "push   %%ecx           \n"
        "jmp    sys_ipc         \n"
        : "=a" (dummy),
          "=d" (dummy)
        : "a" (to_tid.get_raw()),
          "d" (from_tid.get_raw()),
          "c" (continuation)
        : "edi", "esi", "ebx", "memory"
    );
    /* returns to continuation */
    while (1);
}

NORETURN INLINE void SYS_IPC_SLOW(threadid_t to_tid, threadid_t from_tid, continuation_t continuation)
{
    // Use three register parameters (to_tid = EAX, from_tid = EDX)
    word_t dummy;

    __asm__ __volatile__(
        "push   %%ecx      \n"
        "jmp    sys_ipc    \n"
        : "=a" (dummy),
          "=d" (dummy)
        : "a" (to_tid.get_raw()),
        "d" (from_tid.get_raw()),
        "c" (continuation)
        : "edi", "esi", "ebx", "memory"
        );
    /* returns to continuation */
    while (1);
}

/*
 * access functions for ex-regs'able registers
 */
INLINE addr_t tcb_t::get_user_ip()
{
    return (addr_t)arch.context.eip;
}

INLINE addr_t tcb_t::get_user_sp()
{
    return (addr_t)arch.context.esp;
}

INLINE word_t tcb_t::get_user_flags()
{
    return (word_t)arch.context.eflags;
}

INLINE void tcb_t::set_user_ip(addr_t ip)
{
    arch.context.eip = (u32_t)ip;
}

INLINE void tcb_t::set_user_sp(addr_t sp)
{
    arch.context.esp = (u32_t)sp;
}

INLINE void tcb_t::set_user_flags(const word_t flags)
{
    arch.context.eflags = (get_user_flags() & (~IA32_USER_FLAGMASK)) | (flags & IA32_USER_FLAGMASK);
}

INLINE void tcb_t::copy_user_regs(tcb_t *src)
{
    ia32_exceptionframe_t *to = &arch.context;
    ia32_exceptionframe_t *from = &(src->arch.context);

    to->eax = from->eax;
    to->ebx = from->ebx;
    to->ecx = from->ecx;
    to->edx = from->edx;
    to->ebp = from->ebp;
    to->esi = from->esi;
    to->edi = from->edi;
    to->eflags = from->eflags;

    /*
     * Copy the LDT as it is a part of the segment register state.
     */
    memcpy(&arch.ldt, &src->arch.ldt, sizeof(arch.ldt));
    to->gs = from->gs;
    to->fs = from->fs;
    /* what about the other segment regs?  -gl */
}

INLINE void tcb_t::return_from_ipc (void)
{
    extern void * ia32_ipc_return;
    ACTIVATE_CONTINUATION(&ia32_ipc_return);
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    extern void * ia32_return_to_user;
    ACTIVATE_CONTINUATION(&ia32_return_to_user);
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

#endif /* __IA32__TCB_H__ */
