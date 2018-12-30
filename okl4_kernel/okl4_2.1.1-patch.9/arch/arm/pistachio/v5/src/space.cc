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
 * Description: ARM space_t implementation.
 */

#include <l4.h>
#include <debug.h>                      /* for UNIMPLEMENTED    */
#include <linear_ptab.h>
#include <space.h>              /* space_t              */
#include <tcb.h>
#include <arch/pgent.h>
#include <arch/memory.h>
#include <config.h>
#include <arch/fass.h>
#include <cpu/syscon.h>
#include <arch/special.h>
#include <bitmap.h>
#include <generic/lib.h>


EXTERN_KMEM_GROUP(kmem_utcb);
EXTERN_KMEM_GROUP(kmem_pgtab);


#if UTCB_AREA_SECTIONS != CONFIG_MAX_SPACES
#error CONFIG_MAX_SPACES must be the same as UTCB_AREA_SECTIONS on the ARM architecture
#endif

/* The kernel space is statically defined beause it is needed
 * before the virtual memory has been setup or the kernel
 * memory allocator.
 */
ALIGNED(ARM_HWL1_SIZE) char UNIT("kspace") _kernel_space_pagetable[ARM_HWL1_SIZE];

/* allocators for pagetable levels if they are needed */
#if (ARM_L0_SIZE < KMEM_CHUNKSIZE)
small_alloc_t l0_allocator;
#endif
#if (ARM_L1_SIZE < KMEM_CHUNKSIZE)
small_alloc_t l1_allocator;
#endif

void SECTION(".init") init_kernel_space()
{
    space_t * kspace = get_kernel_space();

    TRACE_INIT("Initializing kernel space @ %p...\n\r", get_kernel_space());

#if (ARM_L0_SIZE < KMEM_CHUNKSIZE)
    l0_allocator.init(kmem_pgtab, ARM_L0_SIZE);
#endif
#if (ARM_L1_SIZE < KMEM_CHUNKSIZE)
    l1_allocator.init(kmem_pgtab, ARM_L1_SIZE);
#endif


#if 0
    fpage.set(USER_KIP_PAGE, KIP_KIP_BITS, true, true, true);
    kspace->set_kip_area(fpage);
    fpage.set(UTCB_AREA_START, UTCB_AREA_BITS, true, true, true);
    kspace->set_utcb_area(fpage);
#endif

    kspace->init_kernel_mappings();

    kspace->set_space_id(spaceid_t::kernelspace());

    get_arm_fass()->init();

    kspace->enqueue_spaces();

}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init") generic_space_t::init_kernel_mappings()
{
    /* Kernel space's mappings already setup in initial root page table */
}

/**
 * initialize a space
 *
 * @param utcb_page     fpage describing location of UTCB area
 */
bool generic_space_t::init (fpage_t utcb_page)
{
    ((space_t *)this)->set_domain(INVALID_DOMAIN);

    /* initialize UTCB pgentry in space */
    ((space_t *)this)->pgent_utcb()->clear(this, pgent_t::size_1m, true);

    /* Ignore user provided kip/user area */
    this->utcb_area.set(UTCB_AREA_START + get_space_id().get_spaceno()*ARM_SECTION_SIZE, BITS_WORD - ARM_SECTION_BITS, true, true, true);
    bitmap_init(((space_t *)this)->get_utcb_bitmap(), UTCB_BITMAP_LENGTH, true);
    bitmap_init(((space_t *)this)->get_client_spaces_bitmap(), CONFIG_MAX_SPACES, 0);
    bitmap_init(((space_t *)this)->get_shared_spaces_bitmap(), CONFIG_MAX_SPACES, 0);

    word_t section = ( UTCB_AREA_START/ARM_SECTION_SIZE +
            (get_space_id().get_spaceno()) );
    /* We have invalid domain, so store the space_t pointer in the
     * section */
    pgent_t entry;
    entry.raw = (word_t)this;
    get_cpd()[section] = entry;

    return true;
}

/**
 * Clean up a Space
 */
void generic_space_t::arch_free()
{
    word_t section;
    // TRACEF("freed utcb_section %d for %p\n", get_space_id(), this);

    section = ( UTCB_AREA_START/ARM_SECTION_SIZE + (get_space_id().get_spaceno()) );

    pgent_t entry, *pg;
    entry.raw = 0;
    /* We need to clear the space_t pointer in the cpd section */
    get_cpd()[section] = entry;

    pg = ((space_t *)this)->pgent_utcb();
    //printf("[a] pg=%p, *pg=%lx\n", pg, *pg);
    if (pg->raw) {
        pg->remove_subtree (this, pgent_t::size_1m, true);
    }

    space_t * spc = (space_t*)this;

    /* Clean the domain if needed */
    if (spc->get_domain() != INVALID_DOMAIN)
        get_arm_fass()->free_domain(spc);

    spc->flush_sharing_on_delete();
}


/**
 * Allocate a UTCB
 * @param tcb   Owner of the utcb
 */
utcb_t * generic_space_t::allocate_utcb(tcb_t * tcb)
{
    ASSERT (DEBUG, tcb);
    bitmap_t * utcb_bitmap = ((space_t *)this)->get_utcb_bitmap();
    int pos = bitmap_findfirstset(utcb_bitmap, UTCB_BITMAP_LENGTH);

    if (pos == -1)
    {
        get_current_tcb()->set_error_code (EUTCB_AREA);
        return (utcb_t *)0;
    }

    bitmap_clear(utcb_bitmap, pos);

    utcb_t * utcb = (utcb_t*)( UTCB_AREA_START +
                    (get_space_id().get_spaceno() * ARM_SECTION_SIZE) +
                    (pos * UTCB_SIZE) );

    /* Try lookup the UTCB page for this utcb */
    space_t * space = (space_t *)this;
    pgent_t level1 = *space->pgent_utcb();

    bool is_valid = true;

    if (EXPECT_TRUE(level1.is_valid(space, pgent_t::size_1m)))
    {
        if (EXPECT_TRUE(level1.is_subtree(space, pgent_t::size_1m))) {
            pgent_t leaf =
                    *level1.subtree(space, pgent_t::size_1m)->next(
                                        space, UTCB_AREA_PGSIZE,
                                        ((word_t)utcb & (PAGE_SIZE_1M-1)) >> ARM_PAGE_BITS
                                        );
            if (leaf.l2.fault.zero == 0)
                is_valid = false;
        } else {
            WARNING("1MB page in UTCB area");
            return (utcb_t *)0;
        }
    } else {
        is_valid = false;
    }

    /* No UTCB page present for this utcb, alloc and map a new one */
    if (!is_valid) {
        addr_t page = kmem.alloc (kmem_utcb, UTCB_AREA_PAGESIZE, true);
        if (page == NULL) {
            get_current_tcb()->set_error_code(ENO_MEM);
            return NULL;
        }
        arm_cache::cache_flush_d_ent(page, UTCB_AREA_PAGEBITS);

        if (space->add_mapping((addr_t)addr_align(utcb, UTCB_AREA_PAGESIZE),
                               virt_to_ram(page),
                               UTCB_AREA_PGSIZE, space_t::read_write, false) == false)
        {
            kmem.free(kmem_utcb, page, UTCB_AREA_PAGESIZE);
            get_current_tcb()->set_error_code(ENO_MEM);
            return NULL;
        }
    } else {
        /* Clear UTCB if page already present.  */
        word_t *source = (word_t*)utcb;
        word_t *source_end = source + (UTCB_SIZE / sizeof(word_t));

        while(source < source_end) {
            *source++ = 0x0;
        }
    }
//TRACEF("Using UTCB %ld (%p) for tcb %p  (%s)\n", pos, utcb, tcb,
//              is_valid ? "valid map" : "new map");
    tcb->set_utcb_location((word_t)utcb);
    return utcb;
}

/**
 * Free a UTCB
 * @param utcb   The utcb
 */
void generic_space_t::free_utcb(utcb_t * utcb)
{
    /*
     * Sometimes thread creation fails due to out of memory,
     * this can cause utcb == NULL
     */
    if (utcb == NULL) {
        return;
    }
    /* We have to free the thread's UTCB */
    space_t *space = (space_t *)this;

    /* Can't free UTCBs of kernel space */
    ASSERT(DEBUG, space);

    word_t offset, utcb_num;

    offset = (word_t)utcb - UTCB_AREA_START -
            (space->get_space_id().get_spaceno() * ARM_SECTION_SIZE);
    utcb_num = offset >> UTCB_BITS;

    /* Free this utcb */
    bitmap_set(space->get_utcb_bitmap(), utcb_num);

    /* Search to see if all UTCBs in this page are now free */
    offset = (offset & (~(UTCB_AREA_PAGESIZE-1)));
    utcb_num = offset >> UTCB_BITS;

    if (EXPECT_TRUE(!bitmap_israngeset(space->get_utcb_bitmap(), utcb_num,
                utcb_num + (UTCB_AREA_PAGESIZE>>UTCB_BITS) - 1)))
    {
        return;
    }

    pgent_t *pg;
    space_t *mapspace = space;

    pg = mapspace->pgent_utcb();
    if (pg->is_valid(mapspace, pgent_t::size_1m) &&
        pg->is_subtree(mapspace, pgent_t::size_1m))
    {
        pgent_t *subtree = pg->subtree(mapspace, pgent_t::size_1m);
        word_t utcb_section = ((word_t)utcb &(PAGE_SIZE_1M-1)) >> ARM_PAGE_BITS;

        pg = subtree->next(mapspace, UTCB_AREA_PGSIZE, utcb_section);
    } else {
        pg = NULL;
    }

    if (!pg || !pg->is_valid(mapspace, pgent_t::size_4k) ) {
        enter_kdebug("UTCB mapping not found");
    }

    addr_t page = addr_align(utcb, UTCB_AREA_PAGESIZE);

    /* pass in page aligned address! */
    mapspace->free_utcb_page(pg, UTCB_AREA_PGSIZE, page);
}


void generic_space_t::flush_tlb(space_t *curspace)
{
    if (((space_t *)this)->get_domain() != INVALID_DOMAIN)
    {
        domain_dirty = current_domain_mask;
        arm_cache::cache_flush();
        arm_cache::tlb_flush();
    }
}

void generic_space_t::flush_tlbent_local(space_t *curspace, addr_t vaddr, word_t log2size)
{
    vaddr = addr_align(vaddr, 1 << log2size);
    arm_cache::cache_flush_ent(vaddr, log2size);
    arm_cache::tlb_flush_ent(vaddr, log2size);
}

bool generic_space_t::allocate_page_directory()
{
#if (ARM_L0_SIZE < KMEM_CHUNKSIZE)
    pdir = (pgent_t *)l0_allocator.allocate();
#else
    pdir = (pgent_t *)kmem.alloc(kmem_pgtab, ARM_L0_SIZE, true);
#endif
    if (!pdir)
        return false;

    return true;
}

void generic_space_t::free_page_directory()
{
#if (ARM_L0_SIZE < KMEM_CHUNKSIZE)
    l0_allocator.free(pdir);
#else
    kmem.free(kmem_pgtab, pdir, ARM_L0_SIZE);
#endif
}

/**
 * Set up hardware context to run the tcb in this space.
 */
void generic_space_t::activate(tcb_t *tcb)
{
#if defined(CONFIG_ENABLE_FASS)
    if (this != get_kernel_space())
    {
        USER_UTCB_REF = tcb->get_utcb_location();

        get_arm_globals()->current_clist = this->get_clist();
        get_arm_fass()->activate_domain((space_t *)this);
        /* Set PID of new space */
        write_cp15_register(C15_pid, C15_CRm_default, C15_OP2_default,
                            ((space_t *)this)->get_pid() << 25);
    }
    else
    {
        get_arm_fass()->activate_domain((space_t *)NULL);
    }
#else
#error Not implemented.
#endif
}

/*
 *  Domain Sharing
 */

/**
 * Is this space sharing the domain of target?
 */
bool space_t::is_sharing_domain(space_t *target)
{
    word_t targetid = target->get_space_id().get_spaceno();

    return bitmap_isset(this->get_shared_spaces_bitmap(), targetid);
}

/**
 * Is this space a manager of the domain of target?
 */
bool space_t::is_manager_of_domain(space_t *target)
{
    word_t targetid = target->get_space_id().get_spaceno();

    return bitmap_isset(this->get_manager_spaces_bitmap(), targetid);
}

bool space_t::add_shared_domain(space_t *space, bool manager)
{
    arm_domain_t domain = space->get_domain();

    //printf("insert shared  %p->%p : %d\n", this, space, domain);
    word_t clientid = space->get_space_id().get_spaceno();
    word_t thisid = this->get_space_id().get_spaceno();

    if (bitmap_isset(space->get_client_spaces_bitmap(), thisid)) {
//printf("already added\n");
        // XXX: We reshare the domain to sync up mappings.
        //get_current_tcb()->set_error_code(EINVALID_PARAM);
        //return false;
    }

    bitmap_set(space->get_client_spaces_bitmap(), thisid);
    bitmap_set(this->get_shared_spaces_bitmap(), clientid);
    if (manager) {
        bitmap_set(this->get_manager_spaces_bitmap(), clientid);
    } else {
        bitmap_clear(this->get_manager_spaces_bitmap(), clientid);
    }

    /* update domain masks */
    if (domain != INVALID_DOMAIN) {
        this->add_domain_access(domain, manager);
        if (space == get_current_space()) {
            current_domain_mask = space->get_domain_mask();
            domain_dirty |= current_domain_mask;
        }
    }
    return true;
}

bool space_t::remove_shared_domain(space_t *space)
{
    word_t clientid = space->get_space_id().get_spaceno();
    word_t thisid = this->get_space_id().get_spaceno();

    if (!bitmap_isset(space->get_client_spaces_bitmap(), thisid)) {
        get_current_tcb()->set_error_code(EINVALID_PARAM);
        return false;
    }

    /* unlink this space from the shared space */
    (void) this->domain_unlink(space);

    arm_domain_t domain = space->get_domain();

    /* update domain masks */
    if (domain != INVALID_DOMAIN) {
        remove_domain_access(domain);
        if (space == get_current_space()) {
            current_domain_mask = space->get_domain_mask();
        }
    }

    bitmap_clear(space->get_client_spaces_bitmap(), thisid);
    bitmap_clear(this->get_shared_spaces_bitmap(), clientid);

    return true;
}

/*
 * Remove access to this space's domain,
 * base space must have valid domain!
 */
void space_t::flush_sharing_spaces(void)
{
    word_t i;

    ASSERT(DEBUG, this->domain != INVALID_DOMAIN);

    if (bitmap_isallclear(this->get_client_spaces_bitmap(), CONFIG_MAX_SPACES)) {
        //printf("%p flush - no shares!\n", this);
        return;
    }

    //printf("%p flush - with shares!\n", this);
    for (i = 0; i < CONFIG_MAX_SPACES; i++) {
        if (bitmap_isset(this->get_client_spaces_bitmap(), i)) {
            space_t *space = get_space_list()->lookup_space(spaceid(i));

            /* Check for valid space id */
            if (EXPECT_FALSE( space == NULL ))
            {
                panic("found null space in client list");
//                printf("lazy clear spc %ld\n", i);
//                bitmap_clear(this->get_share_bitmap(), i);
//                continue;
            }

            //printf("remove shared  %p : %d\n", space, domain);
            //printf("dmsk rm: %p->%p\n", this, space);
            space->remove_domain_access(this->domain);
            if (space == get_current_space()) {
                current_domain_mask = space->get_domain_mask();
            }
        }
    }
}

void space_t::flush_sharing_on_delete(void)
{
    word_t i;

    if (bitmap_isallclear(this->get_shared_spaces_bitmap(), CONFIG_MAX_SPACES)) {
        //printf("%p flush - no shares!\n", this);
    } else {
        //printf("%p delete - WITH SHARES!\n", this);
        for (i = 0; i < CONFIG_MAX_SPACES; i++) {
            if (bitmap_isset(this->get_shared_spaces_bitmap(), i)) {
                space_t *source = get_space_list()->lookup_space(spaceid(i));

                /* Check for valid space id */
                if (EXPECT_FALSE( source == NULL ))
                {
                    panic("found null space in source list");
                    //                printf("lazy clear spc %ld\n", i);
                    //                bitmap_clear(this->get_share_bitmap(), i);
                    //continue;
                }

                //printf("remove shared  %p : %d\n", space, domain);
                //printf("dmsk rm: %p->%p\n", this, space);

                bitmap_clear(source->get_client_spaces_bitmap(), this->get_space_id().get_spaceno());
            }
        }
    }
    if (bitmap_isallclear(this->get_client_spaces_bitmap(), CONFIG_MAX_SPACES)) {
        //printf("%p flush - no shares!\n", this);
    } else {
        //printf("%p delete - with shares!\n", this);
        for (i = 0; i < CONFIG_MAX_SPACES; i++) {
            if (bitmap_isset(this->get_client_spaces_bitmap(), i)) {
                space_t *space = get_space_list()->lookup_space(spaceid(i));

                /* Check for valid space id */
                if (EXPECT_FALSE( space == NULL ))
                {
                    panic("found null space in client list");
                    //                printf("lazy clear spc %ld\n", i);
                    //                bitmap_clear(this->get_share_bitmap(), i);
                    //continue;
                }

                printf("del: %S, remove client space %S\n", this, space);
                (void) space->remove_shared_domain(this);
            }
        }
    }
}


bool space_t::is_client_space(space_t *space)
{
    word_t spc_no = space->get_space_id().get_spaceno();

    if (bitmap_isset(this->get_client_spaces_bitmap(), spc_no)) {
        //printf("%p is_shared by %p\n", this, space);
        return true;
    }
    //printf("%p is_NOT_shared by %p\n", this, space);
    return false;
}

