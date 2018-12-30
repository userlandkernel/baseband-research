/*
 * Copyright (c) 2003-2005, National ICT Australia (NICTA)
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
 * Description:   FASS functionality
 */

#ifdef CONFIG_ENABLE_FASS

#include <l4.h>
#include <space.h>
#include <arch/fass.h>
#include <arch/special.h>
#include <tcb.h>
#include <tracebuffer.h>
#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT(ARM_FASS_RECYCLE);

arm_fass_t arm_fass;

void arm_fass_t::init(void)
{
    domain_dirty = 0;
    this->domain_space[0] = get_kernel_space();
    get_kernel_space()->set_domain(0);
    rand = 1;
}

void arm_fass_t::clean_all(word_t flush)
{
    if (flush) {
        domain_dirty = current_domain_mask;
        arm_cache::cache_flush();
    }

    arm_cache::tlb_flush();
}

/* Choose a domain to replace - try select a clean domain, otherwise replace one.
 * Should consider using a more sophisticated selection if
 * it will buy anything (for example, consider the number of CPD slots used by
 * the domains). Also, should consider moving this to user-level.
 */

int arm_fass_t::replacement_domain(void)
{
    word_t domain;
    rand += 7333;

    /* First search for a clean domain */
    word_t clean = (~domain_dirty) & 0x55555555;

    if (EXPECT_TRUE(clean)) {
        domain = msb(clean);
        if (EXPECT_TRUE(domain)) {  /* Don't select domain 0 */
            return domain/2;
        }
    }

    /* Pick the next available domain */
    do {
        rand = rand + (rand << 7) + 6277;
        rand = (rand << 7) ^ (rand >> 17) ^ (rand << 11);

        domain = rand % ARM_DOMAINS;
    } while ((domain == 0) || (domain == current_domain));

    return domain;
}

arm_domain_t arm_fass_t::domain_recycle(space_t *space)
{
    arm_domain_t target;
    space_t *old_space;
    //word_t flush = 0;

    target = replacement_domain();
    ASSERT(DEBUG, target != current_domain);

    TRACEPOINT(ARM_FASS_RECYCLE,
            printf("Recycling domain %d for %p\n", target, space));

    old_space = domain_space[target];

    if (old_space)
    {
        //word_t old_vspace = old_space->get_vspace();

        /* remove access to this domain */
        old_space->flush_sharing_spaces();
        old_space->set_domain(INVALID_DOMAIN);

        //if ((!old_vspace) || (space->get_vspace() != old_vspace))
        //    flush = 1;
    }

    space->set_domain(target);

    domain_space[target] = space;

    /*
     * Remove the elements in the CPD belonging to the domain to be
     * recycled.
     */
    pgent_t *cpd = get_cpd();
    cpd_bitfield_t *domain_set = cpd_set[target];
    for (word_t i = 0, j; i < CPD_BITFIELD_ARRAY_SIZE*CPD_BITFIELD_WORD_SIZE; i+=CPD_BITFIELD_WORD_SIZE) {
        j = *domain_set ++;

        if (EXPECT_FALSE(j))
        {
            do {
                word_t k = msb(j);

                word_t section = i + k;
                cpd[section].raw = 0;

                j &= ~(1UL << k);
            } while (j);

            *(domain_set-1) = 0;
        }
    }

    if (old_space) {
        bool dirty = TEST_DOMAIN(domain_dirty, target);

        /* Remove mapping to UTCBs from old_space */
        word_t section = ( UTCB_AREA_START/ARM_SECTION_SIZE +
                        (old_space->get_space_id().get_spaceno()) );
        // TRACEF("save old_space %p utcb cpd section %d\n", old_space, section);
        cpd[section].raw = (word_t)old_space;

        clean_all(dirty);
    }

    return target;
}

/* Free up a domain, space must have a valid domain */
void arm_fass_t::free_domain(space_t *space)
{
    if (!space)
        return;

    arm_domain_t target;

    /* remove access to this domain */
    space->flush_sharing_spaces();

    target = space->get_domain();
    space->set_domain(INVALID_DOMAIN);
    domain_space[target] = NULL;

    /*
     * Remove the elements in the CPD belonging to the domain to be
     * removed.
     */
    pgent_t *cpd = get_cpd();
    cpd_bitfield_t *domain_set = cpd_set[target];
    for (word_t i = 0, j; i < CPD_BITFIELD_ARRAY_SIZE*CPD_BITFIELD_WORD_SIZE; i+=CPD_BITFIELD_WORD_SIZE) {
        j = *domain_set ++;

        if (EXPECT_FALSE(j))
        {
            do {
                word_t k = msb(j);

                word_t section = i + k;
                cpd[section].raw = 0;

                j &= ~(1UL << k);
            } while (j);

            *(domain_set-1) = 0;
        }
    }

#if 0
    /*
     * We only free-domain on deleting a space and the TLB/cache should
     * already be clean.
     */
    if (TEST_DOMAIN(domain_dirty, target))
    {
        clean_all(true);
    }
#endif
}

space_t *arm_fass_t::get_space(arm_domain_t domain)
{
    if (domain == INVALID_DOMAIN)
        return NULL;

    return domain_space[domain];
}

void arm_fass_t::set_space(arm_domain_t domain, space_t *space)
{
    if (domain != INVALID_DOMAIN)
        domain_space[domain] = space;
}



#endif /* CONFIG_ENABLE_FASS */
