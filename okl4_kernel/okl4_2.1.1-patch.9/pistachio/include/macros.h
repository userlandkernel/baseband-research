/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   Omnipresent macro definitions
 */

#ifndef __MACROS_H__
#define __MACROS_H__

#include <compat/c.h>
#include <l4/macros.h>

/* use this to place code/data in a certain section */
#if defined(__APPLE_CC__)
#define UNIT(x)                 SECTION("__DATA, " x)
#define CALIGN_16 ".align 4"
#else
#define UNIT(x)                 SECTION(".data." x)
#define CALIGN_16 ".align 16"
#endif

/*
  There is a gcc extension that allows you to provid asm names for
  declared methods. E.g:
  class foo {
     void bar(void) __asm__("bar");
  }
  Instead of directly using this we provide the ASM_NAME macro so it becomes:
  class foo {
     void bar(void) ASM_NAME("bar");
  }
  This is much more readable, and allows us to easily ignore it when using flint.
*/
#ifdef _lint
#define ASM_NAME(x)
#else
#define ASM_NAME(x) __asm__(x)
#endif

#ifdef _lint
#define ASM_REG(x)
#else
#define ASM_REG(x) asm(x)
#endif

/*
 * Produce a link-time error of the given string. If the LINK_ERROR()
 * macro is not compiled away by the dead-code optimiser, a link-time
 * error will be produced.
 *
 * The string must be a valid function name (and can not contain spaces,
 * for instance.)
 */
#if defined(__GNUC__)
#define LINK_ERROR(x)                              \
        do {                                       \
            extern NORETURN void __LINK_ERROR__##x(void);   \
            __LINK_ERROR__##x();                   \
        } while (0)
#else
#define LINK_ERROR(x)
#endif

/*
 * Ensure that the given token is a compile-time constant, or
 * generate a link-time error.
 */
#if defined(__GNUC__)
#define ENSURE_CONSTANT(x, msg)                    \
        do {                                       \
            if (!__constant_p(x)) {                \
                LINK_ERROR(msg);                   \
            }                                      \
        } while (0)
#else
#define ENSURE_CONSTANT(x, msg)
#endif

/*
 * Mark code as unreachable, and remove it for fussy compilers.
 */
#if defined(__RVCT_GNU__)
#define UNREACHABLE(x)
#else
#define UNREACHABLE(x)          x
#endif

/*
 * Mark code as unreachable, and produce a link-time error if it is
 * actually reached.
 */
#if defined(__GNUC__)
/*
 * The compiler will optimise away NOTREACHED() calls if they are in
 * fact not reachable. If they can be reached, the compiler will
 * generate a link-time error with the non-existant symbol
 * "__unreachable_code_reached".
 */
#define NOTREACHED()                                      \
        do {                                              \
            extern void __unreachable_code_reached(void); \
            __unreachable_code_reached();                 \
            for (;;);                                     \
        } while(0)
#else
#define NOTREACHED() \
        panic("Unreachable code reached.\n")
#endif

#undef INLINE
#define INLINE inline

/* Functions for critical path optimizations */
#define EXPECT_FALSE(x)         __unlikely(x)
#define EXPECT_TRUE(x)          __likely(x)
#define EXPECT_VALUE(x,val)     __expect((x), (val))

/* Convenience functions for memory sizes. */
#define KB(x)   (x * 1024)
#define MB(x)   (x * 1024*1024)
#define GB(x)   (x * 1024*1024*1024)

/* other convenience functions */
#if !defined(ASSEMBLY)
template<typename T>
inline const T&
max(const T& a, const T& b)
{
        if (a > b)
                return a;
        return b;
}

template<typename T>
inline const T&
min(const T& a, const T& b)
{
        if (a < b)
                return a;
        return b;
}
#endif

/* Offset for struct fields. */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define offsetof(type, field)   __builtin_offsetof(type, field)
#else
#define offsetof(type, field)   ((word_t) (&((type *) 0)->field))
#endif

/* Turn preprocessor symbol definition into string */
#define MKSTR(sym)      MKSTR2(sym)
#define MKSTR2(sym)     #sym

/* Safely "append" an UL suffix for also asm values */
#if defined(ASSEMBLY)
#define __UL(x)         x
#else
#define __UL(x)         x##UL
#endif

#define TWOSCOMP(X) ((~(X))+1)

#endif /* !__MACROS_H__ */
