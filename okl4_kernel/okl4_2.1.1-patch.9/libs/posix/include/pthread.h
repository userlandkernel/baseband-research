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
 * Description: pthread.h as per IEEE 1003.13 (POSIX)
 */

#ifndef _POSIX_PTHREAD_H_
#define _POSIX_PTHREAD_H_

/* Limits for max threads, max keys etc. */
#include <limits.h>

/* Get all C sys types - includes POSIX specific */
#include <sys/types.h>

/* Default stack size, in bytes */
#define PTHREAD_DEFAULT_STACK_SIZE (1024 * 1024)

/* Thread status
 * Default is active*/
#define PTHREAD_STATUS_ACTIVE 0
#define PTHREAD_STATUS_TERMINATED 1
#define PTHREAD_STATUS_GENESIS 2 /* Pthread has been created, but hasn't yet had
                                    had a chance to initialize its TLS etc. */

/* Detach status / creation arguments
 * Default is joinable */
#define PTHREAD_CREATE_DETACHED 0
#define PTHREAD_CREATE_JOINABLE 1
#define PTHREAD_DETACHED PTHREAD_CREATE_DETACHED
#define PTHREAD_JOINABLE PTHREAD_CREATE_JOINABLE

/* On a successful cancellation request, the exit status
   of the PThread is set to PTHREAD_CANCELED.
   It must expand to a void* whose value matches no pointer
   to an object in memory nor the value NULL. */
#define PTHREAD_CANCELED ((void *) 1)

/* Cancelability state - enabled or disable */
#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1

/* Cancelability type - async or deferred.
 * Default is async */
#define PTHREAD_CANCEL_ASYNCHRONOUS 0
#define PTHREAD_CANCEL_DEFERRED 1

/* Cancel request */
#define PTHREAD_CANCEL_NOREQUEST 0
#define PTHREAD_CANCEL_REQUEST 1

/* pthread_once initialization */
#define PTHREAD_ONCE_CONTROL_DONE 1
#define PTHREAD_ONCE_CONTROL_NOTDONE 0
#define PTHREAD_ONCE_CONTROL_MAGIC 0xFADE0911
#define PTHREAD_ONCE_INIT {{PTHREAD_ONCE_CONTROL_MAGIC, 0, 0, 0, PTHREAD_ONCE_CONTROL_NOTDONE, 0}}

/* pthread_mutex initialization */
#define PTHREAD_MUTEX_MAGIC 0x0119EDAF
//TODO: default attributes here, when we have them
#define PTHREAD_MUTEX_INITIALIZER {{PTHREAD_MUTEX_MAGIC, 0, 0, 0}}
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_ERRORCHECK 1
#define PTHREAD_MUTEX_NORMAL 2
#define PTHREAD_MUTEX_RECURSIVE 3

/* pthread_cond initialization */
#define PTHREAD_COND_MAGIC 0xEDAF0119
//TODO: default pthread cond attributes here, when we have them
#define PTHREAD_COND_INITIALIZER {{PTHREAD_COND_MAGIC, 0, 0, 0, NULL, NULL}}

/* Library initialization */
void __pthread_lib_init(void);
void __pthread_thread_init(pthread_t pthread);

/* Creation and Execution functions */
int pthread_create(pthread_t * restrict, const pthread_attr_t * restrict,
                   void *(*)(void *), void * restrict);
/* __pthread_state_new: main_tid is only valid if called from a server thread */
pthread_t __pthread_state_new(const pthread_attr_t * restrict);
int pthread_once(pthread_once_t *, void (*)(void));

/* Termination, Cleanup, and Deletion functions */
void pthread_exit(void *);
int pthread_join(pthread_t, void **);
int pthread_detach(pthread_t);
void pthread_cleanup_push(void (*)(void *), void *);
void pthread_cleanup_pop(int);

/* Identity and Comparison functions */
int pthread_equal(pthread_t, pthread_t);
pthread_t pthread_self(void);

/* Attribute functions */
int pthread_attr_init(pthread_attr_t *);
int pthread_attr_destroy(pthread_attr_t *);
int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
int pthread_attr_setdetachstate(pthread_attr_t *, int);
int pthread_attr_getstackaddr(const pthread_attr_t * restrict, void ** restrict);
int pthread_attr_setstackaddr(pthread_attr_t *, void *);
int pthread_attr_getstacksize(const pthread_attr_t * restrict, size_t * restrict);
int pthread_attr_setstacksize(pthread_attr_t *, size_t);

/* Key functions */
int pthread_key_create(pthread_key_t *, void (*)(void *));
int pthread_key_delete(pthread_key_t);
void * pthread_getspecific(pthread_key_t);
int pthread_setspecific(pthread_key_t, const void *);

/* Cancellation functions */
int pthread_cancel(pthread_t);
int pthread_setcancelstate(int, int *);
int pthread_setcanceltype(int, int *);
void pthread_testcancel(void);

/* Mutexes */
int pthread_mutex_init(pthread_mutex_t *restrict, const pthread_mutexattr_t *restrict);
int pthread_mutex_destroy(pthread_mutex_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);

/* Mutexes Unimplemented:
int pthread_mutex_timedlock(pthread_mutex_t *, const struct timespec *);
int pthread_mutex_getprioceiling(const pthread_mutex_t *restrict, int *restrict);
int pthread_mutex_setprioceiling(pthread_mutex_t *restrict, int, int *restrict);
int pthread_mutexattr_init(pthread_mutexattr_t *);
int pthread_mutexattr_destroy(pthread_mutexattr_t *);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
*/

/* Condition Variables */
int pthread_cond_init(pthread_cond_t *restrict, const pthread_condattr_t *restrict);
int pthread_cond_destroy(pthread_cond_t *);
int pthread_cond_wait(pthread_cond_t *restrict, pthread_mutex_t *restrict);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_broadcast(pthread_cond_t *);

/* Condition Variables Unimplemented:
int pthread_cond_timedwait(pthread_cond_t *restrict, pthread_mutex_t *restrict, const struct timespec *restrict);
int pthread_condattr_destroy(pthread_condattr_t *);
int pthread_condattr_getclock(const pthread_condattr_t *restrict, clockid_t *restrict);
int pthread_condattr_getpshared(const pthread_condattr_t *restrict,int *restrict);
int pthread_condattr_init(pthread_condattr_t *);
int pthread_condattr_setclock(pthread_condattr_t *, clockid_t);
int pthread_condattr_setpshared(pthread_condattr_t *, int);
*/

#endif  /* !_POSIX_PTHREAD_H_ */
