/*
 * Copyright (c) 2005 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   UART Driver
 */

#ifndef __IMX31_SERIAL_H__
#define __IMX31_SERIAL_H__

/*
 * Register constants.  These should be part of the .dx file
 */
#define UARTEN_DISABLE      0x0     /* disable the uart device */
#define UARTEN_ENABLE       0x1     /* enable the uart device */

#define TX_RDY_IRQ_ON       0x1     /* enable the tx ready interrupt */
#define RX_RDY_IRQ_ON       0x1     /* enable the rx ready interrupt */

#define PARITY_NONE         0x0     /* no parity bits */
#define STOP_BITS_1         0x0     /* use 1 stop bit */
#define WORD_SIZE_8_BITS    0x1     /* transmit 8 bits per character */

#define TXEN_ENABLE         0x1     /* enable the transmitter */
#define RXEN_ENABLE         0x1     /* enable the receiver */

#ifdef OLD_CODE

struct uart {
    uint32_t rxr;
    uint32_t pad0[15];
    uint32_t txr;
    uint32_t pad1[15];
    uint32_t cr1;
    uint32_t cr2;
    uint32_t cr3;
    uint32_t cr4;
    uint32_t fifocr;
    uint32_t sr1;
    uint32_t sr2;
    uint32_t escchar;
    uint32_t esctimer;
    uint32_t brminc;
    uint32_t brmmod;
    uint32_t brcount;
    uint32_t milli;
    uint32_t test;
};

extern volatile struct uart *uart;

static inline int is_tx_interrupt(void)
{
    return (uart->sr1 & 0x2000)?1:0;
}

static inline int is_rx_interrupt(void)
{
    return (uart->sr1 & 0x200)?1:0;
}

static inline void tx_mask_interrupt(void)
{
    uart->cr1 &= ~(0x2000);
}

static inline void rx_mask_interrupt(void)
{
    uart->cr1 &= ~(0x200);
}

static inline void tx_unmask_interrupt(void)
{
    uart->cr1 |= (0x2000);
}

static inline void rx_unmask_interrupt(void)
{
    uart->cr1 |= (0x200);
}


static inline int tx_ready(void)
{
    return (uart->test & 0x10)?0:1;
}

static inline int rx_ready(void)
{
    return (uart->test & 0x20)?0:1;
}

static inline void tx_data(unsigned char data)
{
    uart->txr = data;
}

static inline unsigned char rx_data(void)
{
    return (unsigned char) uart->rxr & 0xFF;
}
#endif

#endif

