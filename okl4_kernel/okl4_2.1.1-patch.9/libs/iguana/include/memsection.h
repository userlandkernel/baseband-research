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

#ifndef _LIBIGUANA_MEMSECTION_H_
#define _LIBIGUANA_MEMSECTION_H_

#include <stdint.h>
#include <l4/types.h>
#include <iguana/types.h>
#include <stddef.h>

#define MEM_NORMAL      0x1
#define MEM_FIXED       0x2
#define MEM_DIRECT      0x4
#define MEM_UTCB        0x8
#define MEM_USER        0x10
#define MEM_INTERNAL    0x20
#define MEM_LOAD        0x40
#define MEM_PROTECTED   0x80

/**
 * @brief Create a new memory section.
 * 
 * @param[in] size The size of the memory section in bytes
 * @param[out] *base The (virtual) base address of the new memory
 *
 * \return A reference to the new memory section.
*/
memsection_ref_t memsection_create(uintptr_t size, uintptr_t *base);
memsection_ref_t memsection_create_with_pools(uintptr_t size, uintptr_t *base,
                                             physpool_ref_t physpool,
                                             virtpool_ref_t virtpool);
memsection_ref_t memsection_create_user(uintptr_t size, uintptr_t *base);
memsection_ref_t memsection_create_dma(uintptr_t size, uintptr_t *base, physmem_ref_t *pm,
                                             uintptr_t attributes);

memsection_ref_t memsection_create_direct(uintptr_t size, uintptr_t *base);

/**
 * @brief Create a new memory section at a fixed location. 
 *
 * @param[in] size The size of the memory section in bytes
 * @param[in] base The virtual base address of the new memory
 *
 * \return A reference to the new memory section.
*/
memsection_ref_t memsection_create_fixed(uintptr_t size, uintptr_t base);
memsection_ref_t memsection_create_fixed_user(uintptr_t size, uintptr_t base);

memsection_ref_t memsection_create_in_area(uintptr_t size, uintptr_t base,
                                           uintptr_t end, uintptr_t *base_out);

memsection_ref_t memsection_create_in_zone(uintptr_t size, uintptr_t *base,
                                           zone_ref_t zone);

memsection_ref_t memsection_create_direct_in_zone(uintptr_t size, uintptr_t *base,
                                                  zone_ref_t zone);

cap_t _cap_memsection_create(uintptr_t size, uintptr_t *base);

/**
 * @brief Register a server thread for this memory section.
 *
 * @param[in] memsect The memory section to register the server for
 * @param[in] server Iguana thread reference to the server thread
 *
 * \return Undefined.
*/
int memsection_register_server(memsection_ref_t memsect, thread_ref_t server);

uintptr_t memsection_virt_to_phys(uintptr_t vaddr, size_t *size);

/**
 * @brief Return the memsection and server thread for the supplied object.
 *
 * @param[in] object The object for which memsection and server information is required
 * @param[out] *server The server thread for the supplied object
 *
 * \return The memsection for the supplied object.
*/
memsection_ref_t memsection_lookup(objref_t object, thread_ref_t *server);

void memsection_delete(memsection_ref_t);
void memsection_delete_dma(memsection_ref_t memsect, physmem_ref_t pm);

void memsection_set_attributes(memsection_ref_t, uintptr_t);

/**
 * Return the base address of a given memory section
 *
 * @param[in] memsect The memory section to lookup
 *
 * \return The base address. NULL on failure
 */
void *memsection_base(memsection_ref_t memsect);

uintptr_t memsection_size(memsection_ref_t memsect);

int memsection_page_map(memsection_ref_t memsect, L4_Fpage_t from_page,
                        L4_Fpage_t to_page);
int memsection_page_unmap(memsection_ref_t memsect, L4_Fpage_t to_page);

int memsection_map(memsection_ref_t memsect, uintptr_t offset,
                   physmem_ref_t phys);

int memsection_unmap(memsection_ref_t memsect, uintptr_t offset,
                     uintptr_t size);

/**
 * @brief Request that part of an auto-memsection be backed now.
 *
 * @param[in] ms The memory section to be backed
 * @param[in] base The start of the range to be backed
 * @param[in] end The end of the range (inclusive)
 *
 * \return Error code, 0 on success.
 */
int memsection_back_range(memsection_ref_t ms, uintptr_t base, uintptr_t end);

#endif /* _LIBIGUANA_MEMSECTION_H_ */
