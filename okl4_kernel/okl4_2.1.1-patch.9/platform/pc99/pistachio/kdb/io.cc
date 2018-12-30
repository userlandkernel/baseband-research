/*
 * Copyright (c) 2001, 2004-2003, Karlsruhe University
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
 * Description:   PC99 specific I/O functions
 */
#include <l4.h>
#include <arch/cpu.h>
#include <arch/ioport.h>
#include <arch/platform.h>
#include <kdb/kdb.h>
#include <kdb/init.h>
#include <kdb/input.h>
#include <kdb/cmd.h>
#include <kdb/console.h>
#include <init.h>
#include <debug.h>
#include <linear_ptab.h>

#if defined(CONFIG_KDB_CONS)

#if !defined(CONFIG_KDB_COMPORT)
#define CONFIG_KDB_COMPORT 0x3f8
#endif

#if !defined(CONFIG_KDB_COMSPEED)
#define CONFIG_KDB_COMSPEED 115200
#endif

#define KERNEL_VIDEO            (KERNEL_RW_OFFSET + 0xb8000)
#define KERNEL_VIDEO_HERC       (0xF00b0000)

#define SEC_PC99_IO             SEC_KDEBUG

/* external firewire init routines */
#ifdef CONFIG_KDB_CONS_DBG1394
#define SEC_EFI_IO              SEC_KDEBUG
void init_dbg1394(void) SECTION(SEC_EFI_IO);
void putc_dbg1394(char) SECTION(SEC_EFI_IO);
char getc_dbg1394(bool) SECTION(SEC_EFI_IO);
#endif

#if defined(CONFIG_KDB_CONS_SCREEN)

/*
**
** Console I/O functions.
**
*/
#if defined(CONFIG_DEBUGGER_IO_SCREEN_HERC)
# define DISPLAY ((char *) KERNEL_VIDEO_HERC)
# define NUM_LINES (25)
# define NUM_COLS (80)
#else
# define DISPLAY ((char *) KERNEL_VIDEO)
# define NUM_LINES (25)
# define NUM_COLS  (80)
#endif


void Platform::serial_init(void)
{
}

void Platform::serial_putc(char c)
{
    static unsigned cursor = 160 * (NUM_LINES - 1) ;
    static unsigned color = 7;
    static unsigned new_color = 0;
    static unsigned esc = 0;
    static unsigned esc2 = 0;
    static const unsigned col[] = { 0, 4, 2, 14, 1, 5, 3, 15 };

    if (esc == 1)
    {
        if (c == '[')
        {
            esc++;
            return;
        }
    }
    else if (esc == 2)
    {
        switch (c)
        {
        case '0': case '1': case '2':
        case '3': case '4': case '7':
            esc++;
            esc2 = c;
            return;
        }
    }
    else if (esc == 3)
    {
        switch (c)
        {
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            if (esc2 == '3' || esc2 == '4')
            {
                // New foreground or background color
                new_color = col[c - '0'];
                esc++;
                return;
            }
            break;
        case 'J':
            if (esc2 == '2')
            {
                // Clear screen
                for (int i = 0; i < 80*NUM_LINES; i++)
                    ((u16_t *) DISPLAY)[i] = (color << 8) + ' ';
                cursor = 0;
                esc = 0;
                return;
            }
            break;
        case 'm':
            switch (esc2)
            {
            case '0':
                // Normal text
                color = 7;
                esc = 0;
                return;
            case '1':
                // Bright text
                color = 15;
                esc = 0;
                return;
            case  '7':
                // Reversed
                color = (7 << 4);
                esc = 0;
                return;
            }
        }
    }
    else if (esc == 4)
    {
        if (c == 'm' && esc2 == '3')
        {
            // Foreground color
            color = (color & 0xf0) | new_color;
            esc = 0;
            return;
        }
        else if (c == 'm' && esc2 == '4')
        {
            // Background color
            color = (color & 0x0f) | (new_color << 4);
            esc = 0;
            return;
        }
    }


    switch(c) {
    case '\e':
        esc = 1;
        return;
    case '\r':
        cursor -= (cursor % cursor % (NUM_COLS * 2));
        break;
    case '\n':
        cursor += ((NUM_COLS * 2) - (cursor % (NUM_COLS * 2)));
        break;
    case '\t':
        cursor += (8 - (cursor % 8));
        break;
    case '\b':
        cursor -= 2;
        break;
    default:
        DISPLAY[cursor++] = c;
        DISPLAY[cursor++] = color;
    }

    esc = 0;

    if ((cursor /  (NUM_COLS * 2)) == NUM_LINES)
        {
            for (int i = NUM_COLS; i < NUM_COLS*NUM_LINES; i++)
                ((u16_t *) DISPLAY)[i - NUM_COLS] = ((u16_t *) DISPLAY)[i];
            for (int i = 0; i < NUM_COLS; i++)
                ((u16_t * )DISPLAY)[NUM_COLS * (NUM_LINES-1) + i] = 0;
            cursor -= (NUM_COLS*2);
    }
}

#define KBD_STATUS_REG          0x64
#define KBD_CNTL_REG            0x64
#define KBD_DATA_REG            0x60

#define KBD_STAT_OBF            0x01    /* Keyboard output buffer full */

#define kbd_read_input() in_u8(KBD_DATA_REG)
#define kbd_read_status() in_u8(KBD_STATUS_REG)

static unsigned char keyb_layout[2][128] =
{
    "\000\0331234567890-=\010\t"                        /* 0x00 - 0x0f */
    "qwertyuiop[]\r\000as"                              /* 0x10 - 0x1f */
    "dfghjkl;'`\000\\zxcv"                              /* 0x20 - 0x2f */
    "bnm,./\000*\000 \000\201\202\203\204\205"          /* 0x30 - 0x3f */
    "\206\207\210\211\212\000\000789-456+1"             /* 0x40 - 0x4f */
    "230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000"
    "\r\000/"                                           /* 0x60 - 0x6f */
    ,
    "\000\033!@#$%^&*()_+\010\t"                        /* 0x00 - 0x0f */
    "QWERTYUIOP{}\r\000AS"                              /* 0x10 - 0x1f */
    "DFGHJKL:\"`\000\\ZXCV"                             /* 0x20 - 0x2f */
    "BNM<>?\000*\000 \000\201\202\203\204\205"          /* 0x30 - 0x3f */
    "\206\207\210\211\212\000\000789-456+1"             /* 0x40 - 0x4f */
    "230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000"
    "\r\000/"                                           /* 0x60 - 0x6f */
};

int Platform::serial_getc(bool can_block)
{
    static u8_t last_key = 0;
    static u8_t shift = 0;
    char c;
    while(1) {
        unsigned char status = kbd_read_status();
        while (status & KBD_STAT_OBF) {
            u8_t scancode;
            scancode = kbd_read_input();
            /* check for SHIFT-keys */
            if (((scancode & 0x7F) == 42) || ((scancode & 0x7F) == 54))
            {
                shift = !(scancode & 0x80);
                continue;
            }
            /* ignore all other RELEASED-codes */
            if (scancode & 0x80)
                last_key = 0;
            else if (last_key != scancode)
            {
                last_key = scancode;
                c = keyb_layout[shift][scancode];
                if (c > 0) return c;
            }
        }
    }
}

#endif /* CONFIG_KDB_CONS_SCREEN */

#if defined(CONFIG_KDB_CONS_SERIAL)

/*
**
** Serial port I/O functions.
**
*/

#define COMPORT         CONFIG_KDB_COMPORT
#define RATE            CONFIG_KDB_COMSPEED

void Platform::serial_init(void)
{
#define IER     (COMPORT+1)
#define EIR     (COMPORT+2)
#define LCR     (COMPORT+3)
#define MCR     (COMPORT+4)
#define LSR     (COMPORT+5)
#define MSR     (COMPORT+6)
#define DLLO    (COMPORT+0)
#define DLHI    (COMPORT+1)

    out_u8(LCR, 0x80);          /* select bank 1        */
    for (volatile int i = 10000000; i--; );
    out_u8(DLLO, (((115200/RATE) >> 0) & 0x00FF));
    /*lint -e572 Depending on rate, this is OK, and not excessive shift */
    out_u8(DLHI, (((115200/RATE) >> 8) & 0x00FF));
    out_u8(LCR, 0x03);          /* set 8,N,1            */
    out_u8(IER, 0x00);          /* disable interrupts   */
    out_u8(EIR, 0x07);          /* enable FIFOs */
    out_u8(IER, 0x01);          /* enable RX interrupts */
    (void) in_u8(IER);
    (void) in_u8(EIR);
    (void) in_u8(LCR);
    (void) in_u8(MCR);
    (void) in_u8(LSR);
    (void) in_u8(MSR);
}


void Platform::serial_putc(char c)
{
    while ((in_u8(COMPORT+5) & 0x60) == 0);
    out_u8(COMPORT,c);
    if (c == '\n')
        Platform::serial_putc('\r');

}

int Platform::serial_getc(bool can_block)
{
    if ((in_u8(COMPORT+5) & 0x01) == 0)
    {
        return -1;
    }
    return (int) in_u8(COMPORT);
}

DECLARE_CMD (cmd_dumpvga, arch, 'V', "screendump", "dump VGA screen contents");

CMD(cmd_dumpvga, cg)
{
    addr_t display = (addr_t) get_hex ("VGA base:", KERNEL_VIDEO, "VGA screen");
    space_t *space = get_space ("Space");
    space = space ? space : get_kernel_space();

    printf("VGA screen dump %p\n"
           "========================================"
           "========================================", display);

    do {
        for (unsigned y = 0; y < 25; y++)           // rows
        {
            printf("\n");
            for (unsigned x = 0; x < 80; x++)       // columns
            {

                unsigned char v;
                (void) readmem(space, addr_offset(display, y*160 + 2*x), &v);
                printf ("%c", ((v >= 32 && v < 127) ||
                               (v >= 161 && v <= 191) ||
                               (v >= 224)) ? v : (v == 0) ? ' ' : '.');
            }
        }
        printf("\n"
               "========================================"
               "========================================\n");

        display = addr_offset (display, 80 * 25 * 2);
    } while (get_choice ("Continue?", "Continue/Quit", 'c') != 'q');

    return CMD_NOQUIT;
}

#endif /* CONFIG_KDB_CONS_SERIAL */

#endif /* CONFIG_KDB_CONS */
