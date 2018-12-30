/*
 * Copyright (c) 2002, University of New South Wales
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
 * Description:   MIPS-64 CPUID
 */
                
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>

#include <arch/mips_cpu.h>
#include <arch/cpu.h>

/**
 * cmd_dpuid: dump CPU ID
 */
DECLARE_CMD (cmd_cp0, arch, '0', "dumpcp0", "dump cp0 registers");

#ifdef L4_64BIT
#define NONE            0x0000deadbeef0000ULL
#else
#define NONE            0xdeadbeefUL
#endif

typedef struct
{
    char    *name;
    int     num;
    int     word64;
} cp0_reg;

/* Coprocessor 0 register names */
#define INDEX           0
#define RANDOM          1
#define ENTRYLO0        2
#define ENTRYLO1        3
#define CONTEXT         4
#define PAGEMASK        5
#define WIRED           6
#define BADVADDR        8
#define COUNT           9
#define ENTRYHI         10
#define COMPARE         11
#define STATUS          12
#define CAUSE           13
#define EPC             14
#define PRID            15
#define CONFIG          16
#define LLADDR          17
#define WATCHLO         18
#define WATCHHI         19
#define XCONTEXT        20
#define FRAMEMASK       21
#define DIAGNOSTIC      22
#define PERFORMANCE     25
#define ECC             26
#define CACHEERR        27
#define TAGLO           28
#define TAGHI           29
#define ERROREPC        30

static cp0_reg regs[] =
{
    { "BadVaddr",  BADVADDR, 1 },
    { "CacheErr",  CACHEERR, 0 },
    { "Cause",     CAUSE, 0 }, 
    { "Compare",   COMPARE, 0 },
    { "Config",    CONFIG, 0 },
    { "Context",   CONTEXT, 1 },
    { "Count",     COUNT, 0 },
    { "ECC",       ECC, 0 },
    { "EntryHi",   ENTRYHI, 1 },
    { "EntryLo0",  ENTRYLO0, 1 },
    { "EntryLo1",  ENTRYLO1, 1 },
    { "EPC",       EPC, 1 },
    { "ErrorEPC",  ERROREPC, 1 },
    { "Index",     INDEX, 0 },
    { "LLaddr",    LLADDR, 0 },
    { "PageMask",  PAGEMASK, 0 },
    { "PerfCount", PERFORMANCE, 0 },
    { "PRID",      PRID, 0 },
    { "Random",    RANDOM, 0 },
    { "Status",    STATUS, 0 },
    { "TagHi0",    TAGHI, 0 },
    { "TagLo0",    TAGLO, 0 },
    { "WatchHi",   WATCHHI, 0 },
    { "WatchLo",   WATCHLO, 0 },
    { "Wired",     WIRED, 0 },
    { "XContext",  XCONTEXT, 1 },
    { 0, 0, 0 }
};

word_t read_cp0_reg(word_t i)
{
#if defined(L4_32BIT)
    UNIMPLEMENTED();
    return NONE;
#else
    word_t ret;

    switch (i) {
    case INDEX:         read_32bit_cp0_register($0 , ret); break;
    case RANDOM:        read_32bit_cp0_register($1 , ret); break;
    case ENTRYLO0:      read_64bit_cp0_register($2 , ret); break;
    case ENTRYLO1:      read_64bit_cp0_register($3 , ret); break;
    case CONTEXT:       read_64bit_cp0_register($4 , ret); break;
    case PAGEMASK:      read_32bit_cp0_register($5 , ret); break;
    case WIRED:         read_32bit_cp0_register($6 , ret); break;
    case BADVADDR:      read_64bit_cp0_register($8 , ret); break;
    case COUNT:         read_32bit_cp0_register($9 , ret); break;
    case ENTRYHI:       read_64bit_cp0_register($10, ret); break;
    case COMPARE:       read_32bit_cp0_register($11, ret); break;
    case STATUS:        read_32bit_cp0_register($12, ret); break;
    case CAUSE:         read_32bit_cp0_register($13, ret); break;
    case EPC:           read_64bit_cp0_register($14, ret); break;
    case PRID:          read_32bit_cp0_register($15, ret); break;
    case CONFIG:        read_32bit_cp0_register($16, ret); break;
    case LLADDR:        read_32bit_cp0_register($17, ret); break;
    case WATCHLO:       read_32bit_cp0_register($18, ret); break;
    case WATCHHI:       read_32bit_cp0_register($19, ret); break;
    case XCONTEXT:      read_64bit_cp0_register($20, ret); break;
    case FRAMEMASK:     read_64bit_cp0_register($21, ret); break;
    case DIAGNOSTIC:    read_64bit_cp0_register($22, ret); break;
    case PERFORMANCE:   read_32bit_cp0_register($25, ret); break;
    case ECC:           read_32bit_cp0_register($26, ret); break;
    case CACHEERR:      read_32bit_cp0_register($27, ret); break;
    case TAGLO:         read_32bit_cp0_register($28, ret); break;
    case TAGHI:         read_32bit_cp0_register($29, ret); break;
    case ERROREPC:      read_64bit_cp0_register($30, ret); break;
    default: ret = NONE;
    }
    return ret;
#endif
}

CMD (cmd_cp0, cg)
{
    word_t x;

    printf("Dumping Contents of CP0 Registers\n");
    x = 0;
    while (regs[x].name) {
        if (regs[x].word64)
            printf("  %12s [%2d]  0x%016lx\n", regs[x].name,
                            regs[x].num, read_cp0_reg(regs[x].num));
        else
            printf("  %12s [%2d]          0x%08lx\n", regs[x].name,
                            regs[x].num, read_cp0_reg(regs[x].num));
        x++;
    } 
    return CMD_NOQUIT;
}
