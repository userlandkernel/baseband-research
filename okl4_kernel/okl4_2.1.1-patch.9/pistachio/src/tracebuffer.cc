/*
 * Copyright (c) 2005, National ICT Australia
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
 * Description:   tracebuffer interface
 */

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <schedule.h>
#include <arch/hwspace.h>
#include <space.h>
#include <memdesc.h>

#if defined(CONFIG_TRACEBUFFER)
#include <stdarg.h>     /* for va_list, ... comes with gcc */

word_t tb_irq = ~0UL;
bool tb_irq_masked = true;
bool tb_irq_pending = false;

DECLARE_KMEM_GROUP (kmem_trace);

trace_buffer_t * trace_buffer = NULL;
spinlock_t trace_lock;

SECTION(SEC_INIT) void init_tracebuffer()
{
    word_t buffer_first;
    
    trace_buffer = (trace_buffer_t*) kmem.alloc( kmem_trace, TBUFF_SIZE, true );
    ASSERT(ALWAYS, trace_buffer);

    /* Setup tracebuffer header */
    trace_buffer->magic     = (word_t)TBUF_MAGIC;
    trace_buffer->version   = TBUF_VERSION;
    trace_buffer->bufid     = TBUF_ID;
    trace_buffer->buffers   = TBUF_BUFFERS;

    /* Calculate buffer size */
    buffer_first = sizeof(trace_buffer_t) + 7 & (~7UL);
    trace_buffer->buffer_size =
        ((TBUFF_SIZE - buffer_first) / TBUF_BUFFERS) & (~7UL);

    /* Setup buffer offsets + head pointers */
    for (int i = 0; i < TBUF_BUFFERS; i ++ )
    {
        trace_buffer->buffer_head[i] = 0;
        trace_buffer->buffer_offset[i] = buffer_first + (i * trace_buffer->buffer_size);
    }

    /* Set active buffer = 0 and set rest to emtpy */
    trace_buffer->active_buffer = 0;
    trace_buffer->buffer_empty = ((1UL << TBUF_BUFFERS) - 1) &
        (~(1UL << trace_buffer->active_buffer));

    /* By default, enable all traces */
    trace_buffer->log_mask = ~(0UL);

    TRACE_INIT("Initialized tracebuffer @ %p\n", virt_to_phys(trace_buffer));
}

/*
 * flip to the next available tracebuffer + do virtual interrupt
 * note: tracebuffer lock is held here
 */
static word_t
tb_switch_buffers()
{
    word_t orig_buffer = trace_buffer->active_buffer;
    word_t next_buffer = trace_buffer->active_buffer;
    word_t empty_bitmask = trace_buffer->buffer_empty;
    word_t nbufs = trace_buffer->buffers;

    do {
        next_buffer++;
        if (next_buffer >= nbufs)
            next_buffer = 0;

        if ((1UL << next_buffer) & empty_bitmask)
        {
            /* Set next active buffer, mark it as not empty */
            //printf("switch buffer -> %d\n", next_buffer);
            trace_buffer->active_buffer = next_buffer;
            trace_buffer->buffer_empty = empty_bitmask & (~(1UL << next_buffer));
            trace_buffer->buffer_head[next_buffer] = 0;

            return next_buffer;
        }
    } while (next_buffer != orig_buffer);

#if defined(CONFIG_DEBUG) && (CONFIG_ASSERT_LEVEL >= DEBUG)
        //TRACEF( "tracebuffer overflow!\n");
#endif
    trace_buffer->active_buffer = next_buffer = ~(0UL);

    return ~(0UL);
}

/* Alocate space in the tracebuffer */
static word_t
tb_allocate(word_t entry_size, word_t *buffer)
{
    word_t buf;
    word_t entry;

    /* try allocate space in the tracebuffer */
    trace_lock.lock();
    {
        buf = trace_buffer->active_buffer;

        /* no active buffer? */
        if (buf == ~(0UL))
            goto alloc_error;

        word_t new_head = trace_buffer->buffer_head[buf] + entry_size;

        /* test for enough space for this entry and buffer terminating entry */
        if (EXPECT_FALSE(new_head >= trace_buffer->buffer_size))
        {
            buf = tb_switch_buffers();
            if (buf == ~(0UL))
                goto alloc_error;
            new_head = trace_buffer->buffer_head[buf] + entry_size;
        }
        /* enough space, save current position and increment head */
        entry = trace_buffer->buffer_head[buf];
        trace_buffer->buffer_head[buf] = new_head;
    }

    trace_lock.unlock();

    *buffer = buf;
    return entry;

alloc_error:
    trace_lock.unlock();
    return ~(0UL);
}

/* Log an event */
void tb_log_event(word_t traceid)
{
    word_t entry_size = sizeof(trace_entry_t);
    word_t entry, buffer;

    /* try allocate space in the tracebuffer */
    entry = tb_allocate(entry_size, &buffer);
    //printf("event: (%d, %d, %d) %d\n", buffer, entry_size, entry, traceid);

    if (entry == ~(0UL))
        return;

    /* get the tracebuffer entry */
    trace_entry_t * tbe = (trace_entry_t*)((word_t)trace_buffer +
            trace_buffer->buffer_offset[buffer] + entry);

    /* Write out tracebuffer event entry */
    u64_t time = get_current_time();
    tbe->timestamp_lo = time & 0xffffffffULL;
    tbe->timestamp_hi = (time>>32);
    tbe->hdr.x.id = (word_t)EVENT;
    tbe->hdr.x.user = 0;
    tbe->hdr.x.reclen = entry_size/sizeof(word_t);
    tbe->hdr.x.args = 1;
    tbe->hdr.x.major = (word_t)EVENT_major;
    tbe->hdr.x.minor = 0;

    tbe->data[0] = traceid;
}

/* Log a full tracebuffer entry */
void tb_log_trace(word_t data, char *str, ...)
{
    va_list args;
    word_t major = data >> 24;
    word_t traceid = (data >> 8 & 0xffff);
    int nargs = (data & 0xf);

    /* calc entry_size, works for nargs = 0 due to data[1] in trace_entry_t */
    word_t entry_size = sizeof(trace_entry_t) + (sizeof(word_t)*(nargs-1));
    word_t entry, buffer;

    /* try allocate space in the tracebuffer */
    entry = tb_allocate(entry_size, &buffer);

    //printf("entry: (%d, %d, %d) (%d, %d, %d)\n", buffer, entry_size, entry, major, traceid, nargs);

    if (entry == ~(0UL))
        return;

    /* get the tracebuffer entry */
    trace_entry_t * tbe = (trace_entry_t*)((word_t)trace_buffer +
            trace_buffer->buffer_offset[buffer] + entry);

    /* Write out tracebuffer event entry */
    u64_t time = get_current_time();
    tbe->timestamp_lo = time & 0xffffffffULL;
    tbe->timestamp_hi = (time>>32);
    tbe->hdr.x.id = traceid;
    tbe->hdr.x.user = 0;
    tbe->hdr.x.reclen = entry_size/sizeof(word_t);
    tbe->hdr.x.args = nargs;
    tbe->hdr.x.major = major;
    tbe->hdr.x.minor = 0;

    int i = 0;
    va_start(args, str);
    while (i < nargs) {
        tbe->data[i++] = va_arg(args, word_t);;
    }
    va_end(args);
}

/* Virtual tracebuffer interrupt handling */
bool tb_reply_irq()
{
    if (tb_irq_pending)
    {
        tb_irq_pending = false;
        return true;
    }

    tb_irq_masked = false;
    return false;
}

void tb_irq_control(bool enable)
{
    tb_irq_masked = !enable;
}

#endif /* CONFIG_TRACEBUFFER */
