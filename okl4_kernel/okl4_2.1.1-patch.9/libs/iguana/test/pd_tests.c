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

#include "test_libs_iguana.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <iguana/pd.h>
#include <iguana/cap.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/schedule.h>

START_TEST(PD0100)
{
    pd_ref_t myself = 0;

    myself = pd_myself();
    fail_if(myself == 0, "NULL pd returned");
}
END_TEST

START_TEST(PD0200)
{
    pd_ref_t newpd = 0;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    if (newpd != 0) {
        pd_delete(newpd);
    }
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD0201)
{
    pd_ref_t newpd = 0;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    if (newpd != 0) {
        newpd |= EXECUTE_IID;

        pd_delete(newpd);
    }
}
END_TEST

#if 0
/* SKIPPED BECAUSE IGUANA SERVER CANNOT COPE WITH BAD POINTERS YET. */
/* Test deleting a completely invalid reference. */
START_TEST(PD0202)
{
    pd_delete(0xdeadbeef);
}
END_TEST
#endif

#ifdef CONFIG_LOWMEMORY
START_TEST(PD0300)
{
    pd_ref_t *pds = NULL;
    int max_pds;
    int i = 0, cursize = 100;

    pds = malloc(sizeof(struct session *) * cursize);

    do {
        if (i >= cursize) {
            cursize += 100;
            pds = realloc(pds, sizeof(pd_ref_t) * cursize);
            if (pds == NULL) {
                fail("Unable to allocate memory");
                break;
            }
        }
        pds[i] = pd_create();
    } while (pds[i++] != 0);

    max_pds = i - 1;
    printf("Created %d PDs\n", max_pds);

    for (i = 0; i < max_pds; i++) {
        pd_delete(pds[i]);
    }

    free(pds);

    fail_unless(max_pds > 0, "Unable to create any protection domains");

    for (i = 0; i < max_pds * 3; i++) {
        pd_ref_t pd;

        pd = pd_create();
        fail_if(pd == 0, "Unable to create PD");
        if (pd == 0) {
            printf("Couldn't create session\n");
            break;
        }
        pd_delete(pd);
    }
}
END_TEST
#endif

START_TEST(PD0400)
{
    pd_ref_t newpd = 0;

    newpd = pd_create_pd(pd_myself());
    fail_if(newpd == 0, "NULL pd returned");
    if (newpd != 0) {
        pd_delete(newpd);
    }
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD0401)
{
    pd_ref_t newpd = 0;
    pd_ref_t myself = pd_myself();

    myself |= EXECUTE_IID;

    newpd = pd_create_pd(myself);
    fail_if(newpd == 0, "NULL pd returned");
    if (newpd != 0) {
        pd_delete(newpd);
    }
}
END_TEST

START_TEST(PD0500)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD0501)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = pd_create_thread(pd_myself(), &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    if (iguana_thrd != 0) {
        iguana_thrd |= EXECUTE_IID;
        
        thread_delete(l4_thrd);
    }
}
END_TEST

START_TEST(PD0600)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;

    iguana_thrd = pd_create_thread_with_priority(pd_myself(), 100, &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    if (iguana_thrd != 0) {
        thread_delete(l4_thrd);
    }
}
END_TEST

#if 0
START_TEST(test_pd_create_thread)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    pd_ref_t newpd = 0;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    iguana_thrd = pd_create_thread(newpd, &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    thread_delete(l4_thrd);
    pd_delete(newpd);
}
END_TEST

START_TEST(test_pd_create_thread_with_priority)
{
    thread_ref_t iguana_thrd = 0;
    L4_ThreadId_t l4_thrd;
    pd_ref_t newpd = 0;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    iguana_thrd = pd_create_thread_with_priority(newpd, 100, &l4_thrd);
    fail_if(iguana_thrd == 0, "NULL thread returned");
    thread_delete(l4_thrd);
    pd_delete(newpd);
}
END_TEST
#endif

START_TEST(PD0700)
{
    memsection_ref_t memsec = 0;
    uintptr_t base;
    int i = 0;
    char *p;

    memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
    fail_if(memsec == 0, "NULL memsection returned");

    if (memsec == 0) {
        return;
    }
    p = (char *)base;

    memset(p, '%', 0x10);

    for (i = 0; i < 0x10; i++) {
        fail_if(*p != '%', "error data");
        p++;
    }

    memsection_delete(memsec);
}
END_TEST

START_TEST(PD0800)
{
    memsection_ref_t memsec = 0;
    uintptr_t base;

    memsec = pd_create_memsection_user(pd_myself(), MEM_SIZE, &base);
    fail_if(memsec == 0, "NULL memsection returned");

    memsection_delete(memsec);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD0801)
{
    memsection_ref_t memsec = 0;
    pd_ref_t myself = pd_myself();
    uintptr_t base;

    myself |= EXECUTE_IID;

    memsec = pd_create_memsection_user(myself, MEM_SIZE, &base);
    fail_if(memsec == 0, "NULL memsection returned");

    memsection_delete(memsec);
}
END_TEST

START_TEST(PD0900)
{
    memsection_ref_t memsec = 0;
    uintptr_t base;

    memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
    fail_if(memsec == 0, "NULL memsection returned");
    if (memsec == 0) {
        return;
    }

    memsection_delete(memsec);

    memsec = pd_create_memsection_fixed_user(pd_myself(), MEM_SIZE, base);
    fail_if(memsec == 0, "NULL memsection returned");

    memsection_delete(memsec);
}
END_TEST

START_TEST(PD1000)
{
    pd_ref_t newpd = 0;
    memsection_ref_t memsec = 0;
    uintptr_t base;
    int i;
    char *p;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    memsec = pd_create_memsection(newpd, MEM_SIZE, &base);
    fail_if(memsec == 0, "NULL memsection returned");
    i = 0;
    p = (char *)base;

    memset((void *)base, '%', 0x10);
    for (i = 0; i < 0x10; i++) {
        fail_if(*p != '%', "error data");
        p++;
    }

    pd_delete(newpd);
}
END_TEST

START_TEST(PD1100)
{
    uintptr_t callback, callback_base;

    callback = pd_create_memsection(pd_myself(), MEM_SIZE, &callback_base);
    fail_if(callback == 0, "NULL memsection returned");
    pd_set_callback(pd_myself(), callback);
    memsection_delete(callback);
    pd_set_callback(pd_myself(), 0);
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD1101)
{
    uintptr_t callback, callback_base;
    pd_ref_t myself = pd_myself();

    myself |= EXECUTE_IID;

    callback = pd_create_memsection(myself, MEM_SIZE, &callback_base);
    fail_if(callback == 0, "NULL memsection returned");

    callback |= EXECUTE_IID;
    
    pd_set_callback(myself, callback);
    memsection_delete(callback);
    pd_set_callback(myself, 0);
}
END_TEST

START_TEST(PD1200)
{
    pd_ref_t newpd = 0;
    uintptr_t callback, callback_base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    callback = pd_create_memsection(newpd, MEM_SIZE, &callback_base);
    fail_if(callback == 0, "NULL memsection returned");
    pd_set_callback(newpd, callback);
    pd_delete(newpd);
}
END_TEST

START_TEST(PD1300)
{
    uintptr_t clist, clist_base;

    clist = pd_create_memsection(pd_myself(), MEM_SIZE, &clist_base);
    fail_if(clist == 0, "NULL clist returned");

    pd_add_clist(pd_myself(), clist);
    pd_release_clist(pd_myself(), clist, 0);

    memsection_delete(clist);
}
END_TEST

START_TEST(PD1400)
{
    pd_ref_t newpd = 0;
    uintptr_t clist, clist_base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");
    clist = pd_create_memsection(newpd, MEM_SIZE, &clist_base);
    fail_if(clist == 0, "NULL memsection returned");
    pd_add_clist(newpd, clist);
    pd_release_clist(newpd, clist, 0);
    pd_delete(newpd);

}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD1401)
{
    pd_ref_t newpd = 0;
    uintptr_t clist, clist_base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    newpd |= EXECUTE_IID;

    clist = pd_create_memsection(newpd, MEM_SIZE, &clist_base);
    fail_if(clist == 0, "NULL memsection returned");

    clist |= EXECUTE_IID;

    pd_add_clist(newpd, clist);
    pd_release_clist(newpd, clist, 0);
    pd_delete(newpd);

}
END_TEST

static volatile int worker_a_done = 0;
static volatile int worker_b_done = 0;

static void
worker_a(void *ignore)
{
    ARCH_THREAD_INIT
    memsection_ref_t m;
    uintptr_t base;
    int i;

    L4_KDB_SetThreadName(thread_l4tid(thread_myself()), "worker_a");
    L4_Yield();

    for (i = 0; i < 10000; i++) {
        m = pd_create_memsection(pd_myself(), 1UL << 31, &base);
        assert(m == 0);
    }

    worker_a_done = 1;
    L4_WaitForever();
}

static void
worker_b(void *ignore)
{
    ARCH_THREAD_INIT
    memsection_ref_t m;
    uintptr_t base;
    int i;
    char *p;

    L4_KDB_SetThreadName(thread_l4tid(thread_myself()), "worker_b");
    L4_Yield();

    for (i = 0; i < 1000; i++) {
        m = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        assert(m != 0);
        p = (char *)base;
        memset(p, '%', 0x10);
        memsection_delete(m);
    }

    worker_b_done = 1;
    L4_WaitForever();
}

START_TEST(PD1500)
{
    thread_ref_t a, b;

    a = thread_create_simple(worker_a, NULL, 100);
    b = thread_create_simple(worker_b, NULL, 100);

    while (!worker_a_done || !worker_b_done) {
        L4_Yield();
    }

    thread_delete(thread_l4tid(a));
    thread_delete(thread_l4tid(b));
}
END_TEST

/*
 * Simple use case:  Create a memsection and attached it to another
 * pd.
 */
START_TEST(PD1600)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching memsection failed.");

            if (res == 0)
                pd_detach(newpd, memsec);

            memsection_delete(memsec);
        }

        pd_delete(newpd);
    }
}
END_TEST

/*
 * Repeat of the above test with a changed IID in the reference.
 *
 * EXECUTE_IID is chosen because this will result in an unaligned
 * pointer on most architectures.
 */
START_TEST(PD1601)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        newpd |= EXECUTE_IID;

        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            memsec |= EXECUTE_IID;

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching memsection failed.");

            if (res == 0)
                pd_detach(newpd, memsec);

            memsection_delete(memsec);
        }

        pd_delete(newpd);
    }
}
END_TEST

/* Feed null PDs and memsections to pd_attach() and expect errors. */
START_TEST(PD1700)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            res = pd_attach(newpd, 0, L4_FullyAccessible);

            fail_if(res == 0, "Attaching null memsection succeeded");

            res = pd_attach(0, memsec, L4_FullyAccessible);

            fail_if(res == 0, "Attaching memsection to null PD succeeded");

            res = pd_attach(0, 0, L4_FullyAccessible);

            fail_if(res == 0, "Attaching null memsection to null PD succeeded");

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching PD failed.");

            if (res == 0)
                pd_detach(newpd, memsec);

            memsection_delete(memsec);
        }

        pd_delete(newpd);
    }
}
END_TEST

#if 0
/* Disabled becuase Iguana does not track deleted memsections */
/*
 * Try and attach a deleted memsection to a PD.
 */
START_TEST(PD1800)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            memsection_delete(memsec);

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res == 0, "Attaching deleted memsection succeeded.");

            if (res == 0)
                pd_detach(newpd, memsec);

        }

        pd_delete(newpd);
    }
}
END_TEST
#endif

/*
 * Try and attach a memsection to a deleted PD.
 */
START_TEST(PD1900)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            pd_delete(newpd);

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res == 0, "Attaching memsection to deleted PD succeeded.");

            if (res == 0)
                pd_detach(newpd, memsec);

            memsection_delete(memsec);
        }
        else
            pd_delete(newpd);
    }
}
END_TEST

/*
 * Attach a memsection twice to the one PD.
 */
START_TEST(PD2000)
{
    pd_ref_t newpd = 0;
    uintptr_t base;

    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    if (newpd != 0) {
        memsection_ref_t memsec = 0;
        
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");

        if (memsec != 0) {
            int res;

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching memsection failed.");

            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching memsection failed.");

            if (res == 0) {
                pd_detach(newpd, memsec);
                pd_detach(newpd, memsec);
            }

            /* Attach again, to see if any data structures are
             * damaged.
             */
            res = pd_attach(newpd, memsec, L4_FullyAccessible);

            fail_if(res != 0, "Attaching memsection failed.");

            if (res == 0) {
                pd_detach(newpd, memsec);
            }

            memsection_delete(memsec);
        }

        pd_delete(newpd);
    }
}
END_TEST

/* 
 * Attach a memsection to 2 different pd 
 */
START_TEST(PD2100)
{
    pd_ref_t newpd1 = 0;
    pd_ref_t newpd2 = 0;
    memsection_ref_t memsec = 0;
    uintptr_t base;
    int res;

    newpd1 = pd_create();
    fail_if(newpd1 == 0, "NULL pd returned");
    newpd2 = pd_create();
    fail_if(newpd2 == 0, "NULL pd returned");

    if ((newpd1 != 0) && (newpd2 != 0)) {
        memsec = pd_create_memsection(pd_myself(), MEM_SIZE, &base);
        fail_if(memsec == 0, "NULL memsection returned");
        if (memsec != 0) {
            res = pd_attach(newpd1, memsec, L4_FullyAccessible);
            fail_if(res != 0, "Attaching memsection failed.");

            res = pd_attach(newpd2, memsec, L4_FullyAccessible);
            fail_if(res != 0, "Attaching memsection failed.");

            if (res == 0) {
                pd_detach(newpd1, memsec);
                pd_detach(newpd2, memsec);
            }
            memsection_delete(memsec);
        }
        pd_delete(newpd1);
        pd_delete(newpd2);
    }
}
END_TEST

/*
 * This test triggers the bug #1702 in function refcmp() which causes the security check to fail.
 * It can not be run on MIPS32 as MIPS32 address space ends at 0x80000000.
 */ 
#if !(defined(L4_ARCH_MIPS) && defined(L4_32BIT))
#define PD2200_MEM_SIZE 0x33000
START_TEST(PD2200)
{
    pd_ref_t newpd = 0;
    uintptr_t clist, clist_base;
    uintptr_t clist2, clist_base2;
    uintptr_t clist3, clist4, clist5;
    uintptr_t clist_ref;
    char *p;
    int i, res;

    clist_ref = 0x80000000;
    newpd = pd_create();
    fail_if(newpd == 0, "NULL pd returned");

    /* 
     * The following virtual address is needed to trigger the bug #1702 
     * If no such virtual address is available on the platfom, the test is
     * skipped
     */
    clist_base = 0xb0000000;
    clist3 = pd_create_memsection_fixed_user(newpd, PD2200_MEM_SIZE, clist_base);
    if (clist3 == 0) {
        printf("This test does not apply to this platform\n");
        return;
    }
    res = pd_attach(newpd, (memsection_ref_t)clist3, L4_FullyAccessible);
    fail_if(res != 0, "Attaching memsection failed.");
    clist_base += PD2200_MEM_SIZE;
    clist4 = pd_create_memsection_fixed_user(newpd, PD2200_MEM_SIZE, clist_base);
    if (clist4 == 0) {
        printf("This test does not apply to this platform\n");
        return;
    }
    res = pd_attach(newpd, (memsection_ref_t)clist4, L4_FullyAccessible);
    fail_if(res != 0, "Attaching memsection failed.");
    clist_base += PD2200_MEM_SIZE;
    clist5 = pd_create_memsection_fixed_user(newpd, PD2200_MEM_SIZE, clist_base);
    if (clist5 == 0) {
        printf("This test does not apply to this platform\n");
        return;
    }
    res = pd_attach(newpd, (memsection_ref_t)clist5, L4_FullyAccessible);
    fail_if(res != 0, "Attaching memsection failed.");
    clist_base += PD2200_MEM_SIZE;
    clist = pd_create_memsection_fixed_user(newpd, MEM_SIZE, clist_base);
    if (clist == 0) {
        printf("This test does not apply to this platform\n");
        return;
    }
    res = pd_attach(newpd, (memsection_ref_t)clist, L4_FullyAccessible);
    fail_if(res != 0, "Attaching memsection failed.");
    //printf("First memsection ref: %lx base: %lx\n", clist, clist_base);
    memsection_delete(clist3);
    memsection_delete(clist4);
    memsection_delete(clist5);
    //L4_KDB_Enter("PD2200");
    i = 0;
    do {
        clist2 = pd_create_memsection(newpd, MEM_SIZE, &clist_base2);
        res = pd_attach(newpd, (memsection_ref_t)clist2, L4_FullyAccessible);
        fail_if(res != 0, "Attaching memsection failed.");
        //printf("%lx, ", clist2);
        if (++i == 100) {
            clist2 = 0;
        }
    } while ((clist2 < clist_ref) && (clist2 != 0));
    if (clist && clist2) {
        //printf("\nSecond memsection ref: %lx, base: %lx\n", clist2, clist_base2);
        p = (char *)clist_base2;
        memset((void *)clist_base2, '%', 0x10);
        for (i = 0; i < 0x10; i++) {
            fail_if(*p != '%', "error data");
            p++;
        }
        memsection_delete(clist2);
    } else {
        printf("Test skipped: Could not create requested memsection\n");
    }

    memsection_delete(clist);
    pd_delete(newpd);
}
END_TEST
#endif

TCase *
pd_tests(void)
{
    TCase *tc;

    tc = tcase_create("PD tests");
    tcase_add_test(tc, PD0100);
    tcase_add_test(tc, PD0200);
    tcase_add_test(tc, PD0201);
#if 0
    tcase_add_test(tc, PD0202);
#endif
#ifdef CONFIG_LOWMEMORY
    tcase_add_test(tc, PD0300);
#endif
    tcase_add_test(tc, PD0400);
    tcase_add_test(tc, PD0401);
    tcase_add_test(tc, PD0500);
    tcase_add_test(tc, PD0501);
    tcase_add_test(tc, PD0600);
    // tcase_add_test(tc, test_pd_create_thread);
    // tcase_add_test(tc, test_pd_create_thread_with_priority);
    tcase_add_test(tc, PD0700);
    tcase_add_test(tc, PD0800);
    tcase_add_test(tc, PD0801);
    tcase_add_test(tc, PD0900);
    tcase_add_test(tc, PD1000);
    tcase_add_test(tc, PD1100);
    tcase_add_test(tc, PD1101);
    tcase_add_test(tc, PD1200);
    tcase_add_test(tc, PD1300);
    tcase_add_test(tc, PD1400);
    tcase_add_test(tc, PD1401);
    tcase_add_test(tc, PD1500);
    tcase_add_test(tc, PD1600);
    tcase_add_test(tc, PD1601);
    tcase_add_test(tc, PD1700);
    /*tcase_add_test(tc, PD1800);*/
    tcase_add_test(tc, PD1900);
    tcase_add_test(tc, PD2000);
    tcase_add_test(tc, PD2100);
#if !(defined(L4_ARCH_MIPS) && defined(L4_32BIT))
    tcase_add_test(tc, PD2200);
#endif

    return tc;
}
