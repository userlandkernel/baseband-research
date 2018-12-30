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
pipe_read(int fildes, void *buf, size_t nbyte)
{
    struct fdesc *f;
    uintptr_t wrap_around;
    size_t old_size;

    ssize_t bytes_read = 0;
    char *src;
    struct blocked_thread *bt;

    f = &(ftable[fildes]);
    if ((f->flags & O_ACCMODE) == O_WRONLY) {
        errno = EACCES;
        return -1;
    }

    if (f->refcount == 0) {
        errno = EBADF;
        return -1;
    }

    if (f->U.p.rp == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (f->ftype == INTER_VM) {
        bt = memsec_malloc(intervm_ctl, sizeof(struct blocked_thread));
    } else {
        bt = malloc(sizeof(struct blocked_thread));
    }

    /*
     * lock the pipe while reading 
     */
  tryagain:
    okl4_mutex_lock(f->U.p.rp->mutex);
    
    if (f->U.p.rp->pipe_size == 0) {
        if ((f->U.p.rp->flags & P_WIDOWED)) { 
            okl4_mutex_unlock(f->U.p.rp->mutex);
            if (f->ftype == INTER_VM) {
                memsec_free(intervm_ctl, bt);
            } else {
                free(bt);
            }
            return 0;
        }
        if ((f->flags & O_NONBLOCK)) {
            okl4_mutex_unlock(f->U.p.rp->mutex);
            errno = EAGAIN;
            if (f->ftype == INTER_VM) {
                memsec_free(intervm_ctl, bt);
            } else {
                free(bt);
            }
            return -EAGAIN;
        }
        bt->tid.raw = ((L4_Word_t *)__L4_TCR_ThreadLocalStorage())[TLS_THREAD_ID];
        STAILQ_INSERT_HEAD(&(f->U.p.rp->readq), bt, links);
        okl4_mutex_unlock(f->U.p.rp->mutex);
        block_myself();
        goto tryagain;
    }
    old_size = f->U.p.rp->pipe_size;
    src = (char *)(f->U.p.rp->pipe_base + f->U.p.posr);
    wrap_around = f->size - f->U.p.posr;
    if (nbyte <= f->U.p.rp->pipe_size) {
        bytes_read = nbyte;
    } else {
        bytes_read = f->U.p.rp->pipe_size;
    }
    if (bytes_read <= wrap_around) {
        memcpy(buf, src, bytes_read);
    } else {
        memcpy(buf, src, wrap_around);
        memcpy((char *)buf + wrap_around, (char *)(f->U.p.rp->pipe_base),
               bytes_read - wrap_around);
    }
    f->U.p.rp->pipe_size -= bytes_read;
    f->U.p.posr += bytes_read;
    if (f->U.p.posr >= f->size) {
        f->U.p.posr -= f->size;
    }

    /*
     * see if there are any threads that were blocked on a full pipe that can
     * be woken up 
     */
    if (f->ftype == INTER_VM) {
        memsec_free(intervm_ctl, bt);
    } else {
        free(bt);
    }
    if (f->U.p.rp->pipe_size < PIPE_REGION_SIZE &&
        !STAILQ_EMPTY(&(f->U.p.rp->writeq))) {
        while (!STAILQ_EMPTY(&f->U.p.rp->writeq)) {
            bt = STAILQ_FIRST(&(f->U.p.rp->writeq));
            STAILQ_REMOVE(&(f->U.p.rp->writeq), bt, blocked_thread, links);
            unblock_thread(bt->tid);
        }
    }

    if (f->ftype == INTER_VM && old_size == 0) {
        L4_Notify(cb->linux_thread, cb->mask);
    }

    okl4_mutex_unlock(f->U.p.rp->mutex);

    return bytes_read;
}

static ssize_t
regular_read(int fildes, char *buf, size_t nbyte)
{
    struct fdesc *f = &(ftable[fildes]);
    int bytes_read;

    if ((f->size - f->U.r.offset) >= nbyte) {
        bytes_read = nbyte;
    } else {
        bytes_read = f->size - f->U.r.offset;
    }
    memcpy(buf, (void *)(f->U.r.base + f->U.r.offset), bytes_read);

    f->U.r.offset += bytes_read;
    return bytes_read;
}

ssize_t
okl4_read(int fildes, void *buf, size_t nbyte)
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
        return regular_read(fildes, buf, nbyte);
        break;
    case SAS_PIPE:
        return pipe_read(fildes, buf, nbyte);
        break;
    case SERIAL:
#ifdef __USE_POSIX
        return fread(buf, 1, nbyte, ftable[fildes].U.s.serial);
#endif
        break;
    case INTER_VM:
        return pipe_read(fildes, buf, nbyte);
        break;
    default:
        break;
    }

    return 0;
}


