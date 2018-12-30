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

#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/kdebug.h>
#include <l4/mutex.h>

#include <stdio.h>
#include <stdlib.h>


/* Utility functions -------------------------------------------------------*/

#define LOOPS   20
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static L4_ThreadId_t main_thread, sender, receiver, middle, canceller, caller, high_prio, sender2, int_counter;
static volatile L4_Word_t cnt0, cnt1;
static volatile L4_Word_t g_started, g_counter, g_do_exreg;

/*
 * A thread that increments a counter.
 */
static void
counting_thread(void)
{
    /* If we get preempted, restart the loop. This gives the effect that all
     * our increments are atomic. */
    L4_Set_PreemptCallbackIP((L4_Word_t)counting_thread);
    L4_EnablePreemptionCallback();

    g_started = 1;
    while(1) {
        g_counter++; // increment the counter
    }
}

/*
 * A thread that makes a series of calls to a lower priority thread
 */
static void
calling_thread(L4_ThreadId_t myself)
{
    int i;
    L4_Word_t me = myself.raw;

    // wait for main thread
    L4_Receive(main_thread);
    if (sender.raw == me) {
        // Make sure receiver has finished sending page fault to handler
        L4_Send(receiver);
    }

    // do loop
    for (i = 0; i < LOOPS; i++) {
        if (sender.raw == me) 
            L4_Call(receiver);
        else
            L4_Call(middle);
    }

    // done, send to main_thread
    L4_Call(main_thread);
    for (;;) {}
}

/*
 * A thread that makes a series of replyWait to a lower priority thread
 */
static void
replying_waiting_thread(void)
{
    int i;
    L4_ThreadId_t any_thread;

    // wait for main thread
    L4_Receive(main_thread);
    // Make sure receiver has finished sending page fault to handler
    L4_Send(caller);

    L4_Receive(caller);
    any_thread = caller;
    cnt0 = 0;
    // do loop
    for (i = 0; i < 2 * LOOPS; i++) {
        L4_ReplyWait(any_thread, &any_thread);
    }

    // done, send to main_thread
    L4_Call(main_thread);
    for (;;) {}
}

/*
 * A thread that makes a series of sends to a lower priority thread
 */
static void
high_sending_thread(void)
{
    int i;

    // wait for main thread
    L4_Receive(main_thread);
    // Make sure receiver has finished sending page fault to handler
    L4_Send(receiver);

    // do loop
    for (i = 0; i < LOOPS; i++) {
        L4_Send(receiver);
    }

    // done, send to main_thread
    L4_Call(main_thread);
    for (;;) {}
}

/*
 * A thread that makes a series of receives from a lower priority thread
 */
static void
high_receiving_thread(void)
{
    int i;

    // wait for main thread
    L4_Receive(main_thread);
    // Make sure sender has finished sending page fault to handler
    L4_Send(sender);

    // do loop
    for (i = 0; i < LOOPS; i++) {
        L4_Receive(sender);
    }

    // done, send to main_thread
    L4_Call(main_thread);
    for (;;) {}
}

/*
 * A thread that receives from a lower priority thread
 */
static void
high_single_receiving_thread(void)
{
    L4_MsgTag_t tag = L4_Niltag;
    L4_Receive(middle);

    // done, send to main_thread
    L4_Set_Label(&tag, 0x333);
    L4_Set_MsgTag(tag);
    L4_Call(main_thread);
    for (;;) {}
}

/*
 * A thread that receives from a higher priority thread and replies to it
 */
static void
receiving_replying_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t i;

    // wait for sender thread
    tag = L4_Receive(sender);
    if (L4_IpcFailed(tag)) {
        printf("IPC failed\n");
    }
    cnt1 = 0;

    while(1){
        cnt0 ++;
        tag = L4_Receive(sender);
        if (L4_IpcFailed(tag)) {
            printf("IPC failed\n");
        }
        for (i = 0; i < (g_counter * 2); i++) { }
        L4_Send(sender);
    }
}

/*
 * A thread that calls a higher priority thread
 */
static void
low_calling_thread(void)
{
    L4_Receive(high_prio);
    cnt1 = 0;

    while (1) {
        cnt0++;
        L4_Call(high_prio);
    }
}

/*
 * A thread that receives from a higher priority thread
 */
static void
low_receiving_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t i;

    // wait for sender thread
    tag = L4_Receive(sender);
    if (L4_IpcFailed(tag)) {
        printf("IPC failed\n");
    }
    cnt1 = 0;

    while(1){
        cnt0 ++;
        for (i = 0; i < (g_counter * 2); i++) { }
        tag = L4_Receive(sender);
        if (L4_IpcFailed(tag)) {
            printf("IPC failed\n");
        }
    }
}

/*
 * A thread that deletes its sender
 */
static void
low_deleting_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t i;

    // wait for sender thread
    tag = L4_Receive(sender);
    if (L4_IpcFailed(tag)) {
        printf("IPC failed\n");
    }

    // perform work for two timeslices
    for (i = 0; i < (g_counter * 2); i++) { }

    // delete the sender
    deleteThread(sender);

    // increment the counter to show we have been scheduled
    cnt0++;

    // call back main
    L4_Send(main_thread);
}

/*
 * A thread that sends to a higher priority thread
 */
static void
low_sending_thread(void)
{
    L4_MsgTag_t tag;
    L4_Word_t i;

    // wait for receiver thread
    tag = L4_Receive(receiver);
    if (L4_IpcFailed(tag)) {
        printf("IPC failed\n");
    }
    cnt1 = 0;

    while(1){
        cnt0 ++;
        for (i = 0; i < (g_counter * 2); i++) { }
        L4_LoadMR(0, 0x12340000);
        tag = L4_Send(receiver);
        if (L4_IpcFailed(tag)) {
            printf("IPC failed\n");
        }
    }
}

/*
 * A thread that sends only once to a higher priority thread
 */
static void
low_single_sending_thread(void)
{
    L4_MsgTag_t tag = L4_Niltag;
    L4_Send(middle);

    // done, send to main_thread
    L4_Set_Label(&tag, 0x111);
    L4_Set_MsgTag(tag);
    L4_Call(main_thread);
    for (;;) {}
}

/* A thread in middle priority */
static void 
middle_thread(void)
{
    while(1){
        cnt1 ++;
    }
}

static void 
middle_switching_thread(void)
{
    while(1){
        cnt1 ++;
        L4_ThreadSwitch(caller);
    }
}

/* A thread with middle priority that sends back to the main thread */
static void
middle_callback_thread(void)
{
    while (1) {
        cnt1++;
        L4_Send(main_thread);
    }
}

/*
 * A thread that receives from one thread, calls/sends another thread before replying
 * to the first one.
 */
static void
middle_calling_thread(L4_ThreadId_t myself)
{
    L4_Word_t me = myself.raw;

    if (sender.raw == me)
        L4_Send(receiver);

    while (1) {
        L4_Receive(caller);
        if (sender.raw == me)
            L4_Send(receiver);
        else
            L4_Call(receiver);
        L4_Send(caller);
    }
}

/*
 * A thread that performs an atomic IPC send to a higher thread and receives
 * from a lower thread.
 */
static void
middle_okl4_atomic_ipc_thread(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t dummy;

    // wait for main thread
    L4_Receive(main_thread);
    
    L4_LoadMR(0, 0);
    tag = L4_Niltag;
    L4_Set_Label(&tag, 0x222);
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgTag(tag);
    L4_Ipc(receiver, sender, tag, &dummy);
    
    L4_WaitForever();
}

/*
 * A thread that receives from a thread and waits for any thread before replying
 * to the first thread.
 */
static void
open_waiting_thread(void)
{
    L4_ThreadId_t any_thread;
    L4_MsgTag_t tag;

    L4_Send(sender);

    while (1) {
        L4_Receive(middle);
        tag = L4_Wait(&any_thread);
        fail_unless(L4_Label(tag) == 0x1234, "Received from wrong thread");
        L4_Send(middle);
    }
}

/*
 * A thread that cancels IPC between 2 other threads
 */
static void
exreg_thread(void)
{
    L4_Word_t dummy, control;
    L4_ThreadId_t r, dummy_tid;

    while (1) {
        L4_Wait(&dummy_tid);
        g_do_exreg = 1;
        r = L4_ExchangeRegisters(high_prio, L4_ExReg_AbortIPC, 0, 0, 0, 0, L4_nilthread, &control, &dummy, &dummy, &dummy, &dummy, &dummy_tid);
    }
}

/*
 * A thread that spins around and triggers the canceller thread
 */
static void
spinning_triggering_thread(void)
{
    // wait for high_prio thread
    L4_Receive(high_prio);

    while (1) {
        if (g_do_exreg) {
            g_do_exreg = 0;
            L4_Send(canceller);
        }
    }
}

/* A thread that spins around */
static void 
spinning_counting_thread(void)
{
    L4_ThreadId_t partner = L4_nilthread;

    partner.raw = L4_UserDefinedHandle();
    L4_Receive(partner);

    while(1){
        if (partner.raw == sender.raw)
            cnt0 ++;
        else
            cnt1++;
    }
}

/* A thread that does nothing but spin. */
static void
simple_spinning_thread(void)
{
    for (;;);
}

/* Wait for any IPC, then return control back to main. */
static void
simple_receiving_thread(void)
{
    int i;
    L4_ThreadId_t tid;
    L4_Wait(&tid);

    // Perform some work
    for (i = 0; i < (g_counter * 2); i++) { }

    // done, send to main_thread
    L4_Call(main_thread);
    for (;;) {}
}

/* A thread that causes an exception. */
static void
faulting_thread(void)
{
    /* Wait for parent thread to start us. */
    L4_ThreadId_t any_thread;
    L4_Wait(&any_thread);

    /* Generate fault */
    trigger_cpu_fault();

    /* Should not be reached. */
    fail("Platform could not generate a fault.");
    L4_WaitForever();
}

/*
 * A thread that does a single IPC send to another thread
 */
static void
single_sending_thread(void)
{
    L4_ThreadId_t partner = L4_nilthread;

    partner.raw = L4_UserDefinedHandle();
    if (int_counter.raw == main_thread.raw)
        cnt0++;
    // Make sure receiver has finished sending page fault to handler
    L4_Send(partner);
    // notify interrupt counter thread we're ready and and wait for the go
    if (int_counter.raw == main_thread.raw)
        L4_Call(main_thread);
    else
        L4_Send(int_counter);
    while (!g_started) ;

    L4_Send(partner);

    L4_WaitForever();
}

/*
 * The thread that counts the number of ticks for the timeslice
 * expiration test.
 */
static void
interrupt_counter_thread(void)
{
    int i;

    L4_Receive(main_thread);
    L4_Receive(sender);
    L4_Receive(sender2);
    g_started = 1;

    for (i = 0; i < 10; i++) {
        L4_Yield();
    }

    L4_Call(main_thread);
    for (;;) {}
}

static void
blocking_send_thread(void)
{
    L4_ThreadId_t partner = L4_nilthread;

    partner.raw = L4_UserDefinedHandle();
    cnt0++;
    L4_Send(partner);

    L4_WaitForever();
}

static void
do_nothing_thread(void)
{
    // Notify main thread we're ready
    L4_Receive(main_thread);
    while (cnt0 < g_started) ;
    L4_Call(main_thread);

    for (;;) {}
}

/*
 * Compute a counter representing roughly a time tick.
 */
static void
compute_counter(void)
{
    L4_ThreadId_t new_cnt_thread;
    L4_Word_t result;

    g_started = g_counter = 0;
    new_cnt_thread = createThread(counting_thread);
    /*
     * We set the time slice to the MinTimeQuantum so we might get a counter
     * that roughly represents the tick length.
     */
    result = L4_Set_Timeslice(new_cnt_thread, 1);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to set Time slice");
    result = L4_Set_Priority(new_cnt_thread, 200);
    if (g_started)
        g_counter = 0;
    while (g_counter == 0) ;
    deleteThread(new_cnt_thread);
}



/* IPC schedule inheritance tests --------------------------------------------*/

/*
\begin{test}{IPCSI0100}
  \TestDescription{IPC schedule inheritance on Call}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is not run
    when blocking IPC call from high to low priority.
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0100)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    sender   = createThread((void (*)(void))calling_thread);
    middle   = createThread(middle_thread);
    receiver = createThread(receiving_replying_thread);

    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(receiver, 1);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(sender);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless((cnt0 == LOOPS) && (cnt1 == 0), "IPC schedule inheritance failed");

    deleteThread(sender);
    deleteThread(middle);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI0101}
  \TestDescription{IPC schedule inheritance on ReplyWait}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread run
    when blocking IPC ReplyWait from high to low priority.
    Counters used to check threads are running.
  }
  \TestPostConditions{Counters are equal}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0101)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    high_prio  = createThread(replying_waiting_thread);
    middle   = createThread(middle_switching_thread);
    caller   = createThread(low_calling_thread);

    res = L4_Set_Priority(high_prio, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(caller, 1);

    res = L4_Set_Timeslice(high_prio, 0UL);
    res = L4_Set_Timeslice(middle, 0UL);
    res = L4_Set_Timeslice(caller, 0UL);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(high_prio);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless(cnt0 == cnt1, "IPC schedule inheritance failed");

    deleteThread(high_prio);
    deleteThread(middle);
    deleteThread(caller);
}
END_TEST

/*
\begin{test}{IPCSI0102}
  \TestDescription{IPC schedule inheritance on Send}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is not run
    when blocking IPC send from high to low priority.
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0102)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    sender   = createThread(high_sending_thread);
    middle   = createThread(middle_thread);
    receiver = createThread(low_receiving_thread);

    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(receiver, 1);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(sender);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless((cnt0 == LOOPS) && (cnt1 == 0), "IPC schedule inheritance failed");

    deleteThread(sender);
    deleteThread(middle);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI0103}
  \TestDescription{IPC schedule inheritance on Receive}
  \TestImplementationProcess{
    IPC between two threads with an intermediate priority
    thread. Ensure intermediate priority thread is not run
    when blocking IPC receive from high to low priority.
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0103)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    receiver   = createThread(high_receiving_thread);
    middle   = createThread(middle_thread);
    sender = createThread(low_sending_thread);

    res = L4_Set_Priority(receiver, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(sender, 1);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(receiver);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless((cnt0 == LOOPS) && (cnt1 == 0), "IPC schedule inheritance failed");

    deleteThread(sender);
    deleteThread(middle);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI0200}
  \TestDescription{IPC schedule inheritance on cancelled Call}
  \TestImplementationProcess{
    IPC between two threads with a third thread who cancels 
    the operation. 
    Ensure the IPC cancellation removes the dependency chain 
    when blocking IPC call from high to low priority.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0200)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    high_prio   = createThread((void (*)(void))calling_thread);
    canceller   = createThread(exreg_thread);
    receiver = createThread(spinning_triggering_thread);
    sender = high_prio;

    res = L4_Set_Priority(high_prio, 2);
    res = L4_Set_Priority(canceller, 2);
    res = L4_Set_Priority(receiver, 1);

    g_do_exreg = 1;

    L4_LoadMR(0, 0);
    tag = L4_Send(high_prio);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(high_prio);
    deleteThread(canceller);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI0201}
  \TestDescription{IPC schedule inheritance on cancelled Send}
  \TestImplementationProcess{
    IPC between two threads with a third thread who cancels 
    the operation. 
    Ensure the IPC cancellation removes the dependency chain 
    when blocking IPC send from high to low priority.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0201)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    high_prio   = createThread(high_sending_thread);
    canceller   = createThread(exreg_thread);
    receiver = createThread(spinning_triggering_thread);

    res = L4_Set_Priority(high_prio, 2);
    res = L4_Set_Priority(canceller, 2);
    res = L4_Set_Priority(receiver, 1);

    g_do_exreg = 1;

    L4_LoadMR(0, 0);
    tag = L4_Send(high_prio);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(high_prio);
    deleteThread(canceller);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI0202}
  \TestDescription{IPC schedule inheritance on cancelled Receive}
  \TestImplementationProcess{
    IPC between two threads with a third thread who cancels 
    the operation. 
    Ensure the IPC cancellation removes the dependency chain 
    when blocking IPC receive from high to low priority.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0202)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    high_prio   = createThread(high_receiving_thread);
    canceller  = createThread(exreg_thread);
    sender = createThread(spinning_triggering_thread);

    res = L4_Set_Priority(high_prio, 2);
    res = L4_Set_Priority(canceller, 2);
    res = L4_Set_Priority(sender, 1);

    g_do_exreg = 1;

    L4_LoadMR(0, 0);
    tag = L4_Send(high_prio);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(high_prio);
    deleteThread(canceller);
    deleteThread(sender);
}
END_TEST

/*
\begin{test}{IPCSI0300}
  \TestDescription{IPC schedule inheritance failure}
  \TestImplementationProcess{
    A high priority thread calls a lower priority thread which 
    also calls another lower priority thread which does an open 
    wait. A fourth thread sends to the open waiting thread allowing
    it to continue.
    Ensure fourth thread is scheduled and thus avoid the test to hang.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0300)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t dummy;

    caller   = createThread((void (*)(void))calling_thread);
    middle   = createThread((void (*)(void))middle_calling_thread);
    receiver = createThread(open_waiting_thread);
    sender = createThread(low_sending_thread);

    res = L4_Set_Priority(caller, 4);
    res = L4_Set_Priority(middle, 3);
    res = L4_Set_Priority(receiver, 2);
    res = L4_Set_Priority(sender, 1);

    L4_LoadMR(0, 0);
    tag = L4_Send(caller);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(caller);
    deleteThread(middle);
    deleteThread(receiver);
    deleteThread(sender);
}
END_TEST

/*
\begin{test}{IPCSI0400}
  \TestDescription{IPC nested schedule inheritance}
  \TestImplementationProcess{
    A high priority thread calls a lower priority thread which 
    sends to a higher priority thread which does some work prior 
    to receiving. A fourth thread of intermediate priority.
    Ensure intermediate priority thread is never run.
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0400)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t intermediate;
    L4_ThreadId_t dummy;

    caller   = createThread((void (*)(void))calling_thread);
    middle = createThread((void (*)(void))middle_calling_thread);
    intermediate = createThread(middle_thread);
    receiver = createThread(low_receiving_thread);
    sender = middle;

    res = L4_Set_Priority(caller, 4);
    res = L4_Set_Priority(intermediate, 3);
    res = L4_Set_Priority(receiver, 2);
    res = L4_Set_Priority(sender, 1);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(caller);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless((cnt0 == LOOPS) && (cnt1 == 0), "IPC schedule inheritance failed");

    deleteThread(caller);
    deleteThread(intermediate);
    deleteThread(receiver);
    deleteThread(sender);
}
END_TEST

/*
\begin{test}{IPCSI0500}
  \TestDescription{IPC timeslice donation with schedule inheritance}
  \TestImplementationProcess{
    Two high priority threads send to two lower priority threads. 
    The two lower priority threads have different priorities.
    Counters used to check threads are running and assess the 
    amount of work done.
  }
  \TestPostConditions{Counters difference is less than two percent}
  \TestPassStatus{does not pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0500)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_Word_t diff, limit;
    L4_ThreadId_t receiver2;
    L4_ThreadId_t dummy;

    sender    = createThread(single_sending_thread);
    sender2   = createThread(single_sending_thread);
    receiver  = createThread(spinning_counting_thread);
    receiver2 = createThread(spinning_counting_thread);
    int_counter = createThread(interrupt_counter_thread);

    L4_Set_UserDefinedHandleOf(sender, receiver.raw);
    L4_Set_UserDefinedHandleOf(sender2, receiver2.raw);
    L4_Set_UserDefinedHandleOf(receiver, sender.raw);
    L4_Set_UserDefinedHandleOf(receiver2, sender2.raw);

    L4_Set_Timeslice(sender, 10000);
    L4_Set_Timeslice(sender2, 10000);
    L4_Set_Timeslice(receiver, 5000);
    L4_Set_Timeslice(receiver2, 20000);

    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(sender2, 3);
    res = L4_Set_Priority(int_counter, 3);
    res = L4_Set_Priority(receiver, 2);
    res = L4_Set_Priority(receiver2, 1);

    g_started = 0;
    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Send(int_counter);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    diff = abs((long)cnt0 - cnt1);
    limit = (MAX(cnt0, cnt1) * 2) / 100;
    printf("counter 0 = %ld\n", cnt0);
    printf("counter 1 = %ld\n", cnt1);
    printf("difference is %ld => %ld%%, limit is %ld\n", diff, (diff * 100) / MAX(cnt0, cnt1), limit);
    fail_unless(diff <= limit, "Timeslice is not equally shared");

    deleteThread(sender);
    deleteThread(sender2);
    deleteThread(int_counter);
    deleteThread(receiver);
    deleteThread(receiver2);
}
END_TEST

/*
\begin{test}{IPCSI0600}
  \TestDescription{IPC dead-lock detection with schedule inheritance}
  \TestImplementationProcess{
    A high priority thread sends to a middle priority thread which sends to
    a lower priority thread. The third thread sends back to the high priority
    thread creating a depedency loop.
    A fourth runnable thread busy-waits and should be scheduled.
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0600)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_ThreadId_t do_nothing;
    L4_ThreadId_t dummy;

    high_prio  = createThread(blocking_send_thread);
    sender     = createThread(blocking_send_thread);
    sender2    = createThread(blocking_send_thread);
    do_nothing = createThread(do_nothing_thread);

    L4_Set_UserDefinedHandleOf(high_prio, sender.raw);
    L4_Set_UserDefinedHandleOf(sender, sender2.raw);
    L4_Set_UserDefinedHandleOf(sender2, high_prio.raw);

    res = L4_Set_Priority(high_prio, 4);
    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(sender2, 2);
    res = L4_Set_Priority(do_nothing, 1);

    cnt0 = 0;
    g_started = 3;

    L4_LoadMR(0, 0);
    tag = L4_Send(do_nothing);
    L4_Wait(&dummy);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    deleteThread(high_prio);
    deleteThread(sender);
    deleteThread(sender2);
    deleteThread(do_nothing);
}
END_TEST

/*
\begin{test}{IPCSI0900}
  \TestDescription{Delete inheriting thread}
  \TestImplementationProcess{
    A thread with an inherited priority deletes the thread whose
    priority is being inherited. A thread with a priority between the
    two threads should be scheduled after the high-priority inheriting
    thread is deleted.
  }
  \TestPostConditions{The middle priority thread is scheduled.}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI0900)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;

    sender   = createThreadInSpace(L4_rootspace, high_sending_thread);
    middle   = createThreadInSpace(L4_rootspace, middle_callback_thread);
    receiver = createThreadInSpace(L4_rootspace, low_deleting_thread);

    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(receiver, 1);

    cnt0 = cnt1 = 0;

    // send to sender
    L4_LoadMR(0, 0);
    tag = L4_Send(sender);
    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    // wait for reply from middle callback thread
    tag = L4_Receive(middle);
    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    // Ensure the low thread has not been scheduled after the delete
    fail_unless(cnt0 == 0, "Low priority thread scheduled after inheritor deleted.");

    // Wait for the low thread to finish their delete of the high thread
    tag = L4_Receive(receiver);

    deleteThread(middle);
    deleteThread(receiver);
}
END_TEST

/*
\begin{test}{IPCSI1000}
  \TestDescription{Ensure correct schedule inheritance after atomic IPC}
  \TestImplementationProcess{
    A low priority thread is blocked sending to a middle priority thread.
    A high priority thread is blocked receiving on the same middle priority thread.
    The thread with a priority between the two threads is scheduled and perform an 
    atomic IPC send to the high priority thread and receive from the low priority thread.
    After the atomic IPC is performed the high priority thread should be scheduled before
    the low priority thread is run.
  }
  \TestPostConditions{The high priority thread is scheduled.}
  \TestPassStatus{does not pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI1000)
{
    L4_MsgTag_t tag;
    L4_Word_t res;
    L4_ThreadId_t any_thread = L4_nilthread;

    receiver = createThread(high_single_receiving_thread);
    middle   = createThread(middle_okl4_atomic_ipc_thread);
    sender   = createThread(low_single_sending_thread);
    
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(sender, 201);
    // Sender will be scheduled
    res = L4_Set_Priority(sender, 1);
    res = L4_Set_Priority(receiver, 201);
    // Receiver will be scheduled
    res = L4_Set_Priority(receiver, 3);

    L4_LoadMR(0, 0);
    tag = L4_Send(middle);
    tag = L4_Wait(&any_thread);

    fail_unless(L4_Label(tag) == 0x333, "Priority inverted");

    deleteThread(receiver);
    deleteThread(middle);
    deleteThread(sender);
}
END_TEST

/*
\begin{test}{IPCSI1100}
  \TestDescription{Ensure correct schedule inheritance after exception}
  \TestImplementationProcess{
    A low priority thread is a pager for a high priority thread.
    The high priority thread hits an exception.
    A medium priority thread spins busy.}
  \TestPostConditions{The low priority thread is scheduled.}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(IPCSI1100)
{
    L4_MsgTag_t tag;
    L4_Word_t res;
    L4_ThreadId_t any_thread = L4_nilthread;

    sender   = createThread(faulting_thread);
    middle   = createThread(simple_spinning_thread);
    receiver = createThread(simple_receiving_thread);

    res = L4_Set_Priority(sender, 3);
    res = L4_Set_Priority(middle, 2);
    res = L4_Set_Priority(receiver, 201);
    // 'low' will be scheduled
    res = L4_Set_Priority(receiver, 1);

    res = L4_Set_Timeslice(receiver, 1);

    // setup 'low' as the exception handler of 'high'.
    res = L4_Set_ExceptionHandlerOf(sender, receiver);

    L4_LoadMR(0, 0);
    tag = L4_Send(sender);
    tag = L4_Wait(&any_thread);

    deleteThread(sender);
    deleteThread(middle);
    deleteThread(receiver);
}
END_TEST


/* MUTEX schedule inheritance tests --------------------------------------------*/

static L4_ThreadId_t high_prio_thread, middle_prio3_thread, middle_prio2_thread, low_prio_thread, middle_prio_thread;
static void
high_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;
    L4_ThreadId_t partner = L4_nilthread;
    int i;

    L4_Receive(main_thread);
    partner.raw = L4_UserDefinedHandle();
    for (i = 0; i < LOOPS; i++) {
        L4_Receive(partner);
        res = L4_Lock(m);
        fail_unless(res == 1, "L4_Lock() failed");
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }

    L4_Call(main_thread);
    L4_WaitForever();
}

static void
locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;
    L4_ThreadId_t partner = L4_nilthread;

    partner.raw = L4_UserDefinedHandle();
    cnt1 = 0;
    while (1) {
        res = L4_Lock(m);
        fail_unless(res == 1, "L4_Lock() failed");
        L4_LoadMR(0, 0);
        L4_Send(partner);
        cnt0++;
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }
}

static void
mutex02xx_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;
    L4_ThreadId_t partner = L4_nilthread;
    L4_MsgTag_t tag = L4_Niltag;

    tag = L4_Receive(main_thread);
    partner.raw = L4_UserDefinedHandle();
    L4_Set_MsgTag(tag);
    L4_Send(partner);
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    res = L4_Unlock(m);
    fail_unless(res == 1, "L4_Unlock() failed");

    L4_Call(main_thread);
    L4_WaitForever();
}

static void
deleting_thread(void)
{
    L4_ThreadId_t dummy_tid;
    L4_MsgTag_t tag = L4_Niltag;
    L4_Word_t label;

    tag = L4_Wait(&dummy_tid);
    label = L4_Label(tag);
    if (label == 0x2) {
        deleteThread(low_prio_thread);
        cnt0++;
    }
    L4_Call(main_thread);

    L4_WaitForever();
}

/*
 * A thread that locks a mutex and triggers the canceller thread
 */
static void
locking_triggering_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res, label;
    L4_ThreadId_t partner = L4_nilthread;
    L4_MsgTag_t tag = L4_Niltag;

    partner.raw = L4_UserDefinedHandle();
    res = L4_Lock(m);
    fail_unless(res == 1, "L4_Lock() failed");
    tag = L4_Receive(partner);
    label = L4_Label(tag);
    if (label == 0x1) {
        L4_Set_MsgTag(tag);
        L4_Send(canceller);
        deleteThread(partner);
    }
    if (label == 0x2) {
        L4_Set_MsgTag(tag);
        L4_Call(canceller);
    }

    cnt1++;
    L4_Send(main_thread);
    L4_WaitForever();
}

static L4_Word_t flag[3];

static void
mutex0300_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res, nb;
    L4_ThreadId_t dummy_thread;
    L4_Word_t i, j;

    nb = L4_UserDefinedHandle();
    if (nb == 0)
        L4_Receive(main_thread);
    for (i = 0; i < LOOPS; i++) {
        L4_Wait(&dummy_thread);
        flag[nb] = 0;
        res = L4_Lock(m);
        flag[nb] = 1;
        fail_unless(res == 1, "L4_Lock() failed");
        for (j = 0; j <= 2; j++) {
            if (j > nb)
                fail_unless(flag[j] == 0, "Priority order in nested mutex is not respected");
            else
                fail_unless(flag[j] == 1, "Priority order in nested mutex is not respected");
        }
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }

    if (nb == 0)
        L4_Wait(&dummy_thread);
    if (nb == 2) {
        L4_LoadMR(0, 0);
        L4_Send(high_prio_thread);
    }
    L4_Call(main_thread);
    L4_WaitForever();
}

static void
low_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    while (1) {
        res = L4_Lock(m);
        fail_unless(res == 1, "L4_Lock() failed");
        L4_LoadMR(0, 0);
        L4_Send(middle_prio2_thread);
        L4_LoadMR(0, 0);
        L4_Send(middle_prio3_thread);
        L4_LoadMR(0, 0);
        L4_Send(high_prio_thread);
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }
}

static void
high_mutex0301_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;
    L4_ThreadId_t dummy_thread;

    while (1) {
        L4_Wait(&dummy_thread);
        L4_LoadMR(0, 0);
        L4_Send(middle_prio3_thread);
        L4_LoadMR(0, 0);
        L4_Send(middle_prio2_thread);
        res = L4_Lock(m);
        fail_unless(res == 1, "L4_Lock() failed");
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }
}

static void
intermediate_thread(void)
{
    L4_ThreadId_t dummy_thread;

    while (1) {
        L4_Wait(&dummy_thread);
        cnt0 = 1;
        L4_LoadMR(0, 0);
        L4_Send(main_thread);
        L4_LoadMR(0, 0);
        L4_Send(main_thread);
        cnt0 = 1;
    }
}

static void
middle_mutex0301_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(1);
    L4_Word_t res;
    L4_ThreadId_t dummy_thread;

    while (1) {
        L4_Wait(&dummy_thread);
        fail_unless(flag[1] == 1, "Priority not respected");
        cnt0 = 0;
        res = L4_Lock(m);
        fail_unless(res == 1, "L4_Lock() failed");
        res = L4_Unlock(m);
        fail_unless(res == 1, "L4_Unlock() failed");
    }
}

static void
low_multiple_locking_thread(void)
{
    L4_MutexId_t m1 = L4_MutexId(0);
    L4_MutexId_t m2 = L4_MutexId(1);
    L4_ThreadId_t dummy_thread;
    L4_Word_t res;

    while (1) {
        res = L4_Lock(m1);
        fail_unless(res == 1, "L4_Lock() failed");
        res = L4_Lock(m2);
        fail_unless(res == 1, "L4_Lock() failed");
        L4_LoadMR(0, 0);
        L4_Send(high_prio_thread);
        fail_unless(cnt0 == 0, "Timeslice donation failed");
        L4_Wait(&dummy_thread);
        fail_unless(cnt0 == 0, "Schedule inheritance failed when mutexes are nested");
        res = L4_Unlock(m1);
        fail_unless(res == 1, "L4_Unlock() failed");
        fail_unless(cnt0 == 1, "Priority not respected after mutex1 release");
        res = L4_Unlock(m2);
        fail_unless(res == 1, "L4_Unlock() failed");
        cnt0 = 0;
    }
}

/*
\begin{test}{MUTEXSI0100}
  \TestDescription{Basic mutex schedule inheritance}
  \TestImplementationProcess{
    Three threads of three different priorities.
    \begin{enumerate}
        \item Low priority thread acquires mutex1.
        \item High priority thread blocks trying to acquire mutex1.
        \item Low priority thread runs and performs some work before releasing mutex1.
        \item High priority thread acquires mutex1.
    \end{enumerate}
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0100)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    high_prio_thread   = createThread(high_locking_thread);
    middle_prio_thread = createThread(middle_thread);
    low_prio_thread    = createThread(locking_thread);

    L4_Set_UserDefinedHandleOf(high_prio_thread, low_prio_thread.raw);
    L4_Set_UserDefinedHandleOf(low_prio_thread, high_prio_thread.raw);

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    res = L4_Set_Priority(high_prio_thread, 3);
    res = L4_Set_Priority(middle_prio_thread, 2);
    res = L4_Set_Priority(low_prio_thread, 1);

    cnt0 = cnt1 = 0;

    L4_LoadMR(0, 0);
    tag = L4_Call(high_prio_thread);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    fail_unless((cnt0 == LOOPS) && (cnt1 == 0), "Mutex schedule inheritance failed");

    L4_DeleteMutex(m);
    deleteThread(high_prio_thread);
    deleteThread(middle_prio_thread);
    deleteThread(low_prio_thread);
}
END_TEST

/*
\begin{test}{MUTEXSI0200}
  \TestDescription{Mutex schedule inheritance on holder deletion}
  \TestImplementationProcess{
    Three threads of three different priorities.
    \begin{enumerate}
        \item Low priority thread acquires mutex1.
        \item High priority thread blocks trying to acquire mutex1.
        \item Low priority thread runs and triggers intermediate thread.
        \item Intermediate thread deletes the low priority thread.
        \item High priority thread is scheduled and acquires mutex1.
    \end{enumerate}
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of intermediate thread is zero}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0200)
{
    L4_MsgTag_t tag = L4_Niltag;
    L4_Word_t ErrorCode;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    high_prio_thread = createThread(mutex02xx_locking_thread);
    canceller = createThread(deleting_thread);
    low_prio_thread = createThread(locking_triggering_thread);

    L4_Set_UserDefinedHandleOf(high_prio_thread, low_prio_thread.raw);
    L4_Set_UserDefinedHandleOf(low_prio_thread, high_prio_thread.raw);

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    res = L4_Set_Priority(high_prio_thread, 3);
    res = L4_Set_Priority(canceller, 2);
    res = L4_Set_Priority(low_prio_thread, 1);

    cnt0 = 0;

    L4_Set_Label(&tag, 0x2);
    L4_Set_MsgTag(tag);
    L4_Call(high_prio_thread);
    fail_unless(cnt0 == 0, "Schedule inheritance failed: no reschedule after Mutex holder is deleted");
    tag = L4_Receive(canceller);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    L4_DeleteMutex(m);
    deleteThread(high_prio_thread);
    deleteThread(canceller);
}
END_TEST

/*
\begin{test}{MUTEXSI0201}
  \TestDescription{Mutex schedule inheritance on timeslice donater deletion}
  \TestImplementationProcess{
    Three threads of three different priorities.
    \begin{enumerate}
        \item Low priority thread acquires mutex1.
        \item High priority thread blocks trying to acquire mutex1.
        \item Low priority thread runs and deletes high priority thread.
        \item Intermediate thread is scheduled.
    \end{enumerate}
    Counters used to check threads are running.
  }
  \TestPostConditions{Counter of low priority thread is zero}
  \TestPassStatus{do not pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0201)
{
    L4_ThreadId_t high_prio_thread, low_prio_thread;
    L4_ThreadId_t dummy;
    L4_MsgTag_t tag = L4_Niltag;
    L4_Word_t ErrorCode;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    high_prio_thread = createThread(mutex02xx_locking_thread);
    canceller = createThread(deleting_thread);
    low_prio_thread = createThread(locking_triggering_thread);

    L4_Set_UserDefinedHandleOf(high_prio_thread, low_prio_thread.raw);
    L4_Set_UserDefinedHandleOf(low_prio_thread, high_prio_thread.raw);

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    res = L4_Set_Priority(high_prio_thread, 3);
    res = L4_Set_Priority(canceller, 2);
    res = L4_Set_Priority(low_prio_thread, 1);

    cnt1 = 0;

    L4_Set_Label(&tag, 0x1);
    L4_Set_MsgTag(tag);
    L4_Send(high_prio_thread);
    tag = L4_Wait(&dummy);
    fail_unless(cnt1 == 0, "Schedule inheritance failed: no reschedule after the timeslice donater is deleted");

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");

    /* Wait for the low priority thread to finish of its half-completed delete
     * operation. */
    L4_Receive(low_prio_thread);

    deleteThread(canceller);
    deleteThread(low_prio_thread);
    L4_DeleteMutex(m);
}
END_TEST

/*
\begin{test}{MUTEXSI0300}
  \TestDescription{Mutex schedule inheritance on nested mutex}
  \TestImplementationProcess{
    Four threads of four different priorities: T1, T2, T3 and T4 with 
    respective priorities 1, 2, 3 and 4.
    \begin{enumerate}
        \item T1 acquires mutex1.
        \item T2 blocks trying to acquire mutex1.
        \item T3 blocks trying to acquire mutex1.
        \item T4 blocks trying to acquire mutex1.
        \item T1 runs and releases mutex1.
        \item T4 is scheduled, acquires the mutex and releases it.
        \item idem for T3 and finally T2.
    \end{enumerate}
    Array of flags used to check threads are scheduled in priority order.
  }
  \TestPassStatus{do not pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0300)
{
    L4_ThreadId_t low_prio_thread;
    L4_MsgTag_t tag;
    L4_MutexId_t m = L4_MutexId(0);
    L4_Word_t res;

    high_prio_thread = createThread(mutex0300_locking_thread);
    middle_prio3_thread = createThread(mutex0300_locking_thread);
    middle_prio2_thread = createThread(mutex0300_locking_thread);
    low_prio_thread = createThread(low_locking_thread);

    L4_Set_UserDefinedHandleOf(high_prio_thread, 0);
    L4_Set_UserDefinedHandleOf(middle_prio3_thread, 1);
    L4_Set_UserDefinedHandleOf(middle_prio2_thread, 2);

    res = L4_CreateMutex(m);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    res = L4_Set_Priority(high_prio_thread, 4);
    res = L4_Set_Priority(middle_prio3_thread, 3);
    res = L4_Set_Priority(middle_prio2_thread, 2);
    res = L4_Set_Priority(low_prio_thread, 1);

    flag[0] = 0;
    flag[1] = 0;
    flag[2] = 0;
    L4_LoadMR(0, 0);
    tag = L4_Call(high_prio_thread);

    deleteThread(high_prio_thread);
    deleteThread(middle_prio2_thread);
    deleteThread(middle_prio3_thread);
    deleteThread(low_prio_thread);
    L4_DeleteMutex(m);
}
END_TEST

/*
\begin{test}{MUTEXSI0301}
  \TestDescription{Mutex schedule inheritance on multiple nested mutexes}
  \TestImplementationProcess{
    Four threads of four different priorities: T1, T2, T3 and T4 with 
    respective priorities 1, 2, 3 and 4.
    \begin{enumerate}
        \item T1 acquires mutex1.
        \item T1 acquires mutex2.
        \item T4 blocks trying to acquire mutex1.
        \item T2 blocks trying to acquire mutex2.
        \item T1 runs and releases mutex1.
        \item T4 is scheduled, acquires mutex1 and releases it.
        \item T4 open waits.
        \item T3 is scheduled and open waits.
        \item T1 is scheduled and releases mutex2.
        \item T2 is scheduled, acquires mutex2 and releases it.
    \end{enumerate}
    Counter used to check if intermediate thread is running.
  }
  \TestPostConditions{}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0301)
{
    L4_ThreadId_t low_prio_thread, dummy;
    L4_MutexId_t m1 = L4_MutexId(0);
    L4_MutexId_t m2 = L4_MutexId(1);
    L4_Word_t res;
    int i;

    high_prio_thread = createThread(high_mutex0301_locking_thread);
    middle_prio3_thread = createThread(intermediate_thread);
    middle_prio2_thread = createThread(middle_mutex0301_locking_thread);
    low_prio_thread = createThread(low_multiple_locking_thread);

    res = L4_CreateMutex(m1);
    fail_unless(res == 1, "L4_CreateMutex() failed");
    res = L4_CreateMutex(m2);
    fail_unless(res == 1, "L4_CreateMutex() failed");

    res = L4_Set_Priority(high_prio_thread, 4);
    res = L4_Set_Priority(middle_prio3_thread, 3);
    res = L4_Set_Priority(middle_prio2_thread, 2);
    res = L4_Set_Priority(low_prio_thread, 1);

    cnt0 = 0;

    for (i = 0; i < LOOPS; i++) {
        L4_Wait(&dummy);
        res = L4_Set_Priority(middle_prio3_thread, 1);
        L4_Wait(&dummy);
        res = L4_Set_Priority(middle_prio3_thread, 3);
        L4_LoadMR(0, 0);
        L4_Send(low_prio_thread);
    }

    deleteThread(high_prio_thread);
    deleteThread(middle_prio2_thread);
    deleteThread(middle_prio3_thread);
    deleteThread(low_prio_thread);
    L4_DeleteMutex(m1);
    L4_DeleteMutex(m2);
}
END_TEST
#undef LOOPS

extern L4_ThreadId_t test_tid;

/* Expected priority of the server. */
static int mutexsi0400_expected_server_priority;

/* Tid of the server. */
static L4_ThreadId_t mutexsi0400_server_tid;

/* Server thread. */
static void
mutexsi0400_server(void)
{
    L4_ThreadId_t reply_cap;
    L4_MsgTag_t result;
    L4_MutexId_t m1, m2;
    L4_Word_t success;

    /* Create the two mutexes. */
    m1 = L4_MutexId(0);
    m2 = L4_MutexId(1);
    success = 1;
    success &= L4_CreateMutex(m1);
    success &= L4_CreateMutex(m2);
    success &= L4_Lock(m1);
    success &= L4_Lock(m2);
    fail_unless(success, "Failed to create mutexes.");

    /* Wait for initial send. */
    L4_Set_MsgTag(L4_Niltag);
    result = L4_Wait(&reply_cap);
    fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_Wait() failed.");

    for (;;) {
        /* Ensure our priority is correct. */
        fail_unless(get_current_threads_priority()
                == mutexsi0400_expected_server_priority,
                "Server did not inherit high-priority thread's priority.");

        /* If the tag we receive is the second-phase of the test,
         * reply directly back to the main thread. */
        if (L4_Label(result) == 1) {
            reply_cap = main_thread;
        }

        /* Reply back to the client. */
        L4_Set_MsgTag(L4_Niltag);
        result = L4_ReplyWait(reply_cap, &reply_cap);
        fail_unless(L4_IpcSucceeded(result), "Low-priority server L4_ReplyWait() failed.");
    }
}

/* Lock a mutex, giving them our priority. */
static void
mutexsi0400_locking_thread(void)
{
    L4_MutexId_t m = L4_MutexId(1);
    L4_Lock(m);
    fail("Unexpectantly acquired a mutex.");
}

/* Receive from a thread, giving them our priority. */
static void
mutexsi0400_receiving_thread(void)
{
    L4_Receive(mutexsi0400_server_tid);
    fail("Unexpectantly received an IPC.");
}
/*
\begin{test}{MUTEXSI0400}
  \TestDescription{High priority thread with inherited priority IPC'ing to
          low-priority server.}
  \TestImplementationProcess{
    High-priority thread holds two mutexes. One is uncontended, the other is
    contended by a higher-priority thread. The thread IPCs to a low-priority
    server. Ensure that the low-priority server gains the high-priority
    thread's priority.
  }
  \TestPostConditions{}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
\end{test}
*/
START_TEST(MUTEXSI0400)
{
    L4_ThreadId_t locker;
    L4_ThreadId_t server;
    L4_ThreadId_t receiver;
    L4_MsgTag_t result;
    L4_ThreadId_t dummy;
    int i, j;

    /* Test cases */
    struct {
        int receiver_prio;
        int locker_prio;
        int server_prio;

        int expected_server_prio_closed_wait;
        int expected_server_prio_open_wait;
    } test_cases[] = {
        /* Server always retains its own priority. */
        {  0,   0, 220, 220, 220},

        /* Server inherits our priority on closed call. */
        {  0,   0,   1, 200,   1},

        /* Server inherits mutex priority always. */
        {220,   0,   1, 220, 220},

        /* Server inherits our priority on closed, mutex priority on open. */
        { 10,   0,   1, 200,  10},

        /* Server inherits receiver's priority always. */
        {  0, 220,   1, 220, 220},

        /* Server inherits our priority on closed, receiver's priority on
         * open. */
        {  0,  10,   1, 200,  10},

        /* No more test-cases. */
        { -1,  -1,  -1,  -1,  -1}
    };

    /* Ensure our priority is correct. */
    fail_unless(get_current_threads_priority() == 200,
            "Expected the main thread's priority to be 200, but it was not.");

    /* Create the server, and allow it to start up. */
    server = createThread(mutexsi0400_server);
    L4_Set_Priority(server, 220);
    mutexsi0400_server_tid = server;

    /* Create the locker. */
    locker = createThread(mutexsi0400_locking_thread);

    /* Create the receiver. */
    receiver = createThread(mutexsi0400_receiving_thread);

    for (j = 0; test_cases[j].receiver_prio >= 0; j++) {
        /* Setup priorities. */
        L4_Set_Priority(server,   test_cases[j].server_prio);
        L4_Set_Priority(receiver, test_cases[j].receiver_prio);
        L4_Set_Priority(locker,   test_cases[j].locker_prio);

        /* Call the server ten times. */
        mutexsi0400_expected_server_priority
                = test_cases[j].expected_server_prio_closed_wait;
        for (i = 0; i < 10; i++) {
            L4_Set_MsgTag(L4_Make_MsgTag(0, 0));
            result = L4_Call(server);
            fail_unless(L4_IpcSucceeded(result), "high-priority client L4_Call() failed.");
        }

        /* Send/OpenWait the server ten times. */
        mutexsi0400_expected_server_priority
                = test_cases[j].expected_server_prio_open_wait;
        for (i = 0; i < 10; i++) {
            L4_MsgTag_t tag = L4_Make_MsgTag(1, 0);
            L4_Set_SendBlock(&tag);
            L4_Set_ReceiveBlock(&tag);
            result = L4_Ipc(server, L4_anythread, tag, &dummy);
            fail_unless(L4_IpcSucceeded(result), "high-priority client L4_Call() failed.");
        }
    }

    deleteThread(server);
    deleteThread(receiver);
    deleteThread(locker);
}
END_TEST

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
    compute_counter();
}

static void test_teardown(void)
{

}

TCase *
make_ipc_schedule_inheritance_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Ipc schedule inheritance");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, IPCSI0100);
    tcase_add_test(tc, IPCSI0101);
    tcase_add_test(tc, IPCSI0102);
    tcase_add_test(tc, IPCSI0103);
    tcase_add_test(tc, IPCSI0200);
    tcase_add_test(tc, IPCSI0201);
    tcase_add_test(tc, IPCSI0202);
    tcase_add_test(tc, IPCSI0300);
    tcase_add_test(tc, IPCSI0400);
// FIXME: Disable fair share test temporarily - awiggins.
//    if (has_feature_string(L4_GetKernelInterface(), "BOGUS")) {
    if (0) {
        tcase_add_test(tc, IPCSI0500);
    }
    tcase_add_test(tc, IPCSI0600);
    tcase_add_test(tc, IPCSI0900);
    tcase_add_test(tc, IPCSI1000);
    tcase_add_test(tc, IPCSI1100);

    return tc;
}

TCase *
make_mutex_schedule_inheritance_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Mutex schedule inheritance");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, MUTEXSI0100);
    tcase_add_test(tc, MUTEXSI0200);
    tcase_add_test(tc, MUTEXSI0201);
    tcase_add_test(tc, MUTEXSI0300);
    tcase_add_test(tc, MUTEXSI0301);
    tcase_add_test(tc, MUTEXSI0400);

    return tc;
}
