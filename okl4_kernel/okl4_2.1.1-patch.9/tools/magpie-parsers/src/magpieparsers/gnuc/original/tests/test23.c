now() {
     union foo { int i; double d; };
     int x;
     double y;
     void hack (union foo);
     union foo u;
     u = (union foo) x  ==  u.i = x;
     u = (union foo) y  ==  u.d = y;
     hack ((union foo) x);

}        

