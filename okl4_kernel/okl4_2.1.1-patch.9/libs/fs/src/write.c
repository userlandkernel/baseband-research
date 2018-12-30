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
#include <errno.h>
#include <string.h>

#include <fs/fs.h>

#include <iguana/memsection.h>
#include <iguana/thread.h>
#include <l4/utcb.h>
#include <iguana/tls.h>
#include <l4/kdebug.h>

static ssize_t
pipe_write(int fildes, const void *buf, size_t nbyte)
{
    struct fdesc *f;
    uintptr_t free_space, wrap_around;
    size_t old_size;

    ssize_t bytes_written;
    char *dst;
    struct blocked_thread *bt = NULL;

    f = &(ftable[fildes]);
    if (!(f->flags & O_WRONLY || f->flags & O_RDWR)) {
        errno = EACCES;
        return -1;
    }

    if (f->refcount == 0) {
        errno = EBADF;
        return -1;
    }

    if (f->U.p.wp == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (f->ftype == SAS_PIPE)
        bt = malloc(sizeof(struct blocked_thread));
    else {
        bt = memsec_malloc(intervm_ctl, sizeof(struct blocked_thread));
    }

    /*
     * lock the pipe while writing 
     */
  tryagain:
    okl4_mutex_lock(f->U.p.wp->mutex);
     if ((f->U.p.wp->flags & P_WIDOWED)) { // == P_WIDOWED) {
        if (f->ftype == INTER_VM) {
            memsec_free(intervm_ctl, bt);
        } else {
            free(bt);
        }
        /*
         * FIX ME: should send SIGPIPE to itself 
         */
        okl4_mutex_unlock(f->U.p.wp->mutex);
        errno = EPIPE;
        return -1;
    }
   if (f->U.p.wp->pipe_size == f->size) {
        if ((f->flags & O_NONBLOCK)) { // == O_NONBLOCK) {
            okl4_mutex_unlock(f->U.p.wp->mutex);
            errno = EAGAIN;
            if (f->ftype == INTER_VM) {
                memsec_free(intervm_ctl, bt);
            } else {
                free(bt);
            }
            return -1;
        }
        bt->tid.raw = ((L4_Word_t *)__L4_TCR_ThreadLocalStorage())[TLS_THREAD_ID];
        STAILQ_INSERT_HEAD(&(f->U.p.wp->writeq), bt, links);
        okl4_mutex_unlock(f->U.p.wp->mutex);
        block_myself();
        goto tryagain;
    }
    old_size = f->U.p.wp->pipe_size;
    dst = (char *)(f->U.p.wp->pipe_base + f->U.p.posw);
    free_space = f->size - f->U.p.wp->pipe_size;
    wrap_around = f->size - f->U.p.posw;
    if (nbyte <= free_space) {
        bytes_written = nbyte;
    } else {
        bytes_written = free_space;
    }
    if (bytes_written <= wrap_around) {
        memcpy(dst, buf, bytes_written);
    } else {
        memcpy(dst, buf, wrap_around);
        memcpy((char *)(f->U.p.wp->pipe_base), (char *)buf + wrap_around,
               bytes_written - wrap_around);
    }

    f->U.p.wp->pipe_size += bytes_written;
    f->U.p.posw += bytes_written;
    if (f->U.p.posw >= f->size) {
        f->U.p.posw -= f->size;
    }

    /*
     * see if there is a thread waiting for something to be written to the pipe
     * that can be unblocked 
     */
    if (f->ftype == SAS_PIPE)
        free(bt);
    else {
        memsec_free(intervm_ctl, bt);
    }
    if (f->U.p.wp->pipe_size > 0 && !STAILQ_EMPTY(&(f->U.p.wp->readq))) {
        while (!STAILQ_EMPTY(&f->U.p.wp->readq)) {
            bt = STAILQ_FIRST(&(f->U.p.wp->readq));
            STAILQ_REMOVE(&(f->U.p.wp->readq), bt, blocked_thread, links);
            unblock_thread(bt->tid);
        }
    }
    if (f->ftype == INTER_VM && old_size == 0) {
        L4_Notify(cb->linux_thread, cb->mask);
    }

    okl4_mutex_unlock(f->U.p.wp->mutex);
    return bytes_written;
}

ssize_t
okl4_write(int fildes, const void *buf, size_t nbyte)
{
    if (fildes < 0 || fildes >= MAX_FILES) {
        errno = EINVAL;
        return -1;
    }
    if (buf == NULL) {
        errno = EFAULT;
        return -1;
    }

    switch (ftable[fildes].ftype) {
    case REGULAR:
        errno = EACCES;
        return -1;
        break;
    case SAS_PIPE:
        return pipe_write(fildes, buf, nbyte);
        break;
    case SERIAL:
#ifdef __USE_POSIX
        return fwrite(buf, 1, nbyte, ftable[fildes].U.s.serial);
#endif
        break;
    case INTER_VM:
        return pipe_write(fildes, buf, nbyte);
        break;
    default:
        break;
    }

    return 0;
}


