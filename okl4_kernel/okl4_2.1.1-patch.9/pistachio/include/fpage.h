/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   V4 flexpages
 */
#ifndef __API__V4__FPAGE_H__
#define __API__V4__FPAGE_H__

/**
 * Flexpages are size-aligned memory objects and can cover
 * multiple hardware pages. fpage_t implements the V4 specific
 * flexpage type, having read, write and execute bits.
 */
class fpage_t
{
    /* data members */
public:
    union {
        struct {
            BITFIELD7(word_t,
                execute         : 1,
                write           : 1,
                read            : 1,
                meta            : 1,    /* extended attributes */
                size            : 6,
                base            : L4_FPAGE_BASE_BITS,
                                : (BITS_WORD - L4_FPAGE_BASE_BITS - 10)
                );
        } x;
        word_t raw;
    };

    /* member functions */
public:
    /**
     * sets the flexpage
     */
    void set(word_t base, word_t size, bool read, bool write, bool exec)
        {
            raw = 0;
            x.base = (base & (~0 >> (L4_FPAGE_BASE_BITS - size))) >> 10;
            x.size = size;
            x.read = read;
            x.write = write;
            x.execute = exec;
        }

    /**
     * @return true if the flexpage is a nil fpage
     */
    bool is_nil_fpage()
        { return raw == 0; }

    /**
     * @return true if flexpage covers the whole address space
     * the complete fpage is a special value defined in the V4 API
     */
    bool is_complete_fpage()
        { return (x.size == 1) && (x.base == 0); }

    /**
     * checks if a given address range is contained completely within
     * the fpage
     *
     * @param start start address of range
     * @param end end address of range
     * @return true if range is contained within fpage, false otherwise */
    bool is_range_in_fpage(addr_t start, addr_t end)
        {
            /* VU: should we make sure, that base is valid? */
            return is_complete_fpage () ||
                (get_address() <= start &&
                 addr_offset(get_address(), get_size()) >= end);
        }

    /**
     * @param addr returns true if addr is within the range of
     * the flexpage
     */
    bool is_addr_in_fpage(addr_t addr)
        { return is_range_in_fpage (addr, (addr_t)((word_t)addr + sizeof(addr_t))); }

    /**
     * checks if the current and the given fpage overlap
     * @return true if fpages overlap
     */
    bool is_overlapping(fpage_t fpage)
        {
            if (fpage.get_address() < this->get_address())
                return (addr_offset(fpage.get_address(), fpage.get_size()) >
                        this->get_address());
            else
                return (addr_offset(this->get_address(), this->get_size()) >
                        fpage.get_address());
        }

    bool is_range_overlapping(addr_t start, addr_t end)
        {
            if (start < get_address())
                return end > get_address();
            else
                return addr_offset(get_address(), get_size()) > start;
        }

    /**
     * @return base address of the fpage
     * get_base does not size-align the address
     */
    addr_t get_base()
        { return (addr_t)(x.base << 10); }

    /**
     * @return size aligned address of the fpage
     */
    addr_t get_address()
        { return (addr_t)(x.base << 10 & ~((~0UL) >> ((sizeof(word_t)*8) - x.size))); }

    /**
     * @return size of the flexpage
     */
    word_t get_size()
        { return 1UL << x.size; }

    /**
     * @return log2 size of the fpage
     */
    word_t get_size_log2()
        { return is_complete_fpage() ? sizeof(word_t) * 8 : x.size; }

    /**
     * @return true if the read bit is set
     */
    bool is_read()
        { return x.read; }

    /**
     * @return true if the write bit is set
     */
    bool is_write()
        { return x.write; }

    /**
     * @return true if the execute bit is set
     */
    bool is_execute()
        { return x.execute; }

    /**
     * @return true if the meta bit is set
     */
    bool is_meta()
        { return x.meta; }

    /**
     * @return true if read, write and execute bits are set
     */
    bool is_rwx()
        { return x.read && x.write && x.execute; }

    /**
     * sets all permission bits in the fpage
     */
    void set_rwx()
        { x.read = 1; x.write = 1; x.execute = 1; }

    void set_rwx(word_t rwx)
        { raw |= (rwx & 7); }

    /**
     * @return access rights of fpage
     */
    word_t get_rwx()
        { return raw & 7; }

    /**
     * @return delivers an fpage covering the complete address space
     */
    static fpage_t complete()
        {
            fpage_t ret;
            ret.raw = 0;
            ret.x.size = 1;
            return ret;
        }

    /**
     * @return delivers an nil fpage
     */
    static fpage_t nilpage()
        {
            fpage_t ret;
            ret.raw = 0;
            return ret;
        }
};

#endif /* !__API__V4__FPAGE_H__ */
