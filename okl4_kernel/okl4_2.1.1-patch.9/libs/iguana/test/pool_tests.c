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

#include <stdio.h>
#include <iguana/memsection.h>
#include <iguana/physmem.h>
#include <iguana/physpool.h>
#include "test_libs_iguana.h"

START_TEST(POOL0100)
{
    physmem_ref_t pm;

    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_delete(pm);
}
END_TEST

START_TEST(POOL0200)
{
    memsection_ref_t ms;
    physmem_ref_t pm1, pm2;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(2 * PAGE_SIZE, &base);
    pm1 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm1 == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm1, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    pm2 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm2 == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm2, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm1);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
    }
    res = memsection_map(ms, PAGE_SIZE, pm2);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base + PAGE_SIZE)) = 1;
    }

    memsection_delete(ms);
    physmem_delete(pm1);
    physmem_delete(pm2);
}
END_TEST

START_TEST(POOL0300)
{
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;

    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    base = paddr;
    paddr = 0;
    psize = 0;
    physmem_delete(pm);

    pm = physpool_alloc_fixed(default_physpool, PAGE_SIZE, base);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(paddr == base, "Invalid Physmem base address");
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    physmem_delete(pm);
}
END_TEST

START_TEST(POOL0400)
{
    physmem_ref_t pm;

    pm = physpool_alloc_fixed(default_physpool, PAGE_SIZE, ~0UL);
    fail_unless(pm == INVALID_ADDR, "physpool_alloc_fixed() failed to return error");

    physmem_delete(pm);
}
END_TEST

START_TEST(POOL0500)
{
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;

    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    base = paddr;

    pm = physpool_alloc_fixed(default_physpool, PAGE_SIZE, base);
    fail_unless(pm == INVALID_ADDR, "physpool_alloc_fixed() failed to return error");

    physmem_delete(pm);
}
END_TEST

START_TEST(POOL0600)
{
    memsection_ref_t ms;
    physmem_ref_t pm1, pm2;
    uintptr_t base;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm1 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm1 == INVALID_ADDR, "Failed to create the new Physmem");
    res = memsection_map(ms, 0x0, pm1);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
    }
    physmem_delete(pm1);

    pm2 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm2 == INVALID_ADDR, "Failed to create the new Physmem");
    res = memsection_map(ms, 0x0, pm2);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 2;
    }

    memsection_delete(ms);
    physmem_delete(pm2);
}
END_TEST

TCase *
pool_tests()
{
    TCase *tc;

    tc = tcase_create("Pools");
    tcase_add_test(tc, POOL0100);
    tcase_add_test(tc, POOL0200);
    tcase_add_test(tc, POOL0300);
    tcase_add_test(tc, POOL0400);
    tcase_add_test(tc, POOL0500);
    tcase_add_test(tc, POOL0600);

    return tc;
}
