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

#include <bench/bench.h>
#include <l4/kdebug.h>
#include <stdio.h>
#include <cycles/counters.h>

#if defined(__HAVE_ARM_PMN_COUNTER)
static int get_num_counters(struct counter *__unused);
static const char *get_name(struct counter *counter, int sub_counter);
static const char *get_unit(struct counter *__unused, int counter);
static void init(struct counter *counter);
static void setup(struct counter *counter);
static void stop(void);
static void start(void);
static uint64_t get_count(int counter);

#define COUNTER {   \
        .get_num_counters = get_num_counters, \
    .get_name = get_name, \
    .get_unit = get_unit, \
    .init = init, \
    .setup = setup, \
    .start = start, \
    .stop = stop, \
    .get_count = get_count \
}

/* Common Counters */
struct counter arm_PMN_branch_prediction = COUNTER;     //0x5, 0x6
struct counter arm_PMN_stall = COUNTER;                 //0x1, 0x2
struct counter arm_PMN_cache_miss = COUNTER;            //0x0, 0xb
struct counter arm_PMN_d_cache_detail = COUNTER;        //0xa, 0xc
struct counter arm_PMN_PC = COUNTER;                    //0x7, 0xd

struct counter arm_PMN_i_cache_efficiency = COUNTER;    //0x7, 0x0
struct counter arm_PMN_d_cache_efficiency = COUNTER;    //0xa, 0xb
struct counter arm_PMN_i_fetch_latency = COUNTER;       //0x1, 0x0
struct counter arm_PMN_d_stall = COUNTER;               //0x2, 0xc

/* XSCALE Counters */
struct counter arm_xscale_PMN_tlb = COUNTER;            //0x3, 0x4
struct counter arm_xscale_PMN_d_buffer_stall = COUNTER; //0x8, 0x9
/* ARM V6 common Counters */
struct counter arm_V6_PMN_i_tlb = COUNTER;              //0x3, 0xf
struct counter arm_V6_PMN_d_tlb = COUNTER;              //0x4, 0xf
struct counter arm_V6_PMN_d_L2 = COUNTER;               //0x9, 0x10
struct counter arm_V6_PMN_d_buffer = COUNTER;           //0x11, 0x12
/* ARM1176 Counters */
struct counter arm_V6_PMN_RS = COUNTER;                 //0x23, 0x24
struct counter arm_V6_PMN_RS_Prediction = COUNTER;      //0x25, 0x26

static int
get_num_counters(struct counter *__unused)
{
    return 2;
}

static const char *
get_name(struct counter *counter, int sub_counter)
{
    /* Common Counters */
    if (counter == &arm_PMN_branch_prediction)
    {
       return (sub_counter == 0) ? "Branch Predicted successfully" : "Branch Mis-Predicted";
    }
    else if (counter == &arm_PMN_stall)
    {
        return (sub_counter == 0) ? "Instruction Stall (i cache or i TLB miss)" : "Data Stall (data dependency)";
    }
    else if (counter == &arm_PMN_cache_miss)
    {
        return (sub_counter == 0) ? "I Cache Miss" : "D Cache Miss";
    }
    else if (counter == &arm_PMN_d_cache_detail)
    {
        return (sub_counter == 0) ? "D Cache Access" : "D Cache WB";
    }
    else if (counter == &arm_PMN_PC)
    {
        return (sub_counter == 0) ? "Executed Instruction" : "Non-mode PC Change";
    }
    else if (counter == &arm_PMN_i_cache_efficiency)
    {
        return (sub_counter == 0) ? "Executed Instruction": "I Cache Miss";
    }
    else if (counter == &arm_PMN_d_cache_efficiency)
    {
        return (sub_counter == 0) ? "D Cache Access": "D Cache Miss";
    }
    else if (counter == &arm_PMN_i_fetch_latency)
    {
        return (sub_counter == 0) ? "Instruction Stall (i cache or iTLB miss)": "I Cache Miss";
    }
    else if (counter == &arm_PMN_d_stall)
    {
        return (sub_counter == 0) ? "Data Stall(data dependency)": "D Cache WB";
    }

#if defined(CONFIG_CPU_ARM_XSCALE)
    /* XSCALE Counters */
    else if (counter == &arm_xscale_PMN_tlb)
    {
        return (sub_counter == 0) ? "I TLB Miss" : "D TLB Miss";
    }
    else if (counter == &arm_xscale_PMN_d_buffer_stall)
    {
        return (sub_counter == 0) ? "D Buffer Stall Cycle" : "D Buffer Stall Event";
    }
#endif
#if (defined(CONFIG_CPU_ARM_ARM1136JS) || defined (CONFIG_CPU_ARM_ARM1176JZS))
    /* ARM V6 Common Counters */
    else if (counter == &arm_V6_PMN_i_tlb)
    {
        return (sub_counter == 0) ? "I Micro TLB Miss" : "Main TLB Miss";
    }
    else if (counter == &arm_V6_PMN_d_tlb)
    {
        return (sub_counter == 0) ? "D Micro TLB Miss" : "Main TLB Miss";
    }
    else if (counter == &arm_V6_PMN_d_L2)
    {
        return (sub_counter == 0) ? "L1 Data Access (Cachable D Cache Access)" : "Explicit L2 Data Access (Not include WB)"; 
    }
    else if (counter == &arm_V6_PMN_d_buffer)
    {
        return (sub_counter == 0) ? "D Buffer Stall Cycle" : "Drain Write Buffer";
    }
#if defined(CONFIG_CPU_ARM_ARM1176JZS)
    /* ARM 1176 Counters */
    else if (counter == &arm_V6_PMN_RS)
    {
        return (sub_counter == 0) ? "Push Return Stack" : "Pop Return Stack"; 
    }
    else if (Counter == &arm_V6_PMN_RS_Prediction)
    {
        return (sub_counter == 0) ? "Return Stack Predicted successfully" : "Return Stack Mis-Predicted";
    }
#endif
#endif
    printf("BADNESS %p\n", counter);
    return "ERROR";
}

static const char *
get_unit(struct counter *__unused, int counter)
{
    return "counts";
}

static void
init(struct counter *counter)
{
}

void setup_Event(L4_Word_t e0, L4_Word_t e1)
{
    L4_Word_t PMNC = L4_KDB_PMN_Read(REG_PMNC) & PMNC_EVT0_MASK & PMNC_EVT1_MASK;
    PMNC |= (e0 << PMNC_EVT0_SHIFT) | (e1 << PMNC_EVT1_SHIFT) | PMNC_EVT_OFL;
    L4_KDB_PMN_Write(REG_PMNC, PMNC);
}

void restart_PMN(void)
{
    L4_KDB_PMN_Ofl_Write(REG_PMN0, 0);
    L4_KDB_PMN_Ofl_Write(REG_PMN1, 0);
    L4_Word_t PMNC = L4_KDB_PMN_Read(REG_PMNC);
    L4_KDB_PMN_Write(REG_PMNC, PMNC | PMNC_EVENTS_ENABLE);
}

void stop_PMN(void)
{
    L4_Word_t PMNC = L4_KDB_PMN_Read(REG_PMNC);
    //On ARM11, PMNC IRQ can only be deasserted when PMU is enabled,
    //therefore, need to clear overflow flags and disable IRQ before disbale PMU.
    L4_KDB_PMN_Write(REG_PMNC, (PMNC | PMNC_EVT_OFL) & ~PMNC_EVT_ENIRQ);
    L4_KDB_PMN_Write(REG_PMNC, PMNC & ~PMNC_EVENTS_ENABLE);
}

uint64_t get_PMN0_overflow_counter(void)
{
    return (uint64_t)L4_KDB_PMN_Ofl_Read(REG_PMN0);
}

uint64_t get_PMN0_value(void)
{
    return (uint64_t)L4_KDB_PMN_Read(REG_PMN0);
}

uint64_t get_PMN1_overflow_counter(void)
{
    return (uint64_t)L4_KDB_PMN_Ofl_Read(REG_PMN1);
}

uint64_t get_PMN1_value(void)
{
    return (uint64_t)L4_KDB_PMN_Read(REG_PMN1);
}
static void
setup(struct counter *counter)
{
    /* Common Counters */
    if (counter == &arm_PMN_branch_prediction)
    {
        setup_Event(Branch_Instr, Branch_MisPredict);  
    }
    else if (counter == &arm_PMN_stall)
    {
        setup_Event(Inst_Stall, Data_Stall);
    }
    else if (counter == &arm_PMN_cache_miss)
    {
        setup_Event(I_Cache_Miss, D_Cache_Miss);
    }
    else if (counter == &arm_PMN_d_cache_detail)
    {
        setup_Event(D_Cache_Access, D_Cache_WB);
    }
    else if (counter == &arm_PMN_PC)
    {
        setup_Event(Instructions, Software_Change_PC);
    }
    else if (counter == &arm_PMN_i_cache_efficiency)
    {
        setup_Event(Instructions, I_Cache_Miss);
    }
    else if (counter == &arm_PMN_d_cache_efficiency)
    {
        setup_Event(D_Cache_Access, D_Cache_Miss);
    }
    else if (counter == &arm_PMN_i_fetch_latency)
    {
        setup_Event(Inst_Stall, I_Cache_Miss);
    }
    else if (counter == &arm_PMN_d_stall)
    {
        setup_Event(Data_Stall, D_Cache_WB);
    }
#if defined(CONFIG_CPU_ARM_XSCALE)
    /* XSCALE Counters */
    else if (counter == &arm_xscale_PMN_tlb)
    {
        setup_Event(ITLB_Miss, DTLB_Miss);
    }
    else if (counter == &arm_xscale_PMN_d_buffer_stall)
    {
        setup_Event(D_Buffer_Stall_Cycle, D_Buffer_Stall_Event);
    }
#endif
#if (defined(CONFIG_CPU_ARM_ARM1136JS) || defined (CONFIG_CPU_ARM_ARM1176JZS))
    /* ARM V6 Common Counters */
    else if (counter == &arm_V6_PMN_i_tlb)
    {
        setup_Event(I_Micro_TLB_Miss, Main_TLB_Miss);
    }
    else if (counter == &arm_V6_PMN_d_tlb)
    {
        setup_Event(D_Micro_TLB_Miss, Main_TLB_Miss);
    }
    else if (counter == &arm_V6_PMN_d_L2)
    {
        setup_Event(D_Cachable_Cache_access, Explicit_L2_D_Access);
    }
    else if (counter == &arm_V6_PMN_d_buffer)
    {
        setup_Event(D_Buffer_Stall_Cycle, Drain_Write_Buffer);
    }
#if defined(CONFIG_CPU_ARM_ARM1176JZS)
    /* ARM 1176 Counters */
    else if (counter == &arm_V6_PMN_RS)
    {
        setup_Event(Push_Return_Stack, Pop_Return_Stack);
        return (sub_counter == 0) ? "Push Return Stack" : "Pop Return Stack"; 
    }
    else if (Counter == &arm_V6_PMN_RS_Prediction)
    {
        setup_Event(Branch_Return_Stack, MisPredict_Return_Stack);
    }
#endif
#endif
}

static void
start(void)
{
    restart_PMN();
}

static void
stop(void)
{
    stop_PMN();
}

static uint64_t
get_count(int counter)
{
    return ((counter == 0) ? ((get_PMN0_overflow_counter() << 32) + get_PMN0_value()) : ((get_PMN1_overflow_counter() << 32) + get_PMN1_value()));
}
#endif
