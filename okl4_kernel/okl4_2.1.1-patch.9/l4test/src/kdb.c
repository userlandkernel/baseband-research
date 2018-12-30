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
#include <stddef.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/mutex.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Length of the string to generate to test for buffer overflow
 * problems. */
#define LONG_STRING_TEST_SIZE 4096

/* Generate a long NUL-terminated string. The return pointer must be free()'d
 * by the caller. */
static char *
generate_long_string(void)
{
    int i;

    /* Allocate memory for the string. */
    char *buff = (char *)malloc(LONG_STRING_TEST_SIZE);
    fail_unless(buff != NULL, "Could not allocate memory for test string.");

    /* Generate the string. */
    for (i = 0; i < LONG_STRING_TEST_SIZE - 1; i++) {
        buff[i] = 'x';
    }
    buff[LONG_STRING_TEST_SIZE - 1] = '\0';
    return buff;
}

/*
\begin{test}{KDB0100}
  \TestDescription{Test L4\_KDB\_SetThreadName()}
  \TestFunctionalityTested{\Func{L4\_KDB\_SetThreadName()}}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(KDB0100)
{
    L4_ThreadId_t dummy = createThread(NULL);
    char *long_string = generate_long_string();

    L4_KDB_SetThreadName(dummy, "dummy");
    L4_KDB_SetThreadName(dummy, "");
    L4_KDB_SetThreadName(dummy, long_string);

    free(long_string);
    deleteThread(dummy);
}
END_TEST

/*
\begin{test}{KDB0200}
  \TestDescription{Test L4\_KDB\_SetSpaceName()}
  \TestFunctionalityTested{\Func{L4\_KDB\_SetSpaceName()}}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(KDB0200)
{
    L4_SpaceId_t dummy = createSpace();
    char *long_string = generate_long_string();

    L4_KDB_SetSpaceName(dummy, "dummy");
    L4_KDB_SetSpaceName(dummy, "");
    L4_KDB_SetSpaceName(dummy, long_string);

    free(long_string);
    deleteSpace(dummy);
}
END_TEST

/*
\begin{test}{KDB0300}
  \TestDescription{Test L4\_KDB\_SetMutexName()}
  \TestFunctionalityTested{\Func{L4\_KDB\_SetMutexName()}}
  \TestImplementationStatus{Implemented}
  \TestIsFullyAutomated{Yes}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(KDB0300)
{
    L4_MutexId_t mutex = L4_MutexId(1);
    char *long_string = generate_long_string();

    L4_CreateMutex(mutex);
    L4_KDB_SetMutexName(mutex, "mutex");
    L4_KDB_SetMutexName(mutex, "");
    L4_KDB_SetMutexName(mutex, long_string);

    free(long_string);
    L4_DeleteMutex(mutex);
}
END_TEST

static void test_setup(void)
{
}

static void test_teardown(void)
{
}

TCase *
make_kdb_tcase(void)
{
    TCase *tc;

    tc = tcase_create("Kernel Debugger");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, KDB0100);
    tcase_add_test(tc, KDB0200);
    tcase_add_test(tc, KDB0300);

    return tc;
}
