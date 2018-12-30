/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description:   ARMv5 syscall and page fault handlers
 */

#include <l4.h>
#include <debug.h>

#include <space.h>
#include <tcb.h>
#include <tracebuffer.h>
#include <kdb/tracepoints.h>

/* ARM program counter fixup - LSB used for context frame type */
#define PC(x)           (x & (~1UL))

#if !defined(CONFIG_ENABLE_FASS)
#error ARMv5 requires FASS enabled
#endif

DECLARE_TRACEPOINT(ARM_PAGE_FAULT);
DECLARE_TRACEPOINT(ARM_DATA_ABORT);

/* Function declarations */

CONTINUATION_FUNCTION(finish_arm_memory_abort);

bool fass_sync_address (space_t * space, addr_t fault_addr, bool * is_valid = NULL);
void fass_update_pds (space_t * space, pgent_t cpg, pgent_t spg, word_t fault_section);

extern "C" NORETURN void
send_exception_ipc (word_t exc_no, word_t exc_code,
        arm_irq_context_t *context, continuation_t continuation);

/*
 * ARM data/instruction abort handler
 *
 * This function handles ARM pagefaults including domain faults,
 * data and prefetch aborts and alignment faults.
 */

extern "C" void arm_memory_abort(word_t fault_status, addr_t fault_addr,
        arm_irq_context_t *context, word_t data_abt)
{
    /* Note, fs = 0 for execute fault */
    word_t fs = fault_status & 0xf;
    tcb_t * current = get_current_tcb();

    TRACEPOINT(ARM_DATA_ABORT,
            space_t *_s = current->get_space();
            word_t _pid = (word_t)fault_addr;
            if (_s && ((word_t)fault_addr < PID_AREA_SIZE))
            {
                _pid = _s->get_pid();
                _pid = (word_t)fault_addr + (_pid << 25);
            }
            printf("DABT @ %p [%p], pc = %p, tcb = %p, fs = %x\n", (addr_t)_pid,
                fault_addr, (addr_t)PC(context->pc), current, data_abt ? fs : 0x10)
        );

    /* See ARM ARM B3-19 - only deal with prefetch aborts, translation,
     * domain and permission data aborts. Alignment and external aborts are
     * not currently recoverable.
     */
    if (data_abt)
    {
        switch(fs) {
            /* Allowed abort exception types */
            case 7: case 5: case 15: case 13: case 9: case 11:
                break;
            case 2:     /* Terminal exception, not recoverable */
            default:    /* Unhandled exception */
            {
                bool is_kernel = ((context->cpsr & CPSR_MODE_MASK) != CPSR_USER_MODE);

                if (is_kernel) {
                    panic("kernel raised data abort exception: @ %p, ip = %p, fs = %ld\n",
                            fault_addr, (addr_t)context->pc, fs);
                }

                current->arch.misc.exception.exception_continuation = ASM_CONTINUATION;
                current->arch.misc.exception.exception_context = context;
                send_exception_ipc(0x100 + fs, (word_t)fault_addr,
                        context, ASM_CONTINUATION);
            }
        }
    }

    space_t *space = current->get_space();

    if (EXPECT_FALSE(space == NULL))
    {
        space = get_kernel_space();
    }
    else
    {
        /* PID relocation */
        if ((word_t)fault_addr < PID_AREA_SIZE)
        {
            word_t pid = space->get_pid();
            fault_addr = (addr_t)((word_t)fault_addr + (pid << 25));
            //printf("pid relocated %d -> %p\n", pid, fault_addr);
        }
    }

    bool is_kernel = ((context->cpsr & CPSR_MODE_MASK) != CPSR_USER_MODE);
    word_t fault_section = (word_t)fault_addr >> PAGE_BITS_1M;

    if (EXPECT_FALSE(is_kernel)) {
        /* Special case when UTCB sections faulted on */
        if (((word_t)fault_addr >= UTCB_AREA_START) &&
                ((word_t)fault_addr < UTCB_AREA_END))
        {
            pgent_t cpd_entry = get_cpd()[fault_section];

            // TRACEF("utcb space = %p\n", utcb_space);
            if (cpd_entry.raw && ((cpd_entry.raw & 0xf0000003) == 0xf0000000)) {
                space_t * utcb_space = (space_t*)cpd_entry.raw;
                get_arm_fass()->activate_other_domain(utcb_space);
                /* NB. We don't add utcb domain's section to dirty set */
                return;
            }
            TRACEF("utcb fault, addr = %p, tcb = %p, space = %p\n",
                            fault_addr, current, space);
            TRACEF("  entry = %lx (%lx), currdomain = %lx\n",
                            cpd_entry.raw, cpd_entry.get_domain(), current_domain);
            TRACEF("  * pc = %p, fs = %x, fdomain = %lx\n", (addr_t)PC(context->pc),
                            fs, (fault_status & 0xf0) >> 4);
            return;
        }
    }

    bool is_valid = false;
    if (space->is_user_area(fault_addr))
    {
        arm_domain_t fault_domain;
        /*
         * Cases -= Shrd domains:
         *  Fault since thread no access to domain
         *  Fault since page not mapped
         *      - Valid domain
         *      - No domain
         */
        pgent_t cpd_entry = get_cpd()[fault_section];
        fault_domain = cpd_entry.get_domain();

        if (fault_domain != current_domain)
        {
            /* if a domain fault */
            if (!(space->get_domain_mask() & (3UL << (fault_domain*2))))
            {
                space_t *share_space = get_arm_fass()->get_space(fault_domain);

//printf("check domain is shared: %p  (me %p, dspc %p)\n", fault_addr, space, share_space);
                if (share_space && share_space->is_client_space(space))
                {
                    //printf(" - fault at %p in spc %p  ip = %p\n", fault_addr,
                    //        space, (addr_t)PC(context->pc));
                    //printf(" - ### share space %p! fs = %d (%lx - %d:%d)\n", share_space, fs,
                    //        space->get_domain_mask(), space->get_domain(), share_space->get_domain());
                    bool manager = space->is_manager_of_domain(share_space);
                    space->add_domain_access(fault_domain, manager);
                    current_domain_mask = space->get_domain_mask();
                    domain_dirty |= current_domain_mask;
                    return;
                }
            }

            pgent_t *pg = space->pgent((word_t)fault_addr >> PAGE_BITS_TOP_LEVEL);
//printf("check for shared domain: %p,  -- %p/%lx\n", fault_addr, pg, pg->raw);
            if (pg->is_valid(space, pgent_t::size_level0))
            {
                pgent_t section = *(pg->subtree(space, pgent_t::size_level0)->next(
                            space, pgent_t::size_1m,
                            ((word_t)fault_addr & (PAGE_SIZE_TOP_LEVEL-1)) >> PAGE_BITS_1M));

                //printf("  - look for spc: %p = %lx\n", pg->subtree(space, pgent_t::size_level0)->next(
                //            space, pgent_t::size_1m,
                //            ((word_t)fault_addr & (PAGE_SIZE_TOP_LEVEL-1)) >> PAGE_BITS_1M),  section.raw);
                if (EXPECT_FALSE(section.is_window(space, pgent_t::size_1m)))
                {
                    space_t *share_space = (space_t*)section.get_window(space);
                    pgent_t *share_pg = share_space->pgent((word_t)fault_addr >> PAGE_BITS_TOP_LEVEL);

                    //printf("  - fault at %p in spc %p  ip = %p  shr : %p\n", fault_addr,
                    //        space, (addr_t)PC(context->pc), share_space);

                    if (share_pg->is_valid(share_space, pgent_t::size_level0))
                    {
                        pgent_t share_sec = *(share_pg->subtree(share_space, pgent_t::size_level0)->next(
                                    share_space, pgent_t::size_1m,
                                    ((word_t)fault_addr & (PAGE_SIZE_TOP_LEVEL-1)) >> PAGE_BITS_1M));

                        if (share_sec.is_valid(share_space, pgent_t::size_1m))
                        {
                            arm_domain_t share_domain = share_space->get_domain();
                            bool manager = space->is_manager_of_domain(share_space);

                            if (share_domain == INVALID_DOMAIN)
                            {
                                //printf(" -- shr domain activate\n");
                                get_arm_fass()->activate_other_domain(share_space);

                                /* get domain again if we activate_other_domain */
                                share_domain = share_space->get_domain();

                                space->add_domain_access(share_domain, manager);

                                current_domain_mask = space->get_domain_mask();
                                domain_dirty |= current_domain_mask;

                                /* Update the faulter domain's set of sections in the CPD */
                                get_arm_fass()->add_set(share_domain, fault_section);
                                /* Update the CPD */

                                share_sec.set_domain(share_domain);

                                /* keep TLB in sync if replacing entries */
                                if (get_cpd()[fault_section].is_valid(space, pgent_t::size_1m)) {
                                    arm_cache::tlb_flush();
                                }
                                get_cpd()[fault_section] = share_sec;
                                return;
                            }
                            //printf(" -- handle dm flt\n");
                            if (fault_domain != share_domain)
                            {
                                space->add_domain_access(share_domain, manager);
                                current_domain_mask = space->get_domain_mask();
                                domain_dirty |= current_domain_mask;

                                current_domain = share_domain;
                                fass_update_pds(share_space, cpd_entry, share_sec, fault_section);
                                current_domain = space->get_domain();
                                return;
                            }
                        }
                        is_valid = true;
                    }

                    /* translation fault in shared area */
                    if (section.is_callback()) {
                        current->set_preempted_ip( current->get_user_ip() );
                        current->get_utcb()->share_fault_addr = (word_t)fault_addr;
                        //printf(" - restart ip = %p\n", (void*)current->get_preempt_callback_ip());
                        current->set_user_ip( current->get_preempt_callback_ip() );
                        return;
                    }
                    goto handle_fault;
                }
            }
        }
    }

    if (fass_sync_address(space, fault_addr, &is_valid))
        return;

handle_fault:
    current->arch.misc.fault.fault_addr = fault_addr;
    current->arch.misc.fault.page_fault_continuation = ASM_CONTINUATION;

    TRACEPOINT(ARM_PAGE_FAULT,
            space_t *_s = current->get_space();
            word_t _pid = (word_t)fault_addr;
            if (_s && ((word_t)fault_addr < PID_AREA_SIZE))
            {
                _pid = _s->get_pid();
                _pid = (word_t)fault_addr + (_pid << 25);
            }
            printf("PF @ %p [%p], pc = %p, tcb = %p, fs = %x\n", (addr_t)_pid,
                fault_addr, (addr_t)PC(context->pc), current, fs)
        );

    space->handle_pagefault(fault_addr, (addr_t)PC(context->pc),
                    data_abt ?
                        (is_valid ? space_t::write : space_t::read)
                        : space_t::execute,
                    is_kernel, finish_arm_memory_abort);

    /*
     * We only return from space->handle_pagefault() on a kernel fault
     * where fault_addr != user_address. Otherwise, continuation
     * finish_arm_memory_abort() is called.
     */
    /* return value is ignored here since we return anyway */
    (void)fass_sync_address(space, fault_addr);

    return;
}

/* Continue after handling a user pagefault (from arm_memory_abort) */
CONTINUATION_FUNCTION(finish_arm_memory_abort)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();

    addr_t fault_addr = current->arch.misc.fault.fault_addr;

    if (!space) {
        space = get_kernel_space();
    }

    /* return value is ignored here since we return anyway */
    (void)fass_sync_address(space, fault_addr);

    ACTIVATE_CONTINUATION(current->arch.misc.fault.page_fault_continuation);
}

void fass_update_pds (space_t * space, pgent_t cpg, pgent_t spg, word_t fault_section)
{
    bool cpd_section_valid = cpg.is_valid(get_kernel_space(), pgent_t::size_1m);
    arm_domain_t cpd_section_domain = cpg.get_domain();
    /* Clean the kernel's UTCB synonym if necessary */
    bool need_flush = (cpd_section_valid &&
            TEST_DOMAIN(domain_dirty, cpd_section_domain));

    /* If there is an existing mapping in the CPD for this section,
     * remove it from the owner domain's set of sections in the CPD
     */
    if (cpd_section_valid)
        get_arm_fass()->remove_set(cpd_section_domain, fault_section);

    /* Update the faulter domain's set of sections in the CPD */
    get_arm_fass()->add_set(current_domain, fault_section);

    /* Update the CPD */
    spg.set_domain(current_domain);
    get_cpd()[fault_section] = spg;

    if (need_flush) {
        bool flush = 1;

        space_t *old_space = get_arm_fass()->get_space(cpd_section_domain);
        if (old_space) {
            word_t old_vspace = old_space->get_vspace();

            if (old_vspace && (space->get_vspace() == old_vspace))
                flush = 0;
        }

        get_arm_fass()->clean_all(flush);
        domain_dirty |= current_domain_mask;
    } else {
        // need to remove all entries corresponding to fault section
        // from TLB. Cheaper to flush than to iterate over section
        // on SA-1100 (FIXME - check other archs' MMU).
        arm_cache::tlb_flush();
    }
}

bool fass_sync_address (space_t * space, addr_t fault_addr, bool * is_valid)
{
    /* Try lookup the mapping */
    pgent_t *pg = space->pgent((word_t)fault_addr >> PAGE_BITS_TOP_LEVEL);
    if (EXPECT_FALSE(!pg->is_valid(space, pgent_t::size_level0)))
        return false;
    pgent_t section = *(pg->subtree(space, pgent_t::size_level0)->next(
                space, pgent_t::size_1m,
                ((word_t)fault_addr & (PAGE_SIZE_TOP_LEVEL-1)) >> PAGE_BITS_1M));
    if (EXPECT_FALSE(!section.is_valid(space, pgent_t::size_1m)))
        return false;
    if (section.is_subtree(space, pgent_t::size_1m))
    {
        pgent_t& leaf =
            *section.subtree(space, pgent_t::size_1m)->next(
                    space, pgent_t::size_min,
                    ((word_t)fault_addr & (PAGE_SIZE_1M-1)) >> ARM_PAGE_BITS);
        if (leaf.l2.fault.zero == 0)
            return false;
    }

    /* Does the faulter's space's section match that in the CPD for the fault
     * address?
     */

    word_t fault_section = (word_t)fault_addr >> PAGE_BITS_1M;
    pgent_t cpg = get_cpd()[fault_section];

    if (EXPECT_TRUE((cpg.get_domain() != current_domain) &&
                    space->is_user_area(fault_addr)))
    {
        fass_update_pds(space, cpg, section, fault_section);
        return true;
    }
    /* this is the uncommon case so take the performance hit (dereferencing the pointer) here */
    if (is_valid)
        *is_valid = true;
    return false;
}

