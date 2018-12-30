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
/**
 * @file Mutex identifiers.
 */
#ifndef __MUTEXID_H__
#define __MUTEXID_H__

#include <kernel/idtable.h>

class mutex_t;

/** Mutex identifier objects. */
class mutexid_t
{
public:
    /**
     * Return the numeric value of the mutex identifier.
     *
     * @return The numeric value of this mutex identifier. 
     */
    word_t get_number(void);

    /**
     * Set the numeric value of the mutex identifier.
     *
     * @param number  The number to set the mutex identifier to.
     */
    void set_number(word_t number);

private:
    /* This mutex identifier's numeric value. */
    word_t number;
};

/** Mutex identifier table object. */
class mutexid_table_t : private id_lookup_t
{
public:
    /**
     * Validate a mutex identifier.
     *
     * @param id Mutex indentifier being tested for validity.
     *
     * @return If this mutex identifier is valid.
     */
    bool is_valid(mutexid_t id) {
        return valid_id(id.get_number());
    }

    /**
     * Lookup a mutex object in the mutex lookup table.
     *
     * @pre id.is_valid()
     *
     * @param id Mutex identifier of the mutex object we want.
     *
     * @return Mutex object associated with mutex identifier.
     */
    mutex_t * lookup(mutexid_t id) {
        return (mutex_t*)lookup_id(id.get_number());
    }

    /**
     * Add a new mutex object to the mutex lookup table.
     *
     * @pre id.is_valid()
     *
     * @param id Mutex identifier of the newly added mutex object.
     * @param mutex Mutex object being added.
     */
    void insert(mutexid_t id, mutex_t * mutex) {
        add_object(id.get_number(), mutex);
    }

    /**
     * Remove a mutex object from the mutex lookup table.
     *
     * @pre id.is_valid()
     *
     * @param id Mutex identifier of mutex object being removed.
     */
    void remove(mutexid_t id) {
        remove_object(id.get_number());
    }

    friend void init_mutex(void);
};

/*
 *  mutexid_t class methods.
 */

INLINE word_t
mutexid_t::get_number(void)
{
    return this->number;
}

INLINE void
mutexid_t::set_number(word_t number)
{
    this->number = number;
}

/**
 * Create a Mutex with the given ID.
 */
INLINE mutexid_t mutexid(word_t rawid)
{
    mutexid_t m;
    m.set_number(rawid);
    return m;
}

/** Table mapping mutexid_t's to mutex_t pointers */
extern mutexid_table_t mutexid_table;

#endif /* !__MUTEXID_H__ */
