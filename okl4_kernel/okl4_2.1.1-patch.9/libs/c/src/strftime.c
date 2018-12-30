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
 * Author: Carlos Dyonisio <medaglia@ok-labs.com>
 */

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#include "libtime.h"

static inline int
copy_char_to_buf(char **buf, size_t *maxsize, char c)
{
    if (*maxsize > 0) {
        (*maxsize)--;
        (void)memcpy(*buf, &c, 1);
        (*buf)++;
        return 1;
    }
    return 0;
}

static inline int
copy_str_to_buf(char **dst, size_t *maxsize, const char *fmt, ...)
{
    int rd;
    va_list ap;

    va_start(ap, fmt);
    rd = vsnprintf(*dst, *maxsize, fmt, ap);
    va_end(ap);

    if (rd > *maxsize) {
        *dst += *maxsize;
        *maxsize = 0;
        return 0;
    } else {
        *maxsize -= rd;
        *dst += rd;
        return 1;
    }
}

size_t
strftime(char *s, size_t _maxsize, const char *fmt, const struct tm *pt)
{
    char *p;
    char *dst = s;
    int tmp;
    size_t maxsize = _maxsize;

    tzset();

    for (p = (char*)fmt; *p != '\0'; p++) {
        if (*p != '%') {
char_cpy:
            if (!copy_char_to_buf(&dst, &maxsize, *p)) {
                goto err;
            }
            continue;
        }
        /* *p == '%' */
again:
        p++;
        switch(*p) {
        case '%':
            goto char_cpy;
            break;
        case 'E':
        case 'O':
        /* 
         * XXX TODO FIXME:
         * We don't have locale support,
         * therefore, E and O don't change anything.
         */
            goto again;
            break;
        case 'a':
            if (!copy_str_to_buf(&dst, &maxsize, "%s",
                                          get_locale_abrv_wday_name(pt))) {
                goto err;
            }
            break;
        case 'A':
            if (!copy_str_to_buf(&dst, &maxsize, "%s",
                                            get_locale_wday_name(pt))) {
                goto err;
            }
            break;
        case 'h':
        case 'b':
            if (!copy_str_to_buf(&dst, &maxsize, "%s",
                                          get_locale_abrv_mon_name(pt))) {
                goto err;
            }
            break;
        case 'B':
            if (!copy_str_to_buf(&dst, &maxsize, "%s",
                                              get_locale_mon_name(pt))) {
                goto err;
            }
            break;
        case 'c':
            if (!copy_str_to_buf(&dst, &maxsize, "%s %s %s%d %.2d:%.2d:%.2d %d",
                                get_locale_abrv_wday_name(pt),
                                get_locale_abrv_mon_name(pt),
                                pt->tm_mday > 9 ? "" : " ",
                                pt->tm_mday,
                                pt->tm_hour, pt->tm_min, pt->tm_sec,
                                1900 + pt->tm_year)) {
                goto err;
            }
            break;
        case 'C':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                              (1900 + pt->tm_year) / 100)) {
                goto err;
            }
            break;
        case 'd':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", pt->tm_mday)) {
                goto err;
            }
            break;
        case 'D':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d/%.2d/%.2d",
                                               pt->tm_mon + 1, pt->tm_mday,
                                               (1900 + pt->tm_year) % 100)) {
                goto err;
            }
            break;
        case 'e':
            if (!copy_str_to_buf(&dst, &maxsize, "%s%d", 
                                      pt->tm_mday > 9 ? "" : " ",
                                      pt->tm_mday)) {
                goto err;
            }
            break;
        case 'F':
            if (!copy_str_to_buf(&dst, &maxsize, "%d-%.2d-%.2d",
                                                1900 + pt->tm_year,
                                                pt->tm_mon + 1,
                                                pt->tm_mday)) {
                goto err;
            }
            break;
        case 'g':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                             get_iso_year(pt) % 100)) {
                goto err;
            }
            break;
        case 'G':
            if (!copy_str_to_buf(&dst, &maxsize, "%d", get_iso_year(pt))) {
                goto err;
            }
            break;
        case 'H':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", pt->tm_hour)) {
                goto err;
            }
            break;
        case 'I':
            tmp = get_hour_in_12(pt);
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", tmp)) {
                goto err;
            }
            break;
        case 'j':
            if (!copy_str_to_buf(&dst, &maxsize, "%.3d", pt->tm_yday+1)) {
                goto err;
            }
            break;
        case 'm':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", pt->tm_mon+1)) {
                goto err;
            }
            break;
        case 'M':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", pt->tm_min)) {
                goto err;
            }
            break;
        case 'n':
            if (!copy_char_to_buf(&dst, &maxsize, '\n')) {
                goto err;
            }
            break;
        case 'p':
            if (!copy_str_to_buf(&dst, &maxsize, "%s", get_am_or_pm(pt))) {
                goto err;
            }
            break;
        case 'r':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d:%.2d:%.2d %s",
                                      get_hour_in_12(pt), pt->tm_min,
                                      pt->tm_sec, get_am_or_pm(pt))) {
                goto err;
            }
            break;
        case 'R':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d:%.2d", pt->tm_hour,
                                                             pt->tm_min)) {
                goto err;
            }
            break;
        case 'S':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", pt->tm_sec)) {
                goto err;
            }
            break;
        case 't':
            if (!copy_char_to_buf(&dst, &maxsize, '\t')) {
                goto err;
            }
            break;
        /*
         * XXX FIXME TODO
         * Because we only have C locale support,
         * %X is equivalent to %T.
         * It can be different otherwise.
         */
        case 'X':
        case 'T':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d:%.2d:%.2d",
                                    pt->tm_hour, pt->tm_min, pt->tm_sec)) {
                goto err;
            }
            break;
        case 'u':
            if (pt->tm_wday == 0) {
                tmp = 7;
            } else {
                tmp = pt->tm_wday - 1;
            }
            if (!copy_str_to_buf(&dst, &maxsize, "%d", tmp)) {
                goto err;
            }
            break;
        case 'U':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d", get_week_number(pt))) {
                goto err;
            }
            break;
        case 'V':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                                    get_iso_week_number(pt))) {
                goto err;
            }
            break;
        case 'w':
            if (!copy_str_to_buf(&dst, &maxsize, "%d", pt->tm_wday)) {
                goto err;
            }
            break;
        case 'W':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                               get_week_number_monday(pt))) {
                goto err;
            }
            break;
        case 'x':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d/%.2d/%.2d",
                                            pt->tm_mon + 1,
                                            pt->tm_mday,
                                            (1900 + pt->tm_year) % 100)) {
                goto err;
            }
            break;
        case 'y':
            if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                       (1900 + pt->tm_year) % 100)) {
                goto err;
            }
            break;
        case 'Y':
            if (!copy_str_to_buf(&dst, &maxsize, "%d", 1900 + pt->tm_year)) {
                goto err;
            }
            break;
        case 'z':
            if (pt->tm_isdst) {
                // TODO: DST timezone
            } else {
                if (*tzname[0] == '\0') {
                    break;
                }
                if (!copy_char_to_buf(&dst, &maxsize, timezone < 0 ?
                                                     '-' : '+')) {
                    goto err;
                }
                if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                                     timezone / 3600)) {
                    goto err;
                }
                if (!copy_str_to_buf(&dst, &maxsize, "%.2d",
                                                  (timezone % 3600)) / 60) {
                    goto err;
                }
            }
            break;
        case 'Z':
            if (pt->tm_isdst) {
                if (!copy_str_to_buf(&dst, &maxsize, "%s", tzname[1])) {
                    goto err;
                }
            } else {
                if (!copy_str_to_buf(&dst, &maxsize, "%s", tzname[0])) {
                    goto err;
                }
            }
            break;
        default:
            goto err;
            break;
/*
        case '':
            if (!copy_str_to_buf(&dst, &maxsize)) {
                goto err;
            }
            break;
*/
        };
    }
    if (!copy_char_to_buf(&dst, &maxsize, '\0')) {
        goto err;
    }

    return _maxsize - maxsize;

err:
    return 0;
}
