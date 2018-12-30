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
 * Author: Carl van Schaik Created: Mon Oct 10 2005 
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

#include <l4/config.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/schedule.h>

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/env.h>
#include <iguana/thread.h>
#include <iguana/trace.h>
#include <trace/tracebuffer.h>
/* FIXME See Mothra bug #2106 
#include <timer/timer.h>
*/

#include <util/trace.h>

#define IRQ_LABEL (((~(0UL)) << 20) >> 16)

#define TXT_NORMAL              "\e[0m"
#define TXT_BRIGHT              "\e[1m"
#define TXT_FG_GREEN            "\e[32m"

L4_Word_t kernel_trace_irq = 0;
trace_buffer_t *kernel_tracebuffer;

/* FIXME See Mothra bug #2106
timer_t timer;
*/

static int
setup(void)
{
    int r;
    L4_Word_t phys = 0, size = 0;
    uintptr_t addr;
    memsection_ref_t ms_tb;

    DEBUG_TRACE(1, "Trace server starting\n");

    kernel_tracebuffer = NULL;

    r = okl4_tracebuffer_phys(&phys, &size);
    if (r != 0) {
        DEBUG_TRACE(1, "No trace buffer\n");
        return 0;
    }

    /* Get tracebuffer and map to ourselves */
    ms_tb = memsection_create_user(size, &addr);
    if (ms_tb == 0) {
        DEBUG_TRACE(1, "Unable to create a trace buffer memsection\n");
        return 0;
    }
    r = hardware_back_memsection(ms_tb, phys, 0);
    if (r != 0) {
        DEBUG_TRACE(1, "Error backing memory section: (%d)\n", r);
        return 0;
    }

    kernel_tracebuffer = (trace_buffer_t *)addr;
    DEBUG_TRACE(1, "kernel tracebuffer %p(%" PRIdPTR ") at %p\n", (void *)phys, size,
           kernel_tracebuffer);


    /* FIXME See Mothra bug #2106 */
    return 1;
}

#ifdef __arm__
#include <l4/cache.h>
static void
l4e_cache_flush(void)
{
    (void)L4_CacheFlushAll();
}
#else
static void
l4e_cache_flush(void)
{
}
#endif

static void
dump_kernel_trace(trace_buffer_t *tb)
{
    uintptr_t buffer, count;
    uintptr_t buffer_mask;

#if 0
    DEBUG_TRACE(1, "trace_buffer magic: %" PRIxPTR ", version: %" PRIxPTR ", bufid: %"
           PRIxPTR "\n", tb->magic, tb->version, tb->bufid);
#endif

    buffer = tb->active_buffer;
    buffer_mask = tb->buffer_empty;

    if (buffer == ~(0ul))       /* Trace inactive/blocked */
        buffer = 0;

    count = tb->buffers;
    while (count--) {
        /* Data to be read in this buffer?  */
        if ((buffer != tb->active_buffer) && ((~buffer_mask) & (1ul << buffer))) {
            uintptr_t head = tb->buffer_head[buffer];
            uintptr_t entry = 0;

            // DEBUG_TRACE(1, "Data in %d\n", buffer);

            /* Do statistics on buffer */
            {
                uintptr_t logs = 0, ipcs = 0;
                uint64_t start = 0, end = 0, total;

                while (entry < head) {
                    trace_entry_t *tbe = (trace_entry_t *)((uintptr_t)tb +
                                                           tb->
                                                           buffer_offset[buffer]
                                                           + entry);
                    end = trace_entry_timestamp(tbe);
                    if (logs == 0)
                        start = end;

                    /*
                     * XXX kernel hard coded here 
                     */
                    switch (tbe->hdr.x.id) {
                    case 1:
                        logs++;
                        break;
                    case 2:
                        logs++;
                        break;
                    default:
                        if (tbe->hdr.x.major == 3)
                            ipcs++;
                        logs++;
                        break;
                    }

                    entry = entry + (tbe->hdr.x.reclen * sizeof(uintptr_t));
                }

                total = end - start;
                DEBUG_TRACE(1, TXT_BRIGHT TXT_FG_GREEN
                       "TRACE: %" PRId64 " logs/s, %" PRId64 " ipc/s "
                       TXT_NORMAL "\n",
                       total ? (uint64_t)logs * 1000000 / (total) : -1,
                       total ? (uint64_t)ipcs * 1000000 / (total) : -1);
            }
            /* mark this buffer as clear */
            tb->buffer_empty |= (1ul << buffer);
#if 0
            /* Dump trace buffer to console */
            while (entry < head) {
                trace_entry_t *tbe = (trace_entry_t *)((uintptr_t)tb +
                                                       tb->
                                                       buffer_offset[buffer] +
                                                       entry);

                DEBUG_TRACE(1, "%6d: %9" PRIx64 " ", entry, tbe->timestamp);

                /*
                 * XXX kernel hard coded here 
                 */
                switch (tbe->hdr.x.id) {
                case 1:
                    DEBUG_TRACE(1, "(%d, %d, %d) evt: %d", tbe->hdr.x.major,
                           tbe->hdr.x.minor, tbe->hdr.x.user, tbe->data[0]);
                    break;
                case 2:
                    DEBUG_TRACE(1, "(%d, %d, %d) \"%s\"", tbe->hdr.x.major,
                           tbe->hdr.x.minor, tbe->hdr.x.user,
                           (char *)tbe->data);
                    break;
                default:
                    DEBUG_TRACE(1, "(%d, %d, %d) %d: ",
                           tbe->hdr.x.major, tbe->hdr.x.minor, tbe->hdr.x.user,
                           tbe->hdr.x.id);

                    for (int i = 0; i < (int)tbe->hdr.x.args; i++)
                        DEBUG_TRACE(1, "%08" PRIxPTR ", ", tbe->data[i]);
                    break;
                }

                DEBUG_TRACE(1, "\n");

                entry = entry + (tbe->hdr.x.reclen * sizeof(uintptr_t));
            }
#endif
        }

        buffer++;
        if (buffer >= tb->buffers)
            buffer = 0;
    }
}

int
main(void)
{
    if (!setup()) {
        DEBUG_TRACE(1, "no kernel tracebuffer found\n");
        return 0;
    }

    L4_Set_Priority(thread_l4tid(env_thread(iguana_getenv("MAIN"))), 1);
    while (1) {
        L4_Yield();
        l4e_cache_flush();
        dump_kernel_trace(kernel_tracebuffer);
    }

    return 0;
}
