/*
 * Copyright (c) 2003, National ICT Australia (NICTA)
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
 * Description:   Exception frame dumping
 */
#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include <arch/asm.h>
#include <arch/thread.h>
#include <tcb.h>

/**
 * cmd_dump_cp15regs: dump processor registers
 */
DECLARE_CMD (cmd_dump_cp15regs, arch, 'r', "cp15dump",
             "dump cp15 registers");

CMD (cmd_dump_cp15regs, cg)
{
    {
        word_t mainid = 0, cachetype = 0;
#if (CONFIG_ARM_VER >= 6) || defined(CONFIG_CPU_ARM_ARM926EJS)
        word_t tcmtype = 0;
#endif
#if CONFIG_ARM_VER >= 6
        word_t tlbtype = 0;
#endif
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(mainid)", c0, c0, 0      " /* main id */
#if defined(__GNUC__)
                : [mainid] "=r" (mainid)
#endif
                );
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(cachetype)", c0, c0, 1   " /* cache type */
#if defined(__GNUC__)
                 : [cachetype] "=r" (cachetype)
#endif
                );
#if (CONFIG_ARM_VER >= 6) || defined(CONFIG_CPU_ARM_ARM926EJS)
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(tcmtype)", c0, c0, 2     " /* TCM type */
#if defined(__GNUC__)
                 : [tcmtype] "=r" (tcmtype)
#endif
                );
#endif
#if CONFIG_ARM_VER >= 6
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(tlbtype)", c0, c0, 3     " /* TLB type */
#if defined(__GNUC__)
                 : [tlbtype] "=r" (tlbtype)
#endif
                );
#endif
        printf("%16s: %08lx\n", "Main Id", mainid);
        printf("%16s: %08lx\n", "Cache Type", cachetype);
#if (CONFIG_ARM_VER >= 6) || defined(CONFIG_CPU_ARM_ARM926EJS)
        printf("%16s: %08lx\n", "TCM Type", tcmtype);
#endif
#if CONFIG_ARM_VER >= 6
        printf("%16s: %08lx\n", "TLB Type", tlbtype);
#endif
        printf("----\n");
    }

    {
        word_t control = 0;

        __asm__  __volatile__ (
                "   mrc     p15, 0,"_(control)", c1, c0, 0      " // control reg
#if defined(__GNUC__)
                : [control] "=r" (control)
#endif
                );

        printf("%16s: %08lx\n", "Control Reg", control);

#if (CONFIG_ARM_VER >= 6) || defined(CONFIG_CPU_ARM_XSCALE)
        word_t auxcontrol = 0;

        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(auxcontrol)", c1, c0, 1       " // aux control
#if defined(__GNUC__)
                : [auxcontrol] "=r" (auxcontrol)
#endif
                );

        printf("%16s: %08lx\n", "Aux Control", auxcontrol);
#endif
#if CONFIG_ARM_VER >= 6
        word_t coproaccess = 0;

        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(coproaccess)", c1, c0, 2       " // copro-access
#if defined(__GNUC__)
                : [coproaccess] "=r" (coproaccess)
#endif
                );

        printf("%16s: %08lx\n", "CoPro Access", coproaccess);
#endif
        printf("----\n");
    }

    {
        word_t ttbase0 = 0;

        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(ttbase0)", c2, c0, 0     " // ttbase0
#if defined(__GNUC__)
                :[ttbase0] "=r" (ttbase0)
#endif
                );

        printf("%16s: %08lx\n", "TTBase0", ttbase0);

#if CONFIG_ARM_VER >= 6
        word_t ttbase1 = 0, ttcontrol = 0;

        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(ttbase1)", c2, c0, 1     " // ttbase1
#if defined(__GNUC__)
                : [ttbase1] "=r" (ttbase1)
#endif
                );

        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(ttcontrol)", c2, c0, 2   " // ttcontrol
#if defined(__GNUC__)
                : [ttcontrol] "=r" (ttcontrol)
#endif
                );

        printf("%16s: %08lx\n", "TTBase1", ttbase1);
        printf("%16s: %08lx\n", "TTControl", ttcontrol);
#endif
        printf("----\n");
    }

    {
#if (CONFIG_ARM_VER >= 6) || defined(CONFIG_CPU_ARM_ARM926EJS)
        word_t context = 0;
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(context)", c13, c0, 0    " // fcse/pid
#if defined(__GNUC__)
                :[context] "=r" (context)
#endif
                );

        printf("%16s: %08lx\n", "Context ID", context);
#endif
        word_t fcse = 0;
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(fcse)", c13, c0, 1       " // context id
#if defined(__GNUC__)
                : [fcse] "=r" (fcse)
#endif
                );

        printf("%16s: %08lx\n", "FCSE/PID", fcse);
    }

    {
        word_t status = 0;
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(status)", c5, c0, 0    " // data fault status
#if defined(__GNUC__)
                :[status] "=r" (status)
#endif
                );

        printf("%16s: %08lx\n", "D-Fault Sts", status);

#if (CONFIG_ARM_VER >= 6)
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(status)", c5, c0, 1    " // instr fault status
#if defined(__GNUC__)
                :[status] "=r" (status)
#endif
                );

        printf("%16s: %08lx\n", "I-Fault Sts", status);
#endif
        word_t addr = 0;
        __asm__  __volatile__ (
                "   mrc     p15, 0, "_(addr)", c6, c0, 0    " // fault address
#if defined(__GNUC__)
                :[addr] "=r" (addr)
#endif
                );

        printf("%16s: %08lx\n", "Fault Addr", addr);
    }

    return CMD_NOQUIT;
}

#ifdef CONFIG_CPU_ARM_XSCALE

/**
 * cmd_find_copproc: dump coprocessor registers
 */
DECLARE_CMD (cmd_dump_coproc, arch, 'p', "coprocessor",
             "dump coprocessor registers");

CMD (cmd_dump_coproc, cg)
{
    word_t clkcfg = 0;
    __asm__  __volatile__ (
        "mrc        p14, 0, "_(clkcfg)", c6, c0, 0  "
#if defined(__GNUC__)
        :[clkcfg]"=r" (clkcfg)
#endif
    );

    printf("ClkConfig: %x\n", clkcfg);
    printf("This command currently not impemented.\n");
    return CMD_NOQUIT;
}

#endif
