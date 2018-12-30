/*
 * Copyright (c) 2002, 2004, Karlsruhe University
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
 * Description:   Generic console functionality
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/console.h>
#include <arch/platform.h>
#include <tcb.h>

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
#define CONFIG_SMP_OUTPUTPREFIX 1
#endif

#if defined(CONFIG_SMP_OUTPUTPREFIX)
static bool use_cpuprefix = false;

DECLARE_CMD (cmd_toggle_cpuprefix, config, 'p', "cpuprefix",
             "Toggle CPU prefix");

CMD (cmd_toggle_cpuprefix, cg)
{
    use_cpuprefix = ! use_cpuprefix;
    printf ("CPU output-prefix: %s\n", use_cpuprefix ? "on" : "off");
    return CMD_NOQUIT;
}
#endif

#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)
{
    char c;

    if ((c = getc(false)) != NO_CHAR) {
        if (c == 27) {
            enter_kdebug("breakin");
        }
    }
}
#endif

#if defined(CONFIG_KDB_CONS)

void init_console (void)
{
    Platform::serial_init();
}

void putc (char c)
{
#if defined(CONFIG_SMP_OUTPUTPREFIX)
    static bool beginning_of_line = true;

    word_t hwtid  = get_current_context().unit;
    word_t cpuid  = get_current_context().domain;
#if defined(CONFIG_THREAD_NAMES)
    char * name = get_current_tcb()->debug_name;
#endif

    if (beginning_of_line && use_cpuprefix)
    {
        Platform::serial_putc('[');

//      Platform::serial_putc('C'); Platform::serial_putc('P'); Platform::serial_putc('U'); Platform::serial_putc(' ');
        if (cpuid >= 10)
            Platform::serial_putc('0' + ((cpuid / 10) % 10));
        Platform::serial_putc('0' + (cpuid % 10));

        Platform::serial_putc('-');

        if (hwtid >= 10)
            Platform::serial_putc('0' + ((hwtid / 10) % 10));

        Platform::serial_putc('0' + (hwtid % 10));

        Platform::serial_putc(']'); Platform::serial_putc(' ');

#if defined(CONFIG_THREAD_NAMES)
        while(get_current_tcb() && *name){
                Platform::serial_putc(*name);
                name++;
        }
#endif
        Platform::serial_putc(':');
        Platform::serial_putc(' ');
    }

    Platform::serial_putc(c);

    beginning_of_line = (c == '\n' || c == '\r');
#else
    Platform::serial_putc(c);
#endif
}

char getc (bool block)
{
    int key;

    while ((key = Platform::serial_getc(block)) == -1 && block)
        ;

    if (key == KEY_NEWLINE)
        key = KEY_RETURN;
    return (char) key;
}

#endif
