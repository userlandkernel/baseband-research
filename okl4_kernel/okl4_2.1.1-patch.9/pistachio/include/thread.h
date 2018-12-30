/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   thread operations
 */
#ifndef __THREAD_H__
#define __THREAD_H__

#include <kernel/rootserver.h>

#if defined(L4_32BIT)
#define ANYTHREAD_RAW       0xffffffffUL
#define WAITNOTIFY_RAW      0xfffffffeUL
#define MYSELF_RAW          0xfffffffdUL
#define SPECIAL_RAW_LIMIT   0xfffffffcUL
#define INVALID_RAW         0x7fffffffUL

#elif defined(L4_64BIT)
#define ANYTHREAD_RAW       0xffffffffffffffffUL
#define WAITNOTIFY_RAW      0xfffffffffffffffeUL
#define MYSELF_RAW          0xfffffffffffffffdUL
#define SPECIAL_RAW_LIMIT   0xfffffffffffffffcUL
#define INVALID_RAW         0x7fffffffffffffffUL

#else
#error Unknown wordsize
#endif

class threadid_t
{
public:
    static threadid_t anythread() {
        threadid_t tid;
        tid.raw = ANYTHREAD_RAW;
        return tid;
    };

    static threadid_t waitnotify() {
        threadid_t tid;
        tid.raw = WAITNOTIFY_RAW;
        return tid;
    };

    static threadid_t nilthread()
        {
            threadid_t tid;
            tid.raw = 0;
            return tid;
        }

    static threadid_t irqthread(word_t irq)
        {
            threadid_t tid;
            tid.raw = 0;
            tid.global.version = 1;
            tid.global.threadno = irq;
            return tid;
        }

    static threadid_t threadid(word_t threadno, word_t version)
        {
            threadid_t tid;
            tid.raw = 0;
            tid.global.version = version;
            tid.global.threadno = threadno;
            return tid;
        }

    inline bool is_global() { return (global.version % 64 != 0) &&
                                  (global.threadno != threadid_t::anythread().global.threadno); }

    /* check for specific (well known) thread ids */
    bool is_nilthread() { return this->raw == 0; }
    bool is_anythread() { return this->raw == ANYTHREAD_RAW; }
    bool is_waitnotify(){ return this->raw == WAITNOTIFY_RAW; }
    bool is_myself(){ return this->raw == MYSELF_RAW; }

    bool is_threadhandle()
        {
            return (this->raw & (1UL << (BITS_WORD - 1))) &&
                   (this->raw < SPECIAL_RAW_LIMIT);
        }

    word_t get_threadno() { return global.threadno; }
    word_t get_version() { return global.version; }
    word_t get_irqno() { return get_threadno(); }
    void set_global_id(word_t threadno, word_t version);

    word_t get_raw() { return this->raw; }
    void set_raw(word_t raw) { this->raw = raw; }

    /* operators */
    bool operator == (const threadid_t & tid) const
        {
            return this->raw == tid.raw;
        }

    bool operator != (const threadid_t & tid) const
        {
            return this->raw != tid.raw;
        }

private:
    union {
        word_t raw;

        struct {
            BITFIELD2( word_t,
                       version  : L4_GLOBAL_VERSION_BITS,
                       threadno : L4_GLOBAL_THREADNO_BITS );
        } global;
    };
};

INLINE void threadid_t::set_global_id(word_t threadno, word_t version)
{
    global.threadno = threadno;
    global.version = version;
}

INLINE threadid_t threadid (word_t rawid)
{
    threadid_t t;
    t.set_raw (rawid);
    return t;
}

INLINE threadid_t threadhandle (word_t handle)
{
    threadid_t t;
    t.set_raw(1UL << (BITS_WORD -1) | handle);
    return t;
}

/* special thread ids */
#define NILTHREAD       (threadid_t::nilthread())
#define ANYTHREAD       (threadid_t::anythread())
#define WAITNOTIFY      (threadid_t::waitnotify())


#endif /* !__THREAD_H__ */
