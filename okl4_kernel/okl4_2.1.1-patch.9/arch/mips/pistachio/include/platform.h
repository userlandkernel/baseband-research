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
 * Description: Services provided by the platform.
 */

#ifndef __ARCH__PLATFORM_H__
#define __ARCH__PLATFORM_H__

#include <kernel/l4.h>
#include <kernel/interrupt.h>
#include <kernel/platform.h>
#if defined(CONFIG_KDB_CONS)
#include <kernel/kdb/console.h>
#endif

/*
 * Platform API.
 *
 * These functions provide the platform specific code to the wider
 * kernel.
 */
namespace Platform {
    /*
     * The current version of the Platform interface.
     */
    static const int API_VERSION = 1;

    /*
     * void _start()
     *
     * The platform must provide the entry point for the kernel.  This
     * will be called with virtual memory disabled.
     *
     * The startup code must end by calling arch_init() to boot the
     * kernel.
     */

    /*
     * Initialise the platform.
     *
     * This function should perform any required platform
     * initialisation, apart from interrupts and timers, and return
     * true if the initialisation succeeded.
     *
     * It will be passed the values of Platform::API_VERSION and
     * PlatformSupport::API_VERSION that were compiled into the
     * generic kernel and should return false if it is not compatible
     * with these versions.
     *
     * This function will be called after virtual memory have been
     * enabled and the memory subsystem has been initialised.
     */
    bool init(int plat_ver, int support_ver);

    /*
     * Initialise and start the timers.
     *
     * Returns the length of the timer interval in microseconds.
     */
    word_t init_clocks();

    /*
     * Interrupt handling
     */
    class irq_desc_t;

    extern "C" void handle_interrupt(word_t arg1, word_t arg2, word_t arg3);
    word_t security_control_interrupt(irq_desc_t *desc, void *owner, word_t control);
    word_t config_interrupt(irq_desc_t *desc, void *handler,
                            irq_control_t control, void *utcb);

    /* Cache Attributes */
    void dcache_attributes(word_t * size, word_t * line_size, word_t * sets, word_t * ways);
    void icache_attributes(word_t * size, word_t * line_size, word_t * sets, word_t * ways);

    /* Platform Control */
    word_t do_platform_control(void* current, plat_control_t control, word_t param1,
                           word_t param2, word_t param3, continuation_t cont);

    /* KDB console support. */

#if defined(CONFIG_KDB_CONS)
    /*
     * Initialise the console driver.
     */
    void serial_init(void);

    /*
     * Return the current character from the console serial device, or
     * -1 if there is no character available.  If can_block is true
     * then the function may block while waiting for a character to
     * arrive.
     */
    int serial_getc(bool can_block);

    /*
     * Output the character 'c' to the console serial device.  This
     * function may block while waiting for the device to accept the
     * character.
     */
    void serial_putc(char c);
#endif

    /*
     * Reboot the platform.
     *
     * This function must not return under any circumstances.
     */
    void NORETURN reboot(void);

    void enable_timer_tick(void);
    void disable_timer_tick(void);
}

#endif /* __ARCH__PLATFORM_H__ */
