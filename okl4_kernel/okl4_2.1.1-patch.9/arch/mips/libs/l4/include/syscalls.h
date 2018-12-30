/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Description:   MIPS system call ABI
 */                

#ifndef __L4__MIPS__SYSCALLS_H__
#define __L4__MIPS__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

#define SYSCALL_ipc                     -101UL
#define SYSCALL_thread_switch           -102UL
#define SYSCALL_thread_control          -103UL
#define SYSCALL_exchange_registers      -104UL
#define SYSCALL_schedule                -105UL
#define SYSCALL_map_control             -106UL
#define SYSCALL_space_control           -107UL
/* Unused system call slot:             -108UL */
#define SYSCALL_cache_control           -109UL
#define SYSCALL_security_control        -110UL
#define SYSCALL_interrupt_control       -111UL
#define SYSCALL_cap_control             -112UL
#define SYSCALL_platform_control        -116UL
#define SYSCALL_space_switch            -117UL
#define SYSCALL_mutex                   -118UL
#define SYSCALL_mutex_control           -119UL

/* The application gets the kernel info page by doing some illegal
 * instruction, with at ($1) == 0x1face (interface) ca11 (call) 14 (L4) e1f
 * (ELF) 64 (MIPS64)
 */
#if defined(L4_32BIT)
#define __L4_MAGIC_KIP_REQUEST          (0x1faceCa1L)
#elif defined(L4_64BIT)
#define __L4_MAGIC_KIP_REQUEST          (0x1faceCa1114e1f64ULL)
#endif

/* Memory attributes for MIPS memory control */
#define L4_WriteThroughNoAllocMemory    8
#define L4_FlushICache          29
#define L4_FlushDCache          30
#define L4_FlushCache           31


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
                                     L4_Word_t *ApiFlags,
                                     L4_Word_t *KernelId)
{
    register void * base_address        asm ("$8");  /* t0 */
    register L4_Word_t api_version      asm ("$9");  /* t1 */
    register L4_Word_t api_flags        asm ("$10"); /* t2 */
    register L4_Word_t kernel_id        asm ("$11"); /* t3 */
    register L4_Word_t req;

    req = __L4_MAGIC_KIP_REQUEST;

    __asm__ __volatile__ (
        ".set noat;             \n\r"
        "   move    $1, %4;     \r\n"
        "   wait;               \r\n"
        ".set at;               \n\r"
        : "=r" (base_address), "=r" (api_version), "=r" (api_flags),
          "=r" (kernel_id)
        : "r" (req)
        : "$1"
    );


    if( ApiVersion ) *ApiVersion = api_version;
    if( ApiFlags ) *ApiFlags = api_flags;
    if( KernelId ) *KernelId = kernel_id;

    return base_address;
}

L4_INLINE L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
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
    register L4_ThreadId_t r_result     asm ("$2");
    register L4_ThreadId_t r_dest       asm ("$4") = dest;
    register L4_Word_t r_control        asm ("$5") = control;
    register L4_Word_t r_sp             asm ("$6") = sp;
    register L4_Word_t r_ip             asm ("$7") = ip;
    register L4_Word_t r_flags          asm ("$8") = flags;
    register L4_Word_t r_userhandle     asm ("$9") = UserDefHandle;
    register L4_ThreadId_t r_pager      asm ("$10") = pager;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (r_result),
          "+r" (r_dest), "+r" (r_control), "+r" (r_sp), "+r" (r_ip),
          "+r" (r_flags), "+r" (r_userhandle), "+r" (r_pager)
        : [sysnum] "i" (SYSCALL_exchange_registers)
        : "$1", "$3", "$11", "$12", "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    *old_control = r_control;
    *old_sp = r_sp;
    *old_ip = r_ip;
    *old_flags = r_flags;
    *old_UserDefHandle = r_userhandle;
    *old_pager = r_pager;

    return r_result;
}

L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
                                      L4_SpaceId_t SpaceSpecifier,
                                      L4_ThreadId_t Scheduler,
                                      L4_ThreadId_t Pager,
                                      L4_ThreadId_t ExceptionHandler,
                                      L4_Word_t     Resources,
                                      void * UtcbLocation)
{
    register L4_Word_t result       asm ("$2");
    register L4_ThreadId_t r_dest   asm ("$4") = dest;
    register L4_SpaceId_t r_space   asm ("$5") = SpaceSpecifier;
    register L4_ThreadId_t r_sched  asm ("$6") = Scheduler;
    register L4_ThreadId_t r_pager  asm ("$7") = Pager;
    register L4_ThreadId_t r_excep  asm ("$8") = ExceptionHandler;
    register L4_Word_t r_resc       asm ("$9") = Resources;
    register void *r_utcb           asm ("$10") = UtcbLocation;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_dest), "+r" (r_space), "+r" (r_sched), "+r" (r_pager),
          "+r" (r_resc), "+r" (r_excep), "+r" (r_utcb)
        : [sysnum] "i" (SYSCALL_thread_control)
        : "$1", "$3", "$11", "$12", "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t L4_ThreadSwitch (L4_ThreadId_t dest)
{
    register L4_Word_t result       asm ("$2");
    register L4_ThreadId_t r_dest   asm ("$4") = dest;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_dest)
        : [sysnum] "i" (SYSCALL_thread_switch)
        : "$1", "$3", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
                                  L4_Word_t timeslice,
                                  L4_Word_t hw_thread_bitmask,
                                  L4_Word_t doman_control,
                                  L4_Word_t prio,
                                  L4_Word_t flags,
                                  L4_Word_t *rem_timeslice)
{
    register L4_Word_t r_result     asm ("$2");
    register L4_ThreadId_t r_dest   asm ("$4") = dest;
    register L4_Word_t r_tslen      asm ("$5") = timeslice;
    register L4_Word_t r_hwthrdmask asm ("$6") = hw_thread_bitmask;
    register L4_Word_t r_domainctl  asm ("$7") = doman_control;
    register L4_Word_t r_prio       asm ("$8") = prio;
    register L4_Word_t r_flags      asm ("$9") = flags;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (r_result), "+r" (r_dest), "+r" (r_tslen),
          "+r" (r_hwthrdmask), "+r" (r_domainctl), "+r" (r_prio),
          "+r" (r_flags)
        : [sysnum] "i" (SYSCALL_schedule)
        : "$1", "$3", "$10", "$11", "$12", "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    if (rem_timeslice)
        *rem_timeslice = r_tslen;

    return r_result;
}

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
                              L4_ThreadId_t FromSpecifier,
                              L4_MsgTag_t tag,
                              L4_ThreadId_t * from)
{
    register L4_ThreadId_t to_r asm ("$4") = to;
    register L4_ThreadId_t from_r asm ("$5") = FromSpecifier;
    register L4_ThreadId_t result asm ("$2");
    register L4_Word_t mr0 asm ("$3") = tag.raw;
    register L4_Word_t mr1 asm ("$16");
    register L4_Word_t mr2 asm ("$17");
    register L4_Word_t mr3 asm ("$18");
    register L4_Word_t mr4 asm ("$19");
    register L4_Word_t mr5 asm ("$20");
    register L4_Word_t mr6 asm ("$21");
    register L4_Word_t mr7 asm ("$22");
    register L4_Word_t mr8 asm ("$23");

    // Only load MRs if send phase is included
    if (! L4_IsNilThread (to))
    {
        mr1 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  1];
        mr2 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  2];
        mr3 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  3];
        mr4 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  4];
        mr5 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  5];
        mr6 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  6];
        mr7 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  7];
        mr8 = (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  8];

        __asm__ __volatile__ (
            "   li      $2,     %[sysnum]       \n"
            "   syscall                         \n"
            :
            "+r" (to_r), "+r" (from_r),
            "+r" (mr0), "+r" (mr1), "+r" (mr2), "+r" (mr3),
            "+r" (mr4), "+r" (mr5), "+r" (mr6), "+r" (mr7), "+r" (mr8),
            "=r" (result)
            : [sysnum] "i" (SYSCALL_ipc)
            : "$1", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
              "$13", "$14", "$15", "$24", "$25", /*"$28",*/ "$31",
              "memory"
        );
    } else {
        __asm__ __volatile__ (
            "   li      $2,     %[sysnum]       \n"
            "   syscall                         \n"
            :
            "+r" (to_r), "+r" (from_r),
            "+r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
            "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7), "=r" (mr8),
            "=r" (result)
            : [sysnum] "i" (SYSCALL_ipc)
            : "$1", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
              "$13", "$14", "$15", "$24", "$25", /*"$28",*/ "$31",
              "memory"
        );
    }

    if( !L4_IsNilThread(FromSpecifier) ) {
        *from = result;

        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  1] = mr1;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  2] = mr2;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  3] = mr3;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  4] = mr4;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  5] = mr5;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  6] = mr6;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  7] = mr7;
        (__L4_Mips_Utcb())[__L4_TCR_MR_OFFSET +  8] = mr8;
    }

    /* Return MR0 */
    tag.raw = mr0;
    return tag;
}

L4_INLINE L4_MsgTag_t L4_Notify (L4_ThreadId_t to, L4_Word_t mask)
{
    L4_MsgTag_t tag;
    register L4_ThreadId_t to_r asm ("$4") = to;
    register L4_ThreadId_t from_r asm ("$5") = L4_nilthread;
    register L4_Word_t mr0 asm ("$3") = L4_Notifytag.raw;
    register L4_Word_t mr1 asm ("$16") = mask;
    register L4_ThreadId_t result asm ("$2");

    __asm__ __volatile__ (
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "+r" (mr0), "+r" (mr1), "=r" (result),
          "+r" (to_r), "+r" (from_r)
        : [sysnum] "i" (SYSCALL_ipc)
        : "$1", "$6", "$7", "$8", "$9", "$10", "$11",
        "$12", "$13", "$14", "$15", /*"$16",*/ "$17", "$18", "$19", 
        "$20", "$21", "$22", "$23", "$24", "$25", "$31",
        "memory"
    );

    tag.raw = mr0;

    return tag;
}

L4_INLINE L4_MsgTag_t L4_WaitNotify (L4_Word_t * mask)
{
    L4_MsgTag_t tag;
    register L4_ThreadId_t to_r asm ("$4") = L4_nilthread;
    register L4_ThreadId_t from_r asm ("$5") = L4_waitnotify;
    register L4_Word_t mr0 asm ("$3") = L4_Waittag.raw;
    register L4_Word_t mr1 asm ("$16") = *mask;

    __asm__ __volatile__ (
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "+r" (mr0), "=r" (mr1),
          "+r" (to_r), "+r" (from_r)
        : [sysnum] "i" (SYSCALL_ipc)
        : "$1", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", 
          "$14", "$15", /*"$16",*/ "$17", "$18", "$19", "$20", "$21", 
          "$22", "$23", "$24", "$25", "$31",
          "memory"
    );

    *mask = mr1;
    tag.raw = mr0;

    return tag;
}

L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
                               L4_ThreadId_t FromSpecifier,
                               L4_MsgTag_t tag,
                               L4_ThreadId_t * from)
{
    return L4_Ipc(to, FromSpecifier, tag, from);
}


L4_INLINE L4_Word_t L4_MapControl (L4_SpaceId_t SpaceSpecifier, L4_Word_t control)
{
    register L4_Word_t result       asm ("$2");
    register L4_SpaceId_t r_space   asm ("$4") = SpaceSpecifier;
    register L4_Word_t r_control    asm ("$5") = control;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_space), "+r" (r_control)
        : [sysnum] "i" (SYSCALL_map_control)
        : "$1", "$3", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t L4_SpaceControl (L4_SpaceId_t SpaceSpecifier,
                                     L4_Word_t control,
                                     L4_ClistId_t clist,
                                     L4_Fpage_t UtcbArea,
                                     L4_Word_t resources,
                                     L4_Word_t *old_resources)
{
    register L4_Word_t result       asm ("$2");
    register L4_SpaceId_t r_space   asm ("$4") = SpaceSpecifier;
    register L4_Word_t r_control    asm ("$5") = control;
    register L4_Word_t r_clist      asm ("$6") = clist.raw;
    register L4_Word_t r_utcba      asm ("$7") = UtcbArea.raw;
    register L4_Word_t r_resc       asm ("$8") = resources;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_space), "+r" (r_control), "+r" (r_clist), "+r" (r_utcba),
          "+r" (r_resc)
        : [sysnum] "i" (SYSCALL_space_control)
        : "$1", "$3", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    if (old_resources)
        *old_resources = r_space.raw;   /* return val in register a0 */

    return result;
}

L4_INLINE L4_Word_t L4_CacheControl (L4_SpaceId_t SpaceSpecifier, L4_Word_t control)
{
    register L4_Word_t result       asm ("$2");
    register L4_SpaceId_t r_space   asm ("$4") = SpaceSpecifier;
    register L4_Word_t r_control    asm ("$5") = control;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_space), "+r" (r_control)
        : [sysnum] "i" (SYSCALL_cache_control)
        : "$1", "$3", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t
L4_SecurityControl(L4_SpaceId_t SpaceSpecifier, L4_Word_t control)
{
    register L4_Word_t result       asm ("$2");
    register L4_SpaceId_t r_space   asm ("$4") = SpaceSpecifier;
    register L4_Word_t r_control    asm ("$5") = control;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_space), "+r" (r_control)
        : [sysnum] "i" (SYSCALL_security_control)
        : "$1", "$3", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t
L4_PlatformControl(L4_Word_t control, L4_Word_t param1,
        L4_Word_t param2, L4_Word_t param3)
{
    register L4_Word_t result       asm ("$2");
    register L4_Word_t r_control    asm ("$4") = control;
    register L4_Word_t r_par1       asm ("$5") = param1;
    register L4_Word_t r_par2       asm ("$6") = param2;
    register L4_Word_t r_par3       asm ("$7") = param3;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_control), "+r" (r_par1), "+r" (r_par2),
          "+r" (r_par3)
        : [sysnum] "i" (SYSCALL_platform_control)
        : "$1", "$3", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t
L4_SpaceSwitch(L4_ThreadId_t dest, L4_SpaceId_t SpaceSpecifier,
        void *UtcbLocation)
{
    register L4_Word_t result       asm ("$2");
    register L4_ThreadId_t r_dest   asm ("$4") = dest;
    register L4_SpaceId_t r_space   asm ("$5") = SpaceSpecifier;
    register L4_Word_t r_utcb       asm ("$6") = (L4_Word_t)UtcbLocation;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_dest), "+r" (r_space), "+r" (r_utcb)
        : [sysnum] "i" (SYSCALL_space_switch)
        : "$1", "$3", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

INLINE word_t
L4_Mutex(L4_MutexId_t MutexSpecifier, word_t flags)
{
    register word_t result asm("$2");
    register L4_MutexId_t r_mutex asm("$4") = MutexSpecifier;
    register word_t r_flags asm("$5") = flags;

    __asm__ __volatile__("li $2, %[sysnum]\n"
                         "syscall\n"
                         : "=r" (result), "+r" (r_mutex), "+r" (r_flags)
                         : [sysnum] "i" (SYSCALL_mutex)
                         : "$1", "$3", "$7", "$8", "$9", "$10", "$11", "$12",
                           "$13", "$14", "$15", "$16", "$17", "$18", "$19",
                           "$20", "$21", "$22", "$23", "$24", "$25", /*"$28",*/
                         "$31", "memory");

    return result;
}

INLINE word_t
L4_MutexControl(L4_MutexId_t MutexSpecifier, word_t control)
{
    register word_t result asm("$2");
    register L4_MutexId_t r_mutex asm("$4") = MutexSpecifier;
    register word_t r_control asm("$5") = control;

    __asm__ __volatile__("li $2, %[sysnum]\n"
                         "syscall\n"
                         : "=r" (result), "+r" (r_mutex), "+r"(r_control)
                         : [sysnum] "i" (SYSCALL_mutex_control)
                         : "$1", "$3", "$7", "$8", "$9", "$10", "$11", "$12",
                           "$13", "$14", "$15", "$16", "$17", "$18", "$19",
                           "$20", "$21", "$22", "$23", "$24", "$25", /*"$28",*/
                         "$31", "memory");

    return result;
}

L4_INLINE L4_Word_t
L4_InterruptControl(L4_ThreadId_t dest, L4_Word_t control)
{
    register L4_Word_t result       asm ("$2");
    register L4_ThreadId_t r_tid    asm ("$4") = dest;
    register L4_Word_t r_control    asm ("$5") = control;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_tid), "+r" (r_control)
        : [sysnum] "i" (SYSCALL_interrupt_control)
        : "$1", "$3", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

L4_INLINE L4_Word_t
L4_CapControl(L4_ClistId_t clist, L4_Word_t control)
{
    register L4_Word_t result       asm ("$2");
    register L4_ClistId_t r_clist   asm ("$4") = clist;
    register L4_Word_t r_control    asm ("$5") = control;

    __asm__ __volatile__(
        "   li      $2,     %[sysnum]       \n"
        "   syscall                         \n"
        : "=r" (result),
          "+r" (r_clist), "+r" (r_control)
        : [sysnum] "i" (SYSCALL_cap_control)
        : "$1", "$3", "$6", "$7", "$8", "$9", "$10", "$11", "$12",
          "$13", "$14", "$15",
          "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
          "$24", "$25", /*"$28",*/ "$31", "memory"
    );

    return result;
}

#endif /* !__L4__MIPS__SYSCALLS_H__ */
