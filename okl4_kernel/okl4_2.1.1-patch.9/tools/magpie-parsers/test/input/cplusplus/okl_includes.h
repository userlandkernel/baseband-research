typedef unsigned long long L4_Word64_t;
typedef unsigned long L4_Word32_t;
typedef unsigned short L4_Word16_t;
typedef unsigned char L4_Word8_t;

typedef unsigned long L4_Word_t;
typedef unsigned long L4_PtrSize_t;
typedef unsigned int L4_Size_t;

typedef short L4_Int16_t;
typedef L4_Word_t L4_Bool_t;


struct L4_KernelInterfacePage_struct;
typedef struct L4_KernelInterfacePage_struct L4_KernelInterfacePage_t;
static inline L4_Bool_t
L4_IpcError(L4_Word_t err, L4_Word_t *err_code)
{
    L4_Bool_t phase;

    phase = err & 0x1;
    if (err_code) {
        *err_code = (err >> 1) & 0xf;
    }
    return phase;
}





typedef union {
    L4_Word_t raw;
    struct {
        L4_Word_t rwx:3; L4_Word_t reserved:1; L4_Word_t s:6; L4_Word_t b:22 ;
    } X;
} L4_Fpage_t;
static inline int __L4_Msb(L4_Word_t w) __attribute__ ((const));


static inline int
__L4_Msb(L4_Word_t w)
{
    int bitnum;

    if (w == 0)
        return 32;

    for (bitnum = 0, w >>= 1; w != 0; bitnum++)
        w >>= 1;

    return bitnum;
}



static inline L4_Bool_t
L4_IsNilFpage(L4_Fpage_t f)
{
    return f.raw == 0;
}

static inline L4_Word_t
L4_Rights(L4_Fpage_t f)
{
    return f.X.rwx;
}

static inline L4_Fpage_t
L4_Set_Rights(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx = rwx;
    return *f;
}

static inline L4_Fpage_t
L4_FpageAddRights(L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx |= rwx;
    return f;
}

static inline L4_Fpage_t
L4_FpageAddRightsTo(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx |= rwx;
    return *f;
}

static inline L4_Fpage_t
L4_FpageRemoveRights(L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx &= ~rwx;
    return f;
}

static inline L4_Fpage_t
L4_FpageRemoveRightsFrom(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx &= ~rwx;
    return *f;
}
static inline L4_Fpage_t
L4_Fpage(L4_Word_t BaseAddress, L4_Word_t FpageSize)
{
    L4_Fpage_t fp = { 0UL };
    L4_Word_t msb = __L4_Msb(FpageSize);

    fp.raw = BaseAddress;
    fp.X.s = (1UL << msb) < FpageSize ? msb + 1 : msb;
    fp.X.rwx = (0x00);
    fp.X.reserved = 0;
    return fp;
}

static inline L4_Fpage_t
L4_FpageLog2(L4_Word_t BaseAddress, int FpageSize)
{
    L4_Fpage_t fp = { 0UL };
    fp.raw = BaseAddress;
    fp.X.s = FpageSize;
    fp.X.rwx = (0x00);
    fp.X.reserved = 0;
    return fp;
}

static inline L4_Word_t
L4_Address(L4_Fpage_t f)
{
    return f.raw & ~((1UL << f.X.s) - 1);
}

static inline L4_Word_t
L4_Size(L4_Fpage_t f)
{
    return f.X.s == 0 ? 0 : (1UL << f.X.s);
}

static inline L4_Word_t
L4_SizeLog2(L4_Fpage_t f)
{
    return f.X.s;
}





typedef union {
    L4_Word_t raw;
    struct {
        L4_Word_t version:14; L4_Word_t thread_no:18;
    } X;
} L4_GthreadId_t;

typedef union {
    L4_Word_t raw;
    L4_GthreadId_t global;
} L4_ThreadId_t;






static inline L4_ThreadId_t
L4_GlobalId(L4_Word_t threadno, L4_Word_t version)
{
    L4_ThreadId_t t = { 0UL };
    t.global.X.thread_no = threadno;
    t.global.X.version = version;

    return t;
}

static inline L4_Word_t
L4_Version(L4_ThreadId_t t)
{
    return t.global.X.version;
}

static inline L4_Word_t
L4_ThreadNo(L4_ThreadId_t t)
{
    return t.global.X.thread_no;
}

static inline L4_Bool_t
L4_IsThreadEqual(const L4_ThreadId_t l, const L4_ThreadId_t r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t
L4_IsThreadNotEqual(const L4_ThreadId_t l, const L4_ThreadId_t r)
{
    return l.raw != r.raw;
}
static inline L4_Bool_t
L4_IsNilThread(L4_ThreadId_t t)
{
    return t.raw == 0;
}
typedef union {
    L4_Word_t raw;
    L4_Word_t space_no;
} L4_SpaceId_t;





static inline L4_SpaceId_t
L4_SpaceId(L4_Word_t space_no)
{
    L4_SpaceId_t s = { 0UL };
    s.space_no = space_no;

    return s;
}

static inline L4_Word_t
L4_SpaceNo(L4_SpaceId_t s)
{
    return s.space_no;
}

static inline L4_Bool_t
L4_IsSpaceEqual(const L4_SpaceId_t l, const L4_SpaceId_t r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t
L4_IsSpaceNotEqual(const L4_SpaceId_t l, const L4_SpaceId_t r)
{
    return l.raw != r.raw;
}
static inline L4_Bool_t
L4_IsNilSpace(L4_SpaceId_t s)
{
    return s.raw == ~0UL;
}
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef int8_t int_least8_t;
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
typedef long long int intmax_t;
typedef unsigned long long int uintmax_t;
typedef uintptr_t objref_t;
typedef objref_t memsection_ref_t;
typedef objref_t thread_ref_t;
typedef objref_t pd_ref_t;
typedef objref_t session_ref_t;
typedef objref_t eas_ref_t;
typedef objref_t hw_ref_t;
typedef objref_t quota_ref_t;
typedef objref_t pool_ref_t;
typedef objref_t physmem_ref_t;

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
        pool_ref_t pool;
        physmem_ref_t physmem;
    } ref;
    uintptr_t passwd;
} cap_t;
static const int INTERFACE_IGUANA_EX_UUID = 21;
static const int INTERFACE_IGUANA_PD_UUID = 22;
static const int INTERFACE_IGUANA_EAS_UUID = 23;
static const int INTERFACE_IGUANA_THREAD_UUID = 24;
static const int INTERFACE_IGUANA_HARDWARE_UUID = 25;
static const int INTERFACE_IGUANA_MEMSECTION_UUID = 26;
static const int INTERFACE_IGUANA_SESSION_UUID = 27;
static const int INTERFACE_IGUANA_DEBUG_UUID = 28;
static const int INTERFACE_IGUANA_QUOTA_UUID = 29;
static const int INTERFACE_IGUANA_POOL_UUID = 30;
static const int INTERFACE_IGUANA_PHYSMEM_UUID = 31;
typedef int ptrdiff_t;
typedef unsigned int size_t;







typedef long int wchar_t;
