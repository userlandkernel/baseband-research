
/*
 * Copyright (c) 2001-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Description:   Commonly used L4 types
 */
#ifndef __L4__TYPES_H__
#define __L4__TYPES_H__

#undef L4_32BIT
#undef L4_64BIT
#undef L4_BIG_ENDIAN
#undef L4_LITTLE_ENDIAN

/*
 * Determine which architecture dependent files to include
 */

#if !defined(__L4_ARCH__)
# if defined(__i386__)
#  define L4_ARCH_IA32
#  define __L4_ARCH__ ia32
# elif defined(__ia64__)
#  undef ia64
#  define L4_ARCH_IA64
#  define __L4_ARCH__ ia64
# elif defined(__PPC64__)
#  define L4_ARCH_POWERPC64
#  define __L4_ARCH__ powerpc64
# elif defined(__PPC__)
#  undef powerpc
#  define L4_ARCH_POWERPC
#  define __L4_ARCH__ powerpc
# elif defined(ARCH_ARM) || defined(__arm__) || defined(__thumb__) || defined(__ARMCC_VERSION)
#  define L4_ARCH_ARM
#  define __L4_ARCH__ arm
# elif defined(__x86_64__)
#  define L4_ARCH_AMD64
#  define __L4_ARCH__ amd64
# elif defined(__alpha__)
#  define L4_ARCH_ALPHA
#  define __L4_ARCH__ alpha
# elif defined(__mips__)
#  define L4_ARCH_MIPS
#  define __L4_ARCH__ mips
# elif defined(__sparc__)
#  define L4_ARCH_SPARC64
#  define __L4_ARCH__ sparc64
# else
# endif
#endif

#include <l4/arch/types.h>
/** @todo FIXME: Use base types - awiggins. */
//#include <stdint.h>

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION<210000)
#if defined(__l4_cplusplus)
#define __CPP "C"
#else
#define __CPP
#endif
#define __attribute__(a)
#endif

/** 
 *  @todo FIXME: Rework, mark L4_Word_t as a data-structure alias of
 *  word_t - awiggins.
 */
typedef L4_Word_t word_t;

/*
 * All types used within <arch/syscalls.h> should be declared in this
 * file.
 */

typedef word_t L4_Bool_t;

/*
 * RemoteMemoryDesc
 */
typedef struct {
    word_t address;
    word_t size;
    word_t direction;
} L4_MemoryCopyDesriptor;

typedef word_t mutex_info_t;

/**
 *  @todo FIXME: Remove this mess, mark L4_INLINE as a data-structure alias
 *  and define INLINE in our compat library - awiggins.
 */
#if defined(__ARMCC_VERSION)
#define L4_INLINE               __inline
#else
#define L4_INLINE               static inline
#endif
#ifndef INLINE
#define INLINE L4_INLINE
#endif

/** @todo FIXME Source from libc's limit.h once we can - awiggins. */
#define CHAR_BIT 8
#define WORD_T_BIT (sizeof (word_t) * CHAR_BIT)
#define L4_BITS_PER_WORD WORD_T_BIT

// XXX: magpie workaround
// # define __PLUS32 + (sizeof (L4_Word_t) * 8 - 32)
#if defined(L4_64BIT)
# define __PLUS32       + 32
# define __14           32
# define __18           32
#else
# define __PLUS32
# define __14           14
# define __18           18
#endif

#include <l4/macros.h> /* BITFIELD() and SHUFFLE() macros. */

/**
 *  @todo FIXME: Mark as data-structure aliases - awiggins.
 */
#define L4_BITFIELD2(t,a,b)               BITFIELD2(t,a,b)
#define L4_BITFIELD3(t,a,b,c)             BITFIELD3(t,a,b,c)
#define L4_BITFIELD4(t,a,b,c,d)           BITFIELD4(t,a,b,c,d)
#define L4_BITFIELD5(t,a,b,c,d,e)         BITFIELD5(t,a,b,c,d,e)
#define L4_BITFIELD6(t,a,b,c,d,e,f)       BITFIELD6(t,a,b,c,d,e,f)
#define L4_BITFIELD7(t,a,b,c,d,e,f,g)     BITFIELD7(t,a,b,c,d,e,f,g)
#define L4_BITFIELD8(t,a,b,c,d,e,f,g,h)   BITFIELD8(t,a,b,c,d,e,f,g,h)
#define L4_BITFIELD9(t,a,b,c,d,e,f,g,h,i) BITFIELD9(t,a,b,c,d,e,f,g,h,i)
#define L4_SHUFFLE2(a,b)                  SHUFFLE2(a,b)
#define L4_SHUFFLE3(a,b,c)                SHUFFLE3(a,b,c)
#define L4_SHUFFLE4(a,b,c,d)              SHUFFLE4(a,b,c,d)
#define L4_SHUFFLE5(a,b,c,d,e)            SHUFFLE5(a,b,c,d,e)
#define L4_SHUFFLE6(a,b,c,d,e,f)          SHUFFLE6(a,b,c,d,e,f)
#define L4_SHUFFLE7(a,b,c,d,e,f,g)        SHUFFLE7(a,b,c,d,e,f,g)

/*
 * SpaceControl controls
 */
#define L4_SpaceCtrl_new            (1 << 0)
#define L4_SpaceCtrl_delete         (1 << 1)
#define L4_SpaceCtrl_resources      (1 << 2)
#define L4_SpaceCtrl_space_pager    (1 << 3)

/*
 * Exregs controls
 */
#define L4_ExReg_sp             (1 << 3)
#define L4_ExReg_ip             (1 << 4)
#define L4_ExReg_sp_ip          (L4_ExReg_sp | L4_ExReg_ip)
#define L4_ExReg_flags          (1 << 5)
#define L4_ExReg_sp_ip_flags    (L4_ExReg_sp | L4_ExReg_ip | L4_ExReg_flags)
#define L4_ExReg_user           (1 << 6)
#define L4_ExReg_Tls            (1 << 7)
//#define L4_ExReg_pager          (1 << 7)
#define L4_ExReg_Halt           (1 << 8 | 1 << 0)
#define L4_ExReg_Resume         (1 << 8)
#define L4_ExReg_AbortOperation (1 << 1 | 1 << 2)
#define L4_ExReg_Deliver        (1 << 9)
#define L4_ExReg_CopyRegs       (1 << 10)
#define L4_ExReg_Suspend        (1 << 11 | 1 << 0)
#define L4_ExReg_Unsuspend      (1 << 11 | 0 << 0)
#define L4_ExReg_RegsToMRs      (1 << 12)
#define L4_ExReg_MRsToRegs      (1 << 13)
#define L4_ExReg_SrcThread(x)   (L4_GlobalId(L4_ThreadNo(x), 0UL).raw)

/*
 * Deprecated Exregs controls
 */
#define L4_ExReg_AbortIPC       (1 << 1 | 1 << 2)
#define L4_ExReg_AbortRecvIPC   (1 << 1)
#define L4_ExReg_AbortSendIPC   (1 << 2)

/*
 * SecurityControl controls
 */
#define L4_SecurityCtrl_op(x)       ((x) << 8)
#define L4_SecurityCtrl_domain(x)   ((x) << (L4_BITS_PER_WORD-4))

#define L4_SecurityControl_OpGrant      0
#define L4_SecurityControl_OpRevoke     1
#define L4_SecurityControl_OpManage     2

/*
 *  Mutex controls.
 */
#define L4_Mutex_Aquire      ((word_t)1 << 0)
#define L4_Mutex_Release     ((word_t)0 << 0)
#define L4_Mutex_Blocking    ((word_t)1 << 1)
#define L4_Mutex_NonBlocking ((word_t)0 << 1)
#define L4_Mutex_Hybrid      ((word_t)1 << 2)

/*
 *  MutexControl controls.
 */
#define L4_MutexCtrl_Create ((word_t)1 << 0)
#define L4_MutexCtrl_Delete ((word_t)1 << 1)

/*
 * InterruptControl controls
 */
#define L4_InterruptControl_count(x)        (((x) & ((1UL << 6) - 1)))
#define L4_InterruptControl_op(x)           (((x) & ((1UL << 2) - 1)) << 6)

/*
 * MemoryCopy direction field
 */
#define L4_MemoryCopyFrom                   (0x1)
#define L4_MemoryCopyTo                     (0x2)
#define L4_MemoryCopyBoth                   (L4_MemoryCopyFrom|L4_MemoryCopyTo)

#if defined(L4_64BIT)

#define L4_InterruptControl_request(x)      (((x) & ((1UL << 50) - 1)) << 8)
#define L4_InterruptControl_notifybit(x)    (((x) & ((1UL << 6) - 1)) << 58)

#define L4_InterruptControl_RegisterIrq     0ULL
#define L4_InterruptControl_UnregisterIrq   1ULL
#define L4_InterruptControl_AcknowledgeIrq  2ULL
#define L4_InterruptControl_AckWaitIrq      3ULL

#else

#define L4_InterruptControl_request(x)      (((x) & ((1UL << 18) - 1)) << 8)
#define L4_InterruptControl_notifybit(x)    (((x) & ((1UL << 5) - 1)) << 27)

#define L4_InterruptControl_RegisterIrq     0UL
#define L4_InterruptControl_UnregisterIrq   1UL
#define L4_InterruptControl_AcknowledgeIrq  2UL
#define L4_InterruptControl_AckWaitIrq      3UL

#endif /* defined (L4_64BIT) */


/*
 * Error codes
 */

#define L4_ErrOk                (0)
#define L4_ErrNoPrivilege       (1)
#define L4_ErrInvalidThread     (2)
#define L4_ErrInvalidSpace      (3)
#define L4_ErrInvalidScheduler  (4)
#define L4_ErrInvalidParam      (5)
#define L4_ErrUtcbArea          (6)
/* L4_ErrKipArea was 7 */
#define L4_ErrNoMem             (8)
#define L4_ErrSpaceNotEmpty     (9)
#define L4_ErrInvalidMutex      (10)
#define L4_ErrMutexBusy         (11)
#define L4_ErrDomainConflict    (15)
#define L4_ErrNotImplemented    (16)

/*
 * IPC Error Codes
 */
#define L4_ErrTimeout           (1)
#define L4_ErrNonExist          (2)
#define L4_ErrCanceled          (3)

#define L4_ErrMsgOverflow       (4)
#define L4_ErrNotAccepted       (5)
#define L4_ErrAborted           (7)

/*
 * IPC Error Phase, returned by L4_IpcError
 */
#define L4_ErrSendPhase         (0)
#define L4_ErrRecvPhase         (1)

L4_INLINE L4_Bool_t
L4_IpcError(L4_Word_t err, L4_Word_t *err_code)
{
    L4_Bool_t phase;

    phase = err & 0x1;
    if (err_code) {
        *err_code = (err >> 1) & 0xf;
    }
    return phase;
}

/*
 * Fpages
 */

typedef union {
    L4_Word_t raw;
    struct {
        L4_BITFIELD4(L4_Word_t, rwx:3, meta:1, s:6, b:22 __PLUS32);
    } X;
} L4_Fpage_t;

#define L4_Readable             (0x04)
#define L4_Writable             (0x02)
#define L4_eXecutable           (0x01)
#define L4_FullyAccessible      (0x07)
#define L4_ReadWriteOnly        (0x06)
#define L4_ReadeXecOnly         (0x05)
#define L4_NoAccess             (0x00)

#define L4_Nilpage              L4_FpageLog2(0UL, 0)
#define L4_CompleteAddressSpace L4_FpageLog2(0UL, 1)

#include <l4/arch/specials.h>

L4_INLINE L4_Bool_t
L4_IsNilFpage(L4_Fpage_t f)
{
    return f.raw == 0;
}

L4_INLINE L4_Word_t
L4_Rights(L4_Fpage_t f)
{
    return f.X.rwx;
}

L4_INLINE L4_Fpage_t
L4_Set_Rights(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx = rwx;
    return *f;
}

L4_INLINE L4_Fpage_t
L4_Set_Meta(L4_Fpage_t *f)
{
    f->X.meta = 1;
    return *f;
}

L4_INLINE L4_Fpage_t
L4_FpageAddRights(L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx |= rwx;
    return f;
}

L4_INLINE L4_Fpage_t
L4_FpageAddRightsTo(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx |= rwx;
    return *f;
}

L4_INLINE L4_Fpage_t
L4_FpageRemoveRights(L4_Fpage_t f, L4_Word_t rwx)
{
    f.X.rwx &= ~rwx;
    return f;
}

L4_INLINE L4_Fpage_t
L4_FpageRemoveRightsFrom(L4_Fpage_t *f, L4_Word_t rwx)
{
    f->X.rwx &= ~rwx;
    return *f;
}

#if defined(__l4_cplusplus)
static inline L4_Fpage_t
operator +(const L4_Fpage_t &f, L4_Word_t rwx)
{
    return L4_FpageAddRights(f, rwx);
}

static inline L4_Fpage_t
operator +=(L4_Fpage_t &f, L4_Word_t rwx)
{
    return L4_FpageAddRightsTo(&f, rwx);
}

static inline L4_Fpage_t
operator -(const L4_Fpage_t &f, L4_Word_t rwx)
{
    return L4_FpageRemoveRights(f, rwx);
}

static inline L4_Fpage_t
operator -=(L4_Fpage_t &f, L4_Word_t rwx)
{
    return L4_FpageRemoveRightsFrom(&f, rwx);
}
#endif /* __l4_cplusplus */

L4_INLINE L4_Fpage_t
L4_Fpage(L4_Word_t BaseAddress, L4_Word_t FpageSize)
{
    L4_Fpage_t fp = { 0UL };
    L4_Word_t msb = __L4_Msb(FpageSize);

    fp.raw = BaseAddress;
    fp.X.s = (1UL << msb) < FpageSize ? msb + 1 : msb;
    fp.X.rwx = L4_NoAccess;
    fp.X.meta = 0;
    return fp;
}

L4_INLINE L4_Fpage_t
L4_FpageLog2(L4_Word_t BaseAddress, int FpageSize)
{
    L4_Fpage_t fp = { 0UL };
    fp.raw = BaseAddress;
    fp.X.s = FpageSize;
    fp.X.rwx = L4_NoAccess;
    fp.X.meta = 0;
    return fp;
}

L4_INLINE L4_Word_t
L4_Address(L4_Fpage_t f)
{
    return f.raw & ~((1UL << f.X.s) - 1);
}

L4_INLINE L4_Word_t
L4_Size(L4_Fpage_t f)
{
    return f.X.s == 0 ? 0 : (1UL << f.X.s);
}

L4_INLINE L4_Word_t
L4_SizeLog2(L4_Fpage_t f)
{
    return f.X.s;
}

/*
 * Thread IDs
 */

typedef union {
    L4_Word_t raw;
    struct {
        L4_BITFIELD2(L4_Word_t, version:__14, thread_no:__18);
    } X;
} L4_GthreadId_t;

typedef union {
    L4_Word_t raw;
    L4_GthreadId_t global;
} L4_ThreadId_t;

#define L4_nilthread            L4_GlobalId(0UL, 0UL)
#define L4_anythread            L4_GlobalId(-1UL, -1UL)
#define L4_waitnotify           L4_GlobalId(-1UL, -2UL)
#define L4_myselfconst          L4_GlobalId(-1UL, -3UL)
#define L4_anylocalthread       L4_GlobalId(-1UL, -64UL)

#define L4_rootserverno         0UL
#define L4_rootserver           L4_GlobalId(L4_rootserverno, 1UL)
L4_INLINE L4_ThreadId_t
L4_GlobalId(L4_Word_t threadno, L4_Word_t version)
{
    L4_ThreadId_t t = { 0UL };
    t.global.X.thread_no = threadno;
    t.global.X.version = version;

    return t;
}

L4_INLINE L4_Word_t
L4_Version(L4_ThreadId_t t)
{
    return t.global.X.version;
}

L4_INLINE L4_Word_t
L4_ThreadNo(L4_ThreadId_t t)
{
    return t.global.X.thread_no;
}

L4_INLINE L4_Bool_t
L4_IsThreadEqual(const L4_ThreadId_t l, const L4_ThreadId_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t
L4_IsThreadNotEqual(const L4_ThreadId_t l, const L4_ThreadId_t r)
{
    return l.raw != r.raw;
}

#if defined(__l4_cplusplus)
static inline L4_Bool_t
operator ==(const L4_ThreadId_t &l, const L4_ThreadId_t &r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t
operator !=(const L4_ThreadId_t &l, const L4_ThreadId_t &r)
{
    return l.raw != r.raw;
}
#endif /* __l4_cplusplus */

L4_INLINE L4_Bool_t
L4_IsNilThread(L4_ThreadId_t t)
{
    return t.raw == 0;
}

#undef __14
#undef __18
#undef __PLUS32

/*
 *  Mutex Identifiers.
 */

typedef union {
    word_t raw;
    word_t number;
} L4_MutexId_t;

INLINE L4_MutexId_t
L4_MutexId(word_t mutex_number)
{
    L4_MutexId_t m = {0UL};
    m.number = mutex_number;
    return m;
}

INLINE word_t
L4_MutexNo(L4_MutexId_t m)
{
    return m.number;
}

/*
 *  Space Identifiers.
 */

typedef union {
    L4_Word_t raw;
    L4_Word_t space_no;
} L4_SpaceId_t;

#define L4_nilspace             L4_SpaceId(~0UL)
#define L4_rootspace            L4_SpaceId(0UL)

L4_INLINE L4_SpaceId_t
L4_SpaceId(L4_Word_t space_no)
{
    L4_SpaceId_t s = { 0UL };
    s.space_no = space_no;

    return s;
}

L4_INLINE L4_Word_t
L4_SpaceNo(L4_SpaceId_t s)
{
    return s.space_no;
}

L4_INLINE L4_Bool_t
L4_IsSpaceEqual(const L4_SpaceId_t l, const L4_SpaceId_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t
L4_IsSpaceNotEqual(const L4_SpaceId_t l, const L4_SpaceId_t r)
{
    return l.raw != r.raw;
}

#if defined(__l4_cplusplus)
static inline L4_Bool_t
operator ==(const L4_SpaceId_t &l, const L4_SpaceId_t &r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t
operator !=(const L4_SpaceId_t &l, const L4_SpaceId_t &r)
{
    return l.raw != r.raw;
}
#endif /* __l4_cplusplus */

L4_INLINE L4_Bool_t
L4_IsNilSpace(L4_SpaceId_t s)
{
    return s.raw == ~0UL;
}

/*
 *  Clist Identifiers.
 */

typedef union {
    L4_Word_t raw;
    L4_Word_t clist_no;
} L4_ClistId_t;

#define L4_rootclist            L4_ClistId(0UL)

L4_INLINE L4_ClistId_t
L4_ClistId(L4_Word_t clist_no)
{
    L4_ClistId_t id = { 0UL };
    id.clist_no = clist_no;

    return id;
}

/*
 * Result values from schedule system call
 */

#define L4_SCHEDRESULT_ERROR            (0)
#define L4_SCHEDRESULT_DEAD             (1)
#define L4_SCHEDRESULT_INACTIVE         (2)
#define L4_SCHEDRESULT_RUNNING          (3)
#define L4_SCHEDRESULT_PENDING_SEND     (4)
#define L4_SCHEDRESULT_SENDING          (5)
#define L4_SCHEDRESULT_WAITING          (6)
#define L4_SCHEDRESULT_RECEIVING        (7)
#define L4_SCHEDRESULT_WAITING_NOTIFY   (8)
#define L4_SCHEDRESULT_WAITING_XCPU     (9)
#define L4_SCHEDRESULT_WAITING_MUTEX    (10)


#endif /* !__L4__TYPES_H__ */
