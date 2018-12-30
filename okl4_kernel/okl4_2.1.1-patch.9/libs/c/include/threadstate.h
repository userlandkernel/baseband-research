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

#ifndef _THREADSTATE_H_
#define _THREADSTATE_H_

#ifdef THREAD_SAFE
#include <stdlib.h>
#include <unistd.h>

/* We define the opaque pthread types again, if they haven't already been
 * defined, since they is a circular inclusion condition on including sys/types.h
 */
#ifdef __USE_POSIX
#include <sys/types.h>
#ifndef __POSIX_PTHREAD_DEFINED
#define __POSIX_PTHREAD_DEFINED
typedef struct pthread_i * pthread_t;
typedef struct pthread_attr_i * pthread_attr_t;
typedef struct pthread_key_i * pthread_key_t;
/* pthread_once_t is not a pointer - see comments in pthread_internal.h */
typedef struct _pthread_once_t { L4_Word_t padding[6]; } pthread_once_t;
/* pthread_mutex_t is not a pointer - see comments in pthread_internal.h */
typedef struct _pthread_mutex_t { L4_Word_t padding[4]; } pthread_mutex_t;
typedef struct pthread_mutexattr_i * pthread_mutexattr_t;
/* pthread_cond_t is not a pointer - see comments in pthread_internal.h */
typedef struct _pthread_cond_t { L4_Word_t padding[6]; } pthread_cond_t;
typedef struct pthread_condattr_i * pthread_condattr_t;
#endif
#endif

struct thread_state {
    char **_environ;
#ifdef THREAD_SAFE
    int errno;
#endif

#ifdef __USE_POSIX
    pthread_t pthread;
#endif
};

struct thread_state *thread_state_get_base(void);

inline static char ***
thread_state_get_environ(void)
{
    char ***environ_p;
   
    environ_p = &thread_state_get_base()->_environ;

    if (*environ_p == NULL)
    {
        *environ_p = malloc(sizeof(char *));
        **environ_p = NULL;
    }

    return environ_p;
}

#ifdef THREAD_SAFE
inline static int *
thread_state_get_errno(void)
{
    return &((thread_state_get_base())->errno);
}
#endif
#endif

/* POSIX pthread_state related functions */

#ifdef __USE_POSIX

inline static pthread_t
thread_state_get_pthread(void)
{
    return (thread_state_get_base())->pthread;
}

#endif

#endif /* _THREADSTATE_H_ */
