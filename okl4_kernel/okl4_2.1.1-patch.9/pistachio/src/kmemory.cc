/*
 * Copyright (c) 2002, 2003-2004, Karlsruhe University
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
 * Description:   very simple kernel memory allocator
 */
#include <l4.h>
#include <kmemory.h>
#include <debug.h>
#include <init.h>
#include <kdb/tracepoints.h>
#include <sync.h>

#ifdef CONFIG_ARCH_MIPS64
#include INC_PLAT(cache.h)
#endif
#if defined(CONFIG_ARCH_ARM)
#include <arch/globals.h>
#endif

//#define DEBUG_KMEM

#ifdef DEBUG_KMEM
# define ALLOC_TRACE    TRACE
# define FREE_TRACE     TRACE
#else
# define ALLOC_TRACE(x...)
# define FREE_TRACE(x...)
#endif

DECLARE_TRACEPOINT (KMEM_ALLOC);
DECLARE_TRACEPOINT (KMEM_FREE);

/* THE kernel memory allocator */
kmem_t kmem;

#if 0
#define KMEM_CHECK                                              \
{                                                               \
    __asm__ __volatile__ ("" ::: "memory");                     \
    word_t num = 0;                                             \
    word_t * ptr = kmem_free_list;                              \
    while(ptr)                                                  \
    {                                                           \
        ptr = (word_t*)(*ptr);                                  \
        num++;                                                  \
    }                                                           \
    if (num != free_chunks)                                     \
    {                                                           \
        TRACEF("inconsistent kmem list: %d != %d (free_chunks)" \
               "\ncaller=%p\n",                                 \
            num, free_chunks, __return_address());              \
        ptr = kmem_free_list;                                   \
        while(ptr) {                                            \
            printf("%p -> ", ptr);                              \
            ptr = (word_t*)*ptr;                                \
        }                                                       \
        enter_kdebug("broken kmem");                            \
    }                                                           \
}
#else
#define KMEM_CHECK
#endif

SECTION(SEC_INIT) void kmem_t::init(void * start, void * end)
{
#define ISIZE ((word_t) end - (word_t) start)
    TRACE_INIT ("kmem_init (%p, %p) [%d%c]\n", start, end,
                ISIZE >= GB (1) ? ISIZE >> 30 :
                ISIZE >= MB (1) ? ISIZE >> 20 : ISIZE >> 10,
                ISIZE >= GB (1) ? 'G' : ISIZE >= MB (1) ? 'M' : 'K');

    /* initialize members */
    kmem_free_list = NULL;
    free_chunks = 0;

    /* do the real work */
    free(start, (word_t)end - (word_t)start);
}

/* the stupid version */
void kmem_t::free(void * address, word_t size)
{
    word_t* p;
    word_t* prev, *curr;

    spinlock.lock();

    FREE_TRACE("kmem_free(%p, %x)\n", address, size);
    TRACEPOINT (KMEM_FREE,
                printf ("kmem_free (%p, %d [%d%c]), ip: %p\n",
                        address, size,
                        size >= GB (1) ? size >> 30 :
                        size >= MB (1) ? size >> 20 : size >> 10,
                        size >= GB (1) ? 'G' : size >= MB (1) ? 'M' : 'K',
                        __return_address()));

    KMEM_CHECK;

    size = max(size, (word_t)KMEM_CHUNKSIZE);
    ASSERT(NORMAL, (size % KMEM_CHUNKSIZE) == 0);

    for (p = (word_t*)address;
         p < ((word_t*)(((word_t)address) + size - KMEM_CHUNKSIZE));
         p = (word_t*) *p)
        *p = (word_t) p + KMEM_CHUNKSIZE; /* write next pointer */

    /* find the place to insert */
    for (prev = (word_t*) (void*)&kmem_free_list, curr = kmem_free_list;
         curr && (address > curr);
         prev = curr, curr = (word_t*) *curr);
    /* and insert there */
    FREE_TRACE("prev %p/%p, curr %p, p: %p, \n", prev, *prev, curr, p);
    *prev = (word_t) address; *p = (word_t) curr;

    /* update counter */
    free_chunks += (size/KMEM_CHUNKSIZE);
    FREE_TRACE("kmem: free chunks=%x\n", free_chunks);
    KMEM_CHECK;

    spinlock.unlock();
}


/* the stupid version */
void * kmem_t::alloc(word_t size, bool zero)
{
    word_t*     prev;
    word_t*     curr;
    word_t*     tmp;
    word_t      i;

    spinlock.lock();

    ALLOC_TRACE("%s(%d) kfl: %p\n", __FUNCTION__, size, kmem_free_list);
    TRACEPOINT_TB (KMEM_ALLOC,
                   printf ("kmem_alloc (%d [%d%c]), ip: %p\n",
                       size, size >= GB (1) ? size >> 30 :
                       size >= MB (1) ? size >> 20 : size >> 10,
                       size >= GB (1) ? 'G' : size >= MB (1) ? 'M' : 'K',
                       __return_address()),
                   "kmem_alloc (%d bytes), ip=%x\n",
                   size, (word_t)__return_address());
    KMEM_CHECK;

    size = max(size, (word_t)KMEM_CHUNKSIZE);
    ASSERT(NORMAL, (size % KMEM_CHUNKSIZE) == 0);

    for (prev = (word_t*)(void*) &kmem_free_list, curr = kmem_free_list;
         curr;
         prev = curr, curr = (word_t*) *curr)
    {
        ALLOC_TRACE("curr=%x\n", curr);
        /* properly aligned ??? */
        if (!((word_t) curr & (size - 1)))
        {
            ALLOC_TRACE("%s(%d):%d: curr=%x\n", __FUNCTION__,
                        size, __LINE__, curr);

            tmp = (word_t*) *curr;
            for (i = 1; tmp && (i < (size / KMEM_CHUNKSIZE)); i++)
            {
                ALLOC_TRACE("%s():%d: i=%d, tmp=%x\n",
                            __FUNCTION__, __LINE__, i, tmp);

                if ((word_t) tmp != ((word_t) curr + KMEM_CHUNKSIZE*i))
                {
                    ALLOC_TRACE("skip: %x\n", curr);
                    tmp = 0;
                    break;
                };
                tmp = (word_t*) *tmp;
            }
            if (tmp)
            {
                /* dequeue */
                *prev = (word_t) tmp;

                if (zero)
                {
#if defined(HAVE_FASTER_MEMZERO)
                    mem_zero(curr, size);
#else
                    /* zero the page */
                    word_t *p = curr;
                    do {
                        *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
                    } while ((word_t)p < (word_t)curr + size);
#endif
                }

                /* update counter */
                free_chunks -= (size/KMEM_CHUNKSIZE);

                /* successful return */
                ALLOC_TRACE("kmalloc: %x->%p (%d), kfl: %p, caller: %p\n",
                            size, curr, free_chunks, kmem_free_list, __return_address());
                KMEM_CHECK;

#if 0
                TRACEPOINT_TB (KMEM_ALLOC,, "kmem_alloc (%d bytes), ip=%x "
                                            "==> %p\n",
                                            size, __return_address(),
                                            curr);
#endif

                spinlock.unlock();

                return curr;
            }
        }
    }
#if 0
    word_t * tmp1 = kmem_free_list;
    while(tmp1) {
        printf("%p -> ", tmp1);
        tmp1 = (word_t*)*tmp1;
    }
#endif

    spinlock.unlock();
    return NULL;
}

#define ALIGN(x)    (x & mask)

#if defined(CONFIG_ARCH_MIPS) || (defined(CONFIG_ARM_VER) && (CONFIG_ARM_VER == 6))
/* the stupid aligned version */
void * kmem_t::alloc_aligned(word_t size, word_t alignment, word_t mask)
{
    word_t*     prev;
    word_t*     curr;
    word_t*     tmp;
    word_t      i;

    word_t      align = ALIGN(alignment);

    spinlock.lock();

    ALLOC_TRACE("%s(%d) kfl: %p\n", __FUNCTION__, size, kmem_free_list);
    TRACEPOINT_TB (KMEM_ALLOC,
                   printf ("kmem_alloc_aligned (%d [%d%c]), ip: %p\n",
                       size, size >= GB (1) ? size >> 30 :
                       size >= MB (1) ? size >> 20 : size >> 10,
                       size >= GB (1) ? 'G' : size >= MB (1) ? 'M' : 'K',
                       __return_address()),
                   "kmem_alloc_aligned (%d bytes), ip=%x\n",
                   size, (word_t)__return_address());
    KMEM_CHECK;

    size = max(size, (word_t)KMEM_CHUNKSIZE);
    ASSERT(NORMAL, (size % KMEM_CHUNKSIZE) == 0);

    for (prev = (word_t*) &kmem_free_list, curr = kmem_free_list;
         curr;
         prev = curr, curr = (word_t*) *curr)
    {
        ALLOC_TRACE("curr=%x\n", curr);
        /* properly aligned ??? */
        if ((ALIGN((word_t)curr) == align) && (!((word_t) curr & ((size)- 1))))
        {
            ALLOC_TRACE("%s(%d):%d: curr=%x\n", __FUNCTION__,
                        size, __LINE__, curr);

            tmp = (word_t*) *curr;
            for (i = 1; tmp && (i < (size / KMEM_CHUNKSIZE)); i++)
            {
                ALLOC_TRACE("%s():%d: i=%d, tmp=%x\n",
                            __FUNCTION__, __LINE__, i, tmp);

                if ((word_t) tmp != ((word_t) curr + KMEM_CHUNKSIZE*i))
                {
                    ALLOC_TRACE("skip: %x\n", curr);
                    tmp = 0;
                    break;
                };
                tmp = (word_t*) *tmp;
            }
            if (tmp)
            {
                /* dequeue */
                *prev = (word_t) tmp;

                /* zero the page */
                word_t *p = curr;
                do {
                    *p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
                } while ((word_t)p < (word_t)curr + size);

                /* update counter */
                free_chunks -= (size/KMEM_CHUNKSIZE);

                /* successful return */
                ALLOC_TRACE("kmalloc: %x->%p (%d), kfl: %p, caller: %p\n",
                            size, curr, free_chunks, kmem_free_list, __return_address());
                KMEM_CHECK;

#if 0
                TRACEPOINT_TB (KMEM_ALLOC,, "kmem_alloc (%d bytes), ip=%x "
                                            "==> %p\n",
                                            size, __return_address(),
                                            curr);
#endif

                spinlock.unlock();

                return curr;
            }
        }
    }
#if 0
    word_t * tmp1 = kmem_free_list;
    while(tmp1) {
        printf("%p -> ", tmp1);
        tmp1 = (word_t*)*tmp1;
    }
#endif

    spinlock.unlock();
    return NULL;
}
#endif

/**
 * Hook for platform code to allocate and free memory.
 */
DECLARE_KMEM_GROUP(kmem_ll);

namespace PlatformSupport {

void * mem_alloc(word_t size, bool zero)
{
    return kmem.alloc(kmem_ll, size, zero);
}

void mem_free(void * address, word_t size)
{
    kmem.free(kmem_ll, address, size);
}

}
