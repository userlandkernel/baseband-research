typedef unsigned long L4_Word64_t;
typedef unsigned int L4_Word32_t;
typedef unsigned short L4_Word16_t;
typedef unsigned char L4_Word8_t;
typedef L4_Word64_t L4_Word_t;
typedef long unsigned int L4_Size_t;
typedef L4_Word_t L4_Bool_t;
typedef union {
    L4_Word_t raw;
    struct {
 L4_Word_t rwx : 3; L4_Word_t reserved : 1; L4_Word_t s : 6; L4_Word_t b : 22 + (sizeof (L4_Word_t) * 8 - 32);
    } X;
} L4_Fpage_t;
static inline int __L4_Msb (L4_Word_t w) __attribute__ ((const));
static inline int __L4_Msb (L4_Word_t w)
{
    int bitnum;
    if (w == 0)
 return 64;
    for (bitnum = 0, w >>= 1; w != 0; bitnum++)
 w >>= 1;
    return bitnum;
}
typedef union {
    L4_Word_t raw;
    struct {
 L4_Word_t rwx:4;
 L4_Word_t __two:6;
 L4_Word_t s:6;
 L4_Word_t p:48;
    } X;
} L4_PCIConfigFpage_t;
static inline L4_Fpage_t L4_PCIConfigFpage (L4_Word_t BaseAddress, int FpageSize)
{
    L4_PCIConfigFpage_t fp;
    L4_Fpage_t ret;
    L4_Word_t msb = __L4_Msb (FpageSize);
    fp.X.p = BaseAddress;
    fp.X.__two = 2;
    fp.X.s = (1UL << msb) < (L4_Word_t) FpageSize ? msb + 1 : msb;
    fp.X.rwx = (0x00);
    ret.raw = fp.raw;
    return ret;
}
static inline L4_Fpage_t L4_PCIConfigFpageLog2 (L4_Word_t BaseAddress,
         int FpageSize)
{
    L4_PCIConfigFpage_t fp;
    L4_Fpage_t ret;
    fp.X.p = BaseAddress;
    fp.X.__two = 2;
    fp.X.s = FpageSize;
    fp.X.rwx = (0x00);
    ret.raw = fp.raw;
    return ret;
}
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
static inline L4_Fpage_t L4_Fpage (L4_Word_t BaseAddress, L4_Word_t FpageSize)
{
    L4_Fpage_t fp;
    L4_Word_t msb = __L4_Msb (FpageSize);
    fp.raw = BaseAddress;
    fp.X.s = (1UL << msb) < FpageSize ? msb + 1 : msb;
    fp.X.rwx = (0x00);
    return fp;
}
static inline L4_Fpage_t L4_FpageLog2 (L4_Word_t BaseAddress, int FpageSize)
{
    L4_Fpage_t fp;
    fp.raw = BaseAddress;
    fp.X.s = FpageSize;
    fp.X.rwx = (0x00);
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
    L4_Word_t raw;
    struct {
 L4_Word_t version : 32; L4_Word_t thread_no : 32;
    } X;
} L4_GthreadId_t;
typedef union {
    L4_Word_t raw;
    struct {
 L4_Word_t __zeros : 6; L4_Word_t local_id : 26 + (sizeof (L4_Word_t) * 8 - 32);
    } X;
} L4_LthreadId_t;
typedef union {
    L4_Word_t raw;
    L4_GthreadId_t global;
    L4_LthreadId_t local;
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
static inline L4_Bool_t L4_IsNilThread (L4_ThreadId_t t)
{
    return t.raw == 0;
}
static inline L4_Bool_t L4_IsLocalId (L4_ThreadId_t t)
{
    return t.local.X.__zeros == 0;
}
static inline L4_Bool_t L4_IsGlobalId (L4_ThreadId_t t)
{
    return t.local.X.__zeros != 0;
}
typedef union {
    L4_Word64_t raw;
    struct {
 L4_Word32_t low;
 L4_Word32_t high;
    } X;
} L4_Clock_t;
static inline L4_Clock_t L4_ClockAddUsec (const L4_Clock_t c, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : c.raw + r };
}
static inline L4_Clock_t L4_ClockSubUsec (const L4_Clock_t c, const L4_Word64_t r)
{
    return (L4_Clock_t) { raw : c.raw - r };
}
static inline L4_Bool_t L4_IsClockEarlier (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw < r.raw;
}
static inline L4_Bool_t L4_IsClockLater (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw > r.raw;
}
static inline L4_Bool_t L4_IsClockEqual (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw == r.raw;
}
static inline L4_Bool_t L4_IsClockNotEqual (const L4_Clock_t l, const L4_Clock_t r)
{
    return l.raw != r.raw;
}
typedef union {
    L4_Word16_t raw;
    struct {
 L4_Word_t m : 10; L4_Word_t e : 5; L4_Word_t a : 1;
    } period;
    struct {
 L4_Word_t m : 10; L4_Word_t c : 1; L4_Word_t e : 4; L4_Word_t a : 1;
    } point;
} L4_Time_t;
static inline L4_Time_t L4_TimePeriod (L4_Word64_t microseconds)
{
    L4_Time_t time;
    time.raw = 0;
    if (__builtin_constant_p (microseconds)) {
 if (0) {}
 else if (microseconds < (1UL << 10)) do { time.period.m = microseconds >> (10 - 10); time.period.e = 10 - 10; } while (0); else if (microseconds < (1UL << 11)) do { time.period.m = microseconds >> (11 - 10); time.period.e = 11 - 10; } while (0);
 else if (microseconds < (1UL << 12)) do { time.period.m = microseconds >> (12 - 10); time.period.e = 12 - 10; } while (0); else if (microseconds < (1UL << 13)) do { time.period.m = microseconds >> (13 - 10); time.period.e = 13 - 10; } while (0);
 else if (microseconds < (1UL << 14)) do { time.period.m = microseconds >> (14 - 10); time.period.e = 14 - 10; } while (0); else if (microseconds < (1UL << 15)) do { time.period.m = microseconds >> (15 - 10); time.period.e = 15 - 10; } while (0);
 else if (microseconds < (1UL << 16)) do { time.period.m = microseconds >> (16 - 10); time.period.e = 16 - 10; } while (0); else if (microseconds < (1UL << 17)) do { time.period.m = microseconds >> (17 - 10); time.period.e = 17 - 10; } while (0);
 else if (microseconds < (1UL << 18)) do { time.period.m = microseconds >> (18 - 10); time.period.e = 18 - 10; } while (0); else if (microseconds < (1UL << 19)) do { time.period.m = microseconds >> (19 - 10); time.period.e = 19 - 10; } while (0);
 else if (microseconds < (1UL << 20)) do { time.period.m = microseconds >> (20 - 10); time.period.e = 20 - 10; } while (0); else if (microseconds < (1UL << 21)) do { time.period.m = microseconds >> (21 - 10); time.period.e = 21 - 10; } while (0);
 else if (microseconds < (1UL << 22)) do { time.period.m = microseconds >> (22 - 10); time.period.e = 22 - 10; } while (0); else if (microseconds < (1UL << 23)) do { time.period.m = microseconds >> (23 - 10); time.period.e = 23 - 10; } while (0);
 else if (microseconds < (1UL << 24)) do { time.period.m = microseconds >> (24 - 10); time.period.e = 24 - 10; } while (0); else if (microseconds < (1UL << 25)) do { time.period.m = microseconds >> (25 - 10); time.period.e = 25 - 10; } while (0);
 else if (microseconds < (1UL << 26)) do { time.period.m = microseconds >> (26 - 10); time.period.e = 26 - 10; } while (0); else if (microseconds < (1UL << 27)) do { time.period.m = microseconds >> (27 - 10); time.period.e = 27 - 10; } while (0);
 else if (microseconds < (1UL << 28)) do { time.period.m = microseconds >> (28 - 10); time.period.e = 28 - 10; } while (0); else if (microseconds < (1UL << 29)) do { time.period.m = microseconds >> (29 - 10); time.period.e = 29 - 10; } while (0);
 else if (microseconds < (1UL << 30)) do { time.period.m = microseconds >> (30 - 10); time.period.e = 30 - 10; } while (0); else if (microseconds < (1UL << 31)) do { time.period.m = microseconds >> (31 - 10); time.period.e = 31 - 10; } while (0);
 else
     return ((L4_Time_t) { raw : 0UL });
    } else {
 L4_Word_t l4_exp = 0;
 L4_Word_t man = microseconds;
 while (man >= (1 << 10)) {
     man >>= 1;
     l4_exp++;
 }
 if (l4_exp <= 31)
     do { time.period.m = man; time.period.e = l4_exp; } while (0);
 else
     return ((L4_Time_t) { raw : 0UL });
    }
    return time;
}
