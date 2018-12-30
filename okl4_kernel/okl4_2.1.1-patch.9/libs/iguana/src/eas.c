#if defined(CONFIG_EAS)
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
#include <stdio.h>
#include <iguana/cap.h>
#include <iguana/eas.h>
#include <iguana/thread.h>
#include <interfaces/iguana_client.h>

eas_ref_t
#ifndef ARM_PID_RELOC
eas_create(L4_Fpage_t utcb, L4_SpaceId_t *l4_id)
#else
eas_create(L4_Fpage_t utcb, int pid, L4_SpaceId_t *l4_id)
#endif
{
    cap_t cap;
    uintptr_t mypd = iguana_pd_mypd(IGUANA_PAGER, NULL);
    L4_SpaceId_t ret_id;

#ifndef ARM_PID_RELOC
    cap = iguana_pd_create_eas(IGUANA_PAGER, mypd, &utcb, default_clist,
                               &ret_id, NULL);
#else
    cap = iguana_pd_create_eas(IGUANA_PAGER, mypd, &utcb, default_clist,
                               pid, &ret_id, NULL);
#endif

    if (l4_id)
        *l4_id = ret_id;
    return cap.ref.eas;
}

void
eas_delete(eas_ref_t eas)
{
    iguana_eas_delete(IGUANA_PAGER, eas, NULL);
}

L4_ThreadId_t
eas_create_thread(eas_ref_t eas, L4_ThreadId_t pager,
                  L4_ThreadId_t scheduler, void *utcb, L4_ThreadId_t *handle_rv)
{
    cap_t cap;
    L4_ThreadId_t thrd;
    L4_ThreadId_t handle;

    cap = iguana_eas_create_thread(IGUANA_PAGER, eas, &pager, &scheduler,
                                   (uintptr_t)utcb, default_clist, &thrd, &handle, NULL);

    if (handle_rv != NULL)
        *handle_rv = handle;

    return thrd;
}

int
eas_map(eas_ref_t eas, L4_Fpage_t src_fpage, uintptr_t dst_addr,
        uintptr_t attributes)
{
    return iguana_eas_map(IGUANA_PAGER, eas, &src_fpage, dst_addr, attributes,
                          NULL);
}

void
eas_unmap(eas_ref_t eas, L4_Fpage_t dst_fpage)
{
    iguana_eas_unmap(IGUANA_PAGER, eas, &dst_fpage, NULL);
}

#if defined(ARM_SHARED_DOMAINS)
int
eas_share_domain(eas_ref_t eas)
{
    return iguana_eas_share_domain(IGUANA_PAGER, eas, NULL);
}

void
eas_unshare_domain(eas_ref_t eas)
{
    iguana_eas_unshare_domain(IGUANA_PAGER, eas, NULL);
}
#endif

#ifdef ARM_PID_RELOC
uintptr_t
eas_modify(eas_ref_t eas, int pid)
{
    return iguana_eas_modify(IGUANA_PAGER, eas, pid, NULL);
}
#endif

#endif
