/*********************************************************************
 *                
 * Copyright (C) 2004-2006,  Karlsruhe University
 *                
 * File path:     bench/pingpong/ia32.h
 * Description:   IA32 specific pingpong functions
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
 * $Id: ia32.h,v 1.1.2.2 2006/06/08 14:37:10 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/arch.h>

#define HAVE_HANDLE_ARCH_PAGEFAULT
#define HAVE_READ_CYCLES
#define HAVE_ARCH_IPC

#define L4_REQUEST_MASK		( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))

#define UTCB_ADDRESS	(0x00800000UL)
#define KIP_ADDRESS	(0x00C00000UL)

#define L4_IO_PAGEFAULT		(-8UL << 20)
#define L4_IO_PORT_START	(0)
#define L4_IO_PORT_END		(1<<16)

extern L4_Word_t __L4_syscalls_start;
extern L4_Word_t __L4_syscalls_end;
extern "C" void __L4_copy_syscalls_in (L4_Word_t dest);

char syscall_stubs[4096] __attribute__ ((aligned (4096)));

L4_Fpage_t handle_arch_pagefault (L4_MsgTag_t tag, L4_Word_t faddr, L4_Word_t fip, L4_Word_t log2size)
{
    // If pagefault is in the syscall stubs, create a copy of the
    // original stubs and map in this copy.
    if (faddr >= (L4_Word_t) &__L4_syscalls_start &&
	faddr <  (L4_Word_t) &__L4_syscalls_end)
    {
	__L4_copy_syscalls_in ((L4_Word_t) syscall_stubs);
	return L4_FpageLog2 ( (L4_Word_t) syscall_stubs, log2size);
    }

    // If pagefault is an IO-Page, return that IO Fpage 
    L4_Fpage_t fp;
    fp.raw = faddr;
    if ((tag.raw & L4_REQUEST_MASK) == L4_IO_PAGEFAULT && L4_IsIoFpage(fp))
    {
	return fp;
    }
	
    return L4_FpageLog2 (faddr, log2size);
}
