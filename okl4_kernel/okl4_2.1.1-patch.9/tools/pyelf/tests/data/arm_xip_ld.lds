ENTRY(_start)

MEMORY
{

    rom(RX) : ORIGIN = 0xF0000000, LENGTH = 0x01000000
    ram(RWX) : ORIGIN = (0xF0000000 + 0x01000000), LENGTH = 0x10000000



}





SECTIONS
{


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

    _end_rom = .;

    _start_init = .;
    .init . :
    {
        *(.init)
        *(.init.*)
        *(.roinit)
    } > rom
    _end_init = .;



    .data :
    {
        _start_ram = .;

        _start_data = .;

        . += 256;


        *(.data.cpulocal)
        *(.data.cpulocal.stack)
        *(.data.cpulocal.tcb)
        *(.data.cpulocal.utcb)
        *(.sdata)
        *(.data)

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
    .kip . :
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
        . = ALIGN(32);
        *(.user_thumb.*)
    } > ram
    . = ALIGN(4K);
    _end_kip = .;

    _memory_descriptors_offset = memory_descriptors - kip;
    _memory_descriptors_size =
        (2K - ((memory_descriptors - kip) & (4K-1))) / 16;

    .traps . :
    {
        *(.data.traps)
    } > ram

    . = ALIGN(4K);
    _end_ram = .;





    . = ALIGN(4K);
    _end = .;

    /DISCARD/ :
    {

        *(.eh_frame)
        *(.note)
        *(.comment)
        *(.delete)
    }
}
