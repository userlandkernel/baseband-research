/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
#include <assert.h>
#include <stddef.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <l4test/arch/constants.h>

L4_Word_t unmapped_address = (L4_Word_t)0x32103210;

extern L4_ThreadId_t test_tid;

#define COPYING_THREAD_BUFSIZE 0x8000
#define SMALL_BUFFER_SIZE 32
#define COPYING_THREAD_PATTERN 0x10

static char copying_thread_buffer[COPYING_THREAD_BUFSIZE];

#define PATTERN_0100    0x11
#define PATTERN_0200    0x12
#define PATTERN_0300    0x13
#define PATTERN_0400    0x14
#define PATTERN_0500    0x15

static void
run_copying_thread(L4_Word_t direction, int small_remote_buffer, int unmapped)
{
    L4_MsgTag_t tag;

    memset(copying_thread_buffer, COPYING_THREAD_PATTERN, 
        sizeof(copying_thread_buffer));

    tag = L4_Niltag;
    L4_Set_MemoryCopy(&tag);
    L4_LoadMR(0, tag.raw);
    if (unmapped) {
        L4_LoadMR(1, (word_t)unmapped_address);
    } else {
        L4_LoadMR(1, (word_t)copying_thread_buffer);
    }
    if (small_remote_buffer) {
        L4_LoadMR(2, SMALL_BUFFER_SIZE);
    } else {
        L4_LoadMR(2, sizeof(copying_thread_buffer));
    }
    L4_LoadMR(3, direction);

    tag = L4_Call(test_tid);
    assert(L4_IpcSucceeded(tag));
}

static void
copying_thread_unmapped(void)
{
    run_copying_thread(L4_MemoryCopyBoth, 1, 1);
}

static void
copying_thread_small_remote_buf(void)
{
    run_copying_thread(L4_MemoryCopyBoth, 1, 0);
}

static void
copying_thread(void)
{
    run_copying_thread(L4_MemoryCopyBoth, 0, 0);
}

static void
copying_thread_from(void)
{
    run_copying_thread(L4_MemoryCopyFrom, 0, 0);
}

static void
copying_thread_to(void)
{
    run_copying_thread(L4_MemoryCopyTo, 0, 0);
}


static L4_ThreadId_t
create_thread(int newspace, void (*func)(void))
{
    if (newspace) {
        return createThreadInSpace(L4_nilspace, func);
    } else {
        return createThread(func);
    }
}

struct testing_data {
    word_t newspace[2];
    word_t dir[2];
    int ndirs;
    word_t local_size;
    int small_remote_buffer;
    int local_use_unmapped;
    int remote_use_unmapped;
};

/*
 * remote_memcpy_test_simple
 *
 * MemoryCopy() test, simple version
 */
static void 
remote_memcpy_test_simple(struct testing_data *data, int pattern)
{
    int rval, i, j, k;
    L4_ThreadId_t partner, tid;
    char *buffer;
    int orig_pattern = pattern;
    word_t size, orig_size;
    L4_MsgTag_t tag;
    int diff;

    for (i = 0; i < sizeof(data->newspace) / sizeof(data->newspace[0]); i++) {
        for (j = 0; j < data->ndirs; j++) {

            if (data->small_remote_buffer) {
                tid = partner = create_thread(data->newspace[i], 
                    copying_thread_small_remote_buf);
            } else if (data->remote_use_unmapped) {
                tid = partner = create_thread(data->newspace[i], 
                    copying_thread_unmapped);
            } else {
                tid = partner = create_thread(data->newspace[i], 
                    copying_thread);
            }
 
            tag = L4_Wait(&partner);
            assert(L4_IpcSucceeded(tag));

            pattern = orig_pattern;
            size = orig_size = data->local_size;
            if (!data->local_use_unmapped) {
                buffer = malloc(size);
                memset(buffer, pattern, size);
            } else {
                buffer = (void *)unmapped_address;
            }

            rval = L4_MemoryCopy(partner, (word_t)buffer, &size, data->dir[j]);
            if (rval == 0 && L4_ErrorCode() == L4_ErrNotImplemented) {
                goto out;
            }

            switch (pattern) {
                case PATTERN_0100:
                case PATTERN_0300:
                case PATTERN_0400:
                    if (pattern != PATTERN_0400) {
                        fail_unless(rval == 1, 
                            "L4_MemoryCopy did not return successfully");
                        fail_unless(size == orig_size, 
                            "L4_MemoryCopy did not return correct size");
                    } else {
                        fail_unless(rval == 0,
                            "L4_MemoryCopy did not fail");
                        fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege,
                            "Errorcode not NoPrivilege");
                        fail_unless(size == SMALL_BUFFER_SIZE,
                            "L4_MemoryCopy did not return correct size");
                    }
                    diff = 0;
	            if (data->dir[j] == L4_MemoryCopyTo) {
                        /*
                         * from remote buffer to local buffer.  
                         * Local buffer should
                         * now have contents of copying thread buffer.
                         */
                        pattern = COPYING_THREAD_PATTERN;
                    }

                    for (k = 0; k < size; k++) {
                        if (copying_thread_buffer[k] != buffer[k] || 
                            buffer[k] != pattern) {
                            diff = 1;
                            break;
                        }
                    }
                    fail_unless(diff == 0, 
                        "Remote buffer and local buffer mismatch");
                    break;
                case PATTERN_0500:
                    fail_unless(rval == 0,
                        "L4_MemoryCopy did not fail");
                    fail_unless(L4_ErrorCode() == L4_ErrNoMem,
                        "Errorcode not NoMem");
                    fail_unless(size == 0, 
                        "L4_MemoryCopy did not return zero size copy");
                    break;
                default:
                    break;
            }
out:
            deleteThread(tid);
            if (!data->local_use_unmapped) {
                free(buffer);
            }
        }
    }
}

/*
 * remote_memcpy_test_bad_dir
 *
 * Test for invalid copying direction
 */
static void
remote_memcpy_test_bad_dir(struct testing_data *data)
{
    int rval, i, j;
    L4_ThreadId_t partner, tid;
    char *buffer;
    int pattern;
    word_t size;
    L4_MsgTag_t tag;

    for (i = 0; i < sizeof(data->newspace) / sizeof(data->newspace[0]); i++) {
        for (j = 0; j < data->ndirs; j++) {
            if (data->dir[j] == L4_MemoryCopyFrom) {
                tid = partner = create_thread(data->newspace[i], 
                    copying_thread_from);
            } else {
                tid = partner = create_thread(data->newspace[i], 
                    copying_thread_to);
            }
            tag = L4_Wait(&partner);
            assert(L4_IpcSucceeded(tag));

            size = data->local_size;
            pattern = PATTERN_0200;
            buffer = malloc(size);
            memset(buffer, pattern, size);

            rval = L4_MemoryCopy(partner, (word_t)buffer, &size, data->dir[j]);
            if (rval == 0 && L4_ErrorCode() == L4_ErrNotImplemented) {
                goto out;
            }
            fail_unless(rval == 0, "L4_MemoryCopy did not fail");
            fail_unless(L4_ErrorCode() == L4_ErrNoPrivilege,
                "error not L4_ErrNoPrivilege");
            fail_unless(size == 0, "Number of bytes copied not zero");
out:
            deleteThread(tid);
            free(buffer);
        }
    }
}

/*
\begin{test}{RMC0100}
  \TestDescription{Perform a remote copy and test whether the remote memory copy succeeded}
er}
  \TestFunctionalityTested{MemoryCopy() system call}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread
      \item New thread sets up memory buffer descriptor and prepares 
            for MemoryCopy
      \item Perform a MemoryCopy() system call from local buffer to buffer 
            specified by memory buffer descriptor
      \item Check that the source and destination data buffer matches
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(RMC0100)
{
    struct testing_data testing_data = {
        { 0, 1 },
        { L4_MemoryCopyFrom, L4_MemoryCopyTo },
        2,
        32
    };

    remote_memcpy_test_simple(&testing_data, PATTERN_0100);
}
END_TEST

/*
\begin{test}{RMC0200}
  \TestDescription{Perform a MemoryCopy() on an invalid direction and 
         check that the copy failed}
  \TestFunctionalityTested{MemoryControl() system call}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread
      \item New thread sets up memory buffer descriptor and prepares for 
            MemoryCopy, setting specifying a specific direction
      \item Perform a MemoryCopy() system call using the same direction 
            specified by the remote thread
      \item Check the copy failed with NoPrivilege
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(RMC0200)
{
    struct testing_data testing_data = {
        { 0, 1 },
        { L4_MemoryCopyFrom, L4_MemoryCopyTo },
        2,
        32
    };

    remote_memcpy_test_bad_dir(&testing_data);
}
END_TEST

/*
\begin{test}{RMC0300}
  \TestDescription{Try to overflow the kernel bounce buffer for the 
                   memory copy operation}
er}
  \TestFunctionalityTested{MemoryCopy() system call}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread in a new address space
      \item New thread sets up memory buffer descriptor and prepares 
            for MemoryCopy
      \item Perform a MemoryCopy() system call with a large size to 
           try and overflow the kernel bounce buffer
      \item Check that the source and destination data buffer matches and the 
            full size is copied
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(RMC0300)
{
    struct testing_data testing_data = {
        { 0, 1 },
        { L4_MemoryCopyFrom, L4_MemoryCopyTo },
        2,
        0x4000
    };

    remote_memcpy_test_simple(&testing_data, PATTERN_0300);
}
END_TEST

/*
\begin{test}{RMC0400}
  \TestDescription{Try to overflow the destination buffer, check to 
                   make sure that it does not happen}
er}
  \TestFunctionalityTested{MemoryCopy() system call}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread 
      \item New thread sets up memory buffer descriptor and prepares for 
            MemoryCopy
      \item Perform a MemoryCopy() system call and try to overflow the
            remote buffer
      \item Check that a NoPrivilege error is returned, with the copied size 
            equal to the size of the destination buffer
    \end{enumerate}
  } 
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(RMC0400)
{
    struct testing_data testing_data = {
        { 0, 1 },
        { L4_MemoryCopyFrom },
        1,
        0x4000,
        1
    };

    remote_memcpy_test_simple(&testing_data, PATTERN_0400);
}
END_TEST

/*
\begin{test}{RMC0500}
  \TestDescription{Try to do a memory copy operation on unmapped memory, 
      check make sure it fails}
  \TestFunctionalityTested{MemoryControl() system call}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread
      \item New thread sets up memory buffer descriptor and prepares for 
            MemoryCopy
      \item Perform a MemoryCopy() system call on unmapped memory
      \item Check that an error of NoMem is returned
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(RMC0500)
{
    struct testing_data testing_data = {
        { 0, 1 },
        { L4_MemoryCopyFrom, L4_MemoryCopyTo },
        2,
        32,
        0,
        1,
        0
    };

    remote_memcpy_test_simple(&testing_data, PATTERN_0500);

    testing_data.local_use_unmapped = 0;
    testing_data.remote_use_unmapped = 1;

    remote_memcpy_test_simple(&testing_data, PATTERN_0500);
}
END_TEST

static void
test_setup(void)
{
    initThreads(1);
}

static void
test_teardown(void)
{
    initThreads(1);
}

TCase *
make_remote_memcpy_tcase(void)
{
    TCase *tc;

    tc = tcase_create("Remote Memory Copy");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, RMC0100);
    tcase_add_test(tc, RMC0200);
    tcase_add_test(tc, RMC0300);
    tcase_add_test(tc, RMC0400);
    tcase_add_test(tc, RMC0500);

    return tc;
}
