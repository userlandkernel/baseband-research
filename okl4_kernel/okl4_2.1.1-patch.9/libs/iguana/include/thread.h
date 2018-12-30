/*
 * Copyright (c) 2004, National ICT Australia
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

#ifndef _LIBIGUANA_THREAD_H_
#define _LIBIGUANA_THREAD_H_

#include <l4/types.h>
#include <iguana/types.h>

/**
 * @brief Return the L4 global thread ID for this Iguana thread.
 *
 * @param[in] server The Iguana thread for which an L4 global ID is desired
 *
 * \return The L4 global thread ID for this thread
*/
L4_ThreadId_t thread_l4tid(thread_ref_t server);

/**
 * @brief Retrieve the Iguana thread reference of the current thread
 *
 * \return The Iguana thread reference of the current thread
*/
thread_ref_t thread_myself(void);

/**
 * @brief Create a new thread in the current PD
 *
 * A new Iguana thread is created in the current protection domain. The
 * thread is initially created inactive; use L4's ExchangeRegisters()
 * function, or thread_start(), below, to activate it.
 *
 * @param[out] thrd The L4 global thread Id of the new thread is stored here
 *
 * \return the Iguana thread reference for the new thread
*/
thread_ref_t thread_create(L4_ThreadId_t *thrd);

/**
 * @brief Create a new thread in the current PD and assign it a non-default priority.
 *
 * @param[in] priority An integer between 1 (lowest) and 255 (highest) specifying the priority
 * @param[out] *thrd The L4 global thread Id of the new thread is stored here
 *
 * \return the Iguana thread reference for the new thread
*/
thread_ref_t thread_create_priority(int priority, L4_ThreadId_t *thrd);

thread_ref_t thread_create_simple(void (*fn) (void *), void *arg, int priority);

#if defined(CONFIG_SMT)
#if defined(CONFIG_SMT_BITMASK)
thread_ref_t thread_create_smt(void (*fn) (void *), void * arg, int priority, L4_Word_t);
#else
static inline thread_ref_t
thread_create_smt(void (*fn) (void *), void * arg, int priority, L4_Word_t hw_thread_bitmask)
{
    (void)hw_thread_bitmask;
    return thread_create_simple(fn, arg, priority);
}
#endif
#endif

/**
 * @brief Activate an inactive thread
 *
 * Make an inactive thread schedulable. The thread will start executing
 * at the supplied instruction pointer.
 *
 * @param[in] thread the Iguana thread reference to the thread to activate
 * @param[in] ip the initial IP address of the thread
 * @param[in] sp the initial stack pointer of the thread
*/
void thread_start(thread_ref_t thread, uintptr_t ip, uintptr_t sp);

/**
 * @brief Delete a thread.
 *
 * @param[in] thrd the L4 thread Id of the thread to delete
*/
void thread_delete(L4_ThreadId_t thrd);

void thread_delete_self(void);

/**
 * @brief Return the Iguana thread reference for a given L4 global thread Id.
 *
 * @param[in] thrd the L4 thread Id
 *
 * \return the Iguana thread reference for "thrd"
*/
thread_ref_t thread_id(L4_ThreadId_t thrd);

void thread_set_exception(thread_ref_t thread, uintptr_t exc, uintptr_t sp,
                          uintptr_t ip);

/** @brief the cached value of L4_Pager
 *
 */
extern L4_ThreadId_t __iguana_pager;

#define IGUANA_PAGER __iguana_pager

/** @brief get the value of the iguana pager and cache it to
 * save the overhead of a a system call each time libiguana needs
 * to know the tid of the iguana root server
 *
 *\@param Pointer to the Iguana Object Environment.
 *
 * \return void
 */
void __lib_iguana_init(uintptr_t *obj_env);
void __lib_iguana_init_env(uintptr_t *obj_env);
void __lib_iguana_init_pager(void);
void __lib_iguana_thread_init(uintptr_t *obj_env);
void __lib_iguana_library_ready(void);


#endif /* _LIBIGUANA_THREAD_H_ */
