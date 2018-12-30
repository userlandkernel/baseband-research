/* check ALIGN(4K) before use !!! */

#include <l4.h>
#include <arch/offsets.h>

ENTRY(_start)
_start_text_phys        = 0x50000;
_start_text             = KERNEL_RO_OFFSET + _start_text_phys;

SECTIONS
{
    .text _start_text : AT (ADDR(.text) - KERNEL_RO_OFFSET)
    {
        . = ALIGN(64K);
        *(.base)
        *(.text.startup)
        *(.text)
        *(.gnu.linkonce.*)
    }

    . = ALIGN(4K);
    .rodata . : AT (ADDR(.rodata) - KERNEL_RO_OFFSET)
    {
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(4K);
    .kip .  : AT (ADDR(.kip) - KERNEL_RW_OFFSET)
    {
#include <kip.ldi>
        . = ALIGN(2K);
        *(.user.*)
    }
    . = ALIGN(4K);

    _memory_descriptors_offset = memory_descriptors - kip;
    _memory_descriptors_size =
        (2K - ((memory_descriptors - kip) & (4K-1))) / 16;

    .data . :  AT (ADDR(.data) - KERNEL_RW_OFFSET)
    {
        *(.sdata)
        *(.data)
        *(.data.*)
    }

    .kdebug . : AT(ADDR(.kdebug) - KERNEL_RW_OFFSET)
    {
        *(.kdebug)
    }

    .bss . : AT(ADDR(.bss) - KERNEL_RW_OFFSET)
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
    }

    .kdebug-data . : AT(ADDR(.kdebug-data) - KERNEL_RW_OFFSET)
    {
        *(.data.kdebug)
        *(.kdebug.data)
        *(.kdebug-bss)
    }

    _end_text = ALIGN(4K);
    _end_text_phys = _end_text - KERNEL_RO_OFFSET;

    . = ALIGN(4K);
    .init _end_text : AT(ADDR(.init) - KERNEL_RW_OFFSET)
    {
        *(.init)
        *(.init.*)
        *(.roinit)
        *(.init.data)
    }

    /* special section that is discarded during linking
       all unwanted sections should go here */

    /* Required for user code! */
    _end = ALIGN(8K) - KERNEL_RW_OFFSET;
    /DISCARD/ :
    {
        *(*)
        *(.eh_frame)
        *(.note)
        *(.comment)
        *(.delete)
    }
}
