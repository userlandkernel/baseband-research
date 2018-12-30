PHDRS
{
    text PT_LOAD;
    data PT_LOAD;
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

    . = ALIGN(4K);
    .data : {
        * (.data)
    } : data

    .bss : {
        * (.bss)
    } : data

}
