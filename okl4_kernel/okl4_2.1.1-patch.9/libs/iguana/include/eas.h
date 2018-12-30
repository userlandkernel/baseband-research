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

#ifndef _LIBIGUANA_EAS_H_
#define _LIBIGUANA_EAS_H_

#include <l4/types.h>
#include <iguana/types.h>

/**
 * @brief Create an external address space
*/
#ifndef ARM_PID_RELOC
eas_ref_t eas_create(L4_Fpage_t utcb, L4_SpaceId_t *l4_id);
#else
eas_ref_t eas_create(L4_Fpage_t utcb, int pid, L4_SpaceId_t *l4_id);
#endif

/**
 * @brief Delete an external address space
 *
 * @param[in] eas the EAS to delete.
*/
void eas_delete(eas_ref_t eas);

/**
 * @brief Create a new thread in the external address space. 
 *
 * @param[in] eas The external address space
 * @param[in] pager The new thread's pager
 * @param[in] scheduler The new thread's scheduler
 * @param[out] handle_rv The new thread's handle
*/
L4_ThreadId_t eas_create_thread(eas_ref_t eas, L4_ThreadId_t pager,
                                L4_ThreadId_t scheduler, void *utcb, L4_ThreadId_t *handle_rv);
void eas_delete_thread(eas_ref_t eas, L4_ThreadId_t thread);

/**
 * @brief Set up mapping in target external address space
 *
 * @param[in] eas The external address space
 * @param[in] src_fpage The source fpage
 * @param[in] dst_addr The destination base address
 * @param[in] attributes The attributes to apply to the mapping
 *
 * \return success of operation
*/
int eas_map(eas_ref_t eas, L4_Fpage_t src_fpage, uintptr_t dst_addr,
            uintptr_t attributes);

/**
 * @brief Unmap region of target external address space
 *
 * @param[in] eas The external address space
 * @param[in] dst_fpage The target fpage
*/
void eas_unmap(eas_ref_t eas, L4_Fpage_t dst_fpage);

#if defined(ARM_SHARED_DOMAINS)
/**
 * @brief Share region of external address space in caller space
 *
 * @param[in] eas The external address space
*/
int eas_share_domain(eas_ref_t eas);
/**
 * @brief Unshare region of external address space in caller space
 *
 * @param[in] eas The external address space
*/
void eas_unshare_domain(eas_ref_t eas);
#endif

#if defined(ARM_PID_RELOC)
/**
 * @brief Modify resources of an eas
 *
 * @param[in] eas The external address space
 * @param[in] pid The ARM PID value
*/
uintptr_t eas_modify(eas_ref_t eas, int pid);
#endif

#endif /* _LIBIGUANA_EAS_H_ */
