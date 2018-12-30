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
 * Description:   implementation of apic timer
 */

/**
 *  FIXME: Mothra Issue #2038 - This file needs to be updated to for
 *  scheduler interface.
 */
#error "File is out of date. Check Mothra Issue #2038."

#include <l4.h>

/*#define DEBUG_TIMER*/

#include <arch/trapgate.h>

#include <arch/idt.h>
#include <arch/intctrl.h>
#include <arch/timer.h>

#include <plat/rtc.h>

#include <schedule.h>
#include <ipc.h>

#include <kdb/tracepoints.h>

// timer is bus_cycles / (TIMER_NSECS) = 
// bus_cycles / ( 1000000 / 1000 ) = (bus_cycles * 1000) / 10000000
#define TIMER_NSECS (1000000 / TIMER_TICK_LENGTH)
#define TIMER_ABSOLUTE (((u64_t)bus_cycles * TIMER_TICK_LENGTH) / 1000000)
#define ADJUST(x) (((u32_t)(x)) < (bus_cycles/1000000) ? (bus_cycles/1000000) : (x))

// Print a 64bit int, because printf sure can't
static
void print_num_actual(u64_t num)
{
    if(num) {
        print_num_actual(num/10);
        printf("%c", num % 10 + '0');
    }
}
#if 0
static void print_num(u64_t num)
{
    if(num)
        print_num_actual(num);
    else
        printf("0");
    printf("\n");
}
#define dprintf printf
#else
#define dprintf(x...)
#define print_num(x)
#endif

//#define DECLARE_TRACEPOINT(APIC_TIMER_FIRE);

void send_async(tcb_t *receiver, word_t notify_bits,
                continuation_t continuation);
bool notify_tcb(tcb_t *receiver, word_t notify_bits, bool enqueue);


EXTERN_KMEM_GROUP(kmem_misc);

/*
 * IA-32 kernel timer interface
 *
 * This is an implementation of the kernel timer interface.  As of now
 * it is mostly generic (except for the bits which talk to the actual 
 * hardware).
 *
 * The timer is implemented as follows.  An arch_timer struct is 
 * defined in the architecture context.  It contains the absolute timeout
 * in ticks, a previous pointer and a next pointer and the notify bits
 * for the wakeup.
 *
 * The list chained with the previous pointer and the next pointer and is
 * sorted by timeout.  The idle tcb is also chained on there, in the case
 * of the idle tcb, it is a special case with does periodic timer ticks.
 * In case a thread tries to register a timer multiple times, the latest
 * one takes effect and the old one is discarded.
 *
 * SECURITY CONSIDERATIONS
 *
 * The timer like an interrupt, but as the timer may be freely programmable
 * there might be issues with this as this could be used to circumvent 
 * fair scheduling.
 * The rounding up of the minimum sleep time might also delay threads for
 * an amount of time as much as the minimum time to sleep.
 *
 * TODO
 *
 * Look at security issue mentioned above
 * Most of this code can and should be generic (with exception of actually
 * taking to the h/w)
 * currently there is no way to cancel a pending timer.  the only way to
 * do this is to ignore it (by not listening for async).  since we are using
 * absolute timeout we can abuse this and use zero to unset the timer?
 * Currently we store the time in APIC ticks, we should just do one
 * conversion though (in cpu ticks) then down-convert to APIC timer.
 * On tcb deletion a hook should be called so that if there are outstanding
 * timers for a thread it may be unchained.
 * Have to think about how to do absolute / relative timeouts: not every 
 * architecture allows reading of the timer, maybe we can just put current
 * time in the KIP?  But it means it won't be updated until next timer tick!!
 * At the same time we want an interface that OS designers cannot abuse 
 * to set absolute time using get / set method (2 syscall)
 *
 * -gl.
 */

/*
 * Remove tcb from timeout queue
 */
tcb_t *
timer_t::remove(tcb_t * target)
{
    tcb_t * prev, * next;

    ASSERT(ALWAYS, target);

    prev = target->arch.timer.prev;
    next = target->arch.timer.next;
    if (target == timeout_queue) {
        timeout_queue = next;
    }
    if (prev) {
        prev->arch.timer.next = next;
    }
    if (next) {
        next->arch.timer.prev = prev;
    }
    target->arch.timer.prev = target->arch.timer.next = 0;
    target->arch.timer.timeout = 0;

    return (target);
}

/*
 * insert()
 *
 * Inserts the time based on absolute timeout ordering (numerical compare)
 */
void
timer_t::insert(tcb_t * target)
{
    tcb_t * q, * prev;
    u64_t timeout;

    q = prev = 0;
    timeout = target->arch.timer.timeout;

    // Queue is empty - just insert
    // This should not normally happen but the idle tcb could
    // have been dequeued during timer interrupt and we only have
    // the idle tcb queued
    if (!timeout_queue) {
        ASSERT(ALWAYS, target == get_idle_tcb());
        timeout_queue = target;
        target->arch.timer.next = target->arch.timer.prev = 0;
        return;
    }

    // if next pointer is null, then it means q is the last element.
    // this means we need to tack this on as the last element.
    for (q = timeout_queue; q->arch.timer.next; q = q->arch.timer.next) {
        if (q->arch.timer.timeout > timeout) {
             // q timeout > target timeout, insert before q
             
             prev = q->arch.timer.prev;
             if (prev) { // take care of head case
                 prev->arch.timer.next = target;
             }
             else {
                 timeout_queue = target;
             }
             q->arch.timer.prev = target;
             target->arch.timer.prev = prev;
             target->arch.timer.next = q;
             return;
        }
    }

    // if we can get here, q points to the last element in a non-zero list
    // shift prev to be q and q to be zero to insert at last
    if (q->arch.timer.timeout < timeout) {
        // last elem timeo < req timeo, put at end of list
        prev = q;
        q = 0;
    }
    else { // second last
        prev = q->arch.timer.prev;
        // additionally we may need to fix up head as it may be 
        // a single element, in which case the for loop falls down here
        // and second last insertion means "put at front" in this case
        if (q == timeout_queue)
            timeout_queue = target;
    }

    if (prev) {
        prev->arch.timer.next = target;
    }
    if (q) {  // no need to set current pointer if we are inserting at end
        q->arch.timer.prev = target;
    }
    target->arch.timer.prev = prev;
    target->arch.timer.next = q;
}
            
/* global instance of timer object */
timer_t timer UNIT("cpulocal");
static local_apic_t<APIC_MAPPINGS> local_apic;

extern "C" void timer_interrupt(void);
IA32_EXC_NO_ERRORCODE(timer_interrupt, 0)
{
    timer.handle(ASM_CONTINUATION);
}

/*
 * pop()
 *
 * Pops the head of the tcb off.  Special case to take care idle tcb.
 * In this case it is reinserted (idle tcb can never get removed, it is
 * timer interrupt)
 */
tcb_t *
timer_t::pop()
{
    tcb_t * tcb;
    u64_t now = ia32_rdtsc();

    if ((tcb = remove(timeout_queue)) == get_idle_tcb()) {   // refresh timer
        tcb->arch.timer.timeout = TIMER_ABSOLUTE + now;
        insert(tcb);
    }
    return tcb;
}

void
timer_t::handle(continuation_t cont)
{
    tcb_t * first_tcb = NULL, * dest_tcb;
    bool expired;
    bool enqueue = false;

    settime();
    local_apic.EOI();

    //TRACEPOINT(APIC_TIMER_FIRE, printf("APIC timer fired\n"));

    /*
     * Check the head.  If it is the idle tcb, it means we do
     * regular timer interrupt.
     *
     * If we are pushing the head on again and it is the first one
     * then we do not need to reprogram the timer as it is already
     * in periodic mode.
     *
     * There could be multiple timers that have already expired,
     * and in this case we try to wake them up too but we only
     * switch_to() the first one.  If we encounter a timer tick
     * we stop to process the timer tick.  The wakeup flag will
     * indicate whether we need to wake things up or not.  
     */

    dest_tcb = pop();


    // the first_tcb is the one that we want to fire.  This could be timer
    // interrupt, or could be real timeout
    //
    // we want to enqueue tcb for as long as possible until we find the
    // first one that does not need to be
    do {
        expired = !reprogram();
        // send the wakeup if not timer tick
        if (dest_tcb != get_idle_tcb()) {
            dprintf("timer fire for thread %p\n", dest_tcb);
            // notify_tcb won't queue if thread is not waiting, so
            if (notify_tcb(dest_tcb, dest_tcb->arch.timer.notify_bits, 
                enqueue)) {  // switch_to() OK on this thread!
                ASSERT(ALWAYS, !first_tcb);
                first_tcb = dest_tcb;
                enqueue = true;
            }
        }
        else {
            // if first_tcb is set, then we need to enqueue it
            if (first_tcb)
            {
                ASSERT(ALWAYS, first_tcb != get_idle_tcb());
                dprintf("giving prio to timer interrupt\n");
                notify_tcb(first_tcb, first_tcb->arch.timer.notify_bits, true);
            }
            first_tcb = dest_tcb;
             enqueue = true;
        }
        if (expired) {
            dest_tcb = pop();
        }
    } while (expired);

    if (first_tcb == get_idle_tcb()) {
        savetime();
        get_current_scheduler()->handle_timer_interrupt(cont);
    }
    else if (first_tcb) {
        dprintf("sending async\n");
        savetime();
        send_async(first_tcb, first_tcb->arch.timer.notify_bits, cont);
    }
    else {
        if (/*print_nothing*/1) {
            dprintf("nothing to do? %d ms since last fire\n", 
                (unsigned)((thisfire - lastfire) / 2160000));
            tcb_t *q = timeout_queue;
#ifdef DEBUG_TIMER
            dprintf("real ");
            print_num_actual(ia32_rdtsc());
            dprintf(" ");
#endif
            while (q) {
                dprintf("%p ", q);
                q = q->arch.timer.next;
            }
            dprintf("\n");
        }
        savetime();
        //enter_kdebug("misfire?");
    }
    ACTIVATE_CONTINUATION(cont);
}

/*
 * Reprograms what is at the head of the timer.  If idle tcb, then set
 * to periodic.
 */
bool
timer_t::reprogram()
{
    tcb_t * tcb; // tcb to program for
    u64_t now;
    u64_t deadline;

    now = ia32_rdtsc();                 // SMP?
    tcb = timeout_queue;                // head
    deadline = timeout_queue->arch.timer.timeout;

    if (deadline < now)
        return false;  // fire now

    if (tcb == get_idle_tcb())
        local_apic.timer_setup(IDT_LAPIC_TIMER, true);
     else
        local_apic.timer_setup(IDT_LAPIC_TIMER, false);

    local_apic.timer_set(ADJUST((deadline - now) / bus_ratio));
    return true;
}

void
timer_t::init_global()
{
    TRACE_INIT("init_global timer - trap gate %d\n", IDT_LAPIC_TIMER);
    idt.add_int_gate(IDT_LAPIC_TIMER, timer_interrupt);
}

void
timer_t::init_cpu()
{
    // avoid competing for the RTC
    static spinlock_t timer_lock;
    timer_lock.lock();

    TRACE_INIT("calculating processor speed...\n");
    local_apic.timer_set_divisor(1);
    local_apic.timer_setup(IDT_LAPIC_TIMER, false);
    local_apic.timer_set(-1UL);

    /* calculate processor speed */
    wait_for_second_tick();

    cpu_cycles = ia32_rdtsc();
    bus_cycles = local_apic.timer_get();

    wait_for_second_tick();

    cpu_cycles = ia32_rdtsc() - cpu_cycles;
    bus_cycles -= local_apic.timer_get();

    proc_freq = cpu_cycles / 1000;
    bus_freq = bus_cycles / 1000;
    bus_ratio = cpu_cycles/bus_cycles;

    /* is this per proc? */
    timeout_queue = get_idle_tcb();

    TRACE_INIT("CPU speed: %d MHz, bus speed: %d MHz\n",
               (word_t)(cpu_cycles / (1000000)), bus_cycles / (1000000));

    /* now set timer IRQ to periodic timer */
    local_apic.timer_setup(IDT_LAPIC_TIMER, true);
    local_apic.timer_set(ADJUST( bus_cycles / TIMER_NSECS ));
    timer_lock.unlock();
}

/*
 * add_timeout()
 *
 * We consider 3 cases:
 * 
 * If the specified time (in absolute ticks) is less than
 * the current time, then we send the timeout immediately.
 *
 * If the timeout specified is less than the decrementor (meaning that
 * it needs to be woken up before the next tick) then we reprogram the timer.
 *
 * Otherwise, we just tack the timer onto the timer queue.
 */
void
timer_t::add_timeout(tcb_t * dest_tcb, word_t notify_bits, u64_t timeout)
{
    u64_t now;

    now = ia32_rdtsc(); // SMP?
    dprintf(">>> TIMER: now: "); print_num(now);
    dprintf(">>> TIMER: then: "); print_num(timeout);

    // if a timer exists, remove it first
    if (dest_tcb->arch.timer.timeout) {
        dprintf("dest_tcb %p has pending timer\n", dest_tcb);
        remove(dest_tcb);
    }

    if (now > timeout) {
        // case 1: need a wakeup
        dprintf(">>> TIMER: sending notify to %t\n", dest_tcb);
        // XXX this may short-circuit scheduling -gl
        notify_tcb(dest_tcb, notify_bits, false);
        send_async(dest_tcb, notify_bits, 
            get_current_tcb()->arch.syscall_continuation);
        enter_kdebug("return?");
        return;
    }
    else {
        // common (2 + 3): shove into queue
        dprintf(">>> TIMER: queue push\n");
        dest_tcb->arch.timer.timeout = timeout;
        dest_tcb->arch.timer.notify_bits = notify_bits;
        insert(dest_tcb);
    }

    if (ADJUST((timeout-now)/bus_ratio) < local_apic.timer_get()) {
        // case 2: adjusted wakeup less than current set value, need to program
        local_apic.timer_setup(IDT_LAPIC_TIMER, false);
        local_apic.timer_set(ADJUST((timeout-now)/bus_ratio));
    }
}

/*
 * notify_tcb
 *
 * Ack on a notify bit if it accepts notifications and the thread is
 * active.  
 *
 * Enqueue states whether we want to enqueue it.
 *
 * Notify_tcb() is safe to call multiple times, provided that nothing is
 * enqueued twice
 *
 * Returns whether thread can be used on a switch_to() or not
 */
bool
notify_tcb(tcb_t * receiver, word_t notify_bits, bool enqueue)
{
    tcb_t * current = get_current_tcb();

    // if the handler TID is not valid -- we don't want to send an IPC
    if (EXPECT_FALSE(!receiver->get_utcb() != NULL)) {
        goto out;
    }
    // all seems well ... tack on the notify bits
    receiver->add_notify_bits(notify_bits);

    // We also have to check for the waiting state here, if it is
    // waiting we should signal it  -- we check against the current
    // tcb to see whether we should enqueue it on the scheduler queue
    // or simply mark as running
    if (EXPECT_TRUE(receiver->get_acceptor().accept_notify())
        && (receiver->get_state().is_waiting_notify()
            || (receiver->get_partner().is_anythread()
                && receiver->get_state().is_waiting()))) {
        if (enqueue
            // FIXME: Replace with appropriate scheduler calls - AGW.
            || !get_current_scheduler()->check_dispatch_thread(current,
                                                               receiver)) {
            // only set running if enqueuing: we want the is_waiting
            // to match in check_async, same for set_partner: we want the
            // check on anythread to succeed
            // FIXME: Migrate into scheduler_t calls - AGW.
            receiver->set_state(thread_state_t::running);
            receiver->set_partner(NILTHREAD);
            get_current_scheduler()->enqueue_ready(receiver);
            enqueue = false;    // switch_to() not OK
        }
        else {
            enqueue = true;     // switch_to() OK
        }
    }
    else {
        enqueue = false;        // will pick up on next Wait()
    }
out:
    return enqueue;
}

/*
 * Determine if the given thread is waiting for an asynchronous
 * notification message.
 */
static bool
is_waiting_for_notification(tcb_t * tcb)
{
    /* Are they accepting notifications? */
    if (!receiver->get_acceptor().accept_notify()) {
        return false;
    }

    /* Is the thread waiting for notification directly? */
    if (receiver->get_state().is_waiting_notify()) {
        return true;
    }

    /* Is the thread waiting for anythread, and is accepting
     * asynchronous notiications? */
    if (receiver->get_partner().is_anythread() &&
            receiver->get_state().is_waiting()) {
        return true;
    }

    /* Otherwise, they are not waiting. */
    return false;
}

//extern void check_async_ipc(void);

// Check this - I think this seems to work?  -gl
// Should be ok even for non-waiting case: once they call Wait()
// the async will be delivered, if not then won't get the async anyway
// The assumption here is that the thread already has the notify bit
// set, and is NOT on the ready queue
void
send_async(tcb_t * receiver, word_t notify_bits, continuation_t continuation)
{
    tcb_t * current = get_current_tcb();

    /* Does the receiver have a bit set that will wake them up? */
    if (!(receiver->get_notify_bits() & receiver->get_notify_mask())) {
        dprintf("bits not set\n");
        ACTIVATE_CONTINUATION(continuation);
        /* NOTREACHED */
    }

    /* Is the receiver's UTCB setup? */
    if (EXPECT_FALSE(receiver->get_utcb() == NULL)) {
        dprintf("not activated\n");
        ACTIVATE_CONTINUATION(continuation);
        /* NOTREACHED */
    }

    /* Are the waiting for notification? */
    if (!is_waiting_for_notification(receiver)) {
        ACTIVATE_CONTINUATION(continuation);
    }

    /* Wake up the thread and perform a schedule. */
    receiver->set_partner(NILTHREAD);
    dprintf("switch_to currernt %p receiver %p cont %p\n",
            current, receiver, continuation);

    get_current_scheduler()->activate_sched(receiver,
            thread_state_t::running, current, continuation,
            scheduler_t::preempting_thread);

    /* NOTREACHED */
}

