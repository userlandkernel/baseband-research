/*
 * Copyright (c) 2006, National ICT Australia
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
 * K&R Malloc 
 *
 * System specifc code should implement `more_core'
 */
#include "k_r_malloc.h"
#include "malloc.h"
#include <stddef.h>             /* For NULL */
#include <stdlib.h>
#include <string.h>             /* For memcpy */
#include <errno.h>
#include <assert.h>

#ifdef THREAD_SAFE
#include <mutex/mutex.h>
struct okl4_mutex malloc_mutex;
#endif

Header __malloc_base;             /* empty list to get started */


#ifdef CONFIG_MALLOC_INSTRUMENT
size_t __malloc_instrumented_allocated;
#endif

#ifdef CONFIG_MALLOC_DEBUG_INTERNAL
#include <stdio.h>
int __malloc_check(void);
void __malloc_dump(void);
#endif

/*
 * malloc: general-purpose storage allocator 
 */
void *
malloc(size_t nbytes)
{
    Header *p, *prevp;
    unsigned nunits;

    if (nbytes == 0) {
        return NULL;
    }

    nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

    MALLOC_LOCK;
    if ((prevp = freep) == NULL) {      /* no free list yet */
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {      /* big enough */
            if (p->s.size == nunits)    /* exactly */
                prevp->s.ptr = p->s.ptr;
            else {              /* allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
#ifdef CONFIG_MALLOC_DEBUG
            {
                /* Write bit pattern over data */
                char *x = (char *)(p + 1);
                int i;

                for (i = 0; i < nbytes; i++)
                    x[i] = 0xd0;
            }
#endif

#ifdef CONFIG_MALLOC_INSTRUMENT
            __malloc_instrumented_allocated += nunits;
#endif
#ifdef CONFIG_MALLOC_DEBUG_INTERNAL
            if (__malloc_check() != 0) {
                printf("malloc %u %p\n", nbytes, (void *)(p + 1));
                __malloc_dump();
                assert(__malloc_check() == 0);
            }
#endif
            MALLOC_UNLOCK;
            return (void *)(p + 1);
        }
        if (p == freep) {       /* wrapped around free list */
            if ((p = (Header *)morecore(nunits)) == NULL) {
                MALLOC_UNLOCK;
                errno = ENOMEM;
                return NULL;    /* none left */
            } else {

            }
        }
    }
    MALLOC_UNLOCK;
}

#ifdef CONFIG_MALLOC_DEBUG_INTERNAL

int
__malloc_check(void)
{
    Header *p, *prevp;

    if ((prevp = freep) == NULL) {      /* no free list yet */
        return 0;
    }
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
        if ((void *)p == NULL) {
            return 1;
        }
        /* Free bits should be in order */
        if (p > p->s.ptr && p != &base && p->s.ptr != &base) {
            return 1;
        }
        if ((uintptr_t)p + (p->s.size * sizeof(Header)) > (uintptr_t)p->s.ptr &&
            p != &base && p->s.ptr != &base) {
            return 1;
        }
        /* shouldn't have zero sized free bits */
        if (p->s.size == 0 && p != &base) {
            return 1;
        }
        if (p == freep) {       /* wrapped around free list */
            break;
        }
    }
    return 0;
}

void
__malloc_dump(void)
{
    Header *p, *prevp;

    if ((prevp = freep) == NULL) {      /* no free list yet */
        return;
    }
    printf("Malloc dump\n");
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
        if (p > p->s.ptr && p != &base) {
            printf("* ");
        }
        if (p->s.size == 0 && p != &base) {
            printf("# ");
        }
        if ((uintptr_t)p + (p->s.size * sizeof(Header)) > (uintptr_t)p->s.ptr &&
            p != &base) {
            printf("$ ");
        }
        if (p == &base) {
            printf(" p: <base>\n");
        } else {
            printf(" p: %p (%d) -> %p\n", p, p->s.size, p->s.ptr);
        }
        assert(p != NULL);
        if (p == freep) {       /* wrapped around free list */
            return;
        }
    }
}
#endif
