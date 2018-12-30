/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     l4/ia32/tracebuffer.h
 * Description:   Functions for accessing the tracebuffer
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
 *
 ********************************************************************/
#ifndef __L4__IA32__TRACEBUFFER_H__
#define __L4__IA32__TRACEBUFFER_H__

#if defined(TRACEBUFFER)
#include <l4/types.h>

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

extern L4_Tracebuffer_t * L4_Tracebuffer;

#define L4_TBUF_MAGIC 0x20121977

#if defined(PERFMON)
#define __L4_RDPMC "rdpmc\n"
#else
#define __L4_RDPMC "xor %%eax, %%eax\n"
#endif

#define __L4_TBUF_GET_NEXT_RECORD(addr)			\
   asm volatile (					\
       "	movl	%%fs:0, %%esi		\n"	\
       "	addl	$32, %%esi		\n"	\
       "	movl   %%esi, %%edi		\n"	\
       "	andl   $0x1fffff, %%esi		\n"	\
       "	movl	%%esi, %%fs:0		\n"	\
       "	rdtsc				\n"	\
       "	movl	%%eax, %%fs:0(%%edi)	\n"	\
       "	xor	%%ecx, %%ecx		\n"	\
       __L4_RDPMC					\
       "	movl   %%eax, %%fs:4(%%edi)	\n"	\
       "	inc	%%ecx			\n"	\
       __L4_RDPMC					\
       "	movl	%%eax, %%fs:8(%%edi)	\n"	\
       :"=D" (addr)					\
       :						\
       :"eax", "ecx", "edx", "esi")

#define __L4_TBUF_STORE_ITEM(addr, offset, item)		\
    asm volatile ("movl %0, %%fs:(" #offset "*4+12)(%%edi)"	\
		  :						\
		  :"r" (item), "D" (addr))

L4_INLINE void L4_TBUF_RECORD_EVENT (unsigned _id)
{
    unsigned addr;
    __L4_TBUF_GET_NEXT_RECORD (addr);
    __L4_TBUF_STORE_ITEM (addr, 0, _id & 0x7FFFFFFF);
}

L4_INLINE void L4_TBUF_RECORD_EVENT_1 (unsigned _id,
				       unsigned _p0)
{
    unsigned addr;
    __L4_TBUF_GET_NEXT_RECORD (addr);
    __L4_TBUF_STORE_ITEM (addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM (addr, 1, _p0);
}

L4_INLINE void L4_TBUF_RECORD_EVENT_2 (unsigned _id,
				       unsigned _p0,
				       unsigned _p1)
{
    unsigned addr;
    __L4_TBUF_GET_NEXT_RECORD (addr);
    __L4_TBUF_STORE_ITEM (addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM (addr, 1, _p0);
    __L4_TBUF_STORE_ITEM (addr, 2, _p1);
}

L4_INLINE void L4_TBUF_RECORD_EVENT_3 (unsigned _id,
				       unsigned _p0,
				       unsigned _p1,
				       unsigned _p2)
{
    unsigned addr;
    __L4_TBUF_GET_NEXT_RECORD (addr);
    __L4_TBUF_STORE_ITEM (addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM (addr, 1, _p0);
    __L4_TBUF_STORE_ITEM (addr, 2, _p1);
    __L4_TBUF_STORE_ITEM (addr, 3, _p2);
}

L4_INLINE void L4_TBUF_RECORD_EVENT_4 (unsigned _id,
				       unsigned _p0,
				       unsigned _p1,
				       unsigned _p2,
				       unsigned _p3)
{
    unsigned addr;
    __L4_TBUF_GET_NEXT_RECORD (addr);
    __L4_TBUF_STORE_ITEM (addr, 0, _id & 0x7FFFFFFF);
    __L4_TBUF_STORE_ITEM (addr, 1, _p0);
    __L4_TBUF_STORE_ITEM (addr, 2, _p1);
    __L4_TBUF_STORE_ITEM (addr, 3, _p2);
    __L4_TBUF_STORE_ITEM (addr, 4, _p3);
}

#if defined(__cplusplus)
L4_INLINE void L4_TBUF_RECORD_EVENT (unsigned _id, unsigned _p0)
{
    L4_TBUF_RECORD_EVENT_1 (_id, _p0);
}

L4_INLINE void L4_TBUF_RECORD_EVENT (unsigned _id,
				     unsigned _p0,
				     unsigned _p1)
{
    L4_TBUF_RECORD_EVENT_2 (_id, _p0, _p1);
}

L4_INLINE void L4_TBUF_RECORD_EVENT (unsigned _id,
				     unsigned _p0,
				     unsigned _p1,
				     unsigned _p2)
{
    L4_TBUF_RECORD_EVENT_3 (_id, _p0, _p1, _p2);
}

L4_INLINE void L4_TBUF_RECORD_EVENT (unsigned _id,
				     unsigned _p0,
				     unsigned _p1,
				     unsigned _p2,
				     unsigned _p3)
{
    L4_TBUF_RECORD_EVENT_4 (_id, _p0, _p1, _p2, _p3);
}
#endif /* __cplusplus */

#define L4_TBUF_INCREMENT_TRACE_COUNTER()			\
    asm volatile (						\
	"	movl	%%fs:8, %%edx		\n"		\
	"	movl	%%fs:12, %%edi		\n"		\
	"	inc	%%edx			\n"		\
	"	movl	%%edx, %%fs:8		\n"		\
	"	cmpl	%%edx, %%edi		\n"		\
	"	jge	1f			\n"		\
	"	int3				\n"		\
	"	xor	%%edx, %%edx		\n"		\
	"	movl	%%edx, %%fs:8		\n"		\
	"1:					\n"		\
	:							\
	:							\
	:"edx", "edi")

#define L4_TBUF_CLEAR_BUFFER() \
    asm volatile ("movl $0, %fs:0")


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
