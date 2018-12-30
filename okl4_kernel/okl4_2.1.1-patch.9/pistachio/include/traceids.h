/*
 * Copyright (c) 2005, National ICT Australia
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
 * Description:   generic L4 trace-IDs
 */
#ifndef __GENERIC_TRACEIDS_H__
#define __GENERIC_TRACEIDS_H__

#include <kernel/arch/traceids.h>

enum trace_ids {
    EVENT                       = 1,
    STRING                      = 2,
    INTERRUPT                   = 5,

    SYSCALL_IPC                 = 10,
    SYSCALL_EXCHANGE_REGISTERS  = 11,
    SYSCALL_SPACE_CONTROL       = 12,
    SYSCALL_THREAD_CONTROL      = 13,
    SYSCALL_THREAD_SWITCH       = 14,
    SYSCALL_SCHEDULE            = 15,
    SYSCALL_MAP_CONTROL         = 16,
    SYSCALL_RESERVED17          = 17,
    SYSCALL_RESERVED18          = 18,
    SYSCALL_CACHE_CONTROL       = 19,
    SYSCALL_SECURITY_CONTROL    = 20,
    SYSCALL_PLATFORM_CONTROL    = 21,
    SYSCALL_SPACE_SWITCH        = 22,
    SYSCALL_MUTEX               = 23,
    SYSCALL_MUTEX_CONTROL       = 24,
    SYSCALL_INTERRUPT_CONTROL   = 25,
    SYSCALL_CAP_CONTROL         = 26,
    SYSCALL_MEMORY_COPY         = 27,

    IPC_TRANSFER                = 30,
    IPC_MESSAGE_OVERFLOW,

    TIMESLICE_EXPIRED           = 35,
    PREEMPTION_FAULT            = 37,
    PREEMPTION_SIGNALED,
    DEADLOCK_DETECTED,

    FPAGE_MAP                   = 40,
    FPAGE_OVERMAP,
    FPAGE_UNMAP,
    FPAGE_READ,

    UNWIND                      = 50,

    PAGEFAULT_USER              = 55,
    PAGEFAULT_KERNEL,

    KMEM_ALLOC                  = 60,
    KMEM_FREE,

    XCPU_IPC_SEND               = 65,
    XCPU_IPC_SEND_DONE,
    XCPU_IPC_RECEIVE,
    XCPU_UNWIND,

    /* IDs >= 70 - architecture defined */
#if defined(ARCH_TRACEIDS)
    ARCH_TRACEIDS
#endif
};

#if defined(CONFIG_TRACEBUFFER)
/* L4 Tracebuffer major numbers
 * XXX - Divide up into more groups
 *
 * 0 : Events
 * 1 : Info
 * 2 : Interrupts
 * 3 : IPC
 * 4 : SYSCALLS
 * 5 : Scheduling
 * 6 : Warnings
 */

enum trace_major_ids {
    EVENT_major                         = 0,
    STRING_major                        = 1,
    INTERRUPT_major                     = 2,

    SYSCALL_IPC_major                   = 3,
    SYSCALL_EXCHANGE_REGISTERS_major    = 4,
    SYSCALL_SPACE_CONTROL_major         = 4,
    SYSCALL_THREAD_CONTROL_major        = 4,
    SYSCALL_THREAD_SWITCH_major         = 4,
    SYSCALL_SCHEDULE_major              = 4,
    SYSCALL_MAP_CONTROL_major           = 4,
    SYSCALL_CACHE_CONTROL_major         = 4,
    SYSCALL_SECURITY_CONTROL_major      = 4,
    SYSCALL_SPACE_SWITCH_major          = 4,
    SYSCALL_MUTEX_major                 = 4,
    SYSCALL_MUTEX_CONTROL_major         = 4,
    SYSCALL_INTERRUPT_CONTROL_major     = 4,
    SYSCALL_CAP_CONTROL_major           = 4,
    SYSCALL_MEMORY_COPY_major           = 4,

    IPC_TRANSFER_major                  = 1,
    IPC_MESSAGE_OVERFLOW_major          = 6,

    TIMESLICE_EXPIRED_major             = 5,
    PREEMPTION_FAULT_major              = 5,
    PREEMPTION_SIGNALED_major           = 5,
    DEADLOCK_DETECTED_major             = 5,

    FPAGE_MAP_major                     = 1,
    FPAGE_OVERMAP_major                 = 1,
    FPAGE_UNMAP_major                   = 1,
    FPAGE_READ_major                    = 1,

    UNWIND_major                        = 1,

    PAGEFAULT_USER_major                = 1,
    PAGEFAULT_KERNEL_major              = 1,

    KMEM_ALLOC_major                    = 1,
    KMEM_FREE_major                     = 1,

    XCPU_IPC_SEND_major                 = 1,
    XCPU_IPC_SEND_DONE_major            = 1,
    XCPU_IPC_RECEIVE_major              = 1,
    XCPU_UNWIND_major                   = 1,

    /* IDs >= 70 - architecture defined */
#if defined(ARCH_TRACEIDS)
    ARCH_TRACE_MAJORIDS
#endif
};
#endif  /* CONFIG_TRACEBUFFER */


#if defined(CONFIG_TRACEPOINTS) || \
    (defined(CONFIG_TRACEBUFFER) && defined(CONFIG_KDB_CLI))
extern char * trace_names[];
#endif

#endif
