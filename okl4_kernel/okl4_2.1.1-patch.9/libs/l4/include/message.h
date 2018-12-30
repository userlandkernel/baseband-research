/*
 * Copyright (c) 2001, 2002, 2003-2004, Karlsruhe University
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
 * Description:   Message construction functions
 */
#ifndef __L4__MESSAGE_H__
#define __L4__MESSAGE_H__

#include <l4/types.h>
#include <l4/utcb.h>

#if defined(L4_64BIT)
# define __PLUS32       + 32
#else
# define __PLUS32
#endif

/*
 * Message tag
 */

typedef union {
    L4_Word_t raw;
    struct {
        L4_BITFIELD4(L4_Word_t, u:6, __res:6, flags:4, label:16 __PLUS32);
    } X;
} L4_MsgTag_t;

#define L4_Niltag               L4_Make_MsgTag(0UL, 0UL)
#define L4_Notifytag            L4_Make_MsgTag(0UL, 1UL << 1)
#define L4_Waittag              L4_Make_MsgTag(0UL, 1UL << 2)

L4_INLINE L4_MsgTag_t
L4_Make_MsgTag(L4_Word_t label, L4_Word_t flags)
{
    L4_MsgTag_t tag = { 0UL };

    tag.X.label = label;
    tag.X.flags = flags;
    return tag;
}

L4_INLINE L4_Bool_t
L4_IsMsgTagEqual(const L4_MsgTag_t l, const L4_MsgTag_t r)
{
    return l.raw == r.raw;
}

L4_INLINE L4_Bool_t
L4_IsMsgTagNotEqual(const L4_MsgTag_t l, const L4_MsgTag_t r)
{
    return l.raw != r.raw;
}

#if defined(__l4_cplusplus)
static inline L4_Bool_t
operator ==(const L4_MsgTag_t &l, const L4_MsgTag_t &r)
{
    return l.raw == r.raw;
}

static inline L4_Bool_t
operator !=(const L4_MsgTag_t &l, const L4_MsgTag_t &r)
{
    return l.raw != r.raw;
}
#endif /* __l4_cplusplus */

L4_INLINE L4_MsgTag_t
L4_MsgTagAddLabel(const L4_MsgTag_t t, L4_Word_t label)
{
    L4_MsgTag_t tag = t;

    tag.X.label = label;
    return tag;
}

L4_INLINE L4_MsgTag_t
L4_MsgTagAddLabelTo(L4_MsgTag_t *t, L4_Word_t label)
{
    t->X.label = label;
    return *t;
}

#if defined(__l4_cplusplus)
static inline L4_MsgTag_t
operator +(const L4_MsgTag_t &t, L4_Word_t label)
{
    return L4_MsgTagAddLabel(t, label);
}

static inline L4_MsgTag_t
operator +=(L4_MsgTag_t &t, L4_Word_t label)
{
    return L4_MsgTagAddLabelTo(&t, label);
}
#endif /* __l4_cplusplus */

L4_INLINE L4_Word_t
L4_Label(L4_MsgTag_t t)
{
    return t.X.label;
}

L4_INLINE L4_Word_t
L4_UntypedWords(L4_MsgTag_t t)
{
    return t.X.u;
}

L4_INLINE void
L4_Set_Label(L4_MsgTag_t *t, L4_Word_t label)
{
    t->X.label = label;
}

L4_INLINE L4_MsgTag_t
L4_MsgTag(void)
{
    L4_MsgTag_t msgtag;

    L4_StoreMR(0, &msgtag.raw);
    return msgtag;
}

L4_INLINE void
L4_Set_MsgTag(L4_MsgTag_t t)
{
    L4_LoadMR(0, t.raw);
}

/*
 * Message objects
 */

typedef union {
    L4_Word_t raw[__L4_NUM_MRS];
    L4_Word_t msg[__L4_NUM_MRS];
    L4_MsgTag_t tag;
} L4_Msg_t;

L4_INLINE void
L4_MsgPut(L4_Msg_t *msg, L4_Word_t label, int u, L4_Word_t *Untyped)
{
    int i;

    for (i = 0; i < u; i++)
        msg->msg[i + 1] = Untyped[i];

    msg->tag.X.label = label;
    msg->tag.X.flags = 0;
    msg->tag.X.u = u;
    msg->tag.X.__res = 0;
}

L4_INLINE void
L4_MsgGet(const L4_Msg_t *msg, L4_Word_t *Untyped)
{
    int i, u;

    u = msg->tag.X.u;

    for (i = 0; i < u; i++)
        Untyped[i] = msg->msg[i + 1];
}

L4_INLINE L4_MsgTag_t
L4_MsgMsgTag(const L4_Msg_t *msg)
{
    return msg->tag;
}

L4_INLINE void
L4_Set_MsgMsgTag(L4_Msg_t *msg, L4_MsgTag_t t)
{
    msg->tag = t;
}

L4_INLINE L4_Word_t
L4_MsgLabel(const L4_Msg_t *msg)
{
    return msg->tag.X.label;
}

L4_INLINE void
L4_Set_MsgLabel(L4_Msg_t *msg, L4_Word_t label)
{
    msg->tag.X.label = label;
}

L4_INLINE void
L4_MsgLoad(L4_Msg_t *msg)
{
    L4_LoadMRs(0, msg->tag.X.u + 1, &msg->msg[0]);
}

L4_INLINE void
L4_MsgStore(L4_MsgTag_t t, L4_Msg_t *msg)
{
    L4_StoreMRs(1, t.X.u, &msg->msg[1]);
    msg->tag = t;
}

L4_INLINE void
L4_MsgClear(L4_Msg_t *msg)
{
    msg->msg[0] = 0;
}

L4_INLINE void
L4_MsgAppendWord(L4_Msg_t *msg, L4_Word_t w)
{
    msg->msg[++msg->tag.X.u] = w;
}

L4_INLINE void
L4_MsgPutWord(L4_Msg_t *msg, L4_Word_t u, L4_Word_t w)
{
    msg->msg[u + 1] = w;
}

L4_INLINE L4_Word_t
L4_MsgWord(L4_Msg_t *msg, L4_Word_t u)
{
    return msg->msg[u + 1];
}

L4_INLINE void
L4_MsgGetWord(L4_Msg_t *msg, L4_Word_t u, L4_Word_t *w)
{
    *w = msg->msg[u + 1];
}

#if defined(__l4_cplusplus)
L4_INLINE void
L4_Put(L4_Msg_t *msg, L4_Word_t label, int u, L4_Word_t *Untyped)
{
    L4_MsgPut(msg, label, u, Untyped);
}

L4_INLINE void
L4_Get(const L4_Msg_t *msg, L4_Word_t *Untyped)
{
    L4_MsgGet(msg, Untyped);
}

L4_INLINE L4_MsgTag_t
L4_MsgTag(const L4_Msg_t *msg)
{
    return L4_MsgMsgTag(msg);
}

L4_INLINE void
L4_Set_MsgTag(L4_Msg_t *msg, L4_MsgTag_t t)
{
    L4_Set_MsgMsgTag(msg, t);
}

L4_INLINE L4_Word_t
L4_Label(const L4_Msg_t *msg)
{
    return L4_MsgLabel(msg);
}

L4_INLINE void
L4_Set_Label(L4_Msg_t *msg, L4_Word_t label)
{
    L4_Set_MsgLabel(msg, label);
}

L4_INLINE void
L4_Load(L4_Msg_t *msg)
{
    L4_MsgLoad(msg);
}

L4_INLINE void
L4_Store(L4_MsgTag_t t, L4_Msg_t *msg)
{
    L4_MsgStore(t, msg);
}

L4_INLINE void
L4_Clear(L4_Msg_t *msg)
{
    L4_MsgClear(msg);
}

L4_INLINE void
L4_Append(L4_Msg_t *msg, L4_Word_t w)
{
    L4_MsgAppendWord(msg, w);
}

L4_INLINE void
L4_Append(L4_Msg_t *msg, int w)
{
    L4_MsgAppendWord(msg, (L4_Word_t)w);
}

L4_INLINE void
L4_Put(L4_Msg_t *msg, L4_Word_t u, L4_Word_t w)
{
    L4_MsgPutWord(msg, u, w);
}

L4_INLINE L4_Word_t
L4_Get(L4_Msg_t *msg, L4_Word_t u)
{
    return L4_MsgWord(msg, u);
}

L4_INLINE void
L4_Get(L4_Msg_t *msg, L4_Word_t u, L4_Word_t *w)
{
    L4_MsgGetWord(msg, u, w);
}

#endif /* __l4_cplusplus */

/*
 * Acceptors and mesage buffers.
 */

typedef union {
    L4_Word_t raw;
    struct {
        L4_BITFIELD3(L4_Word_t, __zero1:1, notify:1, __zero2:30 __PLUS32);
    } X;
} L4_Acceptor_t;

#define L4_UntypedWordsAcceptor         L4_Acceptor(0)
#define L4_NotifyMsgAcceptor            L4_Acceptor(1)

#define L4_AsynchItemsAcceptor          L4_NotifyMsgAcceptor    // OBSOLETE

L4_INLINE L4_Acceptor_t
L4_Acceptor(L4_Bool_t notify)
{
    L4_Acceptor_t acceptor = { 0UL };

    acceptor.X.notify = notify;
    return acceptor;
}

L4_INLINE L4_Acceptor_t
L4_AddAcceptor(const L4_Acceptor_t l, const L4_Acceptor_t r)
{
    L4_Acceptor_t a;

    a.raw = 0;
    a.X.notify = (l.X.notify | r.X.notify);
    return a;
}

L4_INLINE L4_Acceptor_t
L4_AddAcceptorTo(L4_Acceptor_t l, const L4_Acceptor_t r)
{
    l.X.notify = (l.X.notify | r.X.notify);
    return l;
}

L4_INLINE L4_Acceptor_t
L4_RemoveAcceptor(const L4_Acceptor_t l, const L4_Acceptor_t r)
{
    L4_Acceptor_t a = l;

    if (r.X.notify)
        a.X.notify = 0;
    return a;
}

L4_INLINE L4_Acceptor_t
L4_RemoveAcceptorFrom(L4_Acceptor_t l, const L4_Acceptor_t r)
{
    if (r.X.notify)
        l.X.notify = 0;
    return l;
}

#if defined(__l4_cplusplus)
static inline L4_Acceptor_t
operator +(const L4_Acceptor_t &l, const L4_Acceptor_t &r)
{
    return L4_AddAcceptor(l, r);
}

static inline L4_Acceptor_t
operator +=(L4_Acceptor_t &l, const L4_Acceptor_t &r)
{
    return L4_AddAcceptorTo(l, r);
}

static inline L4_Acceptor_t
operator -(const L4_Acceptor_t &l, const L4_Acceptor_t &r)
{
    return L4_RemoveAcceptor(l, r);
}

static inline L4_Acceptor_t
operator -=(L4_Acceptor_t &l, const L4_Acceptor_t &r)
{
    return L4_RemoveAcceptorFrom(l, r);
}
#endif /* __l4_cplusplus */

L4_INLINE void
L4_Accept(const L4_Acceptor_t a)
{
    __L4_TCR_Set_Acceptor(a.raw);
}

L4_INLINE L4_Acceptor_t
L4_Accepted(void)
{
    L4_Acceptor_t a;

    a.raw = __L4_TCR_Acceptor();
    return a;
}

L4_INLINE void
L4_Set_NotifyMask(const L4_Word_t mask)
{
    __L4_TCR_Set_NotifyMask(mask);
}

L4_INLINE L4_Word_t
L4_Get_NotifyMask(void)
{
    L4_Word_t mask;

    mask = __L4_TCR_NotifyMask();
    return mask;
}

L4_INLINE void
L4_Set_NotifyBits(const L4_Word_t mask)
{
    __L4_TCR_Set_NotifyBits(mask);
}

L4_INLINE L4_Word_t
L4_Get_NotifyBits(void)
{
    L4_Word_t mask;

    mask = __L4_TCR_NotifyBits();
    return mask;
}

#undef __PLUS32

#endif /* !__L4__MESSAGE_H__ */
