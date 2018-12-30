/*
 * Copyright (c) 2005-2006, National ICT Australia
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
 * Description:  Memory mapping API class definitions
 */
#ifndef __MAP_H__
#define __MAP_H__

class phys_desc_t
{
public:
    inline word_t get_raw()
        { return raw; }

    inline void clear()
        { raw = 0; }

    inline u64_t get_base()
        { return ((u64_t)x.base) << 10; }

    inline void set_base(u64_t base)
        { x.base = base >> 10; }

    inline l4attrib_e get_attributes()
        { return (l4attrib_e)x.attr; }

    inline void set_attributes(l4attrib_e attr)
        { x.attr = attr; }

    inline void operator = (word_t val)
        { this->raw = val; }
private:
    union {
        struct {
            BITFIELD2(word_t,
                    attr        : 6,
                    base        : BITS_WORD - 6);
        } x;
        word_t raw;
    };
};

class perm_desc_t
{
public:
    inline word_t get_raw()
        { return raw; }

    inline void clear()
        { raw = 0; }

    inline void set_perms(word_t rwx)
        { x.rwx = rwx; }

    inline void set_reference(word_t RWX)
        { x.RWX = RWX; }

    inline void set_size(word_t size)
        { x.size = size; }

    inline void operator = (word_t val)
        { this->raw = val; }
private:
    union {
        struct {
            BITFIELD4(word_t,
                    rwx         : 4,
                    size        : 6,
                    __res       : BITS_WORD - 14,
                    RWX         : 4);
        } x;
        word_t raw;
    };
};

class map_control_t
{
public:
    inline bool is_modify()
        { return x.m == 1; }

    inline bool is_query()
        { return x.q == 1; }

#if defined(CONFIG_ARM_V5)
    inline bool is_window()
        { return x.win == 1; }
#endif

    inline word_t highest_item()
        { return x.n; }

    inline void operator = (word_t val)
        { this->raw = val; }
private:
    union {
        struct {
#if defined(CONFIG_ARM_V5)
            BITFIELD5(word_t,
                    n           : 6,
                    __res       : BITS_WORD - 9,
                    win         : 1,
                    q           : 1,
                    m           : 1);
#else
            BITFIELD5(word_t,
                    n           : 6,
                    __res       : BITS_WORD - 9,
                                : 1,
                    q           : 1,
                    m           : 1);
#endif
        } x;
        word_t raw;
    };
};

#endif /* __MAP_H__ */
