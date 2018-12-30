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
 * Read/Write Lock
 */
#ifndef __READ_WRITE_LOCK_H__
#define __READ_WRITE_LOCK_H__

#if defined(CONFIG_MDOMAINS) ||  defined(CONFIG_MUNITS)

/* Include the architecture-specific read/write lock. */
#include <kernel/arch/read_write_lock.h>

#else /* !CONFIG_MDOMAINS && !CONFIG_MUNITS */

/**
 * Read/Write lock.
 *
 * Only a single writer may hold the lock at any time, but an
 * unlimited number of reader. If a writing thread is waiting
 * on the lock, they have preference over the reading threads.
 */
struct read_write_lock_t
{
    /** Initialise the read/write lock. */
    void init();

    /**
     * Acquire this lock for reading. Other readers may still acquire
     * the lock.
     */
    void lock_read(void);

    /**
     * Try acquire this lock for reading. Other readers may still acquire
     * the lock.
     *
     * @returns true if lock aquired, false if lock held
     */
    bool try_lock_read(void);

    /**
     * Release this lock for reading.
     */
    void unlock_read(void);

    /**
     * Acquire this lock for writing. The lock will not be acquired
     * until all other readers or writers have released the lock. No new
     * readers will be allowed to take the lock until we fully acquire
     * it and release it.
     */
    void lock_write(void);

    /**
     * Try aquire writer lock
     *
     * @returns true if lock aquired, false if lock held
     */
    bool try_lock_write(void);

    /**
     * Release the lock for writing.
     */
    void unlock_write(void);

    /** Determine if the lock is currently held by any thread. */
    bool is_locked(void);

#if defined(CONFIG_DEBUG)
    word_t locked;
#endif
};

INLINE void
read_write_lock_t::init()
{
#if defined(CONFIG_DEBUG)
    locked = 0;
#endif
}

INLINE void
read_write_lock_t::lock_read(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked >= 0);
    locked++;
#endif
}

INLINE bool
read_write_lock_t::try_lock_read(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked >= 0);
    locked++;
#endif
    return true;
}

INLINE void
read_write_lock_t::unlock_read(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked > 0);
    locked--;
#endif
}

INLINE void
read_write_lock_t::lock_write(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked == 0);
    locked = (word_t)-1;
#endif
}

INLINE bool
read_write_lock_t::try_lock_write(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked == 0);
    locked = (word_t)-1;
#endif
    return true;
}

INLINE void
read_write_lock_t::unlock_write(void)
{
#if defined(CONFIG_DEBUG)
    ASSERT(ALWAYS, (int)locked == -1);
    locked = 0;
#endif
}

INLINE bool
read_write_lock_t::is_locked(void)
{
#if defined(CONFIG_DEBUG)
    return locked;
#else
    return false;
#endif
}

/** Import an externally-declared read_write_lock. */
#define DECLARE_READ_WRITE_LOCK(name) extern read_write_lock_t name

/** Definie a statically-allocated read_write_lock. */
#if defined(_lint)
#define DEFINE_READ_WRITE_LOCK(name) \
        read_write_lock_t name
#elif defined(CONFIG_DEBUG)
#define DEFINE_READ_WRITE_LOCK(name) \
        read_write_lock_t name = \
                ((read_write_lock_t){0})
#else
#define DEFINE_READ_WRITE_LOCK(name) \
        read_write_lock_t name
#endif

#endif /* !CONFIG_MDOMAINS && !CONFIG_MUNITS */

#endif /* !__READ_WRITE_LOCK_H__ */

