/*
 * Copyright (c) 2002-2003, University of New South Wales
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
 * Description: L4 Kdebug interface for Mips.
 */

#ifndef __L4__MIPS__KDEBUG_H__
#define __L4__MIPS__KDEBUG_H__

#define __L4_TRAP_KPUTC              (-100ul)
#define __L4_TRAP_KGETC              (-101ul)
#define __L4_TRAP_KDEBUG             (-102ul)
#define __L4_TRAP_KGETC_NB           (-104ul)
#define __L4_TRAP_READ_PERF          (-110ul)
#define __L4_TRAP_WRITE_PERF         (-111ul)
#define __L4_TRAP_KSET_OBJECT_NAME   (-120ul)

#include <compat/c.h>

#if defined(L4_32BIT)
#define ADR     "la"
#elif defined(L4_64BIT)
#define ADR     "dla"
#endif

/**
 * KDB Debugging calls are completed by performing a 'break' instruction with
 * $1 (at) set to the KDB call number, and $2 (v0) set to a magic value,
 * 0xdb0ca110 (DB call).
 */
#if defined(L4_32BIT)
 #define __L4_MAGIC_KDB_REQUEST      (0xDb0Ca110UL)
#elif defined(L4_64BIT)
 #define __L4_MAGIC_KDB_REQUEST      (0xDb0Ca110Db0Ca110UL)
#endif

#define L4_KDB_Enter(str...)                                    \
do {                                                            \
    __asm__ __volatile__ (                                      \
        ".set   noat                \n\t"                       \
        "li     $1, %0              \n\t"                       \
        "move   $2, %1              \n\t"                       \
        ADR"    $3, 1f              \n\t"                       \
        "break                      \n\t"                       \
        ".set   at                  \n\t"                       \
        "       .data               \n\t"                       \
        "1: .string     " #str ";   \n\t"                       \
        "       .previous           \n\t"                       \
        : /* no outputs */                                      \
        : "i" (__L4_TRAP_KDEBUG), "r" (__L4_MAGIC_KDB_REQUEST)  \
        : "memory", "$1", "$2", "$3"                            \
    );                                                          \
} while (0)

#define __L4_KDB_Op_Arg(op, name, argtype)                      \
L4_INLINE void L4_KDB_##name (argtype arg)                      \
{                                                               \
    register char r_c asm("$4") = arg;                          \
    __asm__ __volatile__ (                                      \
        ".set noat                  \n\t"                       \
        "li     $1, %0              \n\t"                       \
        "move   $2, %1              \n\t"                       \
        "break                      \n\t"                       \
        ".set at                    \n\t"                       \
        : /* no outputs */                                      \
        : "i" (op), "r" (__L4_MAGIC_KDB_REQUEST), "r" (r_c)     \
        : "$1", "$2"                                            \
    );                                                          \
}


#define __L4_KDB_Op_Ret(op, name, rettype)                      \
L4_INLINE rettype L4_KDB_##name (void)                          \
{                                                               \
    register unsigned int ret asm("$2");                        \
    __asm__ __volatile__ (                                      \
        ".set noat                  \n\t"                       \
        "li     $1, %1              \n\t"                       \
        "move   $2, %2              \n\t"                       \
        "break                      \n\t"                       \
        ".set at                    \n\t"                       \
        : "=r" (ret)                                            \
        : "i" (op), "r" (__L4_MAGIC_KDB_REQUEST)                \
        : "$1"                                                  \
    );                                                          \
    return (rettype)ret;                                        \
}

__L4_KDB_Op_Ret( __L4_TRAP_KGETC, ReadChar_Blocked, char )
__L4_KDB_Op_Ret( __L4_TRAP_KGETC_NB, ReadChar, long )
__L4_KDB_Op_Arg( __L4_TRAP_KPUTC, PrintChar, char )

L4_INLINE void
L4_KDB_SetObjectName(L4_Word_t type, L4_Word_t id, const char *name)
{
    register L4_Word_t r_type   asm("$4") = type;
    register L4_Word_t r_id     asm("$5") = id;
    register L4_Word_t w1       asm("$6") = 0;
    register L4_Word_t w2       asm("$7") = 0;
#if defined(L4_32BIT)
    register L4_Word_t w3       asm("$8") = 0;
    register L4_Word_t w4       asm("$9") = 0;
#endif
    L4_Word_t i;

    union {
        char buffer[L4_KDB_MAX_DEBUG_NAME_LENGTH];
        L4_Word_t words[L4_KDB_MAX_DEBUG_NAME_LENGTH / sizeof(L4_Word_t)];
    } aligned;

    /* Copy the name to our buffer. */
    for (i = 0; i < L4_KDB_MAX_DEBUG_NAME_LENGTH; i++) {
        aligned.buffer[i] = name[i];
        if (name[i] == '\0') {
            break;
        }
    }

    /* Copy to registers. */
    w1 = aligned.words[0];
    w2 = aligned.words[1];
#if defined(L4_32BIT)
    w3 = aligned.words[2];
    w4 = aligned.words[3];
#endif

    __asm__ __volatile__ (
            ".set noat  \n\t"
            "li $1, %0  \n\t"
            "move $2, %1 \n\t"
            "break      \n\t"
            ".set at    \n\t"
            : /* no outputs */
            : "i" (__L4_TRAP_KSET_OBJECT_NAME),
              "r" (__L4_MAGIC_KDB_REQUEST),
              "r" (r_type), "r" (r_id), "r" (w1), "r" (w2)
#if defined(L4_32BIT)
            , "r" (w3), "r" (w4)
#endif
            : "$1", "$2", "memory" );
}

L4_INLINE void
L4_KDB_SetThreadName(L4_ThreadId_t tid, const char *name)
{
    L4_KDB_SetObjectName(L4_KDB_OBJECT_THREAD, tid.raw, name);
}

L4_INLINE void
L4_KDB_SetSpaceName(L4_SpaceId_t sid, const char *name)
{
    L4_KDB_SetObjectName(L4_KDB_OBJECT_SPACE, sid.raw, name);
}

L4_INLINE void
L4_KDB_SetMutexName(L4_MutexId_t mutexid, const char *name)
{
    L4_KDB_SetObjectName(L4_KDB_OBJECT_MUTEX, mutexid.raw, name);
}

#endif /* !__L4__MIPS__KDEBUG_H__ */
