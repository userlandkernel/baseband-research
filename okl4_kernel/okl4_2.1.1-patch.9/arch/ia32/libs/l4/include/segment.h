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

#ifndef __L4__IA32__SEGMENT_H__
#define __L4__IA32__SEGMENT_H__

#include <l4/types.h>

#define L4_CodeSegmentDescriptor    0xb
#define L4_DataSegmentDescriptor    0x3

#define L4_LDT_Set_Gs           ((L4_Word_t)1 << (L4_BITS_PER_WORD - 1))
#define L4_LDT_Set_Fs           ((L4_Word_t)1 << (L4_BITS_PER_WORD - 2))
#define L4_LDT_Index_Mask       (((L4_Word_t)1 << 13) - 1)


typedef struct {
    union {
        L4_Word32_t raw[2];
        struct {
            L4_Word_t limitlow      : 16;
            L4_Word_t baselow       : 24 __attribute__((packed));
            L4_Word_t type          :  4;
            L4_Word_t s             :  1;
            L4_Word_t dpl           :  2;
            L4_Word_t p             :  1;
            L4_Word_t limithigh     :  4;
            L4_Word_t avl           :  2;
            L4_Word_t d             :  1;
            L4_Word_t g             :  1;
            L4_Word_t basehigh      :  8;
        } d __attribute__((packed));
    } x;
} L4_SegmentDescriptor_t;

/*
 * Simple function to set the descriptor with a simple base and a 
 * page-based limit.
 */
L4_INLINE void
L4_Set_SegmentDescriptor(L4_SegmentDescriptor_t *seg, L4_Word_t base, 
                         L4_Word_t limit)
{
    seg->x.d.limitlow = (limit >> 12) & 0xffff;
    seg->x.d.limithigh = (limit >> 28) & 0xf;
    seg->x.d.g = 1;

    seg->x.d.baselow = base & 0xffffff;
    seg->x.d.basehigh = (base >> 24) & 0xff;
    seg->x.d.type = L4_DataSegmentDescriptor;
    seg->x.d.dpl = 3;

    seg->x.d.p = 1;
    seg->x.d.d = 1;
    seg->x.d.s = 1;

    seg->x.d.avl = 0;
}

#endif
