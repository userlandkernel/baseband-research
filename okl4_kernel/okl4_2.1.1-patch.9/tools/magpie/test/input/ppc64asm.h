int __L4_Msb(int w)
{
    int zeros;
    asm volatile ("cntlzd %0, %1" : "=r" (zeros) : "r" (w) );
	return (zeros == 64) ? zeros : 63-zeros;
}
