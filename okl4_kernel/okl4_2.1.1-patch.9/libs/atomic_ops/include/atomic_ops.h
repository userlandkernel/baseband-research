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
 * Author: Malcolm Purvis <malcolmp@ok-labs.com>
 * Author: Carlos Dyonisio <medaglia@ok-labs.com>
 */

#ifndef __ATOMIC_OPS_H__
#define __ATOMIC_OPS_H__

/*
 * Kernel and Userland definitions.
 *
 * The atomic ops API is common between the kernel and userland
 * but many of the implementation details differ.  
 */
#ifdef KENGE_PISTACHIO

#define __ATOMIC_OPS_IN_KERNEL__
#include <kernel/config.h>
#include <kernel/types.h>

typedef word_t okl4_atomic_plain_word_t;

#else /* In userland */

#include <l4/types.h>

typedef L4_Word_t okl4_atomic_plain_word_t;

#endif /* kernel or userland */

/*-------------------------------------------------------------------------*/
                        /* Atomic Ops API. */

/*
 * IMPORTANT!
 * If you plan to change the structure okl4_atomic_word_t, please add the new
 * elements after value. For more information read the comment in
 * arch/arm/libs/atomic_ops/v5/src/arm_atomic_ops.spp:66
 */

typedef struct {
    volatile okl4_atomic_plain_word_t value;
} okl4_atomic_word_t;

#define ATOMIC_INIT(i)  { (i) }

static inline void
okl4_atomic_init(okl4_atomic_word_t *a, okl4_atomic_plain_word_t v)
{
    a->value = v;
}

#if defined(ARCH_ARM) && defined(ARCH_VER) && (ARCH_VER < 6) && \
         (!defined(__ATOMIC_OPS_IN_KERNEL__) || defined(MACHINE_SMP))

/* 
 * If it is ARMv4/v5, the function declarations may change
 * and are defined in the arch specific header file,
 * as some of then can't be declared static because of
 * the assembler implementation.
 */

#else 

/** Atomic set and return new value */
static okl4_atomic_plain_word_t okl4_atomic_set(okl4_atomic_word_t* target,
                                      okl4_atomic_plain_word_t value);

/*** Bitwise operations. ***/

/** Atomic AND */
static void okl4_atomic_and(okl4_atomic_word_t* target,
                       okl4_atomic_plain_word_t mask);
/** Atomic AND and return new value */
static okl4_atomic_plain_word_t okl4_atomic_and_return(okl4_atomic_word_t* target,
                                             okl4_atomic_plain_word_t mask);

/** Atomic AND and return old value */
static okl4_atomic_plain_word_t okl4_atomic_and_return_old(okl4_atomic_word_t* target,
                                             okl4_atomic_plain_word_t mask);
/** Atomic OR */
static void okl4_atomic_or(okl4_atomic_word_t* target,
                      okl4_atomic_plain_word_t mask);
/** Atomic OR and return new value */
static okl4_atomic_plain_word_t okl4_atomic_or_return(okl4_atomic_word_t* target,
                                            okl4_atomic_plain_word_t mask);

/** Atomic XOR */
static void okl4_atomic_xor(okl4_atomic_word_t* target,
                       okl4_atomic_plain_word_t mask);
/** Atomic XOR and return new value */
static okl4_atomic_plain_word_t okl4_atomic_xor_return(okl4_atomic_word_t* target,
                                             okl4_atomic_plain_word_t mask);

/*** Bitfield operations. ***/

static void okl4_atomic_set_bit(okl4_atomic_word_t *target, unsigned long bit);
static void okl4_atomic_clear_bit(okl4_atomic_word_t *target, unsigned long bit);
static int okl4_atomic_clear_bit_return_old(okl4_atomic_word_t *target, unsigned long bit);
static void okl4_atomic_change_bit(okl4_atomic_word_t *target, unsigned long bit);

/*** Arithmetic operations. ***/
static void okl4_atomic_add(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v);
static okl4_atomic_plain_word_t okl4_atomic_add_return(okl4_atomic_word_t *target,
                                             okl4_atomic_plain_word_t v);

static void okl4_atomic_sub(okl4_atomic_word_t *target, okl4_atomic_plain_word_t v);
static okl4_atomic_plain_word_t okl4_atomic_sub_return(okl4_atomic_word_t *target,
                                             okl4_atomic_plain_word_t v);

static void okl4_atomic_inc(okl4_atomic_word_t *target);
static okl4_atomic_plain_word_t okl4_atomic_inc_return(okl4_atomic_word_t *target);

static void okl4_atomic_dec(okl4_atomic_word_t *target);
static okl4_atomic_plain_word_t okl4_atomic_dec_return(okl4_atomic_word_t *target);

/*** General operations. ***/

static int okl4_atomic_compare_and_set(okl4_atomic_word_t *target,
                                  okl4_atomic_plain_word_t old_val,
                                  okl4_atomic_plain_word_t new_val);

/* Memory barrier operations. */

static void okl4_atomic_barrier_write(void);
static void okl4_atomic_barrier_write_smp(void);
static void okl4_atomic_barrier_read(void);
static void okl4_atomic_barrier_read_smp(void);
static void okl4_atomic_barrier(void);
static void okl4_atomic_barrier_smp(void);

#endif

/*---------------------------------------------------------------------------*/

/* Architecture independent definitions. */

static okl4_atomic_plain_word_t okl4_atomic_read(okl4_atomic_word_t *target);
static void okl4_atomic_compiler_barrier(void);

static inline okl4_atomic_plain_word_t
okl4_atomic_read(okl4_atomic_word_t *target)
{
    return target->value;
}

static inline void okl4_atomic_compiler_barrier()
{
#if defined(__RVCT__) || defined(__RVCT_GNU__)
    __memory_changed();
#elif defined(__ADS__)
    __asm("");
#else
    asm volatile (""::: "memory");
#endif
}


/* Architecture dependent definitions. */
#include <atomic_ops/arch/atomic_ops.h>

#endif /* __ATOMIC_OPS_H__ */
