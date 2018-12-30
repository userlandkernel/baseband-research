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
 * Description:   MIPS WATCH point
 */
                
#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <arch/mipsregs.h>


DECLARE_CMD_GROUP (watchpoint);


/**
 * cmd_mips64_watch: Mips64 WATCH management.
 */
DECLARE_CMD (cmd_mips64_watch, arch, 'w', "watch", "MIPS Watchpoint");

CMD(cmd_mips64_watch, cg)
{
    return watchpoint.interact (cg, "watch");
}

/**
 * cmd_mips64_wshow: show current
 */
DECLARE_CMD (cmd_mips64_wset, watchpoint, 's', "set", "set watch point");

CMD(cmd_mips64_wset, cg)
{
#if defined(L4_32BIT)
    UNIMPLEMENTED();
#else
    word_t addr = get_hex ("Phys Address", 0x0, NULL);

    char r = get_choice ("Watch read access", "y/n", 'y');
    char w = get_choice ("Watch write access", "y/n", 'y');

    u32_t watchlo = (u32_t)addr & (~3);
    u32_t watchhi = (addr>>32)&0xF;
    if (r == 'y')
        watchlo |= 2;
    if (w == 'y')
        watchlo |= 1;

    write_32bit_cp0_register(CP0_WATCHLO, watchlo);
    write_32bit_cp0_register(CP0_WATCHHI, watchhi);
#endif

    return CMD_NOQUIT;
}

/**
 * cmd_mips64_wshow: show current
 */
DECLARE_CMD (cmd_mips64_wshow, watchpoint, 'w', "show", "show watch status");

CMD(cmd_mips64_wshow, cg)
{
    u32_t watchlo;
    read_32bit_cp0_register(CP0_WATCHLO, watchlo);
    u32_t watchhi;
    read_32bit_cp0_register(CP0_WATCHHI, watchhi);

    printf("watch status\n");
    printf("  Watch is %s\n", watchlo & 3 ? "enabled" : "disabled");

    if (watchlo & 3)
    {
#ifdef L4_32BIT
        UNIMPLEMENTED();
#else
        printf("Watching physical address: %p for", watchlo&(~3) | (((word_t)watchhi)<<32));
#endif
        switch (watchlo & 3) {
        case 1: printf("write access\n"); break;
        case 2: printf("read access\n"); break;
        case 3: printf("any access\n"); break;
        default: ;
        }
    }

    return CMD_NOQUIT;
}

