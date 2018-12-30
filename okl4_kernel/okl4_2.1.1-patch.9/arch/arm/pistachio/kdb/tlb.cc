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
 * Description: ARM TLB Inspection
 */

#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include <arch/asm.h>
#include <arch/thread.h>
#include <tcb.h>

#if defined(CONFIG_CPU_ARM_ARM1136JS) || defined(CONFIG_CPU_ARM_ARM926EJS)
/**
 * cmd_dump_tlb: dump processor TLB
 */
DECLARE_CMD (cmd_dump_tlb, arch, 't', "tlbdump",
             "dump tlb state");

#endif

#if defined(CONFIG_CPU_ARM_ARM1136JS)

static char * decode_ap(word_t ap)
{
    char *access;

    switch (ap) {
        case 0x0: access = "Na/Na"; break;
        case 0x1: access = "RW/Na"; break;
        case 0x2: access = "RW/Ro"; break;
        case 0x3: access = "RW/RW"; break;
        case 0x4: access = "DMFLT"; break;
        case 0x5: access = "Ro/Na"; break;
        case 0x6: access = "Ro/Ro"; break;
        default: access =  "??""/??";
    }
    return access;
}

static void print_tlb_entry(word_t index, word_t vareg, word_t pareg,
        word_t atreg, bool micro_tlb)
{
    char *size;
    word_t shift;

    printf("Index: %02d  ", index);

    if (pareg & 1)
    {
        switch ((pareg >> 6) & 0xf) {
            case 0xf: size = "1k";  shift = 10; break;
            case 0xe: size = "4k";  shift = 12; break;
            case 0xc: size = "16k"; shift = 14; break;
            case 0x8: size = "64k"; shift = 16; break;
            case 0x0: size = "1M";  shift = 20; break;
            case 0x1: size = "16M"; shift = 24; break;
            default: size = "??k";  shift = 10;
        }

        printf("VA: 0x%08x  ", (vareg >> shift) << shift);

        printf("PA: 0x%08x  %3s  [%d] D: %3d [%d], %c %c ", (pareg >> shift) << shift, size,
                (pareg >> 4) & 0x3,
                (atreg >> 5) & 0xf, (atreg >> 1) & 0x7, (atreg & 1) ? 'S' : '-',
                (atreg & (1<<4)) ? '-' : 'X');
#if 0
        if (atreg & (1<<25)) {
            printf("AP: [%s,%s,%s,%s]  ",
                    decode_ap((pareg >> 1) & 0x7),
                    decode_ap((atreg >> 26) & 0x3),
                    decode_ap((atreg >> 28) & 0x3),
                    decode_ap((atreg >> 30) & 0x3)
                  );
        }
        else
#endif
        {
            printf("AP: %s  ", decode_ap((pareg >> 1) & 0x7));
        }
        if (vareg & (1<<9)) {
            printf("Global");
        } else {
            printf("ASID: %03d", vareg & 0xff);
        }
    } else {
        printf("---");
    }
    printf("\n");
}


/**
 * cmd_dump_tlb: dump processor TLB
 */
CMD (cmd_dump_tlb, cg)
{
    word_t index;

    printf("\n:Micro Instruction TLB:\n");

    for (index = 0; index < 10; index ++)
    {
        word_t vareg = 0, pareg = 0, atreg = 0;

        __asm__ __volatile__ (
            "   mcr     p15, 7, "_(control)", c15, c1, 0    "   // Write TLB control reg
            :: [control] "r" (1<<1)
        );

        __asm__ __volatile__ (
            "   mcr     p15, 5, "_(index)", c15, c4, 1      "   // Issue Read of Micro-I TLB
            :: [index] "r" (index | (1UL << 31))
        );

        for (volatile int i = 0; i < 100; i++);

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(vareg)", c15, c5, 1      "   // Read TLB VA Register
            : [vareg] "=r" (vareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(pareg)", c15, c6, 1      "   // Read TLB PA Register
            : [pareg] "=r" (pareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(atreg)", c15, c7, 1      "   // Read TLB Attributes Register
            : [atreg] "=r" (atreg)
        );

        __asm__ __volatile__ (
            "   mcr     p15, 7, "_(control)", c15, c1, 0    "   // Write TLB control reg
            :: [control] "r" (0)
        );

        print_tlb_entry(index, vareg, pareg, atreg, true);
    }


    printf("\n:Micro Data TLB:\n");

    for (index = 0; index < 10; index ++)
    {
        word_t vareg = 0, pareg = 0, atreg = 0;

        __asm__ __volatile__ (
            "   mcr     p15, 7, "_(control)", c15, c1, 0    "   // Write TLB control reg
            :: [control] "r" (1<<0)
        );

        __asm__ __volatile__ (
            "   mcr     p15, 5, "_(index)", c15, c4, 0      "   // Issue Read of Micro-D TLB
            :: [index] "r" (index | (1UL << 31))
        );

        for (volatile int i = 0; i < 100; i++);

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(vareg)", c15, c5, 0      "   // Read TLB VA Register
            : [vareg] "=r" (vareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(pareg)", c15, c6, 0      "   // Read TLB PA Register
            : [pareg] "=r" (pareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(atreg)", c15, c7, 0      "   // Read TLB Attributes Register
            : [atreg] "=r" (atreg)
        );

        __asm__ __volatile__ (
            "   mcr     p15, 7, "_(control)", c15, c1, 0    "   // Write TLB control reg
            :: [control] "r" (0)
        );

        print_tlb_entry(index, vareg, pareg, atreg, true);
    }

    printf("\n:Main TLB:\n");

    for (index = 0; index < 64; index ++)
    {
        word_t vareg = 0, pareg = 0, atreg = 0;

        __asm__ __volatile__ (
            "   mcr     p15, 5, "_(index)", c15, c4, 2      "   // Issue Read of TLB
            :: [index] "r" (index)
        );

        for (volatile int i = 0; i < 100; i++);

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(vareg)", c15, c5, 2      "   // Read TLB VA Register
            : [vareg] "=r" (vareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(pareg)", c15, c6, 2      "   // Read TLB PA Register
            : [pareg] "=r" (pareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(atreg)", c15, c7, 2      "   // Read TLB Attributes Register
            : [atreg] "=r" (atreg)
        );

        print_tlb_entry(index, vareg, pareg, atreg, false);
    }

    printf("\n:Main TLB (Lockdown):\n");

    for (index = 0; index < 10; index ++)
    {
        word_t vareg = 0, pareg = 0, atreg = 0;

        __asm__ __volatile__ (
            "   mcr     p15, 5, "_(index)", c15, c4, 2      "   // Issue Read of TLB + L bit
            :: [index] "r" (index | (1UL << 31))
        );

        for (volatile int i = 0; i < 100; i++);

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(vareg)", c15, c5, 2      "   // Read TLB VA Register
            : [vareg] "=r" (vareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(pareg)", c15, c6, 2      "   // Read TLB PA Register
            : [pareg] "=r" (pareg)
        );

        __asm__ __volatile__ (
            "   mrc     p15, 5, "_(atreg)", c15, c7, 2      "   // Read TLB Attributes Register
            : [atreg] "=r" (atreg)
        );

        print_tlb_entry(index, vareg, pareg, atreg, false);
    }

    return CMD_NOQUIT;
}

#endif

#if defined(CONFIG_CPU_ARM_ARM926EJS)

static char * decode_ap(word_t ap)
{
    char *access;

    switch (ap) {
        case 0x0: access = "Na/Na"; break;
        case 0x1: access = "RW/Na"; break;
        case 0x2: access = "RW/Ro"; break;
        case 0x3: access = "RW/RW"; break;
        default: access =  "??""/??";
    }
    return access;
}

static void print_tlb_entry(word_t index, word_t vareg, word_t pareg, word_t way)
{
    char *size;
    word_t shift;

    if (way != ~0UL)
    {
        if (way == 0)
            printf("Index: %02d  ", index);
        else
            printf("           ");
        printf("way: %d  ", way);
    }
    else
    {
        printf("Index: %02d  ", index);
    }

    if (vareg & (1<<4))
    {
        switch (vareg & 0xf) {
            case 0x1: size = "1k";  shift = 10; break;
            case 0x3: size = "4k";  shift = 12; break;
            case 0x5: size = "16k"; shift = 14; break;
            case 0x7: size = "64k"; shift = 16; break;
            case 0xb: size = "1M";  shift = 20; break;
            default: size = "??k";  shift = 10;
        }

        printf("MVA: 0x%08x  ", (vareg >> shift) << shift);

        printf("PA: 0x%08x  %3s  D: %2d ",
               (pareg >> shift) << shift, size,
                (pareg >> 4) & 0xf,
                (pareg >> 2) & 0x3);
        printf("AP: %s  ", decode_ap((pareg >> 2) & 0x3));
        printf("%c%c",
                pareg & 2 ? 'C' : '-',
                pareg & 1 ? 'B' : '-');
    } else {
        printf("---");
    }
    printf("\n");
}

CMD (cmd_dump_tlb, cg)
{
    word_t index;

    printf("\n:TLB (main set-associative):\n");

    for (index = 0; index < 32; index ++)
    {
        for (word_t way = 0; way < 2; way++)
        {
            word_t vareg = 0, pareg = 0;
            /*lint -esym(529, tlbindex) */
            word_t tlbindex = (index << 10 | way << 31);

            __asm__ __volatile__ (
                "   mcr     p15, 0, "_(tlbindex)", c15, c1, 0   "   // Issue Read of Main TLB Way-0
#if defined(__GNUC__)
                :: [tlbindex] "r" (tlbindex)
#endif
            );

            for (volatile int i = 0; i < 10; i++);

            __asm__ __volatile__ (
                "   mrc     p15, 5, "_(vareg)", c15, c2, 0      "   // Read TLB VA Register
#if defined(__GNUC__)
               : [vareg] "=r" (vareg)
#endif
            );

            __asm__ __volatile__ (
                "   mrc     p15, 5, "_(pareg)", c15, c4, 0      "   // Read TLB PA Register
#if defined(__GNUC__)
                : [pareg] "=r" (pareg)
#endif
            );

            print_tlb_entry(index, vareg, pareg, way);
        }
    }

    printf("\n:TLB (lockdown):\n");

    for (index = 0; index < 10; index ++)
    {
            word_t vareg = 0, pareg = 0;
            /*lint -esym(529, tlbindex) */
            word_t tlbindex = (index << 26);

            __asm__ __volatile__ (
                "   mcr     p15, 0, "_(tlbindex)", c15, c1, 0   "   // Issue Read of Main TLB Way-0
#if defined(__GNUC__)
                :: [tlbindex] "r" (tlbindex)
#endif
            );

            for (volatile int i = 0; i < 10; i++);

            __asm__ __volatile__ (
                "   mrc     p15, 5, "_(vareg)", c15, c2, 1      "   // Read TLB lockdown VA Register
#if defined(__GNUC__)
                : [vareg] "=r" (vareg)
#endif
            );

            __asm__ __volatile__ (
                "   mrc     p15, 5, "_(pareg)", c15, c4, 1      "   // Read TLB lockdown PA Register
#if defined(__GNUC__)
                : [pareg] "=r" (pareg)
#endif
            );

            print_tlb_entry(index, vareg, pareg, ~0UL);
    }

    return CMD_NOQUIT;
}

#endif

