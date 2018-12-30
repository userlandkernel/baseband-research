/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/tracebuffer.h
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
#ifndef __ARCH__AMD64__TRACEBUFFER_H__
#define __ARCH__AMD64__TRACEBUFFER_H__

#include <sync.h>

typedef struct {
    u32_t cycles;
    u32_t pmc0;
    u32_t pmc1;
    u32_t pad;	
} tracestatus_t;  
                 
typedef struct {
    tracestatus_t status;	// 16 
    u64_t identifier;		//  8
    u64_t data[5];		// 40
} trace_t;  

typedef struct {
    u64_t current;
    u64_t magic;
    u64_t counter;
    u64_t threshold;
    u64_t pad[4];
    trace_t trace[];
} tracebuffer_t;  

extern bool tracebuffer_initialized;
extern tracebuffer_t *tracebuffer;		
/*
 * jsXXX: a  lock for the buffer is a pretty silly idea, 
 *	  atomic reservations would be way better
 */ 
extern spinlock_t tracebuffer_lock;

#define TBUF_MAGIC 0x0511197820121977

#if defined(CONFIG_PERFMON)
#define RDPMC_0 "rdpmc                          \n" \
                "movl   %%eax, %%fs:4(%%rdi)    \n"

#define RDPMC_1 "rdpmc                          \n"  \
                "movl	%%eax, %%fs:8(%%rdi)    \n"

#if defined(CONFIG_CPU_AMD64_K8)
#define PMC_SEL_0 "xorl  %%ecx, %%ecx \n"
#define PMC_SEL_1 "inc %%ecx \n"
#elif defined(CONFIG_CPU_AMD64_P4)
/* PMC_MSR_IQ_COUNTER 0 and 2 */
#define PMC_SEL_0 "movl  $12, %%ecx \n"
#define PMC_SEL_1 "movl  $14, %%ecx \n"
#endif

#else /* defined(CONFIG_PERFMON) */
#define PMC_SEL_0
#define PMC_SEL_1
#define RDPMC_0
#define RDPMC_1
#endif 


#define TBUF_RETURN_IF_UNITIALIZED()		\
    if (!tracebuffer_initialized)		\
	    return;

#define TBUF_GET_NEXT_RECORD(addr)				\
    do {							\
	tracebuffer_lock.lock();				\
	asm volatile (						\
	    "movq	%%fs:0, %%rdi	        \n"		\
	    "movq	$0x40, %%rax		\n"		\
	    "addq	%%rax, %%rdi		\n"		\
	    "andq	$0x1fffff, %%rdi	\n"		\
	    "cmovzq	%%rax, %%rdi		\n"		\
	    "movq	%%rdi, %%fs:0		\n"		\
	    "rdtsc				\n"		\
	    "movl 	%%eax, %%fs:0(%%rdi)    \n"		\
	    PMC_SEL_0						\
	    RDPMC_0						\
	    PMC_SEL_1						\
	    RDPMC_1						\
	    : "=D" (addr)					\
	    :							\
	    : "rax", "rcx", "rdx"				\
	    );							\
	tracebuffer_lock.unlock();				\
    } while (0)			

#define TBUF_STORE_ITEM(addr, offset, item)     \
    asm volatile (				      \
        "movq  %0, %%fs:(" #offset "*8+16)(%%rdi)\n"  \
        : : "r" (item), "D" (addr)              \
    );

extern inline void TBUF_RECORD_EVENT(word_t _id)

{
    TBUF_RETURN_IF_UNITIALIZED();
    unsigned long addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
}

extern inline void TBUF_RECORD_EVENT(char *str)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (word_t)str);
}

extern inline void TBUF_RECORD_EVENT(char *str, word_t _p0)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (word_t)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
}

extern inline void TBUF_RECORD_EVENT(char *str, word_t _p0, word_t _p1)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (word_t)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
}

extern inline void TBUF_RECORD_EVENT(char *str, word_t _p0, word_t _p1, word_t _p2)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (word_t)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
}

extern inline void TBUF_RECORD_EVENT(char *str, word_t _p0, word_t _p1, word_t _p2, word_t _p3)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (word_t long)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
    TBUF_STORE_ITEM(addr, 4, _p3);
}

extern inline void TBUF_RECORD_EVENT(word_t _id, word_t _p0)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
}

extern inline void TBUF_RECORD_EVENT(word_t _id, word_t _p0, word_t _p1)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
}

extern inline void TBUF_RECORD_EVENT(word_t _id, word_t _p0, word_t _p1, word_t _p2)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
}

extern inline void TBUF_RECORD_EVENT(word_t _id, word_t _p0, word_t _p1, word_t _p2, word_t _p3)

{
    TBUF_RETURN_IF_UNITIALIZED();
    word_t addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
    TBUF_STORE_ITEM(addr, 4, _p3);
}

#define TBUF_INCREMENT_TRACE_COUNTER() \
	asm volatile ( \
			"movl	%%fs:8, %%edx	        \n" \
			"movl	%%fs:12, %%edi	        \n" \
			"inc	%%edx			\n" \
			"movl	%%edx, %%fs:8   	\n" \
			"cmpl	%%edx, %%edi		\n" \
			"jge	1f			\n" \
			"int3				\n" \
			"xor	%%edx, %%edx		\n" \
			"movl	%%edx, %%fs:8   	\n" \
			"1: 				\n" \
			:				    \
			: 				    \
			: "edx", "edi"		    	    \
		     );	

#define TBUF_CLEAR_BUFFER() \
	asm volatile ( \
			"movl	$0, %fs:0		\n" \
		     );	


#endif /* !__ARCH__AMD64__TRACEBUFFER_H__ */
