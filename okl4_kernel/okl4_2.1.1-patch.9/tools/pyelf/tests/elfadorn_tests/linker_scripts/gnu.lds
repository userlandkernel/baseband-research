
PHDRS
{
SEGMENT_1 PT_LOAD;
SEGMENT_2 PT_LOAD;
SEGMENT_3 PT_LOAD;
SEGMENT_4 PT_LOAD;
SEGMENT_5 PT_LOAD;
text PT_LOAD;
data PT_LOAD;
bss PT_LOAD;
}

SECTIONS
{
    SECTION_B : { *(SECTION_B) } : SEGMENT_1
    SECTION_C : { *(SECTION_C) } : SEGMENT_1
    SECTION_D : { *(SECTION_D) } : SEGMENT_3
    SECTION_E : { *(SECTION_E) } : SEGMENT_3
    SECTION_A : { *(SECTION_A) } : SEGMENT_2 
    SECTION_F : { *(SECTION_F) } : SEGMENT_2
    SECTION_G : { *(SECTION_G) } : SEGMENT_2
    SECTION_H : { *(SECTION_H) } : SEGMENT_2
    SECTION_I : { *(SECTION_H) } : SEGMENT_4
    SECTION_J : { *(SECTION_H) } : SEGMENT_5
    
    . = 0x70000000;
    .text : { *(.text) } :text
    . = 0x80000000;
    .data : { *(.data) } :data
    .bss : { *(.bss) } :bss
    _end = .;

}
