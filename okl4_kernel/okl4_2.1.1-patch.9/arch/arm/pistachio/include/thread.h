/*
 * Copyright (c) 2003-2005, National ICT Australia (NICTA)
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
 * Description:   Thread switch and interrupt stack frames
 */

#ifndef __ARCH__ARM__THREAD_H__
#define __ARCH__ARM__THREAD_H__

#define ARM_SYSCALL_STACK_SIZE  16
#if CONFIG_ARM_VER == 5
#define ARM_USER_FLAGS_MASK     UL(0xf8000020)
#elif CONFIG_ARM_VER >= 6
#define ARM_USER_FLAGS_MASK     UL(0xf90f0220)
#endif

#define CPSR_MODE_MASK          0x1f
#define CPSR_USER_MODE          0x10
#define CPSR_SYS_MODE_TEST      0x0f

#define CPSR_THUMB_BIT          0x20
#define CPSR_I_BIT              0x80
#define CPSR_F_BIT              0x40
#define CPSR_JAVA_BIT           0x01000000


#if !defined(__ASSEMBLER__)

class arm_switch_stack_t {
    public:
        u32_t r4;   /* -0 - */
        u32_t r5;   /*  4   */
        u32_t r11;  /*  8   */
        u32_t lr;   /*  12  */
};
#endif

#define SS_R4       0
#define SS_R5       4
#define SS_R11      8
#define SS_LR       12

#if !defined(__ASSEMBLER__)

/*
 * IMPORTANT: A full exception context is indicated by setting the
 * bottom bit of the pc.  This means you need to be very careful at
 * places where the pc is being modified or used.  In particular, when
 * returning from an exception, the bottom bit needs to be masked out.
 */

/* Must match #defines below */
class arm_irq_context_t {
    public:
        /* STRUCT_START_MARKER <-- do not delete */
        u32_t klr;      /*  0   */
        u32_t r0;       /*  4   */
        u32_t r1;       /*--8-- */
        u32_t r2;       /*  12  */
        u32_t r3;       /*  16  */
        u32_t r4;       /*  20  */
        u32_t r5;       /* -24- */
        u32_t r6;       /*  28  */
        u32_t r7;       /*  32  */
        u32_t r8;       /*  36  */
        u32_t r9;       /*--40--*/
        u32_t r10;      /*  44  */
        u32_t r11;      /*  48  */
        u32_t r12;      /*  52  */
        u32_t sp;       /* -56- */
        u32_t lr;       /*  60  */
        u32_t pc;       /*  64  */
        u32_t cpsr;     /*  68  */
        /* STRUCT_END_MARKER <-- do not delete */
};

/* Must match #defines below */
class arm_syscall_context_t {
    public:
        u32_t sp;       /*  0   */
        u32_t lr;       /*  4   */
        u32_t pc;       /*  8   */
        u32_t cpsr;     /*  12  */
};
#endif

#define SC_SP           0
#define SC_LR           4
#define SC_PC           8
#define SC_CPSR         12

#define ARM_SYSCALL_STACK_SIZE  16

#if 0

#define PT_KLR          0
#define PT_R0           4
#define PT_R1           8
#define PT_R2           12
#define PT_R3           16
#define PT_R4           20
#define PT_R5           24
#define PT_R6           28
#define PT_R7           32
#define PT_R8           36
#define PT_R9           40
#define PT_R10          44
#define PT_R11          48
#define PT_R12          52
#define PT_SP           56
#define PT_LR           60
#define PT_PC           64
#define PT_CPSR         68

#define PT_SIZE         72
#endif

#if defined(__GNUC__)
#define SAVE_ALL_INT_TMP                \
    sub     sp,     sp,     #PT_SIZE;   \
    stmib   sp,     {r0-r14}^;          \
    ldr     r1,     tmp_spsr;           \
    str     lr,     [sp, #PT_PC];       \
    mov     r0,     sp;                 \
    str     r1,     [sp, #PT_CPSR];     \
    tst     r1,     #CPSR_SYS_MODE_TEST;\
    ldreq   sp,     stack_top           ;

#ifdef CONFIG_ENABLE_FASS
#define SAVE_ALL_INT_TMP_LINKED_DACR    \
    sub     sp,     sp,     #PT_SIZE;   \
    stmib   sp,     {r0-r14}^;          \
    ldr     r1,     tmp_spsr;           \
    ldr     ip,     kernel_access;      \
    str     lr,     [sp, #PT_PC];       \
    mov     r0,     sp;                 \
    str     r1,     [sp, #PT_CPSR];     \
    tst     r1,     #CPSR_SYS_MODE_TEST;\
    ldreq   sp,     stack_top;

#else
#define SAVE_ALL_INT_TMP_LINKED_DACR    SAVE_ALL_INT_TMP
#endif


#define LOAD_CONTEXT_INTO_SP                    \
    mov     r0, #ARM_GLOBAL_BASE;               \
    ldr     r0, [r0, #OFS_GLOBAL_CURRENT_TCB];  /* Get current TCB */   \
    and     r1, sp, #STACK_TEST;        /* Check for return to kernel */\
    cmp     r1, #STACK_TEST;                    \
    addeq   sp, r0, #OFS_TCB_ARCH_CONTEXT; /* Load context (if return to user) */

#define RESTORE_ALL                     \
    LOAD_CONTEXT_INTO_SP;               \
    ldr     lr,     [sp, #PT_CPSR];     \
    ldmib   sp,     {r0-r14}^;          \
    nop;                                \
    msr     spsr,   lr;                 \
    ldr     lr,     [sp, #PT_PC];       \
    add     sp,     sp,     #PT_SIZE;

#define RESTORE_ALL_ABT                 \
    ldr     lr,     [sp, #PT_CPSR];     \
    ldmib   sp,     {r0-r14}^;          \
    nop;                                \
    str     lr,     tmp_spsr;           \
    ldr     lr,     [sp, #PT_PC];       \
    add     sp,     sp,     #PT_SIZE;   \
    str     lr,     tmp_r14;

#ifdef CONFIG_ENABLE_FASS
#define SET_KERNEL_DACR                 \
    ldr     ip,     kernel_access;      \
    mcr     p15, 0, ip, c3, c0;
#else
#define SET_KERNEL_DACR
#endif

#ifdef CONFIG_ENABLE_FASS
#define SET_KERNEL_DACR_LINKED          \
    mcr     p15, 0, ip, c3, c0;
#else
#define SET_KERNEL_DACR_LINKED
#endif

// dacr = (current_domain_mask)));
#ifdef CONFIG_ENABLE_FASS
#define SET_USER_DACR                   \
    mov     ip,     #ARM_GLOBAL_BASE;   \
    ldr     ip,     [ip, #OFS_ARM_CURRENT_DOMAIN_MASK];\
    mcr     p15,    0, ip, c3, c0;

#else
#define SET_USER_DACR
#endif

#elif defined(__RVCT_GNU__)
#if defined(ASSEMBLY)
    MACRO
    SAVE_ALL_INT_TMP
    sub     sp,     sp,     #PT_SIZE
    stmib   sp,     {r0-r14}^
    ldr     r1,     tmp_spsr
    str     lr,     [sp, #PT_PC]
    mov     r0,     sp
    str     r1,     [sp, #PT_CPSR]
    tst     r1,     #CPSR_SYS_MODE_TEST
    ldreq   sp,     stack_top
    MEND

#ifdef CONFIG_ENABLE_FASS
    MACRO
    SAVE_ALL_INT_TMP_LINKED_DACR
    sub     sp,     sp,     #PT_SIZE
    stmib   sp,     {r0-r14}^
    ldr     r1,     tmp_spsr
    ldr     ip,     kernel_access
    str     lr,     [sp, #PT_PC]
    mov     r0,     sp
    str     r1,     [sp, #PT_CPSR]
    tst     r1,     #CPSR_SYS_MODE_TEST
    ldreq   sp,     stack_top
    MEND
#else
#define SAVE_ALL_INT_TMP_LINKED_DACR    SAVE_ALL_INT_TMP
#endif

    MACRO
    LOAD_CONTEXT_INTO_SP
    mov     r0, #ARM_GLOBAL_BASE
    ldr     r0, [r0, #OFS_GLOBAL_CURRENT_TCB]
    and     r1, sp, #STACK_TEST
    cmp     r1, #STACK_TEST
    addeq   sp, r0, #OFS_TCB_ARCH_CONTEXT
    MEND

    MACRO
    RESTORE_ALL
    LOAD_CONTEXT_INTO_SP
    ldr     lr,     [sp, #PT_CPSR]
    ldmib   sp,     {r0-r14}^
    nop
    msr     spsr_cxsf,   lr
    ldr     lr,     [sp, #PT_PC]
    add     sp,     sp,     #PT_SIZE
    MEND

    MACRO
    RESTORE_ALL_ABT
    ldr     lr,     [sp, #PT_CPSR]
    ldmib   sp,     {r0-r14}^
    nop
    str     lr,     tmp_spsr
    ldr     lr,     [sp, #PT_PC]
    add     sp,     sp,     #PT_SIZE
    str     lr,     tmp_r14
    MEND

#ifdef CONFIG_ENABLE_FASS
    MACRO
    SET_KERNEL_DACR
    ldr     ip,     kernel_access
    mcr     p15, 0, ip, c3, c0
    MEND
#else
#define SET_KERNEL_DACR
#endif

#ifdef CONFIG_ENABLE_FASS
    MACRO
    SET_KERNEL_DACR_LINKED
    mcr     p15, 0, ip, c3, c0
    MEND
#else
#define SET_KERNEL_DACR_LINKED
#endif

// dacr = (current_domain_mask);
#ifdef CONFIG_ENABLE_FASS
    MACRO
    SET_USER_DACR
    mov     ip,     #ARM_GLOBAL_BASE
    ldr     ip,     [ip, #OFS_ARM_CURRENT_DOMAIN_MASK]
    mcr     p15,    0, ip, c3, c0
    MEND

#else
#define SET_USER_DACR
#endif
#endif
#endif

#endif /* __ARCH__ARM__THREAD_H__ */
