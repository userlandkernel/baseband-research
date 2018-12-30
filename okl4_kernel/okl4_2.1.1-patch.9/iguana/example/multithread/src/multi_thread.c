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
/*
 * Author: Malcolm Purvis <malcolmp@ok-labs.com>
 */

#include <stdio.h>
#include <assert.h>
#include <l4/thread.h>
#include <l4/ipc.h>

#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <iguana/env.h>

void main_second_thread(int argc, char** argv);

/* The main thread for the second thread that is started at boottime. */

void
main_second_thread(int argc, char** argv)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;
    L4_ThreadId_t from = L4_nilthread;
    int i;

    /*
     * To avoid multithreaded problems with printf(), wait for the
     * main thread to tell us that it is safe.
     *
     */

    tag = L4_Wait(&from);

    /* As an example, print out the command line arguments. */
    printf("Second Thread Command Line Arguments:\n");
    for (i = 0; i < argc; i++)
        printf("\t%s\n", argv[i]);
    printf("-------------------------------------\n");

    /* Tell the main thread that we're done. */
    L4_MsgClear(&msg);
    L4_MsgLoad(&msg);
    L4_Call(from);

    while(1);
}


int
main(int argc, char **argv)
{
    /* Find the second thread in the environment. */
    thread_ref_t second_thread = env_thread(iguana_getenv("SECOND_THREAD"));
    L4_Msg_t msg;
    L4_MsgTag_t tag;

    /* Message from this thread to start things off. */
    printf("Switching to the second thread:\n");

    /* Tell the second thread that it is time to use printf(). */
    L4_MsgClear(&msg);
    L4_Set_MsgLabel(&msg, 0x1);
    L4_MsgLoad(&msg);

    /* Go! */
    tag = L4_Call(thread_l4tid(second_thread));

    if (L4_IpcFailed(tag)) {
        printf("Couldn't prod the second thread (0x%lx) (%ld).\n", (long) thread_l4tid(second_thread).raw,
               (long) L4_ErrorCode() >> 1);
        return 1;
    }
    
    /*
     * Final message from this thread to show that everything is
     * working.
     */
    printf("Second thread works!\n");
}
