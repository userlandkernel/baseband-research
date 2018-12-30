    //generalized lvalues
foo() {
      int a,b,f;
     (a, b) += 5;
     a, (b += 5);
     &(a, b);
     a, &b;
     (a ? b : c) = 5 ; 
     (a ? b = 5 : (c = 5)) ;

     (int)a = 5 ; 
     (int)(a = (char *)(int)5) ;

     (int)a += 5;
     (int)(a = (char *)(int) ((int)a + 5));

    (int)f = 1;
    (int *)&f;
}

