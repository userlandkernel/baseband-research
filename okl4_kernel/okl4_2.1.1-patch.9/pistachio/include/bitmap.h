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
 * Description:   Shared headers needed by most source files
 */

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <kernel/arch/special.h> /* for msb */
#include <atomic_ops/atomic_ops.h>

//#define TRACEBM TRACEF
#define TRACEBM(...)

#define BITMAP_SIZE(bits)   (((bits) + (BITS_WORD-1)) / BITS_WORD)

#define BITMAP_MASK(bit)    (1UL << ((bit) % BITS_WORD));
#define BITMAP_BITNO(bit)   (bit)

/* base bitmap is just a machine word in size */
typedef word_t bitmap_t;

/* initialize the bitmap to all set or all clear  (length in bits) */
INLINE void bitmap_init(bitmap_t * bm, word_t length, word_t set)
{
    word_t i;

    TRACEBM("bitmap_init bm = %p, length = %d, set = %d\n", bm, length, set);
    /* normalize set value to 0 or -1 */
    if (set) {
        set = ~0UL;
    }

    /* initialize all words in bitmap */
    for (i = 0; i < BITMAP_SIZE(length); i++)
        bm[i] = set;

    /* correctly clear unused bits in last in bitmap word */
    if (length % BITS_WORD) {
        bm[i-1] &= ((1UL << ((length % BITS_WORD))) - 1);
    }

    TRACEBM("bitmap_init: last word %lx\n", bm[i-1]);
}

/* set a particular bit in a bitmap 
 * threadsafe on MUNITS 
 */ 
INLINE void bitmap_set(bitmap_t * bm, word_t bit)
{
    TRACEBM("bm=%p, bit=%ld\n", bm, bit);
#ifdef CONFIG_MUNITS
    okl4_atomic_word_t * w = (okl4_atomic_word_t *)&bm[bit/BITS_WORD];
    okl4_atomic_plain_word_t mask = BITMAP_MASK(bit);
    okl4_atomic_or(w, mask);
#else
    bm[bit/BITS_WORD] |= BITMAP_MASK(bit);
#endif
}

/* clear a particular bit in the bitmap 
 * threadsafe on MUNITS
 */
INLINE void bitmap_clear(bitmap_t * bm, word_t bit)
{
    TRACEBM("bm=%p, bit=%ld\n", bm, bit);
#ifdef CONFIG_MUNITS
    okl4_atomic_word_t * w = (okl4_atomic_word_t *)&bm[bit/BITS_WORD];
    okl4_atomic_plain_word_t mask = ~BITMAP_MASK(bit);
    okl4_atomic_and(w, mask);
#else
    bm[bit/BITS_WORD] &= ~BITMAP_MASK(bit);
#endif
}


/* determine the status of a particular bit in the bitmap */
INLINE bool bitmap_isset(bitmap_t * bm, word_t bit)
{
    TRACEBM("bm=%p, bit=%ld\n", bm, bit);
    return (bm[bit/BITS_WORD]) & BITMAP_MASK(bit);
}

/* Check if every bit in the bitmap is set (length in bits) */
INLINE bool bitmap_isallset(bitmap_t * bm, word_t length)
{
    word_t i;
    word_t check = ~0UL;

    TRACEBM("bitmap_isallset bm = %p, length = %d\n", bm, length);

    /* check complete words in bitmap */
    for (i = 0; i < BITMAP_SIZE(length)-1; i++)
        if(bm[i] != check)
            return false;

    /* correctly check last partial word in bitmap */
    return ( bm[i] == ((1UL << ((length % BITS_WORD))) - 1));
}

/* Check if every bit in the bitmap is clear (length in bits) */
INLINE bool bitmap_isallclear(bitmap_t * bm, word_t length)
{
    word_t i;
    word_t check = 0;

    TRACEBM("bitmap_isallclear bm = %p, length = %d\n", bm, length);

    /* check complete words in bitmap */
    for (i = 0; i < BITMAP_SIZE(length)-1; i++)
        if(bm[i] != check)
            return false;

    /* correctly check last partial word in bitmap */
    return ( bm[i] == 0 );
}

/**
 * Check if every bit in range of the bitmap is set
 * *inclusive* of first and last
 */
INLINE bool bitmap_israngeset(bitmap_t * bm, word_t first, word_t last)
{
    word_t i = first / BITS_WORD;
    word_t j = last / BITS_WORD;
    word_t first_mask = ((1<<(first % BITS_WORD))-1);   /* exclusive mask */
    word_t last_mask = ((1<<((last % BITS_WORD)+1))-1); /* inclusive mask */
    word_t check;

    TRACEBM("bitmap_israngeset bm = %p, a = %ld, b = %ld\n", bm, first, last);

    if (EXPECT_TRUE(i == j))
    {
        check = first_mask ^ last_mask;
        if ((bm[i] & check) == check)
            return true;
        return false;
    }

    check = ~first_mask;
    do {
        if ((bm[i] & check) != check)
            return false;
        check = TWOSCOMP(1UL);
        i++;
    } while (i < j);

    check = last_mask;
    if ((bm[i] & check) != check)
        return false;

    return true;
}


/*
 * Find the first bit set in the bitmap (length in bits)
 * returns -1 if no bit is found.
 *
 * length must be same as length used for bitmap_init()
 */
INLINE int bitmap_findfirstset(bitmap_t * bm, word_t length)
{
    word_t i;

    /* scan the bitfield for a set bit */
    for (i = 0; i < BITMAP_SIZE(length); i++)
    {
        if (EXPECT_TRUE(bm[i] != 0)) {
            TRACEBM("bm[i] = %lx, msb = %ld\n", bm[i], msb(bm[i]));
            return (i*BITS_WORD) + BITMAP_BITNO( msb(bm[i]) );
        }
    }

    TRACEBM("No bits set\n");
    return -1;
}

/*
 * Find the first bit clear in the bitmap (length in bits)
 * returns -1 if no bit is found.
 *
 * length must be same as length used for bitmap_init()
 */
INLINE int bitmap_findfirstclear(bitmap_t * bm, word_t length)
{
    word_t i;

    /* scan the bitfield for a set bit */
    for (i = 0; i < BITMAP_SIZE(length); i++)
    {
        word_t bits = !bm[i];
        if (bits) {
            TRACEBM("bm[i] = %lx, msb = %ld\n", bm[i], msb(bits));
            return (i*BITS_WORD) + BITMAP_BITNO( msb(bits) );
        }
    }

    TRACEBM("No bits clear\n");
    return -1;
}

#endif /* __BITMAP_H__ */
