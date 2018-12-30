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

#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/space.h>
#include <l4/map.h>
#include <l4e/map.h>
#include <l4e/misc.h>

int l4e_split_mapping(L4_SpaceId_t space_id, uintptr_t vaddr, L4_Word_t pagesize)
{
    L4_Fpage_t vpage;
    L4_MapItem_t m;
    L4_PhysDesc_t p;
    uintptr_t paddr;
    int num_mappings;
    int res;
    int i;

    vpage = L4_Fpage(vaddr, pagesize);
    res = L4_ReadFpage(space_id, vpage, &p, &m);

    if(res != 1)
        return 0;

    /* is the mapping already small enough? */
    /* handles the no mapping (size == 0) case too */
    if(vpage.X.s> m.X.size)
        return 1;

    /* we need to split these mappings, move vaddr to the base of the region */
    vaddr = vaddr & ~((1UL << m.X.size) - 1);
    paddr = (p.X.base << 10) & ~((1UL << m.X.size) - 1);
    num_mappings = 1UL << (m.X.size - vpage.X.s);

    /* make sure we can map it atomically */
    if(num_mappings * 2 > __L4_NUM_MRS)
        return 0;

    /* Set up the MRs for the call */
    for(i = 0; i < num_mappings; i++) 
    {
        vpage = L4_Fpage(vaddr, pagesize);
        vpage.X.rwx = m.X.rwx;
        p = L4_PhysDesc(paddr, p.X.attr); 

        L4_LoadMR(i<<1, p.raw);
        L4_LoadMR((i<<1) + 1, vpage.raw);

        vaddr += pagesize;
        paddr += pagesize;
    }

    /* Map everything */
    res = L4_MapControl(space_id, L4_MapCtrl_Modify | (i - 1));

    if(res != 1)
        return 0;
    
    return 1;
}
