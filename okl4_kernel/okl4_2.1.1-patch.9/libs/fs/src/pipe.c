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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <fs/fs.h>

#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/thread.h>
#include <l4/types.h>
#include <iguana/memsection.h>

#define NOTIFY_MASK 0x4

void
block_myself(void)
{
    L4_MsgTag_t tag;
    L4_Word_t mask;

    L4_Accept(L4_NotifyMsgAcceptor);
    L4_Set_NotifyMask(0x4);

    do {
        tag = L4_WaitNotify(&mask);
    } while (mask != NOTIFY_MASK);
}

void
unblock_thread(L4_ThreadId_t tid)
{
    L4_MsgTag_t tag;

    tag = L4_Notify(tid, NOTIFY_MASK);
}

int
okl4_pipe(int fildes[2])
{
    int fd1, fd2, i;

    memsection_ref_t pipe_ref;
    uintptr_t pipe_base, pipe_size;
    struct fdesc *f;
    struct pdesc *p;

    /*
     * get 2 new file descriptors 
     */

    i = 3;
    while (ftable[i].refcount > 0) {
        i++;
    }
    if (i >= MAX_FILES) {
        errno = ENFILE;
        return -1;
    } else {
        fd1 = i;
        ftable[i].refcount++;
    }

    i = 3;
    while (ftable[i].refcount > 0) {
        i++;
    }
    if (i >= MAX_FILES) {
        ftable[fd1].refcount--;
        errno = ENFILE;
        return -1;
    } else {
        fd2 = i;
        ftable[i].refcount++;
    }

    /*
     * get new pipe reference 
     */
    p = malloc(sizeof(struct pdesc));
    if (p == NULL) {
        errno = ENFILE;
        return -1;
    } else {
        p->flags = 0;
    }

    /*
     * create the new memsection 
     */
    pipe_size = (uintptr_t)PIPE_REGION_SIZE;
    pipe_ref = memsection_create(pipe_size, &pipe_base);
    if (pipe_ref == 0) {
        errno = ENFILE;
        return -1;
    }

    p->pipe_size = 0;
    p->pipe_base = pipe_base;
    p->pipe_memsec = pipe_ref;
    p->refcount = 2;

    STAILQ_INIT(&(p->readq));
    STAILQ_INIT(&(p->writeq));

    p->mutex = malloc(sizeof(struct okl4_mutex));
    okl4_mutex_init(p->mutex);

    /*
     * update the file table entries 
     */
    f = &(ftable[fd1]);
    f->size = pipe_size;
    f->U.p.posw = 0;
    f->U.p.posr = 0;
    f->flags = O_RDONLY;
    f->ftype = SAS_PIPE;
    f->U.p.rp = p;
    f->U.p.wp = NULL;

    f = &(ftable[fd2]);
    f->size = pipe_size;
    f->U.p.posw = 0;
    f->U.p.posr = 0;
    f->flags = O_WRONLY;
    f->ftype = SAS_PIPE;
    f->U.p.rp = NULL;
    f->U.p.wp = p;

    /*
     * return file descriptors to the caller 
     */

    fildes[0] = fd1;
    fildes[1] = fd2;

    return 0;
}


