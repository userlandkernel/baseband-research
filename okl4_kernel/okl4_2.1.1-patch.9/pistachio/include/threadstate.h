/*
 * Copyright (c) 2002, Karlsruhe University
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
 * Description:  thread state
 */
#ifndef __THREADSTATE_H__
#define __THREADSTATE_H__

/**
 * thread state definitions
 *
 * VU: they should go into the architecture specific part to allow
 *     special optimised encoding
 */
#define RUNNABLE_STATE(id)      (((id) << 1) | 0)
#define BLOCKED_STATE(id)       (((id) << 1) | 1)

/**
 * thread_state_t: current thread state
 */
class thread_state_t
{
public:
    enum thread_state_e
    {
        running                 = RUNNABLE_STATE(1),
        waiting_forever         = BLOCKED_STATE(~0UL >> 1),
        waiting_notify          = BLOCKED_STATE(2),
        polling                 = BLOCKED_STATE(5),
        halted                  = BLOCKED_STATE(6),
        aborted                 = BLOCKED_STATE(7),
        xcpu_waiting_deltcb     = BLOCKED_STATE(8),
        xcpu_waiting_exregs     = BLOCKED_STATE(12),
        waiting_mutex           = BLOCKED_STATE(13)
    };

    /* constructors */
    thread_state_t(void) {};
    thread_state_t(thread_state_e state_) { this->state = state_; }
    thread_state_t(word_t state_) { this->state = (thread_state_e) state_; }

    /* state manipulation */
    bool is_runnable();
    bool is_sending()
        { return state == polling; }
    bool is_receiving()
        { return state == waiting_forever || state == waiting_notify; }
    bool is_halted()
        { return state == halted; }
    bool is_aborted()
        { return state == aborted; }
    bool is_running()
        { return state == running; }
    bool is_waiting()
        { return state == waiting_forever; }
    bool is_waiting_notify()
        { return state == waiting_notify; }
    bool is_waiting_forever()
        { return state == waiting_forever; }
    bool is_polling()
        { return state == polling; }
    bool is_xcpu_waiting()
        { return state == xcpu_waiting_deltcb ||
              state == xcpu_waiting_exregs; }
    bool is_waiting_mutex()
        {return state == waiting_mutex;}

    /* operators */
    bool operator == (const thread_state_t thread_state) const
        {
            return this->state == thread_state.state;
        }

    bool operator == (const thread_state_e state_) const
        {
            return this->state == state_;
        }

    bool operator != (const thread_state_t thread_state) const
        {
            return this->state != thread_state.state;
        }

    bool operator != (const thread_state_e state_) const
        {
            return this->state != state_;
        }

    operator word_t()
        {
            return (word_t)this->state;
        }

    /* debugging */
    const char * string (void)
        {
#ifdef CONFIG_DEBUG
            switch (state) {
            case running:               return "RUNNING";
            case waiting_forever:       return "WAIT_FOREVER";
            case waiting_notify:        return "WAIT_NOTIFY";
            case waiting_mutex:         return "WAIT_MUTEX";
            case polling:               return "POLLING";
            case halted:                return "HALTED";
            case aborted:               return "ABORTED";
            case xcpu_waiting_deltcb:   return "WAIT_XPU_DELTCB";
            case xcpu_waiting_exregs:   return "WAIT_XPU_EXREGS";
            default:                    return "UNKNOWN";
            }
#else
            return "";
#endif
        }

    thread_state_e state;
};

INLINE bool
thread_state_t::is_runnable()
{
    /* inverse logic - lowermost bit no set means runnable */
    return !((word_t)this->state & 1);
}


#endif /* __THREADSTATE_H__ */
