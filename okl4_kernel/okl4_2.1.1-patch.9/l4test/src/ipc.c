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
#include <l4/schedule.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/kdebug.h>
#include <l4/arch/vregs.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Basic tags specifying send and receive blocking */
#define TAG_NOBLOCK 0x00000000
#define TAG_RBLOCK  0x00004000
#define TAG_SBLOCK  0x00008000
#define TAG_SRBLOCK 0x0000C000

/* Magic numbers for checking things */
#define MAGIC_LABEL1    0xdead
static const L4_ThreadId_t NONEXISTANT_TID = { 0x13371401UL };

static L4_ThreadId_t main_thread;
static L4_ThreadId_t other_thread;

/* Helper thread functions */

/* A thread that does nothing */
static void spinning_thread(void)
{
    while(1)
        ; // do nothing
}

/* A thread that waits for IPCs then sends and waits again */
static void waiting_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    //send and receive both block
    tag.raw = TAG_SRBLOCK;
    while(1)
        L4_Ipc(L4_nilthread, main_thread, tag, &dummy);
}

/* a thread the sends IPCs */
static void sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    //send and receive both block
    tag.raw = TAG_SRBLOCK;

    while(1){
        L4_Ipc(main_thread, L4_nilthread, tag, &dummy);
    }
}

/* a thread the sends IPCs to other_thread */
static void other_sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    //send and receive both block
    tag.raw = TAG_SRBLOCK;

    while(1){
        L4_Ipc(other_thread, L4_nilthread, tag, &dummy);
    }
}

/* A thread that sends IPCs specially formatted*/
static void special_sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    tag.raw = TAG_SBLOCK;
    while(1)
    {
        L4_MsgClear(&msg);
        L4_Set_MsgMsgTag(&msg, tag);
        L4_Set_MsgLabel(&msg, MAGIC_LABEL1);
        L4_MsgLoad(&msg);
        L4_Ipc(main_thread, L4_nilthread, L4_MsgMsgTag(&msg), &dummy);

        fillMessage(&msg, tag);
        L4_MsgLoad(&msg);
        L4_Ipc(main_thread, L4_nilthread, L4_MsgMsgTag(&msg), &dummy);
    }
}

static void blocking_send_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t dummy;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_S | BIT_P | BIT_N, BIT_S))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(main_thread, L4_nilthread, tag, &dummy);
    }
}

/* A thread that sends IPCs for ipc21 */
static void n_sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    int i = 0;
    int j;

    //send and receive both block
    tag.raw = TAG_SBLOCK;
    while(1)
    {
        L4_MsgClear(&msg);
        L4_Set_MsgMsgTag(&msg, tag);
        L4_Set_MsgLabel(&msg, i*i);
        // LibL4 does not range check on messages(for performance), so do it ourselves to avoid
        // stuffing up the unit tests
        for(j = 0; j < ((i<(__L4_NUM_MRS-1))?i:(__L4_NUM_MRS-1)); j++)
            L4_MsgPutWord(&msg, j, (i*i)+(j*j));
        msg.tag.X.u = (i > 63) ? 63 : i;

        /* Be carefull about loading Mrs - can't use L4_MsgLoad */
        L4_LoadMRs(0, (msg.tag.X.u > __L4_NUM_MRS) ? (__L4_NUM_MRS) : msg.tag.X.u+1, &msg.msg[0]);

        L4_Ipc(main_thread, L4_nilthread, L4_MsgMsgTag(&msg), &dummy);

        i++;
    }
}

/* A thread that receives/sends IPCs for ipc25 */
static void ipc25_test_thread(void)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag, rcv_tag;
    L4_Msg_t msg;
    int j, mr_num;
    int i = 0;

    /* send / receive block */
    tag.raw = TAG_SBLOCK | TAG_RBLOCK;

    mr_num = IPC_NUM_MR;

    L4_MsgClear(&msg);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0);

    /* Fill MRs with magic value */
    for(j = 0; j < mr_num; j++) {
        L4_LoadMR(j, 0xdeadbeef + j);
    }

    L4_MsgLoad(&msg);

    /* Wait for IPC */
    rcv_tag = L4_Ipc(L4_nilthread, main_thread, L4_MsgMsgTag(&msg), &from);

    while(1)
    {
        int fail = 0;
        /* Verify IPC receive correctly */
        L4_MsgStore(rcv_tag, &msg);

        if (i >= mr_num)
        {
            while (1)
            {
                fail_unless(getTagE(rcv_tag), "Error not returned");
                fail_unless(getECodeE(L4_ErrorCode()) == 4, "Overflow error code not delivered");
                fail_unless(getECodeP(L4_ErrorCode()) == 1, "Error in send phase");

                L4_MsgClear(&msg);
                L4_Set_MsgMsgTag(&msg, tag);
                L4_MsgLoad(&msg);

                /* Wait for IPC */
                rcv_tag = L4_Ipc(L4_nilthread, main_thread, L4_MsgMsgTag(&msg), &from);
            }
        }

        if (getTagE(rcv_tag))
            fail = 2;
        if (getTagU(rcv_tag) != i)
            fail = 3;
        if (L4_MsgLabel(&msg) != i*i)
            fail = 4;

        for(j = 0; j < ((i<mr_num)?i:mr_num); j++)
        {
            if (L4_MsgWord(&msg, j) != ((i*i)+(j*j)))
                fail = 5;
        }

        if (!fail)
        {
            /* Construct reply message */

            L4_MsgClear(&msg);
            L4_Set_MsgMsgTag(&msg, tag);
            L4_Set_MsgLabel(&msg, i*i);

            // LibL4 does not range check on messages(for performance), so do it ourselves to avoid
            // stuffing up the unit tests
            for (j = 0; j < ((i<(__L4_NUM_MRS-1))?i:(__L4_NUM_MRS-1)); j++)
                L4_MsgPutWord(&msg, j, ~((i*i)+(j*j)));
            msg.tag.X.u = (i > 63) ? 63 : i;
        }

        if (fail)
        {
            /* error case - receive from wrong thread - notify main_thread */
            L4_MsgClear(&msg);
            L4_Set_MsgMsgTag(&msg, tag);
            L4_Set_MsgLabel(&msg, -fail);
        }

        /* Fill MRs with magic value */
        for (j = 0; j < mr_num; j++) {
            L4_LoadMR(j, 0xdeadbeef + j);
        }
        /* Be carefull about loading Mrs - can't use L4_MsgLoad */
        L4_LoadMRs(0, (msg.tag.X.u > __L4_NUM_MRS) ? (__L4_NUM_MRS) : msg.tag.X.u+1, &msg.msg[0]);

        rcv_tag = L4_Ipc(main_thread, main_thread, L4_MsgMsgTag(&msg), &from);

        i++;
    }
}

static void blocking_receive_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t dummy;
    int i;

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, main_thread, tag, &dummy);
    }
}


/* 
\begin{test}{IPC0100}
  \TestDescription{IPC source and destination are nil thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented} 
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0100)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){

        tag = setMsgTagAndAcceptor(i);
        tag = L4_Ipc(L4_nilthread, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Wrong tag, input tag %s", tagIntToString(i));

        // Carl Says the following does not constitute a fail
        // fail_unless(from.raw == 0, "From TID not nilthread %s",
        //        tagIntToString(i));
    }
}
END_TEST

/*
\begin{test}{IPC0200}
  \TestDescription{IPC send to non-existant thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{1337401}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  mr0.E is set,
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 2 
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})} 
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0200)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){

        tag = setMsgTagAndAcceptor(i);

        tag = L4_Ipc(NONEXISTANT_TID, L4_nilthread, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 0, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 2 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));
    }
}
END_TEST

/*
\begin{test}{IPC0300}
  \TestDescription{IPC send to thread which is waiting}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{waiting thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag
    \end{enumerate}}
  \TestPermutedCases{srua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0300)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(waiting_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_P | BIT_N, 0))
            continue;

        // wait for other thread to block
        waitReceiving(partner);
        assert(checkReceiving(partner));

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Wrong tag returned, input tag %s", tagIntToString(i));
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC0400}
  \TestDescription{IPC non-blocking send to thread which is not waiting}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0.E is set,
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 1
    \end{enumerate}}
  \TestPermutedCases{rua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0400)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    L4_Word_t ErrorCode;

    int i;

    wait_tag.raw = TAG_NOBLOCK;
    partner = createThread(spinning_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_S | BIT_P | BIT_N, 0))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 0, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 1 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));

    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC0500}
  \TestDescription{IPC blocking send to thread which is not waiting}
  \TestFunctionalityTested{IPC, ExchangeRegisters}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}
    Examine thread with ExchangeRegisters to verify it is blocked on IPC
  }
  \TestPostConditions{\begin{enumerate}
    \item Thread is blocked on IPC
    \end{enumerate}}
  \TestPermutedCases{rua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0500)
{
    L4_ThreadId_t partner;
    int i;

    partner = createThread(blocking_send_thread);

    // keep i in sync with sending thread
    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_S | BIT_P | BIT_N, BIT_S))
            continue;

        waitSending(partner);

        _fail_unless(checkSending(partner), __FILE__, __LINE__, "Failed to block, input tag %s", tagIntToString(i));

        L4_Start(partner);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC0600}
  \TestDescription{IPC blocking send to thread which is not waiting,
    and then other thread does receive}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag 
    \end{enumerate}}
  \TestPermutedCases{rua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0600)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;

    int i;

    wait_tag.raw = TAG_SBLOCK;
    partner = createThread(waiting_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_N | BIT_P | BIT_S, BIT_S))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // make sure other thread is not receiving yet
        assert(!checkReceiving(partner));
        // test the IPC
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Wrong tag returned, input tag %s", tagIntToString(i));

    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC0700}
  \TestDescription{IPC receive from non-existant thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \myfromtid{13371401}
    \item \mytotid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from= nilthread, 
    \item mr0.E is set, 
    \item ErrorCode.p = 1, 
    \item ErrorCode.E = 2
    \end{enumerate}}
  \TestPermutedCases{srpnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0700)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){

        tag = setMsgTagAndAcceptor(i);

        tag = L4_Ipc(L4_nilthread, NONEXISTANT_TID, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 1, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 2 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));
    }
}
END_TEST

/*
\begin{test}{IPC0800}
  \TestDescription{IPC receive from thread that is blocked sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \myfromtid{sending thread}
    \item \mytotid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{srpnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0800)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    L4_Msg_t msg;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(special_sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){

        // first send of 0 MRS
        // run the partner until he is sending to us
        while(!checkSending(partner))
            L4_ThreadSwitch(partner);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        // second send of 10 MRS
        // run the partner until he is sending to us
        while(!checkSending(partner))
            L4_ThreadSwitch(partner);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC0900}
  \TestDescription{IPC non-blocking receive from thread that is not sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \myfromtid{sending thread}
    \item \mytotid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from=nilthread, 
    \item mr0.E is set, 
    \item ErrorCode.p = 1, 
    \item ErrorCode.e = 1
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC0900)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;
    L4_Word_t ErrorCode;

    partner = createThread(spinning_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if not-asynchronous,  non-blocking recieve
        if(!validTest(i, BIT_R , 0))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 1, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 1 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));

    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1000}
  \TestDescription{IPC blocking receive from thread that is not sending}
  \TestFunctionalityTested{IPC, ExchangeRegisters}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \myfromtid{sending thread}
    \item \mytotid{nilthread}
    \end{enumerate}
    Examine thread with ExchangeRegisters to verify it is blocked on IPC
    }
  \TestPostConditions{\begin{enumerate}
    \item Thread is blocked on IPC
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1000)
{
    L4_ThreadId_t partner;
    int i;

    partner = createThread(blocking_receive_thread);

    // keep i in sync with partner
    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        waitReceiving(partner);

        _fail_unless(checkReceiving(partner), __FILE__, __LINE__, "Failed to block, input tag %s", tagIntToString(i));

        L4_Start(partner);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1100}
  \TestDescription{IPC blocking receive from thread that is not sending, then sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \myfromtid{sending thread}
    \item \mytotid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0),
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1100)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    L4_Msg_t msg;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(special_sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_N is 0
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // check sending of 0 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);

        // make sure partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        // check sending of 10 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // make sure partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1200}
  \TestDescription{IPC receive from anythread that is sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{anythread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{srpnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1200)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    L4_Msg_t msg;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(special_sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){

        // first send of 0 MRS
        // run the partner until he is sending to us
        while(!checkSending(partner))
            L4_ThreadSwitch(partner);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        // second send of 10 MRS
        // run the partner until he is sending to us
        while(!checkSending(partner))
            L4_ThreadSwitch(partner);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1300}
  \TestDescription{IPC non-blocking receive from anythread that is not sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{anythread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  from=nilthread, 
    \item mr0.E is set, 
    \item ErrorCode.p = 1, 
    \item ErrorCode.e = 1
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1300)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i;
    L4_Word_t ErrorCode;

    partner = createThread(spinning_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if not-asynchronous,  non-blocking recieve
        if(!validTest(i, BIT_R , 0))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 1, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 1 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));

    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1400}
  \TestDescription{IPC blocking receive from anythread that is not sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{anythread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item Thread is blocked on IPC
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1400)
{
    L4_ThreadId_t partner;
    int i;

    partner = createThread(blocking_receive_thread);

    // keep i in sync with partner
    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        waitReceiving(partner);

        _fail_unless(checkReceiving(partner), __FILE__, __LINE__, "Failed to block, input tag %s", tagIntToString(i));

        L4_Start(partner);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1500}
  \TestDescription{IPC blocking receive from anythread that is not sending, then sending}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{anythread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1500)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;
    L4_Msg_t msg;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(special_sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_N is 0
        if(!validTest(i, BIT_R, BIT_R))
            continue;


        // first send of 0 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        // second send of 10 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, L4_anythread, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC1600}
  \TestDescription{IPC non-blocking receive only from specified thread, 
    another thread ready to send to thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{non sending thread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  from=nilthread, 
    \item mr0.E is set, 
    \item ErrorCode.p = 1, 
    \item ErrorCode.e = 1
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1600)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_ThreadId_t sender;
    L4_MsgTag_t tag;
    int i;
    L4_Word_t ErrorCode;
    sender = createThread(special_sending_thread);
    partner = createThread(spinning_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, 0))
            continue;

        // first send of 0 MRS
        // run the partner until he is sending to us
        while(!checkSending(sender))
            L4_ThreadSwitch(sender);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        ErrorCode = L4_ErrorCode();

        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 1, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 1 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));

    }


    deleteThread(partner);
    deleteThread(sender);
}
END_TEST

/*
\begin{test}{IPC1700}
  \TestDescription{IPC blocking receive only from specified thread, 
    another thread ready to send to thread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{non sending thread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item Thread is blocked on IPC
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1700)
{
    L4_ThreadId_t partner;
    int i;

    other_thread = createThread(other_sending_thread);
    partner = createThread(blocking_receive_thread);

    // keep i in sync with partner
    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // start the pending send
        waitSending(other_thread);
        assert(checkSending(other_thread));

        // run the thread we are testing for blocking
        waitReceiving(partner);

        _fail_unless(checkReceiving(partner), __FILE__, __LINE__, "Failed to block, input tag %s", tagIntToString(i));

        L4_Start(partner);
        L4_Start(other_thread);
    }


    deleteThread(partner);
    deleteThread(other_thread);
}
END_TEST

/*
\begin{test}{IPC1800}
  \TestDescription{IPC receive only from specified thread, 
    other thread sends after waiting thread blocks}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{non sending thread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item Thread is still blocked on IPC
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1800)
{
    L4_ThreadId_t partner;
    int i;

    other_thread = createThread(other_sending_thread);
    partner = createThread(blocking_receive_thread);

    //keep i in sync with partner
    for(i=0; i < NUM_PERMUTS; ++i){
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // run the thread we are testing for blocking
        waitReceiving(partner);
        assert(checkReceiving(partner));
        assert(!checkSending(other_thread));

        // start the send
        waitSending(other_thread);
        assert(checkSending(other_thread));

        _fail_unless(checkReceiving(partner), __FILE__, __LINE__, "Failed to block, input tag %s", tagIntToString(i));

        L4_Start(partner);
        L4_Start(other_thread);
    }

    deleteThread(partner);
    deleteThread(other_thread);
}
END_TEST

/*
\begin{test}{IPC1900}
  \TestDescription{IPC blocking receive only from specified thread, 
    another thread ready to send to thread then specified thread does send}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{non sending thread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC1900)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_ThreadId_t sender;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_MsgTag_t wait_tag;
    int i;

    wait_tag.raw = TAG_RBLOCK;
    sender = createThread(sending_thread);
    partner = createThread(special_sending_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_N, BIT_R is 1
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // first send of 0 MRS
        // run the partner until he is sending to us
        while(!checkSending(sender))
            L4_ThreadSwitch(sender);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        // check that we got message ok
        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        // now repeat for 10 MRS
        // run the partner until he is sending to us
        while(!checkSending(sender))
            L4_ThreadSwitch(sender);

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
    }


    deleteThread(partner);
    deleteThread(sender);
}
END_TEST

#ifdef CONFIG_STRICT_SCHEDULING
/*
\begin{test}{IPC2000}
  \TestDescription{IPC receive only from specified thread, 
    other thread sends after waiting thread blocks, then specifed thread does send}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{nilthread}
    \item \myfromtid{non sending thread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate (10 or 0), 
    \item tag.label, 
    \item MRs set correctly
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2000)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_ThreadId_t sender;
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;

    wait_tag.raw = TAG_RBLOCK;
    sender = createThread(sending_thread);
    partner = createThread(special_sending_thread);
    L4_Set_Priority(sender, 110);
    L4_Set_Priority(partner, 100);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_N, BIT_R is 1
        if(!validTest(i, BIT_R, BIT_R))
            continue;

        // Check for 0 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);

        // check sender has not started send yet
        assert(!checkSending(sender));
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        // check sender did actually send while we were blocked
        // (as in test description)
        assert(checkSending(sender));

        // check message ok
        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        _fail_unless(getTagU(tag) == 0, __FILE__, __LINE__, "Tag.u has wrong value, input tag %s", tagIntToString(i));
        _fail_unless(L4_MsgLabel(&msg) == MAGIC_LABEL1, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(i));

        L4_Start(sender);

        // repeat for 10 MRS

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);

        // check sender has not started send yet
        assert(!checkSending(sender));
        // check partner is not sending yet
        assert(!checkSending(partner));
        // test the IPC
        tag = L4_Ipc(L4_nilthread, partner, tag, &from);

        // check sender did actually send while we were blocked
        // (as in test description)
        assert(checkSending(sender));

        L4_MsgStore(tag, &msg);

        _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id, input tag %s", tagIntToString(i));
        _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned, input tag %s", tagIntToString(i));
        checkMessage(&msg, i);
        L4_Start(sender);
    }


    deleteThread(partner);
    deleteThread(sender);
}
END_TEST

#endif /* CONFIG_STRICT_SCHEDULING */

/*
\begin{test}{IPC2100}
  \TestDescription{IPC receive message with every possible number of MRs (0-63)}
  \TestImplementationProcess{
    For each xx where xx is from 0 to 64:
    \begin{enumerate}
    \item Receive an IPC 
    \item Check post conditions
    \end{enumerate}}
  \TestPostConditions{
    Correct number of MRs:,
    \begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 0, 
    \item tag.u as appropriate, 
    \item tag.label, 
    \item MRs set correctly 
    \end{enumerate}
    Overflow:
    \begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 1, 
    \item ErrorCode.E = 4, 
    \item ErrorCode.P = 1
    \end{enumerate}}
  \TestPermutedCases{spnua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2100)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    int i, j, p, mr_num;
    L4_Msg_t msg;

    for(p = 0; p < NUM_PERMUTS; p++)
    {
        if(!validTest(p, BIT_R, BIT_R))
            continue;
        partner = createThread(n_sending_thread);
        mr_num = IPC_NUM_MR;
        for(i=0; i < 64; ++i){
            // get our tag to test
            tag = setMsgTagAndAcceptor(p);
            // test the IPC
            tag = L4_Ipc(L4_nilthread, partner, tag, &from);

            if (i >= mr_num)
            {
                _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id %s", tagIntToString(p));
                _fail_unless(getTagE(tag), __FILE__, __LINE__, "Error not returned %s", tagIntToString(p));
                _fail_unless(getECodeE(L4_ErrorCode()) == 4, __FILE__, __LINE__, "Overflow error code not delivered %s", tagIntToString(p));
                _fail_unless(getECodeP(L4_ErrorCode()) == 1, __FILE__, __LINE__, "Error code contains phase field %s", tagIntToString(p));
            }
            else
            {
                L4_MsgStore(tag, &msg);
                _fail_unless(from.raw == lookupReplyHandle(partner).raw, __FILE__, __LINE__, "Wrong Thread-Id %s", tagIntToString(p));
                _fail_unless(getTagE(tag) == 0, __FILE__, __LINE__, "Error returned %s", tagIntToString(p));
                _fail_unless(getTagU(tag) == i, __FILE__, __LINE__, "Tag.u has wrong value %s", tagIntToString(p));
                _fail_unless(L4_MsgLabel(&msg) == i*i, __FILE__, __LINE__, "Tag has wrong label %s", tagIntToString(p));
                for(j = 0; j < ((i<mr_num)?i:mr_num); j++)
                {
                    _fail_unless(L4_MsgWord(&msg, j) == ((i*i)+(j*j)), __FILE__, __LINE__, "Message registers set with incorrect values %s", tagIntToString(p));
                }
            }
        }
        deleteThread(partner);
    }
}
END_TEST

/*
\begin{test}{IPC2200}
  \TestDescription{IPC send a message which overflows the message registers (63 MRS) 
    (will not overflow on architectures with 64 MRS)}
  \TestImplementationProcess{
    For each xx where xx is from 0 to 64:
    \begin{enumerate}
    \item Send an IPC 
    \item Check post conditions
    \end{enumerate}}
  \TestPostConditions{
    Correct number of MRs:
    \begin{enumerate}
    \item mr0 = niltag 
    \end{enumerate}
    Overflow:
    \begin{enumerate}
    \item from = sending thread, 
    \item tag.E = 1, 
    \item ErrorCode.E = 4, 
    \item ErrorCode.P = 0 
    \end{enumerate}}
  \TestPermutedCases{sra}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2200)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;
    int i;

    wait_tag.raw = TAG_RBLOCK;
    partner = createThread(waiting_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_U | BIT_P | BIT_N, 0))
            continue;

        // wait for other thread to begin wait
        waitReceiving(partner);
        assert(checkReceiving(partner));

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // set u to 63
        tag.raw |= 0x3f;
        // test the IPC
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        if (IPC_NUM_MR < 64)
        {
            _fail_unless(getTagE(tag), __FILE__, __LINE__, "Error not returned, %s", tagIntToString(i));
            _fail_unless(getECodeE(L4_ErrorCode()) == 4, __FILE__, __LINE__, "Overflow error code not delivered, %s", tagIntToString(i));
            _fail_unless(getECodeP(L4_ErrorCode()) == 0, __FILE__, __LINE__, "Error code contains phase field, %s", tagIntToString(i));
        }
        else
        {
            _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Wrong tag returned, %s", tagIntToString(i));
        }
    }

    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC2300}
  \TestDescription{IPC send to anythread}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{anythread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  mr0.E is set, 
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 2
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2300)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){

        tag = setMsgTagAndAcceptor(i);

        tag = L4_Ipc(L4_anythread, L4_nilthread, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 0, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 2 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));
    }
}
END_TEST

/*
\begin{test}{IPC2400}
  \TestDescription{IPC send to wait notify}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{waitnotify}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item  mr0.E is set, 
    \item ErrorCode.p = 0, 
    \item ErrorCode.e = 2
    \end{enumerate}}
  \TestPermutedCases{srnpua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2400)
{
    L4_ThreadId_t from;
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    int i;
    for(i=0; i < NUM_PERMUTS; ++i){

        tag = setMsgTagAndAcceptor(i);

        tag = L4_Ipc(L4_waitnotify, L4_nilthread, tag, &from);

        ErrorCode = L4_ErrorCode();
        fail_unless(getTagE(tag), "Failed to return error\n");
        _fail_unless(getECodeP(ErrorCode) == 0, __FILE__, __LINE__, "Wrong error p bit, from tag: %s", tagIntToString(i));
        _fail_unless(getECodeE(ErrorCode) == 2 , __FILE__, __LINE__, "Wrong error e bits, from tag: %s", tagIntToString(i));
    }
}
END_TEST

/*
\begin{test}{IPC2500}
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
/* Test MRs valid on send and receive */
START_TEST(IPC2500)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag, rcv_tag;
    L4_Msg_t msg;
    int i, j, mr_num;

    tag.raw = TAG_SBLOCK | TAG_RBLOCK;

    partner = createThread(ipc25_test_thread);
    // wait for other thread to begin wait
    waitReceiving(partner);
    assert(checkReceiving(partner));

    mr_num = IPC_NUM_MR;

    for (i=0; i < 64; ++i){
        /* Construct reply message */

        L4_MsgClear(&msg);
        L4_Set_MsgMsgTag(&msg, tag);
        L4_Set_MsgLabel(&msg, i*i);

        // LibL4 does not range check on messages(for performance), so do it ourselves to avoid
        // stuffing up the unit tests
        for (j = 0; j < ((i<(__L4_NUM_MRS-1))?i:(__L4_NUM_MRS-1)); j++)
            L4_MsgPutWord(&msg, j, ((i*i)+(j*j)));
        msg.tag.X.u = (i > 63) ? 63 : i;

        /* Fill MRs with magic value */
        for(j = 0; j < mr_num; j++) {
            L4_LoadMR(j, 0xbeefdead + j);
        }
        /* Be carefull about loading Mrs - can't use L4_MsgLoad */
        L4_LoadMRs(0, (msg.tag.X.u > __L4_NUM_MRS) ? (__L4_NUM_MRS) : msg.tag.X.u+1, &msg.msg[0]);

        // test the IPC
        rcv_tag = L4_Ipc(partner, partner, L4_MsgMsgTag(&msg), &from);

        if (i >= mr_num)
        {
            fail_unless(from.raw == lookupReplyHandle(partner).raw, "Wrong Thread-Id");
            fail_unless(getTagE(rcv_tag), "Error not returned");
            fail_unless(getECodeE(L4_ErrorCode()) == 4, "Overflow error code not delivered");
            fail_unless(getECodeP(L4_ErrorCode()) == 0, "Error in receive phase");
        }
        else
        {
            int fail;
            L4_MsgStore(rcv_tag, &msg);

#if defined(L4_64BIT)
#define LABEL_BITS  (16 + 32)
#else
#define LABEL_BITS  16
#endif

            fail = ((L4_Word_t)1 << LABEL_BITS) - L4_MsgLabel(&msg);

            if ((fail > 0) && (fail < 10))
            {
                fail_unless(fail != 1, "Partner: Wrong Thread-Id");
                fail_unless(fail != 2, "Partner: Error returned");
                fail_unless(fail != 3, "Partner: Tag.u has wrong value");
                fail_unless(fail != 4, "Partner: Tag has wrong label");
                fail_unless(fail != 5, "Partner: Message registers set with incorrect values");
            }

            fail_unless(from.raw == lookupReplyHandle(partner).raw, "Wrong Thread-Id");
            fail_unless(getTagE(rcv_tag) == 0, "Error returned");
            fail_unless(getTagU(rcv_tag) == i, "Tag.u has wrong value");
            fail_unless(L4_MsgLabel(&msg) == i*i, "Tag has wrong label");
            for (j = 0; j < ((i<mr_num)?i:mr_num); j++)
            {
                fail_unless(L4_MsgWord(&msg, j) == ~((i*i)+(j*j)), "Message registers set with incorrect values");
            }
        }
    }
    deleteThread(partner);
}
END_TEST

/*
\begin{test}{IPC2650}
  \TestDescription{Repeat of test IPC0600 with setting of old propagation flag
    to confirm that it has no effect}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \begin{enumerate}
    \item \mytotid{other thread}
    \item \myfromtid{nilthread}
    \end{enumerate}}
  \TestPostConditions{\begin{enumerate}
    \item mr0 = niltag 
    \end{enumerate}}
  \TestPermutedCases{rua}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2650)
{
    L4_ThreadId_t from;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_MsgTag_t wait_tag;

    int i;

    wait_tag.raw = TAG_SBLOCK;
    partner = createThread(waiting_thread);

    for(i=0; i < NUM_PERMUTS; ++i){
        // only do the test if BIT_P is 0
        if(!validTest(i, BIT_N | BIT_P | BIT_S, BIT_S))
            continue;

        // get our tag to test
        tag = setMsgTagAndAcceptor(i);
        // Add propagation flag
        tag.X.flags |= 1;
        // make sure other thread is not receiving yet
        assert(!checkReceiving(partner));
        // test the IPC
        tag = L4_Ipc(partner, L4_nilthread, tag, &from);

        _fail_unless(tag.raw == 0, __FILE__, __LINE__, "Wrong tag returned, input tag %s", tagIntToString(i));

    }

    deleteThread(partner);
}
END_TEST

static volatile L4_Word_t cnt0, cnt1, cnt2;

#define LOOPS   40

/* Note; because the thread ids and threads may or may not be used 
   depending on which scheduling algorithm is used, they cannot be declared 
   static or the restultant compiler warnings will stop compilation */
L4_ThreadId_t phigh, psender, pmiddle, psender2, preceiver, preceiver2, initialiser;

void prio_sending_thread(void);
void prio_sending2_thread(void);
void prio_sender_thread(void);
void prio_high_thread(void);
void prio_middle_thread(void);
void prio_receiver_thread(void);
void prio_receiver1_thread(void);
void prio_receiver2_thread(void);
void prio_receiver3_thread(void);

/* a thread that sends and receives IPCs (single IPC invocation) */
void prio_sending_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    int i;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for main thread
    L4_Ipc(L4_nilthread, main_thread, tag, &dummy);
    // Make sure preceiver has finished sending page fault to handler
    L4_Ipc(preceiver, L4_nilthread, tag, &dummy);

    // prime the test, to ensure that all pages have been faulted in.
    for (i = 0; i < 2; i++) {
        L4_Ipc(preceiver, preceiver2, tag, &dummy);
    }
    cnt0 = cnt1 = 0;

    // do loop
    for (i = 0; i < LOOPS; i++) {
        L4_Ipc(preceiver, preceiver2, tag, &dummy);
    }

    // done, send to main_thread
    L4_Ipc(main_thread, main_thread, tag, &dummy);
    for (;;) {}
}

/* a thread that sends and receives IPCs (two IPC invocations) */
void prio_sending2_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    int i;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for main thread
    L4_Ipc(L4_nilthread, main_thread, tag, &dummy);
    // Make sure preceiver has finished sending page fault to handler
    L4_Ipc(preceiver, L4_nilthread, tag, &dummy);

    // do loop
    for (i = 0; i < LOOPS; i++) {
        L4_Ipc(preceiver, L4_nilthread, tag, &dummy);
        L4_Ipc(L4_nilthread, preceiver2, tag, &dummy);
    }

    // done, send to main_thread
    L4_Ipc(main_thread, main_thread, tag, &dummy);
    for (;;) {}
}

/* a thread who sends IPCs */
void prio_sender_thread(void)
{
    L4_Word_t sender_num;

    ARCH_THREAD_INIT

    // wait for main thread
    L4_Receive(main_thread);

    // Read which sender the main thread tells us to be in order to set
    // the respective counter
    sender_num = L4_UserDefinedHandle();

    while(1){
        L4_Send(phigh);
        if (sender_num == 1) {
            cnt1 = 1;
        }
        if (sender_num == 2) {
            cnt2 = 1;
        }
    }
}

/* a thread of highest priority that receives and sends IPCs */
void prio_high_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;
    int i;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for main thread
    L4_Ipc(L4_nilthread, main_thread, tag, &dummy);

    // do loop
    for (i = 0; i < LOOPS; i++) {
        L4_Receive(initialiser);
        // A second Receive (in case initialiser was ready) to ensure the 2 other threads are ready
        L4_Receive(initialiser);
        cnt0 = cnt1 = cnt2 = 0;
        if (L4_IsThreadNotEqual(initialiser, psender2)) {
            L4_Receive(psender2);
        }
        L4_Ipc(preceiver, psender, tag, &dummy);
        fail_unless(!cnt0 && !cnt1 && !cnt2, "Priority not preserved");
    }

    // done, send to main_thread
    L4_Ipc(main_thread, main_thread, tag, &dummy);
    for (;;) {}
}

/* a thread in middle priority */
void prio_middle_thread(void)
{
    ARCH_THREAD_INIT

    while(1){
        cnt1 ++;
        L4_ThreadSwitch(preceiver);
        if (preceiver.raw != preceiver2.raw)
            L4_ThreadSwitch(preceiver2);
    }
}

/* a thread who replies to IPCs */
void prio_receiver_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for sender thread
    L4_Ipc(L4_nilthread, psender, tag, &dummy);
    cnt1 = 0;
    // wait for sender thread
    L4_Ipc(L4_nilthread, psender, tag, &dummy);

    while(1){
        cnt0 ++;
        L4_Ipc(psender, psender, tag, &dummy);
    }
}

/* a thread who replies to IPCs */
void prio_receiver1_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for sender thread
    L4_Ipc(L4_nilthread, psender, tag, &dummy);
    cnt1 = 0;
    // wait for sender thread
    L4_Ipc(L4_nilthread, psender, tag, &dummy);

    while(1){
        cnt0 ++;
        L4_Ipc(preceiver2, psender, tag, &dummy);
    }
}

/* a thread who replies to IPCs */
void prio_receiver2_thread(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag;

    ARCH_THREAD_INIT

    // send and receive both block
    tag.raw = TAG_SRBLOCK;

    // wait for sender thread
    L4_Ipc(L4_nilthread, preceiver, tag, &dummy);

    while(1){
        L4_Ipc(psender, preceiver, tag, &dummy);
    }
}

/* a thread who receives IPCs */
void prio_receiver3_thread(void)
{
    ARCH_THREAD_INIT

    while(1){
        L4_Receive(phigh);
        cnt0 = 1;
    }
}

#ifdef CONFIG_STRICT_SCHEDULING

/*
\begin{test}{IPC2700}
  \TestDescription{IPC priority inversion}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is run
    when blocking IPC from high to low priority.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2700)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    psender   = createThread(prio_sending_thread);
    pmiddle   = createThread(prio_middle_thread);
    preceiver = createThread(prio_receiver_thread);
    preceiver2 = preceiver;

    res = L4_Set_Priority(psender, 3);
    res = L4_Set_Priority(pmiddle, 2);
    res = L4_Set_Priority(preceiver, 1);

    res = L4_Set_Timeslice(psender, 0UL);
    res = L4_Set_Timeslice(pmiddle, 0UL);
    res = L4_Set_Timeslice(preceiver, 0UL);

    cnt0 = cnt1 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(psender, psender, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless(cnt0 == cnt1, "IPC priorities inverted");

    deleteThread(psender);
    deleteThread(pmiddle);
    deleteThread(preceiver);
}
END_TEST

#endif /* CONFIG_STRICT_SCHEDULING */

/*
\begin{test}{IPC2701}
  \TestDescription{IPC priority inversion}
  \TestImplementationProcess{
    This test is identical to the IPC2700 test except that the
    high priority thread performs a ReplyWait instead of a
    Call.
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is run
    when blocking IPC from high to low priority.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2701)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    psender   = createThread(prio_sending_thread);
    pmiddle   = createThread(prio_middle_thread);
    preceiver = createThread(prio_receiver_thread);
    preceiver2 = L4_anythread;

    res = L4_Set_Priority(psender, 3);
    res = L4_Set_Priority(pmiddle, 2);
    res = L4_Set_Priority(preceiver, 1);

    res = L4_Set_Timeslice(psender, 0UL);
    res = L4_Set_Timeslice(pmiddle, 0UL);
    res = L4_Set_Timeslice(preceiver, 0UL);

    cnt0 = cnt1 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(psender, psender, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless(cnt0 == cnt1, "IPC priorities inverted");

    deleteThread(psender);
    deleteThread(pmiddle);
    deleteThread(preceiver);
}
END_TEST

#ifdef CONFIG_STRICT_SCHEDULING
/*
\begin{test}{IPC2710}
  \TestDescription{IPC priority inversion}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is run
    when blocking IPC from high to low priority.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2710)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    psender   = createThread(prio_sending_thread);
    pmiddle   = createThread(prio_middle_thread);
    preceiver = createThread(prio_receiver1_thread);
    preceiver2 = createThread(prio_receiver2_thread);

    res = L4_Set_Priority(psender, 3);
    res = L4_Set_Priority(pmiddle, 2);
    res = L4_Set_Priority(preceiver, 1);
    res = L4_Set_Priority(preceiver2, 1);

    res = L4_Set_Timeslice(psender, 0UL);
    res = L4_Set_Timeslice(pmiddle, 0UL);
    res = L4_Set_Timeslice(preceiver, 0UL);
    res = L4_Set_Timeslice(preceiver2, 0UL);

    cnt0 = cnt1 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(psender, psender, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless(cnt0 == cnt1, "IPC priorities inverted");

    deleteThread(psender);
    deleteThread(pmiddle);
    deleteThread(preceiver);
    deleteThread(preceiver2);
}
END_TEST

/*
\begin{test}{IPC2800}
  \TestDescription{IPC priority preservation}
  \TestImplementationProcess{
    Atomic IPC send and receive between a thread and two 
    threads of lower priority both ready. Ensure the atomic IPC 
    is not interrupted by the partners and that the highest priority 
    thread continues running before all other threads are run.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal to zero}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2800)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    phigh     = createThread(prio_high_thread);
    preceiver = createThread(prio_receiver3_thread);
    psender   = createThread(prio_sender_thread);
    psender2  = createThread(prio_sender_thread);
    initialiser = psender2;

    L4_Set_UserDefinedHandleOf(psender, 1);
    L4_Set_UserDefinedHandleOf(psender2, 0);
    L4_Send(psender);
    L4_Send(psender2);

    res = L4_Set_Priority(phigh, 4);
    res = L4_Set_Priority(preceiver, 3);
    res = L4_Set_Priority(psender, 2);
    res = L4_Set_Priority(psender2, 1);

    cnt0 = cnt1 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(phigh, phigh, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(phigh);
    deleteThread(preceiver);
    deleteThread(psender);
    deleteThread(psender2);
}
END_TEST

/*
\begin{test}{IPC2801}
  \TestDescription{IPC priority preservation}
  \TestImplementationProcess{
    Atomic IPC send and receive between a thread and two 
    threads of lower priority both ready. A fourth intermediate priority
    thread is runnable. Ensure the atomic IPC is not interrupted by the 
    partners and that the highest priority thread continues running 
    before all other threads are run.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal to zero}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2801)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    phigh     = createThread(prio_high_thread);
    preceiver = createThread(prio_receiver3_thread);
    psender   = createThread(prio_sender_thread);
    psender2  = createThread(prio_sender_thread);
    initialiser  = createThread(prio_sender_thread);

    L4_Set_UserDefinedHandleOf(initialiser, 0);
    L4_Set_UserDefinedHandleOf(psender, 1);
    L4_Set_UserDefinedHandleOf(psender2, 2);
    L4_Send(initialiser);
    L4_Send(psender);
    L4_Send(psender2);

    res = L4_Set_Priority(phigh, 5);
    res = L4_Set_Priority(preceiver, 3);
    res = L4_Set_Priority(initialiser, 1);
    res = L4_Set_Priority(psender, 2);
    res = L4_Set_Priority(psender2, 4);

    cnt0 = cnt1 = cnt2 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(phigh, phigh, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(phigh);
    deleteThread(preceiver);
    deleteThread(psender);
    deleteThread(psender2);
    deleteThread(initialiser);
}
END_TEST

/*
\begin{test}{IPC2900}
  \TestDescription{IPC priority preservation}
  \TestImplementationProcess{
    This test is identical to the IPC2700 test except that the
    high priority thread performs 2 atomic IPCs operations (send
    and receive) instead of a Call.
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is run
    when blocking IPC from high to low priority.
    Counters used to check threads are running.}
  \TestPostConditions{Counters are equal}
  \TestPermutedCases{sr}{(see Table~\ref{table:ipcpermuted})}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC2900)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    psender   = createThread(prio_sending2_thread);
    pmiddle   = createThread(prio_middle_thread);
    preceiver = createThread(prio_receiver_thread);
    preceiver2 = preceiver;

    res = L4_Set_Priority(psender, 3);
    res = L4_Set_Priority(pmiddle, 2);
    res = L4_Set_Priority(preceiver, 1);

    res = L4_Set_Timeslice(psender, 0UL);
    res = L4_Set_Timeslice(pmiddle, 0UL);
    res = L4_Set_Timeslice(preceiver, 0UL);

    cnt0 = cnt1 = 0;

    tag.raw = TAG_SRBLOCK;
    tag = L4_Ipc(psender, psender, tag, &from);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless(cnt0 == cnt1, "IPC priorities inverted");

    deleteThread(psender);
    deleteThread(pmiddle);
    deleteThread(preceiver);
}
END_TEST

#endif /* CONFIG_STRICT_SCHEDULING */

extern L4_ThreadId_t test_tid;

/* IPC Reply Cap tests ---------------------------------------------------------*/

/*
\begin{test}{IPC3000}
  \TestDescription{IPC blocking send using an invalid forged thread handle}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Forge a thread handle,
    \item IPC send to the thread handle,
    \item Check it fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3000)
{
    L4_ThreadId_t thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    thread_handle.raw = 0x80000001;
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);
}
END_TEST

/*
\begin{test}{IPC3100}
  \TestDescription{IPC blocking send to not waiting thread using a valid thread handle}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Create a thread that does nothing,
    \item Use the thread handle to IPC send to the thread,
    \item Check it fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3100)
{
    L4_ThreadId_t tid, thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    tid = createThread(spinning_thread);
    L4_StoreMR(0, &thread_handle.raw);
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);

    deleteThread(tid);
}
END_TEST

/* a thread that calls main thread */
static void calling_thread(void)
{
    L4_LoadMR(0, 0);
    L4_Call(main_thread);

    while(1) ;
}

/*
\begin{test}{IPC3101}
  \TestDescription{IPC non blocking send to not waiting thread using a valid thread handle}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Create a thread that calls main thread,
    \item main thread replies twice using thread handle,
    \item Check the second reply fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3101)
{
    L4_ThreadId_t tid, thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    tid = createThread(calling_thread);
    L4_Wait(&thread_handle);
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);
    fail_unless(L4_IpcSucceeded(tag), "Failed to reply\n");
    tag = L4_Send(thread_handle);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);

    deleteThread(tid);
}
END_TEST

/* a thread that open waits */
static void waiting_any_thread(void)
{
    L4_ThreadId_t any_thread;

    L4_LoadMR(0, 0);
    L4_Call(main_thread);
    L4_Wait(&any_thread);

    while(1) ;
}

/*
\begin{test}{IPC3102}
  \TestDescription{IPC non blocking send to open waiting thread using a valid thread handle}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Create a thread that calls main thread and then open waits,
    \item main thread replies twice using thread handle,
    \item Check the second reply fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3102)
{
    L4_ThreadId_t tid, thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    tid = createThread(waiting_any_thread);
    L4_Wait(&thread_handle);
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);
    fail_unless(L4_IpcSucceeded(tag), "Failed to reply\n");
    /* Ensure thread is waiting */
    L4_Set_Priority(tid, 201);
    L4_Set_Priority(tid, 100);
    tag = L4_Send(thread_handle);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);

    deleteThread(tid);
}
END_TEST

/* a thread that waits notify */
static void waiting_notify_thread(void)
{
    L4_Word_t mask;

    L4_LoadMR(0, 0);
    L4_Call(main_thread);
    L4_Set_NotifyMask(0x1);
    L4_Accept(L4_NotifyMsgAcceptor);
    L4_WaitNotify(&mask);

    while(1) ;
}

/*
\begin{test}{IPC3200}
  \TestDescription{Async IPC notifying a thread waiting for notify using a valid thread handle}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Create a thread that calls main thread and then waits for notify,
    \item After main thread replied to the call it notifies using thread handle,
    \item Check the async notify fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3200)
{
    L4_ThreadId_t tid, thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    tid = createThread(waiting_notify_thread);
    L4_Wait(&thread_handle);
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);
    fail_unless(L4_IpcSucceeded(tag), "Failed to reply\n");
    /* Ensure thread is waiting */
    L4_Set_Priority(tid, 201);
    L4_Set_Priority(tid, 100);
    tag = L4_Notify(thread_handle, 0x1);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);

    deleteThread(tid);
}
END_TEST

/* a thread that closed waits to other thread */
static void other_waiting_thread(void)
{
    L4_Receive(other_thread);

    while(1) ;
}

/*
\begin{test}{IPC3300}
  \TestDescription{Use a valid thread handle to IPC blocking send to thread waiting for a third thread}
  \TestImplementationProcess{
  \begin{enumerate}
    \item Create a first thread T1 that does nothing,
    \item Create a second thread T2 that waits for T1,
    \item Use the thread handle to IPC send to T2,
    \item Check it fails.
  \end{enumerate}
  }
  \TestPostConditions{\begin{enumerate}
    \item IPC operation failed,
    \item Error Phase = send, 
    \item ErrorCode = 1. 
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC3300)
{
    L4_ThreadId_t tid, thread_handle;
    L4_MsgTag_t tag;
    L4_Bool_t phase;
    L4_Word_t ErrorCode, IpcErrorCode;

    other_thread = createThread(spinning_thread);
    tid = createThread(other_waiting_thread);
    /* Ensure thread is waiting */
//    L4_Set_Priority(tid, 201);
    L4_Set_Priority(tid, 100);
    L4_StoreMR(0, &thread_handle.raw);
    L4_LoadMR(0, 0);
    tag = L4_Send(thread_handle);

    ErrorCode = L4_ErrorCode();
    fail_unless(L4_IpcFailed(tag), "Failed to return error\n");
    phase = L4_IpcError(ErrorCode, &IpcErrorCode);
    fail_unless(phase == L4_ErrSendPhase, "Wrong error phase");
    _fail_unless(IpcErrorCode == L4_ErrTimeout, __FILE__, __LINE__,
                "Wrong IPC error code : %lu", IpcErrorCode);

    deleteThread(tid);
    deleteThread(other_thread);
}
END_TEST

/* Low priority server tests ------------------------------------------------*/

static void
ipc4000_server(void)
{
    L4_ThreadId_t reply_cap;
    L4_MsgTag_t result;

    /* Wait for initial send. */
    L4_Set_MsgTag(L4_Niltag);
    result = L4_Wait(&reply_cap);
    fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_Wait() failed.");

    for (;;) {
        /* At this point we perform some "work". */
#ifdef CONFIG_SCHEDULE_INHERITANCE
        fail_unless(get_current_threads_priority() == 200,
                "Server did not have expected priority.");
#endif

        /* ...and reply back to the client. */
        L4_Set_MsgTag(L4_Niltag);
        result = L4_ReplyWait(reply_cap, &reply_cap);
        fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_ReplyWait() failed.");
    }
}

/*
\begin{test}{IPC4000}
  \TestDescription{IPC Low-Priority server}
  \TestImplementationProcess{
    Have a low-priority server serving requests from a high-priority
    thread calling it.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC4000)
{
    int i;
    L4_ThreadId_t server;
    L4_MsgTag_t result;

    /* Start the server. */
    server = createThread(ipc4000_server);
    L4_Set_Priority(server, 1);

    /* Call the server ten times. */
    for (i = 0; i < 10; i++) {
        L4_Set_MsgTag(L4_Niltag);
        result = L4_Call(server);
        fail_unless(L4_IpcSucceeded(result), "high-priority client L4_Call() failed.");
    }

    deleteThread(server);
}
END_TEST

static void
ipc4001_server(void)
{
    L4_ThreadId_t dummy;
    L4_MsgTag_t result;

    /* Wait for initial send. */
    L4_Set_MsgTag(L4_Niltag);
    result = L4_Wait(&dummy);
    fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_Wait() failed.");

    for (;;) {
        /* At this point we perform some "work". */
#ifdef CONFIG_SCHEDULE_INHERITANCE
        fail_unless(get_current_threads_priority() == 1,
                "Server did not have expected priority.");
#endif

        /* ...and reply back to the client. */
        L4_Set_MsgTag(L4_Niltag);
        result = L4_ReplyWait(main_thread, &dummy);
        fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_ReplyWait() failed.");
    }
}

/*
\begin{test}{IPC4001}
  \TestDescription{IPC Low-Priority server}
  \TestImplementationProcess{
    Have a low-priority server serving requests from a high-priority
    thread open send/waiting to it.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPC4001)
{
    int i;
    L4_ThreadId_t server;
    L4_ThreadId_t dummy;
    L4_MsgTag_t result;

    /* Start the server. */
    server = createThread(ipc4001_server);
    L4_Set_Priority(server, 1);

    /* Send/OpenWait the server ten times. */
    for (i = 0; i < 10; i++) {
        L4_MsgTag_t tag = L4_Niltag;
        L4_Set_SendBlock(&tag);
        L4_Set_ReceiveBlock(&tag);
        result = L4_Ipc(server, L4_anythread, tag, &dummy);
        fail_unless(L4_IpcSucceeded(result), "high-priority client L4_Call() failed.");
    }

    deleteThread(server);
}
END_TEST

/* -----------------------------------------------------------------------------*/

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
make_ipc_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Ipc simple");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, IPC0100);
    tcase_add_test(tc, IPC0200);
    tcase_add_test(tc, IPC0300);
    tcase_add_test(tc, IPC0400);
    tcase_add_test(tc, IPC0500);
    tcase_add_test(tc, IPC0600);
    tcase_add_test(tc, IPC0700);
    tcase_add_test(tc, IPC0800);
    tcase_add_test(tc, IPC0900);
    tcase_add_test(tc, IPC1000);
    tcase_add_test(tc, IPC1100);
    tcase_add_test(tc, IPC1200);
    tcase_add_test(tc, IPC1300);
    tcase_add_test(tc, IPC1400);
    tcase_add_test(tc, IPC1500);
    tcase_add_test(tc, IPC1600);
    //tcase_add_test(tc, IPC1700);
    //tcase_add_test(tc, IPC1800);
    tcase_add_test(tc, IPC1900);
    tcase_add_test(tc, IPC2100);
    tcase_add_test(tc, IPC2200);
    tcase_add_test(tc, IPC2300);
    tcase_add_test(tc, IPC2400);
    tcase_add_test(tc, IPC2500);
    tcase_add_test(tc, IPC2701);
#ifdef CONFIG_STRICT_SCHEDULING
    tcase_add_test(tc, IPC2000);
    tcase_add_test(tc, IPC2700);
    tcase_add_test(tc, IPC2710);
    tcase_add_test(tc, IPC2800);
    tcase_add_test(tc, IPC2801);
    tcase_add_test(tc, IPC2900);
#endif
    tcase_add_test(tc, IPC3000);
    tcase_add_test(tc, IPC3100);
    tcase_add_test(tc, IPC3101);
    tcase_add_test(tc, IPC3102);
    tcase_add_test(tc, IPC3200);
    tcase_add_test(tc, IPC3300);
    tcase_add_test(tc, IPC4000);
    tcase_add_test(tc, IPC4001);
    
    return tc;
}

TCase *
make_xas_ipc_tcase(void)
{
    TCase *tc;

    initThreads(0);

    // the commented tests cannot run as they need to use Exreg on another A/S
    // when they are not in the roottask
    tc = tcase_create("Ipc simple XAS");
    tcase_add_checked_fixture(tc, xas_test_setup, xas_test_teardown);
    
    tcase_add_test(tc, IPC0100);
    tcase_add_test(tc, IPC0200);
    tcase_add_test(tc, IPC0300);
    tcase_add_test(tc, IPC0400);
    tcase_add_test(tc, IPC0500);
    tcase_add_test(tc, IPC0600);
    tcase_add_test(tc, IPC0700);
    tcase_add_test(tc, IPC0800);
    tcase_add_test(tc, IPC0900);
    tcase_add_test(tc, IPC1000);
    tcase_add_test(tc, IPC1100);
    tcase_add_test(tc, IPC1200);
    tcase_add_test(tc, IPC1300);
    tcase_add_test(tc, IPC1400);
    tcase_add_test(tc, IPC1500);
    tcase_add_test(tc, IPC1600);
    tcase_add_test(tc, IPC1700);
    tcase_add_test(tc, IPC1800);
    tcase_add_test(tc, IPC1900);
    tcase_add_test(tc, IPC2100);
    tcase_add_test(tc, IPC2200);
    tcase_add_test(tc, IPC2300);
    tcase_add_test(tc, IPC2400);
    tcase_add_test(tc, IPC2500);
    tcase_add_test(tc, IPC2650);
#ifdef CONFIG_STRICT_SCHEDULING
    tcase_add_test(tc, IPC2700);
    tcase_add_test(tc, IPC2710);
    tcase_add_test(tc, IPC2800);
    tcase_add_test(tc, IPC2801);
    tcase_add_test(tc, IPC2900);
#endif
    tcase_add_test(tc, IPC3000);
    tcase_add_test(tc, IPC3100);
    tcase_add_test(tc, IPC3101);
    tcase_add_test(tc, IPC3102);
    tcase_add_test(tc, IPC3200);
    tcase_add_test(tc, IPC3300);
    
    return tc;
}
