ENTRY(_start)
_start_text_phys = 0x50000;
_start_text = 0xFFFFFFFF80000000 + _start_text_phys;

SECTIONS
{
    .text _start_text : AT (ADDR(.text) - 0xFFFFFFFF80000000)
    {
        . = ALIGN(64K);
        *(.base)
        *(.text.startup)
        *(.text)
        *(.gnu.linkonce.*)
    }

    . = ALIGN(4K);
    .rodata . : AT (ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(4K);
    .kip . : AT (ADDR(.kip) - 0xFFFFFFFF80000000)
    {

  *(.data.kip)
  *(.data.kip.pdesc)
  *(.data.kip.kdesc)
  *(.data.kip.versionparts)
  *(.data.kip.features)
  *(.data.kip.features.end)
  . = ALIGN(16);
  *(.data.kip.mdesc)
        . = ALIGN(2K);
        *(.user.*)
    }
    . = ALIGN(4K);

    _memory_descriptors_offset = memory_descriptors - kip;
    _memory_descriptors_size =
        (2K - ((memory_descriptors - kip) & (4K-1))) / 16;

    .data . : AT (ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.sdata)
        *(.data)
        *(.data.*)
    }

    .kdebug . : AT(ADDR(.kdebug) - 0xFFFFFFFF80000000)
    {
        *(.kdebug)
    }

    .bss . : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
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

    .kdebug-data . : AT(ADDR(.kdebug-data) - 0xFFFFFFFF80000000)
    {
        *(.data.kdebug)
        *(.kdebug.data)
        *(.kdebug-bss)
    }

    _end_text = ALIGN(4K);
    _end_text_phys = _end_text - 0xFFFFFFFF80000000;

    . = ALIGN(4K);
    .init _end_text : AT(ADDR(.init) - 0xFFFFFFFF80000000)
    {
        *(.init)
        *(.init.*)
        *(.roinit)
        *(.init.data)
    }





    _end = ALIGN(8K) - 0xFFFFFFFF80000000;
    /DISCARD/ :
    {
        *(*)
        *(.eh_frame)
        *(.note)
        *(.comment)
        *(.delete)
    }
}
