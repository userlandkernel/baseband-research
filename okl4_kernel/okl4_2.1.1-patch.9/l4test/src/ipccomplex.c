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

#include "ipccomplex_utility.h"
#include <l4test/utility.h>
#include <l4test/l4test.h>
#include <l4/ipc.h>
#include <stdio.h>

static L4_ThreadId_t ipc_main_thread;

L4_ThreadId_t
get_ipc_main_tid(void)
{
    return ipc_main_thread;
}

/*
\begin{test}{IPCCPX0001}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0001)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE01);
}
END_TEST

/*
\begin{test}{IPCCPX0002}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0002)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE02);
}
END_TEST

/*
\begin{test}{IPCCPX0003}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0003)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE03);
}
END_TEST

/*
\begin{test}{IPCCPX0004}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0004)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE04);
}
END_TEST

/*
\begin{test}{IPCCPX0005}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0005)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE05);
}
END_TEST

/*
\begin{test}{IPCCPX0006}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0006)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE06);
}
END_TEST

/*
\begin{test}{IPCCPX0007}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0007)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE07);
}
END_TEST

/*
\begin{test}{IPCCPX0008}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0008)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE08);
}
END_TEST

/*
\begin{test}{IPCCPX0009}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0009)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE09);
}
END_TEST

/*
\begin{test}{IPCCPX0010}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0010)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE10);
}
END_TEST

/*
\begin{test}{IPCCPX0011}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0011)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE11);
}
END_TEST

/*
\begin{test}{IPCCPX0012}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0012)
{
    ipccomplex_test(MAIN_SBLOCK | MAIN_RBLOCK | CASE12);
}
END_TEST

/*
\begin{test}{IPCCPX0013}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0013)
{
    ipccomplex_test(MAIN_RBLOCK | CASE01);
}
END_TEST

/*
\begin{test}{IPCCPX0014}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0014)
{
    ipccomplex_test(MAIN_RBLOCK | CASE02);
}
END_TEST

/*
\begin{test}{IPCCPX0015}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0015)
{
    ipccomplex_test(MAIN_RBLOCK | CASE03);
}
END_TEST

/*
\begin{test}{IPCCPX0016}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0016)
{
    ipccomplex_test(MAIN_RBLOCK | CASE04);
}
END_TEST

/*
\begin{test}{IPCCPX0017}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0017)
{
    ipccomplex_test(MAIN_RBLOCK | CASE05);
}
END_TEST

/*
\begin{test}{IPCCPX0018}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0018)
{
    ipccomplex_test(MAIN_RBLOCK | CASE06);
}
END_TEST

/*
\begin{test}{IPCCPX0019}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0019)
{
    ipccomplex_test(MAIN_RBLOCK | CASE07);
}
END_TEST

/*
\begin{test}{IPCCPX0020}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0020)
{
    ipccomplex_test(MAIN_RBLOCK | CASE08);
}
END_TEST

/*
\begin{test}{IPCCPX0021}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0021)
{
    ipccomplex_test(MAIN_RBLOCK | CASE09);
}
END_TEST

/*
\begin{test}{IPCCPX0022}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0022)
{
    ipccomplex_test(MAIN_RBLOCK | CASE10);
}
END_TEST

/*
\begin{test}{IPCCPX0023}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0023)
{
    ipccomplex_test(MAIN_RBLOCK | CASE11);
}
END_TEST

/*
\begin{test}{IPCCPX0024}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0024)
{
    ipccomplex_test(MAIN_RBLOCK | CASE12);
}
END_TEST

/*
\begin{test}{IPCCPX0025}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0025)
{
    ipccomplex_test(MAIN_SBLOCK | CASE01);
}
END_TEST

/*
\begin{test}{IPCCPX0026}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0026)
{
    ipccomplex_test(MAIN_SBLOCK | CASE02);
}
END_TEST

/*
\begin{test}{IPCCPX0027}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0027)
{
    ipccomplex_test(MAIN_SBLOCK | CASE03);
}
END_TEST

/*
\begin{test}{IPCCPX0028}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0028)
{
    ipccomplex_test(MAIN_SBLOCK | CASE04);
}
END_TEST

/*
\begin{test}{IPCCPX0029}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0029)
{
    ipccomplex_test(MAIN_SBLOCK | CASE05);
}
END_TEST

/*
\begin{test}{IPCCPX0030}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0030)
{
    ipccomplex_test(MAIN_SBLOCK | CASE06);
}
END_TEST

/*
\begin{test}{IPCCPX0031}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0031)
{
    ipccomplex_test(MAIN_SBLOCK | CASE07);
}
END_TEST

/*
\begin{test}{IPCCPX0032}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0032)
{
    ipccomplex_test(MAIN_SBLOCK | CASE08);
}
END_TEST

/*
\begin{test}{IPCCPX0033}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0033)
{
    ipccomplex_test(MAIN_SBLOCK | CASE09);
}
END_TEST

/*
\begin{test}{IPCCPX0034}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0034)
{
    ipccomplex_test(MAIN_SBLOCK | CASE10);
}
END_TEST

/*
\begin{test}{IPCCPX0035}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0035)
{
    ipccomplex_test(MAIN_SBLOCK | CASE11);
}
END_TEST

/*
\begin{test}{IPCCPX0036}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0036)
{
    ipccomplex_test(MAIN_SBLOCK | CASE12);
}
END_TEST

/*
\begin{test}{IPCCPX0037}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0037)
{
    ipccomplex_test(CASE01);
}
END_TEST

/*
\begin{test}{IPCCPX0038}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0038)
{
    ipccomplex_test(CASE02);
}
END_TEST

/*
\begin{test}{IPCCPX0039}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0039)
{
    ipccomplex_test(CASE03);
}
END_TEST

/*
\begin{test}{IPCCPX0040}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0040)
{
    ipccomplex_test(CASE04);
}
END_TEST

/*
\begin{test}{IPCCPX0041}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0041)
{
    ipccomplex_test(CASE05);
}
END_TEST

/*
\begin{test}{IPCCPX0042}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread not allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0042)
{
    ipccomplex_test(CASE06);
}
END_TEST

/*
\begin{test}{IPCCPX0043}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread not allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0043)
{
    ipccomplex_test(CASE07);
}
END_TEST

/*
\begin{test}{IPCCPX0044}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0044)
{
    ipccomplex_test(CASE08);
}
END_TEST

/*
\begin{test}{IPCCPX0045}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0045)
{
    ipccomplex_test(CASE09);
}
END_TEST

/*
\begin{test}{IPCCPX0046}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0046)
{
    ipccomplex_test(CASE10);
}
END_TEST

/*
\begin{test}{IPCCPX0047}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: non blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0047)
{
    ipccomplex_test(CASE11);
}
END_TEST

/*
\begin{test}{IPCCPX0048}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\begin{enumerate}
    \item main thread: non blocking send, non blocking receive.
    \item receiving thread: blocking receive, not ready to receive, IPC send from main thread allowed.
    \item sending thread: non blocking send, not ready to send, IPC send to main thread allowed.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX0048)
{
    ipccomplex_test(CASE12);
}
END_TEST

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(1);
    ipc_main_thread = test_tid;
}

static void test_teardown(void)
{

}

TCase *
make_ipc_cpx_tcase(void)
{
    TCase *tc;
   
    initThreads(0);

    tc = tcase_create("Ipc complex");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    
    tcase_add_test(tc, IPCCPX0001);
    tcase_add_test(tc, IPCCPX0002);
    tcase_add_test(tc, IPCCPX0003);
    tcase_add_test(tc, IPCCPX0004);
    tcase_add_test(tc, IPCCPX0005);
    tcase_add_test(tc, IPCCPX0006);
    tcase_add_test(tc, IPCCPX0007);
    tcase_add_test(tc, IPCCPX0008);
    tcase_add_test(tc, IPCCPX0009);
    tcase_add_test(tc, IPCCPX0010);
    tcase_add_test(tc, IPCCPX0011);
    tcase_add_test(tc, IPCCPX0012);
    tcase_add_test(tc, IPCCPX0013);
    tcase_add_test(tc, IPCCPX0014);
    tcase_add_test(tc, IPCCPX0015);
    tcase_add_test(tc, IPCCPX0016);
    tcase_add_test(tc, IPCCPX0017);
    tcase_add_test(tc, IPCCPX0018);
    tcase_add_test(tc, IPCCPX0019);
    tcase_add_test(tc, IPCCPX0020);
    tcase_add_test(tc, IPCCPX0021);
    tcase_add_test(tc, IPCCPX0022);
    tcase_add_test(tc, IPCCPX0023);
    tcase_add_test(tc, IPCCPX0024);
    tcase_add_test(tc, IPCCPX0025);
    tcase_add_test(tc, IPCCPX0026);
    tcase_add_test(tc, IPCCPX0027);
    tcase_add_test(tc, IPCCPX0028);
    tcase_add_test(tc, IPCCPX0029);
    tcase_add_test(tc, IPCCPX0030);
    tcase_add_test(tc, IPCCPX0031);
    tcase_add_test(tc, IPCCPX0032);
    tcase_add_test(tc, IPCCPX0033);
    tcase_add_test(tc, IPCCPX0034);
    tcase_add_test(tc, IPCCPX0035);
    tcase_add_test(tc, IPCCPX0036);
    tcase_add_test(tc, IPCCPX0037);
    tcase_add_test(tc, IPCCPX0038);
    tcase_add_test(tc, IPCCPX0039);
    tcase_add_test(tc, IPCCPX0040);
    tcase_add_test(tc, IPCCPX0041);
    tcase_add_test(tc, IPCCPX0042);
    tcase_add_test(tc, IPCCPX0043);
    tcase_add_test(tc, IPCCPX0044);
    tcase_add_test(tc, IPCCPX0045);
    tcase_add_test(tc, IPCCPX0046);
    tcase_add_test(tc, IPCCPX0047);
    tcase_add_test(tc, IPCCPX0048);

    return tc;
}
