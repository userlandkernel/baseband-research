     int foo asm ("myfoo") = 2;

     extern func () asm ("FUNC");
     
     func (x, y) {
        int x, y;
        return x+y;
     }


