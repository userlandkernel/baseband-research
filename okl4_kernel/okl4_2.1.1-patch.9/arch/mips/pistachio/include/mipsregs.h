/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Author:       Carl van Schaik
 * Description:   MIPS CPU Registers
 */ 

/*lint -$*/
/* $ is not actually a legal character, however because it is used throughout
   the MIPS port, we add this option, which allows just the use of this illegal
   character, but other illegal characters won't be let through */

#ifndef __ARCH__MIPS__MIPSREGS_H__
#define __ARCH__MIPS__MIPSREGS_H__

/* MIPS CoProcessor-0 Registers*/
#define CP0_INDEX $0    /* selects TLB entry for r/w ops & shows probe success */ 
#define CP0_RANDOM $1   /* counter - random number generator */
#define CP0_ENTRYLO $2  /* low word of a TLB entry */
#define CP0_ENTRYLO0 $2 /* R4k uses this for even-numbered virtual pages */
#define CP0_ENTRYLO1 $3 /* R4k uses this for odd-numbered virtual pages */
#define CP0_CONTEXT $4  /* TLB refill handler's kernel PTE entry pointer */
#define CP0_PAGEMASK $5 /* R4k page number bit mask (impl. variable page sizes) */
#define CP0_WIRED $6    /* R4k lower bnd for Random (controls randomness of TLB) */
#define CP0_ERROR $7    /* R6k status/control register for parity checking */
#define CP0_BADVADDR $8 /* "bad" virt. addr (VA of last failed v->p translation) */
#define CP0_COUNT $9    /* R4k r/w reg - continuously incrementing counter */
#define CP0_ENTRYHI $10 /* High word of a TLB entry */
#define CP0_COMPARE $11 /* R4k traps when this register equals Count */
#define CP0_STATUS $12  /* Kernel/User mode, interrupt enb., & diagnostic states */
#define CP0_CAUSE $13   /* Cause of last exception */
#define CP0_EPC $14     /* Address to return to after processing this exception */
#define CP0_PRID $15    /* Processor revision identifier */
#define CP0_CONFIG $16  /* R4k config options for caches, etc. */
#define CP0_LLADR $17   /* R4k last instruction read by a Load Linked */
#define CP0_LLADDR $17  /* Inconsistencies in naming... sigh. */
#define CP0_WATCHLO $18 /* R4k hardware watchpoint data */
#define CP0_WATCHHI $19 /* R4k hardware watchpoint data */
/* 20-21,23-24 - RESERVED */
#define CP0_PTR $22     /* MIPS64 Performance Trace Register */
#define CP0_PERF $25    /* MIPS64 Performance Counter Register Mapping */
#define CP0_ECC $26     /* R4k cache Error Correction Code */
#define CP0_CACHEERR $27        /* R4k read-only cache error codes */
#define CP0_TAGLO $28   /* R4k primary or secondary cache tag and parity */
#define CP0_TAGHI $29   /* R4k primary or secondary cache tag and parity */
#define CP0_ERROREPC $30        /* R4k cache error EPC */

/* MIPS CoProcessor-0 Registers*/
#define CP1_REVISION $0 /* FPU Revision */
#define CP1_STATUS $31 /* FPU STatus */

/* Config Register - Cacheability Codes */
#define CONFIG_CACHABLE_NO_WA 0
#define CONFIG_CACHABLE_WA 1
#define CONFIG_NOCACHE 2
#define CONFIG_CACHABLE_NONCOHERENT 3
#define CONFIG_CACHABLE_CE 4
#define CONFIG_CACHABLE_COW 5
#define CONFIG_CACHABLE_CUW 6
#define CONFIG_CACHABLE_ACCEL 7
#define CONFIG_CACHE_MASK 7

#if !defined(ASSEMBLY)

#define _INS_(x) #x
#define STR(x) _INS_(x)

/* Read from CP0 register */
#if defined(_lint)
#define read_32bit_cp0_register(reg, ret) ret = 0
#define read_64bit_cp0_register(reg, ret) ret = 0
#define read_64bit_cp0_register_sel(reg, sel, ret) ret = 0
#else
#define read_32bit_cp0_register(reg, ret)       \
        __asm__ __volatile__(           \
        "mfc0 %0,"STR(reg)              \
        : "=r" (ret))

#define read_64bit_cp0_register(reg, ret)       \
        __asm__ __volatile__(           \
        "dmfc0 %0,"STR(reg)             \
        : "=r" (ret))

#define read_64bit_cp0_register_sel(reg, sel, ret)      \
        __asm__ __volatile__(           \
        "dmfc0 %0,"STR(reg)","STR(sel)          \
        : "=r" (ret))
#endif

#if defined(_lint)
void __write_reg(word_t val);
#define write_32bit_cp0_register(reg,value) __write_reg(value)
#define write_64bit_cp0_register(reg,value) __write_reg(value)
#define write_64bit_cp0_register_sel(reg,value,sel) __write_reg(value)
#else
#define write_32bit_cp0_register(reg,value) \
        __asm__ __volatile__(           \
        "mtc0\t%0,"STR(reg)             \
        : : "r" (value));

#define write_64bit_cp0_register(reg,value) \
        __asm__ __volatile__(           \
        "dmtc0\t%0,"STR(reg)            \
        : : "r" (value))

#define write_64bit_cp0_register_sel(reg,value,sel) \
        __asm__ __volatile__(           \
        "dmtc0\t%0,"STR(reg)","STR(sel)         \
        : : "r" (value))
#endif
#endif

/* Interupt Enable/Cause Bits */
#define INT_SW0     (1<<8)
#define INT_SW1     (1<<9)
#define INT_IRQ0    (1<<10)
#define INT_IRQ1    (1<<11)
#define INT_IRQ2    (1<<12)
#define INT_IRQ3    (1<<13)
#define INT_IRQ4    (1<<14)
#define INT_IRQ5    (1<<15)

/* Status register bits */
#define ST_IE           (1<<0)
#define ST_EXL          (1<<1)
#define ST_ERL          (1<<2)  
#define ST_KSU          (3<<3)
#define ST_U            (2<<3)
#define ST_S            (1<<3)
#define ST_K            (0<<3)
#define ST_UX           (1<<5)          /* Not MIPS32 */
#define ST_SX           (1<<6)          /* Not MIPS32 */
#define ST_KX           (1<<7)          /* Not MIPS32 */
#define SP_IPL          (0x3f<<10)      /* MIPS32v2 Interrupt Priority */
#define ST_DE           (1<<16)         /* Not MIPS32 */
#define ST_CE           (1<<17)         /* Not MIPS32 */
#define ST_NMI          (1<<19)
#define ST_SR           (1<<20)
#define ST_TS           (1<<21)
#define ST_BEV          (1<<22)
#define ST_PX           (1<<23)
#define ST_MX           (1<<24)
#define ST_RE           (1<<25)
#define ST_FR           (1<<26)
#define ST_RP           (1<<27)

#define ST_IM           (0xff<<8)
#define ST_CH           (1<<18)
#define ST_SR           (1<<20)
#define ST_TS           (1<<21)
#define ST_BEV          (1<<22)
#define ST_PX           (1<<23)
#define ST_MX           (1<<24)
#define ST_CU           (UL(0xf)<<28)
#define ST_CU0          (UL(0x1)<<28)
#define ST_CU1          (UL(0x2)<<28)
#define ST_CU2          (UL(0x4)<<28)
#define ST_CU3          (UL(0x8)<<28)
#define ST_XX           (UL(0x8)<<28)

#define USER_FLAG_READ_MASK     (ST_CU | ST_RP | ST_FR | ST_RE | ST_MX | ST_PX | ST_UX)
#define USER_FLAG_WRITE_MASK    (ST_XX | ST_RP | ST_FR | ST_RE | ST_MX | ST_PX | ST_UX)

/* Cause register */
#define CAUSE_EXCCODE   (UL(31)<<2)
#define CAUSE_EXCCODE_NUM(x)    ((x>>2) & 31)
#define CAUSE_IP        (UL(255)<<8)
#define CAUSE_IP0       (UL(1)<<8)
#define CAUSE_IP1       (UL(1)<<9)
#define CAUSE_IP2       (UL(1)<<10)
#define CAUSE_IP3       (UL(1)<<11)
#define CAUSE_IP4       (UL(1)<<12)
#define CAUSE_IP5       (UL(1)<<13)
#define CAUSE_IP6       (UL(1)<<14)
#define CAUSE_IP7       (UL(1)<<15)
#define CAUSE_IV        (UL(1)<<23)
#define CAUSE_CE        (UL(3)<<28)
#define CAUSE_CE_NUM(x) ((x>>28) & 3)
#define CAUSE_BD        (UL(1)<<31)

/* register access helpers */
#if defined(L4_32BIT)
#define MTC0            "mtc0"
#define MFC0            "mfc0"
#elif defined(L4_64BIT)
#define MTC0            "dmtc0"
#define MFC0            "dmfc0"
#endif

#endif /* __ARCH__MIPS__MIPSREGS_H__ */
