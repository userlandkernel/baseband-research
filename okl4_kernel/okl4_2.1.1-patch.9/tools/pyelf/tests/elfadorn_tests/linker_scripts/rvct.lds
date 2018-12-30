
SEGMENT_1 0x080000
{
    
    SECTION_B +0
    {
        * (SECTION_B)
    }

    SECTION_C +0
    {
        * (SECTION_C)
    }

}

SEGMENT_2 0x120000
{
    SECTION_A +0
    {
        * (SECTION_A)
    }

    SECTION_F +0
    {
        * (SECTION_F)
    }

    SECTION_G +0
    {
        * (SECTION_G)
    }
    SECTION_H +0
    {
        * (SECTION_H)
    }
}
SEGMENT_3 0x130000
{
    SECTION_D +0
    {
        * (SECTION_D)
    }

    SECTION_E +0
    {
        * (SECTION_E)
    }
}
SEGMENT_4 0x140000
{
    SECTION_I +0
    {
        * (SECTION_I)
    }
}
SEGMENT_5 0x150000
{
    SECTION_J +0
    {
        * (SECTION_J)
    }
}

LR_1 0x160000
{
    ER_RO +0
    {
        * (+RO)
    }

    ER_RW +0
    {
        * (+RW)
    }

}
GIRAFFE 0x180000
{
    ER_ZI +0
    {
        * (+ZI)
    }
}
