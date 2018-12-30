/*
 * Copyright (c) 2002, Karlsruhe University
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
 * Description:   declaration of lib functions
 */
#ifndef __GENERIC__LIB_H__
#define __GENERIC__LIB_H__

extern "C" void * memcpy (void * dst, const void * src, unsigned int len);
extern "C" void * memset (void * dst, unsigned int c, unsigned int len);
extern "C" WEAK void strcpy(char * dst, const char * src);

inline word_t msb_simple(word_t w)
{
    word_t bit = BITS_WORD - 1;
    word_t test = 1UL << (BITS_WORD-1);

    while (!(w & test)) {
        w <<= 1;
        bit --;
    }
    return bit;
}

inline word_t msb_byte_search(word_t w)
{
    word_t bit = BITS_WORD - 1;
    word_t test = 1UL << bit;

    while (!(w >> (BITS_WORD-8))) {
        w <<= 8;
        bit -= 8;
    }

    while (!(w & test)) {
        w <<= 1;
        bit --;
    }
    return bit;
}

inline word_t msb_binary(word_t w)
{
    word_t bit = BITS_WORD - 1;

#if defined(L4_64BIT)
    if (!(w >> (BITS_WORD-32))) {
        bit -= 32;
        w <<= 32;
    }
#endif

    if (!(w >> (BITS_WORD-16))) {
        bit -= 16;
        w <<= 16;
    }

    if (!(w >> (BITS_WORD-8))) {
        bit -= 8;
        w <<= 8;
    }

    if (!(w >> (BITS_WORD-4))) {
        bit -= 4;
        w <<= 4;
    }

    if (!(w >> (BITS_WORD-2))) {
        bit -= 2;
        w <<= 2;
    }

    if (!(w >> (BITS_WORD-1))) {
        bit -= 1;
    }

    return bit;
}

/* Seems to be fast on i386 cpus */
inline word_t msb_binary2(word_t w)
{
    word_t bit=0;

#if defined(L4_64BIT)
    if (w & 0xffffffff00000000ULL) {
        bit |= 32;
        w >>= 32;
    }
#endif

    if (w & 0xffff0000UL) {
        bit |= 16;
        w >>= 16;
    }

    if (w & 0xff00) {
        bit |= 8;
        w >>= 8;
    }

    if (w & 0xf0) {
        bit |= 4;
        w >>= 4;
    }

    if (w & 0xc) {
        bit |= 2;
        w >>= 2;
    }

    if (w & 0x2) {
        bit |= 1;
    }

    return bit;
}

inline word_t msb_nobranch(word_t w)
{
    word_t test;
    word_t bit = 0;

#if defined(L4_64BIT)
    test = ((w & (((1ULL << 32) - 1)<<32)) != 0) * 32; w >>= test; bit |= test;
#endif
    test = ((w & (((1UL << 16) - 1)<<16)) != 0 ? 1 : 0) * 16; w >>= test; bit |= test;
    test = ((w & (((1UL <<  8) - 1)<< 8)) != 0 ? 1 : 0) *  8; w >>= test; bit |= test;
    test = ((w & (((1UL <<  4) - 1)<< 4)) != 0 ? 1 : 0) *  4; w >>= test; bit |= test;
    test = ((w & (((1UL <<  2) - 1)<< 2)) != 0 ? 1 : 0) *  2; w >>= test; bit |= test;
    test = ((w & (((1UL <<  1) - 1)<< 1)) != 0 ? 1 : 0) *  1; w >>= test; bit |= test;

    return bit;
}


#endif /* !__GENERIC__LIB_H__ */
