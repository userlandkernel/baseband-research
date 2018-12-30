/*-template_version = 2 -*/
/* This from $pistachio/include/l4/arm/syscalls.h */
#define MAGPIE__L4_SYSCALL                   \
        "str    fp,     [sp, #-4]! \n" \
        "mov    lr,     pc         \n" \
        "mov    pc,     ip         \n" \
        "ldr    fp,     [sp],   #4 \n"

