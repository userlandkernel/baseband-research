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
 * Description:   Omnipresent macro definitions.
 */

#ifndef __L4__MACROS_H__
#define __L4__MACROS_H__

// FIXME: Unify CONFIG_BIGENDIAN/L4_BIG_ENDIAN - AGW.
#if (defined(CONFIG_BIGENDIAN) || defined(L4_BIG_ENDIAN))
#define BITFIELD2(t,a,b)                t b; t a
#define BITFIELD3(t,a,b,c)              t c; t b; t a
#define BITFIELD4(t,a,b,c,d)            t d; t c; t b; t a
#define BITFIELD5(t,a,b,c,d,e)          t e; t d; t c; t b; t a
#define BITFIELD6(t,a,b,c,d,e,f)        t f; t e; t d; t c; t b; t a
#define BITFIELD7(t,a,b,c,d,e,f,g)      t g; t f; t e; t d; t c; t b; t a
#define BITFIELD8(t,a,b,c,d,e,f,g,h)    t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD9(t,a,b,c,d,e,f,g,h,i)  t i; t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD10(t,a,b,c,d,e,f,g,h,i,j) t j; t i; t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD11(t,a,b,c,d,e,f,g,h,i,j,k) t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD12(t,a,b,c,d,e,f,g,h,i,j,k,l) t l; t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD13(t,a,b,c,d,e,f,g,h,i,j,k,l,m) t m; t l; t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a
#define BITFIELD14(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n) t n; t m; t l; t k; t j; t i; t h; t g; t f; t e; t d; t c; t b; t a

#define BITFIELD17(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) t q; t p; t o; t n; t m; t l; t k; t j;t i; t h; t g; t f; t e; t d; t c; t b; t a

#define SHUFFLE2(a,b)                   b,a
#define SHUFFLE3(a,b,c)                 c,b,a
#define SHUFFLE4(a,b,c,d)               d,c,b,a
#define SHUFFLE5(a,b,c,d,e)             e,d,c,b,a
#define SHUFFLE6(a,b,c,d,e,f)           f,e,d,c,b,a
#define SHUFFLE7(a,b,c,d,e,f,g)         g,f,e,d,c,b,a
#elif (defined(CONFIG_LITTLEENDIAN) || defined(L4_LITTLE_ENDIAN))
#define BITFIELD2(t,a,b)                t a; t b
#define BITFIELD3(t,a,b,c)              t a; t b; t c
#define BITFIELD4(t,a,b,c,d)            t a; t b; t c; t d
#define BITFIELD5(t,a,b,c,d,e)          t a; t b; t c; t d; t e
#define BITFIELD6(t,a,b,c,d,e,f)        t a; t b; t c; t d; t e; t f
#define BITFIELD7(t,a,b,c,d,e,f,g)      t a; t b; t c; t d; t e; t f; t g
#define BITFIELD8(t,a,b,c,d,e,f,g,h)    t a; t b; t c; t d; t e; t f; t g; t h
#define BITFIELD9(t,a,b,c,d,e,f,g,h,i)  t a; t b; t c; t d; t e; t f; t g; t h; t i
#define BITFIELD10(t,a,b,c,d,e,f,g,h,i,j) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j
#define BITFIELD11(t,a,b,c,d,e,f,g,h,i,j,k) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k
#define BITFIELD12(t,a,b,c,d,e,f,g,h,i,j,k,l) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l
#define BITFIELD13(t,a,b,c,d,e,f,g,h,i,j,k,l,m) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l; t m
#define BITFIELD14(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l; t m; t n
#define BITFIELD17(t,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) t a; t b; t c; t d; t e; t f; t g; t h; t i; t j; t k; t l; t m; t n; t o; t p; t q
#define SHUFFLE2(a,b)                   a,b
#define SHUFFLE3(a,b,c)                 a,b,c
#define SHUFFLE4(a,b,c,d)               a,b,c,d
#define SHUFFLE5(a,b,c,d,e)             a,b,c,d,e
#define SHUFFLE6(a,b,c,d,e,f)           a,b,c,d,e,f
#define SHUFFLE7(a,b,c,d,e,f,g)         a,b,c,d,e,f,g
#else
#error "Endianess not defined!"
#endif


#endif /* !__L4__MACROS_H__ */
