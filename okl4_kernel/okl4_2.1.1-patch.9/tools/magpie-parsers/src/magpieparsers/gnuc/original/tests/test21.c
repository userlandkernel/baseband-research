
heynow() {
     struct foo {int a; char b[2];} structure;
     char **foo1 = (char *[]) { "x", "y", "z" };

     structure = ((struct foo) {x + y, 'a', 0});

     {
       struct foo temp = {x + y, 'a', 0};
       structure = temp;
     }

     output = ((int[]) { 2, x, 28 }) [input];
}
    
