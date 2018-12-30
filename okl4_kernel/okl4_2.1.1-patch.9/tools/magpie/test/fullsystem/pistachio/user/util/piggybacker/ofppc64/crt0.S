/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     piggybacker/ofppc64/crt0.S
 * Description:   PowerPC64 Startup
 *                
 * @LICENSE@
 *                
 * $Id: crt0.S,v 1.1 2003/10/23 06:25:05 cvansch Exp $
 *                
 ********************************************************************/

#include <l4/powerpc64/asm.h>

/*
 *  Define a stack.
 */
	.section ".bss"
	.globl	_init_stack_bottom
	.globl	_init_stack_top

#define INIT_STACK_SIZE (4096*2)
.align	4
_init_stack_bottom:
.lcomm	init_stack, INIT_STACK_SIZE
_init_stack_top:

/*
 *  Main entry point.
 */
	.section ".start"
	.align	3
	.globl	_start
	.globl	__start
_start:
__start:
	/*  Use our local stack.  */
	lis	sp, init_stack@ha
	la	sp, init_stack@l(sp)
	addi	sp, sp, INIT_STACK_SIZE-512

	/*  Initialize .bss (which also zeros the stack).  */
#define BSS_START	__bss_start
#define BSS_END		__bss_end
	lis	%r10, BSS_START@ha
	la	%r10, BSS_START@l(%r10)
	subi	%r10, %r10, 4
	lis	%r11, BSS_END@ha
	la	%r11, BSS_END@l(%r11)
	subi	%r11, %r11, 4
	li	%r12, 0
1:	cmp	0, %r10, %r11
	beq	2f
	stwu	%r12, 4(%r10)
	b	1b
2:
	LD_ADDR(toc,__toc_value)

	/*  Jump into C code.  */
	bl	.loader_main
	
3:	b	3b		/* we should never execute this line.  */


/*
 *  Kernel hand-off point.
 *  void enter_kernel( r3, r4, r5, start_ip )
 */
	.section ".text"
	.align	2
	.globl	.enter_kernel
.enter_kernel:
	sync
	isync

	LD_CONST(%r10, 0x1000000000003000)
	
	mtsrr1	%r10		// Reset the MSR - 64-Bit + Data Translation.
	mtsrr0	%r6		// Install the kernel's start ip address.
	rfid
.end .enter_kernel

/*
 * OpenFirmware note section.
 */

#if 0
	.section ".note"
	.align	2
.note_section_header:
	.long	8		/* note name length (including null byte) */
	.long	24		/* note descriptor length */
	.long	0x1275		/* note type */
	.string	"PowerPC"
.note_descriptor:
	.long	0x0		/* real-mode (0 == false)	*/
	.long	0xffffffff	/* real-base (-1 == default)	*/
	.long	0xffffffff	/* real-size (-1 == default)	*/
	.long	0xffffffff	/* virt-base (-1 == default)	*/
	.long	0xffffffff	/* virt-size (-1 == default)	*/
	.long	0xffffffff	/* load-base (-1 == default)	*/
#endif
