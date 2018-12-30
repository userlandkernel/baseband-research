/*
 * Copyright (c) 2006, National ICT Australia
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
 * Description:   Miscelaneous interfaces
 */
/*
 * Creator: Carl van Schaik
 */
#ifndef __L4__CACHE_H__
#define __L4__CACHE_H__

#include <l4/types.h>
#include <l4/arch/syscalls.h>

/*
 * Cache control options
 */

#define L4_CacheCtl_ArchSpecifc         ((L4_Word_t)0 << 6)
#define L4_CacheCtl_FlushRanges         ((L4_Word_t)1 << 6)
#define L4_CacheCtl_FlushIcache         ((L4_Word_t)4 << 6)
#define L4_CacheCtl_FlushDcache         ((L4_Word_t)5 << 6)
#define L4_CacheCtl_FlushCache          ((L4_Word_t)6 << 6)
#define L4_CacheCtl_CacheLock           ((L4_Word_t)8 << 6)
#define L4_CacheCtl_CacheUnLock         ((L4_Word_t)9 << 6)

#define L4_CacheCtl_MaskL1              ( (L4_Word_t)1 << 12)
#define L4_CacheCtl_MaskL2              ( (L4_Word_t)2 << 12)
#define L4_CacheCtl_MaskL3              ( (L4_Word_t)4 << 12)
#define L4_CacheCtl_MaskL4              ( (L4_Word_t)8 << 12)
#define L4_CacheCtl_MaskL5              ((L4_Word_t)16 << 12)
#define L4_CacheCtl_MaskL6              ((L4_Word_t)32 << 12)
#define L4_CacheCtl_MaskAllLs           ((L4_Word_t)63 << 12)

/*
 * Cache control flush attributes
 */
#define L4_CacheFlush_AttrI             ((L4_Word_t)1 << (L4_BITS_PER_WORD-4))
#define L4_CacheFlush_AttrD             ((L4_Word_t)1 << (L4_BITS_PER_WORD-3))
#define L4_CacheFlush_AttrClean         ((L4_Word_t)1 << (L4_BITS_PER_WORD-2))
#define L4_CacheFlush_AttrInvalidate    ((L4_Word_t)1 << (L4_BITS_PER_WORD-1))
#define L4_CacheFlush_AttrAll           (L4_CacheFlush_AttrI | L4_CacheFlush_AttrD |    \
                                         L4_CacheFlush_AttrClean | L4_CacheFlush_AttrInvalidate)

L4_INLINE L4_Word_t
L4_CacheFlushAll(void)
{
    return L4_CacheControl(L4_nilspace,
                           L4_CacheCtl_FlushCache | L4_CacheCtl_MaskAllLs);
}

/*
 * Flush cache range (start..end-1)
 */
L4_INLINE L4_Word_t
L4_CacheFlushRange(L4_SpaceId_t s, L4_Word_t start, L4_Word_t end)
{
    L4_Word_t size = (end - start);

    size = size & (((L4_Word_t)1 << (L4_BITS_PER_WORD - 4)) - 1);
    size |= L4_CacheFlush_AttrAll;
    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    return L4_CacheControl(s,
                           L4_CacheCtl_FlushRanges | L4_CacheCtl_MaskAllLs | 0);
}

/*
 * Flush I-cache range (start..end-1)
 */
L4_INLINE L4_Word_t
L4_CacheFlushIRange(L4_SpaceId_t s, L4_Word_t start, L4_Word_t end)
{
    L4_Word_t size = (end - start);

    size = size & (((L4_Word_t)1 << (L4_BITS_PER_WORD - 4)) - 1);
    size |=
        L4_CacheFlush_AttrI | L4_CacheFlush_AttrClean |
        L4_CacheFlush_AttrInvalidate;
    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    return L4_CacheControl(s,
                           L4_CacheCtl_FlushRanges | L4_CacheCtl_MaskAllLs | 0);
}

/*
 * Flush-clean-invalidate D-cache range (start..end-1)
 */
L4_INLINE L4_Word_t
L4_CacheFlushDRange(L4_SpaceId_t s, L4_Word_t start, L4_Word_t end)
{
    L4_Word_t size = (end - start);

    size = size & (((L4_Word_t)1 << (L4_BITS_PER_WORD - 4)) - 1);
    size |=
        L4_CacheFlush_AttrD | L4_CacheFlush_AttrClean |
        L4_CacheFlush_AttrInvalidate;
    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    return L4_CacheControl(s,
                           L4_CacheCtl_FlushRanges | L4_CacheCtl_MaskAllLs | 0);
}

/*
 * Flush-Invalidate cache range (start..end-1)
 */
L4_INLINE L4_Word_t
L4_CacheFlushRangeInvalidate(L4_SpaceId_t s, L4_Word_t start, L4_Word_t end)
{
    L4_Word_t size = (end - start);

    size = size & (((L4_Word_t)1 << (L4_BITS_PER_WORD - 4)) - 1);
    size |=
        L4_CacheFlush_AttrD | L4_CacheFlush_AttrI |
        L4_CacheFlush_AttrInvalidate;
    L4_LoadMR(0, start);
    L4_LoadMR(1, size);

    return L4_CacheControl(s,
                           L4_CacheCtl_FlushRanges | L4_CacheCtl_MaskAllLs | 0);
}

#endif /* !__L4__CACHE_H__ */
