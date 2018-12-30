     struct foo {int a[4];};
     
     struct foo f();
     
     bar (int index)
     {
       return f().a[index];
     }
