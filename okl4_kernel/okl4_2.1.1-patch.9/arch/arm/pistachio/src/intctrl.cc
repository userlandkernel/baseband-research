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
 * Description:   Implementation of interrupt control functionality
 */

#include <l4.h>
#include <tcb.h>
#include <interrupt.h>
#include <arch/intctrl.h>
#include <schedule.h>
#include <cpu/syscon.h>

void SECTION(".init") init_arm_interrupts()
{
    bool r;
    r = get_kernel_space()->add_mapping((addr_t)ARM_HIGH_VECTOR_VADDR,
                                        (addr_t)virt_to_phys(&arm_high_vector),
                                        pgent_t::size_4k, space_t::read_write_ex, true);
    ASSERT(ALWAYS, r);

    Platform::disable_fiq();

    word_t control;
    read_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, control);
#if CONFIG_ARM_VER == 5

#ifdef CONFIG_DISABLE_ALIGNMENT_EXCEPTIONS
    control &= ~((word_t)C15_CONTROL_A);
#else
    control |= C15_CONTROL_A;
#endif

#else // V6

#ifndef CONFIG_STRICT_ALIGNMENT_EXCEPTIONS
    control &= ~((word_t)C15_CONTROL_A);
#ifdef CONFIG_ALIGNED_ACCESSES
    control |= C15_CONTROL_U;
#endif
#else
    control |= C15_CONTROL_A;
#endif

#endif
    write_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, control);

    /* Read control register again - hardware may not support A-bit */
    read_cp15_register(C15_control, C15_CRm_default, C15_OP2_default, control);
#if CONFIG_ARM_VER == 5
    TRACE_INIT("System running with alignment exceptions %s\n",
                    control & C15_CONTROL_A ? "enabled" : "disabled");
#else
    TRACE_INIT("System running with strict alignment exceptions %s\n",
                    control & C15_CONTROL_A ? "enabled" : "disabled");
    if (!(control & C15_CONTROL_A))
    {
        TRACE_INIT("System running with unaligned accesses %s\n",
                (control & C15_CONTROL_U) ? "enabled" : "disabled");
    }
#endif
}

