/* This from $pistachio/include/l4/arm/syscalls.h */
#define MAGPIE__L4_SYSCALL                   \
        "str    fp,     [sp, #-4]! \n" \
        "mov    lr,     pc         \n" \
        "mov    pc,     ip         \n" \
        "ldr    fp,     [sp],   #4 \n"

#ifdef __cplusplus
extern "C" {
#endif
#include "idl4biguuid_arm/idl4.h"
#ifdef __cplusplus
}
#endif
