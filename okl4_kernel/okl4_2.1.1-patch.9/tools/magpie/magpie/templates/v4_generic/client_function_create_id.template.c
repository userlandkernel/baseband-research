!!explicit_linebreaks
/*-if function.get_is_pagefault()*/
/* ID = -2, signifying a kernel message (refer L4 user manual) */\n
#define /*-?function.get_defable_name()-*/_CALL_ID ((-2UL) << 4) \n
/*fi-*/
/*-if not function.get_is_pagefault()*/
/* the *_CALL_ID is defined as 64 * the interface ID, plus the function number.\n
 * This restricts us to 64 functions per interface.\n
*/\n
#define /*-?function.get_defable_name()-*/_CALL_ID (64 * (/*-?interface.get_uuid()-*/) + /*-?function.get_number()-*/u) \n
/*fi-*/
