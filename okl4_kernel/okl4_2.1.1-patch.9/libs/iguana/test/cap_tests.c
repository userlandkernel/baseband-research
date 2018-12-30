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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <iguana/debug.h>
#include <iguana/memsection.h>
#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/session.h>
#include <iguana/cap.h>
#include <iguana/eas.h>
#include <iguana/object.h>
#include "test_libs_iguana.h"

#include <interfaces/iguana_client.h>

/*
 * Test clist_create()
 */
START_TEST(CAP0100)
{
    clist_ref_t clist;

    clist = clist_create();
    fail_if(clist == 0, "reference not zero");

    clist_delete(clist);
}
END_TEST

/*
 * Test pd_create_clist()
 */
START_TEST(CAP0200)
{
    clist_ref_t clist;
    pd_ref_t pd;

    pd = pd_create();
    clist = pd_create_clist(pd);
    fail_if(clist == 0, "reference not zero");

    clist_delete(clist);
    pd_delete(pd);
}
END_TEST

/*
 * Test clist_insert
 */
START_TEST(CAP0300)
{
    clist_ref_t clist;
    memsection_ref_t ref;
    uintptr_t base;
    int status;
 
    clist = clist_create();
    fail_if(clist == 0, "reference not zero");
 
    ref = memsection_create(MEM_SIZE, &base);
 
    status = clist_insert(clist, iguana_get_cap(ref, MASTER_IID));
    fail_unless(status == 0, "clist_insert failed");
 
    memsection_delete(ref);
    clist_delete(clist);
}
END_TEST

/*
 * Test clist_lookup
 */
START_TEST(CAP0400)
{
    clist_ref_t clist;
    memsection_ref_t ref;
    cap_t cap, cap2;
    uintptr_t base;
    int status;
 
    clist = clist_create();
    fail_if(clist == 0, "reference not zero");
 
    ref = memsection_create(MEM_SIZE, &base);
 
    cap2 = iguana_get_cap(ref, MASTER_IID); 
    status = clist_insert(clist, cap2);
    fail_unless(status == 0, "clist_insert failed");

    status = clist_lookup(clist, ref, MASTER_IID, &cap);
    fail_unless(status == 0, "clist_lookup failed");
 
    memsection_delete(ref);
    clist_delete(clist);
}
END_TEST

/*
 * Test clist_remove
 */
START_TEST(CAP0500)
{
    clist_ref_t clist;
    memsection_ref_t ref;
    cap_t cap;
    uintptr_t base;
    int status;
 
    clist = clist_create();
    fail_if(clist == 0, "reference not zero");
 
    ref = memsection_create(MEM_SIZE, &base);
 
    status = clist_insert(clist, iguana_get_cap(ref, MASTER_IID));
    fail_unless(status == 0, "clist_insert failed");

    status = clist_lookup(clist, ref, MASTER_IID, &cap);
    fail_unless(status == 0, "clist_lookup failed");

    status = clist_remove(clist, iguana_get_cap(ref, MASTER_IID));
    fail_unless(status == 0, "clist_remove failed");

    status = clist_remove(clist, iguana_get_cap(ref, MASTER_IID));
    fail_if(status == 0, "clist_remove should not have found a cap to delete");
 
    memsection_delete(ref);
    clist_delete(clist);
}
END_TEST

TCase *
cap_tests()
{
    TCase *tc;

    tc = tcase_create("CAP Sections");
    tcase_add_test(tc, CAP0100);
    tcase_add_test(tc, CAP0200);
    tcase_add_test(tc, CAP0300);
    tcase_add_test(tc, CAP0400);
    tcase_add_test(tc, CAP0500);

    return tc;
}
