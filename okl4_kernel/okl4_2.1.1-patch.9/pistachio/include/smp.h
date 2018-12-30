/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Description:   multiprocessor handling
 */
#ifndef __API__V4__SMP_H__
#define __API__V4__SMP_H__

#include <types.h>
#include <tcb.h>
#include <mp.h>


#if defined (CONFIG_MDOMAINS)


/**
 * central SMP handler function; should be called in processor_sleep
 * deals with both, sync and async
 */
void process_xcpu_mailbox(continuation_t continuation);

/**
 * Architecture specific XCPU trigger function (IPI) processing XCPU
 * mailboxes
 */
//void smp_xcpu_trigger(cpuid_t cpu);


/**********************************************************************
 *
 *                  Asynchronous XCPU handling
 *
 **********************************************************************/

// maximum number of outstanding XCPU requests
#define MAX_MAILBOX_ENTRIES     32

class cpu_mb_entry_t;
typedef void (*xcpu_handler_t)(cpu_mb_entry_t *, continuation_t);

/* mailbox entry */
class cpu_mb_entry_t
{
public:
    void set(xcpu_handler_t handler, tcb_t * tcb,
             word_t param0, word_t param1, word_t param2)
        {
            this->handler = handler;
            this->tcb = tcb;
            this->param[0] = param0;
            this->param[1] = param1;
            this->param[2] = param2;
        }


    void set(xcpu_handler_t handler, tcb_t * tcb,
             word_t param0, word_t param1, word_t param2, word_t param3,
             word_t param4, word_t param5, word_t param6, word_t param7)
        {
            set (handler, tcb, param0, param1, param2);
            this->param[3] = param3;
            this->param[4] = param4;
            this->param[5] = param5;
            this->param[6] = param6;
            this->param[7] = param7;
        }

public:
    xcpu_handler_t handler;
    tcb_t * tcb;
    word_t param[8];
};

/**
 * Asynchronous XCPU mailbox
 * currently not very efficient using a spin-lock for the mailbox
 */
class cpu_mb_t
{
public:
    void walk_mailbox() NORETURN;
    cpu_mb_entry_t * alloc()
        {
            lock.lock();
            if ( ((first_free + 1) % MAX_MAILBOX_ENTRIES) == first_alloc )
            {
                lock.unlock();
                return NULL;
            }
            unsigned idx = first_free;
            first_free = (first_free + 1) % MAX_MAILBOX_ENTRIES;
            return &entries[idx];
        }

    void commit (cpu_mb_entry_t * entry)
        {
            lock.unlock();
        }

    bool enter (xcpu_handler_t handler, tcb_t * tcb,
                word_t param0, word_t param1, word_t param2)
        {
            cpu_mb_entry_t * entry = alloc();
            if (!entry) return false;

            entry->set(handler, tcb, param0, param1, param2);
            commit(entry);
            return true;
        }

    bool enter (xcpu_handler_t handler, tcb_t * tcb,
                word_t param0, word_t param1, word_t param2, word_t param3,
                word_t param4, word_t param5, word_t param6, word_t param7)
        {
            cpu_mb_entry_t * entry = alloc();
            if (!entry) return false;

            entry->set (handler, tcb,
                        param0, param1, param2, param3,
                        param4, param5, param6, param7);
            commit(entry);
            return true;
        }

private:
    volatile unsigned first_alloc;
    volatile unsigned first_free;
    spinlock_t lock;
    cpu_mb_entry_t entries[MAX_MAILBOX_ENTRIES]
    ALIGNED(CACHE_LINE_SIZE);
} ALIGNED(CACHE_LINE_SIZE);


extern cpu_mb_t cpu_mailboxes[];
INLINE cpu_mb_t * get_cpu_mailbox (cpu_context_t dst)
{
    int index = dst.unit;
    /* Mailboxes only work for MUNITS at the moment */
#ifdef CONFIG_MDOMAINS
#error BROKEN
#endif
    if (index >= CONFIG_NUM_UNITS) {
        printf("trying to get invalid mailbox %d from %p\n", get_pc());
        ASSERT(ALWAYS, !"invalid index");
    }
    return &cpu_mailboxes[index];
}

INLINE void xcpu_request(cpu_context_t dstcpu, xcpu_handler_t handler,
                         tcb_t * tcb = NULL,
                         word_t param0 = 0, word_t param1 = 0,
                         word_t param2 = 0 )
{
    if (! get_cpu_mailbox(dstcpu)->enter(handler, tcb,
                                             param0, param1, param2) )
    {
        printf("Mailbox for %x full, attempted to place handler %p\n", dstcpu.raw, handler);
        UNIMPLEMENTED();
    }
#ifndef CONFIG_SMP_IDLE_POLL
    /* trigger an IPI */
    get_mp()->interrupt_context(dstcpu);
#endif
}

INLINE void xcpu_request(cpu_context_t dstcpu, xcpu_handler_t handler, tcb_t * tcb,
                         word_t param0, word_t param1,
                         word_t param2, word_t param3,
                         word_t param4 = 0, word_t param5 = 0,
                         word_t param6 = 0, word_t param7 = 0)
{
    if (! get_cpu_mailbox(dstcpu)->enter(handler, tcb,
                                             param0, param1, param2, param3,
                                             param4, param5, param6, param7)) {
        UNIMPLEMENTED();
    }
#ifndef CONFIG_SMP_IDLE_POLL
    /* trigger an IPI */
    get_mp()->interrupt_context(dstcpu);
#endif
}


/**********************************************************************
 *
 *                   Synchronous XCPU handling
 *
 **********************************************************************/
#ifdef CONFIG_SMP_SYNC_REQUEST

/*
 * synchronous XCPU request handling, depends on the hardware
 * architecture. Needed e.g. on IA32 for TLB shoot-downs. See
 * api/v4/smp.cc for a detailed description.
 */

class sync_entry_t : public cpu_mb_entry_t
{
public:
    void set_pending(cpuid_t cpu);
    void clear_pending(cpuid_t cpu);
    void ack(cpuid_t cpu);

    void handle_sync_requests();

public:
    word_t pending_mask;
    word_t ack_mask;
};

void sync_xcpu_request(cpuid_t dstcpu, xcpu_handler_t handler,
                       tcb_t * tcb = NULL, word_t param0 = 0,
                       word_t param1 = 0, word_t param2 = 0);

#endif /* CONFIG_SMP_SYNC_REQUEST */

#endif /* CONFIG_MDOMAINS */

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
#include <arch/smp.h>
#endif

#endif /* !__API__V4__SMP_H__ */
