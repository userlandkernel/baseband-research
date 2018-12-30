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

#ifndef _RTC_S3C2410_UTIL_H_
#define _RTC_S3C2410_UTIL_H_

#include <inttypes.h>

#define BCD2BIN(val)    (((val) & 0x0f) + ((val)>>4)*10)
#define BIN2BCD(val)    ((((val)/10)<<4) + (val)%10)

/* This device only works in the 21st century - it does not guarantee to
 * correctly calculate leap years for other centuries. */
#define CURRENT_CENTURY 2000

/* Internal timer representation used by s3c2410 RTC. tm->year is two digits
 * starting in year CURRENT_CENTURY. */
struct rtc_time {
        int sec;
        int min;
        int hour;
        int date;
        int mon;
        int year;
};

static const int * month2days_ptr;

/* Number of days elapsed at the beginning of each month. */
static int const month2days[] =
{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

static int const month2days_ly[] =
{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

/* Is year a leap year? */
static inline int
is_leap_year(unsigned int year)
{
    int ret = 0;

    if ((year % 4) == 0) {
        ret = 1;
    }
    else {
        return 0;
    }

    if (((year % 100) == 0) && ((year % 400) != 0)) {
        ret = 0;
    }

    return ret;
}

/* Counts leap years between range. */
static inline int
leap_years(unsigned int start, unsigned int end)
{
    int i, lys = 0;

    for (i = start; i < end; i++) {
        if (is_leap_year(i)) {
            lys++;
        }
    }

    return lys;
}

/* Convert from s3c2410 internal representation to unix time. */
static inline int
rtc_to_unix(uint64_t *unix_time, const struct rtc_time *tm)
{
    int days_since_jan1, year;
    *unix_time = 0;

    if (is_leap_year(tm->year)){
        month2days_ptr = month2days_ly;
    } else {
        month2days_ptr = month2days;
    }

    days_since_jan1 = month2days_ptr[tm->mon - 1] + tm->date;
    year = tm->year + CURRENT_CENTURY - 1900;

    /* Magic formula to approximate elapsed seconds since epoch. For explanation
     * see POSIX IEEE Std 1003.1 'Base Definitions' document. */
    *unix_time = tm->sec + (tm->min * 60) + (tm->hour * 3600) +
                 (days_since_jan1 * 86400) + ((year - 70) * 31536000) +
                 (((year - 69) / 4) * 86400) -
                 (((year - 1) / 100) * 86400) +
                 (((year + 299) / 400) * 86400);

    /* Check that rtc_time is reasonable? */
    if ((tm->sec > 59 || tm->sec  < 0)  ||
       (tm->min  > 59 || tm->min  < 0)  ||
       (tm->hour > 23 || tm->hour < 0)  ||
       (tm->date > 31 || tm->date < 0)  ||
       (tm->mon  > 12 || tm->mon  < 0)  ||
       (tm->year > 99 || tm->year < 0)) {
           return 1;
    }

    return 0;
}

/* Convert from unix time to s3c2410 internal representation. */
static inline int
unix_to_rtc(uint64_t unix_time, struct rtc_time *tm)
{
    int days, years, leap_yrs;
    int i;

    days = unix_time / 86400;
    unix_time = unix_time % 86400;

    tm->hour = unix_time / 3600;
    unix_time = unix_time % 3600;

    tm->min = unix_time / 60;
    tm->sec = unix_time % 60;

    years = (days / 365) + 1970;
    days = days % 365;

    leap_yrs = leap_years(1970, years);
    days -= leap_yrs;

    /* If days is now negative then subtract a year. */
    if (days < 0) {
        years--;

        if (is_leap_year(years)) {
            days += 366;
        }
        else {
            days += 365;
        }
    }

    if (is_leap_year(years)){
        month2days_ptr = month2days_ly;
    } else {
        month2days_ptr = month2days;
    }

    /* What month are we in? */
    for (i = 0; i < 13; i++) {
        if (days <= month2days_ptr[i]) {
            break;
        }
    }

    tm->mon  = i;
    tm->date = days - month2days_ptr[i - 1];
    tm->year = years - CURRENT_CENTURY;

    /* s3c2410 device only supports a single century. */
    if (tm->year < 0 || tm->year >= 100) {
        tm->year = 0;
        return 1;
    }

    return 0;
}

#endif

