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

#include <l4/types.h>
#include <mutex/mutex.h>

/** @todo FIXME: Add prototypes with doxygen comments - awiggins. */

#if defined(CONFIG_HYBRID_MUTEXES)

#define OKL4_MUTEX_CONTENTION_BIT (WORD_T_BIT - 1)

#define OKL4_MUTEX_FREE (~(word_t)0)
#define OKL4_MUTEX_CONTENDED ((word_t)1 << OKL4_MUTEX_CONTENTION_BIT)

#else /* !CONFIG_HYBRID_MUTEXES */

#define OKL4_MUTEX_FREE (L4_nilthread.raw)

#endif /* CONFIG_HYBRID_MUTEXES */

/**
 *  @file Mutex libraries private utility functions.
 *
 *  @internal
 */

/**
 *  Find the unique thread handle used to identify calling thread as a
 *  mutex holder to the kernel and the other threads sharing the mutex
 *  library.
 *
 *  @return Raw word representing caller threads thread handle for
 *  mutex library.
 */
INLINE word_t okl4_mutex_my_handle(void);

/**
 *  Return the thread handle for the current holder of argument mutex object.
 *
 *  @param mutex The mutex object being inspected.
 *
 *  @retval L4_nilthread.raw Mutex object is unlocked.
 *  @return The mutex thread handle of the current holder of the mutex.
 */
INLINE word_t okl4_mutex_get_holder(okl4_mutex_t mutex);

/**
 *  Initialises a mutex object's holder member to unlocked.
 *
 *  @param mutex The mutex object to initialise.
 */
INLINE void okl4_mutex_init_holder(okl4_mutex_t mutex);

/**
 *  Is the mutex object currently unlocked?
 *
 *  @param mutex The mutex object being inspected.
 *
 *  @return Is the mutex object is unlocked?
 */
INLINE int okl4_mutex_is_unlocked(okl4_mutex_t mutex);

/**
 *  Is the caller the current holder of the argument mutex object?
 *
 *  @param mutex The mutex object being inspected.
 *
 *  @return okl4_mutex_get_holder() == okl4_mutex_my_handle()
 */
INLINE int okl4_mutex_am_holder(okl4_mutex_t mutex);

#if defined(CONFIG_HYBRID_MUTEXES)

/** @todo FIXME: Doxygen header - awiggins. */
INLINE int okl4_mutex_is_contended(okl4_mutex_t mutex);

/** @todo FIXME: Doxygen header - awiggins. */
INLINE word_t okl4_mutex_extract_holder(okl4_atomic_plain_word_t state);

INLINE int
okl4_mutex_is_contended(okl4_mutex_t mutex)
{
    /* Contended bit is inverse logic, clear when mutex is contended. */
    return (okl4_atomic_read(&(mutex->state)) & OKL4_MUTEX_CONTENDED) == 0;
}

INLINE word_t
okl4_mutex_extract_holder(okl4_atomic_plain_word_t state)
{
    /* Turn the state into a thread handle. */
    return state | OKL4_MUTEX_CONTENDED;
}

INLINE word_t
okl4_mutex_get_holder(okl4_mutex_t mutex)
{
    return okl4_mutex_extract_holder(okl4_atomic_read(&(mutex->state)));
}

INLINE void
okl4_mutex_init_holder(okl4_mutex_t mutex)
{
    okl4_atomic_init(&(mutex->state), OKL4_MUTEX_FREE);
}

#elif defined(CONFIG_KERNEL_MUTEXES)

INLINE word_t
okl4_mutex_get_holder(okl4_mutex_t mutex)
{
    return mutex->holder;
}

INLINE void
okl4_mutex_init_holder(okl4_mutex_t mutex)
{
    mutex->holder = OKL4_MUTEX_FREE;
}

#elif defined(CONFIG_USER_MUTEXES)

INLINE word_t
okl4_mutex_get_holder(okl4_mutex_t mutex)
{
    return okl4_atomic_read(&(mutex->holder));
}

INLINE void
okl4_mutex_init_holder(okl4_mutex_t mutex)
{
    okl4_atomic_init(&(mutex->holder), OKL4_MUTEX_FREE);
}

#else
#error No mutex type configured!
#endif

INLINE word_t
okl4_mutex_my_handle(void)
{
    return L4_MyMutexThreadHandle().raw;
}

INLINE int
okl4_mutex_is_unlocked(okl4_mutex_t mutex)
{
    return okl4_mutex_get_holder(mutex) == OKL4_MUTEX_FREE;
}

INLINE int
okl4_mutex_am_holder(okl4_mutex_t mutex)
{
    return okl4_mutex_get_holder(mutex) == okl4_mutex_my_handle();
}
