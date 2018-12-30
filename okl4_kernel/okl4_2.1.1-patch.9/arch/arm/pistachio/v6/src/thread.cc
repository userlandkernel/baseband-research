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
 * Description:   ARMv6 thread handling functions
 */

#include <l4.h>
#include <debug.h>
#include <tcb.h>
#include <schedule.h>
#include <arch/thread.h>
#include <arch/schedule.h>

#if defined(__GNUC__)
#define asm_switch_to(cont, dest)                                           \
    do {                                                                    \
        __asm__ __volatile__ (                                              \
            "   orr     sp,     sp,     %[st]           \n"                 \
            "   str     %[tcb], [%[glob], %[cur_tcb]]   \n"                 \
            "   mov     pc,     %0                      \n"                 \
            "1:                                         \n"                 \
            :                                                               \
            : "r" (cont), [tcb] "r" (dest),                                 \
              [st] "i" (STACK_TOP),                                         \
              [glob] "r" (get_arm_globals()),                               \
              [cur_tcb] "i" (offsetof(arm_globals_t, current_tcb))          \
            : "r12", "memory"                                               \
        );                                                                  \
        __asm__ __volatile__ ("" ::: "r0","r1","r2","r3","r6","r7" );       \
        __asm__ __volatile__ ("" ::: "r8","r9","r10","r11","memory" );      \
        while(1);                                                           \
    } while (0)
#elif defined(__RVCT_GNU__)
#include <asmsyms.h>

NORETURN __asm void asm_switch_to(addr_t cont, word_t dest)
{
    orr     sp,     sp,    #STACK_TOP_ASM
    mov     r12,    #ARM_GLOBAL_BASE
    str     r1,     [r12, #OFS_GLOBAL_CURRENT_TCB]
    bx      r0
}
#elif defined(_lint)
void asm_switch_to(addr_t cont, word_t dest, word_t utcb_location);
#else
#error "Unknown compiler"
#endif

#include <kdb/tracepoints.h>
#include <cpu/syscon.h>


DECLARE_TRACEPOINT(FASTPATH_ASYNC_IPC);

void switch_from(tcb_t * current, continuation_t continuation)
{
    /* Save any resources held by the current thread. */
    if (EXPECT_FALSE(current->resource_bits)) {
        current->resources.save(current);
    }

    /* Save the current thread's wake-up continuation. */
    current->cont = continuation;
}

void switch_to(tcb_t * dest, tcb_t * schedule)
{
    /* Update the global schedule variable. */
    set_active_schedule(schedule);

    /* Restore any resources held by the current thread. */
    if (EXPECT_FALSE(dest->resource_bits)) {
        dest->resources.load(dest);
    }

    /* Setup the destination's address space. */
    space_t *space = dest->get_space();
    if (EXPECT_FALSE(space == NULL)) {
        space = get_kernel_space();
    }
    space->activate(dest);

    asm_switch_to(dest->cont, (word_t)dest/*, dest->get_utcb_location()*/);
    while (1);
}

#include <schedule.h>

#ifdef CONFIG_IPC_FASTPATH
extern "C"
void async_fp_helper(tcb_t * to_tcb, tcb_t * current)
{
    word_t link;
    {
        register word_t r_current   ASM_REG("r9") = (word_t)current;
        register word_t r_to_tcb    ASM_REG("r2") = (word_t)to_tcb;
        register word_t r_from      ASM_REG("lr") = 0;
        /* to_tcb       = r2 */
        /* current      = r9 */
        __asm__ __volatile__ (
            "   .global async_fp_helper_asm             \n"
            "   .balign 32                              \n"
            "async_fp_helper_asm:                       \n"
            "   sub     sp,     sp,     #64             \n"
        : "+r" (r_to_tcb), "+r" (r_current), "+r" (r_from)
        );

        to_tcb = (tcb_t*)r_to_tcb;
        current = (tcb_t*)r_current;

        link = r_from;
    }

    scheduler_t * scheduler = get_current_scheduler();

    TRACEPOINT_TB(FASTPATH_ASYNC_IPC, ,
                  "async_fp %t => %t",
                  TID(current->get_global_id()),
                  TID(to_tcb->get_global_id()));

    scheduler->activate_sched(to_tcb, thread_state_t::running,
                              current, (continuation_t)link,
                              scheduler_t::sched_default);
}
#endif

