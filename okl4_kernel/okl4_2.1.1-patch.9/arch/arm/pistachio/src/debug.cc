/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   KDebug helper functionality
 */

#include <l4.h>
#include <debug.h>
#include <arch/syscalls.h>
#include <arch/thread.h>
#include <context_layout.h>

extern "C" void arm_misc_l4_syscall(arm_irq_context_t *context);

#if PT_SIZE != 72
#error fix asm
#endif

#if defined(CONFIG_DEBUG)

#if defined(__GNUC__)
__asm__ (
    "   .global     enter_kdebug                ;"
    "enter_kdebug:                              ;"
    "   sub         sp,     sp,     #72         ;"
    "   stmib       sp,     {r0-r14}            ;"
    "   mrs         r2,     cpsr                ;"
    "   mov         r1,     sp                  ;"
    "   b           enter_kdebug_c              "
);

#elif defined(__RVCT_GNU__)
extern "C" __asm void enter_kdebug(char *s)
{
    preserve8
    import  enter_kdebug_c
    sub     sp,     sp,     #72
    stmib   sp,     {r0-r14}
    mrs     r2,     cpsr
    mov     r1,     sp
    ldr     r4,     =enter_kdebug_c
    bx      r4
}

extern "C" __asm void exit_kdebug(word_t ret, arm_irq_context_t *context)
{
    preserve8
    mov     lr,     r0
    ldmib   r1,     {r0-r13}
    add     sp,     sp,     #72
    bx      lr
}
#endif

extern "C" void enter_kdebug_c(char *s, arm_irq_context_t *context, word_t cpsr)
{
    context->pc = (u32_t)__return_address();
    context->cpsr = (u32_t)cpsr;

    printf(TXT_BRIGHT "--- KD# %s  ---\n" TXT_NORMAL, s);

    if (EXPECT_FALSE(kdebug_entries.kdebug_entry != NULL))
        kdebug_entries.kdebug_entry(context);
    else
        while (1);

#if defined(__GNUC__)
    /* Return */
    __asm__ __volatile__ (
        "   mov     lr,    %0                   ;"
        "   ldmib   %1,     {r0-r13}            ;"
        "   add     sp,     sp,     #72         ;"
        "   mov     pc,     lr                  "
        :: "r" (context->lr), "r" (context)
    );
#elif defined(__RVCT_GNU__)
    exit_kdebug(context->lr, context);
#endif

    while (1);
}

#endif
