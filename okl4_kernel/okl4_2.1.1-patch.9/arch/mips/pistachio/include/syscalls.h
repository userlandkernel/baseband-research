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
 * Copyright (c) 2005-2006, National ICT Australia (NICTA)
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
 * Description:   MIPS Syscall specific macros
 * Author:        Carl van Schaik
 */

#ifndef __ARCH__MIPS__SYSCALLS_H__
#define __ARCH__MIPS__SYSCALLS_H__

#if defined(ASSEMBLY)
#define UL(x)   (x)
#else
#define UL(x)   (x##ul)
#endif

#define L4_TRAP_KPUTC                   UL(-100)
#define L4_TRAP_KGETC                   UL(-101)
#define L4_TRAP_KDEBUG                  UL(-102)
#define L4_TRAP_UNUSED                  UL(-103)
#define L4_TRAP_KGETC_NB                UL(-104)
#define L4_TRAP_READ_PERF               UL(-110)
#define L4_TRAP_WRITE_PERF              UL(-111)
#define L4_TRAP_KSET_OBJECT_NAME        UL(-120)

/* The syscall assembler depends on the values below */
#define SYSCALL_ipc                     UL(-101)
#define SYSCALL_thread_switch           UL(-102)
#define SYSCALL_thread_control          UL(-103)
#define SYSCALL_exchange_registers      UL(-104)
#define SYSCALL_schedule                UL(-105)
#define SYSCALL_map_control             UL(-106)
#define SYSCALL_space_control           UL(-107)
/* Unused system call slot:             UL(-108) */
#define SYSCALL_cache_control           UL(-109)
#define SYSCALL_security_control        UL(-110)
#define SYSCALL_interrupt_control       UL(-111)
#define SYSCALL_cap_control             UL(-112)
#define SYSCALL_platform_control        UL(-116)
#define SYSCALL_space_switch            UL(-117)
#define SYSCALL_mutex                   UL(-118)
#define SYSCALL_mutex_control           UL(-119)

#if !defined(ASSEMBLY)

//
// System call function attributes.
//

#define SYSCALL_ATTR(sec_name)                  NORETURN

#define SYS_IPC_RETURN_TYPE                     word_t
#define SYS_THREAD_CONTROL_RETURN_TYPE          word_t
#define SYS_EXCHANGE_REGISTERS_RETURN_TYPE      word_t
#define SYS_MAP_CONTROL_RETURN_TYPE             word_t
#define SYS_SPACE_CONTROL_RETURN_TYPE           word_t
#define SYS_SCHEDULE_RETURN_TYPE                word_t
#define SYS_CACHE_CONTROL_RETURN_TYPE           word_t
#define SYS_SECURITY_CONTROL_RETURN_TYPE        word_t
#define SYS_INTERRUPT_CONTROL_RETURN_TYPE       void
#define SYS_CAP_CONTROL_RETURN_TYPE             void
#define SYS_PLATFORM_CONTROL_RETURN_TYPE        word_t
#define SYS_SPACE_SWITCH_RETURN_TYPE            word_t
#define SYS_MUTEX_RETURN_TYPE                   word_t
#define SYS_MUTEX_CONTROL_RETURN_TYPE           word_t

/*
 *  Syscall declaration wrappers.
 */

#define SYS_IPC(to, from)                                       \
  SYS_IPC_RETURN_TYPE SYSCALL_ATTR ("ipc")                      \
  sys_ipc (to, from)

#define SYS_THREAD_CONTROL(dest, space, scheduler, pager,       \
                exception_handler, dummy, utcb)         \
  SYS_THREAD_CONTROL_RETURN_TYPE SYSCALL_ATTR ("thread_control")\
  sys_thread_control (dest, space, scheduler, pager,            \
                  exception_handler, dummy, utcb)

#define SYS_SPACE_CONTROL(space, control, clist, kip_area, utcb_area,  \
                space_resources)                                \
  SYS_SPACE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("space_control")  \
  sys_space_control (space, control, clist, kip_area, utcb_area,       \
                     space_resources)

#define SYS_SCHEDULE(dest, ts_len, hw_thread_bitmask,           \
                     processor_control, prio, flags)            \
  SYS_SCHEDULE_RETURN_TYPE SYSCALL_ATTR ("schedule")            \
  sys_schedule (dest, ts_len, hw_thread_bitmask,                \
                processor_control, prio, flags)

#define SYS_EXCHANGE_REGISTERS(dest, control, usp, uip, uflags, \
                        uhandle)                                \
  SYS_EXCHANGE_REGISTERS_RETURN_TYPE SYSCALL_ATTR ("exchange_registers")        \
  sys_exchange_registers (dest, control, usp, uip,              \
                          uflags, uhandle)

#define SYS_THREAD_SWITCH(dest)                                 \
  void SYSCALL_ATTR ("thread_switch")                           \
  sys_thread_switch (dest)

#define SYS_MAP_CONTROL(space, control)                         \
  SYS_MAP_CONTROL_RETURN_TYPE SYSCALL_ATTR ("map_control")      \
  sys_map_control (space, control)

#define SYS_CACHE_CONTROL(space, control)                       \
  SYS_CACHE_CONTROL_RETURN_TYPE SYSCALL_ATTR ("cache_control")  \
  sys_cache_control (space, control)

#define SYS_SECURITY_CONTROL(space, control)                    \
  SYS_SECURITY_CONTROL_RETURN_TYPE SYSCALL_ATTR ("security_control") \
  sys_security_control (space, control)

#define SYS_INTERRUPT_CONTROL(space, control)                   \
  SYS_INTERRUPT_CONTROL_RETURN_TYPE SYSCALL_ATTR ("interrupt_control") \
  sys_interrupt_control (space, control)

#define SYS_CAP_CONTROL(clist, control)                         \
  SYS_CAP_CONTROL_RETURN_TYPE SYSCALL_ATTR ("cap_control")      \
  sys_cap_control (clist, control)

#define SYS_PLATFORM_CONTROL(control, param1, param2, param3)   \
  SYS_PLATFORM_CONTROL_RETURN_TYPE SYSCALL_ATTR ("platform_control") \
  sys_platform_control (control, param1, param2, param3)

#define SYS_SPACE_SWITCH(dest, spacespecifier, utcblocation)    \
  SYS_SPACE_SWITCH_RETURN_TYPE SYSCALL_ATTR ("space_switch")\
  sys_space_switch (dest, spacespecifier, utcblocation)

#define SYS_MUTEX(mutex_id, flags)                      \
    SYS_MUTEX_RETURN_TYPE SYSCALL_ATTR ("mutex")        \
    sys_mutex(mutex_id, flags)

#define SYS_MUTEX_CONTROL(mutex_id, control)                     \
    SYS_MUTEX_CONTROL_RETURN_TYPE SYSCALL_ATTR ("mutex_control") \
    sys_mutex_control(mutex_id, control)
    
/**
 * The application gets the kernel info page by doing some illegal instruction, with 
 * at ($1) == 0x1face (interface) ca11 (call) 14 (L4) e1f (ELF) 64 (MIPS64)
 */
#if defined(L4_32BIT)
 #define MAGIC_KIP_REQUEST      (0x1faceCa1L)
#elif defined(L4_64BIT)
 #define MAGIC_KIP_REQUEST      (0x1faceCa1114e1f64ULL)
#endif

/**
 * Preload registers and return from sys_ipc
 * @param from The FROM value after the system call
 */

#define return_ipc(from)        \
    do {                        \
        ACTIVATE_CONTINUATION(TCB_SYSDATA_IPC(current)->ipc_return_continuation);\
    } while(false)


/**
 * Preload registers and return from sys_thread_control
 * @param result The RESULT value after the system call
 */

#define return_thread_control(result, cont)     \
    do {                                                        \
        register word_t rslt ASM_REG("$2") = result; /* v0 */   \
        __asm__ __volatile__ (                                  \
                "       .set    push                    \n"     \
                "       .set    noreorder               \n"     \
                "       jr      %[continuation]         \n"     \
                "       ori     $29, $29, %[st]         \n"     /* delay slot */\
                "       .set    pop                     \n"     \
                :                                               \
                : [continuation] "r" (cont),                    \
                [rslt] "r" (rslt), [st] "i" (STACK_TOP)         \
                );                                              \
        while(1);                                               \
    } while(false)

/**
 * Preload registers and return from sys_exchange_registers
 * @param result The RESULT value after the system call
 * @param control The CONTROL value after the system call
 * @param sp The SP value after the system call
 * @param ip The IP value after the system call
 * @param flags The FLAGS value after the system call
 * @param pager The PAGER value after the system call
 * @param handle The USERDEFINEDHANDLE value after the system call
 */

#define return_exchange_registers(result, control, sp, ip, flags, pager, handle, continuation)    \
    do {                                                                \
        register word_t rslt ASM_REG("$2") = (result).get_raw(); /* v0 */\
        register word_t ctrl ASM_REG("$5") = control;   /* a1 */        \
        register word_t sp_r ASM_REG("$6") = sp;        /* a2 */        \
        register word_t ip_r ASM_REG("$7") = ip;        /* a3 */        \
        register word_t flg ASM_REG("$8") = flags;      /* t0 */        \
        register word_t hdl ASM_REG("$9") = handle;     /* t1 */        \
        register word_t pgr ASM_REG("$10") = pager.get_raw();    /* t2 */\
\
        __asm__ __volatile__ (                                          \
                "       .set    push                    \n"             \
                "       .set    noreorder               \n"             \
                "       jr      %[cont]                 \n"             \
                "       ori     $29, $29, %[st]         \n"     /* delay slot */\
                "       .set    pop                     \n"             \
                :                                                       \
                : [cont] "r" (continuation),                            \
                "r" (rslt), "r" (ctrl), "r" (sp_r),                     \
                "r" (ip_r), "r" (flg), "r" (pgr), "r" (hdl),            \
                [st] "i" (STACK_TOP)                                    \
                );                                                      \
        while(1);                                                       \
    } while(false)


/**
 * Return from sys_thread_switch
 */
#define return_thread_switch(cont)  \
    do {                            \
        ACTIVATE_CONTINUATION(cont);\
    } while(false)


/**
 * Return from sys_map_control
 */
#define return_map_control(result, cont)        return_thread_control(result, cont)



/**
 * Preload registers and return from sys_thread_switch
 * @param result The RESULT value after the system call
 * @param old_rsc The OLD_RESOURCES value after the system call
 */
#define return_space_control(result, old_res, cont)             \
    do {                                                        \
        register word_t rslt ASM_REG("$2") = result; /* v0 */   \
        register word_t oldr ASM_REG("$4") = old_res;/* a0 */   \
        __asm__ __volatile__ (                                  \
                "       .set    push                    \n"     \
                "       .set    noreorder               \n"     \
                "       jr      %[continuation]         \n"     \
                "       ori     $29, $29, %[st]         \n"     /* delay slot */\
                "       .set    pop                     \n"     \
                :                                               \
                :[continuation] "r" (cont),                     \
                "r" (rslt), "r" (oldr),                         \
                [st] "i" (STACK_TOP)                            \
                );                                              \
        while(1);                                               \
    } while(false)


/**
 * Preload registers and return from sys_schedule
 * @param result The RESULT value after the system call
 * @param time_control The TIME_CONTROL value after the system call
 */
#define return_schedule(result, rem_ts, continuation)                         \
    do {                                                                      \
        register word_t rslt ASM_REG("$2") = result;    /* v0 */              \
        register word_t remts ASM_REG("$5") = rem_ts;   /* a1 */              \
        __asm__ __volatile__ (                                                \
                "       .set    push             \n"                          \
                "       .set    noreorder        \n"                          \
                "       jr      %[cont]          \n"                          \
                "       ori     $29, $29, %[st]  \n"    /* delay slot */      \
                "       .set    pop              \n"                          \
                :                                                             \
                : [cont] "r" (continuation),                                  \
                  "r" (rslt), "r" (remts),                                    \
                  [st] "i" (STACK_TOP)                                        \
                );                                                            \
        while (1);                                                            \
    } while (false)

/**
 * Return from sys_cache_control
 */
#define return_cache_control(result, cont)      return_thread_control(result, cont)

/**
 * Return from sys_security_control
 */
#define return_security_control(result, cont)   return_thread_control(result, cont)

/**
 * Return from sys_interrupt_control
 */
#define return_interrupt_control(result, cont)  return_thread_control(result, cont)

/**
 * Return from sys_cap_control
 */
#define return_cap_control(result, cont)        return_thread_control(result, cont)

/**
 * Return from sys_platform_control
 */
#define return_platform_control(result, cont)   return_thread_control(result, cont)

/**
 * Return from sys_space_switch
 */
#define return_space_switch(result, cont)       return_thread_control(result, cont)

/**
 * Return from sys_mutex
 */
#define return_mutex(result, cont)              \
    return_thread_control(result, cont)

/**
 * Return from sys_mutex_control
 */
#define return_mutex_control(result, cont)      \
    return_thread_control(result, cont)


#endif /* !ASSEMBLY */

#endif /* !__ARCH__MIPS__SYSCALLS_H__ */
