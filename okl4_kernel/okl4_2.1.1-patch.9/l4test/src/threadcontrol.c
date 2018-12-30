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

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <stddef.h>
#include <l4/ipc.h>
#include <l4/config.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static L4_ThreadId_t main_thread;

#ifdef ARCH_IA32
/*
 * This magic number is here to skip the checking of the whole utcb,
 * as the beginning of the utcb is changed to pass arguments to the syscall.
 */
#define __L4_TCR_SYSCALL_ARGS_OFFSET ((64+(__L4_TCR_SYSCALL_ARGS+3))*4) 
#endif

/*
\begin{test}{tcr0300}
  \TestDescription{Verify that user defined handles work correctly}
  \TestFunctionalityTested{\Func{UserDefinedHandle}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Make a copy of the UTCB and user defined handle
      \item Assign values to the handle using \Func{Set\_UserDefinedHandle}
      \item Read back values using \Func{UserDefinedHandle} and check they are the same
      \item Restore the original handle and verify that the rest of the UTCB is unchanged
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tcr0300)
{
    L4_Word_t utcb_size = L4_GetUtcbSize();
    char *utcb_copy;
    L4_Word_t* user_handle = &(get_arch_utcb_base ())[ __L4_TCR_USER_DEFINED_HANDLE];
    L4_Word_t orig = L4_UserDefinedHandle ();
    L4_Word_t new;

    utcb_copy = malloc(utcb_size);

    /* Copy the UTCB for later comparison. */
    memcpy (utcb_copy, L4_GetUtcbBase (), utcb_size);

    /*
     * Assign some random values as the user handle and see if
     * they can be read back.
     */
    new = 0;
    L4_Set_UserDefinedHandle (new);
    fail_unless (*user_handle == new, "User Handle not set.");
    fail_unless (new == L4_UserDefinedHandle (),
                 "Set handle not fetched back.");
    new = -1;
    L4_Set_UserDefinedHandle (new);
    fail_unless (*user_handle == new, "User Handle not set.");
    fail_unless (new == L4_UserDefinedHandle (),
                 "Set handle not fetched back.");

    L4_Set_UserDefinedHandle (orig);

    /* UTCB should be unchanged. */
    fail_unless(memcmp (utcb_copy, L4_GetUtcbBase (), utcb_size) == 0,
                "UTCB changed during call.");
}
END_TEST

/*
\begin{test}{tcr0400}
  \TestDescription{Verify that a thread's pager can only be set to sane values}
  \TestFunctionalityTested{\Func{Set\_PagerOf}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Make a copy of the pager and UTCB
      \item Try setting the pager to nilthread and check that it fails
      \item Try setting the pager to an invalid thread and check that it fails
      \item Restore the original pager and check that the rest of the UTCB is unchanged
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tcr0400)
{
    /* Let's hope that everything we need has been mapped already! */

    L4_Word_t utcb_size = L4_GetUtcbSize();
#ifdef ARCH_IA32
    utcb_size -= __L4_TCR_SYSCALL_ARGS_OFFSET;
#endif
    char *utcb_copy;
    L4_ThreadId_t orig = L4_Pager ();
    L4_ThreadId_t new;

    utcb_copy = malloc(utcb_size);

    /* Copy the UTCB for later comparison. */
#ifdef ARCH_IA32
    memcpy(utcb_copy, (L4_GetUtcbBase()+__L4_TCR_SYSCALL_ARGS_OFFSET), utcb_size);
#else
    memcpy(utcb_copy, L4_GetUtcbBase(), utcb_size);
#endif

    /*
     * Assign some random values as the pager and see if they
     * change the pager.
     *
     * These tests can't use fail_unless, because the default
     * pager needs to be restored before calling _fail_unless().
     */
    new = L4_nilthread;
    L4_Set_PagerOf(main_thread, new);
    if (L4_IsThreadEqual (new, L4_Pager ())) {
        L4_Set_PagerOf (L4_Myself (), orig);
        fail ("Set pager to nilthread.");
    }

    new = L4_GlobalId (255, 255);
    L4_Set_PagerOf (main_thread, new);
    if (L4_IsThreadEqual (new, L4_Pager ())) {
        L4_Set_PagerOf (main_thread, orig);
        fail ("Set pager set to invalid.");
    }

    /* Fixup pager */
    L4_Set_PagerOf (main_thread, orig);

    /* UTCB should be unchanged. */
#ifdef ARCH_IA32
    fail_unless(memcmp(utcb_copy, L4_GetUtcbBase()+__L4_TCR_SYSCALL_ARGS_OFFSET,
                                  utcb_size) == 0, "UTCB changed during call.");
#else
    fail_unless(memcmp(utcb_copy, L4_GetUtcbBase(), utcb_size) == 0,
                                            "UTCB changed during call.");
#endif
}
END_TEST

static void unpriv_thread(void)
{
    /* Let's hope that everything we need has been mapped already! */
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    L4_Word_t utcb_size = L4_GetUtcbSize();
#ifdef ARCH_IA32
    utcb_size -= __L4_TCR_SYSCALL_ARGS_OFFSET;
#endif
    char *utcb_copy;
    L4_ThreadId_t orig = L4_Pager ();
    L4_ThreadId_t new;

    utcb_copy = malloc(utcb_size);

    CLEAR_ERROR_CODE;
    /* Copy the UTCB for later comparison. */
#ifdef ARCH_IA32
    memcpy(utcb_copy, (L4_GetUtcbBase()+__L4_TCR_SYSCALL_ARGS_OFFSET), utcb_size);
#else
    memcpy(utcb_copy, L4_GetUtcbBase(), utcb_size);
#endif

    /*
     * Assign some random values as the pager and see if they
     * change the pager.
     *
     * These tests can't use fail_unless, because the default
     * pager needs to be restored before calling _fail_unless().
     */
    new = L4_nilthread;
    L4_Set_PagerOf (main_thread, new);
    if (L4_IsThreadEqual (new, L4_Pager ())) {
        L4_Set_PagerOf (main_thread, orig);
        fail ("Set pager to nilthread.");
    }

    new = L4_GlobalId (255, 255);
    L4_Set_PagerOf (main_thread, new);
    if (L4_IsThreadEqual (new, L4_Pager ())) {
        L4_Set_PagerOf (main_thread, orig);
        fail ("Set pager when unpriv");
    }

    new = L4_GlobalId (1024, 1024);
    L4_Set_PagerOf (main_thread, new);
    if (L4_IsThreadEqual (new, L4_Pager ())) {
        L4_Set_PagerOf (main_thread, orig);
        fail ("Set pager when unpriv");
    }
    CLEAR_ERROR_CODE;

    /* UTCB should be unchanged. */
#ifdef ARCH_IA32
    fail_unless(memcmp(utcb_copy, L4_GetUtcbBase()+__L4_TCR_SYSCALL_ARGS_OFFSET,
                                  utcb_size) == 0, "UTCB changed during call.");
#else
    fail_unless(memcmp(utcb_copy, L4_GetUtcbBase(), utcb_size) == 0,
                                            "UTCB changed during call.");
#endif

    //send and receive both block
    tag.raw = TAG_SRBLOCK;

    while(1){
        L4_Ipc(main_thread, L4_nilthread, tag, &dummy);
    }
}

/*
\begin{test}{tcr0500}
  \TestDescription{Verify that an unprivileged thread cannot another thread's pager}
  \TestFunctionalityTested{\Func{Set\_PagerOf}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new thread (unpriv\_thread) in a new, unprivileged address space
      \item Wait for an IPC message from unpriv\_thread
      \item unpriv\_thread makes a copy of its pager and UTCB
      \item unpriv\_thread tries to set the main thread's pager to nilthread and checks that it fails
      \item unpriv\_thread tries to set the main thread's pager to an invalid value and checks that it fails
      \item unpriv\_thread tries to set the main thread's pager to a valid value and checks that it fails      
      \item unpriv\_thread sends an IPC message to the main thread
      \item Receive the IPC and delete unpriv\_thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(tcr0500)
{
    L4_ThreadId_t unpriv;
    L4_MsgTag_t tag;

    initThreads(0);

    unpriv = createThreadInSpace(L4_nilspace, unpriv_thread);

    L4_Set_MsgTag(L4_Niltag);
    tag = L4_Receive(unpriv);
    fail_unless(L4_IpcSucceeded (tag), "ipc failed");
    deleteThread(unpriv);
}
END_TEST

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}

static void test_teardown(void)
{
}

TCase *
make_threadcontrol_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Thread Control Register Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    /* Not implemmented pending SMP hardware. */
    tcase_add_test(tc, tcr0300);
    tcase_add_test(tc, tcr0400);
    tcase_add_test(tc, tcr0500);

    return tc;
}
