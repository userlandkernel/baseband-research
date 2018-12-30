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
 * Author: Adam Clench      <adamc@ok-labs.com>
 *
 */

#ifndef _FCNTL_H
#define _FCNTL_H

#include <stdarg.h>
#include <sys/types.h>

/* commands that can be performed on file descriptors */
#define F_DUPFD     0
#define F_GETFD     1
#define F_SETFD     2
#define F_GETFL     3
#define F_SETFL     4
#define F_GETLK     5
#define F_SETLK     6
#define F_SETLKW    7
#define F_GETOWN    8
#define F_SETOWN    9

/* various flags are defined here */
#define O_ACCMODE   00000003
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#ifndef O_CREAT
#define O_CREAT     00000100    /* not fcntl */
#endif
#ifndef O_EXCL
#define O_EXCL      00000200    /* not fcntl */
#endif
#ifndef O_NOCTTY
#define O_NOCTTY    00000400    /* not fcntl */
#endif
#ifndef O_TRUNC
#define O_TRUNC     00001000    /* not fcntl */
#endif
#ifndef O_APPEND
#define O_APPEND    00002000
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK  00004000
#endif
#ifndef O_SYNC
#define O_SYNC      00010000
#endif
#ifndef FASYNC
#define FASYNC      00020000    /* fcntl, for BSD compatibility */
#endif
#ifndef O_DIRECT
#define O_DIRECT    00040000    /* direct disk access hint */
#endif
#ifndef O_LARGEFILE
#define O_LARGEFILE 00100000
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 00200000    /* must be a directory */
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW  00400000    /* don't follow links */
#endif
#ifndef O_NOATIME
#define O_NOATIME   01000000
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC   02000000    /* set close_on_exec */
#endif
#ifndef O_NDELAY
#define O_NDELAY    O_NONBLOCK
#endif

#define FD_CLOEXEC      0x4
#define F_RDLCK         0x8
#define F_UNLCK         0x10
#define F_WRLCK         0x20
/*
#define O_CREAT         0x40
#define O_EXCL          0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_DSYNC         0x800
#define O_NONBLOCK  00004000
#define O_RSYNC         0x2000
#define O_SYNC          0x4000
#define O_ACCMODE       0x3
#define O_RDONLY        0x1
#define O_WRONLY        0x2
#define O_RDWR          (O_RDONLY | O_WRONLY)
*/
/* File control - modify file descriptors
 *
 * @param fildes        file descriptor to operate on
 * @param cmd           the command to execute. Currently only implements
 *                      F_GETFL and F_SETFL
 *
 * @return 0 for F_SETFL or the flags for F_GETFL, or -1 and set errno on error
 */
int fcntl(int fildes, int cmd, ...);

/* Open a file and associate a file descriptor with it
 *
 * @param path          path name of the file to open
 * @param oflag         file access and mode flags
 *
 * @return a new file descriptor, or -1 and set errno on error
 */
int open(const char *path, int oflag, ...);

/* Create a new file or rewrite an existing one.
 *
 * @param path          path name of the file to open
 * @param mode          specifies the permissions to use
 *
 * @return a new file descriptor, or -1 and set errno on error
 */
int creat(const char *path, mode_t mode);

#endif /* _FCNTL_H */
