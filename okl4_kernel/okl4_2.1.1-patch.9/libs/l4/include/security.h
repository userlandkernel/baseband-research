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
 * Description:   Miscelaneous interfaces
 */
#ifndef __L4__SECURITY_H__
#define __L4__SECURITY_H__

#include <l4/types.h>
#include <l4/arch/syscalls.h>

#ifdef L4_32BIT
#define L4_SecurityControl_PlatCtrlDomain   1UL
#define L4_SecurityControl_SpcSwitchDomain  2UL
#define L4_SecurityControl_IrqAccessDomain  4UL
#define L4_SecurityControl_UserMapDomain    6UL
#elif defined(L4_64BIT)
#define L4_SecurityControl_PlatCtrlDomain   1ULL
#define L4_SecurityControl_SpcSwitchDomain  2ULL
#define L4_SecurityControl_IrqAccessDomain  4ULL
#define L4_SecurityControl_UserMapDomain    6ULL
#endif

#if defined(ARM_SHARED_DOMAINS)
#define L4_SecurityControl_DomainShareDomain  8UL
#endif

/*
 * PlatformControl security
 */
L4_INLINE L4_Word_t
L4_AllowPlatformControl(L4_SpaceId_t target)
{
    L4_LoadMR(0, 0);

    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_PlatCtrlDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant));
}

L4_INLINE L4_Word_t
L4_RestrictPlatformControl(L4_SpaceId_t target)
{
    L4_LoadMR(0, 0);

    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_PlatCtrlDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpRevoke));
}

/*
 * SpaceSwitch security
 */
L4_INLINE L4_Word_t
L4_AllowSpaceSwitch(L4_SpaceId_t from, L4_SpaceId_t to)
{
    L4_LoadMR(0, to.raw);

    return L4_SecurityControl(from,
            L4_SecurityCtrl_domain(L4_SecurityControl_SpcSwitchDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant));
}

L4_INLINE L4_Word_t
L4_RestrictSpaceSwitch(L4_SpaceId_t from, L4_SpaceId_t to)
{
    L4_LoadMR(0, to.raw);

    return L4_SecurityControl(from,
            L4_SecurityCtrl_domain(L4_SecurityControl_SpcSwitchDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpRevoke));
}

/*
 * PlatformControl security
 */
L4_INLINE L4_Word_t
L4_AllowInterruptControl(L4_SpaceId_t target)
{
    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_IrqAccessDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant));
}

L4_INLINE L4_Word_t
L4_RestrictInterruptControl(L4_SpaceId_t target)
{
    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_IrqAccessDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpRevoke));
}

/*
 * UserMapDomain security
 */
L4_INLINE L4_Word_t
L4_AllowUserMapping(L4_SpaceId_t target, word_t phys_start, word_t phys_size)
{
    L4_LoadMR(0, phys_start);
    L4_LoadMR(1, phys_size);

    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_UserMapDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant) | 1);
}

L4_INLINE L4_Word_t
L4_RestrictUserMapping(L4_SpaceId_t target, word_t phys_start, word_t phys_size)
{
    L4_LoadMR(0, phys_start);
    L4_LoadMR(1, phys_size);

    return L4_SecurityControl(target,
            L4_SecurityCtrl_domain(L4_SecurityControl_UserMapDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpRevoke) | 1);
}

#endif /* !__L4__Security_H__ */
