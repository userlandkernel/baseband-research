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
 * Copyright (c) 2005, National ICT Australia (NICTA)
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
 * Description:   IPC declarations
 */
#ifndef __IPC_H__
#define __IPC_H__

class tcb_t;

/**
 * Message protocol tags
 */
#if defined(L4_32BIT)
#define IRQ_TAG                        0xfffffff0UL
#define PAGEFAULT_TAG                  0xffffffe0UL
#define PREEMPTION_TAG                 0xffffffd0UL
#elif defined(L4_64BIT)
#define IRQ_TAG                        0xfffffffffffffff0UL
#define PAGEFAULT_TAG                  0xffffffffffffffe0UL
#define PREEMPTION_TAG                 0xffffffffffffffd0UL
#else
#error Unkwown wordsize
#endif

/**
 * Error codes
 */
#define ERR_IPC_NOPARTNER               (1)
#define ERR_IPC_NON_EXISTING            (2)
#define ERR_IPC_CANCELED                (3)
#define ERR_IPC_MSG_OVERFLOW            (4)
#define ERR_IPC_NOT_ACCEPTED            (5)
#define ERR_IPC_ABORTED                 (7)

/**
 * Error encoding
 */
#define IPC_SND_ERROR(err)              ((err << 1) | 0)
#define IPC_RCV_ERROR(err)              ((err << 1) | 1)

/**
 * MR0 values
 */
#define IPC_MR0_REDIRECTED              (1 << 13)
#define IPC_MR0_XCPU                    (1 << 14)
#define IPC_MR0_ERROR                   (1 << 15)

class msg_tag_t
{
public:
    word_t get_label() { return send.label; }
    word_t get_untyped() { return send.untyped; }
    bool get_memcpy() { return send.memcpy; }

    void clear_flags() { raw &= ~(0xf << 12);}
    void clear_receive_flags() { raw &= ~(0xf << 12); }

    void set(word_t untyped, word_t label,
            bool send_blocks, bool recv_blocks)
        {
            this->raw = 0;
            this->send.untyped = untyped;
            this->send.label = label;
            this->send.rcvblock = recv_blocks;
            this->send.sndblock = send_blocks;
        }

    bool recv_blocks() { return send.rcvblock; }
    void set_recv_blocks(bool val = true) { send.rcvblock = val; }

    bool send_blocks() { return send.sndblock; }
    void set_send_blocks(bool val = true) { send.sndblock = val; }

    bool is_notify() { return send.notify; }

    bool is_error() { return recv.error; }
    void set_error() { recv.error = 1; }

    static msg_tag_t nil_tag()
        {
            msg_tag_t tag;
            tag.raw = 0;
            return tag;
        }

    static msg_tag_t error_tag()
        {
            msg_tag_t tag;
            tag.raw = 0;
            tag.set_error();
            return tag;
        }

    static msg_tag_t tag(word_t untyped, word_t label,
            bool send_blocks, bool recv_blocks)
        {
            msg_tag_t tag;
            tag.set(untyped, label, send_blocks, recv_blocks);
            return tag;
        }

    static msg_tag_t irq_tag()
        {
            /* this is an output parameter */
            return tag(0, IRQ_TAG, false, false);
        }

    static msg_tag_t notify_tag()
        {
            return tag (1, 0, false, false);
        }

    static msg_tag_t preemption_tag()
        {
            return tag (0, PREEMPTION_TAG, true, true);
        }

    static msg_tag_t pagefault_tag(bool read, bool write, bool exec)
        {
            return tag (2, (PAGEFAULT_TAG) |
                        (read  ? 1 << 2 : 0) |
                        (write ? 1 << 1 : 0) |
                        (exec  ? 1 << 0 : 0),
                        true, true);
        }
public:
    union {
        word_t raw;
        struct {
            BITFIELD7(word_t,
                      untyped           : 6,
                      __res             : 6,
                      memcpy            : 1,
                      notify            : 1,
                      rcvblock          : 1,
                      sndblock          : 1,
                      label             : BITS_WORD - 16);
        } send;
        struct {
            BITFIELD7(word_t,
                      untyped           : 6,
                      __res1            : 6,
                      __res2            : 1,
                      __res             : 1,
                      xcpu              : 1,
                      error             : 1,
                      label             : BITS_WORD - 16);
        } recv;
    };
};

INLINE msg_tag_t msgtag (word_t rawtag)
{
    msg_tag_t t;
    t.raw = rawtag;
    return t;
}

class acceptor_t
{
public:
    inline void clear()
        { this->raw = 0; }

    inline void operator = (word_t raw)
        { this->raw = raw; }

    inline bool accept_notify()
        { return x.notify; }

    static inline acceptor_t untyped_words()
        {
            acceptor_t x;
            x.raw = 0;
            return x;
        }
public:
    union {
        word_t raw;
        struct {
            BITFIELD3(word_t,
                      _res1             : 1,
                      notify            : 1,
                      _res2             : BITS_WORD - 2);
        } x;
    };
};

#endif /* !__IPC_H__ */
