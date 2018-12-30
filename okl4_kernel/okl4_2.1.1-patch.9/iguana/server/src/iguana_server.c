/*
 * Copyright (c) 2004, National ICT Australia
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
 * Authors: Ben Leslie, Alex Webster
 * Created: Tue Jul 6 2004
 *
 * Description: The main iguana interface
 */
#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <interfaces/iguana_serverdecls.h>
#include <l4e/map.h>
#include <l4e/misc.h>
#include <l4/kdebug.h>
#include <l4/interrupt.h>
#include "clist.h"
#include "debug.h"
#include "env.h"
#include "exception.h"
#include "util.h"
#include "mutex.h"
#include "trace.h"
#include "objtable.h"
#include "pd.h"
#include "pgtable.h"
#include "physmem.h"
#include "security.h"
#if defined(CONFIG_SESSION)
#include "session.h"
#endif
#include "stats.h"
#include "thread.h"
#include "virtmem.h"
#include "vm.h"
#if defined(CONFIG_ZONE)
#include "zone.h"
#endif
#include <ig_server/arch/exceptions.h>

#define L4_REQUEST_MASK         ( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))
#define L4_PAGEFAULT            (-(2UL << 20))

static int
try_attach(struct pd *pd, struct memsection *ms)
{
    uintptr_t interfaces;
    int r;
    int rwx = L4_NoAccess;

    /* Find out what caps to the memsection the PD holds. */
    interfaces = security_check(pd, (uintptr_t)ms);
    if (interfaces & (1UL << READ_IID)) {
        rwx |= L4_Readable;
    }
    if (interfaces & (1UL << WRITE_IID)) {
        rwx |= L4_Writable;
    }
    if (interfaces & (1UL << EXECUTE_IID)) {
        rwx |= L4_eXecutable;
    }

    if (!rwx) {
        return L4_NoAccess;
    }

    r = pd_attach(pd, ms, rwx);
    if (!r) {
        return rwx;
    }

    return L4_NoAccess;
}


#if defined(IG_DEBUG_PRINT)
#define R_STR(rwx)              ((rwx) & 0x4 ? "R" : "")
#define W_STR(rwx)              ((rwx) & 0x2 ? "W" : "")
#define X_STR(rwx)              ((rwx) & 0x1 ? "X" : "")

static const char *
lookup_ms_name(uintptr_t addr)
{
    struct memsection *elf_ms;
    const envitem_t *item = NULL;
    struct memsection *ms = objtable_lookup((void *)addr);
    const char *name = "???";

    while ((item = env_get_next(item)) == NULL) {
        if (env_type(item) == ENV_MEMSECTION &&
                (struct memsection *)item->u.memsection.ref.memsection == ms) {
            name = item->name;
        } else if (env_type(item) == ENV_ELF_SEGMENT) {
            elf_ms = objtable_lookup((void *)item->u.elf_segment.vaddr);
            if (elf_ms == ms) {
                name = item->name;
            }
        }
    }
    return name;
}
#endif

void
iguana_ex_pagefault_impl(L4_ThreadId_t caller, uintptr_t addr, uintptr_t ip,
                         uintptr_t rwx, idl4_fpage_t *fp,
                         idl4_server_environment *env)
{
#if defined(CONFIG_MEM_PROTECTED)
    int extensions_active;
#endif
    struct memsection *ms = NULL;
    struct pd *pd = NULL;
    int r = -1;
    uintptr_t sp;
    struct thread *thread = NULL;

    /* Work out which Iguana thread caused the exception. */
    thread = thread_lookup(caller);
    if (!thread) {
        goto error;
    }

    /* Find the protection domain the thread belongs to. */
    pd = pd_lookup(L4_SenderSpace());
    if (!pd) {
        goto error;
    }
#if defined(CONFIG_MEM_PROTECTED)
    extensions_active = L4_IsSpaceEqual(L4_SenderSpace(), pd_ext_l4_space(pd));
#endif
    /* Check that there is a memsection attached at the fault address. */
    ms = objtable_lookup((void *)addr);
    if (ms == NULL) {
        goto error;
    }

    /* For protected memsections, check whether extensions are active. */
#if defined(CONFIG_MEM_PROTECTED)
    if ((ms->flags & MEM_PROTECTED) && !extensions_active) {
        goto error;
    }
#endif
    r = pd_map_lookup(pd, ms);
#if defined(CONFIG_MEM_PROTECTED)
    if (extensions_active) {
        goto error;
    }
#endif
    if ((r & rwx) != rwx) {
        /* For legacy reasons, we try to attach the memsection here. */
        r = try_attach(pd, ms);
        if ((r & rwx) != rwx) {
            goto error;
        }
    }

    /* Call the registered page fault handler. */
#if defined(CONFIG_MEMLOAD)
    r = HANDLE_PAGE_FAULT(ms, addr, ip, rwx, pd, thread);
    if (r) {
        goto error;
    }
#endif
    /* Success. */
    return;
error:
    (void)thread_stop(thread, &ip, &sp);

    DEBUG_PRINT(ANSI_RED "Unhandled page fault:\n" ANSI_NORMAL);
    DEBUG_PRINT("  addr=0x%lx/%s priv=%s%s%s\n", (long) addr, lookup_ms_name(addr), R_STR(rwx), W_STR(rwx), X_STR(rwx));
    DEBUG_PRINT("  ip=0x%lx, sp=%lx\n", (long) ip, (long) sp);
    DEBUG_PRINT("  pd=%p thread=0x%lx\n", pd, caller.raw);

    /* XXX: Break in here to examine the thread. */
    // L4_KDB_Enter("thread_delete");

    thread_delete(thread);
    idl4_set_no_response(env);
}

static struct pd *
get_pd(CORBA_Object _caller, const objref_t pd_ref)
{
    struct pd *the_pd;

    the_pd = (struct pd *) (pd_ref & ~IID_MASK);

    if (the_pd == NULL || !is_pd(the_pd)) {
        return NULL;
    }

    return the_pd;
}

static struct memsection *
get_memsection(CORBA_Object _caller, const objref_t memsect_ref)
{
    struct memsection *the_memsect =
        (struct memsection *) (memsect_ref & ~IID_MASK);

    if (the_memsect == NULL || !is_memsection(the_memsect)) {
        return NULL;
    }

    return the_memsect;
}

static struct thread *
get_thread(CORBA_Object _caller, const objref_t thread_ref)
{
    struct thread *the_thread =
        (struct thread *) (thread_ref & ~IID_MASK);

    if (!is_thread(the_thread)) {
        return NULL;
    }

    return the_thread;
}

static struct physmem *
get_physmem(CORBA_Object _caller, const objref_t physmem_ref)
{
    struct physmem *the_physmem;

    if (physmem_ref == INVALID_ADDR) {
        return NULL;
    }

    the_physmem = (struct physmem *) (physmem_ref & ~IID_MASK);

    return the_physmem;
}

objref_t
iguana_pd_mypd_impl(CORBA_Object _caller, idl4_server_environment * _env)
{
    struct thread *thread;
    struct pd *pd;

    thread = thread_lookup(_caller);
    assert(is_thread(thread));
    pd = thread->owner;
    assert(is_pd(pd));

    return (objref_t)pd;
}

cap_t
iguana_pd_create_memsection_impl(CORBA_Object _caller, objref_t pd,
                                 uintptr_t size, uintptr_t base, int flags,
                                 physpool_ref_t physpool,
                                 objref_t virtpool_or_zone,
                                 clist_ref_t clist, uintptr_t *out_base,
                                 idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct memsection *memsection;
    struct pd *owner;
    unsigned int _flags = (unsigned int)flags;
    int r;
    struct virtpool *virtpool = NULL;
#if defined(CONFIG_ZONE)
    struct zone *zone = NULL;
#endif
    /*
     * FIXME: We should change Iguana such that flags isn't a const
     */

    /* Only ever allow user to set valid flags */
    /*
     * FIXME: We should really check for invalid bits here, rather than just
     * masking them off. If a user tries to do something stupid they should get
     * an error.
     */
    _flags &= MEM_VALID_USER_FLAGS;

    owner = get_pd(_caller, pd);

    if (owner == NULL) {
        goto error;
    }
#if defined(CONFIG_ZONE)
    if (is_zone((struct zone *)virtpool_or_zone)) {
        zone = (struct zone *)virtpool_or_zone;
        _flags |= MEM_ZONE;
    } else
#endif
    {
        virtpool = (struct virtpool *)virtpool_or_zone;
    }

    memsection = pd_create_memsection(owner, size, base,
                                      (struct physpool *)physpool, virtpool,
#if defined(CONFIG_ZONE)
                                      zone,
#else
                                      NULL,
#endif
                                      _flags);
    if (memsection != NULL) {
        *out_base = memsection->base;
        cap.ref.memsection = (memsection_ref_t)memsection;
        r = server_clist_insert((struct clist *)clist, cap);
        if (r != 0) {
            memsection_delete(memsection);
            goto error;
        }
    } else {
        goto error;
    }
    return cap;
error:
    *out_base = 0;
    cap.ref.memsection = 0;
    return cap;
}

cap_t
iguana_pd_create_thread_impl(CORBA_Object _caller, const pd_ref_t pd,
                             const int priority, clist_ref_t clist,
                             L4_ThreadId_t *l4_id,
                             idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct thread *thread;
    struct pd *owner;

    owner = get_pd(_caller, pd);
    *l4_id = L4_nilthread;

    if (owner != NULL) {
        thread = pd_create_thread(owner, priority);
        if (thread != NULL) {
            *l4_id = thread->id;
            cap.ref.thread = (thread_ref_t)thread;
        }
    }

    return cap;
}

#if defined(CONFIG_EAS)
cap_t
iguana_pd_create_eas_impl(CORBA_Object _caller, const pd_ref_t pd,
                          L4_Fpage_t *utcb, clist_ref_t clist, 
#if defined(ARM_PID_RELOC)
                          int pid,
#endif
                          L4_SpaceId_t *l4_id,
                          idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct eas *eas;
    struct pd *owner;

    owner = get_pd(_caller, pd);

    eas = pd_create_eas(owner, *utcb, pid, l4_id);

    cap.ref.eas = (eas_ref_t)eas;
    return cap;
}
#endif

cap_t
iguana_pd_create_pd_impl(CORBA_Object _caller, const pd_ref_t pd,
                         clist_ref_t clist, idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct pd *newpd;
    struct pd *owner;

    owner = get_pd(_caller, pd);

    newpd = pd_create(owner, 0 /* Max threads */ );
    cap.ref.pd = (pd_ref_t)newpd;
    return cap;
}

int
iguana_pd_attach_impl(L4_ThreadId_t caller, pd_ref_t pd, memsection_ref_t ms, 
                      int rwx, idl4_server_environment *env)
{
    int r;
    struct memsection *_ms;
    struct pd *_pd;
    uintptr_t interfaces;

    _pd = get_pd(caller, pd);
    _ms = get_memsection(caller, ms);

    if (_pd == NULL || _ms == NULL) {
        return -1;
    }

    rwx &= L4_FullyAccessible;
    interfaces = security_check(pd_lookup(L4_SenderSpace()), (uintptr_t)_ms);
    if (rwx & L4_Readable) {
        if ((interfaces & (1UL << READ_IID)) == 0) {
            rwx ^= L4_Readable;
        }
    }
    if (rwx & L4_Writable) {
        if ((interfaces & (1UL << WRITE_IID)) == 0) {
            rwx ^= L4_Writable;
        }
    }
    if (rwx & L4_eXecutable) {
        if ((interfaces & (1UL << EXECUTE_IID)) == 0) {
            rwx ^= L4_eXecutable;
        }
    }

    if (rwx == 0) {
        return -1;
    }

    r = pd_attach(_pd, _ms, rwx);
    return r;
}

void
iguana_pd_detach_impl(L4_ThreadId_t caller, pd_ref_t pd, memsection_ref_t ms, 
                      idl4_server_environment *env)
{
    struct memsection *_ms = get_memsection(caller, ms);
    struct pd *_pd         = get_pd(caller, pd);

    if (_pd == NULL || _ms== NULL) {
        return;
    }

    pd_detach(_pd, _ms);
}

#if defined(CONFIG_ZONE)
int
iguana_pd_attach_zone_impl(L4_ThreadId_t caller, pd_ref_t pd, zone_ref_t zone,
                      int rwx, idl4_server_environment *env)
{
    return pd_attach_zone((struct pd *)pd, (struct zone *)zone, rwx);
}

void
iguana_pd_detach_zone_impl(L4_ThreadId_t caller, pd_ref_t pd, zone_ref_t zone,
                           idl4_server_environment *env)
{
    pd_detach_zone((struct pd *)pd, (struct zone *)zone);
}
#endif

L4_SpaceId_t
iguana_pd_l4id_impl(L4_ThreadId_t caller, pd_ref_t pd,
                    idl4_server_environment *env)
{
    struct pd *_pd;

    _pd = get_pd(caller, pd);
    return _pd->space.id;
}

#if defined(CONFIG_MEM_PROTECTED)
L4_SpaceId_t
iguana_pd_ext_l4id_impl(L4_ThreadId_t caller, pd_ref_t pd,
                    idl4_server_environment *env)
{
    struct pd *_pd;

    _pd = get_pd(caller, pd);
    return pd_ext_l4_space(_pd);
}

// FIXME: Does not work for ia32 and is a bad approach anyway, rework - AGW. 
uintptr_t
iguana_pd_ext_l4utcb_impl(L4_ThreadId_t caller, pd_ref_t pd,
                          idl4_server_environment * env)
{
    struct pd * _pd;
    L4_Word_t utcb_addr;

    _pd = get_pd(caller, pd);
#if defined(ARCH_ARM) && ARCH_VER <= 5
    utcb_addr = -1UL;
#else
    //return (void *)pd_ext_l4_utcb(_pd); // FIXME: clean up - AGW.
    utcb_addr = _pd->extension ? L4_Address(_pd->extension->utcb_area) : 0;    
#endif
    return utcb_addr;
}
#endif

#if defined(CONFIG_SESSION)
cap_t
iguana_pd_create_session_impl(CORBA_Object _caller, const pd_ref_t pd,
                              const thread_ref_t client,
                              const thread_ref_t server,
                              const clist_ref_t session_clist,
                              clist_ref_t clist,
                              idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct session *session;
    struct pd *owner;

    /*
     * FIXME: Need to do a security check here
     */
    if (client == 0 || server == 0) {
        return cap;
    }

    owner = get_pd(_caller, pd);

    session = pd_create_session(owner, (struct thread *)client,
                                (struct thread *)server,
                                (struct clist *)session_clist);
    cap.ref.pd = (session_ref_t)session;
    return cap;
}
#endif

cap_t
iguana_pd_create_clist_impl(L4_ThreadId_t caller, pd_ref_t pd,
                            clist_ref_t clist, idl4_server_environment *env)
{
    cap_t cap = { {0}, 0 };
    struct clist *new_clist;
    struct pd *owner;

    /*
     * FIXME: Need to do a security check here
     */
    owner = get_pd(caller, pd);
    new_clist = server_clist_create(owner);
    cap.ref.clist = (clist_ref_t)new_clist;
    return cap;
}

#if defined(CONFIG_ZONE)
cap_t
iguana_pd_create_zone_impl(L4_ThreadId_t caller, pd_ref_t pd,
                           virtpool_ref_t pool, clist_ref_t clist,
                           idl4_server_environment *env)
{
    cap_t cap = { {0}, 0 };
    struct pd *owner;
    struct zone *zone;

    /*
     * FIXME: Need to do a security check here
     */
    owner = get_pd(caller, pd);
    zone = pd_create_zone(owner, (struct virtpool *)pool);
    cap.ref.zone = (zone_ref_t)zone;
    return cap;
}
#endif

void
iguana_pd_set_callback_impl(CORBA_Object _caller, const pd_ref_t pd,
                            const memsection_ref_t callback_buffer,
                            idl4_server_environment * _env)
{
    /*
     * Currently PD callback doesn't work, and isn't actually used. so we
     * disable it for now
     */
    struct pd *_pd         = get_pd(_caller, pd);
    struct memsection *_ms = get_memsection(_caller, callback_buffer);

    if (_pd == NULL || (callback_buffer != 0 && _ms == NULL)) {
        return;
    }
    
    pd_setup_callback(_pd, _ms);

    return;
}

uintptr_t
iguana_pd_add_clist_impl(CORBA_Object _caller, const pd_ref_t pd,
                         const clist_ref_t clist,
                         idl4_server_environment * _env)
{
    struct pd *_pd         = get_pd(_caller, pd);

    if (_pd == NULL) {
        return -1;
    }

    return pd_add_clist(_pd, (struct clist *)clist);
}

void
iguana_pd_release_clist_impl(CORBA_Object _caller, const pd_ref_t pd,
                             const clist_ref_t clist, const int slot,
                             idl4_server_environment * _env)
{
    struct pd *_pd         = get_pd(_caller, pd);

    if (_pd == NULL) {
        return;
    }

    pd_release_clist(_pd, (struct clist *)clist);
}

void
iguana_pd_delete_impl(CORBA_Object _caller, const pd_ref_t pd,
                      idl4_server_environment * _env)
{
    /* implementation of iguana_pd::delete */
    objref_t caller_pd;
    struct pd *_pd = get_pd(_caller, pd);

    if (_pd == NULL) {
        return;
    }

    caller_pd = iguana_pd_mypd_impl(_caller, _env);

    (void)pd_delete(_pd);
    
    if (caller_pd == pd) {
        idl4_set_no_response(_env);
    }
    return;
}

#if defined(CONFIG_EAS)
cap_t
iguana_eas_create_thread_impl(CORBA_Object _caller, eas_ref_t eas,
                              L4_ThreadId_t *pager, L4_ThreadId_t *scheduler,
                              uintptr_t utcb, clist_ref_t clist,
                              L4_ThreadId_t *l4_id,
                              L4_ThreadId_t *l4_handle,
                              idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct thread *thread;

    struct thread *caller_thrd;
    L4_ThreadId_t pager_tid;
    L4_ThreadId_t sched_tid;

    /*
     * For pager and scheduler, if the argument is L4_myself, then change it to
     * the caller's real tid.
     */
    caller_thrd = thread_lookup(_caller);
    pager_tid.raw = pager->raw == L4_myselfconst.raw ? caller_thrd->id.raw : pager->raw;
    sched_tid.raw = scheduler->raw == L4_myselfconst.raw ? caller_thrd->id.raw : scheduler->raw;

    thread = eas_create_thread((void *)eas, pager_tid, sched_tid, utcb);

    if (thread) {
        if (l4_id != NULL)
            *l4_id = thread->id;
        if (l4_handle != NULL)
            *l4_handle = thread->handle;
        cap.ref.thread = (thread_ref_t)thread;
    } else {
        if (l4_id != NULL)
            *l4_id = L4_nilthread;
        if (l4_handle != NULL)
            *l4_handle = L4_nilthread;
    }
    return cap;
}

void
iguana_eas_delete_impl(CORBA_Object _caller, const eas_ref_t eas,
                       idl4_server_environment * _env)
{
    eas_delete((void *)eas);
}

int
iguana_eas_map_impl(CORBA_Object _caller, eas_ref_t eas, L4_Fpage_t *src_fpage,
                    uintptr_t dst_addr, uintptr_t attributes,
                    idl4_server_environment * _env)
{
    return eas_map((void *)eas, *src_fpage, dst_addr, attributes);
}

void
iguana_eas_unmap_impl(CORBA_Object _caller, eas_ref_t eas,
                      L4_Fpage_t *dst_fpage, idl4_server_environment * _env)
{
    eas_unmap((void *)eas, *dst_fpage);
}

#if defined(ARM_SHARED_DOMAINS)
int
iguana_eas_share_domain_impl(CORBA_Object _caller, eas_ref_t eas,
                    idl4_server_environment * _env)
{
    return eas_share_domain((void *)eas);
}

void
iguana_eas_unshare_domain_impl(CORBA_Object _caller, eas_ref_t eas,
                    idl4_server_environment * _env)
{
    eas_unshare_domain((void *)eas);
}
#endif

#if defined(ARM_PID_RELOC)
uintptr_t
iguana_eas_modify_impl(CORBA_Object _caller, eas_ref_t eas, int pid,
                    idl4_server_environment * _env)
{
    return eas_modify((void *)eas, pid);
}
#endif

#endif
/*
 * Interface iguana_thread
 */
thread_ref_t
iguana_thread_id_impl(CORBA_Object _caller, L4_ThreadId_t *thread,
                      idl4_server_environment * _env)
{
    thread_ref_t ret_thread = 0;
    if (thread->raw == L4_myselfconst.raw)
        ret_thread = (thread_ref_t)thread_lookup(_caller);
    else
        ret_thread = (thread_ref_t)thread_lookup(*thread);
    return ret_thread;
}

L4_ThreadId_t
iguana_thread_l4id_impl(CORBA_Object _caller, const thread_ref_t thread,
                        idl4_server_environment * _env)
{
    struct thread *the_thread = (struct thread *)thread;

    /*
     * FIXME: Check permission on thread
     */

    if (thread == 0) {
        return L4_nilthread;
    }

    return the_thread->id;
}

void
iguana_thread_start_impl(CORBA_Object _caller, const thread_ref_t thread,
                         const uintptr_t ip, const uintptr_t sp,
                         idl4_server_environment * _env)
{
    /* implementation of iguana_thread::start */
    (void)thread_start((struct thread *)thread, ip, sp);
    return;
}

void
iguana_thread_delete_impl(CORBA_Object _caller, const thread_ref_t thread,
                          idl4_server_environment * _env)
{
    struct thread *thread_p;
    struct thread *victim;


    thread_p = thread_lookup(_caller);

    victim = (struct thread *)thread;


    if (victim == NULL) {
        return;                 /* no victim then we fail */
    }

#if defined(CONFIG_EAS)
    if (victim->eas) {
        eas_delete_thread(victim->eas, victim);
    } else
#endif
    {
        pd_delete_thread(victim->owner, victim);
    }
    if (thread_p == victim) {
        idl4_set_no_response(_env);
    }
    return;
}

void
iguana_thread_set_exception_impl(CORBA_Object _caller, thread_ref_t thread,
                                 uintptr_t exc, uintptr_t sp, uintptr_t ip,
                                 idl4_server_environment * _env)
{
    struct thread *thread_p = (struct thread *)thread;

    /*
     * XXX: check exc is a valid exception
     */
    DEBUG_PRINT("set(%lx, %lx)\n", (long)ip, (long)sp);
    thread_p->exception[exc].sp = sp;
    thread_p->exception[exc].ip = ip;
}

/*
 * Interface iguana_hardware
 */

CORBA_long
iguana_hardware_register_interrupt_impl(CORBA_Object _caller, objref_t hardware,
                                        L4_ThreadId_t *handler,
                                        CORBA_long interrupt,
                                        idl4_server_environment * _env)
{
    CORBA_long __retval = 0;
    L4_SpaceId_t space;

    struct thread *thread;

    if (handler->raw == L4_myselfconst.raw)
    {
        thread = thread_lookup(_caller);
    }
    else
    {
        thread = thread_lookup(*handler);
    }

    if (!thread) {
        return -1;
    }
    space = pd_l4_space(thread->owner);

    L4_LoadMR(0, interrupt);
    __retval = L4_SecurityControl(space,
            L4_SecurityCtrl_domain(L4_SecurityControl_IrqAccessDomain) |
            L4_SecurityCtrl_op(L4_SecurityControl_OpGrant));
    if (!__retval) {
        return -1;
    }
    L4_LoadMR(0, interrupt);
    __retval = L4_RegisterInterrupt(*handler, 31, 1, 0);
    if (!__retval) {
        return -1;
    }

    return 0;
}

CORBA_long
iguana_hardware_back_memsection_impl(CORBA_Object _caller,
                                     const objref_t hardware,
                                     const memsection_ref_t memsection,
                                     const uintptr_t paddr,
                                     const uintptr_t attributes,
                                     idl4_server_environment * _env)
{
    CORBA_long __retval = 0;
    struct memsection *ms = (void *)memsection;
    uintptr_t phys = paddr, virt;
    struct pd_entry *ent;
    int r;

    if (!(ms->flags & MEM_USER))
        return -1;
    ms->attributes = attributes;
    for (virt = ms->base; virt <= ms->end; virt += BASE_PAGESIZE) {
        (void)pt_insert(virt, BASE_LOG2_PAGESIZE, phys);
        phys += BASE_PAGESIZE;
    }
    TAILQ_FOREACH(ent, &ms->pd_list, pd_list) {
        r = pd_sync_range(ent->pd, ms->base, ms->end, NULL, 0);
        if (r != 0) {
            return -1;
        }
    }
    return __retval;
}

/*
 * Interface iguana_memsection
 */

CORBA_long
iguana_memsection_register_server_impl(CORBA_Object _caller,
                                       const memsection_ref_t memsection,
                                       const thread_ref_t thread,
                                       idl4_server_environment * _env)
{
    CORBA_long __retval = 0;
    struct memsection *_ms;
    struct thread *_thread;

    if (memsection == 0) {
        return 0;
    }

    _thread = get_thread(_caller, thread);
    _ms = get_memsection(_caller, memsection);

    if (_thread == NULL || _ms == NULL) {
        return -1;
    }

    memsection_register_server(_ms, _thread);

    return __retval;
}

uintptr_t 
iguana_memsection_virt_to_phys_impl(L4_ThreadId_t _caller, uintptr_t addr,  size_t  *size, idl4_server_environment *env)
{
    struct memsection *ms;
    struct pd *pd;
    uintptr_t perm = L4_NoAccess;
    uintptr_t paddr;
    uintptr_t interfaces;

    ms = objtable_lookup((void *)addr);
    pd = thread_lookup(_caller)->owner;
    /* Find the memory section it belongs too */
    if (ms == NULL) {
        return  -1;
    }

    interfaces = security_check(thread_lookup(_caller)->owner, (uintptr_t)ms);
    if (interfaces & (1 << READ_IID))
        perm |= L4_Readable;
    if (interfaces & (1 << WRITE_IID))
        perm |= L4_Writable;
    if (interfaces & (1 << EXECUTE_IID))
        perm |= L4_eXecutable;

    if (perm == L4_NoAccess){
        return -2;
    }

    if (!memsection_lookup_phys(ms, addr, &paddr, size, &iguana_pd)) {
        return -3;
    }

    return paddr;
}

memsection_ref_t
iguana_memsection_lookup_impl(CORBA_Object _caller, const uintptr_t address,
                              thread_ref_t *server,
                              idl4_server_environment * _env)
{
    struct memsection *memsection = objtable_lookup((void *)address);

    if (memsection == NULL) {
        *server = 0;
        return 0;
    }

    *server = (thread_ref_t)memsection->server;
    return (memsection_ref_t)memsection;
}

void
iguana_memsection_delete_impl(CORBA_Object _caller,
                              const memsection_ref_t memsection,
                              idl4_server_environment * _env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return;
    }
    
    memsection_delete(_ms);
    return;
}

uintptr_t
iguana_memsection_info_impl(CORBA_Object _caller, memsection_ref_t memsection,
                            uintptr_t *size, idl4_server_environment * _env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return 0;
    }
    
    *size = _ms->end - _ms->base + 1;
    return _ms->base;
}

void
iguana_memsection_set_attributes_impl(CORBA_Object _caller,
                                      const memsection_ref_t memsection,
                                      uintptr_t attributes,
                                      idl4_server_environment * _env)
{
    int ret;
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return;
    }

    ret =
        memsection_set_attributes(_ms, attributes);
    assert(ret == 0);
    return;
}

int
iguana_memsection_page_map_impl(CORBA_Object _caller,
                                const memsection_ref_t memsection,
                                L4_Fpage_t *from_page, L4_Fpage_t *to_page,
                                idl4_server_environment * _env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return -1;
    }
    
    return memsection_page_map(_ms, *from_page, *to_page);
}

int
iguana_memsection_page_unmap_impl(CORBA_Object _caller,
                                  const memsection_ref_t memsection,
                                  L4_Fpage_t *to_page,
                                  idl4_server_environment * _env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return -1;
    }
    
    return memsection_unmap(_ms, L4_Address(*to_page), L4_Size(*to_page));
}

int
iguana_memsection_map_impl(L4_ThreadId_t _caller, memsection_ref_t memsection,
                           uintptr_t virt, physmem_ref_t phys,
                           idl4_server_environment *env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);
    struct physmem *_pm    = get_physmem(_caller, phys);

    if (_ms == NULL || _pm == NULL) {
        return -1;
    }
    
    return memsection_map(_ms, virt, _pm);
}

int
iguana_memsection_unmap_impl(L4_ThreadId_t _caller, memsection_ref_t memsection,
                             uintptr_t offset, uintptr_t size,
                             idl4_server_environment *env)
{
    struct memsection *_ms = get_memsection(_caller, memsection);

    if (_ms == NULL) {
        return -1;
    }
    
    return memsection_unmap(_ms, offset, size);
}

#if defined(CONFIG_SESSION)
/*
 * Interface iguana_session
 */

void
iguana_session_delete_impl(CORBA_Object _caller, const session_ref_t session,
                           idl4_server_environment * _env)
{
    session_delete((struct session *)session);
    return;
}

int
iguana_session_revoke_impl(CORBA_Object _caller, session_ref_t session,
                           cap_t *cap, idl4_server_environment * _env)
{
    struct memsection *memsection = (void *)cap->ref.memsection;

    memsection_flush(memsection);
    return 0;
}

void
iguana_session_add_buffer_impl(CORBA_Object _caller,
                               const session_ref_t session,
                               const objref_t call_buf,
                               const objref_t return_buf,
                               idl4_server_environment * _env)
{
    session_add_buffer((struct session *)session,
                       (void *)call_buf, (void *)return_buf);
    return;
}
#endif

#if !defined(NDEBUG)
void
iguana_debug_dump_info_impl(CORBA_Object _caller,
                            idl4_server_environment * _env)
{
    debug_dump_info();
}

void
workloop_work(void)
{
#ifdef CONFIG_TEST_IGUANA
    iguana_check_state();
#endif
}
#endif

cap_t
iguana_physpool_alloc_impl(L4_ThreadId_t caller, physpool_ref_t physpool,
                           uintptr_t size, pd_ref_t owner, clist_ref_t clist,
                           idl4_server_environment *env)
{
    cap_t cap = invalid_cap;
    struct pd *_owner;
    struct physmem *pm;
    struct physpool *_pool;

    _owner = get_pd(caller, owner);
    _pool = (struct physpool *)physpool;

    pm = physpool_alloc(_pool, size);
    if (pm != NULL) {
        pm->pd = _owner;
        TAILQ_INSERT_TAIL(&_owner->pm_list, pm, pm_list);
        cap.ref.physmem = (uintptr_t)pm;
    }
    return cap;
}

cap_t
iguana_physpool_alloc_fixed_impl(L4_ThreadId_t caller, physpool_ref_t physpool,
                             uintptr_t size, uintptr_t base, pd_ref_t owner,
                             clist_ref_t clist,
                             idl4_server_environment *env)
{
    cap_t cap = invalid_cap;
    struct pd *_owner;
    struct physmem *pm;
    struct physpool *_pool;

    _owner = get_pd(caller, owner);
    _pool = (struct physpool *)physpool;

    pm = physpool_alloc_fixed(_pool, size, base);
    if (pm != NULL) {
        pm->pd = _owner;
        TAILQ_INSERT_TAIL(&_owner->pm_list, pm, pm_list);
        cap.ref.physmem = (uintptr_t)pm;
    }
    return cap;
}

void
iguana_physmem_delete_impl(L4_ThreadId_t _caller, physmem_ref_t pm,
                           idl4_server_environment *env)
{
    struct physmem *_pm = get_physmem(_caller, pm);

    if (_pm != NULL && pm != INVALID_ADDR) {
        TAILQ_REMOVE(&_pm->pd->pm_list, _pm, pm_list);
        physmem_delete(_pm);
    }
}

void
iguana_physmem_info_impl(L4_ThreadId_t _caller, physmem_ref_t pm,
                         uintptr_t *paddr, uintptr_t *psize,
                         idl4_server_environment *env)
{
    struct physmem *_pm = get_physmem(_caller, pm);

    if (_pm != NULL && pm != INVALID_ADDR) {
        *paddr = _pm->mem->addr;
        *psize = _pm->mem->size;
    }
}

#if defined(CONFIG_STATS)
cap_t
iguana_statistics_get_stats_impl(CORBA_Object _caller, objref_t pd,
                                 clist_ref_t clist, uintptr_t *out_base,
                                 idl4_server_environment * _env)
{
    cap_t cap = { {0}, 0 };
    struct memsection *memsection;
    struct pd *owner;
    int r;

    owner = get_pd(_caller, pd);

    if (owner == NULL) {
        *out_base = 0;
        return cap;
    }

    memsection = pd_create_memsection(owner, sizeof(iguana_stats_t), 0,
                                      default_physpool,
                                      default_virtpool,
                                      NULL,
                                      MEM_NORMAL);
    if (memsection != NULL) {
        stats_fill((iguana_stats_t*) memsection->base);
        *out_base = memsection->base;
    } else {
        *out_base = 0;
    }
    cap.ref.memsection = (memsection_ref_t)memsection;
    if (IS_VALID_CAP(cap)) {
        r = server_clist_insert((struct clist *)clist, cap);
        if (r != 0) {
            assert(!"BAD");
        }
    }

    return cap;
}
#endif

void
iguana_clist_delete_impl(L4_ThreadId_t caller, clist_ref_t clist,
                         idl4_server_environment *env)
{
    server_clist_delete((struct clist *)clist);
}

int
iguana_clist_insert_impl(L4_ThreadId_t caller, clist_ref_t clist, cap_t *cap,
                         idl4_server_environment *env)
{
    return server_clist_insert((struct clist *)clist, *cap);
}

int
iguana_clist_lookup_impl(L4_ThreadId_t caller, clist_ref_t clist,
                         uintptr_t obj, unsigned interface, cap_t *cap,
                         idl4_server_environment *env)
{
    *cap = server_clist_lookup((struct clist *)clist, obj, interface);
    return IS_VALID_CAP(*cap) == 0;
}

int
iguana_clist_remove_impl(L4_ThreadId_t caller, clist_ref_t clist, cap_t *cap, 
                         idl4_server_environment *env)
{
    return server_clist_remove((struct clist *)clist, *cap);
}

int
iguana_mutex_allocate_impl(L4_ThreadId_t caller, L4_MutexId_t * id,
        idl4_server_environment *env)
{
    return mutex_allocate(id);
}

int iguana_mutex_free_impl(L4_ThreadId_t caller, L4_MutexId_t * id,
        idl4_server_environment *env)
{
    return mutex_free(*id);
}

#if defined(CONFIG_ZONE)
void
iguana_zone_delete_impl(L4_ThreadId_t caller, zone_ref_t zone,
                        idl4_server_environment *env)
{
    zone_delete((struct zone *)zone);
}
#endif

#if defined(CONFIG_TRACEBUFFER)
int iguana_trace_tracebuffer_phys_impl(L4_ThreadId_t caller,
        uintptr_t * phys, uintptr_t * size, idl4_server_environment *env)
{
    return tracebuffer_phys(phys, size);
}
#endif

void
iguana_client_exception(L4_ThreadId_t *caller, L4_Word_t *mr)
{
    L4_MsgTag_t t;
    int i;

    i = arch_handle_exception(caller, mr);

    if (i == 0) {
        return;
    }

    t.raw = mr[0];
    if (i == -1)
        DEBUG_PRINT("<iguana_server> Unknown exception from thread: %lx\n", caller->raw);
    DEBUG_PRINT("  --  TAG: %p\n", (void*)t.raw);
    for (i = 0; i < t.X.u; i++)
    {
        DEBUG_PRINT("     MR%02d: %p\n", i+1, (void*)mr[i+1]);
    }
    /* Clear partner so we don't reply */
    *caller = L4_nilthread;
}

void
iguana_client_unknownipc(L4_ThreadId_t *caller, L4_Word_t *mr)
{
    DEBUG_PRINT("<iguana_server> Unknown IPC from thread %lx, tag = %lx\n", caller->raw, mr[0]);
    /* Clear partner so we don't reply */
    *caller = L4_nilthread;
}

