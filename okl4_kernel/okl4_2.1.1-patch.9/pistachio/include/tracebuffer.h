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
#ifndef __TRACEBUFFER_H__
#define __TRACEBUFFER_H__

#define TBUF_MEMDESC_TYPE   (memdesc_type_t)0xb

class trace_entry_t {
public:
    u32_t timestamp_lo;
    u32_t timestamp_hi;
    union{
        struct {
#if defined(CONFIG_IS_64BIT)
            BITFIELD7(word_t,
                    id          : 16,   /* tracepoint id                */
                    user        : 1,    /* user / kernel                */
                    reclen      : 7,    /* number of words of data      */
                    args        : 8,    /* number of arguments          */
                    major       : 6,    /* major number                 */
                    minor       : 10,   /* minor number                 */
                                : 16    /* unused                       */
                    );
#else /* 32BIT */
            BITFIELD6(word_t,
                    id          : 8,    /* tracepoint id                */
                    user        : 1,    /* user / kernel                */
                    reclen      : 7,    /* number of words of data      */
                    args        : 5,    /* number of arguments          */
                    major       : 5,    /* major number                 */
                    minor       : 6     /* minor number                 */
                    );
#endif
        } x;
        word_t raw;
    } hdr;
    word_t data[1];     /* code assumes size (1 word) here, do not change */
};

#define TBUF_MAGIC      0x7b6b5b4b3b2b1b0bULL
#define TBUF_VERSION    1
#define TBUF_ID         1       /* L4 tracebuffer */
#define TBUF_BUFFERS    2

class trace_buffer_t {
public:
    word_t magic;               /* magic number         */
    word_t version;             /* tracebuffer version  */
    word_t bufid;               /* tracebuffer id       */
    word_t buffers;             /* number of buffers    */

    word_t log_mask;            /* mask of major IDs    */
    word_t active_buffer;       /* current active buffer        */
    word_t buffer_empty;        /* bitmask of available buffers */
    word_t buffer_size;         /* size of each buffer (bytes)  */

    /* offset of each buffer    */
    word_t buffer_offset[TBUF_BUFFERS];
    /* current head pointer of each buffer (past last entry) */
    word_t buffer_head[TBUF_BUFFERS];
};

extern trace_buffer_t * trace_buffer;

/* Tracebuffer helpper functions */
void tb_log_event(word_t traceid);

void tb_log_trace(word_t data, char *str, ...);

INLINE void tb_log_trace(word_t major, word_t traceid, char *str, word_t a)
{
    tb_log_trace((major << 24) | (traceid << 8) | 1, str, a);
}

INLINE void tb_log_trace(word_t major, word_t traceid, char *str, word_t a, word_t b)
{
    tb_log_trace((major << 24) | (traceid << 8) | 2, str, a, b);
}

INLINE void tb_log_trace(word_t major, word_t traceid, char *str, word_t a, word_t b, word_t c)
{
    tb_log_trace((major << 24) | (traceid << 8) | 3, str, a, b, c);
}

INLINE void tb_log_trace(word_t major, word_t traceid, char *str, word_t a, word_t b, word_t c, word_t d)
{
    tb_log_trace((major << 24) | (traceid << 8) | 4, str, a, b, c, d);
}


/*
 * Trace buffer
 */
#if defined(CONFIG_TRACEBUFFER)
void init_tracebuffer();
#define TBUF_RECORD(tp, tbs, tba...)                                    \
    if (EXPECT_FALSE(trace_buffer && trace_buffer->log_mask & (1UL << tp##_major))) \
    {                                                                   \
        tb_log_trace(tp##_major, tp, tbs, tba);                         \
    }

#define TBUF_RECORD_EVENT(tp)                                           \
    if (EXPECT_FALSE(trace_buffer && trace_buffer->log_mask & (1UL << tp##_major))) \
        tb_log_event(tp)

#define TBUF_CLEAR_BUFFER()

# if defined(CONFIG_KDB_NO_ASSERTS)
#  undef TBUF_RECORD
#  define TBUF_RECORD(_a...)
#  undef TBUF_RECORD_EVENT
#  define TBUF_RECORD_EVENT(_a)
#  undef TBUF_CLEAR_BUFFER
#  define TBUF_CLEAR_BUFFER()
# endif /* defined(CONFIG_KDB_NO_ASSERTS) */
#else /* !CONFIG_TRACEBUFFER */
# define init_tracebuffer()
# define TBUF_RECORD(_a...)
# define TBUF_RECORD_EVENT(_a)
# define TBUF_CLEAR_BUFFER()
#endif /* !CONFIG_TRACEBUFFER */

#endif
