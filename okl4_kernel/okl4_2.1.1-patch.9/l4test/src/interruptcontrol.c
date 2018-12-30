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
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/interrupt.h>
#include <l4/security.h>
#include <stdio.h>
#include <l4/kdebug.h>

L4_ThreadId_t main_tid;
extern L4_ThreadId_t test_tid;


#if defined(RUN_TESTS)
/*
\begin{test}{IRQ0100}
  \TestDescription{Check a thread can not call InterruptControl without permission}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{ENO\_PRIVILEGE}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0100)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    fail_unless(result == 0, "InterruptControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
}
END_TEST

/*
\begin{test}{IRQ0200}
  \TestDescription{Check a thread with permission granted can call InterruptControl}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Check that the return value is 1.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0200)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to register handler: Error=%lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_UnregisterInterrupt(main_tid, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to unregister handler: Error=%lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
}
END_TEST

static void
registering_thread(L4_ThreadId_t myself)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;
    L4_Word_t fault = L4_UserDefinedHandle();

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RegisterInterrupt(myself, notifybits, 0, 0);
    if (fault) {
        fail_unless(result == 0, "InterruptControl did not fail");
        fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, "Wrong error code");
    } else {
        _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to register handler: Error=%lu", L4_ErrorCode());
        L4_LoadMR(0, VALID_IRQ1);
        result = L4_UnregisterInterrupt(myself, 0, 0);
        _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to unregister handler: Error=%lu", L4_ErrorCode());
    }

    L4_LoadMR(0, 0);
    L4_Call(main_tid);
    while (1) ;
}

/*
\begin{test}{IRQ0201}
  \TestDescription{Check an unprivileged thread with permission granted can call InterruptControl}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Create a new address space.
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 to new address space.
    \item Create a thread in the new address space that calls \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Check that the return value is 1.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0201)
{
    L4_Word_t result;
    L4_SpaceId_t space;
    L4_ThreadId_t thread;

    space = createSpace();
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(space);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    thread = createThreadInSpace(space, (void (*)(void))registering_thread);
    L4_Set_UserDefinedHandleOf(thread, 0);
    L4_Receive(thread);

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(space);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
    deleteThread(thread);
    deleteSpace(space);
}
END_TEST

/*
\begin{test}{IRQ0202}
  \TestDescription{Check a space can not be granted access to an invalid IRQ}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to invalid interrupt number.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{EINVALID\_PARAM}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0202)
{
    L4_Word_t result;

    L4_LoadMR(0, INVALID_IRQ);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 0, "SecurityControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, "Wrong error code");
}
END_TEST

/*
\begin{test}{IRQ0300}
  \TestDescription{Check a thread with permission granted to a specific interrupt can not register to another interrupt}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl}  to grant access to interrupt number 1.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 2.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{ENO\_PRIVILEGE}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0300)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    L4_LoadMR(0, VALID_IRQ2);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    fail_unless(result == 0, "InterruptControl did not fail");
    _fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __FILE__, __LINE__, "Wrong error code: %lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
}
END_TEST

/*
\begin{test}{IRQ0301}
  \TestDescription{Check no two threads with permission granted can register to the same interrupt}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Create a new thread that calls \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{EINVALID\_PARAMETER}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0301)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;
    L4_ThreadId_t thread;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to register handler: Error=%lu", L4_ErrorCode());

    thread = createThread((void (*)(void))registering_thread);
    L4_Set_UserDefinedHandleOf(thread, 1);
    L4_Receive(thread);

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_UnregisterInterrupt(main_tid, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to unregister handler: Error=%lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");

    deleteThread(thread);
}
END_TEST

/*
\begin{test}{IRQ0400}
  \TestDescription{Check a space can not be granted access to an IRQ already granted to another space}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 for rootspace.
    \item Create a new space.
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 for the new space.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{EINVALID\_PARAM}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0400)
{
    L4_Word_t result;
    L4_SpaceId_t space;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    space = createSpace();
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(space);
    fail_unless(result == 0, "SecurityControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, "Wrong error code");
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
    deleteSpace(space);
}
END_TEST

/*
\begin{test}{IRQ0500}
  \TestDescription{Check grant and revoke permission actually does its job}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 and 2.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 1 and 2.
    \item Check that the return value is 1.
    \item Call \Func{L4\_InterruptControl} to unregister interrupt number 1 and 2.
    \item Call \Func{L4\_SecurityControl} to revoke access to interrupt number 1.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 2.
    \item Check that the return value is 1.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 1.
    \item Check that the return value is 0 and that the error code is EINVALID\_PARAM.
    \item Call \Func{L4\_InterruptControl} to unregister interrupt number 2.
    \item Call \Func{L4\_SecurityControl} to revoke access to interrupt number 2.
    \item Call \Func{L4\_InterruptControl} to register interrupt number 2.
    \item Check that the return value is 0 and that the error code is \Func{EINVALID\_PARAM}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0500)
{
    L4_Word_t result;
    L4_Word_t notifybits = 0x1;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    L4_LoadMR(0, VALID_IRQ2);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");
    L4_LoadMR(0, VALID_IRQ1);
    L4_LoadMR(1, VALID_IRQ2);
    result = L4_RegisterInterrupt(main_tid, notifybits, 1, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to register handler: Error=%lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    L4_LoadMR(1, VALID_IRQ2);
    result = L4_UnregisterInterrupt(main_tid, 1, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to unregister handler: Error=%lu", L4_ErrorCode());

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
    L4_LoadMR(0, VALID_IRQ2);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to register handler: Error=%lu", L4_ErrorCode());
    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    fail_unless(result == 0, "InterruptControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
    L4_LoadMR(0, VALID_IRQ2);
    result = L4_UnregisterInterrupt(main_tid, 0, 0);
    _fail_unless(result == 1, __FILE__, __LINE__, "InterruptControl failed to unregister handler: Error=%lu", L4_ErrorCode());

    L4_LoadMR(0, VALID_IRQ2);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
    L4_LoadMR(0, VALID_IRQ2);
    result = L4_RegisterInterrupt(main_tid, notifybits, 0, 0);
    fail_unless(result == 0, "InterruptControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
}
END_TEST

static void
unpriv_thread(void)
{
    L4_Word_t result;

    L4_LoadMR(0, VALID_IRQ2);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 0, "SecurityControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 0, "SecurityControl did not fail");
    fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, "Wrong error code");
}
/*
\begin{test}{IRQ0600}
  \TestDescription{Check only root space can grant and revoke access to interrupts}
  \TestPostConditions{}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 for rootspace.
    \item Create a new space.
    \item Call \Func{L4\_SecurityControl} to grant access to interrupt number 1 for the new space.
    \item Check that the return value is 0.
    \item Check that the error code is \Func{EINVALID\_PARAM}.
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IRQ0600)
{
    L4_Word_t result;
    L4_SpaceId_t space;
    L4_ThreadId_t thread;

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_AllowInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to grant access");

    space = createSpace();
    thread = createThreadInSpace(space, unpriv_thread);

    L4_LoadMR(0, VALID_IRQ1);
    result = L4_RestrictInterruptControl(L4_rootspace);
    fail_unless(result == 1, "SecurityControl failed to revoke access");
    deleteThread(thread);
    deleteSpace(space);
}
END_TEST
#endif

static void test_setup(void)
{
    main_tid = test_tid;
    initThreads(0);
}

static void test_teardown(void)
{
}

TCase *
make_interrupt_control_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Interrupt Control Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

#if defined(RUN_TESTS)
    tcase_add_test(tc, IRQ0100);
    tcase_add_test(tc, IRQ0200);
    tcase_add_test(tc, IRQ0201);
    tcase_add_test(tc, IRQ0202);
    tcase_add_test(tc, IRQ0300);
    tcase_add_test(tc, IRQ0301);
    tcase_add_test(tc, IRQ0400);
    tcase_add_test(tc, IRQ0500);
    tcase_add_test(tc, IRQ0600);
#endif

    return tc;
}
