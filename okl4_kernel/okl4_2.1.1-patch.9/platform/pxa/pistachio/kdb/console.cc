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
 * Description: Intel XScale Console
 */

#if defined(CONFIG_KDB_CONS)

#include <kernel/plat/console.h>
#include <kernel/arch/platform.h>

/****************************************************************************
 *
 *    Serial console support
 *
 ****************************************************************************/

struct serial_xscalecon {
    word_t rbr;  /* 0 */
    word_t ier;  /* 4 */
    word_t fcr;  /* 8 */
    word_t lcr;  /* 12 */
    word_t mcr;  /* 16 */
    word_t lsr;  /* 20 */
    word_t msr;  /* 24 */
    word_t scr;  /* 28 */
};

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier
#define dlab lcr

#define LSR_DR          0x01    /* Data ready */
#define LSR_OE          0x02    /* Overrun */
#define LSR_PE          0x04    /* Parity error */
#define LSR_FE          0x08    /* Framing error */
#define LSR_BI          0x10    /* Break */
#define LSR_THRE        0x20    /* Xmit holding register empty */
#define LSR_TEMT        0x40    /* Xmitter empty */
#define LSR_ERR         0x80    /* Error */

static volatile struct serial_xscalecon *serial_regs = 0;


void Platform::serial_putc(char c)
{
    if (serial_regs) {
        while ((serial_regs->lsr & LSR_THRE) == 0 );

        serial_regs->thr = c;

        if (c == '\n')
            Platform::serial_putc('\r');
    }
}

int Platform::serial_getc(bool can_block)
{
    if (serial_regs) {
        if ((serial_regs->lsr & LSR_DR) == 0) {
            return -1;
        }

        return (int)serial_regs->rbr;
    }

    return 0;
}


void Platform::serial_init(void)
{
    serial_regs = (struct serial_xscalecon*)(IODEVICE_VADDR + CONSOLE_VOFFSET);
}

#endif
