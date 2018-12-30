/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Copyright (c) 2006, National ICT Australia
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
 * Description:   defines macros for implementation of trap and
 */
#ifndef __ARCH__IA32__TRAPGATE_H__
#define __ARCH__IA32__TRAPGATE_H__

/* Trick the SCons scanner into not generating false circular dependancies */
#ifndef BUILD_TCB_LAYOUT
#define TCB_LAYOUT_H <tcb_layout.h>
#define KTCB_LAYOUT_H <ktcb_layout.h>
#include TCB_LAYOUT_H
#include KTCB_LAYOUT_H
#endif

#include <kernel/arch/context.h>


#if defined(_lint)
#define IA32_EXC_WITH_ERRORCODE(name, reason) void name##handler(ia32_exceptionframe_t * frame)
#else
#define IA32_EXC_WITH_ERRORCODE(name, reason)                   \
extern "C" void name (void);                                    \
extern "C" void name##handler(ia32_exceptionframe_t * frame);   \
void name##_wrapper()                                           \
{                                                               \
    __asm__ (                                                   \
        ".globl "#name "                \n"                     \
        #name":                         \n"                     \
        /* push gprs */                                         \
        "pusha                          \n"                     \
        /* setup arguments to common handler */                 \
        "movl   %[rsn], %%eax           \n"                     \
        "leal   "#name"handler, %%ecx   \n"                     \
        "jmp    exception_common        \n"                     \
        :                                                       \
        : [rsn] "i" (reason)                                    \
        );                                                      \
}                                                               \
void name##handler(ia32_exceptionframe_t *frame)
#endif


#if defined(_lint)
#define IA32_EXC_NO_ERRORCODE(name, reason) void name##handler(ia32_exceptionframe_t * frame)
#else
#define IA32_EXC_NO_ERRORCODE(name, reason)                     \
extern "C" void name (void);                                    \
extern "C" void name##handler(ia32_exceptionframe_t * frame);   \
void name##_wrapper()                                           \
{                                                               \
    __asm__ (                                                   \
        ".globl "#name "                \n"                     \
        #name":                         \n"                     \
        /* skip error code */                                   \
        "subl   $4, %%esp               \n"                     \
        /* push gprs */                                         \
        "pusha                          \n"                     \
        /* setup arguments to common handler */                 \
        "movl   %[rsn], %%eax           \n"                     \
        "leal   "#name"handler, %%ecx   \n"                     \
        "jmp    exception_common        \n"                     \
        :                                                       \
        : [rsn] "i" (reason)                                    \
        );                                                      \
}                                                               \
void name##handler(ia32_exceptionframe_t *frame)
#endif


/* Assumptions:
 * i)  reason code is in %eax
 * ii) handler is in %ecx
 */
#define EXCEPTION_ENTRY_COMMON()                                \
    __asm__ (                                                   \
        ".globl exception_common        \n"                     \
        "exception_common:              \n"                     \
        /* push segment registers used by kernel */             \
        "push   %%ds                    \n"                     \
        "push   %%es                    \n"                     \
        "push   %%fs                    \n"                     \
        "push   %%gs                    \n"                     \
        /* push reason */                                       \
        "push   %%eax                   \n"                     \
        /* save exception frame pointer */                      \
        "movl   %%esp, %%ebp            \n"                     \
        /* load kernel data segments */                         \
        "movl   %[kds], %%eax           \n"                     \
        "movl   %%eax, %%ds             \n"                     \
        "movl   %%eax, %%es             \n"                     \
        "movl   %%eax, %%fs             \n"                     \
        "movl   %%eax, %%gs             \n"                     \
        /* check whether we came from kernel mode */            \
        "testl  $0x3, %c[cxt_cs](%%ebp) \n"                     \
        "jz     kernel_exception        \n"                     \
        /* from user so setup kernel stack */                   \
        "movl   %[stack_top], %%esp     \n"                     \
        /* call c handler */                                    \
        "pushl  %%ebp                   \n"                     \
        PROFILE_KERNEL_TIME_START()                             \
        "call   *%%ecx                  \n"                     \
        PROFILE_KERNEL_TIME_STOP()                              \
        "jmp    ia32_return_to_user     \n"                     \
                                                                \
        "kernel_exception:              \n"                     \
        /* check whether we need to setup kernel stack */       \
        "movl   %%esp, %%edx            \n"                     \
        "orl    %[stack_mask], %%edx    \n"                     \
        "cmpl   %[stack_top], %%edx     \n"                     \
        "je     1f                      \n"                     \
        "movl   %[stack_top], %%esp     \n"                     \
        "movl   %%esp, %%edx            \n"                     \
        "1:                             \n"                     \
        /* check for preemption continuation */                 \
        "movl   (%%edx), %%edx          \n"                     \
        "movl   %c[ofs_preempt_cont](%%edx), %%eax      \n"     \
        "testl  %%eax, %%eax            \n"                     \
        "jne    continuation_exception  \n"                     \
        /* call c handler */                                    \
        "pushl  %%ebp                   \n"                     \
        PROFILE_KERNEL_TIME_START()                             \
        "call   *%%ecx                  \n"                     \
        PROFILE_KERNEL_TIME_STOP()                              \
        /* get rid of argument */                               \
        "addl   $4, %%esp               \n"                     \
        "jmp    ia32_exception_return   \n"                     \
        "                               \n"                     \
        "continuation_exception:        \n"                     \
        "orl    %[stack_mask], %%esp    \n"                     \
        /* call c handler */                                    \
        "pushl  %%ebp                   \n"                     \
        PROFILE_KERNEL_TIME_START()                             \
        "call   *%%ecx                  \n"                     \
        PROFILE_KERNEL_TIME_STOP()                              \
        /* got here through continuation */                     \
        /* get the current tcb */                               \
        "orl    %[stack_mask], %%esp    \n"                     \
        "movl   (%%esp), %%eax          \n"                     \
        /* activate continuation */                             \
        "jmp   *%c[ofs_preempt_cont](%%eax)     \n"             \
        :                                                       \
        :                                                       \
          [stack_top] "m" (stack_top),                          \
          [stack_mask] "i" (STACK_TOP),                         \
          [cxt_cs] "i" (CXT_CS),                                \
          [ofs_preempt_cont] "i" (OFS_TCB_PREEMPTION_CONTINUATION),\
          [kds] "i" (IA32_KDS)                                  \
        );

#endif /* !__ARCH__IA32__TRAPGATE_H__ */
