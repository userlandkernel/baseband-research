/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   ARM1176JZ coprocessor-15 definitions
 * Author: Carl van Schaik, June 2006
 */

#ifndef _ARCH__ARM__ARM1176JZ__SYSCON_H_
#define _ARCH__ARM__ARM1176JZ__SYSCON_H_

#ifndef CONFIG_ARM_V6
#define CONFIG_ARM_V6
#endif

#ifndef CONFIG_ARM_VFP
#define CONFIG_ARM_VFP
#endif

#define CONFIG_TRUSTZONE

/* XXX this needs an overhaul */

/* Coprocessor 15 */
#define CP15                    p15

/* Primary CP15 registers (CRn) */
#define C15_id                  c0
#define C15_control             c1
#define C15_ttbase              c2
#define C15_domain              c3
#define C15_fault_status        c5
#define C15_fault_addr          c6
#define C15_cache_con           c7
#define C15_tlb                 c8
#define C15_cache_lock          c9
#define C15_tlb_lock            c10
#define C15_pid                 c13
#define C15_breakpoints         c14
#define C15_coprocessor         c15

/* Default secondary register (CRm) */
#define C15_CRm_default         c0
/* Default opcode2 register (opcode2) */
#define C15_OP2_default         0

/* CP15 - Control Register */
#define C15_CONTROL_FIXED       0x50008
#define C15_CONTROL_M           0x0001                  /* Memory management enable     */
#define C15_CONTROL_A           0x0002                  /* Alignment fault enable       */
#define C15_CONTROL_C           0x0004                  /* Data cache enable            */
//#define C15_CONTROL_W         0x0008                  /* Write buffer enable          */
#define C15_CONTROL_B           0x0080                  /* Big endian enable            */
#define C15_CONTROL_S           0x0100                  /* System access checks in MMU  */
#define C15_CONTROL_R           0x0200                  /* ROM access checks in MMU     */
#define C15_CONTROL_Z           0x0800                  /* Branch Target Buffer Enable  */
#define C15_CONTROL_I           0x1000                  /* Instruction cache enable     */
#define C15_CONTROL_X           0x2000                  /* Remap interrupt vector       */
#define C15_CONTROL_RR          0x4000                  /* Replacement strategy         */
#define C15_CONTROL_L4          0x8000                  /* ARMv4 Compatibility - Thumb  */
#define C15_CONTROL_FI          0x200000                /* Configure fast interrupts    */
#define C15_CONTROL_U           0x400000                /* Unaligned data access enable */
#define C15_CONTROL_XP          0x800000                /* Subpage AP bits disable      */
#define C15_CONTROL_VE          0x1000000               /* Vectored interrupts enable   */
#define C15_CONTROL_EE          0x2000000               /* CPSR E-bit on exception      */
#define C15_CONTROL_TRE         0x10000000              /* TEX Remap enable             */
#define C15_CONTROL_AFE         0x20000000              /* Access Flags enable          */

#if defined(CONFIG_BIGENDIAN)
#define     ARM_ENDIAN          C15_CONTROL_B
#else
#define     ARM_ENDIAN          0
#endif

#define C15_CONTROL_INIT        (C15_CONTROL_FIXED | ARM_ENDIAN)

/* Kernel mode - little endian, cached, write buffer, remap to 0xffff0000 */
#define C15_CONTROL_KERNEL      (C15_CONTROL_FIXED | C15_CONTROL_M | C15_CONTROL_C |    \
                                 ARM_ENDIAN | C15_CONTROL_S | C15_CONTROL_Z |           \
                                 C15_CONTROL_I | C15_CONTROL_X | C15_CONTROL_XP)

#if !defined(ASSEMBLY)

#define _INS_(x) #x
#define STR(x) _INS_(x)

/* Read from coprocessor 15 register */
#define read_cp15_register(CRn, CRm, op2, ret)  \
    __asm__ __volatile__ (                      \
    "mrc    p15, 0, %0,"STR(CRn)",              \
    "STR(CRm)","STR(op2)";\n"                   \
    : "=r" (ret))

/* Write to coprocessor 15 register */
#define write_cp15_register(CRn, CRm, op2, val) \
    __asm__ __volatile__ (                      \
    "mcr    p15, 0, %0,"STR(CRn)",              \
    "STR(CRm)","STR(op2)";\n"                   \
    :: "r" (val))

/* CPWAIT - Wait for CP15 Update */
#define CPWAIT

/* ARM11 Remap registers */
#define ARM11_REMAP_PERIPH(x, size)     ((x & ~(0xfff)) | size & 0x1f)

#define ARM11_REMAP_0K          0
#define ARM11_REMAP_4K          3
#define ARM11_REMAP_8K          4
#define ARM11_REMAP_16K         5
#define ARM11_REMAP_32K         6
#define ARM11_REMAP_64K         7
#define ARM11_REMAP_128K        8
/* rest of numbers left out */

#endif

/* ARM1176 features */
#define CONFIG_ARM_THREAD_REGISTERS     1

#endif /*_ARCH__ARM__ARM1176JZ__SYSCON_H_*/
