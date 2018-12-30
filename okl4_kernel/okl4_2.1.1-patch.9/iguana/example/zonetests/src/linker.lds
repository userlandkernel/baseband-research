PHDRS
{
    text PT_LOAD;
    data PT_LOAD;
    zone PT_LOAD;
}

SECTIONS
{
    .text : {
        * (.text)
    } : text

    .rodata : {
        * (.rodata)
        /* FIXME: Temporary workaround for problematic toolchains - AGW. */
	* (.rodata.*)
    } : text

    .data ALIGN(0x100000) : {
        * (.data)
    } : data

    .bss : {
        * (.bss)
    } : data

    .zone ALIGN(0x100000) : {
        * (.zone)
    } : zone
}
