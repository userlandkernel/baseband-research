/*
 * Copyright (c) 2004, National ICT Australia
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
#include "../k_r_malloc.h"
#include <stdio.h>
#include <stdint.h>
#include <iguana/cap.h>
#include <iguana/pd.h>
#include <iguana/memsection.h>
#include <iguana/types.h>
#include <l4e/misc.h>

extern void free(void *);

#ifdef MALLOC_LOCKED
#include <mutex/mutex.h>
#include <l4/thread.h>
extern struct mutex malloc_mutex;
#endif /* MALLOC_LOCKED */

void __malloc_init(void *bss_p, void *top_p);
void __morecore_init(void);

Header *_kr_malloc_freep = NULL;

void
__malloc_init(void *bss_p, void *top_p)
{
    Header *header = bss_p;

#ifdef MALLOC_LOCKED
    malloc_mutex.holder = 0;
    malloc_mutex.needed = 0;
    malloc_mutex.count = 0;
#endif /* MALLOC_LOCKED */

    header->s.size = ((char *)top_p - (char *)bss_p + 1) / sizeof(Header);
    free((void *)(header + 1));
}

static void *
iguana_memsection_create(size_t size, uintptr_t *ret_base, uintptr_t *ret_size)
{
    cap_t cap = INVALID_CAP;

    cap = _cap_memsection_create(size, ret_base);
    if (!IS_VALID_CAP(cap)) {
        return NULL;
    }

    /* Now attach so that we can write to it. */
    pd_attach(pd_myself(), cap.ref.memsection, L4_FullyAccessible);
    
    *ret_size = memsection_size(cap.ref.obj);
    return (void *)memsection_base(cap.ref.obj);
}

void
__morecore_init(void)
{
}

#define round_up(address, size) ((((address) + ((size)-1)) & (~((size)-1))))

/*
 * sbrk equiv
 */
Header *
morecore(unsigned num_units)
{
    Header *header;
    uintptr_t ret_base, ret_size;
    uintptr_t num_bytes;

    /* we can only request more memory in 16 * pagesized chunks */
    num_units = round_up(num_units, 16 * l4e_min_pagesize() / sizeof(Header));
    num_bytes = num_units * sizeof(Header);

    if (iguana_memsection_create(num_bytes, &ret_base, &ret_size) == NULL) {
        return NULL;
    }
    header = (Header *)ret_base;
    header->s.size = ret_size / sizeof(Header);

    free((void *)(header + 1));
    return _kr_malloc_freep;
}
