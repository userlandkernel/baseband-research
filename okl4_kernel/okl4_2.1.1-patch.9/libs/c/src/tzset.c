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

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char tzname[2][TZNAME_MAX+1] = { "", "" };
long timezone = 0;
int daylight = 0;

static char buf[TZNAME_MAX+1];

static inline char *
get_std_or_dst(char **p)
{
    register int i = TZNAME_MAX;
    int quoted = 0;
    char *start;
    size_t s;

    if (**p == '<') {
        quoted = 1;
        (*p)++;
    }

    start = *p;

    while (i-- && *p) {
        if (quoted) {
            if (**p == '>' || (!isalnum(**p) && **p != '+' && **p != '-')) {
                break;
            }
        } else {
            if (!isalpha(**p)) {
                break;
            }
        }
        (*p)++;
    }
    s = *p - start;
    memcpy(buf, start, s);
    buf[s] = '\0';
    
    if (quoted) {
        (*p)++;
    }
    return buf;
}

#include <stdio.h>
static inline long
get_offset(char **p)
{
    int neg = 0;
    long tmp;
    long ret = 0;
    char *endptr;

    if (**p == '-') {
        neg = 1;
        (*p)++;
    } else if (**p == '+') {
        (*p)++;
    }

    tmp = (long)strtoll(*p, &endptr, 10);
    *p = endptr;
    ret += tmp * 3600;

    if (**p != ':') {
        goto end;
    }
    /* Minutes */
    (*p)++;
    tmp = strtol(*p, &endptr, 10);
    *p = endptr;
    ret += tmp * 60;

    if (**p != ':') {
        goto end;
    }
    /* Seconds */
    (*p)++;
    tmp = strtol(*p, &endptr, 10);
    *p = endptr;
    ret += tmp;

end:
    if (neg) {
        ret = -ret;
    }
    return ret;
}

void
_tzset(char *_TZ)
{
    char *p;
    long offset;
    long dst_offset = 0;
    int use_dst = 0;

    p = _TZ;
    if (!p) {
        // XXX: error
        return;
    }

    strncpy(tzname[0], get_std_or_dst(&p), TZNAME_MAX);
    offset = get_offset(&p);

    if (*p == ',' || *p == '\0') {
        goto end;
    }
    
    strncpy(tzname[1], get_std_or_dst(&p), TZNAME_MAX);
    if (isdigit(*p) || *p == '+' || *p == '-') {
        dst_offset = get_offset(&p);
    } else {
        dst_offset = offset + 3600;
    }

/*
    if (*p != ',') {
        goto end;
    }
 *
 * TODO
 * Past this point, tzset() should read the rule and compare with
 * the actual date (using time()) to see if it should use the STD offset
 * or the DST offset.
 * It was not implemented now because time() is not implemented.
 * One day, we should have a database of rules, like uClibc and GNU libc,
 * if needed.
 */

end:
    if (use_dst) {
        timezone = dst_offset;
    } else {
        timezone = offset;
    }
    return;
}

void
tzset(void)
{
    /*
     * TODO
     * When we have environment support, tzset must read the environment
     * variable TZ to check the timezone.
     */

    _tzset("GMT0");
}
