/*
 * Copyright (c) 2004, National ICT Australia
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

/*
 * Author: Ben Leslie 
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <assert.h>

#ifdef THREAD_SAFE
#include <mutex/mutex.h>
#define lock_stream(s) do {if (!(s)->mutex_initialised) {okl4_mutex_init(&(s)->mutex);(s)->mutex_initialised = 1;}okl4_mutex_count_lock(&(s)->mutex);} while (0)
#define unlock_stream(s) do {assert((s)->mutex_initialised); okl4_mutex_count_unlock(&(s)->mutex);} while (0)
#else
#define lock_stream(s)
#define unlock_stream(s)
#endif

#define __UNGET_SIZE 10

struct __file {
    void *handle;

    size_t (*read_fn) (void *, long int, size_t, void *);
    size_t (*write_fn) (const void *, long int, size_t, void *);
    int (*close_fn) (void *);
    long int (*eof_fn) (void *);

    unsigned char buffering_mode;
    char *buffer;

    unsigned char unget_pos;
    long int current_pos;

#ifdef THREAD_SAFE
    int mutex_initialised;
    struct okl4_mutex mutex;
#endif

    int eof;
    int error;

    char unget_stack[__UNGET_SIZE];

    /* Added to end so as not to break existing init code */
    char *buffer_end;
    int buffer_size;
    void (*flush_fn) (int count);
};

typedef struct __file FILE;     /* This needs to be done correctly */
typedef long fpos_t;            /* same */

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 256
#define EOF (-1)

#define FOPEN_MAX 37
#define FILENAME_MAX 37
#define L_tmpnam 37

#ifndef SEEK_CUR
#define SEEK_CUR 0
#endif
#ifndef SEEK_END
#define SEEK_END 1
#endif
#ifndef SEEK_SET
#define SEEK_SET 2
#endif

#define TMP_MAX 37

extern FILE *stderr;
extern FILE *stdin;
extern FILE *stdout;

/* 7.19.4 Operations on files */
int remove(const char *);
int rename(const char *, const char *);
FILE *tmpfile(void);
char *tmpnam(char *);
int fclose(FILE *);
FILE *fdopen(int , const char *);
int fflush(FILE *);
int fileno(FILE *);
FILE *fopen(const char *, const char *);
FILE *freopen(const char *, const char *, FILE *);
void setbuf(FILE *, char *);
int setvbuf(FILE *, char *, int, size_t);

/* 7.19.6 Format i/o functions */
int fprintf(FILE *, const char *, ...);
int fscanf(FILE *, const char *, ...);
int printf(const char *, ...);
int scanf(const char *, ...);
int snprintf(char *, size_t, const char *, ...);
int sprintf(char *, const char *, ...);
int sscanf(const char *, const char *, ...);
int vfprintf(FILE *, const char *, va_list);
int vfscanf(FILE *, const char *, va_list);
int vprintf(const char *, va_list);
int vscanf(const char *, va_list);
int vsnprintf(char *, size_t, const char *, va_list);
int vasprintf(char **ret, const char *format, va_list ap);
int vsprintf(char *, const char *format, va_list arg);
int vsscanf(const char *s, const char *format, va_list arg);

/* 7.19.7 Character i/o functions */
int fgetc(FILE *);
char *fgets(char *, int, FILE *);
int fputc(int, FILE *);
int fputs(const char *, FILE *);

/*
 * getc is specified to be the same as fgetc, so it makes the most sense to
 * implement as a macro here 
 */
#define getc fgetc /*int getc(FILE *); */ /* lint -function(getc, fgetc) */

int getchar(void);
char *gets(char *);

/*
 * putc is specified to be the same as fputc, so it makes the most sense to
 * implement as a macro here 
 */
#define putc fputc /*int fetc(int, FILE *); */ /* lint -function(putc, fputc) */

int putchar(int);
int puts(const char *);
int ungetc(int, FILE *);

/* 7.19.8 Direct I/O functions */
size_t fread(void *, size_t, size_t, FILE *);
size_t fwrite(const void *, size_t, size_t, FILE *);

/* 7.19.9 File positioning functions */
int fgetpos(FILE *, fpos_t *);
int fseek(FILE *, long int, int);
int fseeko(FILE *stream, off_t offset, int whence);
int fsetpos(FILE *, const fpos_t *);
long int ftell(FILE *);
off_t ftello(FILE *stream);
void rewind(FILE *);

/* 7.19.10 Error-handling functions */
void clearerr(FILE *);
int feof(FILE *);
int ferror(FILE *);
void perror(const char *);

#endif /* _STDIO_H_ */
