/*
 * Copyright (c) 2005, National ICT Australia
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

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <stddef.h>
#include <l4/ipc.h>
#include <l4/config.h>
/*
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/misc.h>
#include <l4e/map.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TEST_TIDS       20
static L4_ThreadId_t test_tid[TEST_TIDS];
static L4_ThreadId_t main_tid;

static void utcb_test_thread(void)
{
    L4_Word_t* utcb = L4_GetUtcbBase ();

    L4_MsgTag_t tag;
    L4_Word_t i;

    //printf("%p, utcb_size = %ld\n", utcb, utcb_size);

    while (1) {
        L4_Send(main_tid);

        /* Wait for handler */
        tag = L4_Receive(main_tid);

        for (i=511; i >= 64; i--)
            utcb[i] = 0x55555555;
            //utcb[i] = 0x12345678; - kernel crash!! - signals premption

        deleteThread(test_tid[14]);

        for (i=64; i < 512; i++)
            fail_unless(utcb[i] == 0x55555555, "UTCBs corrupted");
    }
}

/*
 * This Test is a pain, its currently very hardcoded and may
 * break if you change anything here. Please verify it caches
 * UTCB corruption due to unaligned cache-invalidate in the kernel
 * on UTCB page freeing.
 *
 * How it works:
 * Create n threads, where n > m  and m is the first thread that
 * gets allocated a new UTCB page. Make sure m is the last thread
 * in that page, ipc to (m-1) who corrupts UTCBs (x..m-2) and then
 * deletes m. Then (m-1) checks the corrupted UTCBs still have the
 * same data.
*/
START_TEST(ARM_UTCB_DELETE)
{
    L4_MsgTag_t tag;
    int loop, i;

    if (L4_GetUtcbSize() != 0) {
        return;
    }

    initThreads(0);

    for (loop = 0; loop < 10; loop++)
    {
        for (i=0; i < TEST_TIDS; i++)
            test_tid[i] = createThreadInSpace(L4_rootspace, utcb_test_thread);

        for (i=0; i < TEST_TIDS; i++)
        {
            L4_Receive(test_tid[i]);
        }

        for (i=TEST_TIDS-1; i >= 15; i--) {
            deleteThread(test_tid[i]);
        }

        for (i=13; i <= 13; i++)
        {
            tag = L4_Niltag;
            L4_Set_MsgTag(tag);
            L4_Send(test_tid[i]);
        }

        for (i=13; i <= 13; i++)
        {
            L4_Receive(test_tid[i]);
        }

        for (i=0; i <= 13; i++) {
            deleteThread(test_tid[i]);
        }
    }
}
END_TEST

/* -------------------------------------------------------------------------*/

TCase * arm_append_bugs(TCase *tc);

TCase * arm_append_bugs(TCase *tc)
{
    tcase_add_test(tc, ARM_UTCB_DELETE);

    return tc;
}
