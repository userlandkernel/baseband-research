main() {
        if (nerror) cleanup();
        else if (argc < 1) faterror("no input file%s", cmdusage);
        else for (;  0 < argc;  cleanup(), ++argv, --argc) {
          targetdelta = 0;
          ffree();
        }
}

