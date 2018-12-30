fn() {
     asm ("fsinx %1,%0" : "=f" (result) : "f" (angle));
     asm ("combine %2,%0" : "=r" (foo) : "0" (foo), "g" (bar));

     asm ("combine %2,%0" : "=r" (foo) : "r" (foo), "g" (bar));

     asm volatile ("movc3 %0,%1,%2"
                   : /* no outputs */
                   : "g" (from), "g" (to), "g" (count)
                   : "r0", "r1", "r2", "r3", "r4", "r5");

      asm ("movl %0,r9;movl %1,r10;call _foo"
          : /* no outputs */
          : "g" (from), "g" (to)
          : "r9", "r10");

     asm ("clr %0;frob %1;beq 0f;mov #1,%0;0:"
          : "g" (result)
          : "g" (input));

     { double __value, __arg = (x);   
        asm ("fsinx %1,%0": "=f" (__value): "f" (__arg));  
        __value; }


     { int __old; 
        asm volatile ("get_and_set_priority %0, %1": "=g" (__old) : "g" (new)); 
        __old; }
} 

