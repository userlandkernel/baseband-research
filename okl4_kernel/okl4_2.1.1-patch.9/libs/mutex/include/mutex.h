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

#ifndef __LIBMUTEX__MUTEX_H__
#define __LIBMUTEX__MUTEX_H__

#include <l4/thread.h>

#if !defined(CONFIG_USER_MUTEXES)
#include <l4/mutex.h>
#endif

#if !defined(CONFIG_KERNEL_MUTEXES)
#include <atomic_ops/atomic_ops.h>
#endif

/**
 *  Mutex structure, the instance is specific to the type of mutex
 *  being provided by OKL4, either a compiletely user level
 *  implementation or one provided by the kernel.
 */
#if defined(CONFIG_HYBRID_MUTEXES)

/* Note: if the position of state changes,
 * qdsp6/libs/mutex/hybrid/mutex.spp
 * should be updated */
struct okl4_mutex {
    L4_MutexId_t id; /** The identifier of the kernel object of the mutex. */
    okl4_atomic_word_t state; /** Mutex status and holder, accessed by kernel. */
    volatile word_t count; /** The number of times the holer has locked the mutex. */
};

#elif defined(CONFIG_USER_MUTEXES)

struct okl4_mutex {
    okl4_atomic_word_t holder; /** The identifier of the thread holding the mutex. */
    word_t needed; /** Is the mutex needed by any other threads. */
    volatile word_t count; /** The number of times the holder has locked the mutex. */
};

#elif defined(CONFIG_KERNEL_MUTEXES)

struct okl4_mutex {
    L4_MutexId_t id; /** The kernel identifier of object backing the mutex. */
    volatile word_t holder; /** The identifier of the thread holding the mutex. */
    volatile word_t count; /** The number of times the holder has locked the mutex. */
};

#else
#error No mutex type configured!
#endif

/** The abstract data type to reference mutex objects by. */
typedef struct okl4_mutex * okl4_mutex_t;


/**
 *  @file
 *
 *  Mutex creation / deletion.
 *
 *  Mutexes must be explicitly created and deleted. (Using memset() is
 *  not sufficient for kernel-based mutexes, for instance.)
 *
 *  These function calls may perform an IPC to iguana if kernel-based
 *  mutexes are in use to perform allocation/deallocation of kernel
 *  Mutexes.
 */

/**
 *  Initialise a mutex object for use.
 *
 *  @param mutex The mutex object to be initialised.
 *
 *  @return Completion status.
 *  @retval 0 Initialisation successful.
 */
int okl4_mutex_init(okl4_mutex_t mutex);

/**
 *  Tear down a mutex object.
 *
 *  Freeing a mutex object does not free the okl4_mutex structure.
 *
 *  @param mutex The mutex object to be torn down.
 *
 *  @return Completion status.
 *  @retval 0 Tear-down successful.
 */
int okl4_mutex_free(okl4_mutex_t mutex);

/**
 *  Acquire a mutex without maintaining a count.
 *
 *  Attempting to double-lock a single mutex with this function will
 *  cause a thread to deadlock on itself.
 *  
 *  @param mutex The mutex object to be acquired.
 */
void okl4_mutex_lock(okl4_mutex_t mutex);

/**
 *  Try to acquire a mutex without maintaining a count.
 *
 *  @param mutex The mutex object to attempt to acquire.
 *
 *  @retval 0 mutex acquired.
 *  @retval 1 mutex already held.
 */
int okl4_mutex_trylock(okl4_mutex_t mutex);

/**
 *  Release a mutex without maintaining a count.
 *
 *  @param mutex The mutex object to be released.
 */
void okl4_mutex_unlock(okl4_mutex_t mutex);

/**
 *  Acquire a mutex recursively, i.e. with a count.
 *
 *  @param mutex The mutex object to be acquired.
 */
void okl4_mutex_count_lock(okl4_mutex_t mutex);

/**
 *  Try to acquire a mutex recursively, i.e. with a count.
 *
 *  @param mutex The mutex object to attempt to acquire.
 *
 *  @retval 0 mutex acquired.
 *  @retval 1 mutex already held.
 */
int okl4_mutex_count_trylock(okl4_mutex_t mutex);

/**
 *  Relase a mutex recursively, i.e. with a count.
 *
 *  Mutex is not completely released until the holder has released as
 *  many times as he has acquired the mutex or mutex_release() is
 *  called.
 *
 *  @param mutex The mutex object to be released.
 */
void okl4_mutex_count_unlock(okl4_mutex_t mutex);


#endif /* __LIBMUTEX__MUTEX_H__ */
