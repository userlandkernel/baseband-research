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
#include <l4test/arch/constants.h>
#include <stddef.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/cache.h>
#include <l4/misc.h>
//#include <bootinfo/bootinfo.h> FIXME
#include <l4/space.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


static L4_ThreadId_t NONEXISTENT_TID;
static L4_SpaceId_t NONEXISTENT_SPACE;
static L4_SpaceId_t BAD_SPACE;

static L4_ThreadId_t main_thread;

extern L4_Word_t kernel_max_spaces;


/*
\begin{test}{FUZZ0200}
  \TestDescription{Cache Control, as the root task}
  \TestFunctionalityTested{\Func{CacheControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{CacheControl} with an invalid operation.
    \item Check that the function returns an error with the error code  
          specified as \Macro{ErrInvalidParam}
    \item Call \Func{CacheControl} with too many MRs specified
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \item Call \Func{CacheControl} with an operation to invalidate the
          D-Cache
    \item Check that the operation succeeds; the roottask should have
          permission to perform this operation
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0200)
{

    /* A block of memory we have capability to for the cache invalidate */
    static L4_Word_t array[32];
    int r;

    /* Invalid Op. */
    r = L4_CacheControl(L4_rootspace, 0);
    fail_unless(r != 1, "Invalid Op: Unexpected success!");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, 
                 __FILE__, __LINE__, 
                "Invalid Op: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidParam);

    /* Too many MRs specified. */
    r = L4_CacheControl(L4_rootspace,
                        L4_CacheCtl_FlushRanges | 
                        L4_CacheCtl_MaskAllLs | 63);
    fail_unless(r != 1, "Too many MRs: Unexpected success!");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, 
                 __FILE__, __LINE__, 
                "Too many MRs: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidParam);

    /* Flushing D-Cache: must be privileged. */
    r = L4_CacheFlushRangeInvalidate(L4_rootspace, 
                                     (L4_Word_t)&(array[0]), 
                                     (L4_Word_t)&(array[31]));
    _fail_unless(r == 1,
                 __FILE__, __LINE__, 
                 "D-Cache flush, unexpected failure, Error Code: %d\n",
                 L4_ErrorCode());
}
END_TEST


/*
\begin{test}{FUZZ0201}
  \TestDescription{Cache Control, as the root task--More Tests}
  \TestFunctionalityTested{\Func{CacheControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{CacheControl} with an non-existent TID
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Fails}
  \TestRegressionStatus{NOT in regression test suite}
  \TestInternalNotes{Not run as part of automated testing}
\end{test}
*/
START_TEST(FUZZ0201)
{
    /* An invalid TID.  The space is never used so this always
     * succeeds!
     */
    /* FIXME: This test has been disabled because it is a known bug that needs
     * to be scheduled to be fixed.  CacheControl does not check the thread id
     * parameter.  It is easy to check but there are many things which pass
     * garbage in that parameter and would break if the check is implemented.
     */
    fail_unless(L4_CacheFlushRangeInvalidate(NONEXISTENT_SPACE, 0, 1) == 0,
                "Nonexistent TID: Unexpected success.");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, __FILE__, __LINE__, 
                 "Nonexistent TID: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidParam);

}
END_TEST

static void
bad_cache_control_params (void)
{
    /* Invalid Op. */
    int r;
    r = L4_CacheControl(L4_rootspace, 0);
    multithreaded_fail_unless(r != 1, 
                              __LINE__,
                              "Invalid Op: Unexpected success!");

    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, 
                              __LINE__,
                              "Invalid Op: unexpected error code "
                              "(was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrInvalidParam);

    /* Too many MRs specified. */
    r = L4_CacheControl(L4_rootspace,
                        L4_CacheCtl_FlushRanges | L4_CacheCtl_MaskAllLs | 63);
    multithreaded_fail_unless(r != 1,
                              __LINE__,
                              "Too many MRs: Unexpected success!");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrInvalidParam, 
                              __LINE__,
                              "Too many MRs: unexpected error code "
                              "(was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrInvalidParam);

    /* Flushing D-Cache: must be privileged. */
    r = L4_CacheFlushRangeInvalidate(L4_rootspace, 0, 1);
    multithreaded_fail_unless(r == 0,
                              __LINE__,
                              "D-Cache flush: Unexpected success!");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "D-Cache flush: unexpected error code "
                              "(was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_pass();
}

/*
\begin{test}{FUZZ0250}
  \TestDescription{Cache Control, as a normal thread}
  \TestFunctionalityTested{\Func{CacheControl}}
  \TestImplementationProcess{
    Perform the following tests in multiple threads, all running in
    different address spaces:

    \begin{enumerate}
    \item Call \Func{CacheControl} with an invalid operation
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \item Call \Func{CacheControl} with too many MRs specified
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \item Call \Func{CacheControl} with an operation to invalidate the
          D-Cache
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0250)
{
    initThreads(1);
    run_multithreaded_test(__FILE__, bad_cache_control_params);
}
END_TEST

static void
bad_thread_switch(void)
{
    L4_ThreadSwitch(NONEXISTENT_TID);
    L4_ThreadSwitch(L4_anythread);
    L4_ThreadSwitch(L4_anylocalthread);
    L4_ThreadSwitch(L4_waitnotify);

    multithreaded_pass();
}


/*
\begin{test}{FUZZ0300}
  \TestDescription{Thread Switch as the roottask}
  \TestFunctionalityTested{\Func{ThreadSwitch}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{ThreadSwitch} with a non-existent TID
    \item Call \Func{ThreadSwitch} with \Macro{L4\_anythread}
    \item Call \Func{ThreadSwitch} with \Macro{L4\_anylocalthread}
    \item Call \Func{ThreadSwitch} with \Macro{L4\_waitnotify}
    \end{enumerate}

    The above calls should not result in any misbehaviour from the kernel.
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0300)
{
    initThreads(0);
    run_multithreaded_test(__FILE__, bad_thread_switch);
}
END_TEST

/*
\begin{test}{FUZZ0350}
  \TestDescription{Thread Switch as a normal thread}
  \TestFunctionalityTested{\Func{ThreadSwitch}}
  \TestImplementationProcess{
    Perform the following tests in multiple threads, all running in
    different address spaces:

    \begin{enumerate}
    \item Call \Func{ThreadSwitch} with a non-existent TID
    \item Call \Func{ThreadSwitch} with \Macro{L4\_anythread}
    \item Call \Func{ThreadSwitch} with \Macro{L4\_anylocalthread}
    \item Call \Func{ThreadSwitch} with \Macro{L4\_waitnotify}
    \end{enumerate}

    The above calls should not result in any misbehaviour from the kernel
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0350)
{
    initThreads(1);
    run_multithreaded_test(__FILE__, bad_thread_switch);
}
END_TEST

/*
\begin{test}{FUZZ0400}
  \TestDescription{Map Control, as the root task}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{MapControl} with \Macro{L4\_nilthread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call \Func{MapControl} with \Macro{L4\_anythread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call \Func{MapControl} with \Macro{L4\_anylocalthread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call \Func{MapControl} with \Macro{L4\_waitnotify} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call \Func{MapControl} with a non-existent TID as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0400)
{
    fail_unless(L4_MapControl(L4_nilspace, 0) == 0,
                "nilthread: unexpected success.");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace,
                 __FILE__, __LINE__,
                 "nilthread: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidSpace);

    fail_unless(L4_MapControl(NONEXISTENT_SPACE, 0) == 0,
                "NONEXISTENT_SPACE: unexpected success.");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace,
                 __FILE__, __LINE__,
                 "NONEXISTENT_SPACE: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidSpace);
}
END_TEST

static void
bad_map_control(void)
{
    multithreaded_fail_unless(L4_MapControl(L4_nilspace, 0) == 0, __LINE__,
                              "nilthread: unexpected success.");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace, __LINE__,
                              "nilthread: unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrInvalidSpace);

    multithreaded_fail_unless(L4_MapControl(NONEXISTENT_SPACE, 0) == 0, __LINE__,
                              "NONEXISTENT_SPACE: unexpected success.");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace, __LINE__,
                              "NONEXISTENT_SPACE: unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrInvalidSpace);

    multithreaded_pass();
}

/*
\begin{test}{FUZZ0450}
  \TestDescription{Map Control, as a normal thread}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    Perform the following tests in multiple threads, all running in
    different address spaces:

    \begin{enumerate}
    \item Call \Func{MapControl} with \Macro{L4\_nilthread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{MapControl} with \Macro{L4\_anythread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{MapControl} with \Macro{L4\_anylocalthread} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{MapControl} with \Macro{L4\_waitnotify} as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{MapControl} with a non-existent TID as the space
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0450)
{
    initThreads(1);
    run_multithreaded_test(__FILE__, bad_map_control);
}
END_TEST

/*
\begin{test}{FUZZ0500}
  \TestDescription{FPage value checking}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4\_UnmapFpage} a fpage that is smaller than 4K
          and unaligned
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \item Call \Func{L4\_MapFpage} a fpage that is smaller than 4K
          and unaligned
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidParam}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Fails on gumstix}
  \TestInternalNotes{Not run as part of automated testing}
\end{test}
*/
START_TEST(FUZZ0500)
{
    /* Create a completely unaligned fpage. */
    L4_Fpage_t bad_fpage = L4_Fpage(1, 256);
    int r;

    L4_KDB_Enter("foo");
    r = L4_UnmapFpage(L4_rootspace, bad_fpage);
    L4_KDB_Enter("blah");
    fail_unless(r == 0, "Unmap Bad Fpage: unexpected success.");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace, __FILE__, __LINE__,
                "Unmap bad_fpage: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidSpace);
    L4_Set_Rights(&bad_fpage, L4_FullyAccessible);

    r = L4_MapFpage(L4_rootspace, bad_fpage, L4_PhysDesc(0, L4_DefaultMemory));

    fail_unless(r == 0, "Map Bad Fpage: unexpected success.");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace, __FILE__, __LINE__,
                "Map bad_fpage: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidSpace);
}
END_TEST

/*
\begin{test}{FUZZ0600}
  \TestDescription{Exchange Registers}
  \TestFunctionalityTested{\Func{ExchangeRegisters}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call the \Func{ExchangeRegisters} function to read the user defined
          handle of \Macro{L4\_nilthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to write the user defined
          handle of \Macro{L4\_nilthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to read the user defined
          handle of \Macro{L4\_anythread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to write the user defined
          handle of \Macro{L4\_anythread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to read the user defined
          handle of \Macro{L4\_anylocalthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to write the user defined
          handle of \Macro{L4\_anylocalthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to read the user defined
          handle of \Macro{L4\_waitnotify}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to write the user defined
          handle of \Macro{L4\_waitnotify}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to read the user defined
          handle of a non-existent TID
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \item Call the \Func{ExchangeRegisters} function to write the user defined
          handle of a non-existent TID
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0600)
{
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;
    L4_ThreadId_t result;

    result = L4_ExchangeRegisters(L4_nilthread, L4_ExReg_Deliver, 0, 0,
                                  0, 0, L4_nilthread, &dummy, &dummy, &dummy,
                                  &dummy, &handle, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "nilthread read value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "nilthread read value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_nilthread, L4_ExReg_user, 0, 0, 0,
                                  0xdeafbeef, L4_nilthread, &dummy, &dummy,
                                  &dummy, &dummy, &dummy, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "nilthread write value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "nilthread write value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_anythread, L4_ExReg_Deliver, 0, 0,
                                  0, 0, L4_nilthread, &dummy, &dummy, &dummy,
                                  &dummy, &handle, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "anythread read value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anythread read value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_anythread, L4_ExReg_user, 0, 0, 0,
                                  0xdeafbeef, L4_nilthread, &dummy, &dummy,
                                  &dummy, &dummy, &dummy, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "anythread write value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anythread write value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_anylocalthread, L4_ExReg_Deliver, 0, 0,
                                  0, 0, L4_nilthread, &dummy, &dummy, &dummy,
                                  &dummy, &handle, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "anylocalthread read value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anylocalthread read value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_anylocalthread, L4_ExReg_user, 0, 0, 0,
                                  0xdeafbeef, L4_nilthread, &dummy, &dummy,
                                  &dummy, &dummy, &dummy, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "anylocalthread write value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anylocalthread write value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(L4_waitnotify, L4_ExReg_Deliver, 0, 0,
                                  0, 0, L4_nilthread, &dummy, &dummy, &dummy,
                                  &dummy, &handle, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "waitnotify read value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "waitnotify read value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(NONEXISTENT_TID, L4_ExReg_user, 0, 0, 0,
                                  0xdeafbeef, L4_nilthread, &dummy, &dummy,
                                  &dummy, &dummy, &dummy, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "waitnotify write value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "waitnotify write value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(NONEXISTENT_TID, L4_ExReg_Deliver, 0, 0,
                                  0, 0, L4_nilthread, &dummy, &dummy, &dummy,
                                  &dummy, &handle, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "Bad TID read value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "Bad TID read value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    result = L4_ExchangeRegisters(NONEXISTENT_TID, L4_ExReg_user, 0, 0, 0,
                                  0xdeafbeef, L4_nilthread, &dummy, &dummy,
                                  &dummy, &dummy, &dummy, &dummy_id);
    fail_unless (L4_IsNilThread (result),
                 "Bad TID write value: Unexpected success");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "Bad TID write value: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);
}
END_TEST

/*
\begin{test}{FUZZ0700}
  \TestDescription{Thread Control as the roottask}
  \TestFunctionalityTested{\Func{ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \item Call \Func{ThreadControl} to create a thread with without 
          specifying a scheduler thread
    \item Call the \Func{ThreadControl} function to delete \Func{L4_Myself()}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \item Call the \Func{ThreadControl} function to delete a non-existent thread
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0700)
{
    fail_unless(L4_ThreadControl(main_thread, L4_nilspace, L4_nilthread, L4_nilthread,
                                 L4_nilthread, 0, (void *)0) == 0,
                "Delete Self: unexpected success (!?!).\n");

    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "Delete Self: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    fail_unless(L4_ThreadControl(NONEXISTENT_TID, L4_nilspace, L4_nilthread,
                                 L4_nilthread, L4_nilthread, 0, (void *)0) == 0,
                "Delete non-existent: unexpected success.\n");

    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "Delete non-existent: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);
}
END_TEST

/*
\begin{test}{FUZZ0701}
  \TestDescription{Thread Control as the roottask--More Tests}
  \TestFunctionalityTested{\Func{ThreadControl}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call the \Func{ThreadControl} function to create a thread with a ThreadID of
          \Macro{L4\_anylocalthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Fails testing anylocalthread}
\end{test}
*/
START_TEST(FUZZ0701)
{
    fail_unless(L4_ThreadControl(L4_anylocalthread, L4_rootspace, L4_Pager(), L4_nilthread,
                                 L4_nilthread, 0, (void *)0) == 0,
        "Create anylocalthread: unexpected success.\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "Create anylocalthread: Unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);
}
END_TEST

static void
bad_thread_control(void)
{
    multithreaded_fail_unless(L4_ThreadControl(L4_nilthread, L4_rootspace, 
                                               L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create nilthread: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create nilthread: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(L4_anythread, L4_rootspace, L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create anythread: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create anythread: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(L4_anylocalthread, L4_rootspace, L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create anylocalthread: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create anylocalthread: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(L4_waitnotify, L4_rootspace, L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create waitnotify: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create waitnotify: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(L4_waitnotify, L4_rootspace, L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create waitnotify: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create waitnotify: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(L4_GlobalId(1, 10), L4_rootspace, L4_Pager(), L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create bad interrupt: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create bad interrupt: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(NONEXISTENT_TID, L4_rootspace, L4_nilthread,
                                               L4_nilthread, L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Create no scheduler: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Create no scheduler: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(main_thread, L4_nilspace, L4_nilthread, L4_nilthread,
                                               L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Delete Self: unexpected success (!?!).\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Delete Self: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_ThreadControl(NONEXISTENT_TID, L4_nilspace, L4_nilthread,
                                               L4_nilthread, L4_nilthread, 0, (void *)0) == 0,
                              __LINE__,
                              "Delete non-existent: unexpected success.\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "Delete non-existent: Unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_pass();
}

/*
\begin{test}{FUZZ0750}
  \TestDescription{Thread Control as a normal thread}
  \TestFunctionalityTested{\Func{ThreadControl}}
  \TestImplementationProcess{
    Perform the following tests in multiple threads, all running in
    different address spaces:

    \begin{enumerate}
    \item Call the \Func{ThreadControl} function to create a thread with 
          a ThreadID of \Macro{L4\_nilthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to create a thread with 
          a ThreadID of \Macro{L4\_anythread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to create a thread with
          a ThreadID of \Macro{L4\_anylocalthread}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to create a thread with 
          a ThreadID of \Macro{L4\_waitnotify}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to create a thread with 
          a ThreadID in the interrupt TID range
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to create a thread without 
          specifying a scheduler thread
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} thread to delete \Func{L4_Myself()}
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call the \Func{ThreadControl} function to delete a non-existent thread
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \end{enumerate}}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0750)
{
    initThreads(1);
    run_multithreaded_test(__FILE__, bad_thread_control);
}
END_TEST

/*
\begin{test}{FUZZ0800}
  \TestDescription{Schedule}
  \TestFunctionalityTested{\Func{Schedule}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{Schedule} with \Macro{L4\_anythread} as the
          destination 
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \item Call \Func{Schedule} with \Macro{L4\_anylocalthread} as the
          destination
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \item Call \Func{Schedule} with \Macro{L4\_waitnotify} as the
          destination
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \item Call \Func{Schedule} with a non-existent TID as the
          destination
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0800)
{
    L4_Word_t dummy;

#if 0
    fail_unless(L4_Schedule(L4_nilthread, -1, -1, -1, -1, 0, NULL) == 0,
                "nilthread: Unexpected success\n");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                "nilthread: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidThread);

    /* Call space control with invalid spaces. */
    fail_unless(L4_Schedule(L4_nilthread, -1, -1, -1, -1, 0, &dummy) == 0,
                "nilthread: Unexpected success\n");
    fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                "nilthread: unexpected error code (was %d, expected %d)\n",
                L4_ErrorCode(), L4_ErrInvalidThread);

#endif

    fail_unless(L4_Schedule(L4_anythread, -1, -1, -1, -1, 0, &dummy) == 0,
                "anythread: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anythread: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    fail_unless(L4_Schedule(L4_anylocalthread, -1, -1, -1, -1, 0, &dummy) == 0,
                "anylocalthread: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "anylocalthread: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    fail_unless(L4_Schedule(L4_waitnotify, -1, -1, -1, -1, 0, &dummy) == 0,
                "waitnotify: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "waitnotify: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

    fail_unless(L4_Schedule(NONEXISTENT_TID, -1, -1, -1, -1, 0, &dummy) == 0,
                "NONEXISTENT_TID: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "NONEXISTENT_TID: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);

}
END_TEST

/*
\begin{test}{FUZZ0801}
  \TestDescription{Schedule--NULL Pointers}
  \TestFunctionalityTested{\Func{Schedule}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{Schedule} with \Macro{L4\_nilthread} as the
          destination and NULL pointers as the return value pointers
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Fails de-referencing a NULL pointer}
  \TestInternalNotes{Not run as part of automated testing}
\end{test}
*/
START_TEST(FUZZ0801)
{
    fail_unless(L4_Schedule(L4_nilthread, -1, -1, -1, -1, 0, NULL) == 0,
                "nilthread: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "nilthread: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);
}
END_TEST

/*
\begin{test}{FUZZ0802}
  \TestDescription{Schedule--More Tests}
  \TestFunctionalityTested{\Func{Schedule}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{Schedule} with \Macro{L4\_nilthread} as the
          destination
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidThread}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Fails with the wrong error code}
  \TestInternalNotes{Not run as part of automated testing}
\end{test}
*/
START_TEST(FUZZ0802)
{
    L4_Word_t dummy;

    /* Call space control with invalid spaces. */
    fail_unless(L4_Schedule(L4_nilthread, -1, -1, -1, -1, 0, &dummy) == 0,
                "nilthread: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidThread,
                 __FILE__, __LINE__,
                 "nilthread: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidThread);
}
END_TEST

/*
\begin{test}{FUZZ0900}
  \TestDescription{Space Control in the roottask}
  \TestFunctionalityTested{\Func{SpaceControl}}
  \TestImplementationProcess{
    \begin{enumerate}

    \item Call \Func{SpaceControl} with an invalid SpaceId as the
          space specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrInvalidSpace}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
  \TestNotes{Needs to be expanded to test more of the combinations of
  good and bad fpages}
\end{test}
*/
START_TEST(FUZZ0900)
{
    /* Create a completely unaligned fpage. */
    L4_Fpage_t bad_fpage = L4_Fpage(1, 256);
    L4_Word_t old_resources;
    int r;

    /* Call space control with invalid space id. */
    r = L4_SpaceControl(BAD_SPACE, L4_SpaceCtrl_new, L4_rootclist,
                        bad_fpage,
                        0, &old_resources);
    fail_unless(r == 0, "bad_spaceid: Unexpected success\n");
    _fail_unless(L4_ErrorCode() == L4_ErrInvalidSpace,
                 __FILE__, __LINE__,
                 "bad_spaceid: unexpected error code (was %d, expected %d)\n",
                 L4_ErrorCode(), L4_ErrInvalidSpace);
}
END_TEST

static void bad_space_control(void)
{
    /* Create a completely unaligned fpage. */
    L4_Fpage_t bad_fpage = L4_Fpage(1, 256);
    L4_Word_t old_resources;

    /* All tests should fail with permission problems. */
    multithreaded_fail_unless(L4_SpaceControl(BAD_SPACE, L4_SpaceCtrl_new,
                                              L4_rootclist,
                                              bad_fpage,
                                              0, &old_resources) == 0, __LINE__,

                              "bad_spaceid: Unexpected success\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "bad_spaceid: unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);

    multithreaded_fail_unless(L4_SpaceControl(NONEXISTENT_SPACE, L4_SpaceCtrl_new,
                                              L4_rootclist,
                                              bad_fpage,
                                              0, &old_resources) == 0, __LINE__,
                              "bad_kip: Unexpected success\n");
    multithreaded_fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege, __LINE__,
                              "bad_kip: unexpected error code (was %d, expected %d)\n",
                              L4_ErrorCode(), L4_ErrNoPrivilege);
    multithreaded_pass();
}

/*
\begin{test}{FUZZ0950}
  \TestDescription{Space Control in a normal thread}
  \TestFunctionalityTested{\Func{SpaceControl}}
  \TestImplementationProcess{
    Perform the following tests in multiple threads, all running in
    different address spaces:

    \begin{enumerate}
    \item Call \Func{Schedule} with \Macro{L4\_nilthread} as the
          space specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{Schedule} with \Macro{L4\_anythread} as the
          space specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{Schedule} with \Macro{L4\_anylocalthread} as the
          space specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{Schedule} with \Macro{L4\_waitnotify} as the
          space specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \item Call \Func{Schedule} with a non-existent TID as the space
          specifier
    \item Check that the function returns an error with the error code 
          specified as \Macro{ErrNoPrivilege}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ0950)
{
    initThreads(1);
    run_multithreaded_test(__FILE__, bad_space_control);
}
END_TEST

/*
\begin{test}{FUZZ1000}
  \TestDescription{IPC Calls}
  \TestFunctionalityTested{\Func{IPC}}
  \TestImplementationProcess{
    \begin{enumerate}
    \item Call \Func{L4_Send_Nonblocking} with a non-existent TID
    \item Check that the call reports an error
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestPassStatus{Passes on gumstix}
\end{test}
*/
START_TEST(FUZZ1000)
{
    L4_MsgTag_t tag;

    tag = L4_Send_Nonblocking(NONEXISTENT_TID);
    
    fail_unless(L4_IpcFailed(tag),
                "IPC, unexpected success.\n");
    
}
END_TEST

extern L4_ThreadId_t test_tid;

static void
test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;

    NONEXISTENT_TID = L4_GlobalId( 555, 1);
    NONEXISTENT_SPACE.raw = kernel_max_spaces - 1;
    BAD_SPACE.raw = kernel_max_spaces + 1;
}

static void
test_teardown(void)
{
}

TCase *
make_fuzz_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Fuzz Tests");
    tcase_add_unchecked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, FUZZ0200);
    /* Currently broken, but I don't see a bug lodged in Mothra */
    /*tcase_add_test(tc, FUZZ0201);*/
    (void) FUZZ0201;
    tcase_add_test(tc, FUZZ0250);
    tcase_add_test(tc, FUZZ0300);
    tcase_add_test(tc, FUZZ0350);
    tcase_add_test(tc, FUZZ0400);
    tcase_add_test(tc, FUZZ0450);
    /* Currently broken, but I don't see a bug lodged in Mothra */
    /*tcase_add_test(tc, FUZZ0500);*/
    (void) FUZZ0500;
    tcase_add_test(tc, FUZZ0600);
    tcase_add_test(tc, FUZZ0700);
    tcase_add_test(tc, FUZZ0701);
    tcase_add_test(tc, FUZZ0750);
    tcase_add_test(tc, FUZZ0800);
    tcase_add_test(tc, FUZZ0801);
    tcase_add_test(tc, FUZZ0802);
    tcase_add_test(tc, FUZZ0900);
    tcase_add_test(tc, FUZZ0950);
    tcase_add_test(tc, FUZZ1000);

    return tc;
}
