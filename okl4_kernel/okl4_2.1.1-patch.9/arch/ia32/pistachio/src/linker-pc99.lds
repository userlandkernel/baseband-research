/*********************************************************************
 *                
 * Copyright (C) 1999, 2000, 2001, 2002, 2003-2004,  Karlsruhe University
 *                
 * Description:   Generic linkser script for x86.
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
 *                
 ********************************************************************/


#include <l4.h>
#include <arch/offsets.h>

ENTRY(_start)

BOOTMEM_SIZE = 128K;

_start_text_phys = 0x00100000 + 0x200;
_start_text = _start_text_phys + KERNEL_RO_OFFSET;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_RO_OFFSET)
	{
		*(.mb_header)
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.*)
		*(.spinlock)
	}

	.rodata . : AT (ADDR(.rodata) - KERNEL_RO_OFFSET)
	{
		*(.rodata*)
	}

	.roinit : AT(ADDR(.roinit) - KERNEL_RO_OFFSET)
	{
		*(.roinit*)
	}

	. = ALIGN(4K);
	_start_cpu_local = .;
	.cpulocal . : AT (ADDR(.cpulocal) - KERNEL_RW_OFFSET)
	{
		*(.data.cpulocal.tcb)
		*(.data.cpulocal.utcb)
		*(.data.cpulocal)
		*(.data.ia32.cpulocal)
	}
	_end_cpu_local = .;

	. = ALIGN(4K);
	.data . :  AT (ADDR(.data) - KERNEL_RW_OFFSET)
	{
		*(.data)
	        *(.data.ia32.idt);
	        *(.data.ia32.exc_all);
	        *(.data.ia32.exc_common);
		*(.data.*)
		_bss_start = .;
		*(.bss)
		_bss_end = .;
	}

	. = ALIGN(4K);
	.kdebug . : AT(ADDR(.kdebug) - KERNEL_RW_OFFSET)
	{
		*(.kdebug)
		*(.kdebug-bss)
		*(.kdebug.*)
	}

#include <generic/linkersets.ldi>
	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - KERNEL_RW_OFFSET;

	. = ALIGN(4K);
	.reserve_bootmem : AT(ADDR(.reserve_bootmem) - KERNEL_RW_OFFSET)
	{
		*(.reserve_bootmem) 
	}

	_start_init = . - KERNEL_RW_OFFSET;
	.init (. - KERNEL_RW_OFFSET) :
	{
		*(.init) 
		*(.init.data)
		*(.init.smp)
		*(.init.*) 
	}
	_end_init = .;

#include <generic/debug.ldi>

	/* special section that is discarded during linking
	   all unwanted sections should go here */

	/DISCARD/ :
	{
		*(*)
		*(.eh_frame)
		*(.note)
		*(.comment)
		*(.delete)
	}

	/* all the physical address calculations */
	_end_text_phys = _end_text - KERNEL_RO_OFFSET;
	_start_bootmem_phys = _start_bootmem - KERNEL_RW_OFFSET;
	_end_bootmem_phys = _end_bootmem - KERNEL_RW_OFFSET;
}
