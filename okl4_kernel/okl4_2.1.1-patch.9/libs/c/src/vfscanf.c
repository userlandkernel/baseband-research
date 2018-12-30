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
 * Authors: Carlos Dyonisio
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <errno.h>

#include <ctype.h>

#ifndef lock_stream
#define lock_stream(s)
#endif
#ifndef unlock_stream
#define unlock_stream(s)
#endif

#ifndef __va_copy
#define __va_copy(save, ap)     save = ap;
#endif

#define GET_ARG(var, type)  if (arg_nth == 0) {                             \
                                var = va_arg(ap, type);                     \
                            } else {                                        \
                                __va_copy(ap_tmp, ap_copy);                 \
                                while (--arg_nth > 0) {                      \
                                    (void)va_arg(ap_tmp, void*);            \
                                }                                           \
                                var = va_arg(ap_tmp, type);                 \
                            }

struct st_mem_stream {
    bool stream_or_memory;
    char **mem;
    FILE *stream;
    char last_stream_char;
    unsigned int read_bytes;
};

static inline char *
__get_next_char(struct st_mem_stream *data)
{
    char *c = NULL;
    if (data->stream_or_memory) {      /* 1 - stream */
        if (data->last_stream_char != '\0') {     /* no inc happened before */
            return &data->last_stream_char;
        }
        if (fread(c, 1, 1, data->stream)) {
            data->read_bytes++;
            data->last_stream_char = *c;
            return c;
        } else {
            data->last_stream_char = '\0';
            return &data->last_stream_char;
        }
    } else {
        c = *data->mem;
        return c;
    }
}

static inline void
__inc_next_char(struct st_mem_stream *data)
{
    if (data->stream_or_memory) {      /* 1 - stream */
        data->last_stream_char = '\0';
    } else {
        data->read_bytes++;
        (*data->mem)++;
    }
}

static inline int
__is_eof(char *s)
{
    if (*s == '\0') {
        return 1;
    } else {
        return 0;
    }
}

static void
inc_next_char(const void **ptr)
{
    struct st_mem_stream *data;

    data = (struct st_mem_stream*)*ptr;
    __inc_next_char(data);
}

static char *
get_next_char(const void **ptr)
{
    struct st_mem_stream *data;

    data = (struct st_mem_stream*)*ptr;
    return __get_next_char(data);
}

/**
 *
 */
static void
eat_spaces(struct st_mem_stream *data)
{
    char *s;

    s = __get_next_char(data);
    while (isspace(*s)) {
        __inc_next_char(data);
        s = __get_next_char(data);
    }
}

#ifndef CONFIG_WITHOUT_FLOATING
static int
eat_infinity(struct st_mem_stream *data)
{
    char *s;

    s = __get_next_char(data);
    if (*s != 'I' && *s != 'i') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'N' && *s != 'n') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'F' && *s != 'f') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'I' && *s != 'i') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'N' && *s != 'n') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'I' && *s != 'i') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'T' && *s != 't') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'Y' && *s != 'y') {
        return 0;
    }
    __inc_next_char(data);
    return 1;
}

static int
eat_nan(struct st_mem_stream *data)
{
    char *s;

    s = __get_next_char(data);
    if (*s != 'N' && *s != 'n') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'A' && *s != 'a') {
        return 0;
    }
    __inc_next_char(data);
    s = __get_next_char(data);
    if (*s != 'N' && *s != 'n') {
        return 0;
    }
    __inc_next_char(data);
    return 1;
}
#endif

/**
 *
 */
static inline int
check_list(char c, char *list_start, char *list_end, bool list_not)
{
    char *p;
    p = list_start;
    if (list_not) {
        while (p != list_end) {
            if (*p == c) {
                return 0;
            }
            p++;
        }
        return 1;
    } else {
        while (p != list_end) {
            if (*p == c) {
                return 1;
            }
            p++;
        }
        return 0;
    }
    /* should never get here */
}

#include "vfscanf.h"
#include "strtoll.h"
#include "strtold.h"
#include <assert.h>
/*
 * parse scanf format string 
 */
int
__scanf(const char *input, FILE *stream, bool stream_or_memory,
                                       const char *fmt, va_list ap)
{
    unsigned int assign_count = 0;
    va_list ap_copy, ap_tmp;
    int width, base = 0;
    char *p, *s;
    long long num_res = 0;
    unsigned long long num_ures = 0;
    bool num_unsigned = false;
    bool suppress = false;
    bool use_width = false;
    bool using_nth = false;
    /* length modifiers */
    bool lm_h, lm_hh, lm_l, lm_ll, lm_j, lm_z, lm_t, lm_L;
    int arg_nth = 0;
    int i = 0;
    bool list_not;
    char *list_start, *list_end;
    unsigned char *user_us;
    unsigned long *user_ul;
    unsigned long long *user_ull;
    unsigned short *user_ush;
    unsigned int *user_ui;
    uintmax_t *user_uj;
    char *user_s = NULL;
    long *user_l;
    long long *user_ll;
    short *user_sh;
    int *user_i;
    intmax_t *user_j;
    size_t *user_z;
    ptrdiff_t *user_t;
    void **user_pp;
#ifndef CONFIG_WITHOUT_FLOATING
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
    long double *user_ld;
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */
    double *user_d;
    float *user_f;
#endif /* CONFIG_WITHOUT_FLOATING */
    struct st_mem_stream mem_stream;
    mem_stream.stream_or_memory = stream_or_memory;
    mem_stream.mem = (char**)&input;
    mem_stream.stream = stream;
    mem_stream.last_stream_char = '\0';
    mem_stream.read_bytes = 0;

    __va_copy(ap_copy, ap);

    if (stream != NULL)
        lock_stream(stream);

    for (p = (char*)fmt; *p != '\0'; p++) {
        use_width = false;
        width = 1;
        num_unsigned = false;
        suppress = false;
        num_res = 0;
        num_ures = 0;
        lm_h = false;
        lm_hh = false;
        lm_l = false;
        lm_ll = false;
        lm_j = false;
        lm_z = false;
        lm_t = false;
        lm_L = false;
        if (*p != '%') {
char_comp:
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            if (isspace(*p)) {
                while (isspace(*s)) {
                    __inc_next_char(&mem_stream);
                    s = __get_next_char(&mem_stream);
                    if (__is_eof(s)) {
                        goto eof_failure;
                    }
                }
            } else {
                if (*p == *s) {
                    __inc_next_char(&mem_stream);
                } else {
                    goto matching_failure;
                }
            }
            continue;
        }
        /* *p == '%' */
again_inc:
        p++;
again:
        switch(*p) {
        case '%':
            goto char_comp;
            break;
        case '*':
            suppress = true;
            goto again_inc;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            i = *p - '0';
            for (p++; p && (unsigned int)(*p - '0') < 10; p++) {
                i = i * 10 + (*p - '0');
            }
            if (*p == '$') {
                using_nth = true;
                if (i == 0) {
                    goto matching_failure;
                } else {
                    arg_nth = i;
                }
                p++;
            } else {
                width = i;
                if (width != 0) {
                    use_width = true;
                }
            }
            goto again;
        /* Length modifiers */
        case 'h':
            if (lm_h) {
                lm_hh = true;
                lm_h = false;
            } else {
                lm_h = true;
            }
            goto again_inc;
        case 'l':
            if (lm_l) {
                lm_ll = true;
                lm_l = false;
            } else {
                lm_l = true;
            }
            goto again_inc;
        case 'j':
            lm_j = true;
            goto again_inc;
        case 'z':
            lm_z = true;
            goto again_inc;
        case 't':
            lm_t = true;
            goto again_inc;
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
        case 'L':
            lm_L = true;
            goto again_inc;
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */
        /* Numbers */
        case 'd':
            base = 10;
            goto number;
        case 'i':
            base = 0;
            goto number;
        case 'o':
            base = 8;
            num_unsigned = true;
            goto number;
        case 'u':
            base = 10;
            num_unsigned = true;
            goto number;
        case 'x':
        case 'X':
            base = 16;
            num_unsigned = true;
number:
            eat_spaces(&mem_stream);
            if (!use_width) {
                width = 0;
            }
    
            errno = 0; /* Check for strto* errors */
            if (suppress) {
                (void)__strtoll(NULL, base, width, get_next_char, inc_next_char,
                                                    (void*)&mem_stream);
            } else {
                if (num_unsigned) {
                    uintmax_t num_tmp;

                    errno = 0;
                    num_tmp = __strtoull(NULL, base, width, get_next_char,
                                               inc_next_char, (void*)&mem_stream);

                    if(errno == EINVAL) {
                        goto matching_failure;
                    }

                    assign_count++;

                    if (lm_ll) {
                        GET_ARG(user_ull, unsigned long long*);
                        *user_ull = (unsigned long long)num_tmp;
                    } else if (lm_l) {
                        GET_ARG(user_ul, unsigned long*);
                        *user_ul = (unsigned long)num_tmp;
                    } else if (lm_hh) {
                        GET_ARG(user_us, unsigned char*);
                        *user_us = (unsigned char)num_tmp;
                    } else if (lm_h) {
                        GET_ARG(user_ush, unsigned short*);
                        *user_ush = (unsigned short)num_tmp;
                    } else if (lm_j) {
                        GET_ARG(user_uj, uintmax_t*);
                        *user_uj = (uintmax_t)num_tmp;
                    } else if (lm_z) {
                        GET_ARG(user_z, size_t*);
                        *user_z = (size_t)num_tmp;
                    } else if (lm_t) {
                        GET_ARG(user_t, ptrdiff_t*);
                        *user_t = (unsigned long)num_tmp;
                    } else {
                        GET_ARG(user_ui, unsigned int*);
                        *user_ui = (unsigned int)num_tmp;
                    }
                } else {
                    intmax_t num_tmp;

                    errno = 0;
                    num_tmp = __strtoll(NULL, base, width, get_next_char,
                                               inc_next_char, (void*)&mem_stream);

                    if(errno == EINVAL) {
                        goto matching_failure;
                    }

                    assign_count++;

                    if (lm_ll) {
                        GET_ARG(user_ll, long long*);
                        *user_ll = (long long)num_tmp;
                    } else if (lm_l) {
                        GET_ARG(user_l, long*);
                        *user_l = (long int)num_tmp;
                    } else if (lm_hh) {
                        GET_ARG(user_s, char*);
                        *user_s = (char)num_tmp;
                    } else if (lm_h) {
                        GET_ARG(user_sh, short*);
                        *user_sh = (short)num_tmp;
                    } else if (lm_j) {
                        GET_ARG(user_j, intmax_t*);
                        *user_j = (intmax_t)num_tmp;
                    } else if (lm_z) {
                        GET_ARG(user_z, size_t*);
                        *user_z = (signed long)num_tmp;
                    } else if (lm_t) {
                        GET_ARG(user_t, ptrdiff_t*);
                        *user_t = (ptrdiff_t)num_tmp;
                    } else {
                        GET_ARG(user_i, int*);
                        *user_i = (int)num_tmp;
                    }
                }
            }
            /*
            if (errno == EINVAL) {
                goto matching_failure;
            }
            */
            break;
#ifndef CONFIG_WITHOUT_FLOATING
        case 'A':
        case 'a':
        case 'E':
        case 'e':
        case 'F':
        case 'f':
        case 'G':
        case 'g':
            eat_spaces(&mem_stream);
            if (!use_width) {
                width = 0;
            }

            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            if (*s == 'i' || *s == 'I') {
                if (!eat_infinity(&mem_stream)) {
                    s = __get_next_char(&mem_stream);
                    if (*s == '\0') {
                        goto input_failure;
                    } else {
                        goto matching_failure;
                    }
                }
                /*
                if (!suppress) {
                    assign_count++;
                    if (lm_l) {
                        GET_ARG(user_d, double*);
                        *user_d = INFINITY;
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
                    } else if (lm_L) {
                        GET_ARG(user_ld, long double*);
                        *user_ld = INFINITY;
#endif
                    } else {
                        GET_ARG(user_f, float*);
                        *user_f = INFINITY;
                    }
                }
                */
            } else if (*s == 'n' || *s == 'N') {
                if (!eat_nan(&mem_stream)) {
                    goto matching_failure;
                }
                /*
                if (!suppress) {
                    assign_count++;
                    if (lm_l) {
                        GET_ARG(user_d, double*);
                        *user_d = NAN;
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
                    } else if (lm_L) {
                        GET_ARG(user_ld, long double*);
                        *user_ld = NAN;
#endif
                    } else {
                        GET_ARG(user_f, float*);
                        *user_f = NAN;
                    }
                }
                */
            } else {
                if (suppress) {
                    (void)__strtold(NULL, width, get_next_char, inc_next_char,
                                                    (void*)&mem_stream);
                } else {
                    assign_count++;
                    if (lm_l) {
                        GET_ARG(user_d, double*);
                        *user_d = (double)__strtold(NULL, width, get_next_char,
                                        inc_next_char, (void*)&mem_stream);
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
                    } else if (lm_L) {
                        GET_ARG(user_ld, long double*);
                        *user_ld = (long double)__strtold(NULL, width,
                                                          get_next_char,
                                                          inc_next_char,
                                                          (void*)&mem_stream);
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */
                    } else {
                        GET_ARG(user_f, float*);
                        *user_f = (float)__strtold(NULL, width, get_next_char,
                                        inc_next_char, (void*)&mem_stream);
                    }
                }
            }
            break;
#endif /* CONFIG_WITHOUT_FLOATING */
        case 'S':
            lm_l = true;
        case 's':
            eat_spaces(&mem_stream);
            if (!suppress) {
                GET_ARG(user_s, char*);
                i = 0;
            }
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            while (s != NULL && *s != '\0' && !isspace(*s)) {
                if (use_width) {
                    if (width > 0) {
                        width--;
                    } else {
                        break;
                    }
                }
                if (!suppress) {
                    user_s[i] = *s;
                    i++;
                }
                __inc_next_char(&mem_stream);
                s = __get_next_char(&mem_stream);
            }
            if (!suppress) {
                assign_count++;
                user_s[i] = '\0';
            }
            break;
        case '[':
            list_not = false;
            p++;
            if (*p == '^') {
                list_not = true;
                p++;
            }
            if (*p == '\0') {
                goto matching_failure;
            }
            list_start = p;
            p++;
            while (*p != ']') {
                if (*p == '\0') {
                    goto matching_failure;
                }
                p++;
            }
            list_end = p;

            if (!suppress) {
                GET_ARG(user_s, char*);
                i = 0;
            }
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            while (s != NULL && *s != '\0' &&
                        check_list(*s, list_start, list_end, list_not)) {
                if (use_width) {
                    if (width > 0) {
                        width--;
                    } else {
                        break;
                    }
                }
                if (!suppress) {
                    user_s[i] = *s;
                    i++;
                }
                __inc_next_char(&mem_stream);
                s = __get_next_char(&mem_stream);
            }
            if (!suppress) {
                assign_count++;
                user_s[i] = '\0';
            }
            break;
        case 'C':
            lm_l = true;
        case 'c':
            use_width = true;
            if (!suppress) {
                GET_ARG(user_s, char*);
                i = 0;
            }
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            for (; width > 0 && s != NULL && *s != '\0'; width--) {
                if (!suppress) {
                    user_s[i] = *s;
                    i++;
                }
                __inc_next_char(&mem_stream);
                s = __get_next_char(&mem_stream);
            }
            if (__is_eof(s) && width > 0) {
                goto eof_failure;
            }
            if (!suppress) {
                assign_count++;
                user_s[i] = '\0';
            }
            break;
        case 'p':
            eat_spaces(&mem_stream);
            if (!use_width) {
                width = 0;
            } else if (width < 2) {
                goto matching_failure;
            }
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            if (*s != '0') {
                goto matching_failure;
            }
            __inc_next_char(&mem_stream);
            s = __get_next_char(&mem_stream);
            if (__is_eof(s)) {
                goto eof_failure;
            }
            if (*s != 'x' && *s != 'X') {
                goto matching_failure;
            }
            __inc_next_char(&mem_stream);
            width -= 2;
            if (suppress) {
                (void)__strtoll(NULL, 16, width, get_next_char, inc_next_char,
                                                    (void*)&mem_stream);
            } else {
                assign_count++;
                GET_ARG(user_pp, void**);
                *user_pp = (void*)(long)__strtoll(NULL, 16, width, get_next_char,
                                    inc_next_char, (void*)&mem_stream);
            }
            break;
        case 'n':
            if (lm_ll) {
                GET_ARG(user_ll, long long*);
                *user_ll = (long long)mem_stream.read_bytes;
            } else if (lm_l) {
                GET_ARG(user_l, long*);
                *user_l = (long)mem_stream.read_bytes;
            } else if (lm_hh) {
                GET_ARG(user_s, char*);
                *user_s = (char)mem_stream.read_bytes;
            } else if (lm_h) {
                GET_ARG(user_sh, short*);
                *user_sh = (short)mem_stream.read_bytes;
            } else if (lm_j) {
                GET_ARG(user_j, intmax_t*);
                *user_j = (intmax_t)mem_stream.read_bytes;
            } else if (lm_z) {
                GET_ARG(user_i, int*);
                *user_i = (int)mem_stream.read_bytes;
            } else if (lm_t) {
                GET_ARG(user_t, ptrdiff_t*);
                *user_t = (ptrdiff_t)mem_stream.read_bytes;
            } else {
                GET_ARG(user_i, int*);
                *user_i = (int)mem_stream.read_bytes;
            }
            break;
        default:
            if (*p == '\0') {
                break;
            }
        }
    }

matching_failure:
    if (stream != NULL) {
        unlock_stream(stream);
    }

    return assign_count;
    /* XXX: va_end */

input_failure:
    if (stream != NULL) {
        unlock_stream(stream);
    }
    return EOF;

eof_failure:
    if (assign_count > 0) {
        goto matching_failure;
    } else {
        goto input_failure;
    }

}

int
vfscanf(FILE *stream, const char *format, va_list arg)
{
    return __scanf(NULL, stream, true, format, arg);
}

int
vscanf(const char *format, va_list arg)
{
    return __scanf(NULL, stdin, true, format, arg);
}

int
vsscanf(const char *s, const char *format, va_list arg)
{
    return __scanf(s, NULL, false, format, arg);
}
