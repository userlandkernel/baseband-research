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
 * Description:   tracebuffer KDB interface
 */

#include <l4.h>
#include <debug.h>
#include <tracebuffer.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <kdb/tracepoints.h>

#if defined(CONFIG_TRACEBUFFER) && defined(CONFIG_KDB_CLI)

DECLARE_CMD_GROUP (tracebuf);

DECLARE_CMD (cmd_tracebuffer, root, 'b', "tracebuffer", "tracebuffer menu");

CMD(cmd_tracebuffer, cg)
{
    if (!trace_buffer)
    {
        printf("Invalid trace_buffer (NULL)\n");
        return CMD_NOQUIT;
    }

    return tracebuf.interact (cg, "tracebuffer");
}

DECLARE_CMD (cmd_tb_info, tracebuf, 'i', "info", "show tracebuffer info");

CMD(cmd_tb_info, cg)
{
    printf("=== Tracebuffer @ %p, magic = %p ===\n", trace_buffer, (void*)trace_buffer->magic);
    printf("version: %8x, bufid:         %8x\n", trace_buffer->version, trace_buffer->bufid);
    printf("buffers: %8d, active_buffer: %8d\n", trace_buffer->buffers, trace_buffer->active_buffer);
    printf("bufsize: %8d, empty mask     %8x\n", trace_buffer->buffer_size, trace_buffer->buffer_empty);
    printf("logmask: %8x\n", trace_buffer->log_mask);
    for (int i = 0; i < (int)trace_buffer->buffers; i++)
    {
        printf("  buffer %d: offset: %8d,  head: %8d\n", i,
                trace_buffer->buffer_offset[i], trace_buffer->buffer_head[i]);
    }
    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_tb_logmask, tracebuf, 'l', "logmask", "change logmask");

CMD(cmd_tb_logmask, cg)
{
    word_t logmask = get_hex ("Log Mask", trace_buffer->log_mask);

    if (logmask == ABORT_MAGIC)
        return CMD_NOQUIT;

    trace_buffer->log_mask = logmask;

    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_tb_dump, tracebuf, 'd', "dump", "dump current buffer");

CMD(cmd_tb_dump, cg)
{
    word_t buffer = trace_buffer->active_buffer;
    word_t head = trace_buffer->buffer_head[buffer];
    word_t index = 0;

    if (buffer == ~(0UL))
    {
        printf("No current active buffer!\n");
        return CMD_NOQUIT;
    }

    while (index < head)
    {
        trace_entry_t * tbe = (trace_entry_t*)((word_t)trace_buffer +
                trace_buffer->buffer_offset[buffer] + index);

        printf("%6d: %9m ", index, ((u64_t)tbe->timestamp_hi << 32) | tbe->timestamp_lo );

        switch (tbe->hdr.x.id) {
        case EVENT:
            printf("(%d, %d, %d) %s", tbe->hdr.x.major, tbe->hdr.x.minor,
                    tbe->hdr.x.user, trace_names[tbe->data[0]]);
            break;
        case STRING:
            printf("(%d, %d, %d) \"%s\"", tbe->hdr.x.major, tbe->hdr.x.minor,
                    tbe->hdr.x.user, (char*)tbe->data);
            break;
        default:
            printf("(%d, %d, %d) %s: ",
                    tbe->hdr.x.major, tbe->hdr.x.minor, tbe->hdr.x.user,
                    trace_names[tbe->hdr.x.id]);

            for ( int i = 0; i < (int)tbe->hdr.x.args; i++ )
            {
                if (i) {
                    printf(", ");
                }
                printf("%p", (void*)tbe->data[i]);
            }
            break;
        }

        printf("\n");

        index += (tbe->hdr.x.reclen * sizeof(word_t));
    }

    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_tb_reset, tracebuf, 'r', "reset", "reset (zero) tracebuffer");

CMD(cmd_tb_reset, cg)
{
    word_t nbufs = trace_buffer->buffers;

    trace_buffer->active_buffer = 0;
    trace_buffer->buffer_empty = ((1UL << nbufs) - 1) &
        (~(1UL << trace_buffer->active_buffer));

    /* Setup buffer offsets + head pointers */
    for (word_t i = 0; i < nbufs; i ++ )
    {
        trace_buffer->buffer_head[i] = 0;
    }

    return CMD_NOQUIT;
}

#endif
