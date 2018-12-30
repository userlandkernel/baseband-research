/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */

#include "types.h"
#include "libgcc.h"
#include <l4/kdebug.h>

uint64_t
__fixunssfdi(float f)
{
    L4_KDB_Enter("__fixunssfdi");
    return 0;
}
