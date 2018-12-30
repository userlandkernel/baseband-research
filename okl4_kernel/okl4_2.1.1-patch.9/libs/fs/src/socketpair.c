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
#include <l4/kdebug.h>

#include <iguana/env.h>
#include <iguana/memsection.h>

memsection_ref_t intervm_ctl;
memsection_ref_t intervm_fwd;
memsection_ref_t intervm_bwd;

static int intervm_setup = 1;
static int mutex_init_done = 0;
static int my_dir = 0;

okl4_mutex_t intervm_ctl_mutex;
struct control_block *cb;

int reset_me = 0;

/* Lookup the memsections for the intervm channel in the Iguana environment.
 *
 * @return 0 on success or -1 on failure
 */
int
lookup_intervm_memsections(void)
{
    const envitem_t *ms;

    ms = iguana_getenv("INTERVM_CTL");
    if (ms == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (env_type(ms) != ENV_MEMSECTION) {
        errno = EINVAL;
        return -1;
    }
    intervm_ctl = env_memsection(ms);
    if (intervm_ctl == 0) {
        errno = EINVAL;
        return 0;
    }

    ms = iguana_getenv("INTERVM_FWD");
    if (ms == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (env_type(ms) != ENV_MEMSECTION) {
        errno = EINVAL;
        return -1;
    }
    intervm_fwd = env_memsection(ms);
    if (intervm_fwd == 0) {
        errno = EINVAL;
        return 0;
    }

    ms = iguana_getenv("INTERVM_BWD");
    if (ms == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (env_type(ms) != ENV_MEMSECTION) {
        errno = EINVAL;
        return -1;
    }
    intervm_bwd = env_memsection(ms);
    if (intervm_bwd == 0) {
        errno = EINVAL;
        return 0;
    }

    return 0;
}

int
okl4_socketpair(int domain, int type, int protocol, int socket_vector[2],
                 int sp_type)
{
    int fd1 = 0, fd2 = 0, i;
    memsection_ref_t pipe_ref1, pipe_ref2;
    uintptr_t pipe_size, pipe_base1, pipe_base2;
    struct fdesc *f;
    struct pdesc *p = NULL, *p2 = NULL;
    uintptr_t u;

    /*
     * return error if socket type or protocol is not supported 
     */
    if (type != SOCK_STREAM || protocol != 0) {
        errno = EINVAL;
        return -1;
    }

    /*
     * get new file descriptors 
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

    if (sp_type == 0) {
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
    }

    /*
     * new pipes 
     */
    if (sp_type == 1) {
        if (lookup_intervm_memsections() != 0) {
            return -1;
        }
        u = (uintptr_t)memsection_base(intervm_fwd);
        intervm_setup = *(int *)u;
        intervm_ctl_mutex = (okl4_mutex_t) memsection_base(intervm_ctl);
        if (!intervm_setup) {
            if (!mutex_init_done) {
                okl4_mutex_init(intervm_ctl_mutex);
                mutex_init_done = 1;
            }
            okl4_mutex_count_lock(intervm_ctl_mutex);
            u += sizeof(int);
            cb = (struct control_block *)u;
            cb->pipe1 = p = memsec_malloc(intervm_ctl, sizeof(struct pdesc));
            cb->pipe2 = p2 = memsec_malloc(intervm_ctl, sizeof(struct pdesc));
        } else {
            if (!mutex_init_done)
                my_dir = 1;
            mutex_init_done = 1;
            okl4_mutex_count_lock(intervm_ctl_mutex);
            u += sizeof(int);
            cb = (struct control_block *)u;
            if (my_dir) {
                p2 = cb->pipe1;
                p = cb->pipe2;
            } else {
                p = cb->pipe1;
                p2 = cb->pipe2;
            }
        }
    } else {
        p = malloc(sizeof(struct pdesc));
    }

    if (p == NULL) {
        errno = ENFILE;
        return -1;
    } else {
        p->flags = 0;
    }

    if (sp_type == 0) {
        p2 = malloc(sizeof(struct pdesc));
        if (p2 == NULL) {
            free(p);
            errno = ENFILE;
            return -1;
        }
    }
    p2->flags = 0;

    /*
     * create the new memory sections 
     */
    if (sp_type == 0) {
        pipe_size = (uintptr_t)PIPE_REGION_SIZE;
        pipe_ref1 = memsection_create(pipe_size, &pipe_base1);
        if (pipe_ref1 == 0) {
            errno = ENFILE;
            return -1;
        }
        pipe_ref2 = memsection_create(pipe_size, &pipe_base2);
        if (pipe_ref2 == 0) {
            errno = ENFILE;
            return -1;
        }
    } else {
        pipe_ref1 = intervm_fwd;
        pipe_ref2 = intervm_bwd;
        pipe_size = memsection_size(intervm_fwd) - sizeof(int) - sizeof(struct control_block);
    }

    if (sp_type == 0) {
        p->pipe_size = 0;
        p->pipe_base = pipe_base1;
        p->pipe_memsec = pipe_ref1;
        p->mutex = malloc(sizeof(struct okl4_mutex));
        okl4_mutex_init(p->mutex);
        p->refcount = 2;
        STAILQ_INIT(&(p->readq));
        STAILQ_INIT(&(p->writeq));
    } else {
        if (!intervm_setup) {
            p->pipe_base = (uintptr_t)memsection_base(pipe_ref1) + sizeof(int) + sizeof(struct control_block);
            p->pipe_memsec = pipe_ref1;
            p->mutex = memsec_malloc(intervm_ctl, sizeof(struct okl4_mutex));
            okl4_mutex_init(p->mutex);
            p->pipe_size = 0;
            STAILQ_INIT(&(p->readq));
            STAILQ_INIT(&(p->writeq));
        }
        p->refcount++;
    }

    if (sp_type == 0) {
        p2->pipe_size = 0;
        p2->pipe_base = pipe_base2;
        p2->pipe_memsec = pipe_ref2;
        p2->refcount = 2;
        STAILQ_INIT(&(p2->readq));
        STAILQ_INIT(&(p2->writeq));
        p2->mutex = malloc(sizeof(struct okl4_mutex));
        okl4_mutex_init(p2->mutex);
    } else if (sp_type == 1 && !intervm_setup) {
        p2->pipe_base = (uintptr_t)memsection_base(pipe_ref2) + sizeof(int) + sizeof(struct control_block);
        p2->pipe_memsec = pipe_ref2;
        p2->mutex = memsec_malloc(intervm_ctl, sizeof(struct okl4_mutex));
        okl4_mutex_init(p2->mutex);
        p2->pipe_size = 0;
        p2->refcount++;
        STAILQ_INIT(&(p2->readq));
        STAILQ_INIT(&(p2->writeq));
    } else {
        p2->refcount++;
    }

    /*
     * update the file table entries 
     */
    f = &(ftable[fd1]);
    if (sp_type == 0) {
        f->size = PIPE_REGION_SIZE;
        f->ftype = SAS_PIPE;
    } else {
        f->size = memsection_size(pipe_ref1) - sizeof(int) - sizeof(struct control_block);
        f->ftype = INTER_VM;
    }
    if (sp_type == 0 || (reset_me || !intervm_setup)) {
        f->U.p.posw = 0;            /* write position */
        f->U.p.posr = 0;            /* read position */
        reset_me = 0;
    }
    f->flags = O_RDWR;
    f->U.p.rp = p;
    f->U.p.wp = p2;

    if (sp_type == 0) {
        f = &(ftable[fd2]);
        f->size = PIPE_REGION_SIZE;
        f->U.p.posw = 0;        /* write position */
        f->U.p.posr = 0;        /* read position */
        f->flags = O_RDWR;
        f->ftype = SAS_PIPE;
        f->U.p.rp = p2;
        f->U.p.wp = p;
    }

    /*
     * return file descriptors to the caller 
     */
    socket_vector[0] = fd1;
    if (sp_type == 0) {
        socket_vector[1] = fd2;
    } else {
        socket_vector[1] = fd1;
        (*(int *)memsection_base(intervm_fwd))++;
    }

    if (sp_type == 1)
        okl4_mutex_count_unlock(intervm_ctl_mutex);


    return 0;
}


