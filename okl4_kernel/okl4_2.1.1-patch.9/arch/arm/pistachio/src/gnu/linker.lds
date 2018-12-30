/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * Description:   ARM Linker Script
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

ENTRY(_start)

MEMORY
{
#if defined(CONFIG_XIP)
    rom(RX)  :   ORIGIN = VIRT_ADDR_ROM, LENGTH = 0x01000000
    ram(RWX) :   ORIGIN = VIRT_ADDR_RAM, LENGTH = 0x10000000
#else
    ram(RWX) :   ORIGIN = VIRT_ADDR_RAM, LENGTH = 0x10000000
#endif
}

#if !defined(CONFIG_XIP)
#define rom     ram
#endif

SECTIONS
{
    /* Readonly sections starts here. */

    .text :
    {
        _start_rom = .;

        . = ALIGN(64K);
        *(.init.head)
        *(.base)
        *(.text.startup)
        *(.text)
        *(.gnu.linkonce.*)
    } > rom

    .rodata . :
    {
        *(.rodata)
        *(.rodata.*)
    } > rom

    .kdebug . :
    {
        *(.kdebug)
    } > rom

    _end_rom = .;                       /* ROM ends here. */

    _start_init = .;
    .init . :
    {
        *(.init)
        *(.init.*)
    } > rom

    .roinit . :
    {
        *(.roinit)   
    } > rom
    _end_init = .;

    /* Readwrite sections starts here. */
    .data :
    {

        _start_ram = .;                 /* RAM starts here. */
        _start_data = .;
#if defined(CONFIG_XIP)
        . += 256;                       /* Software GOT */
#endif

        *(.data.cpulocal)
        *(.data.cpulocal.stack)
        *(.data.cpulocal.utcb)
        *(.data.cpulocal.tcb)
        *(.sdata)
        *(.data)
        /**(.data.*)*/
    } > ram

    .kdebug-data . :
    {
        *(.data.kdebug)
        *(.kdebug.data)
    } > ram

    .bss . :
    {
        . = ALIGN(16);
        _start_setlist = .;
        *(.setlist)
        _end_setlist = .;
        . = ALIGN(16);
        _start_sets = .;
        *(SORT(set_*))
        _end_sets = .;
	. = ALIGN(32);
        *(.bss)
        *(.kdebug-bss)
    } > ram

    . = ALIGN(16K);
    .kspace . :
    {
        *(.data.kspace)
    } > ram
    . = ALIGN(4K);
    .traps . :
    {
        *(.data.traps)
    } > ram

    .utcb_page . :
    {
        *(.data.utcb)
    } > ram

    . = ALIGN(4K);
    _end_ram = .;                   /* RAM ends here. */

    /* special section that is discarded during linking
       all unwanted sections should go here */

    
    /* Required for user code! */
    . = ALIGN(4K);
    _end = .;

    /DISCARD/ :
    {
        /*              *(*)            <-- THIS CAUSES SEGFAULT!!! */
        *(.eh_frame)
        *(.note)
        *(.comment)
        *(.delete)
    }
}

ASSERT(SIZEOF(.traps) <= 0x1000, "ERROR: trap section is larger than 4K!!")
