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
 * Description:
 */
#ifndef __L4__SPACEID_H__
#define __L4__SPACEID_H__

#include <kernel/idtable.h>

class space_t;

class spaceid_t
{
public:
    static spaceid_t kernelspace()
        {
            spaceid_t sid;
            sid.raw = ~0UL;
            return sid;
        }

    static spaceid_t nilspace()
        {
            spaceid_t sid;
            sid.raw = TWOSCOMP(1UL);
            return sid;
        }

    static spaceid_t rootspace()
        {
            spaceid_t sid;
            sid.raw = 0;
            return sid;
        }

    static spaceid_t spaceid(word_t spaceno_)
        {
            spaceid_t sid;
            sid.spaceno = spaceno_;
            return sid;
        }

    /* check for specific (well known) space ids */
    bool is_nilspace() { return this->raw == TWOSCOMP(1UL); }
    bool is_rootspace() { return this->raw == 0; }

    word_t get_spaceno() const { return this->spaceno; }

    word_t get_raw() { return this->raw; }
    void set_raw(word_t raw_) { this->raw = raw_; }

    /* operators */
    bool operator == (const spaceid_t & sid) const
        {
            return this->raw == sid.raw;
        }

    bool operator != (const spaceid_t & sid) const
        {
            return this->raw != sid.raw;
        }

private:
    union {
        word_t raw;
        word_t spaceno;
    };
};

class spaceid_lookup_t : private id_lookup_t
{
public:
    bool is_valid(spaceid_t id) {
        return valid_id(id.get_spaceno());
    }

    space_t * lookup_space(spaceid_t id) {
        return (space_t*)lookup_id(id.get_spaceno());
    }

public:
    void add_space(spaceid_t id, space_t* space) {
        add_object(id.get_spaceno(), space);
    }

    void remove_space(spaceid_t id) {
        remove_object(id.get_spaceno());
    }
private:
    friend void init_spaceids(void);
};

INLINE spaceid_lookup_t *
get_space_list()
{
    extern spaceid_lookup_t space_lookup;
    return &space_lookup;
}

#endif /*__L4__SPACEID_H__*/
