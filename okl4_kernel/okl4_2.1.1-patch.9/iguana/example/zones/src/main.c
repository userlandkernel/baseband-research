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
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/zone.h>
#include <l4/kdebug.h>
#include <l4/types.h>

#define SIZE                    0x1000

static int data;
static const int rodata;
static void text(void) { }

static pd_ref_t
create_pd(void)
{
    memsection_ref_t data_ms;
    pd_ref_t pd;
    int r;
    memsection_ref_t rodata_ms;
    memsection_ref_t text_ms;

    pd = pd_create();
    if (pd == 0 || pd == -1) {
        printf("Failed to create PD.\n");
        return -1;
    }

    text_ms = memsection_lookup((uintptr_t)&text, NULL);
    if (text_ms == 0 || text_ms == -1) {
        printf("Failed to create text section.\n");
        return -1;
    }
    r = pd_attach(pd, text_ms, L4_ReadeXecOnly);
    if (r != 0) {
        printf("Failed to attach text section.\n");
        return -1;
    }

    rodata_ms = memsection_lookup((uintptr_t)&rodata, NULL);
    if (rodata_ms == 0 || rodata_ms == -1) {
        printf("Failed to create rodata section.\n");
        return -1;
    }
    r = pd_attach(pd, rodata_ms, L4_Readable);
    if (r != 0) {
        printf("Failed to attach rodata section.\n");
        return -1;
    }

    data_ms = memsection_lookup((uintptr_t)&data, NULL);
    if (data_ms == 0 || data_ms == -1) {
        printf("Failed to create data section.\n");
        return -1;
    }
    r = pd_attach(pd, data_ms, L4_ReadWriteOnly);
    if (r != 0) {
        printf("Failed to attach data section.\n");
        return -1;
    }

    return pd;
}

static thread_ref_t
create_thread(pd_ref_t pd, void (*start)(void), int priority)
{
    uintptr_t stack_base;
    int r;
    memsection_ref_t stack;
    thread_ref_t thread;
    L4_ThreadId_t tid;

    thread = pd_create_thread_with_priority(pd, priority, &tid);
    if (thread == 0 || thread == -1) {
        printf("Failed to create thread.\n");
        return -1;
    }

    stack = pd_create_memsection(pd, SIZE, &stack_base);
    if (stack == 0 || stack == -1) {
        printf("Failed to create stack.\n");
        return -1;
    }
    r = pd_attach(pd, stack, L4_ReadWriteOnly);
    if (r != 0) {
        printf("Failed to attach stack.\n");
        return -1;
    }

    thread_start(thread, (uintptr_t)start, stack_base + SIZE);
    return thread;
}

static volatile int * volatile addr;
static volatile int ready;
static volatile pd_ref_t other_pd;

static void
testfunc(void)
{
    printf("Hello from other PD.\n");
    while (ready != 1) { }
    printf("%d %d %d\n", addr[0], addr[1], addr[2]);
    addr[0] = 3;
    addr[1] = 4;
    addr[2] = 5;
    ready = 2;
    while (ready != 3) { }
    printf("%d %d %d\n", addr[0], addr[1], addr[2]);
    addr[0] = 9;
    addr[1] = 10;
    addr[2] = 11;
    ready = 4;
    while (1) { }
}

static void
mainfunc(void)
{
    uintptr_t base;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    printf("Hello from main.\n");
    zone = zone_create();
    if (zone == 0 || zone == -1) {
        printf("Failed to create zone.\n");
        while (1) { }
    }

    r = pd_attach_zone(pd_myself(), zone, L4_FullyAccessible);
    if (r != 0) {
        printf("Failed to attach zone.\n");
        while (1) { }
    }

    r = pd_attach_zone(other_pd, zone, L4_FullyAccessible);
    if (r != 0) {
        printf("Failed to attach zone to other PD.\n");
        while (1) { }
    }

    ms = memsection_create_in_zone(SIZE, &base, zone);
    if (ms == 0 || ms == -1) {
        printf("Failed to create memsection.\n");
        while (1) { }
    }

    addr = (volatile int *)base;

    addr[0] = 0;
    addr[1] = 1;
    addr[2] = 2;

    ready = 1;
    while (ready == 1) { }
    printf("%d %d %d\n", addr[0], addr[1], addr[2]);

    ms = memsection_create_in_zone(300 * SIZE, &base, zone);
    if (ms == 0 || ms == -1) {
        printf("Failed to create second memsection.\n");
        while (1) { }
    }

    addr = (volatile int *)base;

    addr[0] = 6;
    addr[1] = 7;
    addr[2] = 8;

    ready = 3;
    while (ready == 3) { }
    printf("%d %d %d\n", addr[0], addr[1], addr[2]);
    pd_detach_zone(other_pd, zone);
    printf("detached from the other guy\n");
    addr[0] = 9;
    pd_detach_zone(pd_myself(), zone);
    printf("detached from me\n");
    // addr[0] = 10;

    // zone_delete(zone);
    // while (1) { }
}

static void
mainfunc2(void)
{
    volatile int * volatile addr[64];
    uintptr_t base;
    int i;
    memsection_ref_t ms;
    int r;
    zone_ref_t zone;

    printf("Hello from main2.\n");
    for (i = 0; i < 64; i++) {
        zone = zone_create();
        if (zone == 0 || zone == -1) {
            printf("Failed to create zone.\n");
            while (1) { }
        }

        r = pd_attach_zone(pd_myself(), zone, L4_FullyAccessible);
        if (r != 0) {
            printf("Failed to attach zone.\n");
            while (1) { }
        }

        ms = memsection_create_in_zone(SIZE, &base, zone);
        if (ms == 0 || ms == -1) {
            printf("Failed to create memsection.\n");
            while (1) { }
        }

        addr[i] = (volatile int *)base;
    }

    for (i = 0; i < 2000; i++) {
        //printf(" >>> %d <<<\n", i % 64);
        addr[i % 64][0] = 0;
    }
    printf("Finished.\n");
}

int
main(void)
{
    thread_ref_t thread;

    printf("Zones example\n");
    //L4_KDB_Enter("waiting...");
    other_pd = create_pd();
    thread = create_thread(other_pd, testfunc, -1);
    mainfunc();
    mainfunc2();
    while (1) { }
    return 0;
}
