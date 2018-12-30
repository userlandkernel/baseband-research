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

#ifndef __TCB_STATES_H__
#define __TCB_STATES_H__

#include <kernel/l4.h>
#include <kernel/generic/lib.h>
#include <kernel/cache.h>
#include <kernel/arch/ktcb.h>

#if defined(CONFIG_DEBUG)
/* Enable sanity checking for compressed TCB continuation data. */
/** @todo FIXME: Mothra issue #2477 - awiggins. */
#define TCB_DATA_SANITY
#endif

/* Pagefault saves three registers, exceptions save 1 */
#define MAX_SAVED_MESSAGE_REGISTERS 3

class tcb_t;
class mutex_t;

/**
 * @brief encapsulate the data of the TCB related to carrying out syscalls.
 *
 * While a thread is running it can be doing, from a kernel perspective, one
 * of several mutually exclusive actions (related to syscalls).  Wrap this
 * action and it's associated data in a union to minimise space.
 */
class tcb_syscall_data_t
{
public:
    /**
     * @brief What's the current action going on in the thread
     *
     * Used for validation during testing.  However, it may be useful to 
     * leave it around for informational purposes.  Definitely if KDB 
     * is available.
     */
    enum action_e {
        action_none,
        action_schedule,
        action_exregs,
        action_ipc,
        action_cache_control,
        action_map_control,
        action_thread_control,
        action_mutex,
        action_remote_memcpy,
        action_other_user_access,
    };

    /**
     * @brief Grouping of continuations and data by state.
     *
     */
    /**
     * @brief sys_schedule
     */
    typedef struct {
        tcb_t          *dest_tcb;
        continuation_t  schedule_continuation;
    } sched_t;

    /**
     * @brief sys_exchange_registers
     */
    typedef struct {
        tcb_t          *source;
        word_t          ctrl;
        word_t          new_control;
        addr_t          sp;
        addr_t          ip;
        word_t          flags;
        threadid_t      pager;
        word_t          user_handle;
        tcb_t *         exreg_tcb;
        continuation_t  exreg_continuation;
    } exregs_t;

    /**
     * IPC in its various forms.
     */
    typedef struct {
        threadid_t      to_tid;
        threadid_t      from_tid;
        /* IPC copy */

        word_t          saved_mr[MAX_SAVED_MESSAGE_REGISTERS];
        word_t          saved_notify_mask;
        word_t          saved_error;

        continuation_t  ipc_return_continuation;
        continuation_t  ipc_restart_continuation;
        continuation_t  ipc_continuation;
        continuation_t  do_ipc_continuation; //< brought in from arch areas
    } ipc_t;

    /**
     * Cache control data for restarting after a preemption
     */
    typedef struct {
        word_t          op_index;   // current operation index
        cache_control_t ctrl;       // cache_control control word
        word_t          op_offset;  // current operation offset
        space_t *       space;

        continuation_t cache_continuation;
    } cache_ctrl_t;

    /**
     * Remote memory copy data for recovering from preemption
     * and data aborts.
     */
    typedef struct {
        /* hooks for the remote memory copy system call */
        addr_t              copy_start;

        /* data stash for memory copy system call preemption */
        addr_t              src;
        addr_t              dest;
        addr_t              orig_src;
        addr_t              orig_dest;
        word_t              size;
        word_t              orig_size;
        word_t              remote_size;

        tcb_t *             from_tcb;
        tcb_t *             to_tcb;
        continuation_t      memory_copy_cont;
    } remote_memcpy_data_t;

    /**
     * User memory access
     */
    typedef struct {
        addr_t              fault_address;
#define CONFIG_REMOTE_MEMCOPY
#if defined(CONFIG_REMOTE_MEMCOPY)
        union {
            remote_memcpy_data_t memcpy;
        };
#endif
    } user_access_t;

    // true if a thread was scheduled on last call to schedule
    bool                scheduled;

    /**
     * ThreadControl system call continuation data.
     */
    typedef struct {
        continuation_t thread_ctrl_continuation;
    } thread_ctrl_t;

    /**
     * Mutex system call continuation data.
     */
    typedef struct {
        continuation_t mutex_continuation;
        mutex_t *      mutex;
        bool           uncontended_hybrid;
        word_t *       hybrid_user_state;
    } mutex_t;

#ifdef CONFIG_MUNITS
    /* SMT notify state - only used on idle thread */
    word_t              notify_arg;
#endif

public:
#ifdef TCB_DATA_SANITY
    action_e state;
#endif

    union {
        sched_t       sched;
        exregs_t      exregs;
        ipc_t         ipc;
        cache_ctrl_t  cache;
        thread_ctrl_t thread_ctrl;
        mutex_t       mutex;
        user_access_t user_access;
    } vals;

public:
    void set_action(action_e val);

#ifdef TCB_DATA_SANITY
    sched_t *       get_sched_vals(void);
    exregs_t *      get_exregs_vals(void);
    ipc_t *         get_ipc_vals(void);
    cache_ctrl_t *  get_cache_vals(void);
    thread_ctrl_t * get_thread_ctrl_vals(void);
    mutex_t *       get_mutex_vals(void);
    user_access_t*  get_user_access_vals(void);
    remote_memcpy_data_t* get_memcpy_vals(void);
#endif
};

INLINE void
tcb_syscall_data_t::set_action(tcb_syscall_data_t::action_e val)
{
#ifdef TCB_DATA_SANITY
    if (this->state != val) {
        this->state = val;
    }
#endif
}

#ifdef TCB_DATA_SANITY

#define TCB_SYSDATA_SCHED(tcb)          tcb->sys_data.get_sched_vals()
#define TCB_SYSDATA_EXREGS(tcb)         tcb->sys_data.get_exregs_vals()
#define TCB_SYSDATA_IPC(tcb)            tcb->sys_data.get_ipc_vals()
#define TCB_SYSDATA_CACHE(tcb)          tcb->sys_data.get_cache_vals()
#define TCB_SYSDATA_THREAD_CTRL(tcb)    tcb->sys_data.get_thread_ctrl_vals()
#define TCB_SYSDATA_MUTEX(tcb)          tcb->sys_data.get_mutex_vals()
#define TCB_SYSDATA_USER_ACCESS(tcb)    tcb->sys_data.get_user_access_vals()
#define TCB_SYSDATA_MEMCPY(tcb)         tcb->sys_data.get_memcpy_vals()

#else

#define TCB_SYSDATA_SCHED(tcb)          (&tcb->sys_data.vals.sched)
#define TCB_SYSDATA_EXREGS(tcb)         (&tcb->sys_data.vals.exregs)
#define TCB_SYSDATA_IPC(tcb)            (&tcb->sys_data.vals.ipc)
#define TCB_SYSDATA_CACHE(tcb)          (&tcb->sys_data.vals.cache)
#define TCB_SYSDATA_THREAD_CTRL(tcb)    (&tcb->sys_data.vals.thread_ctrl)
#define TCB_SYSDATA_MUTEX(tcb)          (&tcb->sys_data.vals.mutex)
#define TCB_SYSDATA_USER_ACCESS(tcb)    (&tcb->sys_data.vals.user_access)
#define TCB_SYSDATA_MEMCPY(tcb)         (&tcb->sys_data.vals.user_access.memcpy)

#endif

#ifdef TCB_DATA_SANITY

INLINE tcb_syscall_data_t::sched_t *
tcb_syscall_data_t::get_sched_vals(void)
{
    ASSERT(ALWAYS, this->state == action_schedule);
    return &(this->vals.sched);
}

INLINE tcb_syscall_data_t::exregs_t *
tcb_syscall_data_t::get_exregs_vals(void)
{
    ASSERT(ALWAYS, this->state == action_exregs);
    return &(this->vals.exregs);
}

INLINE tcb_syscall_data_t::ipc_t *
tcb_syscall_data_t::get_ipc_vals(void)
{
    if (this->state != action_ipc)
        TRACEF("Value is %d, not %d\n", this->state, action_ipc);
    ASSERT(ALWAYS, this->state == action_ipc);
    return &(this->vals.ipc);
}

INLINE tcb_syscall_data_t::cache_ctrl_t *
tcb_syscall_data_t::get_cache_vals(void)
{
    ASSERT(ALWAYS, this->state == action_cache_control);
    return &(this->vals.cache);
}

INLINE tcb_syscall_data_t::thread_ctrl_t *
tcb_syscall_data_t::get_thread_ctrl_vals(void)
{
    ASSERT(ALWAYS, this->state == action_thread_control);
    return &(this->vals.thread_ctrl);
}

INLINE tcb_syscall_data_t::mutex_t *
tcb_syscall_data_t::get_mutex_vals(void)
{
    ASSERT(ALWAYS, this->state == action_mutex);
    return &(this->vals.mutex);
}

INLINE tcb_syscall_data_t::user_access_t *
tcb_syscall_data_t::get_user_access_vals(void)
{
    /*
     * This assert needs to check for any other valid uses of user_access_vals.
     * Currently remote_memcpy is the only one
     */
    ASSERT(ALWAYS, this->state == action_remote_memcpy);
    return &(this->vals.user_access);
}

INLINE tcb_syscall_data_t::remote_memcpy_data_t *
tcb_syscall_data_t::get_memcpy_vals(void)
{
    ASSERT(ALWAYS, this->state == action_remote_memcpy);
    return &(this->vals.user_access.memcpy);
}

#endif /* TCB_DATA_SANITY */

#endif /* ! __TCB_STATES_H__ */
