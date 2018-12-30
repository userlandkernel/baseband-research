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
#ifndef _STDINT_H_
#define _STDINT_H_

#include <limits.h>

/* 7.18.1.1 Exact-width integers */
#include <arch/stdint.h>

/* 7.18.2.1 Limits of exact-wdith integer types */
#define INT8_MIN  SCHAR_MIN
#define INT16_MIN SHRT_MIN
#define INT32_MIN INT_MIN
#define INT64_MIN LLONG_MIN

#define INT8_MAX SCHAR_MAX
#define INT16_MAX SHRT_MAX
#define INT32_MAX INT_MAX
#define INT64_MAX LLONG_MAX

#define UINT8_MAX UCHAR_MAX
#define UINT16_MAX USHRT_MAX
#define UINT32_MAX UINT_MAX
#define UINT64_MAX ULLONG_MAX

#ifndef __ARCH_HAS_LEAST
/* 7.18.1.2 Minimum-width integers */
typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

/* 7.18.2.2 Limits of minimum-width integers */
#define INT_LEAST8_MIN          INT8_MIN
#define INT_LEAST16_MIN         INT16_MIN
#define INT_LEAST32_MIN         INT32_MIN
#define INT_LEAST64_MIN         INT64_MIN

#define INT_LEAST8_MAX          INT8_MAX
#define INT_LEAST16_MAX         INT16_MAX
#define INT_LEAST32_MAX         INT32_MAX
#define INT_LEAST64_MAX         INT64_MAX

#define UINT_LEAST8_MAX         UINT8_MAX
#define UINT_LEAST16_MAX        UINT16_MAX
#define UINT_LEAST32_MAX        UINT32_MAX
#define UINT_LEAST64_MAX        UINT64_MAX
#else
#undef __ARCH_HAS_LEAST
#endif

#ifndef __ARCH_HAS_FAST
/* 7.8.1.3 Fastest minimum-width integer types */
/*
 * Note -- We fulfil the spec, however we don't really know
 * which are fastest here. I assume `int' is probably fastest
 * more most, and should be used for [u]int_fast[8,16,32]_t.
 */

typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

/* 7.18.2.2 Limits of fastest minimum-width integers */
#define INT_FAST8_MIN   INT8_MIN
#define INT_FAST16_MIN  INT16_MIN
#define INT_FAST32_MIN  INT32_MIN
#define INT_FAST64_MIN  INT64_MIN

#define INT_FAST8_MAX   INT8_MAX
#define INT_FAST16_MAX  INT16_MAX
#define INT_FAST32_MAX  INT32_MAX
#define INT_FAST64_MAX  INT64_MAX

#define UINT_FAST8_MAX  UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX
#else
#undef __ARCH_HAS_FAST
#endif

/*
 * 7.18.1.4 Integer types capable of holding object pointers
 * We should fix this to be 32/64 clean.
 */
#if __PTR_SIZE==32
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX

#elif __PTR_SIZE==64
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;

#define INTPTR_MIN INT64_MIN
#define INTPTR_MAX INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#else
#error Unknown pointer size
#endif

#undef __PTR_SIZE

/* 7.18.1.5 Greatest-wdith integer types */
typedef long long int intmax_t;
typedef unsigned long long int uintmax_t;

/* 7.18.2.5 Limits of greateast-width integer types */
#define INTMAX_MIN              LLONG_MIN
#define INTMAX_MAX              LLONG_MAX
#define UINTMAX_MAX             ULLONG_MAX

/* 7.18.3 Limits of other integer types */
/*
 * FIXME: Check these limits are correct
 */
#define PTRDIFF_MIN             INTPTR_MIN
#define PTRDIFF_MAX             INTPTR_MAX

#define SIG_ATOMIC_MIN          INT_MIN
#define SIG_ATOMIC_MAX          INT_MAX

#define SIZE_MAX                UINTPTR_MAX

#define WCHAR_MIN               0
#define WCHAR_MAX               UINT16_MAX

#define WINT_MIN                0
#define WINT_MAX                UINT16_MAX

/*
 * 7.18.4 Macros for integer constants
 */

#define INT8_C(x)               (int8_t)(x)
#define INT16_C(x)              (int16_t)(x)
#define INT32_C(x)              (int32_t)(x)
#define INT64_C(x)              (int64_t)(x)
#define UINT8_C(x)              (uint8_t)(x)
#define UINT16_C(x)             (uint16_t)(x)
#define UINT32_C(x)             (uint32_t)(x)
#define UINT64_C(x)             (uint64_t)(x)

#define INT_FAST8_C(x)          (int_fast8_t)(x)
#define INT_FAST16_C(x)         (int_fast16_t)(x)
#define INT_FAST32_C(x)         (int_fast32_t)(x)
#define INT_FAST64_C(x)         (int_fast64_t)(x)
#define UINT_FAST8_C(x)         (uint_fast8_t)(x)
#define UINT_FAST16_C(x)        (uint_fast16_t)(x)
#define UINT_FAST32_C(x)        (uint_fast32_t)(x)
#define UINT_FAST64_C(x)        (uint_fast64_t)(x)

#define INT_LEAST8_C(x)         (int_least8_t)(x)
#define INT_LEAST16_C(x)        (int_least16_t)(x)
#define INT_LEAST32_C(x)        (int_least32_t)(x)
#define INT_LEAST64_C(x)        (int_least64_t)(x)
#define UINT_LEAST8_C(x)        (uint_least8_t)(x)
#define UINT_LEAST16_C(x)       (uint_least16_t)(x)
#define UINT_LEAST32_C(x)       (uint_least32_t)(x)
#define UINT_LEAST64_C(x)       (uint_least64_t)(x)

#define INTPTR_C(x)             (intptr_t)(x)
#define UINTPTR_C(x)            (uintptr_t)(x)

#define INTMAX_C(x)             (intmax_t)(x)
#define UINTMAX_C(x)            (uintmax_t)(x)

#endif /* _STDINT_H_ */
