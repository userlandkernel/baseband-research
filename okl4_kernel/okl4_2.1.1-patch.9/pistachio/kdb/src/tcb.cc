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
 * Description:   tcb dumping
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <tcb.h>
#include <schedule.h>


#if defined(CONFIG_IS_64BIT)
#define __PADSTRING__ "         "
#else
#define __PADSTRING__ ""
#endif


DECLARE_CMD(cmd_show_tcb, root, 't', "showtcb",  "show thread control block");
DECLARE_CMD(cmd_show_tcbext, root, 'T', "showtcbext", "shows thread control block (extended)");

static inline msg_tag_t SECTION(SEC_KDEBUG) get_msgtag(tcb_t* tcb)
{
    msg_tag_t tag;
    tag.raw = tcb->get_mr(0);
    return tag;
}

void SECTION(SEC_KDEBUG) dump_tcb(tcb_t * tcb)
{
    space_t * space = tcb->get_space();
    printf("=== %s == TCB: %p == ID: %p = %p/%p == APRIO/EPRIO: 0x%02x/0x%02x",
#if defined(CONFIG_THREAD_NAMES)
           tcb->debug_name[0] ? tcb->debug_name : "<noname>",
#else
           "",
#endif
           tcb, tcb->get_global_id().get_raw(),
           tcb->utcb_location, tcb->get_utcb(),
           tcb->base_prio, tcb->effective_prio);
#if !defined(CONFIG_MDOMAINS) && !defined(CONFIG_MUNITS)
    printf(" ===\n");
#else
    printf(" CPU owner: %d ===\n", tcb->get_context().unit);
#endif

    printf("UIP: %p   queues: %c%c%s      space: %S   pager: %wt\n",
           tcb->get_user_ip(),
           tcb->ready_list.is_queued()     ? 'R' : 'r',
           tcb->blocked_list.is_queued()   ? 'B' : 'b',
           __PADSTRING__,
           space,
           tcb->get_utcb() ? tcb->get_pager() : NULL);
    printf("USP: %p   tstate: %ws  ready: %w.wt:%-w.wt   pdir : %p\n",
           tcb->get_user_sp(),
           tcb->get_state().string(),
           tcb->ready_list.next, tcb->ready_list.prev,
           tcb->page_directory);

    printf("sndhd : %-w.wt  blocked : %w.wt:%-w.wt   waiting_for : %-w.wt\n",
           tcb->get_endpoint()->get_send_head(),
           tcb->blocked_list.next, tcb->blocked_list.prev,
           tcb->get_waiting_for());

    printf("ts length  :       0x%x us, curr ts: 0x%x us\n",
           tcb->timeslice_length, tcb->current_timeslice);
    printf("resources:    %p [", (word_t) tcb->resource_bits);
    tcb->resources.dump (tcb);
#if defined(CONFIG_ARCH_ARM) && defined(CONFIG_ENABLE_FASS)
    printf("]");
    printf(", ARM [PID: %d, vspace: %d, domain: %d, dom_mask %lx",
                    space ? space->get_pid() : 0,
                    space ? space->get_vspace() : 0,
                    space ? space->get_domain() : 0,
                    space ? space->get_domain_mask() : 0);
#endif
    printf("]\n");
    printf("continuation: %p   preemption_cont: %p\n", tcb->cont, tcb->preemption_continuation);

#if defined(CONFIG_MUNITS) && defined(CONFIG_CONTEXT_BITMASKS)
    printf("hw_thread bitmask: %02x\n", tcb->context_bitmask);
#endif
    printf("scheduler: %.wt    exception_handler: %.wt\n",
            tcb->get_scheduler(), tcb->get_exception_handler());
    printf("  partner: %.wt        saved partner: %.wt      saved state: %s\n",
            tcb->is_partner_valid() ? tcb->get_partner() : NULL,
            tcb->get_saved_partner(),
            tcb->get_saved_state ().string ());
    printf("  tcb_idx: %lx\n", tcb->tcb_idx);

    printf("references: ");
    cap_t *cap = tcb->master_cap;
    printf("T %p", cap);
    while (cap && cap->next) {
        cap = cap->next;
        printf("->%p(%c)", cap, cap->is_ipc_cap() ? 'I' : '?');
    }
    printf("\n");
}


void SECTION (SEC_KDEBUG) dump_utcb (tcb_t * tcb)
{
    /*lint -e506 Strictly is constant at execution, but not at compile time */
    printf ("\nuser handle:          %p  "
            "cop flags:           %02x%s    "
            "preempt flags: %02x [%c]\n",
            tcb->get_user_handle (), tcb->get_cop_flags (),
            sizeof (word_t) == 8 ? "              " : "      ",
            tcb->get_preempt_flags ().raw,
            tcb->get_preempt_flags ().is_signaled() ? 's' : '~');
    printf("incoming notify bits: %p  "
           "notify mask:         %p\n",
            tcb->get_notify_bits(), tcb->get_notify_mask());
    printf("last preempted_ip:    %p  "
            "preempt_callback_ip: %p\n",
            tcb->get_preempted_ip(), tcb->get_preempt_callback_ip());
}


/**
 * Dumps a message and buffer registers of a thread in human readable form
 * @param tcb   pointer to thread control block
 */
static void SECTION(SEC_KDEBUG) dump_message_registers(tcb_t * tcb)
{
    for (int i = 0; i < IPC_NUM_MR; i++)
    {
        if (!(i % 8)) printf("\nmr(%02d):", i);
        printf(" %p", tcb->get_mr(i));
    }

    printf("\nMessage Tag: %d untyped, label = %x, flags = %c%c\n",
        get_msgtag(tcb).get_untyped(),
        get_msgtag(tcb).send.label,
        get_msgtag(tcb).is_error() ? 'E' : '-'
    );
}

static void SECTION(SEC_KDEBUG) dump_ipc_registers(tcb_t * tcb)
{
    acceptor_t acc;

    acc = tcb->get_acceptor();

    printf("\nAcceptor: %p (%c)", acc.raw, acc.accept_notify() ? 'A' : 'a');
    printf("  Error code: %d\n", tcb->get_error_code());
}

tcb_t SECTION(SEC_KDEBUG) * kdb_get_tcb()
{
    return get_thread("tcb/tid/name");
}

CMD(cmd_show_tcb, cg)
{
    tcb_t * tcb = kdb_get_tcb();
    if (tcb)
        dump_tcb(tcb);
    return CMD_NOQUIT;
}

CMD(cmd_show_tcbext, cg)
{
    tcb_t * tcb = kdb_get_tcb();
    if (tcb)
    {
        dump_tcb(tcb);
        if (tcb->get_utcb())
        {
            dump_utcb(tcb);
            dump_message_registers(tcb);
            dump_ipc_registers(tcb);
        }
        else
            printf("no valid UTCB\n");
    }
    return CMD_NOQUIT;
}

