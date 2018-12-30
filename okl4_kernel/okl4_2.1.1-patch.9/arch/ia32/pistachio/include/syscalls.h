/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Description:   syscall macros
 */
#ifndef __IA32__SYSCALLS_H__
#define __IA32__SYSCALLS_H__

#include <kernel/arch/context.h>

#include <thread.h> /* To declare fast IPC path */

#if defined(_lint)
void empty(word_t foo);
#define return_platform_control(result, cont) cont(); empty (result); return
#define return_cache_control(result, cont) cont(); return (result)
#define return_schedule(result, rem_ts, cont) cont(); empty(rem_ts); empty (result); return
#define return_ipc() return 
#define return_exchange_registers(result, control, sp, ip, flags, pager, handle, cont) cont(); empty(control); empty (result.get_raw()); return
#define return_thread_control(result, cont) cont(); empty (result); return
#define return_thread_switch(cont) cont(); return
#define return_map_control(result, cont) cont(); return (result)
#define return_space_control(result, space_resources, cont) cont(); empty(space_resources); empty (result); return
#define return_security_control(result, cont) cont(); empty (result); return
#define return_space_switch(result, cont) cont(); empty (result); return
#define return_mutex(result, cont) cont(); empty (result); return
#define return_mutex_control(result, cont) cont(); empty (result); return
#define return_interrupt_control(result, cont) cont(); empty (result); return
#define return_cap_control(result, cont) cont(); empty (result); return
#define return_memory_copy(result, size, cont) cont(); empty (result); empty(size); return

#elif defined(__GNUC__)
#define return_ipc(from)                                        \
    do {                                                        \
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);\
    } while(false)

/*
 * Warning! According to Carl:
 * The syscall return functions that use the empty asm statement to get the 
 * registers in the correct place is a little dangerous. I know some other 
 * architectures do it and there is no easy way to prove a water tight 
 * solution. Lets just keep note of it in case we get bugs where the 
 * compiler has decided to reorder some instructions and end up with your 
 * output registers trashed.
 */

#define return_thread_control(result,cont)                      \
{                                                               \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                 \
}
#define return_space_control(result, resource, cont)            \
{                                                               \
    ACTIVATE_CONTINUATION_RESULT2(cont, result, resource);      \
}
#define return_schedule(result, rem_ts, cont)                           \
    {                                                                   \
    ACTIVATE_CONTINUATION_RESULT2(cont, result, rem_ts);                \
}

#define return_exchange_registers(result,                       \
        control, sp, ip, flags, pager, handle, cont)            \
{                                                               \
    ACTIVATE_CONTINUATION_RESULT_EXREG(cont, result, control, sp, ip, flags, pager, handle); \
}
#define return_thread_switch(cont) return

#define return_map_control(result,cont)                 \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_cache_control(result, cont)              \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);         \
}

#define return_security_control(result,cont)            \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_platform_control(result,cont)            \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_space_switch(result,cont)                \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_mutex(result,cont)                       \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_mutex_control(result,cont)               \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_interrupt_control(result,cont)           \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT(cont, result);                        \
}

#define return_cap_control(result,cont)                 \
{                                                       \
    __asm__ __volatile__ ("\n" :: "a"(result));         \
    ACTIVATE_CONTINUATION(cont);                        \
}

#define return_memory_copy(result, size, cont)          \
{                                                       \
    ACTIVATE_CONTINUATION_RESULT2(cont, result, size);                 \
}

#else
#error Unknown compiler
#endif

//
// System call function attributes
//
#define SYSCALL_ATTR(sec_name) NORETURN


//
//      Ipc ()
//
// Use three register parameters (to_tid = EAX, from_tid = EDX).
//
#define SYS_IPC(to, from)                               \
  void __attribute__ ((regparm (2))) sys_ipc (to, from)

/* Define and declare C IPC fastpath for ia32 */
#ifdef CONFIG_IPC_C_FASTPATH
#define SYS_IPC_FAST(to, from)                           \
void __attribute__ ((regparm (2))) sys_ipc_c_fastpath (to, from)
extern "C" SYS_IPC_FAST (threadid_t to_tid, threadid_t from_tid);
#endif /* CONFIG_IPC_C_FASTPATH */

#define RETURN_IPC_SANITY                                       \
    if (!current->get_state().is_running())                     \
    {   printf("line %d\n", __LINE__);                          \
        enter_kdebug("return_ipc ! running");}                  \
    if (current->queue_state.is_set(queue_state_t::wakeup))     \
    {   printf("line %d\n", __LINE__);                          \
        enter_kdebug("return_ipc in wakeup");}

//
//      ThreadControl ()
//
#define SYS_THREAD_CONTROL(dest, space, scheduler, pager,               \
        except_handler, thread_resources, utcb)                         \
  void __attribute__ ((regparm(3))) sys_thread_control (dest, space,    \
                                    scheduler, pager, except_handler,   \
                                    thread_resources, utcb,             \
                                    ia32_exceptionframe_t * __frame)




//
//      SpaceControl ()
//
#define SYS_SPACE_CONTROL(space, control, clist, utcb_area,          \
                          space_resources)                              \
  void __attribute__ ((regparm(3))) sys_space_control (space, control,  \
                                            clist, utcb_area,        \
                                            space_resources,            \
                                            ia32_exceptionframe_t * __frame)



//
//      Schedule ()
//
#define SYS_SCHEDULE(dest, ts_len, hw_thread_bitmask, processor_control,     \
                     prio, flags)                                            \
  void __attribute__ ((regparm(3))) sys_schedule(dest, ts_len,               \
                                        hw_thread_bitmask,                   \
                                        processor_control, prio, flags,      \
                                        ia32_exceptionframe_t * __frame)


//
//      ExchangeRegisters ()
//
#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip,                     \
                               uflags, uhandle)                             \
  void __attribute__ ((regparm(3))) sys_exchange_registers (dest, control,  \
                                                usp, uip, uflags, uhandle,  \
                                            ia32_exceptionframe_t * __frame)

//
//      ThreadSwitch ()
//
#define SYS_THREAD_SWITCH(dest)                                 \
  void __attribute__ ((regparm(1))) sys_thread_switch (dest)

//
//      MapControl ()
//
#define SYS_MAP_CONTROL(space, control)                                 \
  void __attribute__ ((regparm(2))) sys_map_control (space, control,    \
                                        ia32_exceptionframe_t * __frame)

//
//      CacheControl ()
//
#define SYS_CACHE_CONTROL(space, control)                               \
  void __attribute__ ((regparm(2))) sys_cache_control (space, control,  \
                                        ia32_exceptionframe_t * __frame)

//
//      SecurityControl ()
//
#define SYS_SECURITY_CONTROL(space, control)                            \
  void __attribute__ ((regparm(2))) sys_security_control (space,        \
                           control, ia32_exceptionframe_t * __frame)



//
//      PlatformControl ()
//
#define SYS_PLATFORM_CONTROL(control, param1, param2, param3)               \
  void __attribute__ ((regparm(3))) sys_platform_control (control, param1,  \
                                                          param2, param3,   \
                                             ia32_exceptionframe_t * __frame)

//
//      SpaceSwitch ()
//
#define SYS_SPACE_SWITCH(dest, space_specifier, utcb_location)               \
  void __attribute__ ((regparm(3))) sys_space_switch (dest, space_specifier, \
                                                      utcb_location)

//
//      Mutex ()
//
#define SYS_MUTEX(mutex_id, flags, state_p)                               \
    void __attribute__ ((regparm(3))) sys_mutex(mutex_id, flags, state_p, \
                                                ia32_exceptionframe_t * __frame)

//
//      MutexControl ()
//
#define SYS_MUTEX_CONTROL(mutex_id, control) \
    void __attribute__ ((regparm(2))) sys_mutex_control(mutex_id, control, \
                                        ia32_exceptionframe_t * __frame)

//
//      InterruptControl ()
//
#define SYS_INTERRUPT_CONTROL(thread, control)                          \
  void __attribute__ ((regparm(3))) sys_interrupt_control(thread,       \
                                                          control)


//
//      CapControl ()
//
#define SYS_CAP_CONTROL(clist, control)                                 \
  void __attribute__ ((regparm(3))) sys_cap_control(clist, control)


//
//      MemoryCopy ()
//
#define SYS_MEMORY_COPY(remote, local, size, direction)                 \
  void __attribute__ ((regparm(3))) sys_memory_copy(remote, local,      \
                                                    size,               \
                                                    direction)

/* entry functions for exceptions */

extern "C" u32_t syscall_get_kip(void);

#endif /* !__IA32__SYSCALLS_H__ */
