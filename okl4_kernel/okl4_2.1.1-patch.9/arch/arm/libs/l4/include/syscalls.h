/*
 * Copyright (c) 2003-2004, National ICT Australia (NICTA)
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
 * Description:   ARM system call ABI
 */

#ifndef __L4__ARM__SYSCALLS_H__
#define __L4__ARM__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

/*
 * L4_KernelInterface
 */
void *L4_KernelInterface(L4_Word_t *ApiVersion,
                         L4_Word_t *ApiFlags, L4_Word_t *KernelId);

/*
 * L4_ExchangeRegisters
 */
L4_ThreadId_t L4_ExchangeRegisters(L4_ThreadId_t dest,
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
                                   L4_ThreadId_t *old_pager);

/*
 * L4_ThreadControl
 */
L4_Word_t L4_ThreadControl(L4_ThreadId_t dest,
                           L4_SpaceId_t SpaceSpecifier,
                           L4_ThreadId_t Scheduler,
                           L4_ThreadId_t Pager,
                           L4_ThreadId_t ExceptionHandler,
                           L4_Word_t resources, void *UtcbLocation);

/*
 * L4_ThreadSwitch
 */
void L4_ThreadSwitch(L4_ThreadId_t dest);

/*
 * L4_Schedule
 */
L4_Word_t L4_Schedule(L4_ThreadId_t dest,
                      L4_Word_t timeslice,
                      L4_Word_t hw_thread_bitmask,
                      L4_Word_t ProcessorControl,
                      L4_Word_t prio,
                      L4_Word_t flags,
                      L4_Word_t *rem_timeslice);

/*
 * L4_Ipc
 */
L4_MsgTag_t L4_Ipc(L4_ThreadId_t to,
                   L4_ThreadId_t FromSpecifier,
                   L4_MsgTag_t tag, L4_ThreadId_t *from);

/*
 * L4_Notify
 */
L4_MsgTag_t L4_Notify(L4_ThreadId_t to, L4_Word_t mask);

/*
 * L4_WaitNotify
 */
L4_MsgTag_t L4_WaitNotify(L4_Word_t *mask);

/*
 * L4_Lipc
 */
L4_MsgTag_t L4_Lipc(L4_ThreadId_t to,
                    L4_ThreadId_t FromSpecifier,
                    L4_MsgTag_t tag, L4_ThreadId_t *from);

/*
 * L4_MapControl
 */
L4_Word_t L4_MapControl(L4_SpaceId_t SpaceSpecifier, L4_Word_t control);

/*
 * L4_SpaceControl
 */
L4_Word_t L4_SpaceControl(L4_SpaceId_t SpaceSpecifier,
                          L4_Word_t control,
                          L4_ClistId_t clist,
                          L4_Fpage_t UtcbArea,
                          L4_Word_t resources,
                          L4_Word_t *old_resources);

/*
 * L4_CacheControl
 */
L4_Word_t L4_CacheControl(L4_SpaceId_t SpaceSpecifier, L4_Word_t control);

/*
 * L4_IpcControl
 */
L4_Word_t L4_SecurityControl(L4_SpaceId_t SpaceSpecifier, L4_Word_t control);

/*
 * L4_PlatformControl
 */
L4_Word_t L4_PlatformControl(L4_Word_t control, L4_Word_t param1,
                             L4_Word_t param2, L4_Word_t param3);

/*
 * L4_SpaceSwitch
 */
L4_Word_t L4_SpaceSwitch(L4_ThreadId_t dest, L4_SpaceId_t SpaceSpecifier,
                         void *UtcbLocation);

/*
 * L4_Mutex
 */
word_t L4_Mutex(L4_MutexId_t MutexSpecifier, L4_Word_t flags, word_t * state_p);

/*
 * L4_MutexControl
 */
L4_Word_t L4_MutexControl(L4_MutexId_t MutexSpecifier, L4_Word_t control);


/*
 * L4_InterruptControl
 */
L4_Word_t L4_InterruptControl(L4_ThreadId_t dest, L4_Word_t control);

/*
 * L4_CapControl
 */
L4_Word_t L4_CapControl(L4_ClistId_t clist, L4_Word_t control);

/*
 * L4_MemoryCopy
 */
L4_Word_t L4_MemoryCopy(L4_ThreadId_t remote, L4_Word_t local, L4_Word_t *size,
                        L4_Word_t direction);

#endif /* !__L4__ARM__SYSCALLS_H__ */
