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
 * Copyright (c) 2005-2006, National ICT Australia (NICTA)
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
 * Description:   architecture independent parts of space_t
 */

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <kdb/tracepoints.h>
#include <space.h>
#include <tcb.h>
#include <rootserver.h>
#include <schedule.h>
#include <syscalls.h>
#include <threadstate.h>
#include <arch/syscalls.h>
#include <cache.h>
#include <arch/special.h>
#include <bitmap.h>
#include <linear_ptab.h>
#include <generic/lib.h>
#include <smallalloc.h>
#include <profile.h>

space_t * roottask_space;

DECLARE_TRACEPOINT (PAGEFAULT_USER);
DECLARE_TRACEPOINT (PAGEFAULT_KERNEL);
DECLARE_TRACEPOINT (SYSCALL_SPACE_CONTROL);

DECLARE_KMEM_GROUP (kmem_spaceids);
DECLARE_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_stack);

space_t * global_spaces_list = NULL;
spinlock_t spaces_list_lock;

small_alloc_t space_pool;

/* Table containing mappings from spaceid_t to space_t* */
spaceid_lookup_t space_lookup;


void SECTION(SEC_INIT) init_spaceids(void)
{
    void * new_table;
    word_t max_spaceids = get_init_data()->max_space_ids;
    space_pool.init(kmem_space, sizeof(space_t), max_spaceids);
    TRACE_INIT("Init spaceids for %d spaces\n", max_spaceids);
    ASSERT(ALWAYS, max_spaceids >= 1);

    new_table = kmem.alloc(kmem_spaceids,
                           sizeof(space_t*) * max_spaceids,
                           true);
    ASSERT(ALWAYS, new_table);

    space_lookup.init(new_table, max_spaceids);
}

/**
 * Handle a pagefault as appropriate.
 * For a user fault this means a pagefault IPC, for a kernel fault this
 * means syncing the kernel space or mapping the dummy UTCB.
 *
 * This is a control function for a user fault, so the continuation will be
 * activated instead of returning. However for a kernel fault this is not a
 * control function, so the continuation will be ignored and a normal return
 * will be performed.
 *
 * @param addr The address of the fault
 * @param ip The address of the instructin that caused the fault
 * @param access The type of fault (RWX)
 * @param kernel Whether the fault came from kernel or user mode
 * @param continuation The continuation to activate upon completion
 * for a user fault, ignored for a kernel fault
 */
void
generic_space_t::handle_pagefault(addr_t addr, addr_t ip, access_e access,
                                  bool kernel, continuation_t continuation)
{
    tcb_t * current = get_current_tcb();

    if (EXPECT_TRUE(!kernel)) {
        TRACEPOINT_TB (PAGEFAULT_USER,
                       printf("user %s pagefault by %t at %p, ip=%p, ksp=%p\n",
                              access == generic_space_t::write     ? "write" :
                              access == generic_space_t::read      ? "read"  :
                              access == generic_space_t::execute   ? "execute" :
                              access == generic_space_t::readwrite ? "read/write" :
                              "unknown",
                              current, addr, ip, &current),
                       "user page fault at %x (current=%t, ip=%x, access=%x)",
                       (word_t)addr, (word_t)current, (word_t)ip, (int) access);

        // if we have a user fault we may have a stale partner
        current->set_partner(NULL);

        current->send_pagefault_ipc (addr, ip, access, continuation);
    }
    else {
        /* kernel fault */
        bool user_area = is_user_area(addr);

        if (user_area && current->user_access_enabled())
        {
            TRACEPOINT_TB (PAGEFAULT_KERNEL,
                          printf ("kernel user access fault "
                                       "%p @ %p, ip=%p, ksp=%p\n",
                                        this, addr, ip, &current),
                              "kernel user fault at %x "
                              "(current=%t, ip=%x, space=%p)",
                              (word_t)addr, (word_t)current, (word_t)ip, (word_t)this);

            TCB_SYSDATA_USER_ACCESS(current)->fault_address = addr;
            continuation_t cont = current->user_access_continuation();
            /* Disable user address access */
            current->clear_user_access();

            ACTIVATE_CONTINUATION(cont);
            NOTREACHED();
        }

        TRACEPOINT_TB (PAGEFAULT_KERNEL,
                      printf ("kernel pagefault in space "
                                   "%p @ %p, ip=%p, type=%x, ksp=%p\n",
                                    this, addr, ip, access, &current),
                          "kernel page fault at %x "
                          "(current=%t, ip=%x, space=%p)",
                          (word_t)addr, (word_t)current, (word_t)ip, (word_t)this);

        if (!user_area) {
            if (sync_kernel_space(addr)) {
                return;
            }
        }
        else {
            printf("kernel access raised user pagefault @ %p, ip=%p, "
                    "space=%p, esp = %p\n", addr, ip, this, &current);
            printf("current tcb = %p\n", get_current_tcb());
            enter_kdebug("kpf");
        }
    }

    TRACEF("unhandled pagefault @ %p, %p\n", addr, ip);
    enter_kdebug("unhandled pagefault");

    if (current == get_idle_tcb()) {
        panic("Unhandled pagefault in idle thread.");
    }

    get_current_scheduler()->
        deactivate_sched(current, thread_state_t::halted,
                         current, (continuation_t)(-1),
                         scheduler_t::sched_default);
    NOTREACHED();
}



SYS_SPACE_CONTROL (spaceid_t space_id, word_t control, clistid_t clist_id,
                   fpage_t utcb_area, word_t space_resources)
{
    PROFILE_START(sys_space_ctrl);
    continuation_t continuation = ASM_CONTINUATION;
    space_t * space;
    clist_t * clist;

    TRACEPOINT (SYSCALL_SPACE_CONTROL,
                printf("SYS_SPACE_CONTROL: space=%d, control=%p, clist=%d, "
                       "utcb_area=%p, spc_resc=%p\n",  space_id.get_spaceno(),
                       control, clist_id.get_raw(), utcb_area.raw, space_resources));

    tcb_t *current = get_current_tcb();

    // Check privilege
    if (EXPECT_FALSE (! is_privileged_space(get_current_space())))
    {
        current->set_error_code (ENO_PRIVILEGE);
        goto error_out;
    }

    // Check for valid control word
    if ( (control & SPACE_CONTROL_DELETE) &&
            (control & (SPACE_CONTROL_NEW | SPACE_CONTROL_RESOURCES)) )
    {
        current->set_error_code (EINVALID_PARAM);
        goto error_out;
    }

    // Check for valid space id
    if (EXPECT_FALSE (! space_lookup.is_valid(space_id) ))
    {
        get_current_tcb ()->set_error_code (EINVALID_SPACE);
        goto error_out;
    }

    space = get_space_list()->lookup_space(space_id);

    if (control & SPACE_CONTROL_NEW)
    {
        /* Check for non existant space id */
        if (EXPECT_FALSE( space != NULL ))
        {
            current->set_error_code (EINVALID_SPACE);
            goto error_out;
        }

        /* Get the clist */
        clist = get_clist_list()->lookup_clist(clist_id);
        if (EXPECT_FALSE( clist == NULL ))
        {
            current->set_error_code (EINVALID_CLIST);
            goto error_out;
        }

        /*
         * Space does not exist.  Create it.
         */

        /* @todo FIXME: The is_user_area should probably be a static
         * function, for now though we call it on a space that we know
         * exists, e.g: kernel space. This should really be refactored
         * though, so that control word is handled before the space is
         * init, then it makes sense for is_user_area to be a class
         * method - ???.
         */

        bool utcb_area_invalid;

#ifdef NO_UTCB_RELOCATE    /* UTCB is kernel allocated */
        utcb_area_invalid = (utcb_area.raw != 0);
#else   /* UTCB is user allocated */
        utcb_area_invalid = (UTCB_SIZE > utcb_area.get_size()) ||
                  (!get_kernel_space()->is_user_area(utcb_area));
#endif

        if (utcb_area_invalid) {
            /* Invalid UTCB area */
            current->set_error_code (EUTCB_AREA);
            goto error_out;
        }

        /* ok, everything seems fine, now setup the space */
        space = allocate_space(space_id, clist);

        if (EXPECT_FALSE(!space || (space->init(utcb_area) == false)))
        {
            if (space) {
                space->free();
                free_space(space);
            }
            current->set_error_code (ENO_MEM);
            goto error_out;
        }
    }
    else if (control & SPACE_CONTROL_DELETE)
    {
        /* Check that space exists */
        if (EXPECT_FALSE ( space == NULL ))
        {
            get_current_tcb ()->set_error_code (EINVALID_SPACE);
            goto error_out;
        }

        if (EXPECT_FALSE ( space->get_thread_count() != 0 ))
        {
            current->set_error_code (ESPACE_NOT_EMPTY);
            goto error_out;
        }

        space->free();
        free_space(space);

        /* space control returns here on delete */
        PROFILE_STOP(sys_space_ctrl);
        return_space_control (1, 0, continuation);
    }

    if (control & SPACE_CONTROL_RESOURCES)
    {
        word_t old_resources = space->space_control (space_resources);

        PROFILE_STOP(sys_space_ctrl);
        return_space_control (1, old_resources, continuation);
    }

    if (control & SPACE_CONTROL_SPACE_PAGER)
    {
        spaceid_t pager = spaceid(current->get_mr(0));

        if (EXPECT_FALSE(!get_space_list()->is_valid(pager))) {
            current->set_error_code (EINVALID_SPACE);
            goto error_out;
        }

        space->set_space_pager(pager);
    }

    PROFILE_STOP(sys_space_ctrl);
    return_space_control (1, 0, continuation);

error_out:
    PROFILE_STOP(sys_space_ctrl);
    return_space_control (0, 0, continuation);
}


/**
 * allocate_space: allocates a new space_t
 */
space_t * allocate_space(spaceid_t space_id, clist_t *clist)
{
    //TRACEF("Allocating space %d\n", id);

    space_t * space = (space_t *) space_pool.allocate();
    if (!space)
        return NULL;

    if (!space->allocate_page_directory()) {
        space_pool.free(space);
        return NULL;
    }

    space->enqueue_spaces();
    clist->add_space(space);

    space->set_space_id(space_id);
    get_space_list()->add_space(space_id, space);

    space->init_security(clist);

#ifdef CONFIG_MAX_NUM_ASIDS
    /* Set space ASID to be invalid */
    space->get_asid()->init();
#endif

    return space;
}

/**
 * free_space: frees a previously allocated space
 */
void free_space(space_t * space)
{
    ASSERT(DEBUG, space);

    //TRACEF("Freeing %d\n", space->get_space_id());
    space->dequeue_spaces();
    space->free_page_directory();
    space->free_security();

    get_space_list()->remove_space(space->get_space_id());
    space_pool.free(space);
}

void generic_space_t::free()
{
    free_utcb_area_memory();

    // Unmap everything, including UTCB
    fpage_t fp = fpage_t::complete();
    unmap_fpage(fp, true);

#ifdef HAVE_ARCH_FREE_SPACE
    arch_free ();
#endif
}

void generic_space_t::free_utcb_area_memory()
{
    /* ARMv5 frees its UTCBs as the threads are freed in free_thread_resources
     * This is feasible for ARM as it needs a UTCB bitmap anyway
     */
#if !defined(ARCH_ARM) || (CONFIG_ARM_VER >= 6)
    /* Unmap and free the UTCBs.. walk ptab, to see if any pages are mapped */
    fpage_t utcb_area = get_utcb_area();
    pgent_t::pgsize_e size = pgent_t::size_max;
    addr_t utcb = utcb_area.get_base();

    while (utcb_area.is_range_overlapping(utcb, addr_offset (utcb, page_size(UTCB_AREA_PGSIZE))))
    {
        pgent_t * pgent = this->pgent(0)->next(this, size, page_table_index(size, utcb));

        while (size > UTCB_AREA_PGSIZE && pgent->is_valid(this, size))
        {
            ASSERT(DEBUG, pgent->is_subtree(this, size));

            pgent = pgent->subtree(this, size);
            size--;
            pgent = pgent->next(this, size, page_table_index(size, utcb));
        }

        if (pgent->is_valid(this, size))
        {
            free_utcb_page(pgent, size, utcb);
        }
        utcb = addr_offset (utcb, page_size(UTCB_AREA_PGSIZE));
        size = pgent_t::size_max;
    }
#endif
}

void generic_space_t::free_utcb_page (pgent_t * pg, pgent_t::pgsize_e pgsize, addr_t vaddr)
{
    addr_t kaddr;
    word_t pagesize = 1UL << page_shift(pgsize);

    /* make sure vaddr is page aligned */
    ASSERT(DEBUG, !((word_t)vaddr & (pagesize-1)));

    /* flush the page on architectures with a virtual cache
     * so that we don't corrupt the freelist
     */
    cache_t::invalidate_virtual_alias(vaddr, pagesize);

    kaddr = ram_to_virt(pg->address(this, pgsize));

    //TRACEF("Freeing Kernel memory %p %p\n", vaddr, kaddr);

    /* flush from TLB */
    flush_tlbent_local(get_current_space(), vaddr, page_shift(pgsize));

    /* remove mapping */
    pg->clear (this, pgsize, true, vaddr);

    /* XXX flush remote cpu */

    /* free physical memory */
    kmem.free(kmem_utcb, kaddr, pagesize);
}

/* Map a region 
 *
 * Uses max page size available, but does not map any 
 * excess memory (beyond smallest page size granularity)
 */
static bool SECTION(SEC_INIT)
map_region (space_t * space, word_t vaddr, word_t paddr, word_t size)
{
    phys_desc_t phys_desc;
    word_t supported_sizes = HW_VALID_PGSIZES;
    word_t pgsize, pagesize = 0;
    fpage_t fpg;

    //TRACEF("Map region %p -> %p %lx bytes\n", vaddr, paddr, size);
    /* page align all addresses and size */
    size = (word_t)addr_align_up((addr_t)(((word_t)vaddr) + size), page_size(pgent_t::size_min)) - vaddr;
    vaddr = (word_t)addr_align((addr_t)vaddr, page_size(pgent_t::size_min));
    paddr = (word_t)addr_align((addr_t)paddr, page_size(pgent_t::size_min));

    while (size != 0)
    {
        /* find pagesize to use for this portion of the mapping */
        for (pgsize = BITS_WORD; pgsize; pgsize--) {
            pagesize = 1UL << pgsize;
            // Ignore sizes not supported by the kernel
            if (!(pagesize & supported_sizes)) {
                continue;
            }

            // Must be less that the size we're trying to map
            if (size < pagesize) {
                continue;
            }

            // phys and virt addrs must be aligned
            if (paddr & (pagesize-1)) {
                continue;
            }
            if (vaddr & (pagesize-1)) {
                continue;
            }
            break;
        }

        fpg.set(vaddr, pgsize, true, true, true);

        phys_desc.set_base(paddr);
        phys_desc.set_attributes(l4mem_default);

        //TRACEF("Map page %p -> %p %lx bytes\n", vaddr, paddr, page_size(pgsize));
        get_current_tcb()->sys_data.set_action
            (tcb_syscall_data_t::action_map_control);
        if (!space->map_fpage(phys_desc, fpg))
        {
            return false;
        }

        paddr += pagesize;
        vaddr += pagesize;
        size  -= pagesize;
    }
    return true;
}

void SECTION(SEC_INIT) setup_initial_mappings (space_t * space)
{
    const root_server_t *server = &get_init_data()->root_server;

    for (word_t i = 0; i < server->num_maps; i++) {
        if (!map_region(space, 
                    server->regions[i].virt,
                    server->regions[i].phys,
                    server->regions[i].size)) {
            panic("Error setting up initial mappings: V:%lx P:%lx S:%lx (%d)",
                  server->regions[i].virt,
                  server->regions[i].phys,
                  server->regions[i].size,
                i);
        }
    }
}
