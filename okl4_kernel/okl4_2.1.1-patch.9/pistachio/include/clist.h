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
 * Description: Capability list
 */
#ifndef __CLIST_H__
#define __CLIST_H__

#include <l4/config.h>
#include <kernel/debug.h>
#include <kernel/caps.h>
#include <kernel/sync.h>
#include <kernel/thread.h>
#include <kernel/idtable.h>
#if defined(CONFIG_ARCH_ARM)
#include <kernel/arch/globals.h>
#endif

class tcb_t;
class clist_t;

class clistid_t
{
public:
    word_t get_clistno() const { return this->clistno; }

    word_t get_raw() { return this->raw; }
    void set_raw(word_t raw_) { this->raw = raw_; }
private:
    union {
        word_t raw;
        word_t clistno;
    };
};

INLINE clistid_t clistid(word_t rawid)
{
    clistid_t c;
    c.set_raw(rawid);
    return c;
}

/*
 * The Capability List Class
 */
class clist_t
{
public:
    /**
     * Lookup a thread cap in the clist
     *
     * @param tid   Thread-id of thread being looked up.
     * @param write   True if write lock required, false for read lock.
     * @returns TCB if lookup succeeded, NULL if thread not found.
     */
    tcb_t* lookup_thread_cap_locked(threadid_t tid, bool write = false);

    /**
     * Lookup a thread cap in the clist
     *
     * @param tid   Thread-id of thread being looked up.
     * @returns TCB if lookup succeeded, NULL if thread not found.
     */
    tcb_t* lookup_thread_cap_unlocked(threadid_t tid);

    /**
     * Lookup a ipc cap in the clist, and get read-lock
     *
     * @param tid   Thread-id of thread being looked up.
     * @returns TCB if lookup succeeded, NULL if thread not found.
     */
    tcb_t* lookup_ipc_cap_locked(threadid_t tid);

    /**
     * Lookup a cap in the clist, and get read-lock
     *
     * @param tid   Thread-id of thread being looked up.
     * @returns CAP if lookup succeeded, NULL if thread not found.
     */
    cap_t* lookup_cap(threadid_t tid);

    /**
     * Check if cap number is valid in this clist
     *
     * @param tid   Thread-id of thread being looked up.
     * @return true if thread-id in range, false if not in range
     */
    bool is_valid(threadid_t tid) {
        return (tid.get_threadno() > max_id) ? false : true;
    }

    /**
     * Add a thread cap to the clist.
     *
     * @pre is_valid(tid)
     *
     * @returns true if cap was added, false if cap slot not available.
     */
    bool add_thread_cap(threadid_t tid, tcb_t *tcb);

    /**
     * Remove a thread cap from the clist
     *
     * @pre is_valid(tid), thread locked
     *
     * @returns true if cap was removed, false if cap not found.
     */
    bool remove_thread_cap(threadid_t tid);

    /**
     * Add a ipc cap to the clist
     *
     * @pre is_valid(tid)
     *
     * @returns true if cap was added, false if cap slot not available.
     */
    bool add_ipc_cap(threadid_t tid, tcb_t *tcb);

    /**
     * Remove a ipc cap from the clist
     *
     * @pre lookup_ipc_cap()
     *
     * @returns true if cap was removed, false if cap not found.
     */
    bool remove_ipc_cap(threadid_t tid);

    /**
     * Initialize clist
     */
    void init(word_t num_ids) {
        ASSERT(DEBUG, num_ids > 0);
        max_id = num_ids - 1;
        num_spaces = 0;
        list_lock.init();
    }

    /**
     * Get size of clist
     */
    word_t num_entries(void) {
        return max_id + 1;
    }

    /**
     * Add an address space as using this clist
     */
    void add_space(space_t *space) { num_spaces ++; };

    /**
     * Remove an address space using this clist
     */
    void remove_space(space_t *space) { num_spaces --; };

    /**
     * Get number of spaces using clist
     */
    word_t get_space_count(void) {
        return num_spaces;
    }

    /**
     * Check if any caps remaining in the clist
     *
     * @returns true if clist empty, false if clist not empty
     */
    bool is_empty(void);

private:
    friend void init_root_clist(void);
    friend void mkasmsym();

    word_t max_id;
    word_t num_spaces;
    spinlock_t list_lock;

    /*lint -e1501 */
    cap_t entries[0];
};

/*
 * The Capability List lookup table
 */
class clistid_lookup_t : private id_lookup_t
{
public:
    bool is_valid(clistid_t id) {
        return valid_id(id.get_clistno());
    }

    clist_t * lookup_clist(clistid_t id) {
        return (clist_t*)lookup_id(id.get_clistno());
    }

public:
    void add_clist(clistid_t id, clist_t* clist) {
        add_object(id.get_clistno(), clist);
    }

    void remove_clist(clistid_t id) {
        remove_object(id.get_clistno());
    }
private:
    friend void init_clistids(void);
};

/* Access functions */

INLINE clistid_lookup_t *
get_clist_list()
{
    extern clistid_lookup_t clist_lookup;
    return &clist_lookup;
}

INLINE clist_t *get_root_clist(void)
{
    extern clist_t *root_clist;

    return root_clist;
}

void init_clistids(void);

#endif /* !__CLISTH__ */
