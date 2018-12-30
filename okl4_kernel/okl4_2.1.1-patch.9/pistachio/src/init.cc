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
 * Description: Generic L4 init code
 */
#include <l4.h>
#include <debug.h>

/**
 * Entry point from ASM into C kernel
 * Precondition: MMU and page tables are disabled
 * Warning: Do not use local variables in startup_system()
 */

#include <tcb.h>
#include <smp.h>
#include <schedule.h>
#include <space.h>
#include <arch/memory.h>
#include <config.h>
#include <arch/hwspace.h>
#include <generic/lib.h>
#include <arch/platform.h>
#include <mutex.h>
#include <clist.h>

#include <mp.h>
mp_t mp;

void idle_thread();
void init_all_threads();
#if defined(ARCH_MIPS64)
extern void SECTION (SEC_INIT) finalize_cpu_init (word_t cpu_id);
#endif

extern void init_stacks(void);


void __panic(void)
{
#ifndef CONFIG_DEBUG
    /* Reboot if no debugger is present. */
    Platform::reboot();
#else
    /* Enter KDB */
    enter_kdebug("panic");
#endif

    /* Loop just in case the reboot returns. */
    for (;;);
}

void SECTION(SEC_INIT) create_idle_thread()
{
    bool r;
    r = get_idle_tcb()->create_kernel_thread(NILTHREAD, get_idle_utcb());
    ASSERT(DEBUG, r);

    /* set idle-magic */
    get_idle_tcb()->myself_global.set_raw((word_t)0x1d1e1d1e1d1e1d1eULL);

    r = get_idle_tcb()->grab();
    ASSERT(DEBUG, r);
    get_idle_tcb()->initialise_state(thread_state_t::running);
    get_idle_tcb()->effective_prio = -1;
    get_idle_tcb()->base_prio = -1;
    /* set idle thread timeslice to infinite */
    (void)get_current_scheduler()->do_schedule(get_idle_tcb(), 0, ~0UL, 0);
    get_idle_tcb()->release();

#if defined(CONFIG_THREAD_NAMES)
    strcpy(get_idle_tcb()->debug_name, "idle");
#if defined(CONFIG_MUNITS)
    {
        int unit = get_current_context().unit;
        int chars = unit / 10;
        int i;
        for (i = chars; i >= 0; i --) {
            get_idle_tcb()->debug_name[4 + 1] = '0' + unit % 10;
            unit /= 10;
        }
        get_idle_tcb()->debug_name[5 + chars] = 0;
    }
#endif
#endif
}


#if defined(CONFIG_MUNITS)
/* We set this flag here, since now we are running on the idle thread's (unique) stack
 * and this avoids a race where the next context is brought up and starts using the boot
 * stack before we are finished with it.
 */
void SECTION(SEC_INIT) context_stub(void)
{
    get_mp()->context_initialized = true;
    ACTIVATE_CONTINUATION(idle_thread);
}

/* This is the C entry-point after arch specific
 * assembler routines bootstrap a context  */
extern "C" void SECTION (SEC_INIT) startup_context (void)
{
    cpu_context_t context = get_current_context();

    create_idle_thread();
    get_idle_tcb()->notify(context_stub);

    init_xcpu_handling (context);

    //get_current_scheduler()->init (false);
    get_current_scheduler()->start (context);

    /* make sure we don't fall off the edge */
    spin_forever(1);
}
#endif

/**
 * Setup system-wide scheduling.
 */
static NORETURN void SECTION(SEC_INIT)
startup_scheduler()
{
#if defined(CONFIG_SPACE_NAMES)
    strcpy(get_kernel_space()->debug_name, "kernel");
#endif

#if defined (CONFIG_MDOMAINS) || defined (CONFIG_MUNITS)
    TRACE_INIT("Initialising multiprocessing support...\n");
#endif

    /* initialise the mp class */
    get_mp()->probe();
    get_mp()->print_info();

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
    init_xcpu_handling (get_current_context());
#endif

    TRACE_INIT("Initialising scheduler...\n");
    get_current_scheduler()->init(false);

    create_idle_thread();
    get_idle_tcb()->notify (init_all_threads);

#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
    /* initialize other processors */
    TRACE_INIT("Initialising other contexts...\n");
    get_mp()->init_other_contexts();
    TRACE_INIT("Finished initialising other contexts\n");
#endif

#ifdef ARCH_MIPS64
#if defined(CONFIG_MUNITS) || defined(CONFIG_MDOMAINS)
#error fix mips SMP
#endif
    /*get_idle_tcb ()->notify (finalize_cpu_init, 0);*/
#endif

    /* get the thing going - we should never return */
    get_current_scheduler()->start(get_current_context());

    NOTREACHED();
}

extern void init_root_clist(void);
/**
 * Perform architecture independent system-wide initialisation.
 *
 * At this point, all per-architecture initialisation has been performed.
 *
 * This function does not return.
 */
NORETURN void SECTION(SEC_INIT)
generic_init(void)
{
    /* Setup mutex support. */
    init_mutex();
    /* Setup clistids */
    init_clistids();
    /* Initialise root cap list */
    init_root_clist();

    /* Startup the scheduler, and begin to schedule threads. Does not
     * return. */
    startup_scheduler();

    NOTREACHED();
}


