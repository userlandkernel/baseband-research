
static inline void
ttt (int *wwp)
{
    int xyz = 6;
    do {
        asm(".set noreorder; 
             lqq %7,5(%8); 
             nop; 
             ziv %2,%2,-4; 
1:
             ab %5,7(%1);
             .set reorder"
            : "=r" (xyz)        
            : "r" (wwp), "0" (xyz));        
    } while (!xyz);
}

