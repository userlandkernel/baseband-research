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

#include <l4.h>
#include <traceids.h>

#if defined(CONFIG_TRACEPOINTS) || \
    (defined(CONFIG_TRACEBUFFER) && defined(CONFIG_KDB_CLI))

/* This structure must correspond with enum trace_ids */

char * trace_names[] = {
    "FASTPATH_IPC",             /* 0 */
    "EVENT",
    0, 0, 0,
    "INTERRUPT",                /* 5 */
    0, 0, 0, 0,
    "SYSCALL_IPC",              /* 10 */
    "SYSCALL_EXCHANGE_REGISTERS",
    "SYSCALL_SPACE_CONTROL",
    "SYSCALL_THREAD_CONTROL",
    "SYSCALL_THREAD_SWITCH",
    "SYSCALL_SCHEDULE",
    "SYSCALL_MAP_CONTROL",
    0, 0,
    "SYSCALL_CACHE_CONTROL",
    "SYSCALL_SECURITY_CONTROL",
    "SYSCALL_PLATFORM_CONTROL",
    "SYSCALL_SPACE_SWITCH",
    "SYSCALL_MUTEX",
    "SYSCALL_MUTEX_CONTROL",
    "SYSCALL_INTERRUPT_CONTROL",
    "SYSCALL_CAP_CONTROL",
    "SYSCALL_MEMORY_COPY",
    0, 0,
    "IPC_TRANSFER",             /* 30 */
    "IPC_MESSAGE_OVERFLOW",
    0, 0, 0,
    "TIMESLICE_EXPIRED",        /* 35 */
    0,
    "PREEMPTION_FAULT",
    "PREEMPTION_SIGNALED",
    "DEADLOCK_DETECTED",
    "MAP_FPAGE",                /* 40 */
    "FPAGE_OVERMAP",
    "FPAGE_UNMAP",
    "FPAGE_READ",
    0,
    0, 0, 0, 0, 0,
    "UNWIND",                   /* 50 */
    "SYSCALL_THREAD_CONTROL_IRQ",
    0, 0, 0,
    "PAGEFAULT_USER",           /* 55 */
    "PAGEFAULT_KERNEL",
    0, 0, 0,
    "KMEM_ALLOC",               /* 60 */
    "KMEM_FREE",
    0, 0, 0, 0,
    0, 0, 0, 0
    /* 70 */
#if defined(ARCH_TRACEIDS)
    , ARCH_TRACE_STRINGS
#endif
};

#endif
