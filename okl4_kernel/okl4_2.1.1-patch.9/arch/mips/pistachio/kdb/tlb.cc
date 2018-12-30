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
 * Description:   TLB management commands
 */
                
#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <arch/tlb.h>
#include <cpu/config.h>
#include <arch/mipsregs.h>
#include <arch/context.h>


DECLARE_CMD_GROUP (mips64_tlb);

#if defined(L4_32BIT)
#define         SPACE   ""
#define         W       "8lx"
#elif defined(L4_64BIT)
#define         SPACE   "        "
#define         W       "16lx"
#endif


/**
 * cmd_mips64_tlb: Mips64 TLB management.
 */
DECLARE_CMD (cmd_mips64_tlb, arch, 't', "tlb", "TLB management");

CMD(cmd_mips64_tlb, cg)
{
    return mips64_tlb.interact (cg, "tlb");
}

static const char * pagemask[] = {
    "  4k", "  8k", " 16k", " 32k",
    " 64k", "128k", "256k", "  1M",
    "  2M", "  4M", "  8M", " 16M",
    " 32M", " 64M", "128M", "256M",
};

/**
 * cmd_mips64_dum: dump Mips64 TLB
 */
DECLARE_CMD (cmd_mips64_dum, mips64_tlb, 'd', "dump", "dump hardware TLB");

CMD(cmd_mips64_dum, cg)
{
    /*lint -e529 save is only used in asm */
    word_t i, save;
    static const char * cache[] = {
        "Wthr ", "WthrA", "Off  ", "Wb   ",
        "CoEx ", "CoCOW", "CoCUW", "Accel"
    };

    __asm__ __volatile__ (
        MFC0"   %0, "STR(CP0_ENTRYHI)"\n\t"
        : "=r" (save)
    );

    printf ("Index  "SPACE" EntryHi | EntryLo0 (cache v d)  EntryLo1 (cache v d) | Size  ASID  Global\n");

    for (i=0; i<CONFIG_MIPS_TLB_SIZE; i++)
    {
        word_t hi, lo0, lo1, mask, size;
#if defined(_lint)
        extern void __get_tlb_settings(word_t, word_t *, word_t *, word_t *, word_t *);
        __get_tlb_settings(i, &hi, &lo0, &lo1, &mask);
#else
        __asm__ __volatile__ (
            "mtc0 %4,"STR(CP0_INDEX)"\n\t"
            "nop;nop;nop;tlbr;nop;nop;nop;nop\n\t"
            MFC0" %0,"STR(CP0_ENTRYHI)"\n\t"
            MFC0" %1,"STR(CP0_ENTRYLO0)"\n\t"
            MFC0" %2,"STR(CP0_ENTRYLO1)"\n\t" 
            "mfc0 %3,"STR(CP0_PAGEMASK)"\n\t" 
            : "=r" (hi), "=r" (lo0), "=r" (lo1), "=r" (mask)
            : "r" (i)
        );
#endif
        size = 0;
        mask >>= 13;
        while (mask&1)
        {
            mask >>= 1;
            size ++;
        }
        printf("%2d:    %p | %"W" (%s %d %d)  %"W" (%s %d %d) | %s   %3d     %s\n",
                        i, hi,
                        lo0, cache[(lo0&0x38)>>3], (lo0>>1)&1, (lo0>>2)&1,
                        lo1, cache[(lo1&0x38)>>3], (lo1>>1)&1, (lo1>>2)&1,
                        pagemask[size], hi&0xFF, lo0&1 ? "Yes" : " No");
    }

    __asm__ __volatile__ (
        MTC0"   %0, "STR(CP0_ENTRYHI)"\n\t"
        : : "r" (save)
    );

    return CMD_NOQUIT;
}


/**
 * cmd_mips64_tran: translate Mips64 TLB
 */
DECLARE_CMD (cmd_mips64_trans, mips64_tlb, 't', "translate", "translate TLB");

CMD(cmd_mips64_trans, cg)
{
    /*lint -e529 save is only used in asm */
    word_t i, save;

    __asm__ __volatile__ (
        MFC0"   %0, "STR(CP0_ENTRYHI)"\n\t"
        : "=r" (save)
    );

#if defined(L4_32BIT)
    printf ("            Even Page               Odd Page\n");
    printf ("Index    (virt)    (phys)   |   (virt)    (phys)     size   ASID\n");
#elif defined(L4_32BIT)
    printf ("Index       (virt)   Even Page  (phys)      |      (virt)   Odd Page   (phys)        size   ASID\n");
#endif

    for (i=0; i<CONFIG_MIPS_TLB_SIZE; i++)
    {
        word_t hi, lo0, lo1, mask, size;
#if defined(_lint)
        extern void __get_tlb_settings(word_t, word_t *, word_t *, word_t *, word_t *);
        __get_tlb_settings(i, &hi, &lo0, &lo1, &mask);
#else
        __asm__ __volatile__ (
            "   mtc0 %4,"STR(CP0_INDEX)"        \n\t"
            "   nop;nop;nop;                    \n\t"
            "   tlbr;                           \n\t"
            "   nop;nop;nop;nop                 \n\t"
            MFC0" %0,"STR(CP0_ENTRYHI)"         \n\t"
            MFC0" %1,"STR(CP0_ENTRYLO0)"        \n\t"
            MFC0" %2,"STR(CP0_ENTRYLO1)"        \n\t"
            MFC0" %3,"STR(CP0_PAGEMASK)"        \n\t"
            : "=r" (hi), "=r" (lo0), "=r" (lo1), "=r" (mask)
            : "r" (i)
        );
#endif
        size = 0;
        mask >>= 13;
        while (mask&1)
        {
            mask >>= 1;
            size ++;
        }
        printf("%2d:    ", i);

        if (lo0&2)
            printf("%"W" -> %"W"", (hi&(~((1UL<<13)-1))), (lo0>>6)<<CONFIG_MIPS_VPN_SHIFT);
        else
            printf(SPACE"         --         "SPACE);

        printf(" | ");

        if (lo1&2)
            printf("%"W" -> %"W"", (hi&(~((1UL<<13)-1)))+(1UL<<(size+12)), (lo1>>6)<<CONFIG_MIPS_VPN_SHIFT);
        else
            printf(SPACE"         --         "SPACE);
        printf("  (%s)", pagemask[size]);
        if (lo0&1)
            printf("   all\n");
        else
            printf("   %3d\n", hi&0xFF);
    }

    __asm__ __volatile__ (
        MTC0"   %0, "STR(CP0_ENTRYHI)"\n\t"
        : : "r" (save)
    );

    return CMD_NOQUIT;
}



/**
 * cmd_tlb_info: dump TLB information
 */
DECLARE_CMD (cmd_tlb_info, mips64_tlb, 'i', "info", "dump TLB information");

CMD(cmd_tlb_info, cg)
{
    printf("tlb info\n");
    return CMD_NOQUIT;
}
