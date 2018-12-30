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
 * Description: Kernel inrterfaces that platform code is allowed to
 * use.
 */

#ifndef __ARCH__PLATFORM_SUPPORT_H__
#define __ARCH__PLATFORM_SUPPORT_H__

#include <kernel/arch/debug.h>
#include <kernel/cpu/cpu.h>
#include <kernel/tcb.h>
#include <kernel/arch/intctrl.h>
// Next include to remove need for maps
#include <kernel/space.h>

#if defined(CONFIG_DEBUG)
#if defined(CONFIG_KDB_CONS) && !defined(CONFIG_KDB_NO_ASSERTS)

#if defined(_lint)
#define PLAT_ASSERT(level, x) do { if (! (x)) { __panic(); } } while(false)
#else
#define PLAT_ASSERT(level, x)                                                \
    do {                                                                \
        if (EXPECT_FALSE(! (x)) && (level <= CONFIG_ASSERT_LEVEL)) {    \
            PlatformSupport::printf ("Assertion "#x" failed in file %s, line %d (fn=%p)\n", \
                                     __FILE__, __LINE__, __return_address()); \
            PlatformSupport::enter_kdebug ("assert");                   \
        }                                                               \
    } while(false)
#endif

#define PLAT_TRACEF(f, x...)                                         \
    do {                                                        \
        PlatformSupport::printf ("%s:%d: " f, __FUNCTION__, __LINE__ ,##x); \
    } while(false)

#define PLAT_TRACE(x...)    PlatformSupport::printf(x)

# else /* defined(CONFIG_KDB_NO_ASSERTS) */

#define PLAT_ASSERT(level, x)   do { } while (false)
#define PLAT_TRACE(x...)        do { } while (false)
#define PLAT_TRACEF(x...)       do { } while (false)

#endif

#else /* !defined(CONFIG_KDEBUG) */

#define PLAT_ASSERT(level, x)   do { } while (false)
#define PLAT_TRACE(x...)        do { } while (false)
#define PLAT_TRACEF(x...)       do { } while (false)

#endif

/*
 * Functions provided by the general kernel that platform code may
 * call.
 *
 * Calling other functions from the platform code is unsupported.
 */

namespace PlatformSupport {
    /*
     * The current version of the PlatSupport interface.
     */
    static const int API_VERSION = 1;

    enum rwx_e {
        read_only       = space_t::read_only,
        read_write      = space_t::read_write,
        read_execute    = space_t::read_execute,
        read_write_ex   = space_t::read_write_ex
    };

    // Subset of pgsize_e that can be used by platform code.
    enum pgsize_e {
        size_4k = pgent_t::size_4k,
        size_1m = pgent_t::size_1m
    };

    // Subset of memattrib_e that can be used by platform code.
    enum memattrib_e {
        uncached = ::uncached,
        iomemory = ::iomemory
    };

    /* Perform architecture initialisation and start the kernel. */
    extern "C" void arch_init(void* param) NORETURN;

    bool add_mapping_to_kernel(addr_t vaddr, addr_t paddr, pgsize_e size,
                               rwx_e rwx, bool kernel, memattrib_e attrib);

    /*
     * Enter the kernel debugger and display the given message.
     */
    void enter_kdebug(char *s);

#ifdef CONFIG_KDB_CONS
    int printf(const char * format, ...);
#endif
}

#endif /* __ARCH__PLATFORM_SUPPORT_H__ */
