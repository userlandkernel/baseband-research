/*
 * Copyright (c) 2004, National ICT Australia
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
 *  Authors: Changhua Chen
 *  Created: Thu Aug 1 2007
 */
#ifndef _ARCH_ARM_COUNTERS_H_
#define _ARCH_ARM_COUNTERS_H_

#if (defined(CONFIG_CPU_ARM_XSCALE) || (defined(ARCH_ARM) && (ARCH_VER == 6)))
#define __HAVE_ARM_PMN_COUNTER
#endif

#if defined(__HAVE_ARM_PMN_COUNTER)
/* Common events */
#define I_Cache_Miss 0x0 
#define Inst_Stall 0x1 //I Cache Miss or ITLB Miss(v6: Micro TLB Miss) 
#define Data_Stall 0x2
#define Branch_Instr 0x5
#define Branch_MisPredict 0x6
#define Instructions 0x7
#define D_Cache_Access 0xa
#define D_Cache_Miss 0xb
#define D_Cache_WB 0xc
#define Software_Change_PC 0xd //Not include if Mode is changed.
#define Cycle_Count 0xFF //used as cycle Count.
#if defined(CONFIG_CPU_ARM_XSCALE)
/* XSCALE events */
#define ITLB_Miss 0x3
#define DTLB_Miss 0x4
#define D_Buffer_Stall_Cycle 0x8
#define D_Buffer_Stall_Event 0x9
#elif (defined(CONFIG_CPU_ARM_ARM1136JS) || defined (CONFIG_CPU_ARM_ARM176JZS))
/* ARM V6 common events */
#define I_Micro_TLB_Miss 0x3
#define D_Micro_TLB_Miss 0x4
#define D_Cachable_Cache_access 0x9
#define Main_TLB_Miss 0xf
#define Explicit_L2_D_Access 0x10 //Not include WB, I cache line fill, pt walk
#define D_Buffer_Stall_Cycle 0x11 //LSU request queue full, waiting for L2
#define Drain_Write_Buffer 0x12 //D memory barrier and Strongly Order Op.
#if defined(CONFIG_CPU_ARM_ARM1176JZS)
/* ARM1176 events */
#define Push_Return_Stack 0x23
#define Pop_Return_Stack 0x24
#define Branch_Return_Stack 0x25
#define MisPredict_Return_Stack 0x26
#endif
#endif

#ifndef REG_PMNC
#define REG_PMNC    0
#endif
#define REG_PMN0    2
#define REG_PMN1    3
#define PMNC_EVENTS_ENABLE  0x37UL
#define PMNC_EVT_OFL        (3 << 8)
#define PMNC_EVT_ENIRQ      (3 << 4)
#if defined(CONFIG_CPU_ARM_XSCALE)
#define PMNC_EVT0_SHIFT     12
#define PMNC_EVT1_SHIFT     20
#else
#define PMNC_EVT0_SHIFT     20
#define PMNC_EVT1_SHIFT     12
#endif
#define PMNC_EVT0_MASK      ~(0xFF << PMNC_EVT0_SHIFT)
#define PMNC_EVT1_MASK      ~(0xFF << PMNC_EVT1_SHIFT)

void setup_Event(L4_Word_t, L4_Word_t);
void restart_PMN(void);
void stop_PMN(void);
uint64_t get_PMN0_overflow_counter(void);
uint64_t get_PMN0_value(void);
uint64_t get_PMN1_overflow_counter(void);
uint64_t get_PMN1_value(void);
#endif /* __HAVE_ARM_PMN_COUNTER */
#endif /* _ARCH_ARM_COUNTERS_H_ */
