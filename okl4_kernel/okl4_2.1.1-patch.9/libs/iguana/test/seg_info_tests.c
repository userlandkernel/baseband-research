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

#include <string.h>
#include <stdio.h>
#include <iguana/segment_info.h>
#include <iguana/memsection.h>
#include <iguana/env.h>
#include "test_libs_iguana.h"

/*
 * The RVCT and ADS compilers produce different results to GCC.
 * Factor these differences out.
 */
#if defined (__RVCT__) || defined(__RVCT_GNU__) || defined (__ADS__)
/* Name of this test program's segment. */
#define SEGMENT_NAME "TEST.RWX"

/* Name of the crt0 entry point. */
#define ENTRY_POINT __main

/* Flag value.  RVCT uses non-standard extensions! */
#define has_expected_flags(flags) (((flags) & 0xf) == 7)
#else /* !RCVT and !ADS */
/* Name of this test program's segment. */
#define SEGMENT_NAME "TEST.RX"

/* Name of the crt0 entry point.  Assume that our crt0 is used. */
#define ENTRY_POINT _start

/* RX segment flags. */
#define has_expected_flags(flags) ((flags) == 5)
#endif

START_TEST(SEGINFO0100)
{
    int res;
    uintptr_t r_vaddr, r_flags, ms_base;
    uintptr_t r_paddr, r_offset, r_filesz, r_memsz;
    memsection_ref_t ms;
    thread_ref_t thread;

    res = get_segment_info("does_not_exist", &r_vaddr, &r_flags, &r_paddr,
                           &r_offset, &r_filesz, &r_memsz);
    fail_unless(res != 0, "Search for non existent segment succeeded.");

    res = get_segment_info(SEGMENT_NAME, &r_vaddr, &r_flags, &r_paddr,
                           &r_offset, &r_filesz, &r_memsz);
    fail_if(res, "Failed to get segment info");

    if (!res) {
        /*
         * Compare with the memsection for this segment.
         *
         * A memsection's base and size are rounded to page size
         * values, so they won't be exactly the same as the segment
         * details in the environment.
         */
        ms = memsection_lookup(r_vaddr, &thread);
        ms_base = (uintptr_t)memsection_base(ms);
        fail_unless((ms_base <= (uintptr_t)SEGINFO0100) 
                    && ((uintptr_t)SEGINFO0100 <= (ms_base + memsection_size(ms) - 1)), 
                    "Segment info returned bad virtual address");
        fail_unless(r_paddr != 0,
                    "Zero physical address returned.");
        fail_unless(r_offset != 0,
                    "Zero offset returned.");
        fail_unless(has_expected_flags(r_flags),
                    "Segment info returned bad flags");
        _fail_unless(r_filesz <= r_memsz, __FILE__, __LINE__,
                     "File size is larger than memory side, found %ld, expecting less than %ld.",
                     (long) r_filesz, (long) r_memsz);
        _fail_unless(memsection_size(ms) >= r_memsz, __FILE__, __LINE__,
                     "Memory size differs, found %ld, expecting %ld.",
                     (long) memsection_size(ms), (long) r_memsz);
    }


    /*
     * The function should succeed even if we're not asking for
     * anything.
     */
    res = get_segment_info(SEGMENT_NAME, NULL, NULL, NULL,
                           NULL, NULL, NULL);
    fail_if(res, "Failed to get empty segment info");
    
}
END_TEST

extern void ENTRY_POINT(void);

START_TEST(SEGINFO0200)
{
    int res;
    uintptr_t type;
    uintptr_t entry;

    res = get_elf_info("does_not_exist", &type, &entry);
    fail_unless(res != 0, "Search for non existent ELF file succeeded.");

    res = get_elf_info("TEST", &type, &entry);
    fail_if(res, "Failed to get elf file info");

    if (!res) {
        fail_unless(type == ELF_TYPE_PROGRAM,
                    "Unexpected ELF type returned.");
        fail_unless(entry == (uintptr_t) ENTRY_POINT,
                    "Unexpected entry point returned.");
    }
}
END_TEST

#if 0
START_TEST(SEGINFO0300)
{
    const envitem_t* envitem;

    envitem = iguana_getenv("MAIN/STACK");
    fail_if(envitem == NULL,
            "Couldn't find stack entry.");

    envitem = iguana_getenv("main/stack");
    fail_if(envitem == NULL,
            "Couldn't find stack entry (lowercase).");

    envitem = iguana_getenv("does_not_exist");
    fail_if(envitem != NULL,
            "Search for non-existent item succeeded.");

    envitem = env_get_next(NULL);
    fail_if(envitem == NULL,
            "Failed to get the first item.");
    if (envitem != NULL) {
        /*
         * Check the name of the first item.  This test is dependent
         * on the current implementation and may fail if something in
         * elfweaver changes.
         */
        fail_unless(strcmp(env_name(envitem), "MAIN") == 0,
                    "Unexpected name for first item.");

        /*
         * Run through the list.
         * Because the contents of the object environment are unknown
         * it is not possible to test things here.  However, if the
         * test hangs then something is wrong!
         */
        while ((envitem = env_get_next(envitem)) != NULL) {
        }
    }
}
END_TEST
#endif

TCase *
seg_info_tests()
{
    TCase *tc;

    tc = tcase_create("Segment Info");
    tcase_add_test(tc, SEGINFO0100);
    tcase_add_test(tc, SEGINFO0200);
    //tcase_add_test(tc, SEGINFO0300);

    return tc;
} 
