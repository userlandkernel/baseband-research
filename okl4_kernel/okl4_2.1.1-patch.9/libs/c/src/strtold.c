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
 * Authors: Carlos Dyonisio <medaglia@ok-labs.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <errno.h>

#include "strtold.h"

#ifndef CONFIG_WITHOUT_FLOATING

static double powers_of_10[] = {
    10.0,
    100.0,
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256,
    1.0e512,
    1.0e1024,
};

static double
get_power_of_10(int num)
{
    int i;
    double res = 1;

    for (i = 0; i < 11; i++) {
        if (num & (1 << i)) {
            res *= powers_of_10[i];
        }
    }
    return res;
}

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

static inline char *
get_from_buf(const void **ptr)
{
    return (char*)*ptr;
}

static inline void
inc_buf(const void **ptr)
{
    char *p;
    p = (char*)*ptr;

    p++;
    *ptr = p;
}


long_double_t
__strtold(char **endptr, int size, char *(*get_next_char)(const void**),
               void (*inc_next_char)(const void**), const void *func_data)
{
    char buf[50];
    unsigned int pos, mant_start, mant_end;
    char *s;

    int base = 10;
    int sign = 0;
    int exp = 0;
    int exp_sign = 0;

    int mant_size = 0;
    int has_point = 0;

    int possible_underflow = 0;

    buf[0] = '\0';
    pos = 0;
    mant_start = 0;
    mant_end = 0;

    long_double_t result = 0.0;
    long_double_t old_res;

    if (size <= 0) {
        size = 10000;
    }


    s = get_next_char(&func_data);
    if (s == NULL || *s == '\0') {
        goto fail;
    }
    /* Ignore blank spaces */
    while (isspace(*s)) {
        inc_next_char(&func_data);
        s = get_next_char(&func_data);
        if (s == NULL || *s == '\0') {
            goto fail;
        }
    }
    /* Check for sign */
    if (*s == '-') {
        sign = 1;
        inc_next_char(&func_data);
        if (--size <= 0) {
            goto end_ret;
        }
        s = get_next_char(&func_data);
    } else if (*s == '+') {
        sign = 0;
        inc_next_char(&func_data);
        if (--size <= 0) {
            goto end_ret;
        }
        s = get_next_char(&func_data);
    }

    if (s == NULL || *s == '\0') {
        goto fail;
    }

    if (*s == '0') {
        inc_next_char(&func_data);
        if (--size <= 0) {
            goto end_ret;
        }
        s = get_next_char(&func_data);
        if (s == NULL || *s == '\0') { /* input = "(+|-)0" = 0 */
            return 0;
        }
        if (*s == 'x' || *s == 'X') {
            base = 16;
            inc_next_char(&func_data);
            if (--size <= 0) {
                goto end_ret;
            }
            s = get_next_char(&func_data);
        }
    }

    mant_start = pos = 0;
    while (s && *s != '\0' && (isdigit(*s) || *s == '.')) {
        if (*s == '.') {
            if (has_point) {
                break;
            } else {
                has_point = 1;
            }
        } else {
            buf[pos] = *s;
            if (!has_point) {
                mant_size++;
            }
            pos++;
        }
        inc_next_char(&func_data);
        if (--size <= 0) {
            goto end_ret;
        }
        s = get_next_char(&func_data);
    }

    if (s && (*s == 'e' || *s == 'E' || *s == 'p' || *s == 'P')) {
        inc_next_char(&func_data);
        if (--size <= 0) {
            goto end_ret;
        }
        s = get_next_char(&func_data);
        if (s == NULL || *s == '\0') {
            goto end_ret;
        }
        if (*s == '+' || *s == '-') {
            if (*s == '-') {
                exp_sign = 1;
            } else {
                exp_sign = 0;
            }
            inc_next_char(&func_data);
            if (--size <= 0) {
                goto end_ret;
            }
            s = get_next_char(&func_data);
        }
        while (s && *s != '\0' && isdigit(*s)) {
            exp *= 10;
            exp += (*s - '0');
            inc_next_char(&func_data);
            if (--size <= 0) {
                goto end_ret;
            }
            s = get_next_char(&func_data);
        }
    }

    if(exp_sign && exp != 0) {
        possible_underflow = 1;
    }

end_ret:
    if (base == 10) {
        if (exp_sign) {
            exp = exp - (mant_size - 1);
        } else {
            exp = exp + (mant_size - 1);
        }
    } else if (base == 16) {
        if (exp_sign) {
            exp = exp - ((mant_size - 1) * 4);
        } else {
            exp = exp + ((mant_size - 1) * 4);
        }
    }

    if (exp < 0) {
        exp *= -1;
        if (exp_sign) {
            exp_sign = 0;
        } else {
            exp_sign = 1;
        }
    }

    mant_end = pos-1;
    if (base == 10) {
        pos = mant_start;
        for (; pos <= mant_end; pos++) {
            old_res = result;
            if (exp_sign) {
                result = result +
                    (long_double_t)((long_double_t)char_value(buf[pos]) /
                    (long_double_t)get_power_of_10(exp));
            } else {
                result = result +
                    (long_double_t)((long_double_t)char_value(buf[pos]) *
                    (long_double_t)get_power_of_10(exp));
            }
            if (exp_sign) {
                exp++;
            } else {
                exp--;
                if (exp < 0) {
                    exp *= -1;
                    if (exp_sign) {
                        exp_sign = 0;
                    } else {
                        exp_sign = 1;
                    }
                }
            }
            if (result != old_res && result == (result/2)) {
                errno = ERANGE;
                return result;
            }
        }
    } else if (base == 16) {
        pos = mant_start;
        for (; pos <= mant_end; pos++) {
            old_res = result;
            if (exp_sign) {
                result = result +
                    (long_double_t)((long_double_t)char_value(buf[pos]) /
                    (long_double_t)(1 << exp));
            } else {
                result = result +
                    (long_double_t)((long_double_t)(char_value(buf[pos]) *
                    (long_double_t)(1 << exp)));
            }
            if (exp_sign) {
                exp += 4;
            } else {
                exp -= 4;
                if (exp < 0) {
                    exp *= -1;
                    if (exp_sign) {
                        exp_sign = 0;
                    } else {
                        exp_sign = 1;
                    }
                }
            }
            if (result != old_res && result == (result/2)) {
                errno = ERANGE;
                return result;
            }
        }
    }

    if (possible_underflow && result == (result/2)) {
        errno = ERANGE;
    }

    return result;

fail:
    errno = EINVAL;
    return 0;
}

#ifndef CONFIG_WITHOUT_LONG_DOUBLE
long double
strtold(const char *nptr, char **endptr)
{
    return __strtold(endptr, 0, get_from_buf, inc_buf, nptr);
}
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */

double
strtod(const char *nptr, char **endptr)
{
    double res;
    long double old_res;

    old_res = __strtold(endptr, 0, get_from_buf, inc_buf, nptr);
    res = (double)old_res;

    if(old_res != (old_res/2) && res == (res/2)) {
        errno = ERANGE;
    }
    return res;
}

float
strtof(const char *nptr, char **endptr)
{
    float res;
    long double old_res;

    old_res = __strtold(endptr, 0, get_from_buf, inc_buf, nptr);
    res = (float)old_res;

    if(old_res != (old_res/2) && res == (res/2)) {
        errno = ERANGE;
    }
    return res;
}

#endif /* CONFIG_WITHOUT_FLOATING */
