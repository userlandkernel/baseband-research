# 1 "<stdin>"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "<stdin>"

# 1 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h" 1

# 34 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"










# 78 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"




# 1 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/ia32/types.h" 1

# 34 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/ia32/types.h"






typedef unsigned long long      L4_Word64_t;
typedef unsigned long		L4_Word32_t;
typedef unsigned short		L4_Word16_t;
typedef unsigned char		L4_Word8_t;

typedef unsigned long		L4_Word_t;
typedef unsigned int		L4_Size_t;

# 82 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h" 2









typedef L4_Word_t		L4_Bool_t;




// XXX: magpie workaround
// # define __PLUS32	+ (sizeof (L4_Word_t) * 8 - 32)
# 107 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"

# 141 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"


# 162 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"





# 177 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"


















static inline L4_Bool_t L4_IpcError (L4_Word_t err, L4_Word_t *err_code)
{
	L4_Bool_t phase;
	phase = err & 0x1;
	if (err_code) {
		*err_code = (err >> 1) & 0xf;
	}
	return phase;
}






typedef union {
    L4_Word_t	raw;
    struct {
	L4_Word_t   		rwx : 3; L4_Word_t  		reserved : 1; L4_Word_t  		s : 6; L4_Word_t  		b : 22 ;




    } X;
} L4_Fpage_t;















# 243 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"


# 1 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/ia32/specials.h" 1

# 34 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/ia32/specials.h"






static inline int __L4_Msb (L4_Word_t w) __attribute__ ((const));

static inline int __L4_Msb (L4_Word_t w)
{
    int bitnum;

    __asm__ (
	"/* l4_msb()		*/			\n"
	"	bsr	%1, %0				\n"

	: 
	"=r" (bitnum)
	: 
	"rm" (w)
	);

    return bitnum;
}







typedef union {
    L4_Word_t	raw;
    struct {
	unsigned	rwx:4;
	unsigned	__two:6;
	unsigned	s:6;
	unsigned	p:16;
    } X;
} L4_IoFpage_t;

# 111 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/ia32/specials.h"









static inline L4_Word_t L4_SmallSpace (L4_Word_t location, L4_Word_t size)
{
    location >>= 1;
    size >>= 2;
    return ((location & ~(size - 1)) | size) & 0xff;
}




# 245 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h" 2

static inline L4_Bool_t L4_IsNilFpage (L4_Fpage_t f)
{
    return f.raw == 0;
}

static inline L4_Word_t L4_Rights (L4_Fpage_t f)
{
    return f.X.rwx;
}

static inline L4_Fpage_t L4_Set_Rights (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx = rwx;
    return *f;
}

static inline L4_Fpage_t L4_FpageAddRights (L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx |= rwx;
    return f;
}

static inline L4_Fpage_t L4_FpageAddRightsTo (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx |= rwx;
    return *f;
}

static inline L4_Fpage_t L4_FpageRemoveRights (L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx &= ~rwx;
    return f;
}

static inline L4_Fpage_t L4_FpageRemoveRightsFrom (L4_Fpage_t * f, L4_Word_t rwx)
{
    f->X.rwx &= ~rwx;
    return *f;
}

# 307 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"

static inline L4_Fpage_t L4_Fpage (L4_Word_t BaseAddress, L4_Word_t FpageSize)
{
    L4_Fpage_t fp;
    L4_Word_t msb = __L4_Msb (FpageSize);
    fp.raw = BaseAddress;
    fp.X.s = (1UL << msb) < FpageSize ? msb + 1 : msb;
    fp.X.rwx = (0x00);
    fp.X.reserved = 0;
    return fp;
}

static inline L4_Fpage_t L4_FpageLog2 (L4_Word_t BaseAddress, int FpageSize)
{
    L4_Fpage_t fp;
    fp.raw = BaseAddress;
    fp.X.s = FpageSize;
    fp.X.rwx = (0x00);
    fp.X.reserved = 0;
    return fp;
}

static inline L4_Word_t L4_Address (L4_Fpage_t f)
{
    return f.raw & ~((1UL << f.X.s) - 1);
}

static inline L4_Word_t L4_Size (L4_Fpage_t f)
{
    return f.X.s == 0 ? 0 : (1UL << f.X.s);
}

static inline L4_Word_t L4_SizeLog2 (L4_Fpage_t f)
{
    return f.X.s;
}






typedef union {
    L4_Word_t	raw;
    struct {
	 L4_Word_t  		version : 14;  L4_Word_t  		thread_no : 18;


    } X;
} L4_GthreadId_t;

typedef union {
    L4_Word_t		raw;
    L4_GthreadId_t	global;
} L4_ThreadId_t;













static inline L4_ThreadId_t L4_GlobalId (L4_Word_t threadno, L4_Word_t version)
{
    L4_ThreadId_t t;
    t.global.X.thread_no = threadno;
    t.global.X.version = version;
    return t;
}

static inline L4_Word_t L4_Version (L4_ThreadId_t t)
{
    return t.global.X.version;
}

static inline L4_Word_t L4_ThreadNo (L4_ThreadId_t t)
{
    return t.global.X.thread_no;
}

static inline L4_Bool_t L4_IsThreadEqual (const L4_ThreadId_t l,
				      const L4_ThreadId_t r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t L4_IsThreadNotEqual (const L4_ThreadId_t l,
					 const L4_ThreadId_t r)
{
    return l.raw != r.raw;
}

# 418 "/Users/wzdd/uni/src/iguana-project/libs/l4/include/l4/types.h"

static inline L4_Bool_t L4_IsNilThread (L4_ThreadId_t t)
{
    return t.raw == 0;
}






# 2 "<stdin>" 2

# 1 "/Users/wzdd/uni/src/iguana-project/libs/iguana/include/iguana/types.h" 1

# 81 "/Users/wzdd/uni/src/iguana-project/libs/iguana/include/iguana/types.h"



# 1 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h" 1

# 81 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h"


# 1 "/Users/wzdd/uni/src/iguana-project/libs/c/include/limits.h" 1

# 79 "/Users/wzdd/uni/src/iguana-project/libs/c/include/limits.h"





































# 83 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h" 2





# 1 "/Users/wzdd/uni/src/iguana-project/libs/c/arch_include/arm/arch/stdint.h" 1

# 79 "/Users/wzdd/uni/src/iguana-project/libs/c/arch_include/arm/arch/stdint.h"



typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

# 88 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h" 2
























typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;































typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;



























typedef int32_t intptr_t;
typedef uint32_t uintptr_t;





# 205 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h"






typedef long long int intmax_t;
typedef unsigned long long int uintmax_t;






























# 251 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h"

# 260 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h"

# 269 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stdint.h"







# 84 "/Users/wzdd/uni/src/iguana-project/libs/iguana/include/iguana/types.h" 2

typedef uintptr_t objref_t;
typedef objref_t memsection_ref_t;
typedef objref_t thread_ref_t;
typedef objref_t pd_ref_t;
typedef objref_t session_ref_t;
typedef objref_t eas_ref_t;
typedef objref_t hw_ref_t;
typedef objref_t quota_ref_t;

typedef struct {
	union {
		objref_t obj;
		memsection_ref_t memsection;
		thread_ref_t thread;
		pd_ref_t pd;
		session_ref_t session;
		eas_ref_t eas;
		hw_ref_t hw;
		quota_ref_t quota;
	} ref;
	uintptr_t passwd;
} cap_t;




# 3 "<stdin>" 2


# 1 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stddef.h" 1

# 79 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stddef.h"















# 104 "/Users/wzdd/uni/src/iguana-project/libs/c/include/stddef.h"
























# 5 "<stdin>" 2
