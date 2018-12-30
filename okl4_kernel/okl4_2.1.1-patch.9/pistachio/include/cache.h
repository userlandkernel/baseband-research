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
 * Description: Cache control API class definitions
 */
#ifndef __API__V4__CACHE_H__
#define __API__V4__CACHE_H__

enum cacheop_e {
    cop_arch        = 0,
    cop_flush_range = 1,
    cop_flush_I_all = 4,
    cop_flush_D_all = 5,
    cop_flush_all   = 6,
    cop_lock        = 8,
    cop_unlock      = 9,
};

#define CACHE_ATTRIB_OP_CLEAN           (1<<2)
#define CACHE_ATTRIB_OP_INVAL           (2<<2)
#define CACHE_ATTRIB_OP_CLEAN_INVAL     (3<<2)
#define CACHE_ATTRIB_OP_MASK            (3<<2)

#define CACHE_ATTRIB_I(x)               ((x) | 1)
#define CACHE_ATTRIB_D(x)               ((x) | 2)
#define CACHE_ATTRIB_ID(x)              ((x) | 3)
#define CACHE_ATTRIB_MASK               (3)

/* Masks */
#define CACHE_ATTRIB_MASK_I             CACHE_ATTRIB_I(0)
#define CACHE_ATTRIB_MASK_D             CACHE_ATTRIB_D(0)
#define CACHE_ATTRIB_MASK_ID            CACHE_ATTRIB_ID(0)
#define CACHE_ATTRIB_MASK_I_OP          CACHE_ATTRIB_I(CACHE_ATTRIB_OP_MASK)
#define CACHE_ATTRIB_MASK_D_OP          CACHE_ATTRIB_D(CACHE_ATTRIB_OP_MASK)
#define CACHE_ATTRIB_CLEAN_I            CACHE_ATTRIB_I(CACHE_ATTRIB_OP_CLEAN)
#define CACHE_ATTRIB_INVAL_I            CACHE_ATTRIB_I(CACHE_ATTRIB_OP_INVAL)
#define CACHE_ATTRIB_CLEAN_INVAL_I      CACHE_ATTRIB_I(CACHE_ATTRIB_OP_CLEAN_INVAL)
#define CACHE_ATTRIB_CLEAN_D            CACHE_ATTRIB_D(CACHE_ATTRIB_OP_CLEAN)
#define CACHE_ATTRIB_INVAL_D            CACHE_ATTRIB_D(CACHE_ATTRIB_OP_INVAL)
#define CACHE_ATTRIB_CLEAN_INVAL_D      CACHE_ATTRIB_D(CACHE_ATTRIB_OP_CLEAN_INVAL)

enum cacheattr_e {
    attr_invalidate_i   = CACHE_ATTRIB_I (CACHE_ATTRIB_OP_INVAL),
    attr_invalidate_d   = CACHE_ATTRIB_D (CACHE_ATTRIB_OP_INVAL),
    attr_invalidate_id  = CACHE_ATTRIB_ID(CACHE_ATTRIB_OP_INVAL),
    attr_clean_i        = CACHE_ATTRIB_I (CACHE_ATTRIB_OP_CLEAN),
    attr_clean_d        = CACHE_ATTRIB_D (CACHE_ATTRIB_OP_CLEAN),
    attr_clean_id       = CACHE_ATTRIB_ID(CACHE_ATTRIB_OP_CLEAN),
    attr_clean_inval_i  = CACHE_ATTRIB_I (CACHE_ATTRIB_OP_CLEAN_INVAL),
    attr_clean_inval_d  = CACHE_ATTRIB_D (CACHE_ATTRIB_OP_CLEAN_INVAL),
    attr_clean_inval_id = CACHE_ATTRIB_ID(CACHE_ATTRIB_OP_CLEAN_INVAL),
};

class cache_control_t
{
public:
    inline word_t highest_item()
        { return x.n; }

    inline cacheop_e operation()
        { return (cacheop_e)x.op; }

    inline word_t cache_level_mask()
        { return x.lx; }

    inline void operator = (word_t raw)
        { this->m_raw = raw; }
private:
    union {
        word_t m_raw;
        struct {
            BITFIELD4(word_t,
                    n           : 6,
                    op          : 6,
                    lx          : 6,
                    __res       : BITS_WORD - 18);
        } x;
    };
};

#include <kernel/arch/cache.h>

#endif /* __API__V4__CACHE_H__ */
