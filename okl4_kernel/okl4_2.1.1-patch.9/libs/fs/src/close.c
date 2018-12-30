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

#include <l4/types.h>

#include <iguana/memsection.h>

#include <fs/fs.h>
#include <queue/stailq.h>
#include <l4/kdebug.h>

extern int reset_me;

int
okl4_close(int fildes)
{
    struct fdesc *f;
    struct blocked_thread *bt;
    uintptr_t tmp;
            
    if (fildes < 0 || fildes >= MAX_FILES) {
        errno = EBADF;
        return -1;
    }
    
    f = &(ftable[fildes]);
    if (f->refcount == 0) {
        errno = EBADF;
        return -1;
    }

    f->refcount--;
    if (f->ftype == SERIAL) {
        fclose(f->U.s.serial);
        return 0;
    }

    if (f->refcount == 0 && (f->ftype == SAS_PIPE || f->ftype == INTER_VM)) {
        if (f->ftype == INTER_VM) {
            okl4_mutex_count_lock(intervm_ctl_mutex);
        }
        if (f->U.p.rp != NULL) {
            okl4_mutex_lock(f->U.p.rp->mutex);
            f->U.p.rp->refcount--;
            f->U.p.rp->flags |= P_WIDOWED;
            while (!STAILQ_EMPTY(&f->U.p.rp->writeq)) {
                bt = STAILQ_FIRST(&(f->U.p.rp->writeq));
                STAILQ_REMOVE(&(f->U.p.rp->writeq), bt, blocked_thread,
                              links);
                unblock_thread(bt->tid);
            }
            while (!STAILQ_EMPTY(&f->U.p.rp->readq)) {
                bt = STAILQ_FIRST(&(f->U.p.rp->readq));
                STAILQ_REMOVE(&(f->U.p.rp->readq), bt, blocked_thread,
                              links);
                unblock_thread(bt->tid);
            }
            if (f->U.p.rp->refcount == 0) {
                if (f->ftype == SAS_PIPE) {
                    memsection_delete(f->U.p.rp->pipe_memsec);
                    okl4_mutex_unlock(f->U.p.rp->mutex);
                    free(f->U.p.rp->mutex);
                    free(f->U.p.rp);
                } else {
                    okl4_mutex_unlock(f->U.p.rp->mutex);
                    memsec_free(intervm_ctl, f->U.p.rp->mutex);
                    memsec_free(intervm_ctl, f->U.p.rp);
                    reset_me = 1;
                }
            } else {
                okl4_mutex_unlock(f->U.p.rp->mutex);
            }
        }
        if (f->U.p.wp != NULL) {
            okl4_mutex_lock(f->U.p.wp->mutex);
            f->U.p.wp->refcount--;
            f->U.p.wp->flags |= P_WIDOWED;
            while (!STAILQ_EMPTY(&f->U.p.wp->writeq)) {
                bt = STAILQ_FIRST(&(f->U.p.wp->writeq));
                STAILQ_REMOVE(&(f->U.p.wp->writeq), bt, blocked_thread,
                              links);
                unblock_thread(bt->tid);
            }
            while (!STAILQ_EMPTY(&f->U.p.wp->readq)) {
                bt = STAILQ_FIRST(&(f->U.p.wp->readq));
                STAILQ_REMOVE(&(f->U.p.wp->readq), bt, blocked_thread,
                              links);
                unblock_thread(bt->tid);
            }
            if (f->U.p.wp->refcount == 0) {
                if (f->ftype == SAS_PIPE) {
                    memsection_delete(f->U.p.wp->pipe_memsec);
                    okl4_mutex_unlock(f->U.p.wp->mutex);
                    free(f->U.p.wp->mutex);
                    free(f->U.p.wp);
                } else {
                    okl4_mutex_unlock(f->U.p.wp->mutex);
                    memsec_free(intervm_ctl, f->U.p.wp->mutex);
                    memsec_free(intervm_ctl, f->U.p.wp);
                    reset_me = 1;
                }
            } else {
                okl4_mutex_unlock(f->U.p.wp->mutex);
            }
        }
        if (f->ftype == INTER_VM) {
            tmp = (uintptr_t)memsection_base(intervm_fwd);
            (*(int *)tmp)--;
            L4_Notify(cb->linux_thread, cb->mask);
            okl4_mutex_count_unlock(intervm_ctl_mutex);
        }
    }

    return 0;
}


