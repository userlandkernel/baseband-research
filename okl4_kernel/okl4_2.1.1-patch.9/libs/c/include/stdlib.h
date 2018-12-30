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

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stdint.h>
#include <stddef.h>

/*
 * ISOC99 7.20 General Utilities 
 */

/*
 * 7.20.2 div types 
 */
typedef struct {
    int quot, rem;
} div_t;

typedef struct {
    long quot, rem;
} ldiv_t;

typedef struct {
    long long quot, rem;
} lldiv_t;

typedef struct {
    intmax_t quot, rem;
} imaxdiv_t;

static inline div_t
div(int x, int y) {
    div_t res;

    res.quot = x / y;
    res.rem = x % y;

    return res;
}

static inline ldiv_t
ldiv(long x, long y) {
    ldiv_t res;

    res.quot = x / y;
    res.rem = x % y;

    return res;
}

static inline lldiv_t
lldiv(long long x, long long y) {
    lldiv_t res;

    res.quot = x / y;
    res.rem = x % y;

    return res;
}

static inline imaxdiv_t
imaxdiv(intmax_t x, intmax_t y) {
    imaxdiv_t res;

    res.quot = x / y;
    res.rem = x % y;

    return res;
}

/* 7.20.3 EXIT_ macros */
#define EXIT_FAILURE    1
#define EXIT_SUCCESS    0

#define RAND_MAX        INT_MAX
#define MB_CUR_MAX      1

/* 7.20.1 Numeric conversion functions */

#ifndef CONFIG_WITHOUT_FLOATING
/* 7.20.1-3 The strtod, strtof and strtold functions */
double strtod(const char *s, char **endp);
float strtof(const char *s, char **endp);
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
long double strtold(const char *s, char **endp);
#endif
#endif

/* 7.20.1-4 The strtol, stroll, stroul, strtoull functions */
long strtol(const char *s, char **endp, int base);
long long strtoll(const char *s, char **endp, int base);
intmax_t strtoimax(const char *s, char **endp, int base);
unsigned long strtoul(const char *s, char **endp, int base);
unsigned long long strtoull(const char *s, char **endp, int base);
uintmax_t strtoumax(const char *s, char **endp, int base);

/* 7.20.1-1 atof function */
#ifndef CONFIG_WITHOUT_FLOATING
static inline double
atof(const char *nptr)
{
    return strtod(nptr, (char **)NULL);
}
#endif

/* 7.20.1-2 The atoi, atol and atoll functions */
#include <stdio.h>
static inline int
atoi(const char *nptr)
{
    return (int)strtol(nptr, (char **)NULL, 10);
}

static inline long
atol(const char *nptr)
{
    return strtol(nptr, (char **)NULL, 10);
}

static inline long long
atoll(const char *nptr)
{
    return strtoll(nptr, (char **)NULL, 10);
}

/* 7.20.2 Pseudo-random sequence generation functions */

int rand(void);
int rand_r(unsigned *seed);
void srand(unsigned int seed);

/* 7.20.3 Memory management functions */

void *malloc(size_t);
void free(void *);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);

/* 7.20.4 Communcation with the environment */

void abort(void);
int atexit(void (*func) (void));
void exit(int status);
void _Exit(int status);
int system(const char *string);
char *getenv(const char *name);
int unsetenv(const char *name);
int setenv(const char *envname, const char *envval, int overwrite);

/* 7.20.5 Searching and sortin utilities */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t,
              int (*compar) (const void *, const void *));
void qsort(void *base, size_t nmemb, size_t,
           int (*compar) (const void *, const void *));

/* 7.20.6 Integer arithmetic function */

/*
 * FIXME: (benjl) Gcc defines these, but if we aren't using gcc it probably
 * won't, but how do we know? Or maybe we should compile with -fnobuiltin? 
 */

int abs(int);
long labs(long);
long long llabs(long long);
intmax_t imaxabs(intmax_t);

/* Include POSIX symbols if in POSIX environment */
#ifdef __USE_POSIX
#include <posix/stdlib.h>
#endif

#if 0
static inline int
abs(int x)
{
    return x < 0 ? -x : x;
}

static inline long
labs(long x)
{
    return x < 0 ? -x : x;
}

static inline long long
llabs(long long x)
{
    return x < 0 ? -x : x;
}

static inline intmax_t
imaxabs(intmax_t x)
{
    return x < 0 ? -x : x;
}
#endif
/* 7.20.7 Multibyte/wide character conversion functions */
#if 0                           /* We don't have wide characters */
int mblen(const char *s, size_t n);
int mbtowc(wchar_t pwc, const char *s, size_t n);
int wctomb(char *s, wchat_t wc);
#endif

/* 7.20.8 Multibyte/wide string conversion functions */
#if 0                           /* We don't have wide characters */
size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
size_t wcstombs(char *s, constwchat_t * pwcs, size_t n);
#endif

#endif /* _STDLIB_H_ */
