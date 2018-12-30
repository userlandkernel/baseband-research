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
 * Author: Josh Matthews <jmatthews@ok-labs.com>
 * Created: Friday December 14 2007
 * Description: pthread internal definitions
 */

#ifndef PTHREAD_INTERNAL_H_
#define PTHREAD_INTERNAL_H_

#include <l4/types.h>
#include <mutex/mutex.h>

/* Cleanup routine stack */
struct pthread_cleanup_routine
{
    void (*routine) (void*);
    void* arg;
    struct pthread_cleanup_routine * next;
};

/* Joined threads stack */
struct pthread_joined_thread
{
    pthread_t joined;
    struct pthread_joined_thread * next;
};

/* Instance of a key/data for a particular thread */
struct pthread_key_instance
{
    /* Pointer to the data stored for the key in this thread */
    void* data;

    /* Destructor routine funtion pointer */
    void (*destructor)(void*);

    /* Generation counter value at the time the associated key was created */
    unsigned int generation;
};

/* Struct for a linked list of waiters on a condition variable */
struct pthread_cond_waiter
{
    pthread_t pthread;
    struct pthread_cond_waiter * next;
};

/* Internal Pthread struct */
struct pthread_i
{
    /* The L4 thread id */
    L4_ThreadId_t l4tid;

    /* Mutex for synched acccess to the below fields */
    okl4_mutex_t mutex;

    /* Status of the thread - terminated, active, or in creation (genesis).
       Volatile so it can be spun on to wait for a thread to initialize
      (status moves from GENESIS to ACTIVE) */
    volatile unsigned termination_state:2;

    /* Return value from the function that started
     * the thread, or the value passed to pthread_exit,
     * or PTHREAD_CANCELED if cancelled */
    void* exit_status;

    /* Detach status - detached or joinable */
    unsigned detach_status:1;

    /* Threads waiting on us - NULL if no joined threads */
    struct pthread_joined_thread * joined_stack;

    /* Thread we are waiting on (i.e. joined to) */
    pthread_t joinee_thread;

    /* Cancelability state - enabled or disabled */
    unsigned cancel_state:1;

    /* Cancelability type - async or deferred */
    unsigned cancel_type:1;

    /* Buffered cancelation request - true or false */
    unsigned cancel_request:1;

    /* Cleanup routine stack */
    struct pthread_cleanup_routine * cleanup_stack;

    /* Keys array */
    struct pthread_key_instance * keys[PTHREAD_KEYS_MAX];

    /* Pthread_once execution stack */
    struct pthread_once_i * once_exec_stack;

    /* Change these should list implementation change */
    struct pthread_i * next;
    struct pthread_i * previous;

    /* Flag indicating presence in above linked list */
    unsigned in_list:1;
};

/* 
 * Pthread attribute is an opaque type, combining
 * all possible types of attributes.
 * If a new attribute type is added (along with its
 * corresponding set/get routine) its fields should be added here.
 */
struct pthread_attr_i
{
    /* Detach state */
    unsigned detachstate:1;

    /* Stack address */
    void* stackaddr;

    /* Stack size */
    size_t stacksize;
};

/*
 * Pthread key is an opaque type.
 * A Pthread key stores its position in the keys array
 * (the same for all threads) and the value of the generation
 * counter at time of creation of the key.
 * _POSIX_THREAD_KEYS_MAX pthread key objects are statically
 * initialized on library initialization and are stored in used/free list.
 */
#define __PTHREAD_KEY_FREE 0
#define __PTHREAD_KEY_USED 1
struct pthread_key_i
{
    unsigned int slot;
    void (*destructor)(void *);
    unsigned int generation;
    unsigned int status;
    struct pthread_key_i *next;
};

/* Pthread_once is an opaque type.
 * It's a bit different to the above because we need to be able to statically
 * initialize it via a macro accessible to the users.
 * We declare it correctly here, but the user-visible declaration will
 * just be an array of bytes of appropriate length to accommodate the below.
 * If the definition here changes, _make sure_ that the length of the user-visible
 * type is large enough.
 */
struct pthread_once_i
{
    L4_Word_t magic; /* Just for sanity checking */
    struct okl4_mutex mutex;
    L4_Word_t status; /* Only a bit, but we make it a word because mutex is 3 words */
    struct pthread_once_i * next; /* Next in execution stack, so we can release locks
                                   if cancelled in execution */
};

/* Pthread_mutex is an opaque type.
 * As with pthread_once_i, it needs to be possible to statically initialize it.
 * If the definition here changes, ensure that the length of the user-visible type
 * is large enough.
 */
struct pthread_mutex_i
{
    L4_Word_t magic; /* Just for sanity checking */
    struct okl4_mutex mutex;
    //TODO: add attributes here
};

struct pthread_cond_i
{
    L4_Word_t magic;
    /* We don't strictly need this mutex, since callers should always be holding
     * the same mutex. But we don't want completely undefined behaviour
     * (eg: corrupted memory in the internal pthread lists) if the callers mess up. */
    struct okl4_mutex mutex;
    struct pthread_cond_waiter * first_waiter;
    struct pthread_cond_waiter * last_waiter;
};

#endif /* !PTHREAD_INTERNAL_H_ */
