/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description:   ARM C syscall entry and exit
 */
#ifndef __ARM__SYSCALLS_H__
#define __ARM__SYSCALLS_H__

#include <kernel/arch/asm.h>
#include <kernel/arch/config.h>
#include <kernel/arch/ver/utcb.h>
#include <l4/arch/syscalls_asm.h> /* Syscall numbers. */

#ifndef ASSEMBLY
#include <kernel/thread.h> // For IPC C fastpath
#endif

/* Upper bound on syscall number */
#define SYSCALL_limit           SYSCALL_last

/*
 * attributes for system call functions
 */
#if defined(_lint)
#define SYSCALL_ATTR(x) 
#else
#define SYSCALL_ATTR(x) NORETURN
#endif

#define SYS_IPC_RETURN_TYPE                     void
#define SYS_THREAD_CONTROL_RETURN_TYPE          void
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE      void
#define SYS_MAP_CONTROL_RETURN_TYPE             void
#define SYS_SPACE_CONTROL_RETURN_TYPE           void
#define SYS_SCHEDULE_RETURN_TYPE                void
#define SYS_CACHE_CONTROL_RETURN_TYPE           void
#define SYS_THREAD_SWITCH_RETURN_TYPE           void
#define SYS_SECURITY_CONTROL_RETURN_TYPE        void
#define SYS_PLATFORM_CONTROL_RETURN_TYPE        void
#define SYS_SPACE_SWITCH_RETURN_TYPE            void
#define SYS_MUTEX_RETURN_TYPE                   void
#define SYS_MUTEX_CONTROL_RETURN_TYPE           void
#define SYS_INTERRUPT_CONTROL_RETURN_TYPE       void
#define SYS_CAP_CONTROL_RETURN_TYPE             void
#define SYS_MEMORY_COPY_RETURN_TYPE             void

/*
 * Syscall declaration wrappers.
 */

#define SYS_IPC(to, from)                                       \
  SYS_IPC_RETURN_TYPE /*SYSCALL_ATTR ("ipc")*/                  \
  sys_ipc (to, from)

#ifdef CONFIG_IPC_C_FASTPATH
#define SYS_IPC_FAST(to, from)                                  \
  SYS_IPC_RETURN_TYPE /*SYSCALL_ATTR ("ipc")*/                  \
  sys_ipc_c_fastpath(to, from)
#ifndef ASSEMBLY
extern "C" SYS_IPC_FAST(threadid_t to_tid, threadid_t from_tid);
#endif
#endif

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager,       \
                except_handler, thread_resources, utcb)         \
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")\
  sys_thread_control (dest, space, scheduler, pager,            \
                      except_handler, thread_resources, utcb)

#define SYS_SPACE_CONTROL(space, control, clist, utcb_area,  \
                space_resources)                                \
  SYS_SPACE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("space_control")  \
  sys_space_control (space, control, clist, utcb_area, space_resources)

#define SYS_SCHEDULE(dest, ts_len, hw_thread_bitmask,           \
                     processor_control, prio, flags)            \
  SYS_SCHEDULE_RETURN_TYPE SYSCALL_ATTR ("schedule")            \
  sys_schedule (dest, ts_len, hw_thread_bitmask,                \
          processor_control, prio, flags)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip, uflags, \
                        uhandle)                                \
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")\
  sys_exchange_registers (dest, control, usp, uip,              \
                          uflags, uhandle)

#define SYS_THREAD_SWITCH(dest)                                 \
  SYS_THREAD_SWITCH_RETURN_TYPE SYSCALL_ATTR ("thread_switch")  \
  sys_thread_switch (dest)

#define SYS_MAP_CONTROL(space, control)                         \
  SYS_MAP_CONTROL_RETURN_TYPE SYSCALL_ATTR ("map_control")      \
  sys_map_control (space, control)

#define SYS_CACHE_CONTROL(space, control)                       \
  SYS_CACHE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("cache_control")  \
  sys_cache_control (space, control)

#define SYS_SECURITY_CONTROL(space, control)                    \
    SYS_SECURITY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("security_control")\
    sys_security_control (space, control)

#define SYS_PLATFORM_CONTROL(control, param1, param2, param3)   \
  SYS_PLATFORM_CONTROL_RETURN_TYPE SYSCALL_ATTR ("platform_control")\
  sys_platform_control (control, param1, param2, param3)

#define SYS_SPACE_SWITCH(thread, space, utcb_location)          \
  SYS_SPACE_SWITCH_RETURN_TYPE SYSCALL_ATTR ("space_switch")    \
  sys_space_switch(thread, space, utcb_location)

#define SYS_MUTEX(mutex_id, flags, state_p)             \
    SYS_MUTEX_RETURN_TYPE SYSCALL_ATTR("mutex")         \
    sys_mutex(mutex_id, flags, state_p)

#define SYS_MUTEX_CONTROL(mutex_id, control)            \
    SYS_MUTEX_RETURN_TYPE SYSCALL_ATTR("mutex_control") \
    sys_mutex_control(mutex_id, control)

#define SYS_INTERRUPT_CONTROL(thread, control)                  \
  SYS_INTERRUPT_CONTROL_RETURN_TYPE SYSCALL_ATTR ("interrupt_control") \
  sys_interrupt_control(thread, control)

#define SYS_CAP_CONTROL(clist, control)                         \
  SYS_CAP_CONTROL_RETURN_TYPE SYSCALL_ATTR ("cap_control")      \
  sys_cap_control(clist, control)

#define SYS_MEMORY_COPY(remote, local, size, direction)          \
  SYS_MEMORY_COPY_RETURN_TYPE SYSCALL_ATTR ("memory_copy") \
  sys_memory_copy(remote, local, size, direction)

#if defined(_lint)
void empty(word_t foo);
#define return_platform_control(result, cont) cont(); empty (result); return
#define return_cache_control(result, cont) cont(); empty (result); return
#define return_schedule(result, rem_ts, cont) cont(); empty(rem_ts); empty(result); return
#define return_ipc() return 
#define return_exchange_registers(result, control, sp, ip, flags, pager, handle, cont) cont(); empty(control); empty (result.get_raw()); return
#define return_thread_control(result, cont) cont(); empty (result); return
#define return_thread_switch(cont) cont(); return
#define return_map_control(result, cont) cont(); empty (result); return
#define return_space_control(result, space_resources, cont) cont(); empty(space_resources); empty (result); return
#define return_security_control(result, cont) cont(); empty (result)
#define return_space_switch(result, cont) cont(); empty (result); return
#define return_mutex(result, cont) cont(); empty(result); return
#define return_mutex_control(result, cont) cont(); empty(result); return
#define return_interrupt_control(result, cont) cont(); empty (result); return
#define return_cap_control(result, cont) cont(); empty (result); return
#define return_memory_copy(result, size, cont) cont(); empty(result); empty(size); return
#elif defined(__GNUC__)

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */
#define return_ipc() \
    do {                                                \
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);    \
    } while(false)

/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */
#define return_thread_control(result, cont) {           \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Preload registers and return from sys_exchange_registers
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 * @param _sp The SP value after the system call
 * @param ip The IP value after the system call
 * @param flags The FLAGS value after the system call
 * @param pager The PAGER value after the system call
 * @param handle The USERDEFINEDHANDLE value after the system call
 */
#define return_exchange_registers(result, control, _sp, ip, flags, pager, handle, continuation)\
{                                                                       \
    register word_t rslt    ASM_REG("r0") = (result).get_raw();         \
    register word_t ctrl    ASM_REG("r1") = control;                    \
    register word_t sp_r    ASM_REG("r2") = _sp;                        \
    register word_t ip_r    ASM_REG("r3") = ip;                         \
    register word_t fl_r    ASM_REG("r4") = flags;                      \
    register word_t hdl_r   ASM_REG("r5") = handle;                     \
    register word_t pgr_r   ASM_REG("r6") = (pager).get_raw();          \
                                                                        \
    __asm__ __volatile__ (                                              \
        CHECK_ARG("r0", "%1")                                           \
        CHECK_ARG("r1", "%2")                                           \
        CHECK_ARG("r2", "%3")                                           \
        CHECK_ARG("r3", "%4")                                           \
        CHECK_ARG("r4", "%5")                                           \
        CHECK_ARG("r5", "%6")                                           \
        CHECK_ARG("r6", "%7")                                           \
        "orr    sp,     sp,     %8      \n"                             \
        "mov    pc,     %0              \n"                             \
        :: "r"  (continuation),                                         \
           "r" (rslt), "r" (ctrl), "r" (sp_r),                          \
           "r" (ip_r), "r" (fl_r), "r" (hdl_r),                         \
           "r" (pgr_r), "i" (STACK_TOP)                                 \
    );                                                                  \
    while (1);                                                          \
}

/**
 * Return from sys_thread_switch
 */
#define return_thread_switch(cont)                                      \
{                                                                       \
    ACTIVATE_CONTINUATION(cont);                                        \
}

/**
 * Return from sys_map_control
 * @param result The RESULT value after the system call
 */
#define return_map_control(result, cont) {              \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Preload registers and return from sys_space_control
 * @param result The RESULT value after the system call
 * @param space_resources The original SPACE_RESOURCES value from before the system call
 * @param cont The continuation function to jump to
 */
#define return_space_control(result, space_resources, cont) {           \
    register word_t rslt    ASM_REG("r0") = result;                     \
    register word_t spcres  ASM_REG("r1") = space_resources;            \
                                                                        \
    __asm__ __volatile__ (                                              \
        CHECK_ARG("r0", "%0")                                           \
        CHECK_ARG("r1", "%1")                                           \
        "orr    sp,     sp,     %3      \n"                             \
        "mov    pc,     %2              \n"                             \
        :: "r" (rslt), "r" (spcres),                                    \
            "r" (cont), "i" (STACK_TOP)                                 \
    );                                                                  \
    while (1);                                                          \
}

/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 */
#define return_schedule(result, rem_ts, continuation) {                 \
    register word_t rslt    ASM_REG("r0") = result;                     \
    register word_t remts   ASM_REG("r1") = rem_ts;                     \
                                                                        \
    __asm__ __volatile__ (                                              \
        CHECK_ARG("r0", "%1")                                           \
        CHECK_ARG("r1", "%2")                                           \
        "orr    sp,     sp,     %3      \n"                             \
        "mov    pc,     %0              \n"                             \
        :: "r" (continuation),                                         \
           "r" (rslt), "r" (remts),                                     \
           "i" (STACK_TOP)                                              \
    );                                                                  \
    while (1);                                                          \
}

/**
 * Return from sys_cache_control
 */
#define return_cache_control(result, cont) {            \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_security_control
 */
#define return_security_control(result, cont)           \
{                                                       \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_platform_control
 * @param result The RESULT value after the system call
 */
#define return_platform_control(result, cont) {         \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_space_switch
 * @param result The RESULT value after the system call
 */
#define return_space_switch(result, cont) {             \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_mutex
 *
 * @param result The RESULT value after the system call
 */
#define return_mutex(result, cont) {                    \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_mutex_control
 *
 * @param result The RESULT value after the system call
 */
#define return_mutex_control(result, cont) {            \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
            "r" (cont), "i" (STACK_TOP)                 \
    );                                                  \
    while (1);                                          \
}


/**
 * Return from sys_interrupt_control_
 * @param result The RESULT value after the system call
 */
#define return_interrupt_control(result, cont) {        \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
           "r" (cont), "i" (STACK_TOP)                  \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_cap_control
 * @param result The RESULT value after the system call
 */
#define return_cap_control(result, cont) {              \
    register word_t rslt    ASM_REG("r0") = result;     \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        "orr    sp,     sp,     %2      \n"             \
        "mov    pc,     %1              \n"             \
        :: "r" (rslt),                                  \
           "r" (cont), "i" (STACK_TOP)                  \
    );                                                  \
    while (1);                                          \
}

/**
 * Return from sys_interrupt_control_
 * @param result The RESULT value after the system call
 */
#define return_memory_copy(result, size, cont) {        \
    register word_t rslt    ASM_REG("r0") = result;     \
    register word_t siz     ASM_REG("r1") = size;       \
                                                        \
    __asm__ __volatile__ (                              \
        CHECK_ARG("r0", "%0")                           \
        CHECK_ARG("r1", "%1")                           \
        "orr    sp,     sp,     %3      \n"             \
        "mov    pc,     %2              \n"             \
        :: "r" (rslt), "r" (siz),                       \
           "r" (cont), "i" (STACK_TOP)                  \
    );                                                  \
    while (1);                                          \
}

#elif defined(__RVCT_GNU__)

#if !defined(ASSEMBLY)
NORETURN void asm_return_syscall(word_t arg0, word_t arg1, word_t arg2, word_t continuation);
NORETURN void asm_return_exchange_registers(word_t result, word_t control, word_t sp, word_t ip, word_t flags, word_t handle, word_t pager, word_t continuation);
#endif

/**
 * Return from sys_ipc
 */
#define return_ipc()    \
    asm_return_syscall(0, 0, 0, (word_t)TCB_SYSDATA_IPC(current)->ipc_return_continuation);

/**
 * Return from sys_thread_control
 */
#define return_thread_control(result, cont)     \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

#define return_exchange_registers(result, control, sp, ip, flags, pager, handle, continuation) \
    asm_return_exchange_registers(result.get_raw(), control, sp, ip, flags, handle, pager.get_raw(), (word_t)continuation)

/**
 * Return from sys_thread_switch
 */
#define return_thread_switch(cont)      \
    asm_return_syscall(0, 0, 0, (word_t)cont);

/**
 * Return from sys_map_control
 */
#define return_map_control(result, cont)  \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_space_control
 */
#define return_space_control(result, control, cont) \
    asm_return_syscall((word_t)result, (word_t) control, 0, (word_t)cont);

/**
 * Return from sys_schedule
 */
#define return_schedule(result, rem_ts, continuation) \
    asm_return_syscall((word_t)result, (word_t)rem_ts, 0, (word_t)continuation);

/**
 * Return from sys_cache_control
 */
#define return_cache_control(result, cont)      \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_security_control
 */
#define return_security_control(result, cont)        \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_platform_control
 */
#define return_platform_control(result, cont)        \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_space_switch
 */
#define return_space_switch(result, cont)           \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_mutex
 */
#define return_mutex(result, cont)\
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_mutex_control
 */
#define return_mutex_control(result, cont)\
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_interrupt_control
 */
#define return_interrupt_control(result, cont)      \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

/**
 * Return from sys_cap_control
 */
#define return_cap_control(result, cont)            \
    asm_return_syscall((word_t)result, 0, 0, (word_t)cont);

#define return_memory_copy(result, size, cont)      \
    asm_return_syscall((word_t)result, size, 0, (word_t)cont);

#endif

#endif /* __ARM__SYSCALLS_H__ */
