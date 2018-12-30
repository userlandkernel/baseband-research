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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __USE_POSIX
extern int serial_fopen_init(FILE *handle);
#endif

static size_t
regular_read(void *data, long int position, size_t count, void *handle)
{
    return read((uintptr_t)handle, (char *)data, count);
}

static size_t
regular_write(const void *data, long int position, size_t count, void *handle)
{
    return write((uintptr_t)handle, (const char *)data, count);
}

static int
regular_close(void *handle)
{
    return close((uintptr_t)handle);
}

static long int
regular_eof(void *handle)
{
    struct stat buf;

    fstat((uintptr_t)handle, &buf);

    return buf.st_size; 
}

FILE *
fopen(const char *fname, const char *prot)
{
    uintptr_t fd;
    FILE *fp;

    if (strcmp(fname, "serial") == 0) {
#ifdef __USE_POSIX
        fp = malloc(sizeof(FILE));
        if (fp == NULL) {
            return NULL;
        }
        fp->handle = NULL;
        fp->eof_fn = NULL;
        fp->buffering_mode = _IONBF;
        fp->buffer = NULL;
        fp->unget_pos = 0;
        fp->current_pos = 0;
        fp->error = 0;
        fp->eof = 0;
        serial_fopen_init(fp);
        return fp;
#else
       return NULL;
#endif  /* __USE_POSIX */
    }

    /* regular files are read-only */
    if (strcmp(prot, "r") != 0 && strcmp(prot, "rb") != 0) {
        errno = EROFS;
        return NULL;
    }
    
    fd = open(fname, O_RDONLY);
    if (fd == -1) {
        return NULL;
    }

    fp = malloc(sizeof(FILE));
    if (fp == NULL) {
        return NULL;
    }
    fp->handle = (void *)fd;
    fp->read_fn = regular_read;
    fp->write_fn = regular_write;
    fp->close_fn = regular_close;
    fp->eof_fn = regular_eof;
    fp->unget_pos = 0;
    fp->current_pos = 0;

    setvbuf(fp, NULL, _IOFBF, BUFSIZ);

#ifdef THREAD_SAFE
    okl4_mutex_init(&(fp->mutex));
#endif

    fp->eof = 0;
    fp->error = 0;

    return fp;
}

FILE *
fdopen(int fildes, const char *mode)
{
    FILE *fp = NULL;
    struct stat buf;
    unsigned int m;

    fstat(fildes, &buf);
    m = ((buf.st_mode) >> 7) & O_ACCMODE;
    if ((strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0) && ((m & O_RDONLY) != O_RDONLY)) {
        errno = EINVAL;
        return NULL;
    }
    if ((strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0) && ((m & O_WRONLY) != O_WRONLY)) {
        errno = EINVAL;
        return NULL;
    }

    fp = malloc(sizeof(FILE));
    if (fp == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    fp->handle = (void *)(uintptr_t)fildes;
    if ((m & S_IFCHR) == S_IFCHR) {
#ifdef __USE_POSIX
        serial_fopen_init(fp);
#endif
        fp->buffering_mode = _IONBF;
        fp->unget_pos = 0;
        fp->current_pos = 0;
#ifdef THREAD_SAFE
        okl4_mutex_init(&(fp->mutex));
#endif
        fp->eof = 0;
        fp->error = 0;
    } else {
        fp->read_fn = regular_read;
        fp->write_fn = regular_write;
        fp->close_fn = regular_close;
        fp->eof_fn = regular_eof;
        fp->unget_pos = 0;
        fp->current_pos = 0;

        setvbuf(fp, NULL, _IOFBF, BUFSIZ);

#ifdef THREAD_SAFE
        okl4_mutex_init(&(fp->mutex));
#endif

        fp->eof = 0;
        fp->error = 0;
    }

    return fp;
}

