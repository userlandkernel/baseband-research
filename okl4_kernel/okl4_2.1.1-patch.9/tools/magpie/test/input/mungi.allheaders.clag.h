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
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int64_t;
typedef unsigned long uint64_t;
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef uint64_t uintptr_t;
typedef unsigned int uint;
typedef int8_t apdpos_t;
typedef uintptr_t passwd_t;
typedef uintptr_t slabel_t;
static inline L4_Word_t * __L4_IA64_Utcb (void) __attribute__ ((const));
static inline L4_Word_t * __L4_IA64_Utcb (void)
{
    L4_Word_t * utcb;
    __asm__ (
 "/* __L4_IA64_Utcb() */			\n"
 "	mov %0 = ar.k6			\n"
 :
 "=r" (utcb)
 );
    return utcb;
}
static inline L4_Word_t __L4_TCR_MyGlobalId (void)
{
    L4_Word_t tid;
    __asm__ (
 "/* __L4_TCR_MyGlobalId() */		\n"
 "	mov %0 = ar.k5			\n"
 :
 "=r" (tid)
 );
    return tid;
}
static inline L4_Word_t __L4_TCR_MyLocalId (void)
{
    L4_Word_t *dummy = __L4_IA64_Utcb ();
    return (L4_Word_t) dummy;
}
static inline L4_Word_t __L4_TCR_ProcessorNo (void)
{
    return (__L4_IA64_Utcb ())[(1)];
}
static inline L4_Word_t __L4_TCR_UserDefinedHandle (void)
{
    return (__L4_IA64_Utcb ())[(2)];
}
static inline void __L4_TCR_Set_UserDefinedHandle (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(2)] = w;
}
static inline L4_Word_t __L4_TCR_Pager (void)
{
    return (__L4_IA64_Utcb ())[(3)];
}
static inline void __L4_TCR_Set_Pager (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(3)] = w;
}
static inline L4_Word_t __L4_TCR_ExceptionHandler (void)
{
    return (__L4_IA64_Utcb ())[(4)];
}
static inline void __L4_TCR_Set_ExceptionHandler (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(4)] = w;
}
static inline L4_Word_t __L4_TCR_ErrorCode (void)
{
    return (__L4_IA64_Utcb ())[(9)];
}
static inline L4_Word_t __L4_TCR_XferTimeout (void)
{
    return (__L4_IA64_Utcb ())[(6)];
}
static inline void __L4_TCR_Set_XferTimeout (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(6)] = w;
}
static inline L4_Word_t __L4_TCR_IntendedReceiver(void)
{
    return (__L4_IA64_Utcb ())[(7)];
}
static inline L4_Word_t __L4_TCR_ActualSender (void)
{
    return (__L4_IA64_Utcb ())[(8)];
}
static inline void __L4_TCR_Set_VirtualSender (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(8)] = w;
}
static inline L4_Word_t __L4_TCR_ThreadWord (L4_Word_t n)
{
    return (__L4_IA64_Utcb ())[(43) + n];
}
static inline void __L4_TCR_Set_ThreadWord (L4_Word_t n, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(43) + n] = w;
}
static inline void L4_Set_CopFlag (L4_Word_t n)
{
    L4_Word8_t old, readval;
    do {
 old = *(L4_Word8_t *) &(__L4_IA64_Utcb ())[(5)];
 __asm__ __volatile__ (
     "	mov	r14 = ar.ccv			\n"
     "	mov	ar.ccv = %2 ;;			\n"
     "	cmpxchg1.acq %0 = [%1], %3, ar.ccv ;;	\n"
     "	mov	ar.ccv = r14			\n"
     :
     "=r" (readval)
     :
     "r" (&(__L4_IA64_Utcb ())[(5)]),
     "r" (old),
     "r" (old | (1 << n))
     :
     "r14");
    } while (old != readval);
}
static inline void L4_Clr_CopFlag (L4_Word_t n)
{
    L4_Word8_t old, readval;
    do {
 old = *(L4_Word8_t *) &(__L4_IA64_Utcb ())[(5)];
 __asm__ __volatile__ (
     "	mov	r14 = ar.ccv			\n"
     "	mov	ar.ccv = %2 ;;			\n"
     "	cmpxchg1.acq %0 = [%1], %3, ar.ccv ;;	\n"
     "	mov	ar.ccv = r14			\n"
     :
     "=r" (readval)
     :
     "r" (&(__L4_IA64_Utcb ())[(5)]),
     "r" (old),
     "r" (old & ~(1 << n))
     :
     "r14");
    } while (old != readval);
}
static inline L4_Bool_t L4_EnablePreemptionFaultException (void)
{
    return 0;
}
static inline L4_Bool_t L4_DisablePreemptionFaultException (void)
{
    return 0;
}
static inline L4_Bool_t L4_EnablePreemption (void)
{
    return 0;
}
static inline L4_Bool_t L4_DisablePreemption (void)
{
    return 0;
}
static inline L4_Bool_t L4_PreemptionPending (void)
{
    return 0;
}
static inline void L4_StoreMR (int i, L4_Word_t * w)
{
    *w = (__L4_IA64_Utcb ())[(48) + i];
}
static inline void L4_LoadMR (int i, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(48) + i] = w;
}
static inline void L4_StoreMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + (48) + i;
    while (k-- > 0)
 *w++ = *mr++;
}
static inline void L4_LoadMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + (48) + i;
    while (k-- > 0)
 *mr++ = *w++;
}
static inline void L4_StoreBR (int i, L4_Word_t * w)
{
    *w = (__L4_IA64_Utcb ())[(10) + i];
}
static inline void L4_LoadBR (int i, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[(10) + i] = w;
}
static inline void L4_StoreBRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + (10) + i;
    while (k-- > 0)
 *w++ = *mr++;
}
static inline void L4_LoadBRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + (10) + i;
    while (k-- > 0)
 *mr++ = *w++;
}
typedef union {
    L4_Word_t raw;
    struct {
 L4_Word_t u:6;
 L4_Word_t t:6;
 L4_Word_t flags:4;
 L4_Word_t label:16 + 32;
    } X;
} L4_MsgTag_t;
static inline L4_Bool_t L4_IsMsgTagEqual (const L4_MsgTag_t l,
          const L4_MsgTag_t r)
{
    return l.raw == r.raw;
}
static inline L4_Bool_t L4_IsMsgTagNotEqual (const L4_MsgTag_t l,
      const L4_MsgTag_t r)
{
    return l.raw != r.raw;
}
static inline L4_MsgTag_t L4_MsgTagAddLabel (const L4_MsgTag_t t, int label)
{
    L4_MsgTag_t tag = t;
    tag.X.label = label;
    return tag;
}
static inline L4_MsgTag_t L4_MsgTagAddLabelTo (L4_MsgTag_t * t, int label)
{
    t->X.label = label;
    return *t;
}
static inline L4_Word_t L4_Label (L4_MsgTag_t t)
{
    return t.X.label;
}
static inline L4_Word_t L4_UntypedWords (L4_MsgTag_t t)
{
    return t.X.u;
}
static inline L4_Word_t L4_TypedWords (L4_MsgTag_t t)
{
    return t.X.t;
}
static inline void L4_Set_Label (L4_MsgTag_t * t, L4_Word_t label)
{
    t->X.label = label;
}
static inline L4_MsgTag_t L4_MsgTag (void)
{
    L4_MsgTag_t msgtag;
    L4_StoreMR (0, &msgtag.raw);
    return msgtag;
}
static inline void L4_Set_MsgTag (L4_MsgTag_t t)
{
    L4_LoadMR (0, t.raw);
}
typedef union {
    L4_Word_t raw[2];
    struct {
 L4_Word_t C:1;
 L4_Word_t __type:3;
 L4_Word_t __zeros:6;
 L4_Word_t snd_base:22 + 32;
 L4_Fpage_t snd_fpage;
    } X;
} L4_MapItem_t;
static inline L4_MapItem_t L4_MapItem (L4_Fpage_t f, L4_Word_t SndBase)
{
    L4_MapItem_t item;
    item.raw[0] = 0;
    item.X.__type = 0x04;
    item.X.snd_base = SndBase >> 10;
    item.X.snd_fpage.raw = f.raw;
    return item;
}
static inline L4_Bool_t L4_IsMapItem (L4_MapItem_t m)
{
    return m.X.__type == 0x04;
}
static inline L4_Fpage_t L4_MapItemSndFpage (L4_MapItem_t m)
{
    return m.X.snd_fpage;
}
static inline L4_Word_t L4_MapItemSndBase (L4_MapItem_t m)
{
    return m.X.snd_base << 10;
}
typedef union {
    L4_Word_t raw[2];
    struct {
 L4_Word_t C:1;
 L4_Word_t __type:3;
 L4_Word_t __zeros:6;
 L4_Word_t snd_base:22 + 32;
 L4_Fpage_t snd_fpage;
    } X;
} L4_GrantItem_t;
static inline L4_Bool_t L4_IsGrantItem (L4_GrantItem_t g)
{
    return g.X.__type == 0x05;
}
static inline L4_GrantItem_t L4_GrantItem (L4_Fpage_t f, L4_Word_t SndBase)
{
    L4_GrantItem_t item;
    item.raw[0] = 0;
    item.X.__type = 0x05;
    item.X.snd_base = SndBase >> 10;
    item.X.snd_fpage.raw = f.raw;
    return item;
}
static inline L4_Fpage_t L4_GrantItemSndFpage (L4_GrantItem_t m)
{
    return m.X.snd_fpage;
}
static inline L4_Word_t L4_GrantItemSndBase (L4_GrantItem_t m)
{
    return m.X.snd_base << 10;
}
typedef union {
    L4_Word_t raw[2];
    struct {
 L4_Word_t C:1;
 L4_Word_t __type:3;
 L4_Word_t j:5;
 L4_Word_t c:1;
 L4_Word_t string_length:22 + 32;
 union {
     void * string_ptr;
     void * substring_ptr[0];
 } str;
    } X;
} L4_StringItem_t;
static inline L4_StringItem_t L4_StringItem (int size, void * address)
{
    L4_StringItem_t item;
    item.X.C = 0;
    item.X.__type = 0;
    item.X.j = 0;
    item.X.c = 0;
    item.X.string_length = size;
    item.X.str.string_ptr = address;
    return item;
}
static inline L4_Bool_t L4_IsStringItem (L4_StringItem_t * s)
{
    return (s->X.__type & 0x04) == 0;
}
static inline L4_Bool_t L4_CompoundString (L4_StringItem_t * s)
{
    return s->X.c != 0;
}
static inline L4_Word_t L4_Substrings (L4_StringItem_t * s)
{
    return s->X.j + 1;
}
static inline void * L4_Substring (L4_StringItem_t * s, L4_Word_t n)
{
    return s->X.str.substring_ptr[n - 1];
}
static inline L4_StringItem_t * __L4_EndOfString (L4_StringItem_t * s,
           L4_StringItem_t ** p)
{
    L4_StringItem_t *prev;
    do {
 prev = s;
 s = (L4_StringItem_t *) &s->X.str.substring_ptr[s->X.j+1];
    } while (prev->X.c);
    if (p)
 *p = prev;
    return s;
}
static inline L4_StringItem_t * __L4_EndOfStrings (L4_StringItem_t * s,
            L4_StringItem_t ** p)
{
    L4_StringItem_t *prev;
    do {
 prev = s;
 s = __L4_EndOfString (s, (L4_StringItem_t **) 0);
    } while (prev->X.C);
    if (p)
 *p = prev;
    return s;
}
static inline void __L4_Copy_String (L4_StringItem_t * d,
     L4_StringItem_t * s)
{
    L4_Word_t *dest, *from, *end;
    from = (L4_Word_t *) s;
    dest = (L4_Word_t *) d;
    end = (L4_Word_t *) __L4_EndOfString (s, (L4_StringItem_t **) 0);
    while (from < end)
 *dest++ = *from++;
}
static inline L4_StringItem_t * L4_AddSubstringTo (L4_StringItem_t * dest,
            L4_StringItem_t * substr)
{
    L4_StringItem_t *prev;
    dest = __L4_EndOfString (dest, &prev);
    prev->X.c = 1;
    __L4_Copy_String (dest, substr);
    return dest;
}
static inline L4_StringItem_t * L4_AddSubstringAddressTo (L4_StringItem_t * dest,
            void * substring_addr)
{
    L4_StringItem_t *last;
    __L4_EndOfString (dest, &last);
    last->X.j++;
    last->X.str.substring_ptr[last->X.j] = substring_addr;
    return dest;
}
typedef struct L4_CacheAllocationHint {
    L4_Word_t raw;
} L4_CacheAllocationHint_t;
static inline L4_CacheAllocationHint_t L4_CacheAlloctionHint (
    const L4_StringItem_t s)
{
    L4_CacheAllocationHint_t hint;
    hint.raw = s.raw[0] & 0x6;
    return hint;
}
static inline L4_Bool_t L4_IsCacheAllocationHintEqual (
    const L4_CacheAllocationHint_t l, const L4_CacheAllocationHint_t r)
{
    return (l.raw & 0x6) == (r.raw & 0x6);
}
static inline L4_Bool_t L4_IsCacheAllocationHintNotEqual (
    const L4_CacheAllocationHint_t l, const L4_CacheAllocationHint_t r)
{
    return (l.raw & 0x6) != (r.raw & 0x6);
}
static inline L4_StringItem_t L4_AddCacheAllocationHint (
    const L4_StringItem_t dest, const L4_CacheAllocationHint_t h)
{
    L4_StringItem_t item = dest;
    item.raw[0] |= (h.raw & 0x6);
    return item;
}
static inline L4_StringItem_t L4_AddCacheAllocationHintTo (
    L4_StringItem_t * dest, const L4_CacheAllocationHint_t h)
{
    dest->raw[0] |= (h.raw & 0x6);
    return *dest;
}
typedef union {
    L4_Word_t raw[64];
    L4_Word_t msg[64];
    L4_MsgTag_t tag;
} L4_Msg_t;
static inline void L4_MsgPut (L4_Msg_t * msg, L4_Word_t label,
     int u, L4_Word_t * Untyped,
     int t, void * Items)
{
    int i;
    for (i = 0; i < u; i++)
 msg->msg[i+1] = Untyped[i];
    for (i = 0; i < t; i++)
 msg->msg[i+u+1] = ((L4_Word_t *) Items)[i];
    msg->tag.X.label = label;
    msg->tag.X.flags = 0;
    msg->tag.X.u = u;
    msg->tag.X.t = t;
}
static inline void L4_MsgGet (const L4_Msg_t * msg, L4_Word_t * Untyped,
     void * Items)
{
    int i, u, t;
    u = msg->tag.X.u;
    t = msg->tag.X.t;
    for (i = 0; i < u; i++)
 Untyped[i] = msg->msg[i + 1];
    for (i = 0; i < t; i++)
 ((L4_Word_t *) Items)[i] = msg->msg[i + 1 + u];
}
static inline L4_MsgTag_t L4_MsgMsgTag (const L4_Msg_t * msg)
{
    return msg->tag;
}
static inline void L4_Set_MsgMsgTag (L4_Msg_t * msg, L4_MsgTag_t t)
{
    msg->tag = t;
}
static inline L4_Word_t L4_MsgLabel (const L4_Msg_t * msg)
{
    return msg->tag.X.label;
}
static inline void L4_Set_MsgLabel (L4_Msg_t * msg, L4_Word_t label)
{
    msg->tag.X.label = label;
}
static inline void L4_MsgLoad (L4_Msg_t * msg)
{
    L4_LoadMRs (0, msg->tag.X.u + msg->tag.X.t + 1, &msg->msg[0]);
}
static inline void L4_MsgStore (L4_MsgTag_t t, L4_Msg_t * msg)
{
    L4_StoreMRs (1, t.X.u + t.X.t, &msg->msg[1]);
    msg->tag = t;
}
static inline void L4_MsgClear (L4_Msg_t * msg)
{
    msg->msg[0] = 0;
}
static inline void L4_MsgAppendWord (L4_Msg_t * msg, L4_Word_t w)
{
    if (msg->tag.X.t)
    {
 L4_Word_t i = 1 + msg->tag.X.u + msg->tag.X.t;
 for (; i > (L4_Word_t)(msg->tag.X.u + 1); i--)
     msg->msg[i] = msg->msg[i-1];
    }
    msg->msg[++msg->tag.X.u] = w;
}
static inline void L4_MsgAppendMapItem (L4_Msg_t * msg, L4_MapItem_t m)
{
    msg->msg[msg->tag.X.u + msg->tag.X.t + 1] = m.raw[0];
    msg->msg[msg->tag.X.u + msg->tag.X.t + 2] = m.raw[1];
    msg->tag.X.t += 2;
}
static inline void L4_MsgAppendGrantItem (L4_Msg_t * msg, L4_GrantItem_t g)
{
    msg->msg[msg->tag.X.u + msg->tag.X.t + 1] = g.raw[0];
    msg->msg[msg->tag.X.u + msg->tag.X.t + 2] = g.raw[1];
    msg->tag.X.t += 2;
}
static inline void L4_MsgAppendSimpleStringItem (L4_Msg_t * msg, L4_StringItem_t s)
{
    s.X.c = 0;
    s.X.j = 0;
    msg->msg[msg->tag.X.u + msg->tag.X.t + 1] = s.raw[0];
    msg->msg[msg->tag.X.u + msg->tag.X.t + 2] = s.raw[1];
    msg->tag.X.t += 2;
}
static inline void L4_MsgAppendStringItem (L4_Msg_t * msg,
           L4_StringItem_t * s)
{
    L4_StringItem_t *d = (L4_StringItem_t *)
 &msg->msg[msg->tag.X.u + msg->tag.X.t + 1];
    L4_StringItem_t *e = __L4_EndOfString (s, (L4_StringItem_t **) 0);
    int size = ((L4_Word_t) e - (L4_Word_t) s) / sizeof (L4_Word_t);
    __L4_Copy_String (d, s);
    msg->tag.X.t += size;
}
static inline void L4_MsgPutWord (L4_Msg_t * msg, L4_Word_t u, L4_Word_t w)
{
    msg->msg[u+1] = w;
}
static inline void L4_MsgPutMapItem (L4_Msg_t * msg, L4_Word_t t, L4_MapItem_t m)
{
    msg->msg[msg->tag.X.u + t + 1] = m.raw[0];
    msg->msg[msg->tag.X.u + t + 2] = m.raw[1];
}
static inline void L4_MsgPutGrantItem (L4_Msg_t * msg, L4_Word_t t,
       L4_GrantItem_t g)
{
    msg->msg[msg->tag.X.u + t + 1] = g.raw[0];
    msg->msg[msg->tag.X.u + t + 2] = g.raw[1];
}
static inline void L4_MsgPutSimpleStringItem (L4_Msg_t * msg, L4_Word_t t,
       L4_StringItem_t s)
{
    s.X.c = 0;
    s.X.j = 0;
    msg->msg[msg->tag.X.u + t + 1] = s.raw[0];
    msg->msg[msg->tag.X.u + t + 2] = s.raw[1];
}
static inline void L4_MsgPutStringItem (L4_Msg_t * msg, L4_Word_t t,
        L4_StringItem_t * s)
{
    L4_StringItem_t *d = (L4_StringItem_t *) &msg->msg[msg->tag.X.u + t + 1];
    __L4_Copy_String (d, s);
}
static inline L4_Word_t L4_MsgWord (L4_Msg_t * msg, L4_Word_t u)
{
    return msg->msg[u + 1];
}
static inline void L4_MsgGetWord (L4_Msg_t * msg, L4_Word_t u, L4_Word_t * w)
{
    *w = msg->msg[u + 1];
}
static inline L4_Word_t L4_MsgGetMapItem (L4_Msg_t * msg, L4_Word_t t,
          L4_MapItem_t * m)
{
    m->raw[0] = msg->msg[msg->tag.X.u + t + 1];
    m->raw[1] = msg->msg[msg->tag.X.u + t + 2];
    return 2;
}
static inline L4_Word_t L4_MsgGetGrantItem (L4_Msg_t * msg, L4_Word_t t,
     L4_GrantItem_t * g)
{
    g->raw[0] = msg->msg[msg->tag.X.u + t + 1];
    g->raw[1] = msg->msg[msg->tag.X.u + t + 2];
    return 2;
}
static inline L4_Word_t L4_MsgGetStringItem (L4_Msg_t * msg, L4_Word_t t,
      L4_StringItem_t * s)
{
    L4_StringItem_t *b = (L4_StringItem_t *) &msg->msg[msg->tag.X.u + t + 1];
    L4_StringItem_t *e = __L4_EndOfString (b, &s);
    __L4_Copy_String (s, b);
    return ((L4_Word_t) e - (L4_Word_t) b) / sizeof (L4_Word_t);
}
typedef union {
    L4_Word_t raw;
    struct {
 L4_Word_t s:1;
 L4_Word_t __zeros:3;
 L4_Word_t RcvWindow:28 + 32;
    } X;
} L4_Acceptor_t;
typedef union {
    L4_Word_t raw[33 -1];
    L4_StringItem_t string[0];
} L4_MsgBuffer_t;
static inline L4_Acceptor_t L4_MapGrantItems (L4_Fpage_t RcvWindow)
{
    L4_Acceptor_t a;
    a.raw = RcvWindow.raw;
    a.X.s = 1;
    a.X.__zeros = 0;
    return a;
}
static inline L4_Acceptor_t L4_AddAcceptor (const L4_Acceptor_t l,
     const L4_Acceptor_t r)
{
    L4_Acceptor_t a;
    a.raw = 0;
    a.X.s = (l.X.s | r.X.s);
    a.X.RcvWindow = (r.X.RcvWindow != 0) ? r.X.RcvWindow : l.X.RcvWindow;
    return a;
}
static inline L4_Acceptor_t L4_AddAcceptorTo (L4_Acceptor_t l,
       const L4_Acceptor_t r)
{
    l.X.s = (l.X.s | r.X.s);
    if (r.X.RcvWindow != 0)
 l.X.RcvWindow = r.X.RcvWindow;
    return l;
}
static inline L4_Acceptor_t L4_RemoveAcceptor (const L4_Acceptor_t l,
        const L4_Acceptor_t r)
{
    L4_Acceptor_t a = l;
    if (r.X.s)
 a.X.s = 0;
    if (r.X.RcvWindow != 0)
 a.X.RcvWindow = 0;
    return a;
}
static inline L4_Acceptor_t L4_RemoveAcceptorFrom (L4_Acceptor_t l,
            const L4_Acceptor_t r)
{
    if (r.X.s)
 l.X.s = 0;
    if (r.X.RcvWindow != 0)
 l.X.RcvWindow = 0;
    return l;
}
static inline L4_Bool_t L4_HasStringItems (const L4_Acceptor_t a)
{
    return (a.raw & 0x01UL) == 1;
}
static inline L4_Bool_t L4_HasMapGrantItems (const L4_Acceptor_t a)
{
    return (a.raw & ~0x0fUL) != 0;
}
static inline L4_Fpage_t L4_RcvWindow (const L4_Acceptor_t a)
{
    L4_Fpage_t fp;
    fp.raw = a.raw;
    fp.X.rwx = 0;
    return fp;
}
static inline void L4_Accept (const L4_Acceptor_t a)
{
    L4_LoadBR (0, a.raw);
}
static inline void L4_AcceptStrings (const L4_Acceptor_t a,
     const L4_MsgBuffer_t * b)
{
    L4_StringItem_t *prev, *t, *s = (L4_StringItem_t *) &b->string[0];
    int n, i = 1;
    L4_LoadBR (0, a.raw);
    do {
 prev = s;
 do {
     t = s;
     n = s->X.j + 2;
     s = (L4_StringItem_t *) &s->X.str.substring_ptr[n-1];
     L4_LoadBRs (i, n, (L4_Word_t *) t);
     i += n;
 } while (t->X.c);
    } while (prev->X.C);
}
static inline L4_Acceptor_t L4_Accepted (void)
{
    L4_Acceptor_t a;
    L4_StoreBR (0, &a.raw);
    return a;
}
static inline void L4_MsgBufferClear (L4_MsgBuffer_t * b)
{
    b->raw[0] = b->raw[2] = 0;
}
static inline void L4_MsgBufferAppendSimpleRcvString (L4_MsgBuffer_t * b,
        L4_StringItem_t s)
{
    L4_StringItem_t *prev, *tmp;
    s.X.j = 0;
    s.X.c = 0;
    if (b->raw[0] || b->raw[2]) {
 tmp = __L4_EndOfStrings ((L4_StringItem_t*)&b[0], &prev);
 prev->X.C = 1;
 *tmp = s;
    } else {
 b->raw[2] = ~0UL;
 ((L4_StringItem_t*)b)[0] = s;
 b->string[0].X.C = 0;
    }
}
static inline void L4_MsgBufferAppendRcvString (L4_MsgBuffer_t * b,
         L4_StringItem_t * s)
{
    L4_StringItem_t *prev, *tmp;
    if (b->raw[0] || b->raw[2]) {
 tmp = __L4_EndOfStrings ((L4_StringItem_t *) &b->raw[0], &prev);
 prev->X.C = 1;
    } else
 tmp = (L4_StringItem_t *) &b->raw[0];
    __L4_Copy_String (tmp, s);
    tmp->X.C = 0;
}
static inline void L4_KDB_ClearPage (void)
{
}
static inline void L4_KDB_PrintChar (char c)
{
    register char r14 asm ("r14") = c;
    asm volatile (
 "{ .mlx					\n"
 "	break.m	0x1			\n"
 "	movl	r0 = 0 ;;		\n"
 "}					\n"
 :
 :
 "r" (r14));
}
static inline void L4_KDB_PrinString (const char * s)
{
}
static inline char L4_KDB_ReadChar (void)
{
    register char r14 asm ("r14");
    asm volatile (
 "{ .mlx					\n"
 "	break.m	0x4			\n"
 "	movl	r0 = 0 ;;		\n"
 "}					\n"
 :
 "=r" (r14));
    return r14;
}
static inline char L4_KDB_ReadChar_Blocked (void)
{
    register char r14 asm ("r14");
    asm volatile (
 "{ .mlx					\n"
 "	break.m	0x2			\n"
 "	movl	r0 = 0 ;;		\n"
 "}					\n"
 :
 "=r" (r14));
    return r14;
}
static inline void * L4_KernelInterface (L4_Word_t *ApiVersion,
         L4_Word_t *ApiFlags,
         L4_Word_t *KernelId)
{
    register void * base_address asm ("r8");
    register L4_Word_t api_version asm ("r9");
    register L4_Word_t api_flags asm ("r10");
    register L4_Word_t kernel_id asm ("r11");
    __asm__ __volatile__ (
 "/* L4_KernelInterface() */ 			\n"
 "{ .mlx						\n"
 "	break.m	0x1face				\n"
 "	movl	r0 = 0x0 ;;			\n"
 "}						\n"
 :
 "=r" (base_address),
 "=r" (api_version),
 "=r" (api_flags),
 "=r" (kernel_id));
    *ApiVersion = api_version;
    *ApiFlags = api_flags;
    *KernelId = kernel_id;
    return base_address;
}
static inline L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
           L4_Word_t control,
           L4_Word_t sp,
           L4_Word_t ip,
           L4_Word_t flags,
           L4_Word_t UserDefHandle,
           L4_ThreadId_t pager,
           L4_Word_t *old_control,
           L4_Word_t *old_sp,
           L4_Word_t *old_ip,
           L4_Word_t *old_flags,
           L4_Word_t *old_UserDefHandle,
           L4_ThreadId_t *old_pager)
{
    register L4_ThreadId_t r_dest asm ("r14") = dest;
    register L4_Word_t r_control asm ("r15") = control;
    register L4_Word_t r_sp asm ("r16") = sp;
    register L4_Word_t r_ip asm ("r17") = ip;
    register L4_Word_t r_flags asm ("r18") = flags;
    register L4_Word_t r_userhandle asm ("r19") = UserDefHandle;
    register L4_ThreadId_t r_pager asm ("r20") = pager;
    __asm__ __volatile__ (
 "/* L4_ExchangeRegisters() */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_ExchangeRegisters" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "+r" (r_dest),
 "+r" (r_control),
 "+r" (r_sp),
 "+r" (r_ip),
 "+r" (r_flags),
 "+r" (r_userhandle),
 "+r" (r_pager)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r8","r9","r10","r11");
    *old_control = r_control;
    *old_sp = r_sp;
    *old_ip = r_ip;
    *old_flags = r_flags;
    *old_UserDefHandle = r_userhandle;
    *old_pager = r_pager;
    return r_dest;
}
static inline L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
          L4_ThreadId_t SpaceSpecifier,
          L4_ThreadId_t Scheduler,
          L4_ThreadId_t Pager,
          void * UtcbLocation)
{
    register L4_Word_t result asm ("r8");
    register L4_ThreadId_t r_dest asm ("r14") = dest;
    register L4_ThreadId_t r_SpaceSpecifier asm ("r15") = SpaceSpecifier;
    register L4_ThreadId_t r_Scheduler asm ("r16") = Scheduler;
    register L4_ThreadId_t r_Pager asm ("r17") = Pager;
    register void * r_UtcbLocation asm ("r18") = UtcbLocation;
    __asm__ __volatile__ (
 "/* L4_ThreadControl() */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_ThreadControl" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (result),
 "+r" (r_dest),
 "+r" (r_SpaceSpecifier),
 "+r" (r_Scheduler),
 "+r" (r_Pager),
 "+r" (r_UtcbLocation)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r9","r10","r11","r19","r20");
    return result;
}
static inline L4_Clock_t L4_SystemClock (void)
{
    register L4_Clock_t clock asm ("r8");
    __asm__ __volatile__ (
 "/* L4_SystemClock() */\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_SystemClock" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (clock.raw)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r9","r10","r11", "r14","r15","r16","r17","r18","r19","r20");
    return clock;
}
static inline void L4_ThreadSwitch (L4_ThreadId_t dest)
{
    register L4_ThreadId_t r_dest asm ("r14") = dest;
    __asm__ __volatile__ (
 "/* L4_ThreadSwitch() */\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_ThreadSwitch" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "+r" (r_dest)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r8","r9","r10","r11", "r15","r16","r17","r18","r19","r20");
}
static inline L4_Word_t L4_Schedule (L4_ThreadId_t dest,
      L4_Word_t TimeCtrl,
      L4_Word_t ProcessorCtrl,
      L4_Word_t prio,
      L4_Word_t PreemptionCtrl,
      L4_Word_t * old_TimeCtrl)
{
    register L4_Word_t result asm ("r8");
    register L4_Word_t ret_TimeCtrl asm ("r9");
    register L4_ThreadId_t r_dest asm ("r14") = dest;
    register L4_Word_t r_TimeCtrl asm ("r15") = TimeCtrl;
    register L4_Word_t r_ProcessorCtrl asm ("r16") = ProcessorCtrl;
    register L4_Word_t r_prio asm ("r17") = prio;
    register L4_Word_t r_PreemptionCtrl asm ("r18") = PreemptionCtrl;
    __asm__ __volatile__ (
 "/* L4_Schedule() */					\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_Schedule" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (result),
 "=r" (ret_TimeCtrl),
 "+r" (r_dest),
 "+r" (r_TimeCtrl),
 "+r" (r_ProcessorCtrl),
 "+r" (r_prio),
 "+r" (r_PreemptionCtrl)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r10","r11","r19","r20");
    *old_TimeCtrl = ret_TimeCtrl;
    return result;
}
static inline L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
         L4_ThreadId_t FromSpecifier,
         L4_Word_t Timeouts,
         L4_ThreadId_t * from)
{
    register L4_ThreadId_t r_to asm ("r14") = to;
    register L4_ThreadId_t r_FromSpecifier asm ("r15") = FromSpecifier;
    register L4_Word_t r_Timeouts asm ("r16") = Timeouts;
    register L4_ThreadId_t ret_from asm ("r9");
    L4_Word_t * mrs = __L4_IA64_Utcb () + (48);
    register L4_Word_t mr0 asm ("out0");
    register L4_Word_t mr1 asm ("out1");
    register L4_Word_t mr2 asm ("out2");
    register L4_Word_t mr3 asm ("out3");
    register L4_Word_t mr4 asm ("out4");
    register L4_Word_t mr5 asm ("out5");
    register L4_Word_t mr6 asm ("out6");
    register L4_Word_t mr7 asm ("out7");
    L4_MsgTag_t tag;
    L4_Word_t ar_lc, ar_ec;
    __asm__ __volatile__ ("	;;			\n"
     "	mov	%0 = ar.lc	\n"
     "	mov	%1 = ar.ec	\n"
     :
     "=r" (ar_lc), "=r" (ar_ec));
    if (! L4_IsNilThread (to))
    {
 r_to = to;
 r_FromSpecifier = FromSpecifier;
 r_Timeouts = Timeouts;
 mr0 = mrs[0];
 mr1 = mrs[1];
 mr2 = mrs[2];
 mr3 = mrs[3];
 mr4 = mrs[4];
 mr5 = mrs[5];
 mr6 = mrs[6];
 mr7 = mrs[7];
 __asm__ __volatile__ (
     "/* L4_Ipc() */\n"
     "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_Ipc" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
     :
     "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
     "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
     "=r" (ret_from),
     "+r" (r_to), "+r" (r_FromSpecifier), "+r" (r_Timeouts)
     :
     "0" (mr0), "1" (mr1), "2" (mr2), "3" (mr3),
     "4" (mr4), "5" (mr5), "6" (mr6), "7" (mr7)
     :
     "r2", "r3", "r8", "r10", "r11",
     "r17", "r18", "r19", "r20", "r21", "r22",
     "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
     "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",
     "b0", "b6", "b7", "r4", "r5", "r6", "r7", "f2", "f3", "f4", "f5", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f23", "f24", "f25", "f26", "f27", "f29", "f30", "f31", "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29", "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", "p40", "p41", "p42", "p43", "p44", "p45", "p46", "p47", "p48", "p49", "p50", "p51", "p52", "p53", "p54", "p55", "p56", "p57", "p58", "p59", "p60", "p61", "p62", "p63", "b1", "b2", "b3", "b4", "b5");
    }
    else
    {
 r_to = to;
 r_FromSpecifier = FromSpecifier;
 r_Timeouts = Timeouts;
 __asm__ __volatile__ (
     "/* L4_Ipc() */\n"
     "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_Ipc" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
     :
     "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
     "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
     "=r" (ret_from),
     "+r" (r_to), "+r" (r_FromSpecifier), "+r" (r_Timeouts)
     :
     :
     "r2", "r3", "r8", "r10", "r11",
     "r17", "r18", "r19", "r20", "r21", "r22",
     "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
     "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",
     "b0", "b6", "b7", "r4", "r5", "r6", "r7", "f2", "f3", "f4", "f5", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f23", "f24", "f25", "f26", "f27", "f29", "f30", "f31", "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29", "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", "p40", "p41", "p42", "p43", "p44", "p45", "p46", "p47", "p48", "p49", "p50", "p51", "p52", "p53", "p54", "p55", "p56", "p57", "p58", "p59", "p60", "p61", "p62", "p63", "b1", "b2", "b3", "b4", "b5");
    }
    if (! L4_IsNilThread (FromSpecifier))
    {
 *from = ret_from;
 mrs[1] = mr1;
 mrs[2] = mr2;
 mrs[3] = mr3;
 mrs[4] = mr4;
 mrs[5] = mr5;
 mrs[6] = mr6;
 mrs[7] = mr7;
    }
    __asm__ __volatile__ ("	mov	ar.lc = %0	\n"
     "	mov	ar.ec = %1	\n"
     :
     :
     "r" (ar_lc), "r" (ar_ec));
    tag.raw = mr0;
    return tag;
}
static inline L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
          L4_ThreadId_t FromSpecifier,
          L4_Word_t Timeouts,
          L4_ThreadId_t * from)
{
    return L4_Ipc (to, FromSpecifier, Timeouts, from);
}
static inline void L4_Unmap (L4_Word_t control)
{
    register L4_Word_t r_control asm ("r14") = control;
    register L4_Word_t mr0 asm ("out0");
    register L4_Word_t mr1 asm ("out1");
    register L4_Word_t mr2 asm ("out2");
    register L4_Word_t mr3 asm ("out3");
    register L4_Word_t mr4 asm ("out4");
    register L4_Word_t mr5 asm ("out5");
    register L4_Word_t mr6 asm ("out6");
    register L4_Word_t mr7 asm ("out7");
    L4_Word_t * mrs = __L4_IA64_Utcb () + (48);
    mr0 = mrs[0];
    mr1 = mrs[1];
    mr2 = mrs[2];
    mr3 = mrs[3];
    mr4 = mrs[4];
    mr5 = mrs[5];
    mr6 = mrs[6];
    mr7 = mrs[7];
    __asm__ __volatile__ (
 "/* L4_Unmap() */\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_Unmap" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
 "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
 "+r" (r_control)
 :
 "0" (mr0), "1" (mr1), "2" (mr2), "3" (mr3),
 "4" (mr4), "5" (mr5), "6" (mr6), "7" (mr7)
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r8", "r9", "r10", "r11", "r15", "r16", "r17", "r18", "r19", "r20");
    mrs[0] = mr0;
    mrs[1] = mr1;
    mrs[2] = mr2;
    mrs[3] = mr3;
    mrs[4] = mr4;
    mrs[5] = mr5;
    mrs[6] = mr6;
    mrs[7] = mr7;
}
static inline L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
         L4_Word_t control,
         L4_Fpage_t KernelInterfacePageArea,
         L4_Fpage_t UtcbArea,
         L4_ThreadId_t redirector,
         L4_Word_t *old_control)
{
    register L4_Word_t result asm ("r8");
    register L4_Word_t ret_control asm ("r9");
    register L4_ThreadId_t r_space asm ("r14") = SpaceSpecifier;
    register L4_Word_t r_control asm ("r15") = control;
    register L4_Fpage_t r_kiparea asm ("r16") = KernelInterfacePageArea;
    register L4_Fpage_t r_utcbarea asm ("r17") = UtcbArea;
    register L4_ThreadId_t r_redirector asm ("r18") = redirector;
    __asm__ __volatile__ (
 "/* L4_SpaceControl() */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_SpaceControl" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (result),
 "=r" (ret_control),
 "+r" (r_space),
 "+r" (r_control),
 "+r" (r_kiparea),
 "+r" (r_utcbarea),
 "+r" (r_redirector)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r10","r11","r19","r20");
    *old_control = ret_control;
    return result;
}
static inline L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
      L4_Word_t InternalFrequency,
      L4_Word_t ExternalFrequency,
      L4_Word_t voltage)
{
    do { __asm__ __volatile__ ( "{ .mlx					\n" "	break.m	0x3			\n" "	movl	r0 = 1f ;;		\n" "}					\n" "					\n" "	.rodata				\n" "1:	stringz " "\"ProcessorControl\"" "			\n" "	.previous			\n" : : : "memory"); } while (0);
    return 0;
}
static inline L4_Word_t L4_MemoryControl (L4_Word_t control,
          const L4_Word_t * attributes)
{
    register L4_Word_t result asm ("r8");
    register L4_Word_t r_ctrl asm ("r14") = control;
    register L4_Word_t r_att0 asm ("r15") = attributes[0];
    register L4_Word_t r_att1 asm ("r16") = attributes[1];
    register L4_Word_t r_att2 asm ("r17") = attributes[2];
    register L4_Word_t r_att3 asm ("r18") = attributes[3];
    register L4_Word_t mr0 asm ("out0");
    register L4_Word_t mr1 asm ("out1");
    register L4_Word_t mr2 asm ("out2");
    register L4_Word_t mr3 asm ("out3");
    register L4_Word_t mr4 asm ("out4");
    register L4_Word_t mr5 asm ("out5");
    register L4_Word_t mr6 asm ("out6");
    register L4_Word_t mr7 asm ("out7");
    L4_Word_t * mrs = __L4_IA64_Utcb () + (48);
    mr0 = mrs[0];
    mr1 = mrs[1];
    mr2 = mrs[2];
    mr3 = mrs[3];
    mr4 = mrs[4];
    mr5 = mrs[5];
    mr6 = mrs[6];
    mr7 = mrs[7];
    __asm__ __volatile__ (
 "/* L4_MemoryControl */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_MemoryControl" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (result),
 "+r" (mr0), "+r" (mr1), "+r" (mr2), "+r" (mr3),
 "+r" (mr4), "+r" (mr5), "+r" (mr6), "+r" (mr7),
 "+r" (r_ctrl),
 "+r" (r_att0),
 "+r" (r_att1),
 "+r" (r_att2),
 "+r" (r_att3)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r9", "r10", "r11", "r19","r20");
    return result;
}
static inline L4_Word_t L4_PAL_Call (L4_Word_t idx,
     L4_Word_t a1, L4_Word_t a2, L4_Word_t a3,
     L4_Word_t *r1, L4_Word_t *r2, L4_Word_t *r3)
{
    register L4_Word_t status asm ("r8");
    register L4_Word_t ret1 asm ("r9");
    register L4_Word_t ret2 asm ("r10");
    register L4_Word_t ret3 asm ("r11");
    register L4_Word_t out0 asm ("r28") = idx;
    register L4_Word_t out1 asm ("r29") = a1;
    register L4_Word_t out2 asm ("r30") = a2;
    register L4_Word_t out3 asm ("r31") = a3;
    __asm__ __volatile__ (
 "/* L4_PAL_Call */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_PAL_Call" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (status),
 "=r" (ret1),
 "=r" (ret2),
 "=r" (ret3),
 "+r" (out0),
 "+r" (out1),
 "+r" (out2),
 "+r" (out3)
 :
 :
 "r2", "r3", "r14", "r15", "r16", "r17", "r18", "r19",
 "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
 "out4", "out5", "out6", "out7",
 "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",
 "b0", "b6", "b7", "ar.pfs");
    *r1 = ret1;
    *r2 = ret2;
    *r3 = ret3;
    return status;
}
static inline L4_Word_t L4_SAL_Call (L4_Word_t idx,
     L4_Word_t a1, L4_Word_t a2, L4_Word_t a3,
     L4_Word_t a4, L4_Word_t a5, L4_Word_t a6,
     L4_Word_t *r1, L4_Word_t *r2, L4_Word_t *r3)
{
    register L4_Word_t status asm ("r8");
    register L4_Word_t ret1 asm ("r9");
    register L4_Word_t ret2 asm ("r10");
    register L4_Word_t ret3 asm ("r11");
    register L4_Word_t out0 asm ("out0") = idx;
    register L4_Word_t out1 asm ("out1") = a1;
    register L4_Word_t out2 asm ("out2") = a2;
    register L4_Word_t out3 asm ("out3") = a3;
    register L4_Word_t out4 asm ("out4") = a4;
    register L4_Word_t out5 asm ("out5") = a5;
    register L4_Word_t out6 asm ("out6") = a6;
    __asm__ __volatile__ (
 "/* L4_SAL_Call */				\n"
 "	add	r31 = -40, sp			\n" "	add	r30 = -32, sp			\n" "	add	sp  = -40, sp			\n" "	mov	r29 = ar.unat			\n" "	mov	r28 = rp			\n" "	mov	r27 = ar.pfs			\n" "	mov	r26 = ar.fpsr			\n" "	;;					\n" "	st8	[r31] = r29, 16			\n" "	st8	[r30] = r28, 16			\n" "	;;					\n" "	st8	[r31] = r27			\n" "	st8	[r30] = r26			\n" "						\n" "	br.call.sptk.few rp = " "__L4_SAL_Call" "		\n" "						\n" "      	mov	r31 = sp			\n" "	add	r30 = 8, sp			\n" "	;;					\n" "	ld8	r29 = [r31], 16			\n" "	ld8	r28 = [r30], 16			\n" "	;;					\n" "	ld8	r27 = [r31]			\n" "	ld8	r26 = [r30]			\n" "	;;					\n" "	mov	ar.unat = r29			\n" "	mov	rp = r28			\n" "	mov	ar.pfs = r27			\n" "	mov	ar.fpsr = r26			\n" "	add	sp = 40, sp			\n" "	;;					\n"
 :
 "=r" (status),
 "=r" (ret1),
 "=r" (ret2),
 "=r" (ret3),
 "+r" (out0),
 "+r" (out1),
 "+r" (out2),
 "+r" (out3),
 "+r" (out4),
 "+r" (out5),
 "+r" (out6)
 :
 :
 "r2", "r3", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", "f6", "f7", "f8", "f9", "f11", "f12", "f13", "f14", "f15", "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39", "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", "f100", "f101", "f102", "f103", "f104", "f105", "f106", "f107", "f108", "f109", "f110", "f111", "f112", "f113", "f114", "f115", "f116", "f117", "f118", "f119", "f120", "f121", "f122", "f123", "f124", "f125", "f126", "f127", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "b0", "b6", "b7", "ar.pfs" ,"r14", "r15", "r16", "r17", "r18", "r19", "r20", "out7");
    *r1 = ret1;
    *r2 = ret2;
    *r3 = ret3;
    return status;
}
typedef struct L4_ThreadState {
    L4_Word_t raw;
} L4_ThreadState_t;
static inline L4_Bool_t L4_ThreadWasHalted (L4_ThreadState_t s)
{
    return s.raw & (1 << 0);
}
static inline L4_Bool_t L4_ThreadWasReceiving (L4_ThreadState_t s)
{
    return s.raw & (1 << 1);
}
static inline L4_Bool_t L4_ThreadWasSending (L4_ThreadState_t s)
{
    return s.raw & (1 << 2);
}
static inline L4_Bool_t L4_ThreadWasIpcing (L4_ThreadState_t s)
{
    return s.raw & (3 << 1);
}
static inline L4_ThreadId_t L4_MyGlobalId (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_MyGlobalId ();
    return id;
}
static inline L4_ThreadId_t L4_MyLocalId (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_MyLocalId ();
    return id;
}
static inline L4_ThreadId_t L4_Myself (void)
{
    return L4_MyGlobalId ();
}
static inline int L4_ProcessorNo (void)
{
    return __L4_TCR_ProcessorNo ();
}
static inline L4_Word_t L4_UserDefinedHandle (void)
{
    return __L4_TCR_UserDefinedHandle ();
}
static inline void L4_Set_UserDefinedHandle (L4_Word_t NewValue)
{
    __L4_TCR_Set_UserDefinedHandle (NewValue);
}
static inline L4_ThreadId_t L4_Pager (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_Pager ();
    return id;
}
static inline void L4_Set_Pager (L4_ThreadId_t NewPager)
{
    __L4_TCR_Set_Pager (NewPager.raw);
}
static inline L4_ThreadId_t L4_ExceptionHandler (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_ExceptionHandler ();
    return id;
}
static inline void L4_Set_ExceptionHandler (L4_ThreadId_t NewHandler)
{
    __L4_TCR_Set_ExceptionHandler (NewHandler.raw);
}
static inline L4_Word_t L4_ErrorCode (void)
{
    return __L4_TCR_ErrorCode ();
}
static inline L4_Word_t L4_XferTimeouts (void)
{
    return __L4_TCR_XferTimeout ();
}
static inline void L4_Set_XferTimeouts (L4_Word_t NewValue)
{
    __L4_TCR_Set_XferTimeout (NewValue);
}
static inline L4_ThreadId_t L4_IntendedReceiver (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_IntendedReceiver ();
    return id;
}
static inline L4_ThreadId_t L4_ActualSender (void)
{
    L4_ThreadId_t id;
    id.raw = __L4_TCR_ActualSender ();
    return id;
}
static inline void L4_Set_VirtualSender (L4_ThreadId_t t)
{
    __L4_TCR_Set_VirtualSender (t.raw);
}
static inline L4_ThreadId_t L4_GlobalIdOf (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    if (L4_IsLocalId (t))
 return L4_ExchangeRegisters (t, 0, 0, 0, 0, 0, ((L4_ThreadId_t) { raw : 0UL}),
         &dummy, &dummy, &dummy, &dummy, &dummy,
         &dummy_id);
    else
 return t;
}
static inline L4_Bool_t L4_SameThreads (L4_ThreadId_t l, L4_ThreadId_t r)
{
    return L4_GlobalIdOf (l).raw == L4_GlobalIdOf (r).raw;
}
static inline L4_ThreadId_t L4_LocalIdOf (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    if (L4_IsGlobalId (t))
 return L4_ExchangeRegisters (t, 0, 0, 0, 0, 0, ((L4_ThreadId_t) { raw : 0UL}),
         &dummy, &dummy, &dummy, &dummy, &dummy,
         &dummy_id);
    else
 return t;
}
static inline L4_Word_t L4_UserDefinedHandleOf (L4_ThreadId_t t)
{
    L4_Word_t dummy, handle;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (1 << 9), 0, 0, 0, 0, ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &handle,
     &dummy_id);
    return handle;
}
static inline void L4_Set_UserDefinedHandleOf (L4_ThreadId_t t, L4_Word_t handle)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (1 << 6), 0, 0, 0, handle, ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &dummy_id);
}
static inline L4_ThreadId_t L4_PagerOf (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t pager;
    (void) L4_ExchangeRegisters (t, (1 << 9), 0, 0, 0, 0, ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &pager);
    return pager;
}
static inline void L4_Set_PagerOf (L4_ThreadId_t t, L4_ThreadId_t p)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (1 << 7), 0, 0, 0, 0, p,
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &dummy_id);
}
static inline void L4_Start (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (1 << 8) + 6, 0, 0, 0, 0, ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &dummy_id);
}
static inline void L4_Start_SpIp (L4_ThreadId_t t, L4_Word_t sp, L4_Word_t ip)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (3 << 3) + (1 << 8) + 6, sp, ip, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &dummy_id);
}
static inline void L4_Start_SpIpFlags (L4_ThreadId_t t, L4_Word_t sp,
       L4_Word_t ip, L4_Word_t flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    (void) L4_ExchangeRegisters (t, (7 << 3) + (1 << 8) + 6, sp, ip, flags, 0,
     ((L4_ThreadId_t) { raw : 0UL}),
     &dummy, &dummy, &dummy, &dummy, &dummy,
     &dummy_id);
}
static inline L4_ThreadState_t L4_Stop (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 1 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, &dummy, &dummy,
     &dummy, &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_Stop_SpIpFlags (L4_ThreadId_t t,
           L4_Word_t *sp,
           L4_Word_t *ip,
           L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 1 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, sp, ip, flags,
     &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortReceive_and_stop (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 3 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, &dummy, &dummy,
     &dummy, &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortReceive_and_stop_SpIpFlags (
    L4_ThreadId_t t,
    L4_Word_t *sp,
    L4_Word_t *ip,
    L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 3 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, sp, ip, flags,
     &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortSend_and_stop (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 5 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, &dummy, &dummy,
     &dummy, &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortSend_and_stop_SpIpFlags (
    L4_ThreadId_t t,
    L4_Word_t *sp,
    L4_Word_t *ip,
    L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 5 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, sp, ip, flags,
     &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortIpc_and_stop (L4_ThreadId_t t)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 7 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, &dummy, &dummy,
     &dummy, &dummy, &dummy_id);
    return state;
}
static inline L4_ThreadState_t L4_AbortIpc_and_stop_SpIpFlags (L4_ThreadId_t t,
          L4_Word_t *sp,
          L4_Word_t *ip,
          L4_Word_t *flags)
{
    L4_Word_t dummy;
    L4_ThreadId_t dummy_id;
    L4_ThreadState_t state;
    (void) L4_ExchangeRegisters (t, 7 + (1 << 8) + (1 << 9), 0, 0, 0, 0,
     ((L4_ThreadId_t) { raw : 0UL}), &state.raw, sp, ip, flags,
     &dummy, &dummy_id);
    return state;
}
static inline L4_Word_t L4_AssociateInterrupt (L4_ThreadId_t InterruptThread,
        L4_ThreadId_t HandlerThread)
{
    return L4_ThreadControl (InterruptThread, InterruptThread,
        ((L4_ThreadId_t) { raw : 0UL}), HandlerThread, (void *) -1);
}
static inline L4_Word_t L4_DeassociateInterrupt (L4_ThreadId_t InterruptThread)
{
    return L4_ThreadControl (InterruptThread, InterruptThread,
        ((L4_ThreadId_t) { raw : 0UL}), InterruptThread, (void *) -1);
}
static inline L4_Bool_t L4_IpcSucceeded (L4_MsgTag_t t)
{
    return (t.X.flags & 0x8) == 0;
}
static inline L4_Bool_t L4_IpcFailed (L4_MsgTag_t t)
{
    return (t.X.flags & 0x8) != 0;
}
static inline L4_Bool_t L4_IpcPropagated (L4_MsgTag_t t)
{
    return (t.X.flags & 0x1) != 0;
}
static inline L4_Bool_t L4_IpcRedirected (L4_MsgTag_t t)
{
    return (t.X.flags & 0x2) != 0;
}
static inline L4_Bool_t L4_IpcXcpu (L4_MsgTag_t t)
{
    return (t.X.flags & 0x4) != 0;
}
static inline void L4_Set_Propagation (L4_MsgTag_t * t)
{
    t->X.flags = 1;
}
static inline L4_Word_t L4_Timeouts (L4_Time_t SndTimeout, L4_Time_t RcvTimeout)
{
    return (SndTimeout.raw << 16) + (RcvTimeout.raw);
}
static inline L4_MsgTag_t L4_Call_Timeouts (L4_ThreadId_t to,
     L4_Time_t SndTimeout,
     L4_Time_t RcvTimeout)
{
    L4_ThreadId_t dummy;
    return L4_Ipc (to, to, L4_Timeouts (SndTimeout, RcvTimeout),
     &dummy);
}
static inline L4_MsgTag_t L4_Call (L4_ThreadId_t to)
{
    return L4_Call_Timeouts (to, ((L4_Time_t) { raw : 0UL }), ((L4_Time_t) { raw : 0UL }));
}
static inline L4_MsgTag_t L4_Send_Timeout (L4_ThreadId_t to, L4_Time_t SndTimeout)
{
    return L4_Ipc (to, ((L4_ThreadId_t) { raw : 0UL}), (L4_Word_t)SndTimeout.raw << 16, (L4_ThreadId_t *) 0);
}
static inline L4_MsgTag_t L4_Send (L4_ThreadId_t to)
{
    return L4_Send_Timeout (to, ((L4_Time_t) { raw : 0UL }));
}
static inline L4_MsgTag_t L4_Reply (L4_ThreadId_t to)
{
    return L4_Send_Timeout (to, ((L4_Time_t) { period : { 0,1,0 }}));
}
static inline L4_MsgTag_t L4_Receive_Timeout (L4_ThreadId_t from,
       L4_Time_t RcvTimeout)
{
    L4_ThreadId_t dummy;
    return L4_Ipc (((L4_ThreadId_t) { raw : 0UL}), from, (L4_Word_t)RcvTimeout.raw, &dummy);
}
static inline L4_MsgTag_t L4_Receive (L4_ThreadId_t from)
{
    return L4_Receive_Timeout (from, ((L4_Time_t) { raw : 0UL }));
}
static inline L4_MsgTag_t L4_Wait_Timeout (L4_Time_t RcvTimeout,
           L4_ThreadId_t * from)
{
    return L4_Ipc (((L4_ThreadId_t) { raw : 0UL}), ((L4_ThreadId_t) { raw : ~0UL}), (L4_Word_t)RcvTimeout.raw, from);
}
static inline L4_MsgTag_t L4_Wait (L4_ThreadId_t * from)
{
    return L4_Wait_Timeout (((L4_Time_t) { raw : 0UL }), from);
}
static inline L4_MsgTag_t L4_ReplyWait_Timeout (L4_ThreadId_t to,
         L4_Time_t RcvTimeout,
         L4_ThreadId_t * from)
{
    return L4_Ipc (to, ((L4_ThreadId_t) { raw : ~0UL}), L4_Timeouts (((L4_Time_t) { period : { 0,1,0 }}), RcvTimeout),
     from);
}
static inline L4_MsgTag_t L4_ReplyWait (L4_ThreadId_t to, L4_ThreadId_t * from)
{
    return L4_ReplyWait_Timeout (to, ((L4_Time_t) { raw : 0UL }), from);
}
static inline void L4_Sleep (L4_Time_t t)
{
    L4_LoadMR(0, L4_Receive_Timeout (L4_MyGlobalId (), t).raw);
}
static inline L4_MsgTag_t L4_Lcall (L4_ThreadId_t to)
{
    L4_ThreadId_t dummy;
    return L4_Lipc (to, to, L4_Timeouts (((L4_Time_t) { raw : 0UL }), ((L4_Time_t) { raw : 0UL })), &dummy);
}
static inline L4_MsgTag_t L4_LreplyWait (L4_ThreadId_t to, L4_ThreadId_t * from)
{
    return L4_Lipc (to, ((L4_ThreadId_t) { local : { X : {0,~0UL}}}),
      L4_Timeouts (((L4_Time_t) { period : { 0,1,0 }}), ((L4_Time_t) { raw : 0UL })), from);
}
typedef union
{
    uintptr_t ID;
    struct
    {
     uintptr_t version : 32;
     uintptr_t lthread : 8;
     uintptr_t task : 8;
     uintptr_t _pad : 16;
    } id;
} l4_threadid_t;
typedef l4_threadid_t l4_taskid_t;
typedef struct
{
    uintptr_t reg[8];
} l4_ipc_reg_msg_t;
typedef struct
{
    char magic[4];
    void* dit_hdr;
    short build;
    short version;
    uintptr_t memory_size;
    uintptr_t kernel;
    uintptr_t kernel_data;
} l4_kernel_info;
typedef union
{
    int foo;
    struct
    {
 int error_code;
 int snd_error;
    } md;
} l4_msgdope_t;
typedef union
{
 uintptr_t fpage;
 struct
 {
  uintptr_t grant : 1;
  uintptr_t write : 1;
  uintptr_t size : 6;
  uintptr_t zero : 4;
  uintptr_t page : 64 -12;
 } fp;
} l4_fpage_t;
typedef struct
{
 unsigned int snd_exponent;
 unsigned int snd_mantissa;
 unsigned int rcv_exponent;
 unsigned int rcv_mantissa;
 unsigned int snd_pf;
 unsigned int rcv_pf;
}l4_timeout_t;
typedef l4_ipc_reg_msg_t l4_ipcregs_t;
int task_equal(l4_threadid_t t1,l4_threadid_t t2);
int l4emu_reg_interrupt(uintptr_t, l4_threadid_t);
int l4emu_get_interrupt(uintptr_t, bool);
int l4emu_sleep(l4_threadid_t);
uintptr_t l4emu_convert_timeout(l4_timeout_t);
void l4dbg_enter(char *msg);
void l4dbg_sleep(uint64_t time);
l4_kernel_info *l4_get_old_kip(void);
l4_threadid_t l4_get_mypager(void);
int l4_request_page( void *page );
l4_timeout_t l4_get_timeout( uint64_t snd_micros, uint64_t rcv_micros,
        int spf, int rpf );
void l4_donate_task( l4_threadid_t to, l4_threadid_t task );
int l4_make_uncached( void *addr );
void l4_thread_switch(l4_threadid_t destination);
int l4_id_nearest(l4_threadid_t destination, l4_threadid_t *next_chief);
static inline l4_threadid_t
l4_myself(void)
{
 return (l4_threadid_t)L4_Myself().raw;
}
l4_threadid_t l4_thread_ex_regs(l4_threadid_t destination, uintptr_t pc,
    uintptr_t sp,
    l4_threadid_t *preempter, l4_threadid_t *pager,
    uintptr_t *old_pc, uintptr_t *old_sp);
l4_threadid_t l4_dodgy_hack_ex_regs(l4_threadid_t destination, uintptr_t pc,
        uintptr_t sp, l4_threadid_t *preempter,
        l4_threadid_t *pager, uintptr_t *old_pc,
        uintptr_t *old_sp);
l4_threadid_t l4_existing_thread_ex_regs(l4_threadid_t destination, uintptr_t pc,
      uintptr_t sp, l4_threadid_t *preempter,
      l4_threadid_t *pager, uintptr_t *old_pc,
      uintptr_t *old_sp);
l4_taskid_t l4_task_new( l4_taskid_t destination, uintptr_t mcp_or_new_chief,
    uintptr_t sp, uintptr_t pc, l4_threadid_t pager );
void l4_fpage_unmap(l4_fpage_t fpage, uintptr_t map_mask);
uint64_t l4_sys_time(void);
int l4_ipc_wait(l4_threadid_t *src,
  void *rcv_msg,
  l4_ipcregs_t *rcv_regs,
  l4_timeout_t timeout,
  l4_msgdope_t *result);
int l4_ipc_sleep(l4_timeout_t t, l4_msgdope_t *result);
int l4_ipc_call(l4_threadid_t dest, const void *snd_msg,
  const l4_ipcregs_t *snd_regs, void *rcv_msg,
  l4_ipcregs_t *rcv_regs, l4_timeout_t timeout,
  l4_msgdope_t *result);
int l4_ipc_send_deceiving(l4_threadid_t dest, l4_threadid_t vsend,
     const void *snd_msg, l4_ipcregs_t *snd_reg,
     l4_timeout_t timeout, l4_msgdope_t *result);
int l4_ipc_send(l4_threadid_t dest, const void *snd_msg,
  const l4_ipcregs_t *snd_regs,
  l4_timeout_t timeout, l4_msgdope_t *result);
int l4_ipc_receive(l4_threadid_t src,void *rcv_msg,
     l4_ipcregs_t *rcv_regs,l4_timeout_t timeout,
     l4_msgdope_t *result);
int l4_ipc_reply_and_wait(l4_threadid_t dest, const void *snd_msg,
     const l4_ipcregs_t *snd_regs,
     l4_threadid_t *src, void *rcv_msg,
     l4_ipcregs_t *rcv_regs, l4_timeout_t timeout,
     l4_msgdope_t *result);
unsigned int cpu_mhz(void);
typedef struct {
 uintptr_t address;
 passwd_t passwd;
} cap_t;
typedef int8_t access_t;
typedef int8_t mac_access_t;
struct validation_request {
 uintptr_t subject_label:64 -4;
 uintptr_t set_0_or_1:1;
 uintptr_t read_or_create_domain:1;
 uintptr_t write_or_create_type:1;
 uintptr_t execute_or_transfer:1;
 uintptr_t object_label:64 -4;
 uintptr_t destroy_or_pdx:1;
 uintptr_t result:1;
 uintptr_t undef:2;
};
typedef uint64_t time_t;
typedef int objflags_t;
typedef uintptr_t pdx_t;
typedef struct {
 cap_t clist[32];
 passwd_t passwd[32];
        slabel_t pdx_domain[32];
 slabel_t pdx_type[32];
 slabel_t pdx_default_type[32];
 int n_entry[32];
 int x_entry[32];
 pdx_t entry[192];
} pdxdata_t;
typedef struct objinfo
{
 size_t extent;
 time_t creation;
 time_t modification;
 time_t access;
 time_t accounting;
 void *userinfo;
 void *acctinfo;
 size_t length;
        slabel_t security_type;
 objflags_t flags;
 objflags_t special;
 uint n_caps;
 uint n_pdx;
 cap_t account;
 cap_t pager;
 cap_t cntrl_object;
 passwd_t passwd[128];
 access_t rights[128];
        pdxdata_t pdx;
} objinfo_t;
typedef struct apddesc
{
 cap_t clist[0x10];
 apdpos_t n_locked;
 apdpos_t n_apd;
 slabel_t security_domain;
 slabel_t security_default_type;
} apddesc_t;
typedef uint8_t clistformat_t;
typedef struct clist {
 char type;
 uint8_t rel_ver;
 clistformat_t format;
 uint16_t n_caps;
 uint32_t reserved;
 cap_t caps[1];
} clist_t;
typedef uintptr_t thread_t;
typedef enum threadstate {THREAD_UNUSED = 0,
     THREAD_ACTIVE,
     THREAD_SLEEPING,
     THREAD_BLOCKED,
     THREAD_NEWLY_CREATED,
    } threadstate_t;
typedef struct utcb {
 int32_t flags;
 threadstate_t state;
 time_t start_time;
 time_t cpu_time;
 time_t cpu_limit;
 uint8_t priority;
 pdx_t delete_callback;
 void *bank_account;
 uintptr_t localnode_id;
 uintptr_t userflags;
 void *userinfo;
} utcb_t;
typedef int pagefault_t;
typedef int pagedisp_t;
typedef uintptr_t pager_t;
typedef unsigned int excpt_t;
typedef void *excpthndlr_t;
