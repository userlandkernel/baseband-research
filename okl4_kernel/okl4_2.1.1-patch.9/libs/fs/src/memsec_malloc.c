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

#include <fs/fs.h>

#include <iguana/memsection.h>
#include <mutex/mutex.h>

#define FREE 1
#define ALLOCATED 2

static int usize = 0;

void *
memsec_malloc(memsection_ref_t ms, uintptr_t count)
{
    uintptr_t base, end, chunk_size;//, tmp;
    uintptr_t *u;
    void *ret;

    if (!ms) {
        return NULL;
    }

    intervm_ctl_mutex = (okl4_mutex_t) memsection_base(ms);
    okl4_mutex_count_lock(intervm_ctl_mutex);
    base = (uintptr_t)memsection_base(ms) + sizeof(struct okl4_mutex);
    end = memsection_size(ms) + base;

    u = (uintptr_t *)base;
    if (*u == 0 && *(u + 1) == 0) {
        usize = sizeof(uintptr_t);
        if (count <= (end - (base + 2 * usize))) {
            *u++ = ALLOCATED;
            count = *u++ = ((count / usize) + 1) * usize;
            u += (count / usize);
            *u++ = FREE;
            *u = end - (uintptr_t)(u + 1);
            okl4_mutex_count_unlock(intervm_ctl_mutex);
            return (void *)(base + 2 * usize);
        }
    }
    usize = sizeof(uintptr_t);
    while ((uintptr_t)u < end) {
        if (*u == FREE) {
            chunk_size = *++u;

            if (count <= chunk_size) {
                *(u - 1) = ALLOCATED;
                count = *u++ = ((count / usize) + 1) * usize;
                if (((uintptr_t)u + (count / usize)) >= end) {
                    *(u - 1) = 0;
                }
                ret = (void *)u;
                u += (count / usize);
                if ((uintptr_t)u >= end) {
                    okl4_mutex_count_unlock(intervm_ctl_mutex);
                    return ret;
                }
                if (*u == 0) {
                    *u++ = FREE;
                    *u = chunk_size - count;
                }
                okl4_mutex_count_unlock(intervm_ctl_mutex);
                return ret;
            } else {
                u += (chunk_size / usize);
            }
        } else {
            chunk_size = *++u;
            if (chunk_size == 0) {
                okl4_mutex_count_unlock(intervm_ctl_mutex);
                return NULL;
            }
            u += 1 + (chunk_size / usize);
        }
    }
    okl4_mutex_count_unlock(intervm_ctl_mutex);
    return NULL;
}

void
memsec_free(memsection_ref_t ms, void *ptr)
{
    uintptr_t base, end, chunk_size;
    uintptr_t *u, *next, *chunk_base;

    if (!ms) {
        return;
    }

    intervm_ctl_mutex = (okl4_mutex_t) memsection_base(ms);
    okl4_mutex_count_lock(intervm_ctl_mutex);
    base = (uintptr_t)memsection_base(ms) + sizeof(struct okl4_mutex);
    end = base + memsection_size(ms);

    u = (uintptr_t *)ptr;
    if ((uintptr_t)u < base || (uintptr_t)u >= end) {
        okl4_mutex_count_unlock(intervm_ctl_mutex);
        return;
    }

    chunk_size = *--u;
    *(u - 1) = FREE;
    next = (uintptr_t *)base;
    while ((uintptr_t)next < end) {
        if (*next == ALLOCATED) {
            next += 2 + (*next / usize);
            continue;
        } else {
            u = next + 1;
            chunk_base = next + 2;
            while ((uintptr_t)next < end && *next++ == FREE) {  
                chunk_size = *next;
                if ((uintptr_t *)ptr != chunk_base && chunk_base != (next + 1)) {
                    *(u - 1) = FREE;
                    if (*(u + 1 + (chunk_size / usize)) == FREE) {
                        chunk_size += 8;
                        *u = *u + chunk_size + 4;
                    } else {
                        chunk_size += 4;
                        *u = *u + chunk_size;
                    }
                    next += 0 + (chunk_size / usize);
                } else {
                    *(u - 1) = FREE;
                    next += 1 + (chunk_size / usize);
                }
                if ((uintptr_t)next > end) {
                    okl4_mutex_count_unlock(intervm_ctl_mutex);
                    return;
                }
            }
            okl4_mutex_count_unlock(intervm_ctl_mutex);
            return;
        }
    }
    okl4_mutex_count_unlock(intervm_ctl_mutex);
}


