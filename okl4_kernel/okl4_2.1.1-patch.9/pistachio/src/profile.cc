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
 * Description: Profiling
 */

#ifdef CONFIG_L4_PROFILING

#include <l4.h>
#include <debug.h>

#include <tcb.h>
#include <space.h>
#include <arch/space.h>
#include <arch/tcb.h>
#include <l4/arch/vregs.h>
#include <kernel/arch/profile.h>
#include <kernel/profile.h>

u64_t __profile_total_time;

profile_kernel_time_t profile_kernel_time;

word_t profile_fake_stack[30];

#define MAX_EVENT_NAME_LENGTH   24
typedef char    event_name[MAX_EVENT_NAME_LENGTH];

static event_name event_string[MAX_EVENT_TYPE] =
{
    "L4_CacheControl",
    "L4_MapControl",
    "L4_SecurityControl",
    "L4_ThreadSwitch",
    "L4_SpaceControl",
    "L4_ThreadControl",
    "L4_PlatformControl",
    "L4_SpaceSwitch",
    "L4_Schedule",
    "L4_ExchangeRegisters",
    "L4_Ipc",
    "L4_ProcessorControl"
#if defined(ARCH_EVENT_STR) && defined(ARCH_NUM_EVENTS) && \
    defined(ARCH_EVENT_TYPE)
    ,
    ARCH_EVENT_STR
#endif
};

#if defined(CONFIG_MUNITS)
static profile_stats_t profile_stats[CONFIG_NUM_UNITS][MAX_EVENT_TYPE];
static profile_stats_t profile_stats_all[MAX_EVENT_TYPE];
#define read_stats(e)   profile_stats[get_current_context().unit][e]
#else
static profile_stats_t profile_stats[MAX_EVENT_TYPE];
#define read_stats(e)   profile_stats[e]
#endif

static int profile_enabled = 0;

/*
 * set/unset functions
 */

inline void
profile_event_set_running(profile_thread_data_t *ptd, int e)
{
    ptd->running |= (1 << e);
}

inline void
profile_event_unset_running(profile_thread_data_t *ptd, int e)
{
    ptd->running &= ~(1 << e);
}

inline void
profile_event_set_paused(profile_thread_data_t *ptd, int e)
{
    ptd->paused |= (1 << e);
}

inline void
profile_event_unset_paused(profile_thread_data_t *ptd, int e)
{
    ptd->paused &= ~(1 << e);
}

/*
 * Profile Start
 */

void
profile_start(int e)
{
    profile_thread_data_t *ptd;

    if (profile_enabled == 0) {
        return;
    }

    ptd = &(get_current_tcb()->profile_data);

    profile_event_set_running(ptd, e);
    read_stats(e).errors++;
    read_stats(e).start = profile_arch_read_timer();
}

/*
 * Profile Stop
 */

static inline void
__profile_stop(tcb_t *tcb, u64_t timer, int e)
{
    if (read_stats(e).start == 0) {
        return;
    }
    read_stats(e).errors--;
    read_stats(e).counter++;
    profile_event_unset_running(&tcb->profile_data, e);
    if (tcb->profile_data.paused & (1 << e)) {
        profile_event_unset_paused(&tcb->profile_data, e);
        read_stats(e).errors--;
    } else {
        read_stats(e).cycles += timer - read_stats(e).start;
    }
}

void
profile_stop_if_running(tcb_t *tcb, int e)
{
    u64_t timer;

    timer = profile_arch_read_timer();

    if (tcb->profile_data.running & (1 << e)) {
        __profile_stop(tcb, timer, e);
    }
}

void
profile_stop_all(tcb_t *tcb)
{
    register int i;
    u64_t timer;

    timer = profile_arch_read_timer();

    for (i = 1; i < MAX_EVENT_TYPE; i++) {
        if (tcb->profile_data.running & (1 << i)) {
            __profile_stop(tcb, timer, i);
        }
    }
}

void
profile_stop(int e)
{
    u64_t timer;
    tcb_t *tcb;

    timer = profile_arch_read_timer();
    tcb = get_current_tcb();
    if (tcb) {
        __profile_stop(tcb, timer, e);
    } else {
        printf("%p", tcb);
    }
}

/*
 * Profile Pause
 */

inline void
profile_pause(int e, u64_t timer)
{
    read_stats(e).cycles += timer - read_stats(e).start;
}

inline void
profile_pause_event(profile_thread_data_t *ptd, int e, u64_t timer)
{
    read_stats(e).switches++;
    read_stats(e).errors++;
    profile_pause(e, timer);
    profile_event_set_paused(ptd, e);
}

void
profile_pause_all(tcb_t *tcb, u64_t timer)
{
    register int i;
    profile_thread_data_t *ptd = &tcb->profile_data;

    for (i = 1; i < MAX_EVENT_TYPE; i++) {
        if (ptd->running & (1 << i) && !(ptd->paused & (1 << i))) {
            profile_pause_event(ptd, i, timer);
        }
    }
}

/*
 * Profile Resume
 */

inline void
profile_resume(int e, u64_t timer)
{
    read_stats(e).start = timer;
}

void
profile_resume_all(tcb_t *tcb)
{
    u64_t timer;
    register int i;
    profile_thread_data_t *ptd = &tcb->profile_data;

    timer = profile_arch_read_timer();
    for (i = 1; i < MAX_EVENT_TYPE; i++) {
        if (ptd->running & (1 << i) && ptd->paused & (1 << i)) {
            read_stats(i).errors--;
            profile_resume(i, timer);
            profile_event_unset_paused(ptd, i);
        }
    }
}

/*
 * switch_to handlers
 */

void
profile_switch_from(tcb_t *from)
{
    u64_t timer;

    if (profile_enabled == 0) {
        return;
    }

    timer = profile_arch_read_timer();

    profile_pause_all(from, timer);

    if (from->profile_data.start != 0) {
        from->profile_data.time += timer - from->profile_data.start;
    }
    from->profile_data.switches++;
}

void
profile_switch_to(tcb_t *to)
{
    if (profile_enabled == 0) {
        return;
    }

    to->profile_data.start = profile_arch_read_timer();
    profile_resume_all(to);
}

/*
 * Profile Init
 */

static void
profile_init(void)
{
    int i;

    profile_kernel_time.start = 0xffffffffffffffffULL;
    profile_kernel_time.time = 0;
    profile_kernel_time.entry_counter = 0;

#if defined(CONFIG_MUNITS)
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        register int j;
        for (j = 0; j < CONFIG_NUM_UNITS; j++) {
            profile_stats[j][i].counter = 0;
            profile_stats[j][i].cycles = 0;
            profile_stats[j][i].errors = 0;
            profile_stats[j][i].switches = 0;
        }
    }
#else
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        read_stats(i).counter = 0;
        read_stats(i).cycles = 0;
        read_stats(i).errors = 0;
        read_stats(i).switches = 0;
    }
#endif
}

void
profile_tcb_init(tcb_t *tcb)
{
    tcb->profile_data.running = 0;
    tcb->profile_data.paused = 0;
    tcb->profile_data.start = 0;
    tcb->profile_data.time = 0;
    tcb->profile_data.switches = 0;
}

static void
profile_t_clear(void)
{
    tcb_t *t, *first;
    profile_thread_data_t *ptd;

    first = t = global_present_list;
    do {
        printf("== thread %p ==\n", t);
        ptd = &(t->profile_data);
        ptd->start = 0;
        ptd->time = 0;
        ptd->switches = 0;
        t = t->present_list.next;
    } while (t != first);
}

void
profile_print_idle_stats(void)
{
    int i;
    extern tcb_t *__idle_tcb[];

    printf("**** Idle time per UNIT ****\n");
    printf(" Unit " " | " "     Idle cycles\n");
    printf("------" "-|-" "---------------------\n");
    for (i = 0; i < CONFIG_NUM_UNITS; i++) {
        printf(" %-2u   " " | " "%20llu\n", i, __idle_tcb[i]->profile_data.time);
    }
}

void
profile_print_threads(void)
{
    tcb_t *t, *first;
    profile_thread_data_t *ptd;

    printf(" Thread Id  " " | " "              Cycles" " "
           "      Switches" " " " Cycles/Switch\n");
    printf("------------" "-|-" "--------------------" "-"
           "--------------" "-" "--------------\n");

    first = t = global_present_list;
    do {
        if (t) {
            ptd = &(t->profile_data);
            printf(" 0x%-9x "
            "| %20llu %14u %14u\n", (word_t)/*t*/t->get_global_id().get_raw(),
            ptd->time,
                                                    ptd->switches,
                     ptd->switches ? (u32_t)(ptd->time / (u64_t)ptd->switches) : 0);
        }
        t = t->present_list.next;
    } while (t != first);
}

void
profile_print_l4_time(void)
{
    printf("Total cycles inside OKL4 = %llu, %u, %u\n", profile_kernel_time.time,
                            profile_kernel_time.entry_counter,
                            profile_kernel_time.entry_counter
                            ? profile_kernel_time.time / profile_kernel_time.entry_counter
                            : 0);
}

static void
profile_print_profiling_time(void)
{
    if (profile_enabled == 0 && __profile_total_time > 0) {
        /*
        printf("Profiling total running time (so far): %llu\n",
                        profile_arch_read_timer() - __profile_total_time);
    } else {
        */
        printf("Profiling total running time: %llu\n", __profile_total_time);
    }
}

void
profile_print(void)
{
    int i;

    printf("==== Profile data dump ===\n");

    profile_print_profiling_time();

    profile_print_l4_time();

    profile_print_threads();

    profile_print_idle_stats();

    printf
    (
        "%24s \t %10.10s \t %10.10s \t %10.10s \t %10.10s \t %10.10s\n",
        "Event", "#", "Total", "Avg", "Err", "Switches"
    );

#if defined(CONFIG_MUNITS)
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        profile_stats_all[i].counter = 0;
        profile_stats_all[i].cycles = 0;
        profile_stats_all[i].errors = 0;
        profile_stats_all[i].switches = 0;
        register int j;
        for (j = 0; j < CONFIG_NUM_UNITS; j++)
        {
            profile_stats_all[i].counter += profile_stats[j][i].counter;
            profile_stats_all[i].cycles += profile_stats[j][i].cycles;
            profile_stats_all[i].errors += profile_stats[j][i].errors;
            profile_stats_all[i].switches += profile_stats[j][i].switches;
        }
        printf
        (
            "%24s \t %10.10llu \t %10.10llu \t %10.10llu \t %d \t %10.10llu\n",
            event_string[i],
            profile_stats_all[i].counter,
            profile_stats_all[i].cycles, // / profile_stats_all[i].counter,
            profile_stats_all[i].counter > 0 ?
                profile_stats_all[i].cycles / profile_stats_all[i].counter : 0,
            profile_stats_all[i].errors,
            profile_stats_all[i].switches
        );
    }
#else
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        printf
        (
            "%24s \t %10.10llu \t %10.10llu \t %10.10llu \t %d \t %10.10llu\n",
            event_string[i],
            read_stats(i).counter,
            read_stats(i).cycles, // / read_stats(i).counter,
            read_stats(i).counter > 0 ?
                read_stats(i).cycles / read_stats(i).counter : 0,
            read_stats(i).errors,
            read_stats(i).switches
        );
    }
#endif
    printf("\n");
}

void
profile_enable(void)
{
    __profile_total_time = profile_arch_read_timer();
    profile_kernel_time.start = 0;
    profile_enabled = 1;
}

void
profile_disable(void)
{
    register int i;

    profile_enabled = 0;
    profile_kernel_time.start = 0xffffffffffffffffULL;
    __profile_total_time = profile_arch_read_timer() - __profile_total_time;
#if defined(CONFIG_MUNITS)
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        register int j;
        for (j = 0; j < CONFIG_NUM_UNITS; j++) {
            profile_stats[j][i].start = 0;
        }
    }
#else
    for (i = 0; i < MAX_EVENT_TYPE; i++)
    {
        read_stats(i).start = 0;
    }
#endif
}

void
profile_reset(void)
{
    printf("\n=== Profile data reset!! ===\n\n");
    profile_init();
    profile_t_clear();
}

void
profile_handler(word_t op)
{
    switch (op)
    {
    case 0x0: /* enable */
        profile_enable();
        break;

    case 0x1: /* disable */
        profile_disable();
        break;

    case 0x2: /* reset */
        profile_reset();
        break;

    /* XXX: Should we print something if not from KDB? */
    case 0x3: /* print */
        profile_print();
        break;

    default:
        break;
    }
}

#endif /* CONFIG_L4_PROFILING */
