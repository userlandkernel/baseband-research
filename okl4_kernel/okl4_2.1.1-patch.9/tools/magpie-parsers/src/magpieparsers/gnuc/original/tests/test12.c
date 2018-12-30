int foo();
int abs;
int main() {
  abs = ({
        __label__ hey, now;

        int y = foo ();
        int z;
        void *it;
        it = &&hey;

        hey: if (y > 0) z = y;

        else z = - y;
        z;
        });
    
}

