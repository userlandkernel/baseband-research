/*
 * Copyright (c) 2006, National ICT Australia
 */
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *sys_tmpfile(void);

struct tmp {
    long int total_size;
    char *buffer;
};

static long int
tmp_eof(void *handle)
{
    struct tmp *tmp = handle;

    return tmp->total_size;
}

static size_t
tmp_write(const void *data, long int position, size_t count, void *handle)
{
    void *tmp_buf;
    struct tmp *tmp = handle;

    if (position + count > tmp->total_size) {
        tmp->total_size = position + count;
        tmp_buf = realloc(tmp->buffer, tmp->total_size);
        assert(tmp_buf != NULL);
        if (tmp_buf == NULL) {
            return 0;
        }
        tmp->buffer = tmp_buf;
    }
    (void)memcpy(&tmp->buffer[position], data, count);
    return count;
}

static size_t
tmp_read(void *data, long int position, size_t count, void *handle)
{
    struct tmp *tmp = handle;

    if (position + count > tmp->total_size) {
        return 0;
    }
    (void)memcpy(data, &tmp->buffer[position], count);
    return count;
}

static int
tmp_close(void *handle)
{
    struct tmp *tmp = handle;

    free(tmp->buffer);
    free(handle);
    return 0;
}

FILE *
sys_tmpfile(void)
{
    /*
     * Note: We may want to allocate froma different pool of memory to minimise 
     * this being tramped on a by a user 
     */
    struct tmp *tmp;
    FILE *tmpf;
    tmpf = malloc(sizeof(FILE));
    if (tmpf == NULL)
        return NULL;
    tmp = malloc(sizeof(struct tmp));
    if (tmp == NULL) {
        free(tmpf);
        return NULL;
    }

    tmp->total_size = 0;
    tmp->buffer = NULL;

    tmpf->handle = tmp;
    tmpf->write_fn = tmp_write;
    tmpf->read_fn = tmp_read;
    tmpf->close_fn = tmp_close;
    tmpf->eof_fn = tmp_eof;
    tmpf->current_pos = 0;
    tmpf->buffering_mode = _IONBF;
    tmpf->buffer = NULL;
    tmpf->unget_pos = 0;
    tmpf->eof = 0;
#ifdef THREAD_SAFE
    okl4_mutex_init(&tmpf->mutex);
#endif

    return tmpf;
}
