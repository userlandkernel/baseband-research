/*
 * Copyright (c) 2002-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
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
/*
 * Description:   Multiprocessor handling for cross-processor
 */

#include <sync.h>
#include <kdb/tracepoints.h>
#include <mp.h>
#include <smp.h>
#include <schedule.h>
#include <queueing.h>

#if defined(CONFIG_MDOMAINS)

//#define TRACE_IPI(x...) do { printf("CPU %d: ", get_current_cpu()); printf(x); } while(0)
#define TRACE_IPI(x...)

#ifdef CONFIG_SMP_SYNC_REQUEST

/*
 * VU: Synchronous XCPU handling
 *
 * Each CPU has a dedicated synchronous sender-based mailbox.  When
 * sending a request to another CPU, the parameters are filled in and
 * a pending bit is flipped in the destination CPU's mailbox.
 * Afterwards, the CPU polls for an ack on the local mailbox but still
 * handles incoming synchronous requests.  This scheme allows to send
 * remote requests even with disabled interrupts. The handler MUST NOT
 * preempt the current thread.  Synchronous XCPU handling has to be
 * enabled explicitly by defining CONFIG_SMP_SYNC_REQUEST in
 * INC_GLUE(config.h) */

static sync_entry_t sync_xcpu_entry[CONFIG_NUM_CONTEXTS];

void sync_entry_t::handle_sync_requests()
{
    while (this->pending_mask)
    {
        for (int cpu = 0; cpu < CONFIG_NUM_CONTEXTS; cpu++)
            if (this->pending_mask & (1 << cpu))
            {
                sync_xcpu_entry[cpu].handler(&sync_xcpu_entry[cpu]);
                this->clear_pending(cpu);
                sync_xcpu_entry[cpu].ack(get_current_cpu());
            }
    }
}

void sync_xcpu_request(cpuid_t dstcpu, xcpu_handler_t handler, tcb_t * tcb,
                       word_t param0, word_t param1, word_t param2)
{
    sync_entry_t * entry = &sync_xcpu_entry[get_current_cpu()];

    entry->ack_mask = 0;
    entry->set (handler, tcb, param0, param1, param2);

    // now signal the other CPU
    sync_xcpu_entry[dstcpu].set_pending(get_current_cpu());

    // trigger other side to make sure it gets processed ASAP
    smp_xcpu_trigger(dstcpu);

    // now we poll till the CPU has finished the request
    while (entry->ack_mask == 0)
    {
        spin(70);
        sync_xcpu_entry[get_current_cpu()].handle_sync_requests();
    }
}
#endif /* CONFIG_SMP_SYNC_REQUEST */


/*
 * VU: Asynchronous handling
 *
 * Each CPU has a mailbox with multiple entries. When performing an
 * asynchronous XCPU request, an entry is allocated triggering a
 * remote handler with the corresponding parameters.  To free entries
 * quickly they are copied onto the current stack. Async requests are
 * receiver based and therefore require locking.  To avoid contention
 * a sender-receiver matrix could be used with n*(n-1) mailboxes. */

/**
 * The Control/Continuation function that invokes the actual mailbox walker
 *
 * This function is simply a callback that invokes the class member function walk_mailbox
 *
 */
static CONTINUATION_FUNCTION(do_process_xcpu_mailbox)
{
#ifdef CONFIG_SMP_SYNC_REQUEST
    sync_xcpu_entry[get_current_cpu()].handle_sync_requests();
#endif
    get_cpu_mailbox (get_current_context())->walk_mailbox();
}


/**
 * The SMP/SMT mailbox walker loop
 * invokes mailbox entries as they are found
 *
 * Mailbox entries used to be control functions, however this was later removed.  As such
 * the convoluted continuation method used to call these functions could now be removed.
 */
void cpu_mb_t::walk_mailbox()
{
    lock.lock();
    while(first_free != first_alloc)
    {
        cpu_mb_entry_t entry = entries[first_alloc];
        entries[first_alloc].handler = NULL;
        first_alloc = (first_alloc + 1) % MAX_MAILBOX_ENTRIES;
        lock.unlock();
        ASSERT(ALWAYS, entry.handler);
        entry.handler(&entry, do_process_xcpu_mailbox);
        lock.lock();
    }
    lock.unlock();
    ACTIVATE_CONTINUATION(get_current_tcb()->xcpu_continuation);
}

/**
 * Walk through the items in this execution unit's mailbox and execute them one at a time
 *
 * This is a Control Function
 *
 * @param continuation the continuation to invoke when the mailbox is emptied
 */
void process_xcpu_mailbox(continuation_t continuation)
{
#ifdef CONFIG_SMP_SYNC_REQUEST
#error NOT THIS ONE
    sync_xcpu_entry[get_current_cpu()].handle_sync_requests();
#endif
    get_current_tcb()->xcpu_continuation = continuation;
    ACTIVATE_CONTINUATION(do_process_xcpu_mailbox);
}


#endif
