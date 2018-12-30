/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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

#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

class tcb_t;

/**
 *  @brief End-point objects. End-points are a form of synchronisation
 *  point which implement inter-process communication for users.
 *
 *  The end-point's send and receive queues are both backed by the
 *  sync_point member's blocked-queue. As end-points are currently
 *  tied a single thread the receive-queue is only used by schedule
 *  inheritance and is conditionally compiled in.
 */
class endpoint_t
{
public:

    /**
     *  Initialise this syncpoint.
     *
     *  @param tcb The TCB reference of thread this end-point is a
     *  part of.
     */
    void init(tcb_t * tcb);

    /**
     *  Find the head of this end-point's send-queue, if any.
     *
     *  @return The TCB reference of the head thread.
     *  @retval NULL This end-point's send-queue is empty.
     */
    tcb_t * get_send_head(void);

    /**
     *  Enqueue argument thread onto this end-point's send-queue.
     *
     *  @param tcb The TCB reference of the thread to enqueue.
     */
    void enqueue_send(tcb_t * tcb);

    /**
     *  Dequeue argument thread from this end-point's send-queue.
     *
     *  @param tcb The TCB reference of the thread to dequeue.
     */
    void dequeue_send(tcb_t * tcb);

    /**
     *  Find the head of this end-point's receive-queue, if any.
     *
     *  @return The TCB reference of the head thread.
     *  @retval NULL This end-point's receive-queue is empty.
     */
    tcb_t * get_recv_head(void);

    /**
     *  Enqueue argument thread onto this end-point's receive-queue.
     *
     *  @param tcb The TCB reference of the thread to enqueue.
     */
    void enqueue_recv(tcb_t * tcb);

    /**
     *  Dequeue argument thread from this end-point's send-queue
     *
     *  @param tcb The TCB reference of the thread to dequeue.
     */
    void dequeue_recv(tcb_t * tcb);

    /**
     *  Get the send queue syncpoint.
     */
    syncpoint_t * get_send_syncpoint(void);

    /**
     *  Get the receive queue syncpoint.
     */
    syncpoint_t * get_recv_syncpoint(void);

private:

    /** The sync-point used to back this end-points send queue. */
    syncpoint_t send_queue;

    /* The receive queue is only required for schedule inheritance. */
#if defined(CONFIG_SCHEDULE_INHERITANCE)

    /** The sync-point used to back this end-points receive queue. */
    syncpoint_t recv_queue;

#endif

    friend void mkasmsym(void);
};

INLINE tcb_t *
endpoint_t::get_send_head(void)
{
    return this->send_queue.get_blocked_head();
}

INLINE tcb_t *
endpoint_t::get_recv_head(void)
{
#if defined(CONFIG_SCHEDULE_INHERITANCE)
    return this->recv_queue.get_blocked_head();
#else
    return NULL;
#endif
}

INLINE syncpoint_t *
endpoint_t::get_send_syncpoint(void)
{
    return &send_queue;
}

#if defined(CONFIG_SCHEDULE_INHERITANCE)
INLINE syncpoint_t *
endpoint_t::get_recv_syncpoint(void)
{
    return &recv_queue;
}
#endif

#endif /* __ENDPOINT_H__ */
