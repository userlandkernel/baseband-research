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

#include <rt_sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <l4/kdebug.h>

#pragma import(__use_no_semihosting_swi)

#define OPEN_MAX                20

enum {
    TYPE_INVALID = 0, TYPE_IO, TYPE_TMPFILE
};

const char __stdin_name[] = "0";
const char __stdout_name[] = "1";
const char __stderr_name[] = "2";

struct {
    int type;
    int mode;
    long pos;
    unsigned char *start;
    long size;
} file[OPEN_MAX] = {{TYPE_IO, 0, 0}, {TYPE_IO, 0, 0}, {TYPE_IO, 0, 0}};

static int
get_fh(FILEHANDLE * fh)
{
    for (*fh = 0; *fh < OPEN_MAX; (*fh)++)
        if (file[*fh].type == TYPE_INVALID)
            return 0;
    return -1;
}

FILEHANDLE
_sys_open(const char *name, int openmode)
{
    FILEHANDLE fh;

    /* Special cases for std streams.  */
    if (strcmp(name, __stdin_name) == 0) {
        return 0;
    } else if (strcmp(name, __stdout_name) == 0) {
        return 1;
    } else if (strcmp(name, __stderr_name) == 0) {
        return 2;
    }

    /* XXX: We only handle tmpfiles. */
    if (strncmp(name, "tmp", 3) == 0) {
        if (get_fh(&fh) != 0) {
            return -1;          /* EMFILE */
        }
        file[fh].type = TYPE_TMPFILE;
        file[fh].mode = openmode;
        file[fh].pos = 0;
        file[fh].start = NULL;
        file[fh].size = 0;

        return fh;
    }
    L4_KDB_Enter("_sys_open not implemented for real files");
    return -1;
}

int
_sys_close(FILEHANDLE fh)
{
    if (fh < 0 || OPEN_MAX <= fh || file[fh].type == TYPE_INVALID) {
        return -1;              /* EBADF */
    }

    if (file[fh].type == TYPE_TMPFILE) {
        free(file[fh].start);
        file[fh].type = TYPE_INVALID;
    }
    return 0;
}

int
_sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
    if (fh < 0 || OPEN_MAX <= fh || file[fh].type == TYPE_INVALID) {
        return -1;              /* EBADF */
    }

    if (file[fh].type == TYPE_IO) {
        while (len--) {
            int ch;

            ch = L4_KDB_PrintChar(*buf++);
            if (ch == -1) {
                return -1;
            }
        }
        return 0;
    } else if (file[fh].type == TYPE_TMPFILE) {
        if (file[fh].pos + len > file[fh].size) {
            file[fh].start = realloc(file[fh].start, file[fh].pos + len);
            file[fh].size = file[fh].pos + len;
        }
        (void)memcpy(file[fh].start + file[fh].pos, buf, len);
        file[fh].pos += len;
    }
    return 0;
}

int
_sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode)
{
    int ret = 0;

    if (fh < 0 || OPEN_MAX <= fh || file[fh].type == TYPE_INVALID) {
        return -1;              /* EBADF */
    }

    if (file[fh].type == TYPE_IO) {
        while (len--) {
            char ch;

            L4_KDB_ReadChar(&ch);

            if (ch == 0) {
                return len;
            } else if (ch == -1) {
                return 0x80000000 | len;
            } else {
                *buf++ = ch;
            }
        }
        return 0;
    } else if (file[fh].type == TYPE_TMPFILE) {
        if (file[fh].pos >= file[fh].size) {
            return 0x80000000 | len;
        }
        if (file[fh].pos + len > file[fh].size) {
            ret = len - file[fh].size;
        }
        (void)memcpy(buf, file[fh].start + file[fh].pos, len - ret);
        file[fh].pos += len - ret;
        return ret;
    }
    return 0;
}

int
_sys_istty(FILEHANDLE fh)
{
    return 0 <= fh && fh <= 2;
}

int
_sys_seek(FILEHANDLE fh, long pos)
{
    if (fh < 0 || OPEN_MAX <= fh || file[fh].type == TYPE_INVALID) {
        return -1;              /* EBADF */
    }

    if (file[fh].type == TYPE_IO) {
        return -1;
    } else {
        file[fh].pos = pos;
        if (file[fh].pos >= file[fh].size) {
            file[fh].start = realloc(file[fh].start, file[fh].pos);
            memset(file[fh].start + file[fh].size, '\0',
                   file[fh].pos - file[fh].size);
            file[fh].size = file[fh].pos;
        }
    }
    return 0;
}

int
_sys_ensure(FILEHANDLE fh)
{
    return 0;
}

long
_sys_flen(FILEHANDLE fh)
{
    if (fh <= 2 || OPEN_MAX <= fh || file[fh].type == TYPE_INVALID) {
        return -1;      /* EBADF */
    }
    if (file[fh].type == TYPE_TMPFILE) {
        return file[fh].size;
    }
    return -1;
}

int
_sys_tmpnam(char *name, int sig, unsigned maxlen)
{
    (void)snprintf(name, maxlen, "tmp%04d", sig);
    return 1;
}
