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
 * Author: Ben Leslie Created: Fri Oct 8 2004 
 */

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "strtoll.h"

static int internal_error;

/**
 * Work out the numeric value of a char, assuming up to base 36
 *  
 * @param ch The character to decode
 *
 * \return Numeric value of character, or 37 on failure
 */
static inline unsigned short
char_value(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }

    return 37;
}


uintmax_t
__strtoull_noendptr(int base, int *size, char *(*get_next_char)(const void**),
                void (*inc_next_char)(const void**), const void *func_data)
{
    char *c;
    unsigned int value;
    uintmax_t return_value = 0;

    c = get_next_char(&func_data);
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    if (base > 36) {
        goto fail;
    }

    if (base == 16) {
        /* _May_ have 0x prefix */
        if (*c == '0') {
            inc_next_char(&func_data);
            if (--(*size) < 0) {
                goto end_ret;
            }
            c = get_next_char(&func_data);
            if (c != NULL && (*c == 'x' || *c == 'X')) {
                inc_next_char(&func_data);
                if (--(*size) < 0) {
                    goto end_ret;
                }
                c = get_next_char(&func_data);
            }
        }
    }
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    /* [0(x|X)+] */
    if (base == 0) {
        /* Could be hex or octal or decimal */
        if (*c != '0') {
            base = 10;
        } else {
            inc_next_char(&func_data);
            if (--(*size) < 0) {
                goto end_ret;
            }
            c = get_next_char(&func_data);
            if (c == NULL || *c == '\0') {
                goto fail;
            }
            if (*c == 'x' || *c == 'X') {
                base = 16;
                inc_next_char(&func_data);
                if (--(*size) < 0) {
                    goto end_ret;
                }
                c = get_next_char(&func_data);
            } else {
                base = 8;
            }
        }
    }
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    /* Ok, here we have a base, and we might have a valid number */
    value = char_value(*c);
    if (value >= base) {
        goto fail;
    } else {
        return_value = value;
        inc_next_char(&func_data);
        if (--(*size) < 0) {
            goto end_ret;
        }
        c = get_next_char(&func_data);
    }

    while (c != NULL && *c != '\0' && (value = char_value(*c)) < base
                                                           && --(*size) >= 0) {
        if (return_value > (UINTMAX_MAX / base)) {
            errno = ERANGE;
            goto fail;
        }
        return_value = (return_value * base) + value;
        inc_next_char(&func_data);
        c = get_next_char(&func_data);
    }

end_ret:
    return return_value;

fail:
    internal_error = 1;
    return 0;
}

/*
 *
 */
uintmax_t
__strtoull(char **endptr, int base, int size,
                char *(*get_next_char)(const void**),
                void (*inc_next_char)(const void**),
                const void *func_data)
{
    char *c = NULL;
    uintmax_t return_value = 0;

    internal_error = 0;

    if (size <= 0) {
        size = 10000;
    }

    c = get_next_char(&func_data);
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    /* check [+|-] */
    if (*c == '+') {
        inc_next_char(&func_data);
        if (--size < 0) {
            goto end_ret;
        }
        c = get_next_char(&func_data);
    } else if (*c == '-') {
        errno = ERANGE;
        return_value = UINTMAX_MAX;
        goto end_ret;
    }
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    return_value = __strtoull_noendptr(base, &size, get_next_char, inc_next_char,
                                                                func_data);

    if (internal_error) {
        goto fail;
    }

end_ret:
    if (endptr != NULL) {
        c = get_next_char(&func_data);
        *endptr = c;
    }

    return return_value;

fail:
    if (endptr != NULL) {
        *endptr = (char*)func_data;
    }

    if(errno == ERANGE) {
        return UINTMAX_MAX;
    }

    errno = EINVAL;
    return 0;
}

intmax_t
__strtoll(char **endptr, int base, int size,
               char *(*get_next_char)(const void**),
               void (*inc_next_char)(const void**),
               const void *func_data)
{
    char *c = NULL;
    bool negative = false;
    intmax_t return_value = 0;
    uintmax_t ret = 0;

    internal_error = 0;

    if (size <= 0) {
        size = 10000;
    }

    c = get_next_char(&func_data);
    if (c == NULL || *c == '\0') {
        goto fail;
    }

    /* check [+|-] */
    if (*c == '+') {
        inc_next_char(&func_data);
        if (--size < 0) {
            goto end_ret;
        }
        c = get_next_char(&func_data);
    } else if (*c == '-') {
        negative = true;
        inc_next_char(&func_data);
        if (--size < 0) {
            goto end_ret;
        }
        c = get_next_char(&func_data);
    }

    if (c == NULL || *c == '\0') {
        goto fail;
    }

    ret = __strtoull_noendptr(base, &size, get_next_char, inc_next_char,
                                                                func_data);

    if (internal_error) {
        goto fail;
    }

    if(ret > INTMAX_MAX) {
        errno = ERANGE;
    }

end_ret:
    if (endptr != NULL) {
        c = get_next_char(&func_data);
        *endptr = c;
    }

    return_value = (intmax_t)ret;
    if (negative) {
        return_value = -return_value;
    }

    return return_value;

fail:
    if (endptr != NULL) {
        *endptr = (char*)func_data;
    }

    if(errno == ERANGE) {
        return INTMAX_MAX;
    }

    errno = EINVAL;
    return 0;
}

long long
strtoll(const char *str, char **endptr, int base)
{
    struct st_buf_ptr buf;
    buf.p = (char*)str;
    intmax_t ret;

    remove_spaces(&buf);

    ret = __strtoll(endptr, base, 0, get_from_buf, inc_buf, (void*)&buf);

    if (errno == ERANGE) {
        ret = LLONG_MAX;
    } else if (ret > LLONG_MAX) {
        ret = LLONG_MAX;
        errno = ERANGE;
    }
    
    return (long long)ret;
}

intmax_t
strtoimax(const char *str, char **endptr, int base)
{
    struct st_buf_ptr buf;
    buf.p = (char*)str;

    remove_spaces(&buf);

    return __strtoll(endptr, base, 0, get_from_buf, inc_buf, (void*)&buf);
}
