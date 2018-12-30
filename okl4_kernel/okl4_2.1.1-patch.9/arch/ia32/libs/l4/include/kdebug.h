/*
 * Copyright (c) 2002, 2003, Karlsruhe University
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
 * Description:   L4 Kdebug interface for ia32
 */
#ifndef __L4__X86__KDEBUG_H__
#define __L4__X86__KDEBUG_H__

#define L4_KDB_Enter(str...)                    \
do {                                            \
    __asm__ __volatile__(                       \
        "/* L4_KDB_Enter() */           \n"     \
        "       int     $3              \n"     \
        "       jmp     2f              \n"     \
        "       mov     $1f, %eax       \n"     \
        "1:     .ascii \"" str "\"      \n"     \
        "       .byte 0                 \n"     \
        "2:                             \n");   \
} while (0)

#define __L4_KDB_Op(op, name)                   \
INLINE okl4_kdb_res_t                           \
L4_KDB_##name(void)                             \
{                                               \
    __asm__ __volatile__(                       \
        "/* L4_KDB_"#name"() */         \n"     \
        "       int     $3              \n"     \
        "       cmpb    %0, %%al        \n"     \
        :                                       \
        : "i" (op));                            \
    return OKL4_KDB_RES_OK;                     \
}

#define __L4_KDB_Op_Arg(op, name, argtype)      \
INLINE okl4_kdb_res_t                           \
L4_KDB_##name(argtype arg)                      \
{                                               \
    __asm__ __volatile__ (                      \
        "/* L4_KDB_"#name"() */         \n"     \
        "       int     $3              \n"     \
        "       cmpb    %1, %%al        \n"     \
        :                                       \
        : "a" (arg), "i" (op));                 \
    return OKL4_KDB_RES_OK;                     \
}

#define __L4_KDB_Op_Ret(op, name, rettype)      \
INLINE okl4_kdb_res_t                           \
L4_KDB_##name(rettype * ret)                    \
{                                               \
    __asm__ __volatile__ (                      \
        "/* L4_KDB_"#name"() */         \n"     \
        "       int     $3              \n"     \
        "       cmpb    %1, %%al        \n"     \
        : "=a" (*ret)                           \
        : "i" (op));                            \
    return OKL4_KDB_RES_OK;                     \
}

INLINE okl4_kdb_res_t
L4_KDB_SetObjectName(word_t type, word_t id, const char * name)
{
    const word_t * name_p = (const word_t *)name;
    register word_t first asm("eax") = name_p[0];
    register word_t second asm("esi") = name_p[1];
    register word_t third asm("ecx") = name_p[2];
    register word_t fourth asm("edx") = name_p[3];
    register word_t input_id asm("edi") = id;
    register word_t input_type asm("ebx") = type;

    __asm__ __volatile__(
            "          int     $3          \n"
            "          cmpb    %0, %%al    \n"
            : /* no outputs */
            : "i"(0x0f), "r"(input_id), "r"(input_type),
              "r"(first), "r"(second), "r"(third), "r"(fourth)
            : "flags");

    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_TracepointCountStop(void)
{
    return OKL4_KDB_RES_UNIMPLEMENTED;
}

INLINE okl4_kdb_res_t
L4_KDB_TracepointCountReset(void)
{
    return OKL4_KDB_RES_UNIMPLEMENTED;
}

INLINE okl4_kdb_res_t
L4_KDB_GetTracepointName(word_t tp, char * dest)
{
    return OKL4_KDB_RES_UNIMPLEMENTED;
}

__L4_KDB_Op_Ret(0x08, ReadChar, char)
__L4_KDB_Op_Ret(0x0d, ReadChar_Blocked, char)
__L4_KDB_Op_Arg(0x00, PrintChar, char)
__L4_KDB_Op_Arg(0x01, PrintString, char *)
__L4_KDB_Op(0x02, ClearPage)


#endif /* !__L4__X86__KDEBUG_H__ */
