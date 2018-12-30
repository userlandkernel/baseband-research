typedef unsigned int L4_Word_t;

typedef union {
    L4_Word_t   raw;
    struct {
		L4_Word_t version : 14;
		L4_Word_t thread_no : 18;
	} X;
} L4_GthreadId_t;

typedef union {
    L4_Word_t           raw;
    L4_GthreadId_t      global;
} L4_ThreadId_t;

static const L4_ThreadId_t L4_nilthread = { 0UL };

