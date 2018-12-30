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

#ifndef __FS_H__
#define __FS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <iguana/types.h>
#include <mutex/mutex.h>
#include <queue/stailq.h>

#include "config.h"

#ifndef ESPIPE
#define ESPIPE 19
#endif

#ifndef EPIPE
#define EPIPE 32
#endif

extern struct fdesc ftable[MAX_FILES];

extern struct control_block *cb;

extern memsection_ref_t intervm_ctl;
extern memsection_ref_t intervm_fwd;
extern memsection_ref_t intervm_bwd;

extern okl4_mutex_t intervm_ctl_mutex;
extern L4_ThreadId_t linux_thread;

/* structure used to represent a file descriptor */
struct fdesc {
    uint8_t ftype;
    size_t size;        /* size of the file (in bytes) */
    uint32_t flags;      
    uint8_t refcount;
    union {
        struct {
            uintptr_t posw;     /* current write position in the file */
            uintptr_t posr;     /* current read position in the file */
            struct pdesc *rp;   /* pipe to read from */
            struct pdesc *wp;   /* pipe to write from */
        } p;
        struct {
            off_t offset;   /* offset of regular file in static fs */
            uintptr_t base;     /* base address of regular file */
        } r;
        struct {
            FILE *serial;
        } s;
        struct {
            memsection_ref_t ms;
        } vm;
    } U;
};

/* queue node */
typedef struct blocked_thread {
    L4_ThreadId_t tid;
    STAILQ_ENTRY(blocked_thread) links;
} blocked_thread;

struct pdesc {
    size_t pipe_size;    /* amount of unread data in the pipe */
    uintptr_t pipe_base;    /* base virtual address of the pipe */
    memsection_ref_t pipe_memsec;
    uint8_t refcount;
    uint8_t flags;
    STAILQ_HEAD(rqhead, blocked_thread) readq;  /* blocked on empty pipe */
    STAILQ_HEAD(wqhead, blocked_thread) writeq; /* blocked on full pipe */
    okl4_mutex_t mutex;
};

struct control_block {
    struct pdesc *pipe1;
    struct pdesc *pipe2;
    L4_ThreadId_t linux_thread;
    int mask;
};

/* file types */
#define REGULAR     0
#define SAS_PIPE    1
#define SERIAL      2
#define INTER_VM    3

/* pipe flags */
#define P_WIDOWED   0x1

/* Actual implementations of the POSIX functions defined in the standard header
 * files
 */
int okl4_open(const char *path, int oflag, ... );
int okl4_pipe(int fildes[2]);
int okl4_poll(struct pollfd *fds, nfds_t nfds, int timeout);
ssize_t okl4_read(int fildes, void *buf, size_t nbyte);
ssize_t okl4_write(int fildes, const void *buf, size_t nbyte);
int okl4_close(int fildes);
off_t okl4_lseek(int fildes, off_t offset, int whence);
int okl4_fsync(int fildes);
int okl4_fcntl(int fildes, int cmd, va_list vl);
int okl4_mknod(const char *path, mode_t mode, dev_t dev);

/* Takes the same parameters as POSIX socketpair plus the additional sp_type
 * parameter, which is set to 0 for a standard socketpair and 1 for the cross-VM
 * channel
 */
int okl4_socketpair(int domain, int type, int protocol, int socket_vector[2], int sp_type);

/* Allocate memory out of a specific memsection. Probably very naive and
 * inefficient, but it works. Assumes the entire memsection is initially unused
 * and can be allocated.
 *
 * @param ms            memsection to allocate memory from
 * @param count         number of bytes to allocate
 *
 * @return a pointer to the base address of the allocated chunk
 */
void *memsec_malloc(memsection_ref_t ms, uintptr_t count);

/* Free memory that has previously been allocated by memsec_malloc
 *
 * @param ms            memsection that memory belongs to
 * @param ptr           base address of chunk to free
 *
 */
void memsec_free(memsection_ref_t ms, void *ptr);

/* Blocks the currently executing thread until it receives an asynchronous
 * notification
 */
void block_myself(void);                

/* Sends an asynchronous notification to wake up a thread
 *
 * @param tid           thread id of the thread to unblock
 *
 */
void unblock_thread(L4_ThreadId_t tid); 

int lookup_intervm_memsections(void);

#endif /* __FS_H__ */

