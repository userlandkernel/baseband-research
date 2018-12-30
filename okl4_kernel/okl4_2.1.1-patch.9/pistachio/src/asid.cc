/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description: Generic L4 ASID management
 */
#include <kernel/l4.h>
#include <kernel/space.h>
#include <kernel/tcb.h>

/* Only compile if ASIDS required */
#if defined(CONFIG_MAX_NUM_ASIDS)

#if !defined(CONFIG_ASIDS_DIRECT)
asid_t * asid_cache_t::lookup(hw_asid_t hw_asid)
{
    space_t *space;
    if (asids[hw_asid].asid == (space_ref)~0) {
        space = get_kernel_space();
    } else {
        space = get_space_list()->lookup_space(spaceid_t::spaceid(asids[hw_asid].asid));
        if (space == NULL) {
            return (asid_t*)-1;
        }
    }
    return space->get_asid();
}
#endif

hw_asid_t asid_cache_t::allocate(space_t *space, asid_t *asid)
{
    ASSERT(DEBUG, !asid->is_valid());
    hw_asid_t idx = asid_t::invalid;

    asid_lock.lock();
#ifdef CONFIG_ASIDS_LRU
    if (first_free == -1) {
        idx = asids[lru_head].prev;
        move_asid(idx, true, false);

        asid_t * old = get_asid_cache()->lookup(idx);
        old->preempt();
    } else {
        idx = first_free;
        first_free = asids[first_free].next;
    }

    move_asid(idx, false, true);
#endif /* CONFIG_ASIDS_LRU */

#ifdef CONFIG_ASIDS_ROUNDR
    static hw_asid_t next = 0;
    space_t *cur_spc = get_current_space();

    if (!cur_spc) {
        cur_spc = get_kernel_space();
    }
    hw_asid_t current = cur_spc->get_asid()->value();

again:
    idx = next;

#if defined(CONFIG_ASIDS_DIRECT)
    while (asids[idx].asid == (asid_t*)-1)
#else
    while (asids[idx].asid == INVALID_REF)
#endif
    {
        idx ++;
        if (idx >= CONFIG_MAX_NUM_ASIDS) {
            idx = 0;
        }
    }
    next = idx + 1;
    if (next >= CONFIG_MAX_NUM_ASIDS) {
        next = 0;
    }
    if (idx == current) {
        goto again;
    }

    if (asids[idx].asid != FREE_REF) {
        asid_t * old = get_asid_cache()->lookup(idx);
        old->preempt();
    }

#endif /* CONFIG_ASIDS_ROUNDR */

#ifdef CONFIG_ASIDS_RANDR
#error fixme
#endif

#ifdef CONFIG_ASIDS_STATIC
    static hw_asid_t next = 0;
    word_t count = 0;

    idx = next;

#if defined(CONFIG_ASIDS_DIRECT)
    while (asids[idx].asid == (asid_t*)-1)
#else
    while ((asids[idx].asid == INVALID_REF) || (asids[idx].asid != FREE_REF))
#endif
    {
        idx ++;
        count ++;
        if (idx >= CONFIG_MAX_NUM_ASIDS) {
            idx = 0;
        }
        if (count >= CONFIG_MAX_NUM_ASIDS) {
            panic("run out of free asids");
        }
    }
    next = idx + 1;
    if (next >= CONFIG_MAX_NUM_ASIDS) {
        next = 0;
    }

#endif

#if defined(CONFIG_ASIDS_DIRECT)
    asids[idx].asid = asid;
#else
    asids[idx].asid = (space_ref)space->get_space_id().get_spaceno();
#endif

    asid_lock.unlock();
    return idx;
}

void asid_cache_t::set_valid(hw_asid_t start, hw_asid_t end)
{
    hw_asid_t idx;
    asid_lock.lock();

#if defined(CONFIG_ASIDS_LRU)
    hw_asid_t *prev, old_head;

    old_head = first_free;
    prev = &first_free;
#endif

    for (idx = start; idx <= end; idx++)
    {
#if defined(CONFIG_ASIDS_DIRECT)
        if (asids[idx].asid != (asid_t *) -1)
            continue;
#else
        if (asids[idx].asid != INVALID_REF)
            continue;
#endif

        asids[idx].asid = FREE_REF;
#if defined(CONFIG_ASIDS_LRU)
        *prev = idx;
        prev = &asids[idx].next;
#endif
    }

#if defined(CONFIG_ASIDS_LRU)
    *prev = old_head;
#endif
    asid_lock.unlock();
}

#endif /* !CONFIG_MAX_NUM_ASIDS */
