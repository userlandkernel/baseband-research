/*
 * Copyright (c) 2002, Karlsruhe University
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
 * Description:   IA32 Segment Descriptor
 */
#ifndef __ARCH__IA32__SEGDESC_H__
#define __ARCH__IA32__SEGDESC_H__

#include <kernel/debug.h>

class ia32_segdesc_t
{
public:
    enum segtype_e
    {
        ldt  = 0x2,
        code = 0xb,
        data = 0x3,
        tss  = 0x9
    };

    void set_seg(u32_t base, u32_t limit, int dpl, segtype_e type);
    void set_sys(u32_t base, u32_t limit, int dpl, segtype_e type);
    void set_raw(word_t seg_low, word_t seg_high);
    bool user_check();
    u32_t get_base();
    u32_t get_limit();
    void dump(bool global, int index);

private:
    union {
        u32_t raw[2];
        struct {
            u32_t limit_low     : 16;
            u32_t base_low      : 24 __attribute__((packed));
            u32_t type          :  4;
            u32_t s             :  1;
            u32_t dpl           :  2;
            u32_t p             :  1;
            u32_t limit_high    :  4;
            u32_t avl           :  2;
            u32_t d             :  1;
            u32_t g             :  1;
            u32_t base_high     :  8;
        } d __attribute__((packed));
    } x;
    friend class kdb_t;
};

INLINE bool ia32_segdesc_t::user_check()
{
    if (x.d.dpl != 3)
        return false;
    if (x.d.s == 0/*system segment*/)
        return false;
    return true;
}

INLINE void ia32_segdesc_t::set_raw(word_t low, word_t high)
{
    x.raw[0] = low;
    x.raw[1] = high;
}

INLINE void ia32_segdesc_t::dump(bool global, int index)
{
    printf("%s[%d] = %p:%p", global ? "GDT" : "LDT", index,
        x.raw[0], x.raw[1]);
    if ( (x.raw[0] == 0 && x.raw[1] == 0) ||
         (! x.d.s) )
    {
        printf("\n");
        return;
    }
    printf(" <%p,%p> ",
            x.d.base_low + (x.d.base_high << 24),
            x.d.base_low + (x.d.base_high << 24) +
            (x.d.g ? 0xfff |
                (x.d.limit_low + (x.d.limit_high << 16)) << 12 :
                (x.d.limit_low + (x.d.limit_high << 16))));
    printf("dpl=%d %d-bit ", x.d.dpl, x.d.d ? 32 : 16);
    if ( x.d.type & 0x8 )
        printf("code %cC %cR ",
                x.d.type & 0x4 ? ' ' : '!',
                x.d.type & 0x2 ? ' ' : '!');
    else
        printf("data E%c R%c ",
                x.d.type & 0x4 ? 'D' : 'U',
                x.d.type & 0x2 ? 'W' : 'O');
    printf("%cP %cA\n",
            x.d.p ? ' ' : '!',
            x.d.type & 0x1 ? ' ' : '!');

}

INLINE u32_t ia32_segdesc_t::get_base()
{
    return (x.d.base_low|(x.d.base_high << 24));
}

INLINE u32_t ia32_segdesc_t::get_limit()
{
    u32_t limit;

    limit = (x.d.limit_low|(x.d.limit_high & 0xf) << 16);
    if (x.d.g == 1/*4K*/)
        limit <<= 12;
    return limit;
}

INLINE void ia32_segdesc_t::set_seg(u32_t base, u32_t limit,
                                    int dpl, segtype_e type)
{
    if (limit > ( 1 << 20))
    {
        x.d.limit_low  = (limit >> 12) & 0xFFFF;
        x.d.limit_high = (limit >> 28) & 0xF;
        x.d.g = 1;      /* 4K granularity       */
    }
    else
    {
        x.d.limit_low  =  limit        & 0xFFFF;
        x.d.limit_high = (limit >> 16) & 0xFF;
        x.d.g = 0;      /* 1B granularity       */
    }

    x.d.base_low   = base & 0xFFFFFF;
    x.d.base_high  = (base >> 24) & 0xFF;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.p = 1;  /* present              */
    x.d.d = 1;  /* 32-bit segment       */
    x.d.s = 1;  /* non-system segment   */

    /* unused fields */
    x.d.avl = 0;
}

INLINE void ia32_segdesc_t::set_sys(u32_t base, u32_t limit,
                                    int dpl, segtype_e type)
{
    x.d.limit_low  =  limit        & 0xFFFF;
    x.d.limit_high = (limit >> 16) & 0xFF;
    x.d.base_low   = base        & 0xFFFFFF;
    x.d.base_high  = (base >> 24) &     0xFF;
    x.d.type = type;
    x.d.dpl = dpl;

    /* default fields */
    x.d.p = 1;  /* present              */
    x.d.g = 0;  /* byte granularity     */
    x.d.d = 0;  /* 32-bit segment       */
    x.d.s = 0;  /* non-system segment   */

    /* unused fields */
    x.d.avl = 0;
}



#endif /* !__ARCH__IA32__SEGDESC_H__ */
