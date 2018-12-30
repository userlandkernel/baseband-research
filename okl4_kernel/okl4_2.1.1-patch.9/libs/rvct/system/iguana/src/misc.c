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

#include <rt_misc.h>
#include <iguana/cap.h>
#include <iguana/memsection.h>
#include <iguana/env.h>
#include <iguana/pd.h>
#include <iguana/physpool.h>
#include <iguana/thread.h>
#include <iguana/tls.h>
#include <iguana/virtpool.h>
#include <l4/thread.h>

#pragma import(__use_two_region_memory)

/* Internal function in iguana's env.c. */
extern const envitem_t *__iguana_getenv(const char *name);

extern int __argc;
extern char **__argv;
int __used;

void *
__user_perthread_libspace(void)
{
        return (char *)__L4_TCR_ThreadLocalStorage() + 32;
}

__value_in_regs struct __initial_stackheap
__user_initial_stackheap(unsigned R0, unsigned SP, unsigned R2, unsigned SL)
{
        struct __initial_stackheap ret;
        void *__env;

        /* See iguana/init for the initial stack layout. */
        __env = *((void **)SP)++;
        __argc = *((int *)SP)++;
        __argv = *((char ***)SP)++;

        /* Don't call __lib_iguana_init because there's not enough stack. */
        __lib_iguana_init_env(__env);

        /* Find the default memory pools. */
        default_virtpool = env_virtpool(__iguana_getenv("OKL4_DEFAULT_VIRTPOOL"));
        default_physpool = env_physpool(__iguana_getenv("OKL4_DEFAULT_PHYSPOOL"));

        /* For RVCT, we put the TLS above the stack. */
        __L4_TCR_Set_ThreadLocalStorage(SP -= 128);

        ret.heap_base = env_const(__iguana_getenv("HEAP_BASE"));
        ret.stack_base = SP;
        ret.heap_limit = ret.heap_base + env_const(__iguana_getenv("HEAP_SIZE")) - 1;
        ret.stack_limit = SL;
        return ret;
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000)
unsigned
__user_heap_extend(int ignore, unsigned requestedsize, void **base)
{
        cap_t cap;

        /* 
         * XXX: ADS hack.  The docs and headers seem to be wrong,
         * objdump indicates that base is passed in R1 and requestedsize
         * in R2, so we flip them here.
         */
        {
                unsigned tmp;

                tmp = requestedsize;
                requestedsize = (unsigned)base;
                base = (void **)tmp;
        }
        /* 
         * ADS (but not RVCT) seems to access memory just after the end of the
         * heap, so we extend the size by 8 bytes.
         */
        requestedsize += 8;
        if (requestedsize < 0x10000)
                requestedsize = 0x10000;
        if (__used == 16)
                return 0;
        cap = _cap_memsection_create(requestedsize, (uintptr_t *)base);
        if (cap.ref.memsection == 0)
                return 0;

        /* Now attach so that we can write to it. */
        pd_attach(pd_myself(), cap.ref.memsection, L4_FullyAccessible);

        return memsection_size(cap.ref.memsection) - 8;
}
#else
unsigned
__user_heap_extend(int ignore, void **base, unsigned requestedsize)
{
        cap_t cap;

        if (requestedsize < 0x10000)
                requestedsize = 0x10000;
        if (__used == 16)
                return 0;
        cap = _cap_memsection_create(requestedsize, (uintptr_t *)base);
        if (cap.ref.memsection == 0)
                return 0;

        /* Now attach so that we can write to it. */
        pd_attach(pd_myself(), cap.ref.memsection, L4_FullyAccessible);

        return memsection_size(cap.ref.memsection);
}
#endif
