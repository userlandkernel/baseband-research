/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia32/thread.h
 * Description:   IA-32 specific thread management
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
 * $Id: thread.h,v 1.4.4.1 2003/09/24 19:12:15 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH_IA32_THREAD_H__
#define __ARCH_IA32_THREAD_H__

#include INC_ARCH(config.h)
#include INC_API(thread.h)
#include INC_API(ktcb.h)

INLINE ktcb_t * get_ktcb(threadid_t tid)
{
    return (ktcb_t*)((KTCB_AREA_START) + (tid.get_threadno() * KTCB_SIZE));
}

INLINE ktcb_t * get_current_ktcb()
{
    word_t stack;
    asm("mov	%%esp, %0\n"
	: "=r"(stack)
	);
    return (ktcb_t*)(stack & KTCB_MASK);
}



#endif /*__ARCH_IA32_THREAD_H__*/
