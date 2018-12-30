/*
 * Copyright (c) 2001-2004, Karlsruhe University
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
 * Description:   Interfaces for handling address spaces/mappings
 */
#ifndef __L4__SPACE_H__
#define __L4__SPACE_H__

#include <l4/types.h>
#include <l4/map.h>
#include <l4/kdebug.h>
#include <l4/arch/syscalls.h>

/*
 * Derived functions
 */

L4_INLINE L4_Word_t
L4_MapFpage(L4_SpaceId_t s, L4_Fpage_t f, L4_PhysDesc_t p)
{
    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);

    return L4_MapControl(s, L4_MapCtrl_Modify);
}

L4_INLINE L4_Word_t
L4_MapFpages(L4_SpaceId_t s,
             L4_Word_t n, L4_Fpage_t *fpages, L4_PhysDesc_t *descs)
{
    L4_Word_t i;

    for (i = 0; i < n; i++) {
        if ((fpages[i].raw & 0x7) == 0)
            L4_KDB_Enter("eek");
        L4_LoadMR(2 * i, descs[i].raw);
        L4_LoadMR(2 * i + 1, fpages[i].raw);
    }
    return L4_MapControl(s, L4_MapCtrl_Modify | (n - 1));
}

L4_INLINE L4_Word_t
L4_UnmapFpage(L4_SpaceId_t s, L4_Fpage_t f)
{
    L4_LoadMR(1, f.raw & (~0xFUL));

    return L4_MapControl(s, L4_MapCtrl_Modify);
}

L4_INLINE L4_Word_t
L4_UnmapFpages(L4_SpaceId_t s, L4_Word_t n, L4_Fpage_t *fpages)
{
    L4_Word_t i;

    for (i = 0; i < n; i++) {
        L4_LoadMR(2 * i + 1, fpages[i].raw & (~0xfUL));
    }
    return L4_MapControl(s, L4_MapCtrl_Modify | (n - 1));
}

#if defined(__l4_cplusplus)
L4_INLINE L4_Fpage_t
L4_Unmap(L4_SpaceId_t s, L4_Fpage_t f)
{
    return L4_UnmapFpage(s, f);
}

L4_INLINE L4_Word_t
L4_Unmap(L4_SpaceId_td s, L4_Word_t n,
         L4_Fpage_t *fpages, L4_PhysDesc_t *descs)
{
    return L4_UnmapFpages(s, n, fpages, descs);
}
#endif

L4_INLINE L4_Word_t
L4_ReadFpage(L4_SpaceId_t s, L4_Fpage_t f, L4_PhysDesc_t *p, L4_MapItem_t *m)
{
    L4_Word_t r;

    L4_LoadMR(0, 0UL);
    L4_LoadMR(1, f.raw);

    r = L4_MapControl(s, L4_MapCtrl_Query);
    if (m)
        L4_StoreMR(1, &m->raw);
    if (p)
        L4_StoreMR(0, &p->raw);
    return r;
}

L4_INLINE L4_Bool_t
L4_WasWritten(L4_Fpage_t f)
{
    return (f.raw & L4_Writable) != 0;
}

L4_INLINE L4_Bool_t
L4_WasReferenced(L4_Fpage_t f)
{
    return (f.raw & L4_Readable) != 0;
}

L4_INLINE L4_Bool_t
L4_WaseXecuted(L4_Fpage_t f)
{
    return (f.raw & L4_eXecutable) != 0;
}

#endif /* !__L4__SPACE_H__ */
