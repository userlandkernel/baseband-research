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

#include <stddef.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <l4test/l4test.h>
#include <l4test/utility.h>


/* Magic numbers for checking things */
#define MAGIC_LABEL1    0xdead

#define NOTIFY_BITS             0xcc
#define NOTIFY_BITS_BLOCKING    0x44
#define NOTIFY_MASK             0xaa
#define NOTIFY_MASK_NO_WAKEUP   0xa0
#define NOTIFY_MESSAGE          0x0f
#define NOTIFY_SUCCESS          0xcf

#define NONEXISTANT_TID ((L4_ThreadId_t) (L4_Word_t) 0x13371401)

static L4_ThreadId_t main_thread;

/* Helper thread functions */

/* a thread that spins without accepting async IPCs */
static void spinning_thread(void)
{
    L4_Accept(L4_UntypedWordsAcceptor);
    while(1)
        L4_ThreadSwitch(main_thread);
}

/* a thread that spins while accepting async IPCs */
static void accepting_spinning_thread(void)
{
    L4_Accept(L4_NotifyMsgAcceptor);
    while(1)
        L4_ThreadSwitch(main_thread);
}

/* a thread that waits without accepting async IPCs */
static void waiting_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    tag.raw = TAG_SRBLOCK;
    L4_Accept(L4_UntypedWordsAcceptor);
    while(1)
        L4_Ipc(L4_nilthread, main_thread, tag, &from);
}

/* a thread that spins while accepting async IPCs */
static void accepting_waiting_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    tag.raw = TAG_SRBLOCK;
    L4_Accept(L4_NotifyMsgAcceptor);
    while(1)
        L4_Ipc(L4_nilthread, main_thread, tag, &from);
}

/* A thread which sends an asynch ipc to it's pager repeatedly */
static void sending_thread(void)
{
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    L4_ThreadId_t from;

    // asynch message
    wait_tag.raw = TAG_SRBLOCK | 0x2001;
    while(1)
    {

        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(main_thread, main_thread, wait_tag, &from);


        if(getTagE(tag) == 1){
            L4_Ipc(L4_nilthread, main_thread, wait_tag, &from);
        }
    }

}

static void receive_specific_thread (void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);
        tag = setMsgTagAndAcceptor(i);

        //should block
        tag = L4_Ipc(L4_nilthread, main_thread, tag, &from);
    }
    L4_Ipc(main_thread, main_thread, tag, &from);
}

static void receive_any_thread (void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_A | BIT_R, BIT_R))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);
        tag = setMsgTagAndAcceptor(i);

        //should block
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);
        _fail_unless(L4_Get_NotifyBits() == NOTIFY_BITS, __FILE__, __LINE__, "Notify bits modified %s\n", tagIntToString(i));
    }
    L4_Ipc(main_thread, main_thread, tag, &from);
}

static void receive_any_check_thread (void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_A | BIT_R, BIT_A | BIT_R))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK_NO_WAKEUP);
        tag = setMsgTagAndAcceptor(i);

        //should block
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        _fail_unless(L4_Get_NotifyBits() == (NOTIFY_BITS_BLOCKING | NOTIFY_MESSAGE), __FILE__, __LINE__,
                     "Notify bits not changed %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     (NOTIFY_BITS_BLOCKING | NOTIFY_MESSAGE), tagIntToString(i));
    }
    L4_Ipc(main_thread, main_thread, tag, &from);
}

static void receive_waitnotify_thread (void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_A | BIT_R, BIT_R | BIT_A))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK);
        tag = setMsgTagAndAcceptor(i);

        //should block
        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);
    }
    L4_Ipc(main_thread, main_thread, tag, &from);
}

static void receive_waitnotify_check_thread (void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_A | BIT_R, BIT_R | BIT_A))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK_NO_WAKEUP);
        tag = setMsgTagAndAcceptor(i);

        //should block
        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);

        _fail_unless(L4_Get_NotifyBits() == (NOTIFY_BITS_BLOCKING | NOTIFY_MESSAGE), __FILE__, __LINE__,
                     "Notify bits not changed %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     (NOTIFY_BITS_BLOCKING | NOTIFY_MESSAGE), tagIntToString(i));
    }
    L4_Ipc(main_thread, main_thread, tag, &from);
}

static void async_send_normal_send_thread(void)
{
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    L4_ThreadId_t from;
    L4_Msg_t msg;

    int counter = 1;

    while(1)
    {
        // asynch message
        wait_tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(main_thread, L4_nilthread, wait_tag, &from);


        // synch message
        // u = 0
        L4_MsgClear(&msg);
        L4_Set_MsgMsgTag(&msg, tag);
        tag.raw |= TAG_SBLOCK;
        L4_Set_MsgLabel(&msg, MAGIC_LABEL1);
        L4_MsgLoad(&msg);
        L4_Ipc(main_thread, L4_nilthread, L4_MsgMsgTag(&msg), &from);



        // asynch message
        wait_tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(main_thread, L4_nilthread, wait_tag, &from);


        // u = 1

        tag.raw |= TAG_SBLOCK;
        fillMessage(&msg, tag);
        L4_MsgLoad(&msg);
        L4_Ipc(main_thread, L4_nilthread, L4_MsgMsgTag(&msg), &from);

        counter++;
    }

}

/*
\begin{test}{AIPC2500}
  \TestDescription{Asynchronous IPC send to not accepting, spinning thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0.E is set, 
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 5
    \end{enumerate}}
  \TestPermutedCases{srpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC2500)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(spinning_thread);

    // allow partner 10 timeslices to start up
    waitSending(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_N, BIT_N))
            continue;

        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(getTagE(tag) == 1, __FILE__, __LINE__, "Error not thrown %s\n", tagIntToString(i));
        _fail_unless(getECodeP(L4_ErrorCode()) == 0, __FILE__, __LINE__, "Wrong Error phase %s\n", tagIntToString(i));
        _fail_unless(getECodeE(L4_ErrorCode()) == 5, __FILE__, __LINE__, "Wrong Error code %s\n", tagIntToString(i));
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC2600}
  \TestDescription{Asynchronous IPC send to accepting, spinning thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag
    \end{enumerate}}
  \TestPermutedCases{srpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC2600)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(accepting_spinning_thread);

    // allow partner 10 timeslices to start up
    waitSending(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_N, BIT_N))
            continue;

        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Incorrect tag %s\n", tagIntToString(i));
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC2700}
  \TestDescription{Asynchronous IPC send to not accepting, waiting thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{waiting thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0.E is set, 
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 5
    \end{enumerate}}
  \TestPermutedCases{srpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC2700)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(waiting_thread);


    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_N, BIT_N))
            continue;

        waitReceiving(partner);
        assert(checkReceiving(partner));

        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(getTagE(tag) == 1, __FILE__, __LINE__, "Error not thrown %s\n", tagIntToString(i));
        _fail_unless(getECodeP(L4_ErrorCode()) == 0, __FILE__, __LINE__, "Wrong Error phase %s\n", tagIntToString(i));
        _fail_unless(getECodeE(L4_ErrorCode()) == 5, __FILE__, __LINE__, "Wrong Error code %s\n", tagIntToString(i));
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC2800}
  \TestDescription{Asynchronous IPC }
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{waiting thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag
    \end{enumerate}}
  \TestPermutedCases{srpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC2800)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(accepting_waiting_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_N, BIT_N))
            continue;

        waitReceiving(partner);
        assert(checkReceiving(partner));

        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Incorrect tag %s\n", tagIntToString(i));
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC2900}
  \TestDescription{Asynchronous IPC spinning thread with acceptor set}

  \TestPostConditions{\begin{enumerate}
    \item  async bits are delivered to UTCB
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC2900)
{
    L4_ThreadId_t partner;

    partner = createThread(sending_thread);

    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyBits(NOTIFY_BITS);
    L4_Set_NotifyMask(NOTIFY_MASK);

    waitReceiving(partner);
    assert(checkReceiving(partner));

    fail_unless(L4_Get_NotifyBits() == NOTIFY_SUCCESS, "Notify bits not modified\n");
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3000}
  \TestDescription{Asynchronous IPC spinning thread with acceptor not set}

  \TestPostConditions{\begin{enumerate}
    \item  async bits are not delivered to UTCB
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3000)
{
    L4_ThreadId_t partner;

    partner = createThread(sending_thread);

    L4_Accept(L4_UntypedWordsAcceptor);
    L4_Set_NotifyBits(NOTIFY_BITS);
    L4_Set_NotifyMask(NOTIFY_MASK);

    waitReceiving(partner);
    assert(checkReceiving(partner));

    fail_unless(L4_Get_NotifyBits() == NOTIFY_BITS, "Notify bits modified\n");
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3100}
  \TestDescription{Asynchronous IPC receive from specific thread, async bits waiting}

  \TestPostConditions{Non-blocking case:
  \begin{enumerate}
    \item block or error
  \end{enumerate}}
  \TestPermutedCases{spnua (special r)}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3100)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    int i;


    partner = createThread(sending_thread);

    L4_Set_NotifyBits(NOTIFY_BITS);
    L4_Set_NotifyMask(NOTIFY_MASK);

    waitReceiving(partner);
    assert(checkReceiving(partner));

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R|BIT_N, 0))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);
        tag = setMsgTagAndAcceptor(i);

        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 1, __FILE__, __LINE__,
                    "Wrong Error phase %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 1 , __FILE__, __LINE__,
                    "Wrong Error code %s", tagIntToString(i));

        tag.raw = TAG_SBLOCK;
        L4_Ipc(partner, L4_nilthread, tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*

*/
START_TEST(AIPC3101)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(receive_specific_thread);

    L4_Set_NotifyBits(NOTIFY_BITS);
    L4_Set_NotifyMask(NOTIFY_MASK);

    // keep in sync with receiver thread
    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        waitReceiving(partner);
        fail_unless(checkReceiving(partner), "Receiver did not block");

        //restart partner for next test
        L4_Start(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3200}
  \TestDescription{Asynchronous IPC receive waitnotify, async bits waiting}

  \TestPostConditions{\begin{enumerate}
    \item  returns async bits from = nilthread, 
    \item tag.E = 0, 
    \item mr1 = async bits, 
    \item UTCB notify bits are cleared
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3200)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(sending_thread);

    for(i = 0; i < NUM_PERMUTS; ++i){
        L4_Word_t bits;
        if(!validTest(i, BIT_A, BIT_A))
            continue;

        tag = setMsgTagAndAcceptor(i);

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);

        waitReceiving(partner);
        assert(checkReceiving(partner));

        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);

        L4_StoreMR(1, &bits);

        fail_unless(getTagE(tag) == 0, "Returned error\n");
        fail_unless(((NOTIFY_MESSAGE | NOTIFY_BITS) & NOTIFY_MASK) == bits, "Notify bits not modified\n");
        _fail_unless(L4_Get_NotifyBits() == ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)),
                     __FILE__, __LINE__,
                     "Notify bits not cleared %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)), tagIntToString(i));

        tag.raw = TAG_SBLOCK;
        L4_Ipc(partner, L4_nilthread, tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3300}
  \TestDescription{Asynchronous IPC receive from anythread, async bits waiting, acceptor set}

  \TestPostConditions{\begin{enumerate}
    \item  returns async bits from = nilthread, 
    \item tag.E = 0, 
    \item mr1 = async bits, 
    \item UTCB notify bits are cleared
    \end{enumerate}}
  \TestPermutedCases{srnpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3300)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(sending_thread);

    for(i = 0; i < NUM_PERMUTS; ++i){
        L4_Word_t bits;
        if(!validTest(i, BIT_A, BIT_A))
            continue;

        tag = setMsgTagAndAcceptor(i);

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);

        waitReceiving(partner);
        assert(checkReceiving(partner));

        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_StoreMR(1, &bits);

        fail_unless(getTagE(tag) == 0, "Returned error\n");
        fail_unless(((NOTIFY_MESSAGE | NOTIFY_BITS) & NOTIFY_MASK) == bits, "Notify bits not modified\n");
        _fail_unless(L4_Get_NotifyBits() == ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)),
                     __FILE__, __LINE__,
                     "Notify bits not cleared %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)), tagIntToString(i));

        tag.raw = TAG_SBLOCK;
        L4_Ipc(partner, L4_nilthread, tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3400}
  \TestDescription{Asynchronous IPC receive from anythread, async bits waiting, acceptor not set}

  \TestPostConditions{
  Non-blocking case:
  \begin{enumerate}
    \item block or error
  \end{enumerate}}
  \TestPermutedCases{srnu (special r)}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3400)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(sending_thread);

    for(i = 0; i < NUM_PERMUTS; ++i){
        L4_Word_t bits;
        if(!validTest(i, BIT_A | BIT_R, 0)){
            continue;
        }

        tag = setMsgTagAndAcceptor(i);

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);

        waitReceiving(partner);
        assert(checkReceiving(partner));

        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_StoreMR(1, &bits);

        _fail_unless(getTagE(tag) == 1, __FILE__, __LINE__, "No Error %s\n", tagIntToString(i));
        _fail_unless(getECodeP(L4_ErrorCode()) == 1, __FILE__, __LINE__, "ErrorCode phase incorrect %s\n", tagIntToString(i));
        _fail_unless(getECodeE(L4_ErrorCode()) == 1, __FILE__, __LINE__, "ErrorCode incorrect %s\n", tagIntToString(i));

        tag.raw = TAG_SBLOCK;
        L4_Ipc(partner, L4_nilthread, tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*

*/
START_TEST(AIPC3401)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(receive_any_thread);

    for(i = 0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_A | BIT_R, BIT_R)){
            continue;
        }

        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        waitReceiving(partner);
        fail_unless(checkReceiving(partner), "Receiver did not block");

        //restart partner for next test
        L4_Start(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3500}
  \TestDescription{Asynchronous IPC blocking receive from specific thread then async IPC sent}

  \TestPostConditions{\begin{enumerate}
    \item check thread is still blocked
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3500)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    wait_tag.raw = TAG_SRBLOCK;


    partner = createThread(receive_specific_thread);
    L4_ThreadSwitch(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // allow partner to block
        waitReceiving(partner);
        assert(checkReceiving(partner));

        // then send async message
        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        fail_unless(checkReceiving(partner), "Receiver woke up");

        //restart partner for next test
        L4_Start(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3600}
  \TestDescription{Asynchronous IPC blocking receive notifywait, accpetor set, thread then normal IPC sent }

  \TestPostConditions{\begin{enumerate}
        \item check thread is still blocked
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3600)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(receive_waitnotify_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R | BIT_A))
            continue;

        // allow partner to block
        waitReceiving(partner);
        assert(checkReceiving(partner));

        // then send async message
        tag.raw = TAG_NOBLOCK;
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        fail_unless(checkReceiving(partner), "Receiver woke up");

        //restart partner for next test
        L4_Start(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3700}
  \TestDescription{Asynchronous IPC blocking receive notifywait, accpetor set, thread then asynch IPC sent, wakes up}

  \TestPostConditions{\begin{enumerate}
    \item  returns async bits from = nilthread, 
    \item tag.E = 0, 
    \item mr1 = async bits,
    \item UTCB notify bits are cleared
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3700)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    L4_Word_t bits;
    int i;
    wait_tag.raw = TAG_SRBLOCK;


    partner = createThread(sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R | BIT_A))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK);

        // the IPC being tested
        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);
        L4_StoreMR(1, &bits);

        _fail_unless(tag.raw == 1, __FILE__, __LINE__, "Incorrect tag %s\n", tagIntToString(i));
        _fail_unless(from.raw == 0, __FILE__, __LINE__, "Incorrect from %s\n", tagIntToString(i));
        _fail_unless(bits == (NOTIFY_MESSAGE & NOTIFY_MASK), __FILE__, __LINE__, "Notify bits incorrect %x %s\n", (unsigned int) bits, tagIntToString(i));
        _fail_unless(L4_Get_NotifyBits() == ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)),
                     __FILE__, __LINE__, 
                     "Notify bits not cleared %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)), tagIntToString(i));

        // reset the sending thread
        L4_Ipc(partner, L4_nilthread, wait_tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3800}
  \TestDescription{Asynchronous IPC receive  notifywait, acceptor set, thread then asynch IPC sent, stays blocked}

  \TestPostConditions{\begin{enumerate}
    \item  check that thread is still blocked and the async bits are in the UTCB
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3800)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(receive_waitnotify_check_thread);

    // allow partner to block
    waitReceiving(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R | BIT_A))
            continue;

        assert(checkReceiving(partner));

        // send async message
        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        fail_unless(checkReceiving(partner), "Receiver woke up");

        //restart partner for next test
        L4_Start(partner);

        // allow partner to check notify bits and block again
        waitReceiving(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC3900}
  \TestDescription{Asynchronous IPC receive blocking notifywait, not accepting}

  \TestPostConditions{\begin{enumerate}
    \item  msgTag.E = 1, 
    \item errorCode.p=1, 
    \item errorCode.e = 5
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC3900)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    wait_tag.raw = TAG_SRBLOCK;


    partner = createThread(sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK_NO_WAKEUP);

        // the IPC being tested
        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);

        _fail_unless(getTagE(tag) == 1, __FILE__, __LINE__, "No Error %s\n", tagIntToString(i));
        _fail_unless(getECodeP(L4_ErrorCode()) == 1, __FILE__, __LINE__, "ErrorCode phase incorrect %s\n", tagIntToString(i));
        _fail_unless(getECodeE(L4_ErrorCode()) == 5, __FILE__, __LINE__, "ErrorCode incorrect %s\n", tagIntToString(i));
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC4000}
  \TestDescription{Asynchronous IPC receive  blocking anythread, accpetor set}
  \TestImplementationProcess{IPC blocking receieve anythread, accpetor set, then asynch ipc sent, wakes up}
  \TestPostConditions{\begin{enumerate}
    \item  returns async bits from = nilthread, 
    \item tag.E = 0, 
    \item mr1 = async bits, 
    \item UTCB notify bits are cleared 
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC4000)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    L4_Word_t bits;
    int i;
    wait_tag.raw = TAG_SRBLOCK;


    partner = createThread(sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R | BIT_A))
            continue;

        L4_Set_NotifyBits(NOTIFY_BITS_BLOCKING);
        L4_Set_NotifyMask(NOTIFY_MASK);

        // the IPC being tested
        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);
        L4_StoreMR(1, &bits);

        _fail_unless(tag.raw == 1, __FILE__, __LINE__, "Incorrect tag %s\n", tagIntToString(i));
        _fail_unless(from.raw == 0, __FILE__, __LINE__, "Incorrect from %s\n", tagIntToString(i));
        _fail_unless(bits == (NOTIFY_MESSAGE & NOTIFY_MASK), __FILE__, __LINE__, 
                     "Notify bits incorrect %x %s\n", (unsigned int) bits, tagIntToString(i));
        _fail_unless(L4_Get_NotifyBits() == ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)),
                     __FILE__, __LINE__, 
                     "Notify bits not cleared %x != %x %s\n", (unsigned int) L4_Get_NotifyBits(),
                     ((NOTIFY_MESSAGE | NOTIFY_BITS_BLOCKING) & (~NOTIFY_MASK)), tagIntToString(i));

        // reset the sending thread
        L4_Ipc(partner, L4_nilthread, wait_tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC4100}
  \TestDescription{Asynchronous IPC receive }

  \TestPostConditions{\begin{enumerate}
    \item  check that thread is still blocked and the async bits are in the UTCB
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC4100)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;

    partner = createThread(receive_any_check_thread);

    waitReceiving(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R | BIT_A))
            continue;

        assert(checkReceiving(partner));

        // send async message
        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        fail_unless(checkReceiving(partner), "Receiver woke up");

        //restart partner for next test
        L4_Start(partner);

        // allow partner to check notify bits and block again
        waitReceiving(partner);

    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC4200}
  \TestDescription{Asynchronous IPC receive }

  \TestPostConditions{\begin{enumerate}
    \item  check that thread is still blocked and the async bits are not in the UTCB
    \end{enumerate}}
  \TestPermutedCases{snpu}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC4200)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    wait_tag.raw = TAG_SRBLOCK;

    partner = createThread(receive_any_thread);

    waitReceiving(partner);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R | BIT_A, BIT_R))
            continue;

        assert(checkReceiving(partner));

        // send async message
        tag.raw = TAG_SRBLOCK | 0x2001;
        L4_LoadMR(1, NOTIFY_MESSAGE);
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        fail_unless(checkReceiving(partner), "Receiver woke up");

        //restart partner for next test
        L4_Start(partner);

        // allow partner to check notify bits and block again
        waitReceiving(partner);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC4300}
  \TestDescription{Asynchronous IPC receive }

  \TestPostConditions{\begin{enumerate}
    \item  returns async bits from = nilthread, 
    \item tag.E = 0, 
    \item mr1 = async bits, 
    \item UTCB notify bits are cleared
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC4300)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(async_send_normal_send_thread);


    for(i = 0; i < NUM_PERMUTS; ++i){
        L4_Word_t bits;
        if(!validTest(i, BIT_A, BIT_A)){
            continue;
        }

        tag = setMsgTagAndAcceptor(i);

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);

        waitSending(partner);

        tag = L4_Ipc(L4_nilthread, L4_waitnotify, tag, &from);

        L4_StoreMR(1, &bits);

        _fail_unless(tag.raw == 1, __FILE__, __LINE__, "Incorrect tag %s\n", tagIntToString(i));
        _fail_unless(from.raw == 0, __FILE__, __LINE__, "Incorrect from %s\n", tagIntToString(i));
        _fail_unless(bits == ((NOTIFY_BITS | NOTIFY_MESSAGE) & NOTIFY_MASK), __FILE__, __LINE__, 
                     "Notify bits %x exp %x %s\n", 
                     (unsigned int) bits, (unsigned int)((NOTIFY_BITS | NOTIFY_MESSAGE) & NOTIFY_MASK), tagIntToString(i));
        _fail_unless(L4_Get_NotifyBits() == ((NOTIFY_MESSAGE | NOTIFY_BITS) & (~NOTIFY_MASK)),
                __FILE__, __LINE__, "Notify bits not cleared %x != %x %s\n",
                (unsigned int) L4_Get_NotifyBits(),
                ((NOTIFY_MESSAGE | NOTIFY_BITS) & (~NOTIFY_MASK)), tagIntToString(i));

        tag.raw = TAG_SRBLOCK;
        L4_Ipc(L4_nilthread, partner, tag, &from);
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{AIPC4400}
  \TestDescription{Asynchronous IPC receive }

  \TestPostConditions{\begin{enumerate}
    \item  from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestNotes{See Table~\ref{table:asyncrecv} for test value combinations.}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(AIPC4400)
{
    L4_ThreadId_t partner;
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;


    partner = createThread(async_send_normal_send_thread);

    for(i = 0; i < NUM_PERMUTS; ++i){
        L4_Msg_t msg;

        if(!validTest(i, BIT_A | BIT_R | BIT_U, 0)){
            continue;
        }

        tag = setMsgTagAndAcceptor(i);

        waitSending(partner);
        assert(checkSending(partner));

        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);


        _fail_unless(from.raw != 0, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));


        i |= BIT_U;

        L4_Set_NotifyBits(NOTIFY_BITS);
        L4_Set_NotifyMask(NOTIFY_MASK);

        waitSending(partner);
        assert(checkSending(partner));

        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw != 0, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
        i &= ~BIT_U;
    }
    deleteThread(partner);
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

static void xas_test_setup(void)
{
    initThreads(1);
    main_thread = test_tid;
}

static void xas_test_teardown(void)
{

}

TCase *
make_aipc_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Ipc async");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, AIPC2500);
    tcase_add_test(tc, AIPC2600);
    tcase_add_test(tc, AIPC2700);
    tcase_add_test(tc, AIPC2800);
    tcase_add_test(tc, AIPC2900);
    tcase_add_test(tc, AIPC3000);
    tcase_add_test(tc, AIPC3100);
    tcase_add_test(tc, AIPC3101);
    tcase_add_test(tc, AIPC3200);
    tcase_add_test(tc, AIPC3300);
    tcase_add_test(tc, AIPC3400);
    tcase_add_test(tc, AIPC3401);
    tcase_add_test(tc, AIPC3500);
    tcase_add_test(tc, AIPC3600);
    tcase_add_test(tc, AIPC3700);
    tcase_add_test(tc, AIPC3800);
    tcase_add_test(tc, AIPC3900);
    tcase_add_test(tc, AIPC4000);
    tcase_add_test(tc, AIPC4100);
    tcase_add_test(tc, AIPC4200);
    tcase_add_test(tc, AIPC4300);
    tcase_add_test(tc, AIPC4400);

    return tc;
}

TCase *
make_xas_aipc_tcase(void)
{
    TCase *tc;

    initThreads(1);

    /* The tests that are commented out cannot be run xas as
     * they use Exreg across address spaces when not running as roottask
     */
    tc = tcase_create("Ipc async XAS");
    tcase_add_checked_fixture(tc, xas_test_setup, xas_test_teardown);
    tcase_add_test(tc, AIPC2500);
    tcase_add_test(tc, AIPC2600);
    tcase_add_test(tc, AIPC2700);
    tcase_add_test(tc, AIPC2800);
    tcase_add_test(tc, AIPC2900);
    tcase_add_test(tc, AIPC3000);
    tcase_add_test(tc, AIPC3100);
    tcase_add_test(tc, AIPC3101);
    tcase_add_test(tc, AIPC3200);
    tcase_add_test(tc, AIPC3300);
    tcase_add_test(tc, AIPC3400);
    tcase_add_test(tc, AIPC3401);
    tcase_add_test(tc, AIPC3500);
    tcase_add_test(tc, AIPC3600);
    tcase_add_test(tc, AIPC3700);
    tcase_add_test(tc, AIPC3800);
    tcase_add_test(tc, AIPC3900);
    tcase_add_test(tc, AIPC4000);
    tcase_add_test(tc, AIPC4100);
    tcase_add_test(tc, AIPC4200);
    tcase_add_test(tc, AIPC4300);
    tcase_add_test(tc, AIPC4400);

    return tc;
}
