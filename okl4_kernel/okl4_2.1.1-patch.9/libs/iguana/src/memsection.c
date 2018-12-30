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
/*
 * Author: Ben Leslie 
 */

#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <l4/thread.h>
#include <l4e/misc.h>
#include <iguana/memsection.h>
#include <iguana/physmem.h>
#include <iguana/pd.h>
#include <iguana/physpool.h>
#include <iguana/cap.h>
#include <iguana/thread.h>
#include <iguana/virtpool.h>
#include <assert.h>
#include <interfaces/iguana_client.h>
#include <mutex/mutex.h>

cap_t
_cap_memsection_create(uintptr_t size, uintptr_t *base)
{
    cap_t cap;

    cap =
        iguana_pd_create_memsection(IGUANA_PAGER, pd_myself(), size, 0,
                                    MEM_NORMAL, default_physpool, 
                                    default_virtpool, default_clist, base, NULL);
    return cap;
}

memsection_ref_t
memsection_create(uintptr_t size, uintptr_t *base)
{
    return _pd_create_memsection(pd_myself(), size, 0, MEM_NORMAL,
                                 default_physpool, default_virtpool, base);
}

memsection_ref_t
memsection_create_with_pools(uintptr_t size, uintptr_t *base,
                            physpool_ref_t physpool, virtpool_ref_t virtpool)
{
    return _pd_create_memsection(pd_myself(), size, 0, MEM_NORMAL,
                                 physpool, virtpool, base);
}

memsection_ref_t
memsection_create_user(uintptr_t size, uintptr_t *base)
{
    return _pd_create_memsection(pd_myself(), size, 0, MEM_NORMAL | MEM_USER,
                                 default_physpool, default_virtpool, base);
}

memsection_ref_t
memsection_create_dma(uintptr_t size, uintptr_t *base, physmem_ref_t *pm, uintptr_t attributes)
{
    memsection_ref_t ms;
    physmem_ref_t new_pm;
    int r;

    ms = memsection_create_user(size, base);
    if (ms == 0) {
        return ms;
    }

    new_pm = physpool_alloc(default_physpool, size);
    if (new_pm == 0) {
        memsection_delete(ms);
        return 0;
    }
    r = memsection_map(ms, 0, new_pm);
    if (r != 0) {
        memsection_delete_dma(ms, new_pm);
        return 0;
    }

    memsection_set_attributes(ms, attributes);

    *pm = new_pm;
    return ms;
}

/* Create some memory that is mapped 1-1 with physical */
memsection_ref_t
memsection_create_direct(uintptr_t size, uintptr_t *base)
{
    return _pd_create_memsection(pd_myself(), size, 0, MEM_DIRECT,
                                 default_physpool, 0, base);
}

/*
 * FIXME: This shouldn't be used and will go away at some stage! 
 */
memsection_ref_t
memsection_create_fixed(uintptr_t size, uintptr_t base)
{
    uintptr_t ret_base;

    return _pd_create_memsection(pd_myself(), size, base, MEM_FIXED,
                                 default_physpool, default_virtpool, &ret_base);
}

memsection_ref_t
memsection_create_fixed_user(uintptr_t size, uintptr_t base)
{
    uintptr_t ret_base;

    return _pd_create_memsection(pd_myself(), size, base, MEM_FIXED | MEM_USER,
                                 default_physpool, default_virtpool, &ret_base);
}

memsection_ref_t
memsection_create_in_zone(uintptr_t size, uintptr_t *base, zone_ref_t zone)
{
    cap_t cap;

    cap = iguana_pd_create_memsection(IGUANA_PAGER, pd_myself(), size, 0,
                                      MEM_NORMAL, default_physpool,
                                      zone, default_clist, base, NULL);
    return cap.ref.memsection;
}

memsection_ref_t
memsection_create_direct_in_zone(uintptr_t size, uintptr_t *base, zone_ref_t zone)
{
    return _pd_create_memsection(pd_myself(), size, 0, MEM_DIRECT,
                                 default_physpool, zone, base);
}

int
memsection_register_server(memsection_ref_t memsect, thread_ref_t server)
{
    return iguana_memsection_register_server(IGUANA_PAGER,
                                             memsect, server, NULL);
}

uintptr_t memsection_virt_to_phys(uintptr_t vaddr, size_t *size ){
   return iguana_memsection_virt_to_phys(IGUANA_PAGER, vaddr, size, NULL); 
}

memsection_ref_t
memsection_lookup(objref_t object, thread_ref_t *server)
{
    thread_ref_t ignored;
    
    if (server == NULL) {
        server = &ignored;
    }
    return iguana_memsection_lookup(IGUANA_PAGER, object, server, NULL);
}

void
memsection_delete(memsection_ref_t memsect)
{
    cap_t cap;
    int r;

    iguana_memsection_delete(IGUANA_PAGER, memsect, NULL);      // can't fail 
    r = clist_lookup(default_clist, memsect, MASTER_IID, &cap);
    if (r == 0) {
        (void)clist_remove(default_clist, cap);
    }
}

void
memsection_delete_dma(memsection_ref_t memsect, physmem_ref_t pm)
{
    memsection_delete(memsect);
    physmem_delete(pm);
}

void
memsection_set_attributes(memsection_ref_t memsect, uintptr_t attributes)
{
    iguana_memsection_set_attributes(IGUANA_PAGER, memsect, attributes, NULL);
}

void *
memsection_base(memsection_ref_t memsect)
{
    uintptr_t size;

    return (void *)iguana_memsection_info(IGUANA_PAGER, memsect, &size, NULL);
}

uintptr_t
memsection_size(memsection_ref_t memsect)
{
    uintptr_t size = 0;

    iguana_memsection_info(IGUANA_PAGER, memsect, &size, NULL);
    return size;
}

int
memsection_page_map(memsection_ref_t memsect, L4_Fpage_t from_page,
                    L4_Fpage_t to_page)
{
    return iguana_memsection_page_map(IGUANA_PAGER, memsect, &from_page,
                                      &to_page, NULL);
}

int
memsection_page_unmap(memsection_ref_t memsect, L4_Fpage_t to_page)
{
    return iguana_memsection_page_unmap(IGUANA_PAGER, memsect, &to_page, NULL);
}

int
memsection_map(memsection_ref_t memsect, uintptr_t offset, physmem_ref_t phys)
{
    return iguana_memsection_map(IGUANA_PAGER, memsect, offset, phys, NULL);
}

int
memsection_unmap(memsection_ref_t memsect, uintptr_t offset, uintptr_t size)
{
    return iguana_memsection_unmap(IGUANA_PAGER, memsect, offset, size, NULL);
}

/*
 * This function basically creates a memsection big enough to cover the
 * requested range, then maps it into the auto-memsection.  Right now,
 * due to other limitations it only does minimal error checking.  It
 * would also be nice if it could acquire physical memory directly.
 */
int
memsection_back_range(memsection_ref_t ms, uintptr_t base, uintptr_t end)
{
    int r;
    L4_Fpage_t dst_fpg, src_fpg;
    uintptr_t mbase, mend, off, size;

    /* Check that args make sense.  */
    assert(base % l4e_min_pagesize() == 0);
    assert(base >= (uintptr_t)memsection_base(ms));
    assert((end + 1) % l4e_min_pagesize() == 0);
    assert(end <= (uintptr_t)memsection_base(ms) + memsection_size(ms) - 1);

    /* This memsection will be the 'source' of physical memory.  */
    size = end - base + 1;
    memsection_create(size, &mbase);
    if (mbase == 0)
        return -1;
    mend = mbase + size - 1;

    /* This loop is ugly because we need to deal with alignment.  */
    for (off = 0; off < size; off += L4_Size(dst_fpg)) {
        dst_fpg = l4e_biggest_fpage(base + off, base + off, end);
        src_fpg = l4e_biggest_fpage(mbase + off, mbase + off, mend);
        if (L4_Size(dst_fpg) < L4_Size(src_fpg))
            src_fpg = L4_Fpage(L4_Address(src_fpg), L4_Size(dst_fpg));
        else if (L4_Size(src_fpg) < L4_Size(dst_fpg))
            dst_fpg = L4_Fpage(L4_Address(dst_fpg), L4_Size(src_fpg));
        r = memsection_page_map(ms, src_fpg, dst_fpg);
        if (r != 0)
            return -1;
    }
    return 0;
}
