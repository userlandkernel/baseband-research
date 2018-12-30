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
 * Description:   Functions for accessing the tracebuffer
 */
#ifndef __L4__IA32__TRACEBUFFER_H__
#define __L4__IA32__TRACEBUFFER_H__

#if defined(TRACEBUFFER)
#include <l4/types.h>

/*
 * Note: this has to correspond to the value specified in the
 * kernel arch/config.h. -gl
 */
#define L4_TBS  ((8 << 3)/*idx8*/ | (0 << 2)/*!local*/ | 3/*rpl3*/)

typedef struct {
    L4_Word32_t cycles;
    L4_Word32_t pmc0;
    L4_Word32_t pmc1;
} L4_Tracestatus_t;

typedef struct {
    L4_Tracestatus_t status;
    L4_Word32_t identifier;
    L4_Word32_t data[4];
} L4_Trace_t;

typedef struct {
    L4_Word32_t current;
    L4_Word32_t magic;
    L4_Word32_t counter;
    L4_Word32_t threshold;
    L4_Word32_t pad[4];
    L4_Trace_t trace[];
} L4_Tracebuffer_t;

extern L4_Tracebuffer_t *L4_Tracebuffer;

#define L4_TBUF_MAGIC 0x20121977

#if defined(PERFMON)
#define __L4_RDPMC "rdpmc\n"
#else
#define __L4_RDPMC "xor %%eax, %%eax\n"
#endif

#define __L4_TBUF_GET_NEXT_RECORD(addr)                 \
   asm volatile (                                       \
       "        push    %%fs                    \n"     \
       "        movw    %[tbs], %%ax            \n"     \
       "        movw    %%ax, %%fs              \n"     \
       "        movl    %%fs:0, %%esi           \n"     \
       "        addl    $32, %%esi              \n"     \
       "        movl   %%esi, %%edi             \n"     \
       "        andl   $0x1fffff, %%esi         \n"     \
       "        movl    %%esi, %%fs:0           \n"     \
       "        rdtsc                           \n"     \
       "        movl    %%eax, %%fs:0(%%edi)    \n"     \
       "        xor     %%ecx, %%ecx            \n"     \
       __L4_RDPMC                                       \
       "        movl   %%eax, %%fs:4(%%edi)     \n"     \
       "        inc     %%ecx                   \n"     \
       __L4_RDPMC                                       \
       "        movl    %%eax, %%fs:8(%%edi)    \n"     \
       "        pop     %%fs                    \n"     \
       :"=D" (addr)                                     \
       : "i"(L4_TBS)                                    \
       :"eax", "ecx", "edx", "esi")

#define __L4_TBUF_STORE_ITEM(addr, offset, item)        \
    asm volatile (                                      \
       "push %%fs                               \n"     \
       "movw %2, %%ax                           \n"     \
       "movw %%ax, %%fs                         \n"     \
       "movl %0, %%fs:(" #offset "*4+12)(%%edi) \n"     \
       "pop  %%fs                               \n"     \
       :                                                \
       : "r" (item), "D" (addr), "i" (L4_TBS)           \
       : "eax" )

L4_INLINE void
L4_TBUF_RECORD_EVENT(unsigned _id)
{
    unsigned addr;

    __L4_TBUF_GET_NEXT_RECORD(addr);
    __L4_TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT_1(unsigned _id, unsigned _p0)
{
    unsigned addr;

    __L4_TBUF_GET_NEXT_RECORD(addr);
    __L4_TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM(addr, 1, _p0);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT_2(unsigned _id, unsigned _p0, unsigned _p1)
{
    unsigned addr;

    __L4_TBUF_GET_NEXT_RECORD(addr);
    __L4_TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM(addr, 1, _p0);
    __L4_TBUF_STORE_ITEM(addr, 2, _p1);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT_3(unsigned _id, unsigned _p0, unsigned _p1, unsigned _p2)
{
    unsigned addr;

    __L4_TBUF_GET_NEXT_RECORD(addr);
    __L4_TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM(addr, 1, _p0);
    __L4_TBUF_STORE_ITEM(addr, 2, _p1);
    __L4_TBUF_STORE_ITEM(addr, 3, _p2);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT_4(unsigned _id,
                       unsigned _p0, unsigned _p1, unsigned _p2, unsigned _p3)
{
    unsigned addr;

    __L4_TBUF_GET_NEXT_RECORD(addr);
    __L4_TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM(addr, 1, _p0);
    __L4_TBUF_STORE_ITEM(addr, 2, _p1);
    __L4_TBUF_STORE_ITEM(addr, 3, _p2);
    __L4_TBUF_STORE_ITEM(addr, 4, _p3);
}

#if defined(__l4_cplusplus)
L4_INLINE void
L4_TBUF_RECORD_EVENT(unsigned _id, unsigned _p0)
{
    L4_TBUF_RECORD_EVENT_1(_id, _p0);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT(unsigned _id, unsigned _p0, unsigned _p1)
{
    L4_TBUF_RECORD_EVENT_2(_id, _p0, _p1);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT(unsigned _id, unsigned _p0, unsigned _p1, unsigned _p2)
{
    L4_TBUF_RECORD_EVENT_3(_id, _p0, _p1, _p2);
}

L4_INLINE void
L4_TBUF_RECORD_EVENT(unsigned _id,
                     unsigned _p0, unsigned _p1, unsigned _p2, unsigned _p3)
{
    L4_TBUF_RECORD_EVENT_4(_id, _p0, _p1, _p2, _p3);
}
#endif /* __l4_cplusplus */

#define L4_TBUF_INCREMENT_TRACE_COUNTER()                       \
    asm volatile (                                              \
        "       push    %%fs                    \n"             \
       "        movw    %[tbs], %%dx            \n"             \
       "        movw    %%dx, %%fs              \n"             \
        "       movl    %%fs:8, %%edx           \n"             \
        "       movl    %%fs:12, %%edi          \n"             \
        "       inc     %%edx                   \n"             \
        "       movl    %%edx, %%fs:8           \n"             \
        "       cmpl    %%edx, %%edi            \n"             \
        "       jge     1f                      \n"             \
        "       int3                            \n"             \
        "       xor     %%edx, %%edx            \n"             \
        "       movl    %%edx, %%fs:8           \n"             \
        "1:                                     \n"             \
        "       pop     %%fs                    \n"             \
        :                                                       \
        : "i" (L4_TBS)                                          \
        :"edx", "edi")

#define L4_TBUF_CLEAR_BUFFER()                                  \
    asm volatile (                                              \
        "push %%fs                              \n"             \
        "mov  %[tbs], %%ax                      \n"             \
        "mov  %%ax, %%fs                        \n"             \
        "movl $0, %%fs:0"                       \n"             \
        "pop  %%fs                              \n"             \
        :                                                       \
        : "i" (L4_TBS)                                          \
        );

#else /* !TRACEBUFFER */

#define L4_TBUF_RECORD_EVENT(args...)
#define L4_TBUF_RECORD_EVENT_1(args...)
#define L4_TBUF_RECORD_EVENT_2(args...)
#define L4_TBUF_RECORD_EVENT_3(args...)
#define L4_TBUF_RECORD_EVENT_4(args...)
#define L4_TBUF_INCREMENT_TRACE_COUNTER()
#define L4_TBUF_CLEAR_BUFFER()

#endif /* !TRACEBUFFER */

#endif /* !__L4__IA32__TRACEBUFFER_H__ */
