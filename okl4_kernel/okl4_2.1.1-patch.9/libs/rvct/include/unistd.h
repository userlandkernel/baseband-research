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

#ifndef _UNISTD_H
#define _UNISTD_H

/*
  PSE-51 Minimal Realtime compliance symbol
  As specified in POSIX 1003.13, Section 6.1.2
  This is defined by the build system for any projects
  built in the pse51_env environment
// #define _POSIX_AEP_REALTIME_MINIMAL 200312L
*/

#ifndef THREAD_SAFE
extern char **environ;
#else
#include <threadstate.h>
#define environ (*(thread_state_get_environ()))
#endif

#include <sys/types.h>

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#ifndef _OFF_T
#define _OFF_T
typedef long off_t;
#endif


/* Creates a channel for interprocess communication using Unix pipe semantics.
 *
 * @param fildes            buffer in which the new file descriptors are
 *                          returned
 *
 * @return 0 on success, or -1 and set errno on failure
 */
int pipe(int fildes[2]);

/* Reads from a file descriptor.
 *
 * @param fildes        file descriptor to read from
 * @param buf           buffer to read data into
 * @param nbyte         number of bytes to read
 *
 * @return the number of bytes actually read, or -1 and set errno if an error
 * occurs. A return value of 0 signifies end-of-file
 */
ssize_t read(int fildes, void *buf, size_t nbyte);

/* Writes to a file descriptor. Currently only works with pipes, as the file
 * system is static.
 *
 * @param fildes        file descriptor to write to
 * @param buf           buffer where data to be written is stored
 * @param nbyte         number of bytes to write
 *
 * @return the number of bytes actually written, or -1 and set errno if an error
 * occurs.
 */
ssize_t write(int fildes, const void *buf, size_t nbyte);

/* Closes a file descriptor
 *
 * @param fildes        file descriptor to close
 *
 * @return 0 on success, or -1 and set errno on failure
 */
int close(int fildes);

/* Move the read/write file offset
 *
 * @param fildes        file descriptor to operate on
 * @param offset        amount to modify the offset by
 * @param whence        seek mode
 *
 * @return the new offset from the beginning of the file, or -1 and set errno on
 * error
 */
off_t lseek(int fildes, off_t offset, int whence);

/* Write changes to the backing store. Since we don't have a backing store this
 * doesn't do anything.
 *
 * @param fildes        file descriptor to operate on
 *
 * @return 0 if successful, or -1 and set errno on error
 */
int fsync(int fildes);
#endif /* _UNISTD_H */
