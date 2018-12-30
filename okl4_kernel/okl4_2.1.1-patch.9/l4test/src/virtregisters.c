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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <bootinfo/bootinfo.h>
#include <l4/config.h>

/* Globals */
static L4_ThreadId_t main_thread;


/*
\begin{test}{VIRT0100}
  \TestDescription{Range test for StoreMR}
  \TestFunctionalityTested{\Func{StoreMR}}
  \TestImplementationProcess{
  For the values of i from zero to MessageRegisters - 1:
    \begin{enumerate}
    \item Call \Func{StoreMR(i, xx)}   
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{xx is a random value}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0100)
{
    L4_Word_t MR_num;
    L4_Word_t i;
    L4_Word_t* mr_base = get_mr_base ();

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    /*
     * Write a known word directly into each register and see if
     * the API can read it back.
     */
    for (i = 0; i < MR_num; i++) {
        L4_Word_t w = 0;

        mr_base[i] = 0xdeadbeef + i;
        L4_StoreMR (i, &w);

        _fail_unless(w == 0xdeadbeef + i, __FILE__, __LINE__,
                    "MR[%d] written as 0x%lx, read as 0x%lx.",
                    (int) i, (long) 0xdeadbeef + i, (long) w);
    }
}
END_TEST

/*
\begin{test}{VIRT0200}
  \TestDescription{Range test for LoadMR}
  \TestFunctionalityTested{\Func{LoadMR}}
  \TestImplementationProcess{
  For the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
    \item Call \Func{LoadMR(i, xx)} 
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{xx is a random value}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0200)
{
    L4_Word_t MR_num;
    L4_Word_t i;
    L4_Word_t* mr_base = get_mr_base ();

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__, "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    /*
     * Write a known word into each register using the API and
     * then see if we can read it back directly.
     */
    for (i = 0; i < MR_num; i++) {
        L4_LoadMR (i, 0xdeadbeef + i);
        _fail_unless(mr_base[i] == 0xdeadbeef + i, __FILE__, __LINE__,
                    "MR[%d] written as 0x%lx, read as 0x%lx.",
                    (int) i, (long) 0xdeadbeef + i, (long) mr_base[i]);
    }
}
END_TEST

/*
\begin{test}{VIRT0300}
  \TestDescription{Range test for LoadMR and StoreMR}
  \TestFunctionalityTested{\Func{LoadMR} and \Func{StoreMR}}
  \TestImplementationProcess{
  For the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
    \item Call \Func{LoadMR(i, xx)}
	\item Call \Func{StoreMR(i, xx)}
	\item Check values including testing for under and overruns 
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{xx is a random value}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0300)
{
    L4_Word_t MR_num;
    L4_Word_t i;
    L4_Word_t* mr_base = get_mr_base ();

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    /*
     * Write a known word and read it back, both through the API
     * and then see if they're the same and that the value is
     * still in the register.
     */
    for (i = 0; i < MR_num; i++) {
        L4_Word_t value = 0xdeadbeef + i;
        L4_Word_t w = 0;

        L4_LoadMR (i, value);
        L4_StoreMR (i, &w);

        _fail_unless(w == value, __FILE__, __LINE__,
                    "MR[%d] written as 0x%lx, read as 0x%lx.",
                    (int) i, (long) value, (long) w);

        _fail_unless(mr_base[i] == 0xdeadbeef + i, __FILE__, __LINE__,
                    "MR[%d] written as 0x%lx, directly read as 0x%lx.",
                    (int) i, (long) 0xdeadbeef + i, (long) mr_base[i]);
    }
}
END_TEST

/*
 * This is the same test as VIRT0300 but it is run in a separate
 * thread.  As a consequence it cannot call fail_unless().  Instead
 * error messages are stored in a per-thread buffer a message is sent
 * back to the master thread to tell it whether is passed or failed.
 * The master thread then logs errors via fail().
 */
static void threaded_load_store_mr_test(void)
{
    L4_Word_t MR_num;
    L4_Word_t i;
    L4_Word_t* mr_base = get_mr_base ();

    MR_num = L4_GetMessageRegisters();

    multithreaded_fail_unless (MR_num == __L4_NUM_MRS, __LINE__,
                               "Unexpected number of MRs (expected %d, got %d).",
                               (int) __L4_NUM_MRS, (int) MR_num);

    /*
     * Write a known word and read it back, both through the API
     * and then see if they're the same and that the value is
     * still in the register.
     */
    for (i = 0; i < MR_num; i++) {
        L4_Word_t value = 0xdeadbeef + i;
        L4_Word_t w = 0;

        L4_LoadMR (i, value);
        L4_StoreMR (i, &w);

        multithreaded_fail_unless(w == value, __LINE__,
                                  "MR[%d] written as 0x%lx, read as 0x%lx.",
                                  (int) i, (long) value, (long) w);
        multithreaded_fail_unless(mr_base[i] == 0xdeadbeef + i, __LINE__,
                                  "MR[%d] written as 0x%lx, directly read as 0x%lx.",
                                  (int) i, (long) 0xdeadbeef + i, (long) mr_base[i]);
    }

    multithreaded_pass ();
}

/*
\begin{test}{VIRT0400}
  \TestDescription{Multi-threaded range test LoadMR and StoreMR}
  \TestFunctionalityTested{\Func{LoadMR} and \Func{StoreMR}}
  \TestImplementationProcess{
  Create multiple threads. For each thread, for the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
    \item Call \Func{LoadMR(i, xx)}
	\item Call \Func{StoreMR(i, xx)}
	\item Check values 
	\end{enumerate}  
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{xx is a random value}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0400)
{
    run_multithreaded_test (__FILE__, threaded_load_store_mr_test);
}
END_TEST

/*
\begin{test}{VIRT0500}
  \TestDescription{Range test for StoreMRs}
  \TestFunctionalityTested{\Func{StoreMRs}}
  \TestImplementationProcess{
  For varying sized arrays, for the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
	\item Call \Func{StoreMR(i, k, xx)}
	\item Check values  
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{
  \begin{enumerate}
  \item xx is a random value
  \item Size of array tested may be varied by altering k 
  \end{enumerate}
  }
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0500)
{
    L4_Word_t MR_num;
    L4_Word_t i, j, copy_size;
    L4_Word_t* mr_base = get_mr_base ();
    L4_Word_t contents[__L4_NUM_MRS];
    L4_Word_t buffer[__L4_NUM_MRS];

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    for (i = 0; i < MR_num; i++) {
        contents[i] = 0xdeadbeef + i;
        buffer[i] = 0;
    }

    /*
     * Fill the registers with known values.  These shouldn't
     * change for the life of the test
     */
    memcpy (mr_base, contents, sizeof(contents));

    /* Check that reading 0 words from any register copies nothing. */
    for (i = 0 ; i < MR_num; i++) {
        L4_StoreMRs (i, 0, buffer);

        for (j = 0; j < __L4_NUM_MRS; j++)
            _fail_unless (buffer[j] == 0, __FILE__, __LINE__,
                         "Zero copy but buffer[%d] contained 0x%lx.",
                         (int) j, (long) buffer[j]);
    }

    /*
     * For each register copy each possible amount of data from
     * it.  Then check that what is copied contains what is
     * expected and that no more has been copied.
     */
    for (i = 0; i < MR_num; i++) {
        for (copy_size = 1; copy_size <= __L4_NUM_MRS - i; copy_size++) {
            /* Zero buffer to check for copy overruns. */
            memset (buffer, '\0', sizeof(buffer));

            L4_StoreMRs (i, copy_size, buffer);

            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j < copy_size)
                    _fail_unless (buffer[j] == contents[i + j], __FILE__, __LINE__,
                                 "Wrong store:  MR %d, copy_size %d, buffer[%d] (0x%lx) != contents[%d] (0x%lx)",
                                 (int) i, (int) copy_size,
                                 (int) j, (long) buffer[j],
                                 (int) i + j, (long) contents[i + j]);
                else
                    _fail_unless (buffer[j] == 0, __FILE__, __LINE__,
                                 "Excess data (0x%ld) copied into buffer[%d]. MR %d, copy_size %d",
                                 (long) buffer[j], (int) j,
                                 (int) i, (int) copy_size);
            }
        }
    }
}
END_TEST


/*
\begin{test}{VIRT0600}
  \TestDescription{Range test for LoadMRs}
  \TestFunctionalityTested{\Func{LoadMRs}}
  \TestImplementationProcess{
    For varying sized arrays, for the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
	\item Call \Func{LoadMRs(i, k, xx)}
	\item Check values  
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{
    \begin{enumerate}
    \item xx is a random value
    \item Size of array tested may be varied by altering k 
    \end{enumerate}
  }
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0600)
{
    L4_Word_t MR_num;
    L4_Word_t i, j, copy_size;
    L4_Word_t* mr_base = get_mr_base ();
    L4_Word_t contents[__L4_NUM_MRS];

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    for (i = 0; i < MR_num; i++)
        contents[i] = 0xdeadbeef + i;

    /* Clear register to detect strange writes */
    memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);

    /* Check that writing 0 words into any register copies nothing. */
    for (i = 0 ; i < MR_num; i++) {
        L4_LoadMRs (i, 0, contents);

        for (j = 0; j < __L4_NUM_MRS; j++)
            _fail_unless (mr_base[j] == 0, __FILE__, __LINE__,
                         "Zero copy but MR[%d] contained 0x%lx.",
                         (int) j, (long) mr_base[j]);
    }

    /*
     * For each register copy each possible amount of data into
     * it.  Then check that what is copied contains what is
     * expected and that no more has been copied.
     */
    for (i = 0; i < MR_num; i++) {
        for (copy_size = 1; copy_size <= __L4_NUM_MRS - i; copy_size++) {
            memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);

            L4_LoadMRs (i, copy_size, contents);

            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j >= i && j < i + copy_size)
                    _fail_unless (mr_base[j] == contents[j - i], __FILE__, __LINE__,
                                 "Wrong load:  MR %d, copy_size %d, MR[%d] (0x%lx) != contents[%d] (0x%lx)",
                                 (int) i, (int) copy_size,
                                 (int) j, (long) mr_base[j],
                                 (int) j - i, (long) contents[j - i]);
                else
                    _fail_unless (mr_base[j] == 0, __FILE__, __LINE__,
                                 "Unexpected data (0x%ld) copied into MR[%d]. MR %d, copy_size %d",
                                 (long) mr_base[j], (int) j,
                                 (int) i, (int) copy_size);
            }
        }

        /* Check for one-off errors failing to write to the last register. */
        _fail_unless(mr_base[__L4_NUM_MRS - 1] == contents[__L4_NUM_MRS - 1 - i], __FILE__, __LINE__,
                    "One-off error.  Last MR is 0x%lx, expecting 0x%lx. i = %d.",
                    (long) mr_base[__L4_NUM_MRS - 1], (long) contents[__L4_NUM_MRS - 1 - i],
                    (int) i);
    }
}
END_TEST

/*
\begin{test}{VIRT0700}
  \TestDescription{Range test for LoadMRs and StoreMRs}
  \TestFunctionalityTested{\Func{LoadMRs} and \Func{StoreMRs}}
  \TestImplementationProcess{
    For varying sized arrays, for the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
	\item Call \Func{LoadMRs(i, k, xx)}
	\item Call \Func{StoreMRs(i,  k, xx)}
	\item Check values including under and overruns 
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{
	\begin{enumerate}
    \item xx is a random value
    \item Size of array tested may be varied by altering k 
    \end{enumerate}
  }
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0700)
{
    L4_Word_t MR_num;
    L4_Word_t i, j, copy_size;
    L4_Word_t* mr_base = get_mr_base ();
    L4_Word_t contents[__L4_NUM_MRS];
    L4_Word_t buffer[__L4_NUM_MRS];

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    for (i = 0; i < MR_num; i++)
        contents[i] = 0xdeadbeef + i;

    /* Clear register to detect strange writes */
    memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);
    memset (buffer, '\0', sizeof(buffer));

    /*
     * Check that writing and then reading 0 words to and from
     * any register copies nothing.
     */
    for (i = 0 ; i < MR_num; i++) {
        L4_LoadMRs (i, 0, contents);
        L4_StoreMRs(i, 0, buffer);

        for (j = 0; j < __L4_NUM_MRS; j++) {
            _fail_unless (mr_base[j] == 0, __FILE__, __LINE__,
                         "Zero copy but MR[%d] contained 0x%lx.",
                         (int) j, (long) mr_base[j]);

            _fail_unless (buffer[j] == 0, __FILE__, __LINE__,
                         "Zero copy but buffer[%d] contained 0x%lx.",
                         (int) j, (long) buffer[j]);
        }
    }

    /*
     * For each register copy each possible amount of data into it
     * and the copy it out again.  Then check that what is copied
     * contains what is expected and that no more has been copied.
     */
    for (i = 0; i < MR_num; i++) {
        for (copy_size = 1; copy_size <= __L4_NUM_MRS - i; copy_size++) {
            memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);

            L4_LoadMRs (i, copy_size, contents);
            L4_StoreMRs (i, copy_size, buffer);

            /*
             * Check that what we read is what we wrote
             * and nothing more.
             */
            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j >= i && j < i + copy_size)
                    _fail_unless (mr_base[j] == contents[j - i], __FILE__, __LINE__,
                                 "Wrong load:  MR %d, copy_size %d, MR[%d] (0x%lx) != contents[%d] (0x%lx)",
                                 (int) i, (int) copy_size,
                                 (int) j, (long) mr_base[j],
                                 (int) (j - i), (long) contents[j - i]);
                else
                    _fail_unless (mr_base[j] == 0, __FILE__, __LINE__,
                                 "Unexpected data (0x%ld) copied into MR[%d]. MR %d, copy_size %d",
                                 (long) mr_base[j], (int) j,
                                 (int) i, (int) copy_size);
            }

            /* Check that the registers contain only what we wrote. */
            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j >= i && j < i + copy_size)
                    _fail_unless (mr_base[j] == contents[j - i], __FILE__, __LINE__,
                                 "Wrong load:  MR %d, copy_size %d, MR[%d] (0x%lx) != contents[%d] (0x%lx)",
                                 (int) i, (int) copy_size,
                                 (int) j, (long) mr_base[j],
                                 (int) (j - i), (long) contents[j - i]);
                else
                    _fail_unless (mr_base[j] == 0, __FILE__, __LINE__,
                                 "Unexpected data (0x%ld) copied into MR[%d]. MR %d, copy_size %d",
                                 (long) mr_base[j], (int) j,
                                 (int) i, (int) copy_size);
            }
        }

        /* Check for one-off errors failing to write to the last register. */
        _fail_unless(mr_base[__L4_NUM_MRS - 1] == contents[__L4_NUM_MRS - 1 - i], __FILE__, __LINE__,
                    "One-off error.  Last MR is 0x%lx, expecting 0x%lx. i = %d.",
                    (long) mr_base[__L4_NUM_MRS - 1], (long) contents[__L4_NUM_MRS - 1 - i],
                    (int) i);
    }
}
END_TEST

/*
 * This is the same test as VIRT0700 but it is run in a separate
 * thread.  As a consequence it cannot call fail_unless().  Instead
 * error messages are stored in a per-thread buffer a message is sent
 * back to the master thread to tell it whether is passed or failed.
 * The master thread then logs errors via fail().
 */
static void threaded_load_store_mrs_test(void)
{
    L4_Word_t MR_num;
    L4_Word_t i, j, copy_size;
    L4_Word_t* mr_base = get_mr_base ();
    L4_Word_t contents[__L4_NUM_MRS];
    L4_Word_t buffer[__L4_NUM_MRS];

    MR_num = L4_GetMessageRegisters();

    multithreaded_fail_unless (MR_num == __L4_NUM_MRS, __LINE__,
                               "Unexpected number of MRs (expected %d, got %d).",
                               (int) __L4_NUM_MRS, (int) MR_num);

    for (i = 0; i < MR_num; i++)
        contents[i] = 0xdeadbeef + i;

    /* Clear register to detect strange writes */
    memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);
    memset (buffer, '\0', sizeof(buffer));

    /*
     * Check that writing and then reading 0 words to and from
     * any register copies nothing.
     */
    for (i = 0 ; i < MR_num; i++) {
        L4_LoadMRs (i, 0, contents);
        L4_StoreMRs(i, 0, buffer);

        for (j = 0; j < __L4_NUM_MRS; j++) {
            multithreaded_fail_unless (mr_base[j] == 0, __LINE__,
                                       "Zero copy but MR[%d] contained 0x%lx.",
                                       (int) j, (long) mr_base[j]);

            multithreaded_fail_unless (buffer[j] == 0, __LINE__,
                                       "Zero copy but buffer[%d] contained 0x%lx.",
                                       (int) j, (long) buffer[j]);
        }
    }

    /*
     * For each register copy each possible amount of data into it
     * and the copy it out again.  Then check that what is copied
     * contains what is expected and that no more has been copied.
     */
    for (i = 0; i < MR_num; i++) {
        for (copy_size = 1; copy_size <= __L4_NUM_MRS - i; copy_size++) {
            memset (mr_base, '\0', sizeof(L4_Word_t) * __L4_NUM_MRS);

            L4_LoadMRs (i, copy_size, contents);
            L4_StoreMRs (i, copy_size, buffer);

            /*
             * Check that what we read is what we wrote
             * and nothing more.
             */
            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j >= i && j < i + copy_size)
                    multithreaded_fail_unless (mr_base[j] == contents[j - i], __LINE__,
                                               "Wrong load:  MR %d, copy_size %d, MR[%d] (0x%lx) != contents[%d] (0x%lx)",
                                               (int) i, (int) copy_size,
                                               (int) j, (long) mr_base[j],
                                               (int) (j - i), (long) contents[j - i]);
                else
                    multithreaded_fail_unless (mr_base[j] == 0, __LINE__,
                                               "Unexpected data (0x%ld) copied into MR[%d]. MR %d, copy_size %d",
                                               (long) mr_base[j], (int) j,
                                               (int) i, (int) copy_size);
            }

            /* Check that the registers contain only what we wrote. */
            for (j = 0; j < __L4_NUM_MRS; j++) {
                if (j >= i && j < i + copy_size)
                    multithreaded_fail_unless (mr_base[j] == contents[j - i], __LINE__,
                                               "Wrong load:  MR %d, copy_size %d, MR[%d] (0x%lx) != contents[%d] (0x%lx)",
                                               (int) i, (int) copy_size,
                                               (int) j, (long) mr_base[j],
                                               (int) (j - i), (long) contents[j - i]);
                else
                    multithreaded_fail_unless (mr_base[j] == 0, __LINE__,
                                               "Unexpected data (0x%ld) copied into MR[%d]. MR %d, copy_size %d",
                                               (long) mr_base[j], (int) j,
                                               (int) i, (int) copy_size);
            }
        }

        /* Check for one-off errors failing to write to the last register. */
        multithreaded_fail_unless(mr_base[__L4_NUM_MRS - 1] == contents[__L4_NUM_MRS - 1 - i], __LINE__,
                                  "One-off error.  Last MR is 0x%lx, expecting 0x%lx. i = %d.",
                                  (long) mr_base[__L4_NUM_MRS - 1], (long) contents[__L4_NUM_MRS - 1 - i],
                                  (int) i);
    }

    multithreaded_pass ();
}

/*
\begin{test}{VIRT0800}
  \TestDescription{Multi-threaded range test for LoadMRs and StoreMRs}
  \TestFunctionalityTested{\Func{LoadMRs} and \Func{StoreMRs}}
  \TestImplementationProcess{
    Create multiple threads. For each thread, for the values of i from zero to MessageRegisters - 1: 
    \begin{enumerate}
    \item Call \Func{LoadMRs(i, k, xx)}
	\item Call \Func{StoreMRs(i, k,~xx)}
	\item Check values including under-runs and overruns 
	\end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestImplementationNotes{
  	\begin{enumerate}
    \item xx is a random value
    \item Size of array tested may be varied by altering k 
    \end{enumerate}
  }
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0800)
{
    run_multithreaded_test(__FILE__, threaded_load_store_mrs_test);
}
END_TEST

extern void main(void);

static void test_regs_pagefault(void)
{
    L4_Word_t MR_num;
    L4_Word_t i;

    MR_num = L4_GetMessageRegisters();

    _fail_unless (MR_num == __L4_NUM_MRS, __FILE__, __LINE__,
                 "Unexpected number of MRs (expected %d, got %d).",
                 (int) __L4_NUM_MRS, (int) MR_num);

    /*
     * Set each MR to a unique value, test if we can read it back correctly
     * after a pagefault.
     */
    for (i = 0; i < MR_num; i++) {
        L4_Word_t value = 0xdeadbeef ^ (i*0x1dffff);

        L4_LoadMR (i, value);
    }

    /*
     * Cause a pagefault: The pager should page any valid phys address to us
     */
    {
        volatile char * x = (char*)main;
        char tmp;
        tmp = *x;
    }

    for (i = 0; i < MR_num; i++) {
        L4_Word_t w = 0;
        L4_StoreMR (i, &w);

        _fail_unless(w == (0xdeadbeef ^ (i*0x1dffff)), __FILE__, __LINE__,
                    "MR[%d] written as 0x%lx, read as 0x%lx.",
                    (int) i, (long) (0xdeadbeef ^ (i*0x1dffff)), (long) w);
    }

    {
        L4_Msg_t msg;
        L4_MsgTag_t tag = L4_Niltag;

        while(1) {
            L4_MsgClear(&msg);
            L4_Set_MsgMsgTag(&msg, tag);
            L4_Set_MsgLabel(&msg, 0);
            L4_MsgLoad(&msg);

            L4_Call(main_thread);
        }
    }
}

/*
\begin{test}{VIRT0900}
  \TestDescription{Verify that MR is read back correctly following a pagefault}
  \TestFunctionalityTested{\Func{LoadMR} and \Func{StoreMR}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a thread to run the test
      \item Set-up the MRs with a known state
      \item Trigger a page fault
      \item Check the MRs after the fault
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(VIRT0900)
{
    L4_ThreadId_t thrd;

    thrd = createThreadInSpace(L4_nilspace, test_regs_pagefault);

    /* wait for test to be done */
    L4_Receive(thrd);

    deleteThread(thrd);
}
END_TEST

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    L4_Accept(L4_UntypedWordsAcceptor);

    main_thread = test_tid;
/*FIXME: Redo for Capabilities
    L4_IpcAllowToAll(L4_rootspace); */
}


static void test_teardown(void)
{
}

TCase *
make_virtual_register_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Virtual Register Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, VIRT0100);
    tcase_add_test(tc, VIRT0200);
    tcase_add_test(tc, VIRT0300);
    tcase_add_test(tc, VIRT0400);
    tcase_add_test(tc, VIRT0500);
    tcase_add_test(tc, VIRT0600);
    tcase_add_test(tc, VIRT0700);
    tcase_add_test(tc, VIRT0800);
    tcase_add_test(tc, VIRT0900);

    return tc;
}
