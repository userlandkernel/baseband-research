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
 * Copyright (c) 2003-2005, National ICT Australia (NICTA)
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
 * Description:   Various asm definitions for arm
 */
#include <l4.h>
#include <mkasmsym.h>

#include <tcb.h>
#include <schedule.h>
#include <space.h>
#include <resources.h>

MKASMSYM_START  /* Do not remove */

MKASMSYM( TSTATE_RUNNING, (word_t) thread_state_t::running );
MKASMSYM( TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever );
MKASMSYM( TSTATE_WAITING_NOTIFY, (word_t) thread_state_t::waiting_notify );
MKASMSYM( TSTATE_POLLING, (word_t) thread_state_t::polling );
MKASMSYM( TSTATE_ABORTED, (word_t) thread_state_t::aborted );

#define cpp_offsetof(type, field) ((unsigned) &(((type *) 4)->field) - 4)

#if CONFIG_ARM_VER == 5
MKASMSYM( OFS_SPACE_DOMAIN, cpp_offsetof(space_t, domain) );
MKASMSYM( OFS_SPACE_DOMAIN_MASK, cpp_offsetof(space_t, domain_mask) );
MKASMSYM( OFS_SPACE_PID, cpp_offsetof(space_t, pid_vspace_raw) );
#elif CONFIG_ARM_VER >= 6
MKASMSYM( OFS_SPACE_PDIR, cpp_offsetof(space_t, pdir) );
MKASMSYM( OFS_SPACE_ASID, cpp_offsetof(space_t, asid) );
#endif
MKASMSYM( OFS_SPACE_CLIST, cpp_offsetof(space_t, clist) );

MKASMSYM( OFS_UTCB_ERROR_CODE, cpp_offsetof(utcb_t, error_code));
MKASMSYM( OFS_UTCB_MR0, cpp_offsetof(utcb_t, mr));
MKASMSYM( OFS_UTCB_ACCEPTOR, cpp_offsetof(utcb_t, acceptor));
MKASMSYM( OFS_UTCB_NOTIFY_BITS, cpp_offsetof(utcb_t, notify_bits));
MKASMSYM( OFS_UTCB_NOTIFY_MASK, cpp_offsetof(utcb_t, notify_mask));
MKASMSYM( OFS_UTCB_THREAD_WORD, cpp_offsetof(utcb_t, thread_word));
MKASMSYM( OFS_UTCB_SENDER_SPACE, cpp_offsetof(utcb_t, sender_space));

MKASMSYM( IPC_ERROR_NOT_ACCEPTED, IPC_SND_ERROR(ERR_IPC_NOT_ACCEPTED) );
MKASMSYM( IPC_ERROR_TAG, IPC_MR0_ERROR );

MKASMSYM( KIPC_RESOURCE_BIT, (word_t) (1ul << KIPC));
MKASMSYM( EXCEPTIONFP_RESOURCE_BIT, (word_t) (1ul << EXCEPTIONFP));
MKASMSYM( OFS_TCB_ARCH_CONTEXT, cpp_offsetof(tcb_t, arch.context));
MKASMSYM( OFS_TCB_ARCH_EXC_NUM, cpp_offsetof(tcb_t, arch.exc_num));

MKASMSYM( OFS_SCHED_INDEX_BITMAP, cpp_offsetof(scheduler_t, prio_queue.index_bitmap));
MKASMSYM( OFS_SCHED_PRIO_BITMAP, cpp_offsetof(scheduler_t, prio_queue.prio_bitmap));

/* Syncpoints */
MKASMSYM( OFS_SYNCPOINT_DONATEE,      cpp_offsetof(syncpoint_t, donatee));
MKASMSYM( OFS_SYNCPOINT_BLOCKED_HEAD, cpp_offsetof(syncpoint_t, blocked_head));

/* Endpoints */
MKASMSYM( OFS_ENDPOINT_SEND_QUEUE,    cpp_offsetof(endpoint_t, send_queue));
#if defined(CONFIG_SCHEDULE_INHERITANCE)
MKASMSYM( OFS_ENDPOINT_RECV_QUEUE,    cpp_offsetof(endpoint_t, recv_queue));
#endif

/* Mutexes */
MKASMSYM( OFS_MUTEX_SYNC_POINT,       cpp_offsetof(mutex_t, sync_point));
MKASMSYM( OFS_MUTEX_HELD_LIST,        cpp_offsetof(mutex_t, held_list));

/* Ringlists */
MKASMSYM( OFS_RINGLIST_NEXT,          cpp_offsetof(ringlist_t<tcb_t>, next));
MKASMSYM( OFS_RINGLIST_PREV,          cpp_offsetof(ringlist_t<tcb_t>, prev));

/* Capabilities */
MKASMSYM( OFS_CLIST_MAX_ID,           cpp_offsetof(clist_t, max_id));
MKASMSYM( OFS_CLIST_ENTRIES,          cpp_offsetof(clist_t, entries));
MKASMSYM( OFS_CAP_RAW,                cpp_offsetof(cap_t, raw));
MKASMSYM( LOG2_SIZEOF_CAP_T,          (word_t) 3); /* log2(sizeof(cap_t)) */

#undef current_domain
#undef current_domain_mask
#undef domain_dirty

MKASMSYM( OFS_GLOBAL_KSPACE,           cpp_offsetof(arm_globals_t, kernel_space));
MKASMSYM( OFS_GLOBAL_CURRENT_TCB,      cpp_offsetof(arm_globals_t, current_tcb));
MKASMSYM( OFS_GLOBAL_CURRENT_SCHEDULE, cpp_offsetof(arm_globals_t, current_schedule));
MKASMSYM( OFS_GLOBAL_CURRENT_CLIST,    cpp_offsetof(arm_globals_t, current_clist));
MKASMSYM( OFS_GLOBAL_RAM_PHYS,         cpp_offsetof(arm_globals_t, phys_addr_ram));
#if defined(CONFIG_ENABLE_FASS)
MKASMSYM( OFS_ARM_CPD,                 cpp_offsetof(arm_globals_t, cpd));
MKASMSYM( OFS_ARM_CURRENT_DOMAIN,      cpp_offsetof(arm_globals_t, current_domain));
MKASMSYM( OFS_ARM_CURRENT_DOMAIN_MASK, cpp_offsetof(arm_globals_t, current_domain_mask));
MKASMSYM( OFS_ARM_DOMAIN_DIRTY,        cpp_offsetof(arm_globals_t, domain_dirty));
#endif
MKASMSYM( ASM_KTCB_SIZE,            (word_t)KTCB_SIZE);
MKASMSYM( ASM_INVALID_CAP_RAW,      (word_t)INVALID_RAW);

MKASMSYM( ASM_MYSELF_RAW,           (word_t) MYSELF_RAW);

#if defined(CONFIG_TRACEBUFFER)
MKASMSYM( TBUF_LOGMASK,     cpp_offsetof(trace_buffer_t, log_mask ));
MKASMSYM( TBUF_ACTIVEBUF,   cpp_offsetof(trace_buffer_t, active_buffer));
MKASMSYM( TBUF_BUFSIZE,     cpp_offsetof(trace_buffer_t, buffer_size));
MKASMSYM( TBUF_BUFHEAD0,    cpp_offsetof(trace_buffer_t, buffer_head[0]));
MKASMSYM( TBUF_BUFHEAD1,    cpp_offsetof(trace_buffer_t, buffer_head[1]));
MKASMSYM( TBUF_BUFOFF0,     cpp_offsetof(trace_buffer_t, buffer_offset[0]));
MKASMSYM( TBUF_BUFOFF1,     cpp_offsetof(trace_buffer_t, buffer_offset[1]));
#endif

#undef cpp_offsetof

MKASMSYM_END    /* Do not remove */
