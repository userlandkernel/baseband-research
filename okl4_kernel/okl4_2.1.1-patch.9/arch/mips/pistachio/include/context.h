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
 * Copyright (c) 2004-2006, National ICT Australia (NICTA)
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
 * Description:   Various context management classes
 * Author:        Carl van Schaik
 */ 

#ifndef __ARCH__MIPS__CONTEXT_H__
#define __ARCH__MIPS__CONTEXT_H__

#include <kernel/cpu/config.h>

#if defined(CONFIG_IS_64BIT)
# define SIZE(x)        ((x)<<1)
#else
# define SIZE(x)        (x)
#endif

#if !defined(ASSEMBLY)

#include <kernel/asid.h>

/* These functions deal with switching to a thread (and saving its context), and
 * the associated notification functions */

extern "C" void mips_return_from_notify0 (void);
extern "C" void mips_return_from_notify1 (void);
extern "C" void mips_return_from_notify2 (void);
extern "C" void mips_return_to_user (void);

class mips_switch_stack_t {
public:
    word_t  s0;         /*  0 |  0 */
    word_t  s1;         /*  4 |  8 */
    word_t  s8;         /*  8 | 16 */
    word_t  gp;         /* 12 | 24 */
    word_t  ra;         /* 16 | 32 */
};

#define MIPS_SWITCH_STACK_SIZE  SIZE(5*4)

/* must match #defines below */
class mips_irq_context_t
{
public:
#if defined(L4_32BIT)
    /* Padding for MIPS o32 ABI - argument save space */
    word_t      stack_res[4]; /* 0 | xxx */
#endif
    word_t      epc;    /*- 16 |   0-*/
    word_t      sp;     /*  20 |   8 */
    word_t      status; /*  24 |  16 */
    word_t      ra;     /*  28 |  24 */
    word_t      v0;     /*- 32 |  32-*/
    word_t      v1;     /*  36 |  40 */
    word_t      a1;     /*  40 |  48 */
    word_t      cause;  /*  44 |  56 */
    word_t      a2;     /*- 48 |  64-*/
    word_t      a3;     /*  52 |  72 */
    word_t      t9;     /*  56 |  80 */
    word_t      a0;     /*  60 |  88 */
    word_t      gp;     /*- 64 |  96-*/
    word_t      at;     /*  68 | 104 */
    word_t      t0;     /*  72 | 112 */
    word_t      t1;     /*  76 | 120 */
    word_t      hi;     /*- 80 | 128-*/
    word_t      t2;     /*  84 | 136 */
    word_t      t3;     /*  88 | 144 */
    word_t      lo;     /*  92 | 152 */
    word_t      t4;     /*- 96 | 160-*/
    word_t      t5;     /* 100 | 168 */
    word_t      t6;     /* 104 | 176 */
    word_t      t7;     /* 108 | 184 */
    word_t      t8;     /*-112 | 192-*/
    word_t      s0;     /* 116 | 200 */
    word_t      s1;     /* 120 | 208 */
    word_t      s2;     /* 124 | 216 */
    word_t      s3;     /*-128 | 224-*/
    word_t      s4;     /* 132 | 232 */
    word_t      s5;     /* 136 | 240 */
    word_t      s6;     /* 140 | 248 */
    word_t      s7;     /*-144 | 256-*/
    word_t      s8;     /* 148 | 264 */
    word_t      badvaddr;/*152 | 272 */
    word_t      x1;     /* 156 | 280 */   /* fills to make frame cache aligned */
};

#endif

/**** Switch ****/
#define SS_S0           SIZE( 0)
#define SS_S1           SIZE( 4)
#define SS_S8           SIZE( 8)
#define SS_GP           SIZE(12)
#define SS_RA           SIZE(16)

#if defined(ASSEMBLY)
/* Context save / restore : check, this is cache optimised */

#include <kernel/arch/regdef.h>
#include <kernel/arch/mipsregs.h>

/* If changing this, modify class above */
#define SAVE_SWITCH_STACK                       \
/* Save the Callee-saved registers:     */      \
/* s0..s2 ($16..$18)                    */      \
/* gp     ($28)                         */      \
/* s8     ($30)                         */      \
/* ra     ($31)                         */      \
    ADDIU   sp, sp, -MIPS_SWITCH_STACK_SIZE;    \
    STR     s0, SS_S0(sp);                      \
    STR     s1, SS_S1(sp);                      \
    STR     s8, SS_S8(sp);                      \
    STR     gp, SS_GP(sp);                      \
    STR     ra, SS_RA(sp);

#define RESTORE_SWITCH_STACK                    \
    LDR     ra, SS_RA(sp);                      \
    LDR     s0, SS_S0(sp);                      \
    LDR     s1, SS_S1(sp);                      \
    LDR     s8, SS_S8(sp);                      \
    LDR     gp, SS_GP(sp);                      \
    ADDIU   sp, sp, MIPS_SWITCH_STACK_SIZE;


/*
 * SAVE_ALL_INT saves all the registers into the user context block in the
 * current thread, and then loads the current TCBs stack.  (This doesn't
 * work for SMP does it??)  it leaves a pointer to the context in a0 for
 * the convenience of the trap handler functions
 */
#define SAVE_ALL_INT                    \
        .set    push;                   \
        .set    reorder;                \
        .set    noat;                   \
        mfc0    k1, CP0_STATUS;         /* get STATUS register k1 */    \
        li      k0, 0xffffffffffffffe0; /* clear IE, EXL, ERL, KSU */   \
        and     k0, k0, k1;             \
        mtc0    k0, CP0_STATUS;         /* Enter kernel mode */         \
        andi    k0, k1, ST_KSU;         /* Isolate KSU bits */          \
                                        \
        .set    noreorder;              \
        beq     k0, zero, 8f;           /* Branch if from KERNEL mode */        \
        lui     k0, %hi(KERNEL_BASE);           \
        .set    reorder;                        \
        STR     t2, %lo(K_TEMP2)(k0);           \
        STR     t0, %lo(K_TEMP0)(k0);           \
        STR     t1, %lo(K_TEMP1)(k0);           \
        STR     t3, %lo(K_TEMP3)(k0);           \
        STR     t4, %lo(K_TEMP4)(k0);           \
                                                \
        MFC0    t2, CP0_EPC;                    \
        mfc0    t3, CP0_CAUSE;                  \
        MFC0    t4, CP0_BADVADDR;               \
        LDR     t0, %lo(K_CURRENT_TCB)(k0);     /* Load current tcb */  \
        move    t1, k1;                         /* Move k1 to t1 in case we fault */    \
        STR     t1, OFS_TCB_ARCH_CONTEXT+PT_STATUS(t0);         /* Save status */       \
        STR     sp, OFS_TCB_ARCH_CONTEXT+PT_SP(t0);             /* Save old stack */    \
        STR     t2, OFS_TCB_ARCH_CONTEXT+PT_EPC(t0);            /* Save EPC */          \
        STR     t3, OFS_TCB_ARCH_CONTEXT+PT_CAUSE(t0);          /* Save CAUSE */        \
        STR     t4, OFS_TCB_ARCH_CONTEXT+PT_BADVADDR(t0);       /* Save BADVADDR */     \
        ADDIU   sp, t0, OFS_TCB_ARCH_CONTEXT;   /* update to user context block */      \
        STR     v0, PT_V0(sp);                  /* give a free register to keep track of */ \
        lui     k0, %hi(KERNEL_BASE);           /* reload k0 since we could have faulted on tcb */\
        LDR     t0, %lo(K_TEMP0)(k0);           \
        LDR     t1, %lo(K_TEMP1)(k0);           \
        LDR     t2, %lo(K_TEMP2)(k0);           \
        LDR     t3, %lo(K_TEMP3)(k0);           \
        LDR     t4, %lo(K_TEMP4)(k0);           \
        ADR     v0, __stack + STACK_TOP;        /* fault from user - we need a new stack. */\
        b       9f;                             \
8:;                                             \
        STR     t3, %lo(K_TEMP3)(k0);           \
        STR     t2, %lo(K_TEMP2)(k0);           \
        STR     t1, %lo(K_TEMP1)(k0);           \
        STR     t0, %lo(K_TEMP0)(k0);           \
        lui     t2, 0x8000;                     \
        and     t1, sp, t2;                     \
        beq     t1, t2, 7f;           /* XXX delay slot optimize */          \
        li      t2, 4;                          \
/* XXX */\
IF_MIPS64(dsll  t2, 60;)                        \
        and     t1, sp, t2;                     \
        beq     t1, t2, 7f;                     \
        li      AT, 2;                          \
        break;                                  \
/* leave the kernel exceptions state on the stack */ \
7:                                              \
        MFC0    t2, CP0_EPC;                    \
        mfc0    t3, CP0_CAUSE;                  \
        MFC0    t0, CP0_BADVADDR;               \
        move    t1, k1;                         \
        STR     t1, PT_STATUS-PT_SIZE(sp);      /* Save status */       \
        STR     t3, PT_CAUSE-PT_SIZE(sp);       /* Save CAUSE */        \
        STR     sp, PT_SP-PT_SIZE(sp);          /* Save old stack */    \
        STR     t0, PT_BADVADDR-PT_SIZE(sp);    /* Save BADVADDR */     \
        ADDIU   sp, sp, -(PT_SIZE);             /* New stack pointer */ \
        STR     t2, PT_EPC(sp);                 /* Save EPC */          \
        STR     v0, PT_V0(sp);                  /* give a free register to keep track of */ \
        lui     k0, %hi(K_CURRENT_TCB); \
        LDR     t0, %lo(K_TEMP0)(k0);           \
        LDR     t1, %lo(K_TEMP1)(k0);           \
        LDR     t2, %lo(K_TEMP2)(k0);           \
        LDR     t3, %lo(K_TEMP3)(k0);           \
        move    v0, sp;                 /* from kernel - keep the current stack */      \
9:;                                     \
        STR     ra, PT_RA(sp);          \
        STR     v1, PT_V1(sp);          \
        STR     a1, PT_A1(sp);          \
        STR     a2, PT_A2(sp);          \
        STR     a3, PT_A3(sp);          \
        STR     t9, PT_T9(sp);          \
        STR     a0, PT_A0(sp);          \
        STR     gp, PT_GP(sp);          \
        STR     $1, PT_AT(sp);          \
        .set    at;                     \
        mfhi    v1;                     \
        STR     t0, PT_T0(sp);          \
        STR     t1, PT_T1(sp);          \
        STR     v1, PT_HI(sp);          \
        mflo    t0;                     \
        STR     t2, PT_T2(sp);          \
        STR     t3, PT_T3(sp);          \
        STR     t0, PT_LO(sp);          \
        STR     t4, PT_T4(sp);          \
        STR     t5, PT_T5(sp);          \
        STR     t6, PT_T6(sp);          \
        STR     t7, PT_T7(sp);          \
        STR     t8, PT_T8(sp);          \
        STR     s0, PT_S0(sp);          \
        STR     s1, PT_S1(sp);          \
        STR     s2, PT_S2(sp);          \
        STR     s3, PT_S3(sp);          \
        STR     s4, PT_S4(sp);          \
        STR     s5, PT_S5(sp);          \
        STR     s6, PT_S6(sp);          \
        STR     s7, PT_S7(sp);          \
        STR     s8, PT_S8(sp);          \
        /* now make sure we have the valid kernel stack, */     \
        /* and leave the context pointer in a0 */       \
        move    a0, sp;                 \
        move    sp, v0;                 /* use the new stack */

#define SAVE_ALL_XTLB           \
        .set    push;           \
        .set    reorder;        \
        .set    noat;           \
        la      sp, XTLB_REFILL_STACK_end - PT_SIZE;    /* Static stack */      \
        STR     ra, PT_RA(sp);  \
        STR     k0, PT_X1(sp);  /* Save UTCB in k0 */           \
        STR     k1, PT_SP(sp);  /* Save old stack in k1 */      \
        MFC0    k0, CP0_EPC;    \
        STR     v0, PT_V0(sp);  \
        STR     k0, PT_EPC(sp); \
        STR     v1, PT_V1(sp);  \
        mfc0    k0, CP0_CAUSE;  \
        STR     a1, PT_A1(sp);  \
        STR     a2, PT_A2(sp);  \
        STR     k0, PT_CAUSE(sp);       \
        STR     a3, PT_A3(sp);  \
        STR     t9, PT_T9(sp);  \
        STR     a0, PT_A0(sp);  \
        STR     gp, PT_GP(sp);  \
        STR     $1, PT_AT(sp);  \
        .set    at;             \
        mfhi    v0;             \
        STR     t0, PT_T0(sp);  \
        STR     t1, PT_T1(sp);  \
        STR     v0, PT_HI(sp);  \
        mflo    v1;             \
        STR     t2, PT_T2(sp);  \
        STR     t3, PT_T3(sp);  \
        STR     v1, PT_LO(sp);  \
        STR     t4, PT_T4(sp);  \
        STR     t5, PT_T5(sp);  \
        STR     t6, PT_T6(sp);  \
        STR     t7, PT_T7(sp);  \
        STR     t8, PT_T8(sp);  \
        STR     s0, PT_S0(sp);  \
        STR     s1, PT_S1(sp);  \
        STR     s2, PT_S2(sp);  \
        STR     s3, PT_S3(sp);  \
        STR     s4, PT_S4(sp);  \
        STR     s5, PT_S5(sp);  \
        STR     s6, PT_S6(sp);  \
        STR     s7, PT_S7(sp);  \
        STR     s8, PT_S8(sp);

#define RESTORE_ALL             \
        .set    push;           \
        .set    noreorder;      \
        lui     k0, %hi(KERNEL_BASE);           \
        LDR     t0, %lo(K_CURRENT_TCB)(k0);     /* Load current tcb */  \
        and     a0, sp, STACK_TOP;  \
        sub     a0, a0, STACK_TOP;  \
        ADDIU   t0, t0, OFS_TCB_ARCH_CONTEXT;   \
        MOVZ    (sp, t0, a0);   \
        .set    reorder;        \
        LDR     a2, PT_A2(sp);  \
        LDR     a3, PT_A3(sp);  \
        LDR     t9, PT_T9(sp);  \
        LDR     a0, PT_A0(sp);  \
        LDR     gp, PT_GP(sp);  \
        .set    noat;           \
        LDR     $1, PT_AT(sp);  \
        LDR     t0, PT_T0(sp);  \
        LDR     t1, PT_T1(sp);  \
        LDR     s0, PT_HI(sp);  \
        LDR     t2, PT_T2(sp);  \
        LDR     t3, PT_T3(sp);  \
        LDR     s1, PT_LO(sp);  \
        mthi    s0;             \
        LDR     t4, PT_T4(sp);  \
        LDR     t5, PT_T5(sp);  \
        LDR     t6, PT_T6(sp);  \
        mtlo    s1;             \
        LDR     t7, PT_T7(sp);  \
        LDR     t8, PT_T8(sp);  \
        LDR     s0, PT_S0(sp);  \
        LDR     s1, PT_S1(sp);  \
        LDR     s2, PT_S2(sp);  \
        LDR     s3, PT_S3(sp);  \
        LDR     s4, PT_S4(sp);  \
        LDR     s5, PT_S5(sp);  \
        LDR     s6, PT_S6(sp);  \
        LDR     s7, PT_S7(sp);  \
        LDR     s8, PT_S8(sp);  \
        \
        mfc0    a1, CP0_STATUS; /* Status in v1 */      \
        LDR     v1, PT_EPC(sp); /* is out of order bad for cache? (still same cache line?) */\
        LDR     v0, PT_STATUS(sp);      \
        /* XXX - NOTE, Status register updates are not ATOMIC!!!, Interrupt Mask bits can change */ \
        ori     a1, a1, ST_EXL; /* set Exception Level */       \
        mtc0    a1, CP0_STATUS; /* to disable interrupts, we now can set EPC */         \
        li      ra, CONFIG_MIPS_STATUS_MASK; /* compute new status register */  \
        \
        and     a1, ra, a1;     \
        nor     ra, zero, ra;   \
        and     v0, ra, v0;     \
        LDR     ra, PT_RA(sp);  \
        or      k1, a1, v0;     \
        MTC0    v1, CP0_EPC;    /* restore EPC */       \
        LDR     v0, PT_V0(sp);  \
        ADDIU   k0, sp, -(OFS_TCB_ARCH_CONTEXT);        \
        LDR     v1, PT_V1(sp);  \
        LDR     a1, PT_A1(sp);  \
        LDR     sp, PT_SP(sp);  /* restore stack */     \
        mtc0    k1, CP0_STATUS; /* new status value */  \
        LDR     k0, OFS_TCB_UTCB_LOCATION(k0);  /* Load UTCB into k0 */ \
        nop


#define RESTORE_XTLB            \
        .set    push;           \
        .set    reorder;        \
        LDR     a2, PT_A2(sp);  \
        LDR     a3, PT_A3(sp);  \
        LDR     t9, PT_T9(sp);  \
        LDR     a0, PT_A0(sp);  \
        LDR     gp, PT_GP(sp);  \
        .set    noat;           \
        LDR     $1, PT_AT(sp);  \
        LDR     t0, PT_T0(sp);  \
        LDR     t1, PT_T1(sp);  \
        LDR     s0, PT_HI(sp);  \
        LDR     t2, PT_T2(sp);  \
        LDR     t3, PT_T3(sp);  \
        LDR     s1, PT_LO(sp);  \
        mthi    s0;             \
        LDR     t4, PT_T4(sp);  \
        LDR     t5, PT_T5(sp);  \
        LDR     t6, PT_T6(sp);  \
        mtlo    s1;             \
        LDR     t7, PT_T7(sp);  \
        LDR     t8, PT_T8(sp);  \
        LDR     s0, PT_S0(sp);  \
        LDR     s1, PT_S1(sp);  \
        LDR     s2, PT_S2(sp);  \
        LDR     s3, PT_S3(sp);  \
        LDR     s4, PT_S4(sp);  \
        LDR     s5, PT_S5(sp);  \
        LDR     s6, PT_S6(sp);  \
        LDR     s7, PT_S7(sp);  \
        LDR     s8, PT_S8(sp);  \
        \
        LDR     a1, PT_EPC(sp); \
        LDR     v0, PT_V0(sp);  \
        MTC0    a1, CP0_EPC;    /* restore EPC */   \
        LDR     v1, PT_V1(sp);  \
        LDR     ra, PT_RA(sp);  \
        LDR     a1, PT_A1(sp);  \
        LDR     k0, PT_X1(sp);  /* Restore UTCB in k0 */        \
        LDR     sp, PT_SP(sp);  /* restore stack */

#endif

#endif /* __ARCH__MIPS__CONTEXT_H__ */
