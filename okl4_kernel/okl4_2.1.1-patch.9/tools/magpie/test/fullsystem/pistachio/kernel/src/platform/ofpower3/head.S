/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofpower3/head.S
 * Description:	Kernel entry point
 *		kernel boot.  Clears .bss, sets up the initial stack,
 *		and jumps into C code.
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
 * $Id: head.S,v 1.3 2003/09/29 00:54:21 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(msr.h)

#warning FIXME

#define INIT_STACK_SIZE	    (4096*4)

/* Initialisation stack */
	.section ".init.data"
	.align 3
init_stack:
	.space (INIT_STACK_SIZE)

/* _start - the Entry point of L4
 *   We should enter in real mode, but we switch to real mode just in case.
 *   It is assumed that we have the same lower 48 bit alignment as the kernel
 *   image.
 */

	.section ".init"
	.align	2
	.globl	_start
	.type	_start,@function
_start:
	LD_LABEL(r10, __real_mode_jump)		/* Load address of real-mode init */
	LD_CONST(r12, MSR_INIT_REAL_MODE)	/* Get the initial real mode MSR setting and jump to 64-bit mode */
	mtsrr0	r10				/* Setup jump address */
	mtsrr1	r12				/* Enable read mode */
	rfid					/* Context Changing Jump */

/* From here, we are running 64-bit with real-addressing.
 * Physical address offset is in r11.
 */
__real_mode_jump:

	/* Load the TOC for  start_kernel */
	LD_ADDR(r10,start_kernel)
	ld	r2, 8(r10)

	/*  Use our local init/boot stack.  */
	LD_ADDR (r1, init_stack)		/* Get a pointer to init_stack. */
	addi	r1, r1, INIT_STACK_SIZE-48	/* Position ourselves at the top of the stack. */

	bl	.start_kernel

	.long	0x00000000

