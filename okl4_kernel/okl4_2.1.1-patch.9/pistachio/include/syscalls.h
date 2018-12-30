/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   declaration of system calls
 */
#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <kernel/fpage.h>
#include <kernel/thread.h>
#include <kernel/arch/syscalls.h>

class spaceid_t;
class mutexid_t;
class clistid_t;

class tc_resources_t;
class security_control_t;
class plat_control_t;
class irq_control_t;
class cap_control_t;

extern "C" {

/**
 * the ipc system call
 * sys_ipc returns using the special return functions return_ipc and
 * return_ipc_error.  These have to be provided by the glue layer
 * @param to_tid destination thread id
 * @param from_tid from specifier
 */
SYS_IPC (threadid_t to_tid, threadid_t from_tid);


/**
 * thread switch system call
 * @param dest_tid thread id to switch to
 */
SYS_THREAD_SWITCH (threadid_t dest_tid);


/**
 * thread control privileged system call
 * @param dest_tid thread id of the destination thread
 * @param space_id space specifier
 * @param scheduler_tid thread id of the scheduler thread
 * @param pager_tid thread id of the pager thread
 * @param except_handler_tid thread id of the exception_handler thread
 * @param thread_resources architecture specific resource bits
 * @param utcb_location location of the UTCB
 */
SYS_THREAD_CONTROL (threadid_t dest_tid, spaceid_t space_id,
                    threadid_t scheduler_tid, threadid_t pager_tid,
                    threadid_t except_handler_tid,
                    tc_resources_t thread_resources,
                    word_t utcb_location);

/**
 * exchange registers system call
 * @param dest_tid thread id of the destination thread (always global)
 * @param control control word specifying the operations to perform
 * @param usp user stack pointer
 * @param uip user instruction pointer
 * @param uflags user flags
 * @param uhandle user defined handle
 */
SYS_EXCHANGE_REGISTERS (threadid_t dest_tid, word_t control,
                        word_t usp, word_t uip, word_t uflags,
                        word_t uhandle);


/**
 * schedule system call
 * (note: the glue layer has to provide a return_schedule macro to load
 * the return values into the appropriate registers)
 * @param dest_tid thread id of the destination thread
 * @param ts_len timeslice length
 * @param hw_thread_bitmask allow user thread to run on set of hw threads
 * @param processor_control processor number the thread migrates to
 * @param prio new priority of the thread
 * @param flags schedule system calls control flags.
 */
SYS_SCHEDULE (threadid_t dest_tid, word_t ts_len,
        word_t hw_thread_bitmsk, word_t processor_control, word_t prio,
        word_t flags);


/**
 * map control system call
 * @param space_id target address space specifier
 * @param control map control word specifying the number of
 *  mapitems to be acted on
 */
SYS_MAP_CONTROL (spaceid_t space_id, word_t control);


/**
 * space control privileged system call
 * (note: the glue layer has to provide a return_space_control macro to
 * load the return values into the appropriate registers)
 * @param space_id address space specifier
 * @param control space control parameter
 * @param clist_id clist to associate with this space
 * @param utcb_area user thread control block area fpage
 * @param space_resources address spaces resources
 */
SYS_SPACE_CONTROL (spaceid_t space_id, word_t control, clistid_t clist_id,
                   fpage_t utcb_area, word_t space_resources);

/**
 * cache control system call
 * @param space_id target address space specifier
 * @param control control word specifing the options to cache_control
 */
SYS_CACHE_CONTROL (spaceid_t space_id, word_t control);

/**
 * Security control privileged system call
 * @param space_id target address space specifier
 * @param control control word specifing the options
 */
SYS_SECURITY_CONTROL (spaceid_t space_id, word_t control);

/**
 * platform control system call
 * (note: the glue layer has to provide a return_platform_control(result)
 * macro to load the return values into the appropriate registers)
 * @param control control word
 * @param param1
 * @param param2
 * @param param3
 */
SYS_PLATFORM_CONTROL (plat_control_t control, word_t param1,
                      word_t param2, word_t param3);

/**
 * space switch system call
 * @param tid thread to modify
 * @param space_id the space to move the thread into
 * @param utcb_location the new utcb address of the thread
 */
SYS_SPACE_SWITCH(threadid_t tid, spaceid_t space_id, word_t utcb_location);

/**
 * Mutex system call.
 *
 * @param mutex_id The identifier of the mutex to modify.
 * @param flags The flags identifying the type of mutex operation to perform.
 * @param state_p The virtual address of the hybrid mutex word.
 */
SYS_MUTEX(mutexid_t mutex_id, word_t flags, word_t * state_p);

/**
 * Mutex control privileged system call
 *
 * @param mutex_id Target mutex identifier.
 * @param control Control flags for system call.
 */
SYS_MUTEX_CONTROL(mutexid_t mutex_id, word_t control);

/**
 * interrupt control system call
 * @param tid thread to modify
 * @param space_id the space to move the thread into
 * @param utcb_location the new utcb address of the thread
 */
SYS_INTERRUPT_CONTROL(threadid_t tid, irq_control_t control);

/**
 * capability control system call
 * @param clist capbility list to operate on
 * @param control control flags for cap_control
 * @param mrs arguments dependant on control flags
 */
SYS_CAP_CONTROL(clistid_t clist, cap_control_t control);

/**
 * memory copy system call
 *
 * @param space_id the space to move the thread into
 * @param utcb_location the new utcb address of the thread
 */
SYS_MEMORY_COPY(threadid_t remote, word_t local, word_t size, 
                word_t direction);

} /* extern "C" */


/*********************************************************************
 *                 control register constants
 *********************************************************************/

#define EXREGS_CONTROL_HALT     (1 << 0)
#define EXREGS_CONTROL_RECV     (1 << 1)
#define EXREGS_CONTROL_SEND     (1 << 2)
#define EXREGS_CONTROL_SP       (1 << 3)
#define EXREGS_CONTROL_IP       (1 << 4)
#define EXREGS_CONTROL_FLAGS    (1 << 5)
#define EXREGS_CONTROL_UHANDLE  (1 << 6)
//#define EXREGS_CONTROL_PAGER    (1 << 7)
#define EXREGS_CONTROL_TLS      (1 << 7)
#define EXREGS_CONTROL_HALTFLAG (1 << 8)
#define EXREGS_CONTROL_DELIVER  (1 << 9)
#define EXREGS_CONTROL_COPY_REGS        (1 << 10)
#define EXREGS_CONTROL_SUSPEND_FLAG     (1 << 11)
#define EXREGS_CONTROL_REGS_TO_MRS      (1 << 12)
#define EXREGS_CONTROL_MRS_TO_REGS      (1 << 13)
#define EXREGS_CONTROL_THREAD_SHIFT     (L4_GLOBAL_VERSION_BITS)

#define SPACE_CONTROL_NEW               (1 << 0)
#define SPACE_CONTROL_DELETE            (1 << 1)
#define SPACE_CONTROL_RESOURCES         (1 << 2)
#define SPACE_CONTROL_SPACE_PAGER       (1 << 3)

#define MUTEX_CONTROL_CREATE (1 << 0)
#define MUTEX_CONTROL_DELETE (1 << 1)


/*
 * Error code values
 */

#define EOK                     (0)
#define ENO_PRIVILEGE           (1)
#define EINVALID_THREAD         (2)
#define EINVALID_SPACE          (3)
#define EINVALID_SCHEDULER      (4)
#define EINVALID_PARAM          (5)
#define EUTCB_AREA              (6)
#define EKIP_AREA               (7)
#define ENO_MEM                 (8)
#define ESPACE_NOT_EMPTY        (9)
#define ECLIST_NOT_EMPTY        (9)
#define EINVALID_MUTEX          (10)
#define EMUTEX_BUSY             (11)
#define EINVALID_CLIST          (12)
#define EINVALID_CAP            (13)
#define EDOMAIN_CONFLICT        (15)
#define ENOT_IMPLEMENTED        (16)


#endif /* !__SYSCALLS_H__ */
