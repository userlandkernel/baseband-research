          void fatal () __attribute__ ((noreturn));
          
          typedef void voidfn ();
          
          volatile voidfn fatal;

          int square (int) __attribute__ ((const));

          typedef int intfn ();
          
          extern const intfn square;

          extern int
          my_printf (void *my_object, const char *my_format, ...)
                __attribute__ ((format (printf, 2, 3)));

          extern char *
          my_dgettext (char *my_domain, const char *my_format)
                __attribute__ ((format_arg (2)));

          extern void foobar (void) __attribute__ ((section ("bar")));

          void __f () { /* do something */; }
          void f () __attribute__ ((weak, alias ("__f")));

          int x __attribute__ ((aligned (16), packed)) = 0;

          struct foo { int x[2] __attribute__ ((aligned (8))); };

          short array[3] __attribute__ ((aligned));

          struct foo1
          {
            char a;
            int x[2] __attribute__ ((packed));
          };

          struct duart a __attribute__ ((section ("DUART_A"))) = { 0 };
          struct duart b __attribute__ ((section ("DUART_B"))) = { 0 };
          char stack[10000] __attribute__ ((section ("STACK"))) = { 0 };
          int init_data __attribute__ ((section ("INITDATA"))) = 0;
          
          struct S { short f[3]; } __attribute__ ((aligned (8)));
          typedef int more_aligned_int __attribute__ ((aligned (8)));

        union wait { int a, __ip; };
        typedef int pid_t;

          typedef union
            {
              int *__ip;
              union wait *__up;
            } wait_status_ptr_t __attribute__ ((__transparent_union__));
          
          pid_t wait (wait_status_ptr_t);

          int w1 () { int w; return wait (&w); }
          int w2 () { union wait w; return wait (&w); }


          pid_t wait (wait_status_ptr_t p)
          {
            return waitpid (-1, p.__ip, 0);
          }

          main()
          {
            /* Initialize stack pointer */
            init_sp (stack + sizeof (stack));
          
            /* Initialize initialized data */
            memcpy (&init_data, &data, &edata - &data);
          
            /* Turn on the serial ports */
            init_duart (&a);
            init_duart (&b);
          }
