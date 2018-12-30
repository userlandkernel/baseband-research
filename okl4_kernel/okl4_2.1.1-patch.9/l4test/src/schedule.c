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
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/kdebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <bootinfo/bootinfo.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static volatile L4_ThreadId_t main_thread;
static volatile L4_Word_t g_started, g_counter = 0;
static volatile L4_ThreadId_t g_donothing_thread;
static volatile L4_ThreadId_t g_simple_thread_ids[2];

/* Utility functions -------------------------------------------------------*/

/*
 * A thread that copies its id.
 */
static void
simple_thread(L4_ThreadId_t me)
{
    /* Add our thread id to the next available slot in g_simple_thread_ids. */
    int i;

    for (i = 0; i < 2; i++) {
        if (g_simple_thread_ids[i].raw == L4_nilthread.raw) {
            break;
        }
    }
    g_simple_thread_ids[i] = me;
    
    /* Halt */
    L4_Send(me);
}

/*
 * A thread that does nothing.
 */
static void
spinning_thread(void)
{
    while(1)
        ; // do nothing
}


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

    while(1) {
        g_counter++; // increment the counter
    }
}

/*
 * A thread with no scheduling privilege that attempts to set its own Time slice.
 */
static void
unpriv_thread_set_timeslice(void)
{
    L4_Word_t result;
    L4_Word_t ErrorCode;

    result = L4_Set_Timeslice(L4_Myself(), ~0UL);
    fail_unless(result == L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to return Error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrNoPrivilege,
                "Error Code should be set to NoPrivilege");
    L4_Send(L4_Myself());
}
/*
 * A thread with no scheduling privilege that attempts to set its own priority.
 */
static void
unpriv_thread_set_priority(void)
{
    L4_Word_t result;
    L4_Word_t ErrorCode;

    result = L4_Set_Priority(L4_Myself(), 90);
    fail_unless(result == L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to return Error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrNoPrivilege,
                "Error Code should be set to NoPrivilege");
    L4_Send(L4_Myself());
}

/*
 * A thread that sets the priority of another thread to a higher level than itself.
 */
static void
setting_prio_thread(void)
{
    L4_Word_t result;
    L4_Word_t ErrorCode;

    result = L4_Set_Priority(g_donothing_thread, 202);
    fail_unless(result == L4_SCHEDRESULT_ERROR,
                "Unprivileged thread : L4_Set_Priority() failed to return Error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrInvalidParam,
                "Error Code should be set to InvalidParameter");
    L4_Send(L4_Myself());
}

/*
 * General do-nothing thread.
 */
static void
do_nothing_thread(void)
{
    L4_Send(L4_Myself());
}

/*
 * Thread for round robin test that increments a counter.
 */
static void 
sending_thread(void)
{
    L4_Msg_t msg;
    L4_ThreadId_t partner;
    L4_MsgTag_t tag;
    L4_Word_t i, mode, loops;
    volatile L4_Word_t *count;

    // wait for startup message
    tag = L4_Receive(main_thread);
    L4_MsgStore(tag, &msg);

    L4_Set_NotifyBits(0);

    partner.raw = L4_MsgWord(&msg, 0);
    count = (L4_Word_t*)L4_MsgWord(&msg, 1);
    mode = L4_MsgWord(&msg, 2);

    switch(mode) {
        case 0: loops = 20 * g_counter; break;
        case 1: loops = 2 * g_counter; break;
        default: loops = 20 * g_counter; break;
    }

    for (i = 0; i < loops; i++) {
        (*count)++;
        if (mode == 1) {
            tag = L4_Notify(partner, i);
        }
    }

    tag = L4_Call(main_thread);
}

/* A thread that spins around */
static void 
spinning_counting_thread(void)
{
    volatile L4_Word_t *counter;

    counter = (L4_Word_t *)L4_UserDefinedHandle();
    L4_Send(main_thread);

    while (!g_started) ;
    while(1){
        (*counter)++;
    }
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
    g_started = 1;

    for (i = 0; i < 10; i++) {
        L4_Yield();
    }

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

    new_cnt_thread = createThread(counting_thread);

    /* We set the time slice to the minimum time possible so we might get a
     * counter that roughly represents the tick length.  */
    result = L4_Set_Timeslice(new_cnt_thread, 1);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to set Time slice");
    result = L4_Set_Priority(new_cnt_thread, 200);

    /* Switch to the other guy at our priority. When we get back, it should
     * be just after the timer tick. */
    L4_Yield();

    /* Reset the counter to work out how much it can increase in a
     * timeslice. */
    g_counter = 0;
    L4_Yield();

    /* Done. */
    deleteThread(new_cnt_thread);
}

/* Priority Levels tests --------------------------------------------*/

/*
\begin{test}{SCHED0100}
  \TestDescription{Set priority level of two threads and check the execution order}
  \TestFunctionalityTested{Priority Levels}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create two threads
      \item Set the a different priority level for each thread
      \item Check that the thread with the highest priority is executed first
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0100)
{
    L4_ThreadId_t thread_p150, thread_p151, first, second;
    L4_Word_t result;
    int i;

    for (i = 0; i < 2; i++) {
        g_simple_thread_ids[i] = L4_nilthread;
    }
    thread_p150 = createThread((void (*)(void))simple_thread);
    thread_p151 = createThread((void (*)(void))simple_thread);

    /* Raise our priority, setup the priorities of two other
     * threads, than drop our priority so the other guys get
     * scheduled. */
    result = L4_Set_Priority(main_thread, 152);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    result = L4_Set_Priority(thread_p150, 150);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    result = L4_Set_Priority(thread_p151, 151);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");
    result = L4_Set_Priority(main_thread, 149);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Priority() failed to set the new priority");

    first = g_simple_thread_ids[0];
    second = g_simple_thread_ids[1];

    //printf("first : %lu, second : %lu\n", first.raw, second.raw);
    fail_if((first.raw == thread_p150.raw) && (second.raw == thread_p151.raw),
                "The thread with the lowest priority ran first");
    fail_unless((first.raw == thread_p151.raw) && (second.raw == thread_p150.raw),
                "The thread with the highest priority did not run first");

    deleteThread(thread_p150);
    deleteThread(thread_p151);
}
END_TEST


/* Remaining Time Slice and Quantum tests --------------------------------------------*/

/*
\begin{test}{SCHED0200}
  \TestDescription{Verify L4\_Schedule can get Remaining Time Slice and set Time Slice}
  \TestFunctionalityTested{\Func{L4\_Timeslice}, \Func{L4\_Set\_Timeslice}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new thread
      \item Get the remaining time slice of the new thread
      \item Check that \Func{L4\_Schedule} returns successfully
      \item Set the time slice to a different value
      \item Check that \Func{L4\_Schedule} returns successfully
      \item Check that the time slice is correctly set
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0200)
{
    L4_ThreadId_t new_spinning_thread;
    L4_Word_t result;
    L4_Word_t rem_timeslice_before;
    L4_Word_t rem_timeslice_after;
    L4_Word_t new_timeslice;

    new_spinning_thread = createThread(spinning_thread);
    result = L4_Set_Priority(new_spinning_thread, 1);
    fail_if(result == L4_SCHEDRESULT_ERROR,
            "L4_Set_Priority() failed to set the new priority");
    result = L4_Timeslice(new_spinning_thread, &rem_timeslice_before);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Time slice");

    new_timeslice = rem_timeslice_before + 5000;
    result = L4_Set_Timeslice(new_spinning_thread, new_timeslice);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_Timeslice() failed to set Time slice");

    result = L4_Timeslice(new_spinning_thread, &rem_timeslice_after);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Time slice");
    fail_unless(rem_timeslice_after == new_timeslice, "Timeslice was not set properly");

    deleteThread(new_spinning_thread);
}
END_TEST

/*
\begin{test}{SCHED0300}
  \TestDescription{Check Remaining Time Slice is decremented}
  \TestFunctionalityTested{\Func{L4\_Timeslice}, \Func{L4\_Set\_Timeslice}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Set the time slice of the main_thread to 40000
      \item Loop until remaining time slice is decremented or counter reaches 0
      \item Check that the remaining time slice has been decremented
      \item Perform a Yield
      \item Check that the remaining time slice has been reset
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0300)
{
    L4_Word_t i;
    L4_Word_t result;
    L4_Word_t old_ts;
    L4_Word_t rem_timeslice_after;
    L4_Word_t rem_timeslice_before;
    L4_Word_t new_timeslice;

    if(!g_counter) {
        compute_counter();
    }
    L4_Yield();
    result = L4_Timeslice(main_thread, (L4_Word_t*)&old_ts);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Old Time slice");

    new_timeslice = old_ts * 6;

    result = L4_Set_Timeslice(main_thread, new_timeslice);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to set Time slice");

    result = L4_Timeslice(main_thread, (L4_Word_t*)&rem_timeslice_before);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Old Time slice");
    fail_unless(rem_timeslice_before == new_timeslice,
                "New Time slice was not set properly");

    for (i = 0; i < 3 * g_counter; i++) {
        result = L4_Timeslice(main_thread, (L4_Word_t*)&rem_timeslice_after);
        fail_unless(result != L4_SCHEDRESULT_ERROR,
                    "L4_Timeslice() failed to retrieve Remaining Time slice");
        if (rem_timeslice_after < rem_timeslice_before) {
            break;
        }
    }
    fail_unless(rem_timeslice_after < rem_timeslice_before,
                    "Remaining Time Slice has not been decremented");

    L4_Yield();
    result = L4_Timeslice(main_thread, (L4_Word_t*)&rem_timeslice_after);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Remaining Time slice");
    fail_unless(rem_timeslice_after == new_timeslice,
                "Remaining Time Slice has not been reset to the time slice");

    result = L4_Set_Timeslice(main_thread, old_ts);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to set Time slice");
}
END_TEST

/*
\begin{test}{SCHED0500}
  \TestDescription{Check Time Slice set to infinite}
  \TestFunctionalityTested{\Func{L4\_Timeslice}, \Func{L4\_Set\_Timeslice}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Get the timer tick length
      \item Set the time slice to an infinite value
      \item Wait at least one time tick length
      \item Check that the remaining time slice has not been modified
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0500)
{
    L4_Word_t i, result;
    L4_Word_t rem_timeslice;

    result = L4_Set_Timeslice(main_thread, 0UL);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to set Time slice");
    if(!g_counter) {
        compute_counter();
    }
    i = 3 * g_counter;
    printf("waiting");
    while(i > 0) {
        i--;
        if(i % (g_counter/20) == 0) {
            printf(".");
        }
    }
    printf("\n");
    result = L4_Timeslice(main_thread, &rem_timeslice);
    fail_unless(result != L4_SCHEDRESULT_ERROR,
                "L4_Timeslice() failed to retrieve Remaining Time slice");
    fail_unless(rem_timeslice == 0,
                "Remaining Time Slice was set to an infintite value and should not have been modified");
}
END_TEST

/* Error Codes tests --------------------------------------------*/

/*
\begin{test}{SCHED0700}
  \TestDescription{Verify the No Privilege and Invalid Thread Error codes}
  \TestFunctionalityTested{\Func{L4\_Set\_Priority}, \Func{L4\_Set\_Timeslice}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new unprivileged thread that attempts a L4\_Schedule system call
      \item Check \Func{L4\_Schedule} returns an error in result
      \item Check the \Func{ErrorCode} for No Privilege
      \item Delete the new thread
      \item Attempt a L4\_Schedule system call on the deleted thread
      \item Check \Func{L4\_Schedule} returns an error in result
      \item Check the \Func{ErrorCode} for Invalid Thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0700)
{
    L4_ThreadId_t new_unpriv_thread;
    L4_Word_t result, ErrorCode;

    new_unpriv_thread = createThreadInSpace(L4_nilspace, unpriv_thread_set_timeslice);
    result = L4_Set_Priority(new_unpriv_thread, 201);
    fail_if(result == L4_SCHEDRESULT_ERROR,
            "L4_Set_Priority() failed to set the new priority");
    L4_Yield();
    deleteThread(new_unpriv_thread);

    result = L4_Set_Timeslice(new_unpriv_thread, ~0UL);
    fail_unless(result == L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to return Error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrInvalidThread,
                "Error Code should be set to InvalidThread");
}
END_TEST

/*
\begin{test}{SCHED0800}
  \TestDescription{Verify the Invalid Thread Error code}
  \TestFunctionalityTested{\Func{L4\_Set\_Timeslice}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Attempt a L4\_Schedule system call on L4\_nilthread
      \item Check L4\_Schedule returns Error in result
      \item Check the Error Code for Invalid Thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0800)
{
    L4_Word_t result, ErrorCode;

    result = L4_Set_Timeslice(L4_nilthread, ~0UL);
    fail_unless(result == L4_SCHEDRESULT_ERROR,
                "L4_Set_TimeSlice() failed to return Error");
    ErrorCode = L4_ErrorCode();
    fail_unless(ErrorCode == L4_ErrInvalidThread,
                "Error Code should be set to InvalidThread");
}
END_TEST

/*
\begin{test}{SCHED0900}
  \TestDescription{Verify the Invalid Parameter Error code with unprivileged thread}
  \TestFunctionalityTested{\Func{L4\_Set\_Priority}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create two unprivileged threads
      \item Set the first thread to be a scheduler of the second
      \item First thread attempts to set the priority of the second to a higher value than its own priority
      \item Check \Func{L4\_Schedule} returns an error in result
      \item Check the \Func{ErrorCode} for Invalid Parameter
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED0900)
{
    L4_ThreadId_t unpriv_faulting_thread;
    L4_Word_t result, ErrorCode;

    unpriv_faulting_thread = createThreadInSpace(L4_nilspace, setting_prio_thread);
    g_donothing_thread = createThreadInSpace(L4_nilspace, do_nothing_thread);
    result = L4_ThreadControl(g_donothing_thread, L4_nilspace, unpriv_faulting_thread,
                              L4_nilthread, L4_nilthread, 0, (void *)~0UL);
    ErrorCode = L4_ErrorCode();
    _fail_unless(result == 1, __FILE__, __LINE__,
                "L4_ThreadControl() failed to set the scheduler parameter of the new thread : %d",
                ErrorCode);
    waitSending(unpriv_faulting_thread);

    deleteThread(unpriv_faulting_thread);
    deleteThread(g_donothing_thread);
}
END_TEST

/*
\begin{test}{SCHED1000}
  \TestDescription{Verify a privileged thread can set a higher priority}
  \TestFunctionalityTested{\Func{L4\_Set\_Priority}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new thread
      \item Set the the main\_thread as the scheduler of the new thread   
      \item Attempt to set the priority of the new thread to a higher value than the priority of the main\_thread
      \item Check that \Func{L4\_Schedule} returns successfully
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED1000)
{
    L4_ThreadId_t new_donothing_thread;
    L4_Word_t result, ErrorCode;

    new_donothing_thread = createThread(do_nothing_thread);
    result = L4_ThreadControl(new_donothing_thread, L4_nilspace, main_thread,
                              L4_nilthread, L4_nilthread, 0, (void *)~0UL);
    ErrorCode = L4_ErrorCode();
    _fail_unless(result == 1, __FILE__, __LINE__,
                "L4_ThreadControl() failed to set the scheduler parameter of the new thread : %d",
                ErrorCode);
    result = L4_Set_Priority(new_donothing_thread, 201);
    fail_if(result == L4_SCHEDRESULT_ERROR,
                "Privileged thread : L4_Set_Priority() failed to set the new priority");

    deleteThread(new_donothing_thread);
}
END_TEST

/*
\begin{test}{SCHED1100}
  \TestDescription{Verify a not privileged thread which is not the scheduler can not set the priority}
  \TestFunctionalityTested{\Func{L4\_Set\_Priority}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create a new unprivileged thread that attempts a L4\_Schedule system call
      \item Check \Func{L4\_Schedule} returns an error in result
      \item Check the \Func{ErrorCode} for No Privilege
      \item Delete the new thread
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED1100)
{
    L4_ThreadId_t new_unpriv_thread;
    L4_Word_t result;

    new_unpriv_thread = createThreadInSpace(L4_nilspace, unpriv_thread_set_priority);
    result = L4_Set_Priority(new_unpriv_thread, 201);
    fail_if(result == L4_SCHEDRESULT_ERROR,
            "L4_Set_Priority() failed to set the new priority");
    L4_Yield();
    deleteThread(new_unpriv_thread);
}
END_TEST

/* Round robin tests --------------------------------------------*/

/*
\begin{test}{SCHED1200}
  \TestDescription{Round robin test}
  \TestFunctionalityTested{Sharing of scheduling time}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create two threads of same priority who increment a counter
      \item The first thread whose counter reaches the limit stops the test
      \item Check the two counters have no more than 4% difference
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED1200)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    volatile L4_Word_t count0, count1;
    L4_ThreadId_t thrd0, thrd1, from;
    L4_Word_t mode;

    if(!g_counter) {
        compute_counter();
    }

    for (mode = 0; mode < 2; mode++)
    {
        count0 = count1 = 0;

        thrd0 = createThread(sending_thread);
        thrd1 = createThread(sending_thread);
        L4_Set_Priority(thrd0, 10);
        L4_Set_Priority(thrd1, 10);

        L4_MsgClear(&msg);

        L4_MsgAppendWord(&msg, thrd1.raw);
        L4_MsgAppendWord(&msg, (L4_Word_t)&count0);
        L4_MsgAppendWord(&msg, mode);
        L4_MsgLoad(&msg);

        // send startup message
        tag = L4_Send(thrd0);

        L4_MsgClear(&msg);

        L4_MsgAppendWord(&msg, thrd0.raw);
        L4_MsgAppendWord(&msg, (L4_Word_t)&count1);
        L4_MsgAppendWord(&msg, mode);
        L4_MsgLoad(&msg);

        // send startup message
        tag = L4_Send(thrd1);

        // wait for end of test
        tag = L4_Wait(&from);
        //tag = L4_Receive(thrd1);

        printf("count0 = %ld\n", count0);
        printf("count1 = %ld\n", count1);
        printf("diff = %ld%%\n", (abs((long)count0 - count1) * 100) / count0);

        deleteThread(thrd0);
        deleteThread(thrd1);

        /* fail on 2% variation */
        fail_unless((abs((long)count0 - count1) * 100) / count0 <= 2,
                "Round robin not fair\n");
    }
}
END_TEST

/*
\begin{test}{SCHED1201}
  \TestDescription{Second round robin test}
  \TestFunctionalityTested{Sharing of scheduling time}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create three threads of same priority
      \item Two of them increment a counter in an infinite loop
      \item The third thread yields 10 times before stoping the test
      \item Check the two counters have no more than 2% difference
    \end{enumerate}
  }
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED1201)
{
    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;
    L4_Word_t res;
    L4_Word_t diff, limit;
    volatile L4_Word_t cnt0, cnt1;
    L4_ThreadId_t spinner, spinner2, int_counter;

    spinner  = createThread(spinning_counting_thread);
    spinner2 = createThread(spinning_counting_thread);
    int_counter = createThread(interrupt_counter_thread);

    res = L4_Set_Priority(int_counter, 10);
    res = L4_Set_Priority(spinner, 10);
    res = L4_Set_Priority(spinner2, 10);

    g_started = 0;
    cnt0 = cnt1 = 0;

    L4_Set_UserDefinedHandleOf(spinner, (L4_Word_t)&cnt0);
    L4_Set_UserDefinedHandleOf(spinner2, (L4_Word_t)&cnt1);
    L4_Receive(spinner);
    L4_Receive(spinner2);
    tag = L4_Call(int_counter);

    ErrorCode = L4_ErrorCode();
    fail_unless(!getTagE(tag), "IPC returned error\n");
    diff = abs((long)cnt0 - cnt1);
    limit = (MAX(cnt0, cnt1) * 2) / 100;
    printf("counter 0 = %ld\n", cnt0);
    printf("counter 1 = %ld\n", cnt1);
    printf("difference is %lu => %lu%%, limit is %lu\n", diff, (diff * 100) / MAX(cnt0, cnt1), limit);
    fail_unless(diff <= limit, "Timeslice is not equally shared");

    deleteThread(int_counter);
    deleteThread(spinner);
    deleteThread(spinner2);
}
END_TEST

/*
\begin{test}{SCHED1300}
  \TestDescription{Verify lazy dequeuing does not have bug fixed in bug2102 branch}
  \TestFunctionalityTested{\Func{find_next_thread}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Create three threads at different priorities that map to same word in priority bitmap
      \item high thread yields to ensure it is enqueued
      \item generate a fastpath ipc from high to medium thread so that high thread is not runnable but still in queue
      \item medium thread yields via ipc_wait (threadswitch will requeue the thread and thus mask the bug)
      \item if bug is present processor will go to sleep when high thread is dequeued in findnextthread
      \item if bug is not present low will run, ipc medium and test will complete
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
static void sender_thread(void)
{
    while(1) {
        L4_Yield();
        L4_Call(main_thread);
    }
}

static void low_thread(void)
{
    while(1) {
        L4_Send(main_thread);
    }
}

START_TEST(SCHED1300)
{
    L4_ThreadId_t high, low, dummy;
    int i;

    high = createThreadInSpace(L4_nilspace, sender_thread);
    low = createThreadInSpace(L4_nilspace, low_thread);
    L4_Set_Priority(low, 199);
    L4_Set_Priority(high, 201);
    // allow other thread to start up
    L4_Receive(high);
    // do it a few times so we get a fastpath ipc
    for(i = 0; i < 10; i++) {
        L4_Call(high);
        L4_Wait(&dummy);
    }

    // if we get here test passed
    deleteThread(high);
    deleteThread(low);
}
END_TEST

/*
\begin{test}{SCHED1400}
  \TestDescription{Basic switch to myself test}
  \TestFunctionalityTested{\Func{L4_ThreadSwitch}}
  \TestImplementationProcess{
    Call \Func{L4_ThreadSwitch} with \Func{L4_Myself}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(SCHED1400)
{
    int i;

    for (i = 0; i < 20; i++) {
        L4_ThreadSwitch(L4_Myself());
    }
}
END_TEST
/* -------------------------------------------------------------------------*/

extern L4_ThreadId_t test_tid;

static void
test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}


static void
test_teardown(void)
{
}

TCase *
make_schedule_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Thread Scheduling Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    tcase_add_test(tc, SCHED0100);
    tcase_add_test(tc, SCHED0200);
    tcase_add_test(tc, SCHED0300);
    tcase_add_test(tc, SCHED0500);
    tcase_add_test(tc, SCHED0700);
    tcase_add_test(tc, SCHED0800);
    tcase_add_test(tc, SCHED0900);
    tcase_add_test(tc, SCHED1000);
    tcase_add_test(tc, SCHED1100);
/* FIXME: Disabled fair share tests temporarily - awiggins. */
//    if (has_feature_string(L4_GetKernelInterface(), "BOGUS")) {
    if (0) {
        tcase_add_test(tc, SCHED1200);
        tcase_add_test(tc, SCHED1201);
    }
    tcase_add_test(tc, SCHED1300);
    tcase_add_test(tc, SCHED1400);
    
    return tc;
}
