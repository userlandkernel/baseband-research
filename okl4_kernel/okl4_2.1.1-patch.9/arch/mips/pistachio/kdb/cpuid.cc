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

#include <arch/mipsregs.h>
#include <arch/cpu.h>

/**
 * cmd_dpuid: dump CPU ID
 */
DECLARE_CMD (cmd_cpuid, arch, 'C', "cpuid", "dump CPUID");

CMD (cmd_cpuid, cg)
{
    word_t prid;
    read_32bit_cp0_register(CP0_PRID, prid);
    word_t config;
    read_32bit_cp0_register(CP0_CONFIG, config);
    int icache_size, dcache_size, icache_width, dcache_width;
    int icache_ways, dcache_ways;
    static const char * pattern[] = {
            "D                 Doubleword every cycle",
            "DDxDDx            2 Doublewords every 3 cycles",
            "DDxxDDxx          2 Doublewords every 4 cycles",
            "DxDxDxDx          2 Doublewords every 4 cycles",
            "DDxxxDDxxx        2 Doublewords every 5 cycles",
            "DDxxxxDDxxxx      2 Doublewords every 6 cycles",
            "DxxDxxDxxDxx      2 Doublewords every 6 cycles",
            "DDxxxxxxDDxxxxxx  2 Doublewords every 8 cycles",
            "DxxxDxxxDxxxDxxx  2 Doublewords every 8 cycles",
    };

    printf ("CPUID: ");
    switch (prid & MIPS_IMP_MASK)
    {
    case MIPS_IMP_SB1:
        printf("Sibyte SB1, Rev %d\n", prid&0x00FF); break;
    case MIPS_IMP_RC64574:
        printf("IDT RC64574/2, Rev %d\n", prid&0x00FF); break;
    case MIPS_IMP_R4700:
        printf("R4700, Rev %d.%d\n", (prid&0x00F0)>>4, prid&0x000F); break;
    case MIPS_IMP_VR41XX:
        printf("NEC ");
        switch (prid & MIPS_REV_MASK_VR)
        {
            case MIPS_REV_VR4121:
                printf("vr4121 rev %d\n", prid&0xf); break;
            case MIPS_REV_VR4181:
                printf("vr4181 rev %d\n", prid&0xf); break;
            default:
                printf("unknown\n");
        }
        break;
    default:
        printf("unknown CP0_PRID=%8x\n", prid);
    }

    switch (prid & MIPS_IMP_MASK)
    {
    case MIPS_IMP_VR41XX:
        icache_width = 16 << ((config>>5)&1);
        dcache_width = 16 << ((config>>4)&1);
        icache_size = (1<<10) << ((config>>9)&7);
        dcache_size = (1<<10) << ((config>>6)&7);
        icache_ways = dcache_ways = 1;
        break;
    case MIPS_IMP_RC64574:
    case MIPS_IMP_R4700:
    default:
        icache_width = 16 << ((config>>5)&1);
        dcache_width = 16 << ((config>>4)&1);
        icache_size = (1<<12) << ((config>>9)&7);
        dcache_size = (1<<12) << ((config>>6)&7);
        icache_ways = dcache_ways = 2;
    }

    switch (prid & MIPS_IMP_MASK)
    {
    case MIPS_IMP_VR41XX:
        printf("MIPS16 is %s\n", (config>>20) ? "available" : "disabled");
        break;
    case MIPS_IMP_RC64574:
        printf("Data transmit pattern: %s\n", pattern[(config>>24)&0xf]);
        printf("System clock is CPU clock divided by %d\n", 2 + (config>>28) & 7);
        printf("Fast Multiply is %s\n", (config>>31) ? "enabled" : "disabled");
        break;
    case MIPS_IMP_R4700:
        printf("Data transmit pattern: %s\n", pattern[(config>>24)&0xf]);
        printf("System clock is CPU clock divided by %d\n", 2 + (config>>28) & 7);
        printf("Master-Checker Mode is %s\n", (config>>31) ? "enabled" : "disabled");
        break;
    default:;
    }

    /*XXX secondary cache probes */
    printf("Detected cache: icache = %dk, %d bytes per line, %d way associative\n",
                    icache_size/1024, icache_width, icache_ways);
    printf("                dcache = %dk, %d bytes per line, %d way associative\n",
                    dcache_size/1024, dcache_width, dcache_ways);

    return CMD_NOQUIT;
}
