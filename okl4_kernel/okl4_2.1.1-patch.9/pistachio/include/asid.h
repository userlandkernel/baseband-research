/*
 * Copyright (c) 2002, University of New South Wales
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
 * Description:   Generic ASID management
 */

#ifndef __ASID_H__
#define __ASID_H__

/*
 * This file contains generic routines for ASID management.  Hardware usually
 * supports a limited number of ASIDs (21264 has ~ 256), and the number of
 * address spaces may be greater than this.  The implication of this is that
 * an ASID may need to be used by a number of address spaces (only one space
 * at a time), and hence need to be managed like any other resource.
 *
 * This means that ASIDs may be pre-empted and managed in a LRU manner
 * (or equivelent).
 */

#include <kernel/debug.h>
#include <kernel/config.h>      /* CONFIG_S */
#include <kernel/sync.h>
#include <kernel/types.h>
#include <kernel/arch/asid.h>

class space_t;

/*
 * ASID Configurations
 *
 * CONFIG_MAX_NUM_ASIDS :   Maximum number of ASIDS supported.
 *                          Runtime may choose to use less
 *
 * -- replacement algorithm --
 *
 * CONFIG_ASIDS_STATIC :    Do not use any replacement / preemption.
 *
 * CONFIG_ASIDS_LRU :       Use least recently used replacement
 *                          algorithm.
 *
 * CONFIG_ASIDS_ROUNDR :    Use round robin replacement algorithm.
 *
 * CONFIG_ASIDS_RANDR :     Use random replacement algorithm.
 *
 * -- datastructures --
 *
 * CONFIG_ASIDS_DIRECT :    Use direct ASID pointers.
 *                          Faster but more memory.
 *
 * CONFIG_ASIDS_INDIRECT :  Use space_ids as indirect ASID references.
 *                          Slower but less memory.
 *
 * -- Miscellaneous --
 *
 * KERNEL_ASID :            ASID reserved for the kernel (if defined)
 *
 */

#if defined(CONFIG_ASIDS_LRU) || defined(CONFIG_ASIDS_ROUNDR) || defined(CONFIG_ASIDS_RANDR)

#if defined(CONFIG_MUNITS)
#error SMT asid preemption not supported
#endif

#define CONFIG_PREEMPT_ASIDS

#elif defined(CONFIG_ASIDS_STATIC)

#undef  CONFIG_PREEMPT_ASIDS

#else
#error ASID algorithm not selected
#endif

#if !defined(CONFIG_ASIDS_DIRECT)

typedef u16_t space_ref;

#define INVALID_REF     ((space_ref)~2)
#define FREE_REF        ((space_ref)~1)

#endif

DECLARE_SPINLOCK(asid_lock);

class asid_t
{
public:
    enum valid_e {
        invalid = -1
    };

    void init(void) {
        asid = invalid;
    }

    /* For architectures that have a separate kernel address space. */
    void init_kernel(hw_asid_t kernel_asid) {
        asid = kernel_asid;
    }

    bool is_valid(void) {
        return asid != invalid;
    }

    /* This will allocate, reference, and return the asid */
    hw_asid_t get(space_t *space);

    void release(void);

    /* Methods asid_cache_t requires */
    /* This just returns the value */
    hw_asid_t value(void) {
        return asid;
    }

    void preempt(void)
    {
        flush_asid(asid);
        asid = invalid;
    }

private:
    hw_asid_t asid;
};

/* This implementation is pretty straight forward and could (should) be optimised */
class asid_cache_t
{
public:
    /**
     * init - Initialise ASID cache
     *
     * Description:  This method initialised the ASID cache.  Note that this
     * should be called before set_valid.
     **/
    void init(void) {
#if defined(CONFIG_ASIDS_LRU)
        first_free = lru_head = -1;
#endif

        /* Initialise all asids to invalid */
        for(int i = 0; i < CONFIG_MAX_NUM_ASIDS; i++) {
#if defined(CONFIG_ASIDS_LRU)
            asids[i].prev = asids[i].next = -1;
#endif
#if defined(CONFIG_ASIDS_DIRECT)
            asids[i].asid = (asid_t *) -1;
#else
            asids[i].asid = INVALID_REF;
#endif
        }
    }

    /**
     * set_valid - Sets a valid range of ASIDs
     * @start: The first valid asid (invlusive)
     * @end: The last valid asid (inclusive)
     *
     * Description: This method sets a range of ASIDs to
     * be valid (allocatable).  Note that the parameters are
     * inclusive.  Multiple invocations allowed.
     **/
    void set_valid(hw_asid_t start, hw_asid_t end);

    /**
     * reference - Used to reference ASIDs to enable efficient pre-emption
     * @asid: The ASID to reference.
     *
     * Description: This method manages the LRU list (if any) of ASIDS to enable
     *  more efficient target selection during ASID pre-emption.
     *
     **/
    void reference(asid_t *asid);

    /**
     * allocate - Allocates an ASID
     * @asid: The destination ASID (used during preemption).
     *
     */
    hw_asid_t allocate(space_t *space, asid_t *asid);

    asid_t *lookup(hw_asid_t hw_asid);

 private:
    void release(asid_t *asid);

    void move_asid(hw_asid_t idx, bool remove, bool insert);

    struct asid_link_t {
#if defined(CONFIG_ASIDS_DIRECT)
        asid_t *asid;
#else
        space_ref asid;
#endif
#if defined(CONFIG_ASIDS_LRU)
        hw_asid_t prev, next; /* index of previous and next asid */
#endif
    } asids[CONFIG_MAX_NUM_ASIDS];

#if defined(CONFIG_ASIDS_LRU)
    hw_asid_t first_free;
    hw_asid_t lru_head;
#endif

    friend class asid_t;
};

INLINE asid_cache_t *get_asid_cache(void)
{
    extern asid_cache_t asid_cache;
    return &asid_cache;
}

INLINE hw_asid_t asid_t::get(space_t *space)
{
    if (EXPECT_FALSE(!is_valid())) {
        asid = get_asid_cache()->allocate(space, this);
    } else {
#ifdef KERNEL_ASID
        if (this->asid != KERNEL_ASID)
#endif
            get_asid_cache()->reference(this);
    }

    return asid;
}

INLINE void asid_t::release(void)
{
    if (this->is_valid()) {
        asid_lock.lock();
        get_asid_cache()->release(this);
        flush_asid(asid);
        asid_lock.unlock();
        asid = invalid;
    }
}

#if defined(CONFIG_ASIDS_DIRECT)
INLINE asid_t * asid_cache_t::lookup(hw_asid_t hw_asid)
{
    return asids[hw_asid].asid;
}
#endif

/* Note: must be already locked! */
INLINE void asid_cache_t::release(asid_t *asid)
{
    hw_asid_t idx = asid->value();

    move_asid(idx, true, false);

#if defined(CONFIG_ASIDS_DIRECT)
    asids[idx].asid = 0;
#else
    asids[idx].asid = FREE_REF;
#endif

#if defined(CONFIG_ASIDS_LRU)
    asids[idx].next = first_free;

    first_free = idx;
#endif
}

INLINE void asid_cache_t::reference(asid_t *asid)
{
    asid_lock.lock();
    ASSERT(DEBUG, asid->is_valid());
    move_asid(asid->value(), true, true);
    asid_lock.unlock();
}

/**
 * move_asid - Moves an ASID to the top of the LRU list
 * @idx: The index of the ASID to be moved
 * @inserted: Whether the ASID is in the LRU list (and therefore whether it
 *  needs to be removed first)
 *
 * Description: This function manages the ASID LRU list.  It also encapsulates
 * all the LRU complexity, and hence allows for easy (and neat) conditional compilation.
 **/
/* NOTE: must be locked already! */
INLINE void asid_cache_t::move_asid(hw_asid_t idx, bool remove, bool insert)
{
#ifdef CONFIG_PREEMPT_LRU
    hw_asid_t prev, next;
    if (remove) {
        /* remove from list */
        prev = asids[idx].prev;
        next = asids[idx].next;

        /* sjw (02/08/2002): This may happen if a space deletes itself --- impossible? */
        if (lru_head == idx)
            lru_head = asids[idx].next;

        if (prev != next) {
            asids[prev].next = next;
            asids[next].prev = prev;
        } else {
            lru_head = -1;
        }
    }

    if (insert) {
        if (lru_head != -1) {
            prev = asids[lru_head].prev;
            next = lru_head;
        } else {
            /* sjw (02/08/2002): Slightly inefficient ... */
            next = prev = idx;
        }

        asids[next].prev = idx;
        asids[prev].next = idx;
        asids[idx].prev = prev;
        asids[idx].next = next;

        lru_head = idx;
    }
#endif /* CONFIG_PREEMPT_ASIDS */
}

#endif /* !__ASID_H__ */
