/*
 * Copyright (c) 2002, 2003, Karlsruhe University
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
 * Description:   Memory dumping code
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <linear_ptab.h>

#include <tcb.h>


char getc (void);
void memdump_loop (space_t * space, addr_t addr);
void memdump (space_t * space, addr_t addr);


/**
 * Current wordsize for memory dumps
 */
static word_t memdump_wordsize = sizeof (word_t);


/**
 * Last memory dump address.
 */
static word_t kdb_last_dump = 0;


/**
 * Change memory dump word size.
 */
DECLARE_CMD (cmd_wordsize, config, 'w', "wordsize",
             "change memdump word size");

CMD(cmd_wordsize, cg)
{
    memdump_wordsize *= 2;
    if (memdump_wordsize > sizeof (word_t))
        memdump_wordsize = 1;

    printf ("Memdump wordsize = %d\n", memdump_wordsize);

    return CMD_NOQUIT;
}


/**
 * Dump memory contents of current space.
 */
DECLARE_CMD (cmd_memdump, root, 'd', "memdump", "dump memory");

CMD (cmd_memdump, cg)
{
    word_t addr = get_hex ("Dump address", kdb_last_dump);

    if (addr == ABORT_MAGIC)
        return CMD_NOQUIT;

    kdb_last_dump = addr;
    memdump_loop (kdb.kdb_current->get_space (), (addr_t) addr);

    return CMD_NOQUIT;
}


/**
 *  Dump memory contents of other space.
 */
DECLARE_CMD (cmd_memdump_remote, root, 'D', "memdump",
             "dump memory in other space");

CMD(cmd_memdump_remote, cg)
{
    word_t addr = get_hex ("Dump address", kdb_last_dump);

    if (addr == ABORT_MAGIC)
        return CMD_NOQUIT;

    kdb_last_dump = addr;
    memdump_loop (get_space ("Space"), (addr_t) addr);

    return CMD_NOQUIT;
}


/**
 *  Dump memory contents of physical memory
 */
DECLARE_CMD (cmd_memdump_phys, root, 'P', "dumpphys",
             "dump physical memory");

CMD(cmd_memdump_phys, cg)
{
    word_t addr = get_hex ("Dump address", kdb_last_dump);

    if (addr == ABORT_MAGIC)
        return CMD_NOQUIT;

    kdb_last_dump = addr;
    memdump_loop ((space_t*)INVALID_ADDR, (addr_t) addr);

    return CMD_NOQUIT;
}


void memdump_loop (space_t * space, addr_t addr)
{
    do {
        memdump (space, addr);
        addr = addr_offset (addr, 16*16);
    } while (get_choice ("Continue?", "Continue/Quit", 'c') != 'q');
}


void memdump (space_t * space, addr_t addr)
{
    space = space ? space : get_kernel_space();

    for (int j = 0; j < 16; j++)
    {
        printf ("%p  ", addr);
        switch (memdump_wordsize)
        {
        case 1:
        {
            u8_t v, *x = (u8_t *) addr;
            for (int i = 0; i < 16; i++, x++)
            {
                if (i == 8) printf (" ");
                if (! readmem (space, x, &v))
                    printf ("## ");
                else
                    printf ("%02x ", v);
            }
            break;
        }
        case 2:
        {
            u16_t v, *x = (u16_t *) addr;
            for (int i = 0; i < 8; i++, x++)
            {
                if (i == 4) printf (" ");
                if (! readmem (space, x, &v))
                    printf ("#### ");
                else
                    printf ("%04x ", v);
            }
            break;
        }
        case 4:
        {
            u32_t v, *x = (u32_t *) addr;
            for (int i = 0; i < 4; i++, x++)
            {
                if (! readmem (space, x, &v))
                    printf ("######## ");
                else
                    printf ("%08x ", v);
            }
            break;
        }
#if !defined(CONFIG_IS_32BIT)
        case 8:
        {
            u64_t v, *x = (u64_t *) addr;
            for (int i = 0; i < 2; i++, x++)
            {
                if (! readmem (space, x, &v))
                    printf ("################ ");
                else
                    printf ("%016x ", v);
            }
            break;
        }
#endif
        }

        u8_t * c = (u8_t *) addr;
        printf ("  ");
        for (int i = 0; i < 16; i++, c++)
        {
            u8_t v, *x = (u8_t *) c;
            if (i == 8) printf (" ");
            if (! readmem (space, x, &v))
                printf ("#");
            else
                printf ("%c", ((v >= 32 && v < 127) ||
                               (v >= 161 && v <= 191) ||
                               (v >= 224)) ? v : '.');
        }
        printf ("\n");
        addr = addr_offset (addr, 16);
    }
}

