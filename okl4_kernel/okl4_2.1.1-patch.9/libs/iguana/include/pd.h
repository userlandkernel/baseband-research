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
#ifndef _IGUANA_PD_H_
#define _IGUANA_PD_H_

#include <l4/types.h>
#include <iguana/types.h>

extern struct okl4_mutex __pd_ext_lock;
extern L4_SpaceId_t __pd_ext_space;
extern L4_SpaceId_t __pd_ext_return_space;
extern void * __pd_ext_utcb;
extern L4_ThreadId_t __pd_ext_thread;

/**
 * @brief Return a reference to the current protection domain.
 *
*/
pd_ref_t pd_myself(void);

/** @brief Create a new PD owned by the current PD.
 *
 * \return A reference to the new PD
*/
pd_ref_t pd_create(void);

/** @brief Create a new PD owned by the specified PD.
 *
 * @param[in] pd The owning PD
 *
 * \return A reference to the new PD
*/
pd_ref_t pd_create_pd(pd_ref_t pd);

/**
 * @brief Delete protection domain. 
 *
 * @param[in] pd The protection domain to delete
 *
 * \return If called on your own protection domain this call will not return
 */
void pd_delete(pd_ref_t pd);

/**
 * @brief Create a new thread in the specified protection domain.
 *
 * A new thread is created in the supplied PD. The L4 thread ID of the
 * new thread is stored in the thrd param, and the Iguana thread ID
 * is returned.
 *
 * @param[in] pd The containing protection domain
 * @param[out] *thrd The L4 thread ID of the new thread
 *
 * \return A reference to the new Iguana thread ID
*/
thread_ref_t pd_create_thread(pd_ref_t pd, L4_ThreadId_t *thrd);

/**
 * @brief Create a new thread in the specified protection domain with the
 * specified priority.
 *
 * A new thread is created in the supplied PD. The L4 thread ID of the
 * new thread is stored in the thrd param, and the Iguana thread ID
 * is returned. 
 *
 * @param[in] pd The containing protection domain
 * @param[in] priority The thread's initial priority, from 1 to 255.
 * @param[out] *thrd The L4 thread ID of the new thread
 *
 * \return A reference to the new Iguana thread ID
*/
thread_ref_t pd_create_thread_with_priority(pd_ref_t pd, int priority,
                                            L4_ThreadId_t *thrd);

/**
 * @brief Create a new memory section in the specified protection domain
 *
 * @param[in] pd The containing protection domain
 * @param[in] size The size of the new memory section in bytes
 * @param[out] *base The base address of the new memory section
 *
 * \return A reference to the new memory section
*/
memsection_ref_t pd_create_memsection(pd_ref_t pd, uintptr_t size,
                                      uintptr_t *base);

memsection_ref_t pd_create_memsection_user(pd_ref_t pd, uintptr_t size,
                                           uintptr_t *base);
memsection_ref_t pd_create_memsection_fixed_user(pd_ref_t pd, uintptr_t size,
                                                 uintptr_t base);

memsection_ref_t _pd_create_memsection(pd_ref_t pd, uintptr_t size,
                                       uintptr_t base, int flags,
                                       physpool_ref_t physpool,
                                       virtpool_ref_t virtpool,
                                       uintptr_t *ret_base);

/**
 * @brief Setup an async communications channel for messages.
 *
 * @param[in] pd The containing protection domain
 * @param[in] callback A reference to the callback buffer.
*/
void pd_set_callback(pd_ref_t pd, memsection_ref_t callback);

/**
 * @brief Add a clist to this protection domain
 *
 * @param[in] pd The protection domain to add the clist to
 * @param[in] clist The new clist to add
*/
uintptr_t pd_add_clist(pd_ref_t pd, memsection_ref_t clist);

/**
 * @brief Release a clist from this protection domain
 *
 * @param[in] pd The protection domain to add the clist to
 * @param[in] clist The new clist to add
*/
void pd_release_clist(pd_ref_t pd, uintptr_t clist, int slot);

/**
 * @brief Attach a memsection to a protection domain.
 *
 * @param[in] pd The protection domain to attach to
 * @param[in] ms The memsection to attach.
 * @param[in] rwx The access rights to attach with
*/
int pd_attach(pd_ref_t pd, memsection_ref_t ms, int rwx);

/**
 * @brief Detach a memsection from a protection domain.
 *
 * @param[in] pd The protection domain to detach from
 * @param[in] ms The memsection to detach.
*/
void pd_detach(pd_ref_t pd, memsection_ref_t ms);

int pd_attach_zone(pd_ref_t pd, zone_ref_t zone, int rwx);
void pd_detach_zone(pd_ref_t pd, zone_ref_t zone);

/** @brief Enable extension memsections of the calling protection domain.
 * 
 *  @return Completion status.
 *  @retval 0 Successful completion.
 */
int pd_extension_activate(void);

/** @brief Disable extension memsections of the calling protection domain.
 *
 *  @return Completion status.
 *  @retval 0 Successful completion.
 */
int pd_extension_deactivate(void);


/**
 * XXX: add header.
 */
L4_SpaceId_t pd_l4id(pd_ref_t pd);
L4_SpaceId_t pd_ext_l4id(pd_ref_t pd);
void * pd_ext_l4utcb(pd_ref_t pd);

#endif /*_IGUANA_PD_H_*/
