/*
 * Copyright (c) 2004, National ICT Australia (NICTA)
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
 * Description: Cogent CSB337 Console
 */

#include <kernel/plat/offsets.h>
#include <kernel/arch/platform.h>

#if defined(CONFIG_KDB_CONS)

#define SERIAL_NAME     "serial"

#define rULCON0     (*(volatile unsigned *)CONSOLE_VADDR) //UART 0 Line control
#define rUCON0      (*(volatile unsigned *)(CONSOLE_VADDR + 0x4)) //UART 0 Control
#define rUFCON0     (*(volatile unsigned *)(CONSOLE_VADDR + 0x8)) //UART 0 FIFO control
#define rUMCON0     (*(volatile unsigned *)(CONSOLE_VADDR + 0xc)) //UART 0 Modem control
#define rUTRSTAT0   (*(volatile unsigned *)(CONSOLE_VADDR + 0x10)) //UART 0 Tx/Rx status
#define rUERSTAT0   (*(volatile unsigned *)(CONSOLE_VADDR + 0x14)) //UART 0 Rx error status
#define rUFSTAT0    (*(volatile unsigned *)(CONSOLE_VADDR + 0x18)) //UART 0 FIFO status
#define rUMSTAT0    (*(volatile unsigned *)(CONSOLE_VADDR + 0x1c)) //UART 0 Modem status
#define rUTXH0      (*(volatile unsigned *)(CONSOLE_VADDR + 0x20)) //UART 0 Transmission Hold
#define rURXH0 (*(volatile unsigned char *)(CONSOLE_VADDR + 0x24)) //UART 0 Receive buffer
#define rUBRDIV0    (*(volatile unsigned *)(CONSOLE_VADDR + 0x28)) //UART 0 Baud rate divisor

void Platform::serial_putc(char c)
{
    if (c == '\n')
            Platform::serial_putc('\r');

    rUCON0 |= 4;                  // Turn the uart on, as userspace may have turned it off
                                  // to avoid excessive IRQs when the xmit buffer is empty
                                  // and nothing to send.
    rUTXH0=c;                     // We'll let userland deal with the spurious irq this
                                  // will generate, and let them turn it back off if
                                  // so desired.
    while (!(rUTRSTAT0 & 0x4)) ;  // Wait until THR is empty.
}

int Platform::serial_getc(bool can_block)
{
    if (!(rUTRSTAT0 & 0x1)) {
        return -1;                // Receive data read
    } else {
        return rURXH0;
    }
}

void Platform::serial_init(void)
{
    u32_t freq = 266000000L; /* Hack */
    rUFCON0=0x00;                                       // FIFO Disable
    rUMCON0=0x00;
    rULCON0=0x03;                                       // Normal,No parity,1 stop,8 bit
    rUCON0=0x245;
    rUBRDIV0=(int)(freq/4/(115200*16)-1);               // freq/4MHz, 115200bps
}
#endif
