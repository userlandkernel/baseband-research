/*
 * Copyright (c) 2004-2006, National ICT Australia (NICTA)
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
 * Description:
 */

#include <kernel/l4.h>
#include <kernel/plat/intctrl.h>
#include <kernel/plat/console.h>
#include <kernel/plat/timer.h>
#include <kernel/plat/interrupt.h>
#include <kernel/arch/platsupport.h>
#include <kernel/arch/platform.h>

static const long XSCALE_DEV_PHYS = 0x40000000;

namespace Platform {

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
bool SECTION(".init")
Platform::init(int plat_ver, int support_ver)
{
    bool r;

    /* Check for API compatibility. */
    if (plat_ver != Platform::API_VERSION ||
        support_ver != PlatformSupport::API_VERSION) {
        return false;
    }

    /* Map in the control registers */

    r = PlatformSupport::add_mapping_to_kernel((addr_t)(IODEVICE_VADDR + CONSOLE_VOFFSET),
                                               (addr_t)(XSCALE_DEV_PHYS + CONSOLE_POFFSET),
                                               PlatformSupport::size_4k, PlatformSupport::read_write, true, 
					       PlatformSupport::uncached);
    PLAT_ASSERT(ALWAYS, r == true);

    r = PlatformSupport::add_mapping_to_kernel((addr_t)(IODEVICE_VADDR + INTERRUPT_VOFFSET),
                                               (addr_t)(XSCALE_DEV_PHYS + INTERRUPT_POFFSET),
                                               PlatformSupport::size_4k, PlatformSupport::read_write, true,
					       PlatformSupport::uncached);
    PLAT_ASSERT(ALWAYS, r == true);

    r = PlatformSupport::add_mapping_to_kernel((addr_t)(IODEVICE_VADDR + TIMER_VOFFSET),
                                               (addr_t)(XSCALE_DEV_PHYS + TIMER_POFFSET),
                                               PlatformSupport::size_4k, PlatformSupport::read_write, true,
					       PlatformSupport::uncached);
    PLAT_ASSERT(ALWAYS, r == true);

    r = PlatformSupport::add_mapping_to_kernel((addr_t)(IODEVICE_VADDR + CLOCKS_VOFFSET),
                                               (addr_t)(XSCALE_DEV_PHYS + CLOCKS_POFFSET),
                                               PlatformSupport::size_4k, PlatformSupport::read_write, true, 
					       PlatformSupport::uncached);
    PLAT_ASSERT(ALWAYS, r == true);

    r = PlatformSupport::add_mapping_to_kernel((addr_t)(IODEVICE_VADDR + GPIO_VOFFSET),
                                               (addr_t)(XSCALE_DEV_PHYS + GPIO_POFFSET),
                                               PlatformSupport::size_4k, PlatformSupport::read_write, true,
					       PlatformSupport::uncached);
    PLAT_ASSERT(ALWAYS, r == true);

    /* Initialize PXA interrupt handling */
    for (word_t i = 0; i < IRQS; i++) {
        irq_mapping[i].handler.init();

        pxa_mask(i);
    }
    bitmap_init(irq_pending, IRQS, false);

    return true;
}

void SECTION(".init")
Platform::dcache_attributes(word_t * size, word_t * line_size, word_t * sets, word_t * ways)
{
#if defined(CONFIG_SUBPLAT_PXA270) || defined(CONFIG_SUBPLAT_PXA255)
    if (size)
        *size = 32 * 1024;
    if (line_size)
        *line_size = 32;
#else
#error Unimplemented PXA architecture
#endif
}

void SECTION(".init")
Platform::icache_attributes(word_t * size, word_t * line_size, word_t * sets, word_t * ways)
{
}


}
