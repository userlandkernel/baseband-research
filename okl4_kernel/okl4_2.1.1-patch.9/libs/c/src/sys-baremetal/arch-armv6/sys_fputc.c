/*
 * Copyright (c) 2004, National ICT Australia
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

#include <stdio.h>
#include <stdint.h>

// #define iPAQ /* FIXME: this is ugly */
// #undef XSCALE
// #define XSCALE
// #undef iPAQ
// #define XSCALE

extern int __fputc(int c, FILE *stream);

/*
 * Put character for elf-loader 
 */
int
__fputc(int c, FILE *stream)
{

/* ----------------------------- iPAQ & PLEB1 (SA-1100) --------------------- */
#ifdef MACHINE_IPAQ_H3800       // iPAQ // SA-1100
    volatile char *base = (char *)0x80050000;   // base serial interface
                                                // address
    /*
     * volatile int *base2 = (int *)0x80030000; // the other serial Arm serial
     * i/f 
     */

    /*
     * UTSR1 32 @ 0x20: tby <0> # Transmitter busy rne <1> # Refeive FIFO not
     * empty tnf <2> # Transmitter not full pre <3> # Parity error fre <4> #
     * Framing error ror <5> # Receive FIFO overrun 
     */

#define UTDR            0x14    // data register
#define UTSR1           0x20    // status register 1 offset
#define UTSR1_TNF       (1 << 2)        // tx FIFO not full (status bit)

    while (!(*((volatile long *)(base + UTSR1)) & UTSR1_TNF));  // busy wait
                                                                // while TX
                                                                // FIFO is full
    *(volatile unsigned char *)(base + UTDR) = c;
    // *base2 = c;
#endif

/* ----------------------------- PLEB2 (XSCALE PXA-255)---------------------- */
#ifdef MACHINE_PLEB2            // XSCALE /* PXA 255 on PLEB2 */

    /* Console port -- taken from kernel/include/platform/pleb2/console.h */
#define CONSOLE_OFFSET          0x100000

    /* IO Base -- taken from kernel/include/arch/arm/xscale/cpu.h */
#define XSCALE_PXA255_IO_BASE   0x40000000

#define DATAR           0x00000000
#define STATUSR         0x00000014

    /* TX empty bit -- uboot/include/asm/arch/hardware.h */
#define LSR_TEMT        (1 << 6)        /* Transmitter Empty */

    volatile char *base = (char *)(XSCALE_PXA255_IO_BASE + CONSOLE_OFFSET);

    /* wait for room in the tx FIFO on FFUART */
    while (!(*((volatile long *)(base + STATUSR)) & LSR_TEMT)); // busy wait
                                                                // while TX
                                                                // FIFO is full
    *(volatile unsigned char *)(base + DATAR) = c;

#endif /* XSCALE */

    return (0);
}
