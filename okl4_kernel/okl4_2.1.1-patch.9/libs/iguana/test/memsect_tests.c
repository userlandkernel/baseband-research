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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/schedule.h>
#include <iguana/debug.h>
#include <iguana/memsection.h>
#include <iguana/physmem.h>
#include <iguana/physpool.h>
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/session.h>
#include <iguana/cap.h>
#include <iguana/eas.h>
#include <iguana/object.h>
#include "test_libs_iguana.h"

#include <interfaces/iguana_client.h>

START_TEST(MEMS0100)
{
    uintptr_t base;
    memsection_ref_t ref;

    ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");
}
END_TEST

START_TEST(MEMS0101)
{
    uintptr_t base;
    memsection_ref_t ref;

    ref = memsection_create_user(256 * 1024 * 1024, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");

    memsection_delete(ref);
}
END_TEST

START_TEST(MEMS0200)
{
    uintptr_t base;
    memsection_ref_t ref;

    ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");

    memsection_delete(ref);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS0201)
{
    uintptr_t base;
    memsection_ref_t ref;

    ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");

    ref |= EXECUTE_IID;

    memsection_delete(ref);
}
END_TEST

#if 0
/* SKIPPED BECAUSE IGUANA SERVER CANNOT COPE WITH BAD POINTERS YET. */
START_TEST(MEMS0300)
{
    uintptr_t base;
    memsection_ref_t ref;

    ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");

    memsection_delete(ref);
    memsection_delete(ref);
}
END_TEST

/* SKIPPED BECAUSE IGUANA SERVER CANNOT COPE WITH BAD POINTERS YET. */
/* Test deleting a completely invalid reference. */
START_TEST(MEMS0400)
{
    memsection_delete(1927);
    printf("delete succeeded.\n");
}
END_TEST
#endif

START_TEST(MEMS0500)
{
    uintptr_t base;
    memsection_ref_t ref;
    char *foo;

    ref = memsection_create(0x1000, &base);

    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");

    if (base == 0 || ref == 0) {
        return;
    }

    foo = (char *)base;
    *foo = 'a';

    memsection_delete(ref);
    ref = memsection_create(0x1000, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");
    foo = (char *)base;

    if (base == 0 || ref == 0) {
        return;
    }

    fail_if(*foo == 'a', "Memory not zeroed");
}
END_TEST

START_TEST(MEMS0600)
{
    uintptr_t base, base2;
    memsection_ref_t ref, ref2;
    memsection_ref_t *refs = NULL;
    int i = 0, cursize = 100;
    int max_memsections;
    int memsection_size = 250 * 0x1000;
    int tmp;

    refs = malloc(sizeof(memsection_ref_t) * cursize);

    do {
        if (i >= cursize) {
            cursize += 100;
            refs = realloc(refs, sizeof(memsection_ref_t) * cursize);
            if (refs == NULL) {
                fail("Unable to allocate memory");
                return;
            }
        }
        refs[i] = memsection_create_direct(memsection_size, &base);
        if (base != 0) {
            tmp = *(volatile int *)base;      /* Touch the memsection */
        }
    } while (refs[i++] != 0);

    max_memsections = i - 1;

    for (i = 0; i < max_memsections; i++) {
        memsection_delete(refs[i]);
    }

    free(refs);

    fail_unless(max_memsections > 0, "Unable to create and direct section");

    /* If we create/del we should easily be able to allocate 2 * the max */
    printf("[[[%d]]]\n", max_memsections);
    for (i = 0; i < max_memsections * 3; i++) {
        printf(".");
        ref = memsection_create(memsection_size, &base);
        fail_if(base == 0, "base pointer not-null");
        if (base == 0) {
            break;
        }
        tmp = *(volatile int *)base;  /* Touch the memsection */
        fail_if(ref == 0, "reference not zero");
        ref2 = memsection_create_direct(memsection_size, &base2);
        fail_if(base2 == 0, "base pointer not-null");
        if (base2 == 0) {
            break;
        }
        tmp = *(volatile int *)base2; /* Touch the memsection */
        fail_if(ref == 0, "reference not zero");
        memsection_delete(ref);
        memsection_delete(ref2);
    }
    printf("\n");
}
END_TEST

/* test fixed location map by remapping a deleted map */
START_TEST(MEMS0700)
{
    uintptr_t base;
    memsection_ref_t ordinary, fixed;

    ordinary = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ordinary == 0, "reference not zero");
    memsection_delete(ordinary);
    fixed = memsection_create_fixed(MEM_SIZE, base);
    fail_if(fixed == 0, "reference not zero");
    memsection_delete(fixed);
}

END_TEST

/* test create_fixed_user meets same requirement as create_fixed */
START_TEST(MEMS0800)
{
    uintptr_t base;
    memsection_ref_t ordinary, fixed;

    ordinary = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ordinary == 0, "reference not zero");
    memsection_delete(ordinary);
    fixed = memsection_create_fixed_user(MEM_SIZE, base);
    fail_if(fixed == 0, "reference not zero");
    memsection_delete(fixed);
}
END_TEST

START_TEST(MEMS0900)
{
    uintptr_t base;
    memsection_ref_t ordinary, fixed;
    int *p;
    int i;

    ordinary = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ordinary == 0, "reference not zero");
    memsection_delete(ordinary);
    fixed = memsection_create_fixed(MEM_SIZE, base);
    fail_if(fixed == 0, "reference not zero");
    if (base == 0 || fixed == 0 || ordinary == 0) {
        return;
    }

    p = (int *)base;
    for (i = 0; i < 10; i++) {
        p[i] = i;
    }
    for (i = 0; i < 10; i++) {
        fail_if(p[i] != i, "memory section corrupt writes");
    }

    memsection_delete(fixed);
}
END_TEST

START_TEST(MEMS1000)
{
    uintptr_t base;
    memsection_ref_t ref;
    uintptr_t size;

    /* 256k should be a multiple of a page size */
    size = 0x40000;
    ref = memsection_create(size, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(ref == 0, "reference not zero");
    fail_if(memsection_size(ref) != size, "memsection size mismatch");
    memsection_delete(ref);
    /* in any case, the memsection size should always be big enough */
    for (size = MEM_SIZE; size < 0x40000; size *= 2) {
        ref = memsection_create(size, &base);
        if ((base == 0) || (ref == 0)) {
            fail("create memsection failed");
            break;
        }
        fail_if(memsection_size(ref) < size, "memsection too small");

        memsection_delete(ref);
    }
}
END_TEST

START_TEST(MEMS1100)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS1101)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");

    mem_ref |= EXECUTE_IID;
    iguana_thrd |= EXECUTE_IID;

    memsection_register_server(mem_ref, iguana_thrd);
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
}
END_TEST

START_TEST(MEMS1200)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0, mem_lookup;
    thread_ref_t iguana_thrd = 0, sever_thrd = 0;
    L4_ThreadId_t l4_thrd;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);
    mem_lookup = memsection_lookup(base + PAGE_SIZE / 2, &sever_thrd);
    fail_if(mem_lookup != mem_ref, "not the right mem section");
    fail_if(sever_thrd != iguana_thrd, "not the right thread");
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS1201)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0, mem_ref_orig, mem_lookup;
    thread_ref_t iguana_thrd = 0, iguana_thrd_orig, sever_thrd = 0;
    L4_ThreadId_t l4_thrd;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    memsection_register_server(mem_ref, iguana_thrd);

    mem_ref_orig = mem_ref;
    mem_ref |= EXECUTE_IID;
    iguana_thrd_orig = iguana_thrd;
    iguana_thrd |= EXECUTE_IID;

    mem_lookup = memsection_lookup(base + PAGE_SIZE / 2, &sever_thrd);
    fail_if(mem_lookup != mem_ref_orig, "not the right mem section");
    fail_if(sever_thrd != iguana_thrd_orig, "not the right thread");
    memsection_delete(mem_ref);
    thread_delete(l4_thrd);
}
END_TEST

START_TEST(MEMS1300)
{
    uintptr_t base = 0, mem_base;
    memsection_ref_t mem_ref = 0;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    mem_base = (uintptr_t)memsection_base(mem_ref);
    fail_if(mem_base != base, "not the right mem section");
    memsection_delete(mem_ref);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS1301)
{
    uintptr_t base = 0, mem_base;
    memsection_ref_t mem_ref = 0;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");

    mem_base |= EXECUTE_IID;

    mem_base = (uintptr_t)memsection_base(mem_ref);
    fail_if(mem_base != base, "not the right mem section");
    memsection_delete(mem_ref);
}
END_TEST

START_TEST(MEMS1400)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");
    memsection_set_attributes(mem_ref, 1);
    memsection_delete(mem_ref);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS1401)
{
    uintptr_t base = 0;
    memsection_ref_t mem_ref = 0;

    mem_ref = memsection_create(MEM_SIZE, &base);
    fail_if(base == 0, "base pointer not-null");
    fail_if(mem_ref == 0, "reference not zero");

    mem_ref |= EXECUTE_IID;

    memsection_set_attributes(mem_ref, 1);
    memsection_delete(mem_ref);
}
END_TEST

START_TEST(MEMS1500)
{
    memsection_ref_t ms1, ms2;
    uintptr_t base1, base2;
    char *p, *q;
    L4_Fpage_t fpage1, fpage2;

    ms1 = memsection_create(MEM_SIZE, &base1);
    p = (void *)base1;
    fpage1 = L4_Fpage(base1, PAGE_SIZE);

    ms2 = memsection_create_user(MEM_SIZE, &base2);
    q = (void *)base2;
    fpage2 = L4_Fpage(base2, PAGE_SIZE);

    memsection_page_map(ms2, fpage1, fpage2);

#define TS1500 "abcdefg123456"
    (void)memcpy(p, TS1500, sizeof(TS1500));
    fail_if(strcmp(p, q) != 0, "page not mapped correctly");

    memsection_delete(ms1);
    memsection_delete(ms2);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS1501)
{
    memsection_ref_t ms1, ms2;
    uintptr_t base1, base2;
    char *p, *q;
    L4_Fpage_t fpage1, fpage2;

    ms1 = memsection_create(MEM_SIZE, &base1);
    p = (void *)base1;
    fpage1 = L4_Fpage(base1, PAGE_SIZE);

    ms2 = memsection_create_user(MEM_SIZE, &base2);
    q = (void *)base2;
    fpage2 = L4_Fpage(base2, PAGE_SIZE);

    ms2 |= EXECUTE_IID;

    memsection_page_map(ms2, fpage1, fpage2);

#define TS1501 "abcdefg123456"
    (void)memcpy(p, TS1501, sizeof(TS1501));
    fail_if(strcmp(p, q) != 0, "page not mapped correctly");

    memsection_delete(ms1);
    memsection_delete(ms2);
}
END_TEST

START_TEST(MEMS1600)
{
    memsection_ref_t ms1, ms2, msu;
    uintptr_t base1, base2, baseu;
    uintptr_t alias1, alias2;
    L4_Fpage_t dst, src;

    ms1 = memsection_create(2 * PAGE_SIZE, &base1);
    memset((void *)base1, 'a', 2 * PAGE_SIZE);
    ms2 = memsection_create(PAGE_SIZE, &base2);
    memset((void *)base2, 'b', PAGE_SIZE);
    msu = memsection_create_user(5 * PAGE_SIZE, &baseu);

    alias1 = baseu + PAGE_SIZE;
    src = L4_Fpage(base1, 2 * PAGE_SIZE);
    dst = L4_Fpage(alias1, 2 * PAGE_SIZE);
    memsection_page_map(msu, src, dst);

    alias2 = baseu + 2 * PAGE_SIZE;
    src = L4_Fpage(base2, PAGE_SIZE);
    dst = L4_Fpage(alias2, PAGE_SIZE);
    memsection_page_map(msu, src, dst);

    fail_if(strchr((const char *)alias1, 'b') != (char *)alias2,
            "pages not mapped correctly");

    memsection_delete(ms1);
    memsection_delete(ms2);
    memsection_delete(msu);
}
END_TEST

typedef memsection_ref_t (*create_type) (uintptr_t size, uintptr_t *base);
typedef void (*delete_type) (memsection_ref_t ms);

static inline void
test_memsection_create_various_sizes(create_type cr, delete_type del)
{
    size_t size;
    uintptr_t base;
    volatile char *p;

    for (size = MEM_SIZE_LEAST; size <= MEM_SIZE_MOST; size *= 2) {
        memsection_ref_t ms = cr(size, &base);

        fail_if(ms == 0, "memsection reference not-zero");
        if (ms == 0)
            continue;

        p = (char *)base;
        *p = 'a';
        fail_if(*p != 'a', "memory access inconsistent");

        p += size - 1;
        *p = 'a';
        fail_if(*p != 'a', "memory access inconsistent");

        del(ms);
    }
}

START_TEST(MEMS1700)
{
    test_memsection_create_various_sizes(memsection_create, memsection_delete);
}
END_TEST

static memsection_ref_t
get_fixed(uintptr_t size, uintptr_t *base)
{
    memsection_ref_t ms = memsection_create(size, base);

    if (ms == 0)
        return 0;

    memsection_delete(ms);

    return memsection_create_fixed(size, *base);
}

START_TEST(MEMS1800)
{
    test_memsection_create_various_sizes(get_fixed, memsection_delete);
}
END_TEST

static memsection_ref_t the_base_memsection_for_user;

static memsection_ref_t
get_user(uintptr_t size, uintptr_t *base)
{
    uintptr_t base2;
    L4_Fpage_t fpage1, fpage2;
    memsection_ref_t ms = memsection_create_user(size, base);

    the_base_memsection_for_user = memsection_create(size, &base2);

    if (size < PAGE_SIZE) {
        fpage1 = L4_Fpage(*base, PAGE_SIZE);
        fpage2 = L4_Fpage(base2, PAGE_SIZE);
    } else {
        fpage1 = L4_Fpage(*base, size);
        fpage2 = L4_Fpage(base2, size);
    }

    memsection_page_map(ms, fpage2, fpage1);

    return ms;
}

static void
del_user(memsection_ref_t ms)
{
    memsection_delete(ms);

    if (the_base_memsection_for_user)
        memsection_delete(the_base_memsection_for_user);
    the_base_memsection_for_user = 0;
}

START_TEST(MEMS1900)
{
    test_memsection_create_various_sizes(get_user, del_user);
}
END_TEST

static void write_squares(void *loc);
START_TEST(MEMS2000)
{
    memsection_ref_t ms;
    uintptr_t base;
    int i;
    volatile int *p;
    thread_ref_t thrd;

    ms = memsection_create(MEM_SIZE, &base);
    p = (int *)base;

    thrd = thread_create_simple(write_squares, (void *)p, 100);
    fail_if(thrd == 0, "");

    /* strange ping pong: we write the even segments, it writes odd */
    for (i = 0; i < 20; i += 2) {
        p[i] = i * i;
        while (p[i + 1] != (i + 1) * (i + 1)) {
            L4_Yield();
        }
    }

    printf("memsection_delete(ms);\n");
    memsection_delete(ms);
    printf("thread_delete(thread_l4tid(thrd));\n");
    thread_delete(thread_l4tid(thrd));
}
END_TEST

static void
write_squares(void *loc)
{
    volatile int *p = (int *)loc;
    int i;

    for (i = 0; i < 20; i += 2) {
        while (p[i] != i * i) {
            L4_Yield();
        }
        p[i + 1] = (i + 1) * (i + 1);
    }

    /* spin, parent will clean up */
    for (;;){
        /* on IA32 without the call to Yield the main thread never runs again */
        L4_Yield();
    }
}

static volatile memsection_ref_t shared_ms_ref;
static void delete_shared_memsection(void *arg);

START_TEST(MEMS2100)
{
    uintptr_t base;

    shared_ms_ref = memsection_create(MEM_SIZE, &base);

    fail_if(shared_ms_ref == 0, "allocate memsection");
    if (shared_ms_ref == 0)
        return;

    thread_create_simple(delete_shared_memsection, NULL, 100);

    while (shared_ms_ref != 0)
        L4_Yield();
}
END_TEST

static void
delete_shared_memsection(void *arg)
{
    memsection_delete(shared_ms_ref);

    shared_ms_ref = 0;
}

/* N is the number of threads. P is some prime > 10 * N */
enum { ARRAY_TEST_N = 10, ARRAY_TEST_P = 89 };

/*
 * an array of memory sections 
 */
volatile memsection_ref_t array[ARRAY_TEST_N][ARRAY_TEST_N];

/* the number of memory sections each thread has deleted */
volatile int array_deleted[ARRAY_TEST_N];
thread_ref_t array_tid[ARRAY_TEST_N];

static void array_test_worker(void *arg);

START_TEST(MEMS2200)
{
    uintptr_t i, j;

    for (i = 0; i < ARRAY_TEST_N; i++)
        for (j = 0; j < ARRAY_TEST_N; j++)
            array[i][j] = 0;
    for (i = 0; i < ARRAY_TEST_N; i++)
        array_deleted[i] = 0;

    for (i = 0; i < ARRAY_TEST_N; i++) {
        fail_if((array_tid[i] =
                 thread_create_simple(array_test_worker, (void *)i, 100)) == 0,
                "");
    }

    for (i = 0; i < ARRAY_TEST_N; i++)
        while (array_deleted[i] != ARRAY_TEST_N)
            L4_Yield();
    for (i = 0; i < ARRAY_TEST_N; i++)
        thread_delete(thread_l4tid(array_tid[i]));
}
END_TEST

static void
array_test_worker(void *arg)
{
    uintptr_t id = (uintptr_t)arg;
    int created = 0, deleted = 0;

    /* seeker cycles mod P (cyclic group), seekpoint mod N */
    int seeker = id * 2, seekpoint;
    int delta = 1;
    int *p;

    while (deleted != ARRAY_TEST_N) {
        if (created < ARRAY_TEST_N) {
            uintptr_t base;
            memsection_ref_t ms;
            int to_create = (id * (id + 1) + created)
                % ARRAY_TEST_N;

            ms = memsection_create(MEM_SIZE, &base);
            fail_if(ms == 0, "");

            p = (int *)memsection_base(ms);
            *p = to_create + id;

            fail_if(array[to_create][id] != 0, "");
            array[to_create][id] = ms;
            created++;
            delta = (base & 0xacffff) % ARRAY_TEST_P;
            delta = delta ? delta : 1;
        }

        seeker = (seeker + delta) % ARRAY_TEST_P;
        seekpoint = seeker % ARRAY_TEST_N;

        if (array[id][seekpoint] != 0) {
            memsection_ref_t ms = array[id][seekpoint];

            array[id][seekpoint] = 0;

            p = (int *)memsection_base(ms);

            fail_if(*p != id + seekpoint, "memory consistency");

            memsection_delete(ms);

            deleted++;
            array_deleted[id] = deleted;
        }

        L4_Yield();
    }
    L4_Call(L4_myselfconst);
}

START_TEST(MEMS2300)
{
    uintptr_t base;
    memsection_ref_t *refs = NULL;
    int i = 0, cursize = 100;
    int max_memsections;
    int memsect_size = 200 * 0x1000;
    volatile char *q;

    /* Linked list of memsection */

    printf("TESTING MEMORY SECTIONS\n");

    q = NULL;
    refs = malloc(sizeof(memsection_ref_t) * cursize);
    refs[i] = memsection_create_direct(memsect_size, &base);
    printf("Create a 1:1 mapping at: %" PRIxPTR "\n", base);

    if (base != 0) {
        fail_if(memsection_size(refs[i]) != memsect_size,
                "memsection size mismatch");
        q = (char *)base;
        *q = 'a';
        fail_unless(*q == 'a', "Memory access inconsistent");
        i++;
    }

    printf("TESTING NORMAL CREATES\n");
    do {
        if (i % 10 == 0) {
            printf("\n%d\t", i);
        }
        if (i >= cursize) {
            cursize += 100;
            refs = realloc(refs, sizeof(memsection_ref_t) * cursize);
            if (refs == NULL) {
                fail("Unable to allocate memory");
                return;
            }
        }
        refs[i] = memsection_create(memsect_size, &base);
        if (q != NULL) {
            fail_unless(*q == 'a', "Memory zeroed");
        }
        if (base != 0) {
            fail_if(memsection_size(refs[i]) != memsect_size,
                    "memsection size mismatch");
            q = (char *)base;
            *q = 'a';
            fail_unless(*q == 'a', "Memory access inconsistent");
        }
        printf(".");
    } while (refs[i++] != 0);

    max_memsections = i - 1;
    printf("Done a lot of creates: %d of %d size total: %d MB\n",
           max_memsections, memsect_size,
           (max_memsections * memsect_size) / 1024 / 1024);

    for (i = 0; i < max_memsections; i++) {
        memsection_delete(refs[i]);
    }
    free(refs);
}
END_TEST


/* Start the iguana-physpools tests -------------------------------------*/

START_TEST(MEMS2400)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, 2 * PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == 2 * PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm);
    fail_unless(res != 0, "memsection_map() failed to return error");

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2500)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 2 * PAGE_SIZE, pm);
    fail_unless(res != 0, "memsection_map() failed to return error");

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2501)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(2 * PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x1, pm);
    fail_unless(res != 0, "memsection_map() failed to return error");

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST


/*
 * Repeat of the test 2500 with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(MEMS2502)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");

    pm |= EXECUTE_IID;
    ms |= EXECUTE_IID;

    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 2 * PAGE_SIZE, pm);
    fail_unless(res != 0, "memsection_map() failed to return error");

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2600)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
        res = memsection_unmap(ms, 0x0, psize + 1);
        fail_unless(res != 0, "memsection_unmap() failed to return error");
    }

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2601)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
        res = memsection_unmap(ms, 0x0, psize + PAGE_SIZE);
        fail_unless(res != 0, "memsection_unmap() failed to return error");
    }

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2602)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(2 * PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, 2 * PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == 2 * PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
        res = memsection_unmap(ms, 0x1, PAGE_SIZE);
        fail_unless(res != 0, "memsection_unmap() failed to return error");
    }

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2603)
{
    memsection_ref_t ms;
    physmem_ref_t pm;
    uintptr_t base;
    uintptr_t paddr, psize;
    int res;

    ms = memsection_create_user(2 * PAGE_SIZE, &base);
    pm = physpool_alloc(default_physpool, 2 * PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");
    physmem_info(pm, &paddr, &psize);
    fail_unless(psize == 2 * PAGE_SIZE, "Invalid Physmem size");

    res = memsection_map(ms, 0x0, pm);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
        res = memsection_unmap(ms, PAGE_SIZE, psize / 2);
        fail_unless(res == 0, "Failed to unmap Physmem");
    }

    memsection_delete(ms);
    physmem_delete(pm);
}
END_TEST

START_TEST(MEMS2700)
{
    memsection_ref_t ms;
    physmem_ref_t pm1, pm2;
    uintptr_t base;
    int res;

    ms = memsection_create_user(PAGE_SIZE, &base);
    pm1 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm1 == INVALID_ADDR, "Failed to create the new Physmem");

    pm2 = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm2 == INVALID_ADDR, "Failed to create the new Physmem");

    res = memsection_map(ms, 0x0, pm1);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 1;
    }
    res = memsection_map(ms, 0x0, pm2);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)(base)) = 2;
    }

    memsection_delete(ms);
    physmem_delete(pm1);
    physmem_delete(pm2);
}
END_TEST

START_TEST(MEMS2800)
{
    memsection_ref_t ms;
    physmem_ref_t pm1, pm2;
    uintptr_t base;
    int res;

    ms = memsection_create_user(3 * PAGE_SIZE, &base);
    pm1 = physpool_alloc(default_physpool, 2 * PAGE_SIZE);
    fail_if(pm1 == INVALID_ADDR, "Failed to create the new Physmem");

    pm2 = physpool_alloc(default_physpool, 2 * PAGE_SIZE);
    fail_if(pm2 == INVALID_ADDR, "Failed to create the new Physmem");

    res = memsection_map(ms, 0x0, pm1);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        *((volatile int *)base) = 1;
        res = memsection_map(ms, PAGE_SIZE, pm2);
        fail_unless(res == 0, "Failed to map Physmem");
        if (!res) {
            *((volatile int *)(base + PAGE_SIZE)) = 1;
        }
    }

    memsection_delete(ms);
    physmem_delete(pm1);
    physmem_delete(pm2);
}
END_TEST

START_TEST(MEMS2900)
{
    memsection_ref_t ms1, ms2;
    physmem_ref_t pm;
    uintptr_t base1, base2;
    char *p, *q;
    int res;

    ms1 = memsection_create_user(PAGE_SIZE, &base1);
    p = (void *)base1;
    ms2 = memsection_create_user(PAGE_SIZE, &base2);
    q = (void *)base2;
    pm = physpool_alloc(default_physpool, PAGE_SIZE);
    fail_if(pm == INVALID_ADDR, "Failed to create the new Physmem");

    res = memsection_map(ms1, 0x0, pm);
    fail_unless(res == 0, "Failed to map Physmem");
    if (!res) {
        res = memsection_map(ms2, 0x0, pm);
        fail_unless(res == 0, "Failed to map Physmem");
        if (!res) {
            strcpy(p, "abcdefg123456");
            fail_if(strcmp(p, q) != 0, "page not mapped correctly");
        }
    }

    memsection_delete(ms1);
    memsection_delete(ms2);
    physmem_delete(pm);
}
END_TEST

TCase *
memsect_tests()
{
    TCase *tc;

    tc = tcase_create("Memory Sections");
    tcase_add_test(tc, MEMS0100);
    tcase_add_test(tc, MEMS0101);
    tcase_add_test(tc, MEMS0200);
    tcase_add_test(tc, MEMS0201);
#if 0
    tcase_add_test(tc, MEMS0300);
    tcase_add_test(tc, MEMS0400);
#endif
    tcase_add_test(tc, MEMS0500);
    tcase_add_test(tc, MEMS0600);
    tcase_add_test(tc, MEMS0700);
    tcase_add_test(tc, MEMS0800);
    tcase_add_test(tc, MEMS0900);
    tcase_add_test(tc, MEMS1000);
    tcase_add_test(tc, MEMS1100);
    tcase_add_test(tc, MEMS1101);
    tcase_add_test(tc, MEMS1200);
    tcase_add_test(tc, MEMS1201);
    tcase_add_test(tc, MEMS1300);
    tcase_add_test(tc, MEMS1301);
    tcase_add_test(tc, MEMS1400);
    tcase_add_test(tc, MEMS1401);
    tcase_add_test(tc, MEMS1500);
    tcase_add_test(tc, MEMS1501);
    tcase_add_test(tc, MEMS1600);
    tcase_add_test(tc, MEMS1700);
    tcase_add_test(tc, MEMS1800);
    tcase_add_test(tc, MEMS1900);
    tcase_add_test(tc, MEMS2000);
    tcase_add_test(tc, MEMS2100);
    tcase_add_test(tc, MEMS2200);
    tcase_add_test(tc, MEMS2300);
    tcase_add_test(tc, MEMS2400);
    tcase_add_test(tc, MEMS2500);
    tcase_add_test(tc, MEMS2501);
    tcase_add_test(tc, MEMS2502);
    tcase_add_test(tc, MEMS2600);
    tcase_add_test(tc, MEMS2601);
    tcase_add_test(tc, MEMS2602);
    tcase_add_test(tc, MEMS2603);
    tcase_add_test(tc, MEMS2700);
    tcase_add_test(tc, MEMS2800);
    tcase_add_test(tc, MEMS2900);

    return tc;
}
