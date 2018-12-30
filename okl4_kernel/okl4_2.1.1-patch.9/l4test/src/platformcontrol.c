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
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/misc.h>
#include <l4/security.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
\begin{test}{PLAT0100}
  \TestDescription{Check platform control not implemented}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_PlatformControl}
    \item Check that the return value is 0
    \item Check that the error code in \Func{ENOT\_IMPLEMENTED}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/

static void test_thread(void)
{
    L4_ThreadId_t from = L4_nilthread;
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_Word_t result;

    /* Wait for msg */
    tag = L4_Wait(&from);
    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not get message.  Error: %ld",
                 (int) L4_ErrorCode ());

    result = L4_PlatformControl(0, 0, 0, 0);

    L4_MsgClear (&msg);
    L4_MsgAppendWord(&msg, (result != 0) ? -1 : L4_ErrorCode ());
    L4_MsgLoad (&msg);
    tag = L4_Call(from);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not send message.  Error: %ld",
                 (int) L4_ErrorCode ());
}

START_TEST(PLAT0100)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_SpaceId_t space;
    L4_ThreadId_t thread;
    L4_Word_t result = -10;

    space = createSpace();
    thread = createThreadInSpace(space, test_thread);

    L4_AllowPlatformControl(space);

    L4_MsgClear (&msg);
    L4_Set_MsgLabel (&msg, 0x1);
    L4_MsgLoad (&msg);
    tag = L4_Call (thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not send message to thread.  Error: %ld",
                 (int) L4_ErrorCode ());

    L4_StoreMR(1, &result);

    /* check for NOT_IMPLEMENTED */
    _fail_unless (result == 16, __FILE__, __LINE__, "Wrong error code: %ld",
                 (int) result );

    deleteThread(thread);
    deleteSpace(space);
}
END_TEST

/*
\begin{test}{PLAT0200}
  \TestDescription{Check platform control privilege}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_PlatformControl} from a deprivileged thread
    \item Check that the return value is 0
    \item Check that the error code in \Func{ENO\_PRIVILEGE}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(PLAT0200)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_SpaceId_t space;
    L4_ThreadId_t thread;
    L4_Word_t result = -10;

    space = createSpace();
    thread = createThreadInSpace(space, test_thread);

    L4_MsgClear (&msg);
    L4_Set_MsgLabel (&msg, 0x1);
    L4_MsgLoad (&msg);
    tag = L4_Call (thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not send message to thread.  Error: %ld",
                 (int) L4_ErrorCode ());

    L4_StoreMR(1, &result);

    /* check for NO_PRIVILEGE */
    _fail_unless (result == 1, __FILE__, __LINE__, "Wrong error code: %ld",
                 (int) result );

    deleteThread(thread);
    deleteSpace(space);
}
END_TEST

static void test_setup(void)
{
    initThreads(0);
}

static void test_teardown(void)
{
}

TCase *
make_platform_control_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Platform Control Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, PLAT0100);
    tcase_add_test(tc, PLAT0200);

    return tc;
}
