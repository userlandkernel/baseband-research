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

#ifndef _LIBTIME_H
#define _LIBTIME_H

/*
 * Author: Carlos Dyonisio <medaglia@ok-labs.com>
 */

#include <stdlib.h>

static inline int
is_leap_year(int year)
{
    if ((year % 4) == 0) {
        if ((year % 100) == 0) {
            if ((year % 400) == 0) {
                return 1;
            } else {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

static inline int
get_unix_years(long *unix_days)
{
    int cur_year = 1970;
    register int year_days = 0;
    int is_negative = 0;

    if (*unix_days < 0) {
        is_negative = 1;
        cur_year--;
    }

    while (1) {
        if (is_leap_year(cur_year)) {
            year_days = 365;
        } else {
            year_days = 364;
        }

        if (labs(*unix_days) > year_days) {
            if (is_negative) {
                cur_year--;
                *unix_days += (year_days + 1);
            } else {
                cur_year++;
                *unix_days -= (year_days + 1);
            }
        } else {
            return (cur_year - 1970);
        }
    }
}

static inline int
get_year_month(int *year_days, int leap_year)
{
    int months_leap[] = { 30, 28, 30, 29, 30, 29, 30, 30, 29, 30, 29, 30 };
    int months_no_leap[] = { 30, 27, 30, 29, 30, 29, 30, 30, 29, 30, 29, 30 };
    int *month_days;
    int i;

    if (leap_year) {
        month_days = months_leap;
    } else {
        month_days = months_no_leap;
    }

    i = 0;
    while (1) {
        if (*year_days > month_days[i]) {
            *year_days -= (month_days[i] + 1);
        } else {
            *year_days += 1;
            return i;
        }
        i++;
    }
}

static inline int
get_week_day(long unix_days)
{
    int wd_tmp; /* [0-6] - 0 = Thu */

    if (unix_days < 0) {
        unix_days = labs(unix_days);

        wd_tmp = unix_days % 7;

        if (wd_tmp > 3) {
            wd_tmp -= 4;
        } else {
            wd_tmp += 3;
        }
        wd_tmp = 6 - wd_tmp;
    } else {
        wd_tmp = unix_days % 7;

        if (wd_tmp > 2) {
            wd_tmp -= 3;
        } else {
            wd_tmp += 4;
        }
    }

    return wd_tmp;
}

static inline time_t
get_year_seconds(int years, int start_year)
{
    time_t sec = 0;

    for (; years > 0; years--) {
        if (is_leap_year(start_year++)) {
            sec += (time_t)31622400;
        } else {
            sec += (time_t)31536000;
        }
    }
    return sec;
}

static inline int
get_yday(struct tm *pt)
{
    int months_leap[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int months_no_leap[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    register int is_leap;
    register int month;
    register int yday = 0;

    month = pt->tm_mon;

    is_leap = is_leap_year((1900 + pt->tm_year));

    while (month--) {
        if (is_leap) {
            yday += months_leap[month];
        } else {
            yday += months_no_leap[month];
        }
    }
    yday += (pt->tm_mday - 1);

    return yday;
}

static inline int
tm_out_of_range(struct tm *pt)
{
    if (pt->tm_sec > 60 || pt->tm_sec < 0) {
        return 1;
    }
    if (pt->tm_min > 59 || pt->tm_min < 0) {
        return 1;
    }
    if (pt->tm_hour > 23 || pt->tm_hour < 0) {
        return 1;
    }
    if (pt->tm_mday > 31 || pt->tm_mday < 1) {
        return 1;
    }
    if (pt->tm_mon > 11 || pt->tm_mon < 0) {
        return 1;
    }
    if (pt->tm_wday > 6 || pt->tm_wday < 0) {
        return 1;
    }
    /*
     * Maybe we should check if it is leap year and then
     * check the range. I don't think it is a big deal, anyway. (CD)
     */
    if (pt->tm_yday > 365 || pt->tm_yday < 0) {
        return 1;
    }
    return 0;
}

static inline char *
get_am_or_pm(const struct tm *pt)
{
    if (pt->tm_hour > 11) {
        return "PM";
    } else {
        return "AM";
    }
}

static inline int
get_hour_in_12(const struct tm *pt)
{
    if (pt->tm_hour == 0 || pt->tm_hour == 12) {
        return 12;
    } else {
        return pt->tm_hour % 12;
    }
}

static char wday_abrv_names[8][4] = {"Sun", "Mon", "Tue", "Wed",
                     "Thu", "Fri", "Sat"};
static char wday_names[8][10] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                     "Thursday", "Friday", "Saturday"};

static inline char *
get_locale_abrv_wday_name(const struct tm *pt) {
    /* XXX: Using C locale */
    return wday_abrv_names[pt->tm_wday];
}

static inline char *
get_locale_wday_name(const struct tm *pt) {
    /* XXX: Using C locale */
    return wday_names[pt->tm_wday];
}

static char mon_abrv_names[13][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; 
static char mon_names[13][10] = {"January", "February", "March", "April",
                        "May", "June", "July", "August", "September",
                        "October", "November", "December"};

static inline char *
get_locale_abrv_mon_name(const struct tm *pt) {
    /* XXX: Using C locale */
    return mon_abrv_names[pt->tm_mon];
}

static inline char *
get_locale_mon_name(const struct tm *pt) {
    /* XXX: Using C locale */
    return mon_names[pt->tm_mon];
}

static inline int
get_iso_year(const struct tm *pt)
{
    int limit;

    if (pt->tm_yday > 2 && pt->tm_yday < 362) {
        return 1900 + pt->tm_year;
    }

    if (pt->tm_yday < 3) {
        if (pt->tm_wday == 0 || (pt->tm_yday - pt->tm_wday) < -4) {
            return (1900 + pt->tm_year) - 1;
        } else {
            return 1900 + pt->tm_year;
        }
    } else {
        if (is_leap_year(1900 + pt->tm_year)) {
            limit = 362;
        } else {
            limit = 361;
        }

        if (pt->tm_wday == 0 || (pt->tm_yday - pt->tm_wday) < limit) {
            return 1900 + pt->tm_year;
        } else {
            return (1900 + pt->tm_year) + 1;
        }
    }
}

static inline int
get_week_number(const struct tm *pt)
{
    int week, yday;

    yday = pt->tm_yday;

    if (yday < pt->tm_wday) {
        return 0;
    }

    yday -= pt->tm_wday;
    week = 0;
    while (1) {
        if (yday < 0) {
            break;
        }
        yday -= 7;
        week++;
    }

    return week;
}

static inline int
get_week_number_monday(const struct tm *pt)
{
    int week, yday, wday;
    int i;

    yday = pt->tm_yday;
    wday = pt->tm_wday == 0 ? 6 : pt->tm_wday - 1;
    week = 0;


    if (yday < wday) {
        return 0;
    }

    yday -= wday;
    i = 1;
    while (1) {
        if (yday < 0) {
            break;
        }
        yday -= 7;
        week++;
    }

    return week;
}

static inline int
get_iso_week_number(const struct tm *pt)
{
    register int week, yday;
    int wday;
    struct tm t;
    int i;

    yday = pt->tm_yday;
    wday = pt->tm_wday == 0 ? 6 : pt->tm_wday - 1;
    week = 1;

    if (yday < wday) {
        yday -= wday;
        if (yday < -3) {
            t.tm_wday = pt->tm_wday == 0 ? 6 : pt->tm_wday - 1;
            t.tm_year = pt->tm_year - 1;
            if (is_leap_year(1900 + t.tm_year)) {
                t.tm_yday = 365;
            } else {
                t.tm_yday = 364;
            }
            return get_iso_week_number(&t);
        } else {
            return 1;
        }
    }

    yday -= wday;
    i = 1;
    while (1) {
        if (yday < 0) {
            break;
        }
        yday -= 7;
        week++;
    }
    if (yday < -3) {
        week--;
    }

    return week;
}

#endif /* _LIBTIME_H */
