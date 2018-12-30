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
 * Author: Carl van Schaik <carl@ok-labs.com>
 *
 * This file is only included by the main atomic_ops.h, so all of that
 * file's definitions are available.
 */

#ifndef __ARCH_MIPS_ATOMIC_OPS_H__
#define __ARCH_MIPS_ATOMIC_OPS_H__

#if defined(__ATOMIC_OPS_IN_KERNEL__) && !defined(MACHINE_SMP)

/* Ensure that the kernel is non-smp. */
#if CONFIG_NUM_UNITS > 1
#error CONFIG_NUM_UNITS > 1 but smp not defined in machines.py.
#endif

/* The kernel does not require atomic ops on non-smp machines. */

#include <atomic_ops/unsafe_generic.h>

#else

#if defined(L4_32BIT)
#define STR     "SW"
#define LLW     "LL"
#define SCW     "SC"
#elif defined(L4_64BIT)
#define STR     "SD"
#define LLW     "LLD"
#define SCW     "SCD"
#endif

static inline okl4_atomic_plain_word_t
okl4_atomic_set(okl4_atomic_word_t* target, okl4_atomic_plain_word_t value)
{
    __asm__ __volatile__ (
        ""STR"  %0, %1  \n"
        :: "r" (value), "m" (target->value)
        : "memory"
    );

    return value;
}

/* Bitwise operations. */
static inline void
okl4_atomic_and(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       and     %0, %1, %3  \n"
        "       "SCW"   %0, %2      \n"
        "       beq     %0, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");
}

static inline okl4_atomic_plain_word_t
okl4_atomic_and_return(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       and     %0, %1, %3  \n"
        "       move    %1, %0      \n"
        "       "SCW"   %1, %2      \n"
        "       beq     %1, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");

    return result;
}

static inline void
okl4_atomic_or(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       or      %0, %1, %3  \n"
        "       "SCW"   %0, %2      \n"
        "       beq     %0, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");
}

static inline okl4_atomic_plain_word_t
okl4_atomic_or_return(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       or      %0, %1, %3  \n"
        "       move    %1, %0      \n"
        "       "SCW"   %1, %2      \n"
        "       beq     %1, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");

    return result;
}

static inline void
okl4_atomic_xor(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       xor     %0, %1, %3  \n"
        "       "SCW"   %0, %2      \n"
        "       beq     %0, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");
}

static inline okl4_atomic_plain_word_t
okl4_atomic_xor_return(okl4_atomic_word_t* target, okl4_atomic_plain_word_t mask)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       xor     %0, %1, %3  \n"
        "       move    %1, %0      \n"
        "       "SCW"   %1, %2      \n"
        "       beq     %1, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (mask)
        : "cc", "memory");

    return result;
}

/* Bitfield operations. */

static inline void
okl4_atomic_set_bit(okl4_atomic_word_t *target, unsigned long bit)
{
    int word = bit / (sizeof(okl4_atomic_word_t) * 8);
    okl4_atomic_plain_word_t mask = 1ul << (bit % (sizeof(okl4_atomic_word_t) * 8));

    okl4_atomic_or(&target[word], mask);
}

static inline void
okl4_atomic_clear_bit(okl4_atomic_word_t *target, unsigned long bit)
{
    int word = bit / (sizeof(okl4_atomic_word_t) * 8);
    okl4_atomic_plain_word_t mask = ~(1ul << (bit % (sizeof(okl4_atomic_word_t) * 8)));

    okl4_atomic_and(&target[word], mask);
}

static inline void
okl4_atomic_change_bit(okl4_atomic_word_t *target, unsigned long bit)
{
    int word = bit / (sizeof(okl4_atomic_word_t) * 8);
    okl4_atomic_plain_word_t mask = 1ul << (bit % (sizeof(okl4_atomic_word_t) * 8));

    okl4_atomic_xor(&target[word], mask);
}


/* Arithmetic operations. */
static inline void
okl4_atomic_add(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       add     %0, %1, %3  \n"
        "       "SCW"   %0, %2      \n"
        "       beq     %0, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (v)
        : "cc", "memory");
}

static inline okl4_atomic_plain_word_t
okl4_atomic_add_return(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       add     %0, %1, %3  \n"
        "       move    %1, %0      \n"
        "       "SCW"   %1, %2      \n"
        "       beq     %1, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "Ir" (v)
        : "cc", "memory");

    return result;
}

static inline void
okl4_atomic_sub(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v)
{
    okl4_atomic_add(target, -v);
}

static inline okl4_atomic_plain_word_t
okl4_atomic_sub_return(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v)
{
    return okl4_atomic_add_return(target, -v);
}

static inline void
okl4_atomic_inc(okl4_atomic_word_t *target)
{
    okl4_atomic_add(target, 1);
}

static inline okl4_atomic_plain_word_t
okl4_atomic_inc_return(okl4_atomic_word_t *target)
{
    return okl4_atomic_add_return(target, 1);
}


static inline void
okl4_atomic_dec(okl4_atomic_word_t *target)
{
    okl4_atomic_add(target, -1);
}

static inline okl4_atomic_plain_word_t
okl4_atomic_dec_return(okl4_atomic_word_t *target)
{
    return okl4_atomic_add_return(target, -1);
}

/* General operations. */

static inline int
okl4_atomic_compare_and_set(okl4_atomic_word_t *target,
                       okl4_atomic_plain_word_t old_val,
                       okl4_atomic_plain_word_t new_val)
{
    okl4_atomic_plain_word_t tmp;
    okl4_atomic_plain_word_t result;

    __asm__ __volatile__(
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "1:     "LLW"   %1, %2      \n"
        "       sub     %0, %1, %3  \n"     // check if value == old value
#if __mips <= 3
        "       beqzl   %0, 9f      \n"
        "       move    %1, %4      \n"     // set new value if above true
        "9:                         \n"
#else
        "       movz    %1, %4, %0  \n"     // set new value if above true
#endif
        "       "SCW"   %1, %2      \n"
        "       beq     %1, 0,  1b  \n"
        "       nop                 \n"
        "       .set    pop         \n"
        : "=&r" (result), "=&r" (tmp)
        : "m" (target->value), "r" (old_val), "r" (new_val)
        : "cc", "memory");

    return !result;
}

static inline void
okl4_atomic_barrier_write(void)
{
    __asm__ __volatile__ (
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "       sync                \n"
        "       .set    pop         \n"
        : : : "memory");
}

static inline void
okl4_atomic_barrier_write_smp(void)
{
    okl4_atomic_barrier_write();
}

static inline void
okl4_atomic_barrier_read(void)
{
    __asm__ __volatile__ (
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "       sync                \n"
        "       .set    pop         \n"
        : : : "memory");
}

static inline void
okl4_atomic_barrier_read_smp(void)
{
    okl4_atomic_barrier_read();
}

static inline void
okl4_atomic_barrier(void)
{
    __asm__ __volatile__ (
        "       .set    push        \n"
        "       .set    noreorder   \n"
        "       sync                \n"
        "       .set    pop         \n"
        : : : "memory");
}

static inline void
okl4_atomic_barrier_smp(void)
{
    okl4_atomic_barrier();
}
#endif

#endif /* __ARCH_MIPS_ATOMIC_OPS_H__ */
