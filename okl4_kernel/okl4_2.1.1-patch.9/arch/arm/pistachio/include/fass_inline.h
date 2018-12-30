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
 * Description:   FASS inline functions (needed to break circular dependency)
 */

#ifndef __ARCH__ARM__FASS_INLINE_H__
#define __ARCH__ARM__FASS_INLINE_H__

#ifdef CONFIG_ENABLE_FASS

#include <kernel/arch/fass.h>
#include <kernel/space.h>

INLINE void arm_fass_t::add_set(arm_domain_t domain, word_t section)
{
    cpd_set[domain][CPD_BITFIELD_POS(section)] |=
            (1UL << CPD_BITFIELD_OFFSET(section));
}

INLINE void arm_fass_t::remove_set(arm_domain_t domain, word_t section)
{
    cpd_set[domain][CPD_BITFIELD_POS(section)] &=
            ~(1UL << CPD_BITFIELD_OFFSET(section));
}

INLINE void arm_fass_t::activate_domain(space_t *space)
{
    /* Don't need to switch domains as we're still inside the kernel if
     * !space.
     */
    if (EXPECT_FALSE(space == NULL)) {
        current_domain = KERNEL_DOMAIN;
        current_domain_mask = 0x1;
        return;
    }

    arm_domain_t target = space->get_domain();

    if (EXPECT_FALSE(target == INVALID_DOMAIN))
    {
        target = domain_recycle(space);
        /* Insert mapping to UTCBs into cpd */
        word_t section = ( UTCB_AREA_START/ARM_SECTION_SIZE +
                        (space->get_space_id().get_spaceno()) );
        pgent_t entry = *space->pgent_utcb();
        entry.set_domain(target);
        get_cpd()[section] = entry;
    }

    current_domain = target;
    current_domain_mask = space->get_domain_mask();

    domain_dirty |= current_domain_mask;
//printf("d %d, m %08lx\n", current_domain, current_domain_mask);
}

INLINE void arm_fass_t::activate_other_domain(space_t *space)
{
    arm_domain_t target = space->get_domain();

    if (EXPECT_FALSE(target == INVALID_DOMAIN))
    {
        target = domain_recycle(space);
        /* Insert mapping to UTCBs into cpd */
        word_t section = ( UTCB_AREA_START/ARM_SECTION_SIZE +
                        (space->get_space_id().get_spaceno()) );
        pgent_t entry = *space->pgent_utcb();
        entry.set_domain(target);
        get_cpd()[section] = entry;
    }
}

#endif

#endif /* __ARCH__ARM__FASS_INLINE_H__ */
