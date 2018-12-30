/*
 * Copyright (c) 2004, National ICT Australia (NICTA)
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
 * Description:   Intel XScale coprocessor-15 definitions
 */


#ifndef _ARCH_ARM_XSCALE_SYSCON_H_
#define _ARCH_ARM_XSCALE_SYSCON_H_

#ifndef CONFIG_ARM_V5
#define CONFIG_ARM_V5
#endif

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
#define C15_CONTROL_FIXED       0x0078
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

#if defined(CONFIG_BIGENDIAN)
#define     ARM_ENDIAN          C15_CONTROL_B
#else
#define     ARM_ENDIAN          0
#endif

#define C15_CONTROL_INIT        (C15_CONTROL_FIXED | ARM_ENDIAN)

/* Kernel mode - little endian, cached, write buffer, remap to 0xffff0000 */
#define C15_CONTROL_KERNEL      (C15_CONTROL_FIXED | C15_CONTROL_M | C15_CONTROL_C |    \
                                 ARM_ENDIAN | C15_CONTROL_S | C15_CONTROL_Z |           \
                                 C15_CONTROL_I | C15_CONTROL_X)

#if !defined(ASSEMBLY)

#define _INS_(x) #x
#define STR(x) _INS_(x)

#if defined(__GNUC__)
#define _OUTPUT(x)  : [x] "=r" (x)
#define _INPUT(x)  :: [x] "r" (x)
#elif defined(__RVCT_GNU__)
#define _OUTPUT(x)
#define _INPUT(x)
#endif

/* Read from coprocessor 15 register */
#if defined(_lint)
#define read_cp15_register(CRn, CRm, op2, ret) (ret = 0)
#else
#define read_cp15_register(CRn, CRm, op2, ret)                        \
    __asm__ __volatile__ (                                            \
        "mrc    p15, 0, "_(ret)","STR(CRn)", "STR(CRm)","STR(op2)";"  \
        _OUTPUT(ret))
#endif

/* Write to coprocessor 15 register */
#if defined(_lint)
void __write_cp15_register(word_t val);
#define write_cp15_register(CRn, CRm, op2, val) __write_cp15_register(val)
#else
#define write_cp15_register(CRn, CRm, op2, val)                     \
{                                                                   \
    word_t v = (word_t)(val);                                         \
    __asm__ __volatile__ (                                          \
        "mcr    p15, 0, "_(v)","STR(CRn)", "STR(CRm)","STR(op2)";"  \
    _INPUT(v));                                                     \
};
#endif
/* CPWAIT - Wait for CP15 Update */
#if defined(__GNUC__)
#define CPWAIT                                  \
    __asm__ __volatile__ (                      \
    "mrc    p15, 0, r0, c2, c0, 0;  \n"         \
    "mov    r0, r0;                 \n"         \
    "sub    pc, pc, #4;             \n"         \
    ::: "r0")
#else
extern "C" void cpwait(void);
#define CPWAIT cpwait()
#endif

#endif

#endif /*_ARCH_ARM_XSCALE_SYSCON_H_*/
