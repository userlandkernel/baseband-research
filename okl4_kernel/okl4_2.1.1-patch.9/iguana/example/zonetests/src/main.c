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
#include <iguana/env.h>
#include <iguana/memsection.h>
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/zone.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/types.h>
#include <l4/schedule.h>
#include <compat/c.h>
#include "util.h"

#define BIG_NUMBER              1000000
#define MAGIC                   0x12321
#define N                       33
#define M                       11
#define X                       6

#define OK()                    printf("OK.\n")
#define FAILED()                printf("FAILED (line=%d).\n", __LINE__)

static L4_ThreadId_t main_tid;

static int use_direct_memsection;

/*
 * A comment on the serialise_mutex.
 *
 * Some tests fire off high priority subthreads to test various
 * faulting conditions.  Our regression test infrastructure expects
 * output in a certain order, which in this case means that the
 * subthread faults before the next test starts.
 *
 * Without case, this can fail on SMP processors because the fault can
 * occur after the next test starts.  The variable serialise_mutex is
 * present to stop this from happening.  It is locked by the main
 * thread and then the main thread spins until the mutex is unlocked
 * by the subthread just before it exits.
 */

static void
setup(void)
{
    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    okl4_mutex_init(serialise_mutex);
}

/*
 * Test01: Attach zone, create memsection, touch it.
 */
static void
zonetest01(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    /* If we don't fault, we have succeeded. */
    OK();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test01a: Attach zone read only, create memsection, write to it.  Fault!
 */
static void
zonetest01a(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x4);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    *(volatile int *)base = 1;

    /* If we don't fault, we have failed. */
    FAILED();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test02: Create memsection, attach zone, touch it.
 */
static void
zonetest02(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    /* If we don't fault, we have succeeded. */
    OK();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test03: Create memsection, touch it, fault!
 */
static void
zonetest03(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    zone_ref_t zone;

    ARCH_THREAD_INIT;

    okl4_mutex_lock(serialise_mutex);
    L4_LoadMR(0, 0);
    L4_Send(main_tid);

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    printf("Faulting...\n");
    okl4_mutex_unlock(serialise_mutex);

    (void) *(volatile int *)base;

    /* If we don't fault, we have failed. */
    FAILED();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test04: Attach N zones, create memsections, touch them.
 */
static void
zonetest04(void)
{
    uintptr_t base[N];
    int i;
    memsection_ref_t ms[N];
    int r;
    zone_ref_t zone[N];

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    for (i = 0; i < N; i++) {
        zone[i] = zone_create();
        if (zone[i] == 0UL || zone[i] == -1UL) {
            FAILED();
            return;
        }

        r = pd_attach_zone(pd_myself(), zone[i], 0x7);
        if (r != 0) {
            FAILED();
            return;
        }
    }

    for (i = 0; i < N; i++) {
        if (use_direct_memsection) {
            ms[i] = memsection_create_direct_in_zone(SIZE, &base[i], zone[i]);
        }
        else {
            ms[i] = memsection_create_in_zone(SIZE, &base[i], zone[i]);
        }

        if (ms[i] == 0UL || ms[i] == -1UL) {
            FAILED();
            return;
        }
    }

    for (i = 0; i < N; i++) {
        (void) *(volatile int *)base[i];
    }

    /* If we don't fault, we have succeeded. */
    OK();

    /* Clean up */
    for (i = 0; i < N; i++) {
        memsection_delete(ms[i]);
    }

    for (i = 0; i < N; i++) {
        zone_delete(zone[i]);
    }
}

/*
 * Test05: Create memsection, attach zone, touch, detach, touch, fault!
 */
static void
zonetest05(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    ARCH_THREAD_INIT;

    okl4_mutex_lock(serialise_mutex);
    L4_LoadMR(0, 0);
    L4_Send(main_tid);

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x7);
    if (r != 0) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    (void) *(volatile int *)base;

    pd_detach_zone(pd_myself(), zone);

    printf("Faulting...\n");
    okl4_mutex_unlock(serialise_mutex);

    (void) *(volatile int *)base;

    /* If we don't fault, we have failed. */
    FAILED();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test06: Create memsection, attach zone, delete, touch, fault!
 */
static void
zonetest06(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    ARCH_THREAD_INIT;

    okl4_mutex_lock(serialise_mutex);
    L4_LoadMR(0, 0);
    L4_Send(main_tid);

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x7);
    if (r != 0) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    memsection_delete(ms);

    printf("Faulting...\n");
    okl4_mutex_unlock(serialise_mutex);

    (void) *(volatile int *)base;

    /* If we don't fault, we have failed. */
    FAILED();

    /* Clean up */
    zone_delete(zone);
}

/*
 * Test07: Two PDs, attach zone, create memsection, touch (from both).
 */
static uintptr_t zonetest07_base;

static void zonetest07_helper(void)
{
    ARCH_THREAD_INIT;
    (void) *(volatile int *)zonetest07_base;

    L4_Call(main_tid);

    while (1) { }
}

static void
zonetest07(void)
{
    memsection_ref_t ms;
    pd_ref_t pd1;
    pd_ref_t pd2;
    int r;
    L4_MsgTag_t tag;
    thread_ref_t thread1;
    thread_ref_t thread2;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    pd1 = create_pd();
    if (pd1 == 0UL || pd1 == -1UL) {
        FAILED();
        return;
    }

    pd2 = create_pd();
    if (pd2 == 0UL || pd2 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd1, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd2, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &zonetest07_base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &zonetest07_base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    thread1 = create_thread(pd1, zonetest07_helper, 101, 0);
    if (thread1 == 0UL || thread1 == -1UL) {
        FAILED();
        return;
    }

    thread2 = create_thread(pd2, zonetest07_helper, 101, 0);
    if (thread2 == 0UL || thread2 == -1UL) {
        FAILED();
        return;
    }

    tag = L4_Receive(thread_l4tid(thread1));
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    tag = L4_Receive(thread_l4tid(thread2));
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    /* If everyone reports back, we have succeeded. */
    OK();

    /* Clean up */
    thread_delete(thread_l4tid(thread1));
    thread_delete(thread_l4tid(thread2));
    pd_delete(pd1);
    pd_delete(pd2);
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test08: Two PDs, attach to A, create memsection, touch, B faults!
 */
static uintptr_t zonetest08_base;

static void zonetest08_helper1(void)
{
    ARCH_THREAD_INIT;
    (void) *(volatile int *)zonetest08_base;

    L4_Call(main_tid);

    while (1) { }
}

static void zonetest08_helper2(void)
{
    ARCH_THREAD_INIT;

    printf("Faulting...\n");
    (void) *(volatile int *)zonetest08_base;
    /* If we don't fault, we have failed. */
    FAILED();

    while (1) { }
}

static void
zonetest08(void)
{
    memsection_ref_t ms;
    pd_ref_t pd1;
    pd_ref_t pd2;
    int r;
    L4_MsgTag_t tag;
    thread_ref_t thread1;
    thread_ref_t thread2;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    pd1 = create_pd();
    if (pd1 == 0UL || pd1 == -1UL) {
        FAILED();
        return;
    }

    pd2 = create_pd();
    if (pd2 == 0UL || pd2 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd1, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &zonetest08_base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &zonetest08_base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    thread1 = create_thread(pd1, zonetest08_helper1, 101, 0);
    if (thread1 == 0UL || thread1 == -1UL) {
        FAILED();
        return;
    }

    thread2 = create_thread(pd2, zonetest08_helper2, 102, 0);
    if (thread2 == 0UL || thread2 == -1UL) {
        FAILED();
        return;
    }

    tag = L4_Receive(thread_l4tid(thread1));
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    /* B doesn't report back... */

    /* Clean up */
    thread_delete(thread_l4tid(thread1));
    pd_delete(pd1);
    pd_delete(pd2);
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test09: Two PDs, attach zone, create memsection, detach from B, B faults.
 */
static uintptr_t zonetest09_base;

static void zonetest09_helper1(void)
{
    ARCH_THREAD_INIT;
    (void) *(volatile int *)zonetest09_base;

    L4_Call(main_tid);

    while (1) { }
}

static void zonetest09_helper2(void)
{
    ARCH_THREAD_INIT;
    printf("Faulting... at %lx\n", (long) zonetest09_base);
    (void) *(volatile int *)zonetest09_base;

    /* If we don't fault, we have failed. */
    FAILED();

    while (1) { }
}

static void
zonetest09(void)
{
    memsection_ref_t ms;
    pd_ref_t pd1;
    pd_ref_t pd2;
    int r;
    L4_MsgTag_t tag;
    thread_ref_t thread1;
    thread_ref_t thread2;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    pd1 = create_pd();
    if (pd1 == 0UL || pd1 == -1UL) {
        FAILED();
        return;
    }

    pd2 = create_pd();
    if (pd2 == 0UL || pd2 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd1, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd2, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &zonetest09_base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &zonetest09_base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    pd_detach_zone(pd2, zone);

    thread1 = create_thread(pd1, zonetest09_helper1, 101, 0);
    if (thread1 == 0UL || thread1 == -1UL) {
        FAILED();
        return;
    }

    thread2 = create_thread(pd2, zonetest09_helper2, 102, 0);
    if (thread2 == 0UL || thread2 == -1UL) {
        FAILED();
        return;
    }

    tag = L4_Receive(thread_l4tid(thread1));
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    /* B doesn't report back... */

    /* Clean up */
    thread_delete(thread_l4tid(thread1));
    pd_delete(pd1);
    pd_delete(pd2);
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test10: Two zones, attach zone1, create memsection in zone1, write, delete
 * memsection, detach zone1, attach zone2, create memsection in zone2 at same
 * address, access ms and ensure old data not there.
 */
static void
zonetest10(void)
{
    memsection_ref_t ms;
    uintptr_t base1, base2;
    int r;
    zone_ref_t zone1;
    zone_ref_t zone2;

    printf("%s: ", __func__);

    zone1 = zone_create();
    if (zone1 == 0UL || zone1 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone1, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base1, zone1);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base1, zone1);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    *(volatile int *)base1 = 1;

    memsection_delete(ms);

    pd_detach_zone(pd_myself(), zone1);

    zone2 = zone_create();
    if (zone2 == 0UL || zone2 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone1, 0x7);
    //r = pd_attach_zone(pd_myself(), zone2, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base2, zone1);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base2, zone1);
    }

    //ms = memsection_create_in_zone(SIZE, &base2, zone2);
    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    if ((base1 != base2) || (*(volatile int *)base2 == 1)) {
        printf("base1 = %lx, base2 = %lx\n", (long) base1, (long) base2);
        FAILED();
        return;
    }

    /* If we don't fault, we have succeeded. */
    OK();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone1);
    zone_delete(zone2);
}

/*
 * Test11: Create memsection, attach zone read-only, touch,  fault!
 *
 * This is like test 01a, but is run in a separate thread in the same
 * PD.
 */
static void
zonetest11(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    ARCH_THREAD_INIT;

    okl4_mutex_lock(serialise_mutex);
    L4_LoadMR(0, 0);
    L4_Send(main_tid);

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x4);
    if (r != 0) {
        FAILED();
        okl4_mutex_unlock(serialise_mutex);

        return;
    }

    printf("Faulting...\n");
    okl4_mutex_unlock(serialise_mutex);

    *(volatile int *)base = 1;

    /* If we don't fault, we have failed. */
    FAILED();

    /* Clean up */
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test12: Two PDs, one RO, one RW, A write, B reads, B writes, fault!
 */
static uintptr_t zonetest12_base;

static void zonetest12_helper1(void)
{
    ARCH_THREAD_INIT;

    if (*(volatile int *)zonetest12_base != 0) {
        FAILED();
    }

    *(volatile int *)zonetest12_base = MAGIC;

    L4_Call(main_tid);

    while (1) { }
}

static void zonetest12_helper2(void)
{
    ARCH_THREAD_INIT;

    if (*(volatile int *)zonetest12_base != MAGIC) {
        FAILED();
    }

    printf("Faulting...\n");
    *(volatile int *)zonetest12_base = 2 * MAGIC;

    /* If we don't fault, we have failed. */
    FAILED();

    while (1) { }
}

static void
zonetest12(void)
{
    memsection_ref_t ms;
    pd_ref_t pd1;
    pd_ref_t pd2;
    int r;
    L4_MsgTag_t tag;
    thread_ref_t thread1;
    thread_ref_t thread2;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    pd1 = create_pd();
    if (pd1 == 0UL || pd1 == -1UL) {
        FAILED();
        return;
    }

    pd2 = create_pd();
    if (pd2 == 0UL || pd2 == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd1, zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd2, zone, 0x4);
    if (r != 0) {
        FAILED();
        return;
    }


    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &zonetest12_base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &zonetest12_base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    thread1 = create_thread(pd1, zonetest12_helper1, 102, 0);
    if (thread1 == 0UL || thread1 == -1UL) {
        FAILED();
        return;
    }

    thread2 = create_thread(pd2, zonetest12_helper2, 101, 0);
    if (thread2 == 0UL || thread2 == -1UL) {
        FAILED();
        return;
    }

    tag = L4_Receive(thread_l4tid(thread1));
    if (L4_IpcFailed(tag)) {
        FAILED();
        return;
    }

    /* B doesn't report back... */

    /* Clean up */
    thread_delete(thread_l4tid(thread1));
    pd_delete(pd1);
    pd_delete(pd2);
    memsection_delete(ms);
    zone_delete(zone);
}

/*
 * Test13: Create N memsections in N different zones. Create M PDs. Write data
 * in each memsection.
 * For each PD attach X of the N zones. Check each PD sees valid data for each
 * zone attached. Check each PD has no access to zones not attached.
 */
static volatile int threads_alive = 0;
static uintptr_t zonetest13_base[N];
static int attach_array[M][X];

static void zonetest13_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t nb;
    int i, j;

    ARCH_THREAD_INIT;

    tag = L4_Receive(main_tid);
    nb = L4_Label(tag);
    for (i = 0; i < X; i++) {
        j = attach_array[nb][i];
        //printf("Thread %lu(%lx) touching ms %d\n", nb, L4_Myself().raw, j);
        if (*(volatile int *)zonetest13_base[j] != j + 1) {
            L4_Send(main_tid);
            FAILED();
        }
    }

    L4_Call(main_tid);

    while (1) { }
}

static void zonetest13_faulting_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t nb_zone;

    ARCH_THREAD_INIT;

    tag = L4_Receive(main_tid);
    nb_zone = L4_Label(tag);
    //printf("Thread %lx touching ms %lu\n", L4_Myself().raw, nb_zone);
    printf("Faulting...\n");

    threads_alive--;
    (void) *(volatile int *)zonetest13_base[nb_zone];

    /* If we don't fault, we have failed. */
    L4_LoadMR(0, 0);
    L4_Send(main_tid);


    while (1) { }
}

static void
zonetest13(void)
{
    int i, j, k;
    memsection_ref_t ms[N];
    int r;
    zone_ref_t zone[N];
    pd_ref_t pd[M];
    thread_ref_t thread[M], tref;
    L4_ThreadId_t tid;
    L4_MsgTag_t tag, rtag;

    printf("%s: ", __func__);

    for (i = 0; i < N; i++) {
        zone[i] = zone_create();
        if (zone[i] == 0UL || zone[i] == -1UL) {
            FAILED();
            return;
        }

        r = pd_attach_zone(pd_myself(), zone[i], 0x7);
        if (r != 0) {
            FAILED();
            return;
        }
    }

    for (i = 0; i < N; i++) {
        ms[i] = memsection_create_in_zone(SIZE, zonetest13_base + i, zone[i]);
        if (ms[i] == 0UL || ms[i] == -1UL) {
            FAILED();
            return;
        }
    }

    for (i = 0; i < N; i++) {
        *(volatile int *)zonetest13_base[i] = i + 1;
    }

    for (i = 0, j = 0; i < M; i++) {
        pd[i] = create_pd();
        if (pd[i] == 0UL || pd[i] == -1UL) {
            FAILED();
            return;
        }
        for (k = 0; k < X; k++, j++) {
            if (j == N)
                j = 0;
            r = pd_attach_zone(pd[i], zone[j], 0x7);
            if (r != 0) {
                FAILED();
                return;
            }
            attach_array[i][k] = j;
        }
        thread[i] = create_thread(pd[i], zonetest13_thread, 101, 0);
        if (thread[i] == 0UL || thread[i] == -1UL) {
            FAILED();
            return;
        }
    }

    /* Touching the attached memsections */
    for (i = 0; i < M; i++) {
        tid = thread_l4tid(thread[i]);
        tag = L4_Niltag;
        L4_Set_Label(&tag, (L4_Word_t)i);
        L4_Set_MsgTag(tag);
        rtag = L4_Call(tid);
        if (L4_IpcFailed(rtag)) {
            FAILED();
            return;
        }
    }
    /* If we don't fault, we have succeeded. */
    OK();

    /* Touching the memsections not attached */
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k <X; k++) {
                if (j == attach_array[i][k])
                    break;
            }
            if (k < X)
                continue;
            threads_alive++;
            tref = create_thread(pd[i], zonetest13_faulting_thread, 101, 0);
            tid = thread_l4tid(tref);
            tag = L4_Niltag;
            L4_Set_Label(&tag, (L4_Word_t)j);
            L4_Set_MsgTag(tag);
            rtag = L4_Send(tid);
            if (L4_IpcFailed(rtag)) {
                FAILED();
                return;
            }
            rtag = L4_Receive_Nonblocking(tid);
            if (L4_IpcSucceeded(rtag)) {
                /* If we don't fault, we have failed. */
                FAILED();
                return;
            }
        }
    }

    while (threads_alive > 0) {
        L4_Yield();
    }

    /* Clean up */
    for (i = 0; i < M; i++) {
        thread_delete(thread_l4tid(thread[i]));
        pd_delete(pd[i]);
    }
    for (i = 0; i < N; i++) {
        memsection_delete(ms[i]);
        zone_delete(zone[i]);
    }

    /* We have succeeded. */
    OK();
}

/*
 * XXX: This test is really, really slow!
 */
#if 0
/*
 * Test14: Test for some possible memory leaks.
 */
static void
zonetest14(void)
{
    int i;
    memsection_ref_t ms;
    pd_ref_t pd;
    int r;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    for (i = 0; i < BIG_NUMBER; i++) {
        pd = create_pd();
        if (pd == 0UL || pd == -1UL) {
            FAILED();
            return;
        }

        r = pd_attach_zone(pd, zone, 0x7);
        if (r != 0) {
            FAILED();
            return;
        }

        ms = memsection_create_in_zone(SIZE, &zonetest07_base, zone);
        if (ms == 0UL || ms == -1UL) {
            FAILED();
            return;
        }

        memsection_delete(ms);
        pd_delete(pd);
    }

    /* If we make it this far, we passed. */
    OK();

    /* Clean up */
    zone_delete(zone);
}
#endif

/*
 * Test15: Touch the memsection in a zone created by elfweaver.
 */


/*
 * Non-GNU compilers don't support the SECTION macro, so run the test
 * anyway, but with the function in the normal text segment, to keep
 * the test output consistent.
 */

#ifndef __GNUC__
#define SECTION(x)
#endif

/* Function living in a zone that we will call. */
int SECTION(".zone") zone_call(int x, int y);

int SECTION(".zone") zone_call(int x, int y) 
{
    return x * y;
}


static void
zonetest15(void)
{
    void* base;
    uintptr_t new_base;
    zone_ref_t zone;
    int zone_ret;
    memsection_ref_t ms;

    printf("%s: ", __func__);

    zone = env_zone(iguana_getenv("Z1"));
    
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    /* Find and read from all of the expected memsections. */
    base = env_memsection_base(iguana_getenv("Z1/ZONED_MS"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    base = env_memsection_base(iguana_getenv("Z1/ZONED_MS1"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    base = env_memsection_base(iguana_getenv("Z1/ZONED_MS2"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    base = env_memsection_base(iguana_getenv("Z1/ZONED_MS3"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    base = env_memsection_base(iguana_getenv("Z1/ZONED_MS4"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    base = env_memsection_base(iguana_getenv("Z1/ZONED_DIRECT"));

    if (base == NULL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    /* Call the function that is in the segment zone. */
    zone_ret = zone_call(10, 20);

    printf("Zone call return: %d\n", zone_ret);

    /* Test that there is free space in the memsection zone. */
    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &new_base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &new_base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)new_base;
    /* If we don't fault, we have succeeded. */
    OK();
}

/*
 * Test16: Attach zone, create multiple memsections, touch them then
 * delete first memsection.  It should still be possible to touch the
 * second memsection.
 */
static void
zonetest16(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    memsection_ref_t ms1;
    int r;
    zone_ref_t zone;

    printf("%s: ", __func__);

    /* XXX: We won't support dynamically created zones. */
    zone = zone_create();
    if (zone == 0UL || zone == -1UL) {
        FAILED();
        return;
    }

    r = pd_attach_zone(pd_myself(), zone, 0x7);
    if (r != 0) {
        FAILED();
        return;
    }

    if (use_direct_memsection) {
        ms = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms == 0UL || ms == -1UL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    if (use_direct_memsection) {
        ms1 = memsection_create_direct_in_zone(SIZE, &base, zone);
    }
    else {
        ms1 = memsection_create_in_zone(SIZE, &base, zone);
    }

    if (ms1 == 0UL || ms1 == -1UL) {
        FAILED();
        return;
    }

    (void) *(volatile int *)base;

    memsection_delete(ms);

    (void) *(volatile int *)base;

    /* If we don't fault, we have succeeded. */
    OK();

    /* Clean up */
    memsection_delete(ms1);
    zone_delete(zone);
}

int
main(int argc, char *argv[])
{
    printf("Zone tests\n");
    printf("----------\n");

    setup();

    for (use_direct_memsection = 0; use_direct_memsection < 2; use_direct_memsection++)
    {
        zonetest01();
        if (0) zonetest01a();
        zonetest02();
        create_thread(pd_myself(), zonetest03, 101, 1);
        zonetest04();
        create_thread(pd_myself(), zonetest05, 101, 1);
        create_thread(pd_myself(), zonetest06, 101, 1);
        zonetest07();
        zonetest08();
        zonetest09();
        zonetest10();
        create_thread(pd_myself(), zonetest11, 101, 1);
        zonetest12();
        zonetest13();
#if 0
        zonetest14();
#endif
        zonetest15();
        zonetest16();
    }

    printf("Finished.\n");
    return 0;
}
