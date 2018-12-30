/*
 * Copyright (c) 2002-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
 * Description:   Implementation of printf
 */
#include <l4.h>
#include <stdarg.h>     /* for va_list, ... comes with gcc */

#include <sync.h>       /* spinlocks    */
#include <thread.h>
#include <tcb.h>
#include <linear_ptab.h>
#include <kdb/console.h>

#if defined(CONFIG_KDB_CONS)

int print_tcb (word_t val, word_t width, word_t precision, bool adjleft);
int print_space (word_t val, word_t width, word_t precision, bool adjleft);
int SECTION(SEC_KDEBUG) print_mutex(word_t val, word_t width,
        word_t precision, bool adjleft);

/* convert nibble to lowercase hex char */
#define hexchars(x) (((x) < 10) ? ('0' + (x)) : ('a' + ((x) - 10)))

/**
 *      Print hexadecimal value
 *
 *      @param val              value to print
 *      @param width            width in caracters
 *      @param precision        minimum number of digits to apprear
 *      @param adjleft          left adjust the value
 *      @param nullpad          pad with leading zeros (when right padding)
 *
 *      Prints a hexadecimal value with leading zeroes of given width
 *      using putc(), or if adjleft argument is given, print
 *      hexadecimal value with space padding to the right.
 *
 *      @returns the number of charaters printed (should be same as width).
 */
int SECTION(SEC_KDEBUG) print_dec(const word_t val,
                                  const int width,
                                  bool negative,
                                  const char pad);
int SECTION(SEC_KDEBUG) print_hex(const word_t val,
                                  int width,
                                  int precision,
                                  bool adjleft,
                                  bool nullpad);
#ifndef CONFIG_IS_64BIT
int SECTION(SEC_KDEBUG) print_dec64(const u64_t val,
                                  const int width = 0,
                                  bool adjleft = false,
                                  bool negative = false,
                                  bool minusspace = false,
                                  const char pad = ' ')
{
    u64_t divisor;
    int digits, extra;

    /* estimate number of spaces and digits */
    for (divisor = 1, digits = 1; (u64_t)(val/divisor) >= 10; divisor *= 10, digits++);

    if (negative) {
        digits++;
    }
    extra = 0;
    if (!adjleft) {
        /* print spaces */
        for ( ; digits < width; digits++ ) {
            extra ++; putc(pad);
        }
    }

    if (negative) {
        putc('-');
    } else if (minusspace) {
        putc(' ');
    }
    /* print digits */
    do {
        putc(((val/divisor) % 10) + '0');
    } while (divisor /= 10);

    if (adjleft) {
        /* print spaces */
        for ( ; digits < width; digits++ ) {
            extra ++; putc(pad);
        }
    }
    /* report number of chars printed */
    return digits + extra;
}

int SECTION(SEC_KDEBUG) print_hex64(const u64_t val,
                                  int width = 0,
                                  int precision = 0,
                                  bool adjleft = false,
                                  bool nullpad = false)
{
    int res = 0;
    u32_t high, low;

    high = val >> 32;
    low = (u32_t)val;

    if (high > 0) {
        res = print_hex(high, 0, 0, false, false);
        res += print_hex(low, sizeof(u32_t), 0, false, true);
    } else {
        res = print_hex(low, 0, 0, adjleft, nullpad);
    }
    return res;
}
#else
#define print_hex64 print_hex
#define print_dec64 print_dec
#endif

int SECTION(SEC_KDEBUG) print_hex(const word_t val,
                                  int width = 0,
                                  int precision = 0,
                                  bool adjleft = false,
                                  bool nullpad = false)
{
    long i, n = 0;
    long nwidth = 0;

    // Find width of hexnumber
    while ((val >> (4 * nwidth)) && (word_t) nwidth <  2 * sizeof (word_t))
        nwidth++;

    if (nwidth == 0)
        nwidth = 1;

    // May need to increase number of printed digits
    if (precision > nwidth)
        nwidth = precision;

    // May need to increase number of printed characters
    if (width == 0 && width < nwidth)
        width = nwidth;

    // Print number with padding
    if (! adjleft)
        for (i = width - nwidth; i > 0; i--, n++)
            putc (nullpad ? '0' : ' ');
    for (i = 4 * (nwidth - 1); i >= 0; i -= 4, n++)
        putc (hexchars ((val >> i) & 0xF));
    if (adjleft)
        for (i = width - nwidth; i > 0; i--, n++)
            putc (' ');

    return n;
}

/**
 *      Print a string
 *
 *      @param s        zero-terminated string to print
 *      @param width    minimum width of printed string
 *
 *      Prints the zero-terminated string using putc().  The printed
 *      string will be right padded with space to so that it will be
 *      at least WIDTH characters wide.
 *
 *      @returns the number of charaters printed.
 */
int SECTION(SEC_KDEBUG) print_string(const char * s,
                                     const int width = 0,
                                     const int precision = 0)
{
    int n = 0;

    for (;;)
    {
        if (*s == 0)
            break;

        putc(*s++);
        n++;
        if (precision && n >= precision)
            break;
    }

    while (n < width) { putc(' '); n++; }

    return n;
}

/**
 *      Print hexadecimal value with a separator
 *
 *      @param val      value to print
 *      @param bits     number of lower-most bits before which to
 *                      place the separator
 *      @param sep      the separator to print
 *
 *      @returns the number of charaters printed.
 */
int SECTION(SEC_KDEBUG) print_hex_sep(const word_t val,
                                      const int bits,
                                      const char *sep)
{
    int n = 0;

    n = print_hex(val >> bits, 0, 0);
    n += print_string(sep);
    n += print_hex(val & ((1UL << bits) - 1), 0, 0);

    return n;
}


/**
 *      Print decimal value
 *
 *      @param val      value to print
 *      @param width    width of field
 *      @param pad      character used for padding value up to width
 *
 *      Prints a value as a decimal in the given WIDTH with leading
 *      whitespaces.
 *
 *      @returns the number of characters printed (may be more than WIDTH)
 */
int SECTION(SEC_KDEBUG) print_dec(const word_t val,
                                  const int width = 0,
                                  bool adjleft = false,
                                  bool negative = false,
                                  bool minusspace = false,
                                  const char pad = ' ')
{
    word_t divisor;
    int digits, extra;

    /* estimate number of spaces and digits */
    for (divisor = 1, digits = 1; val/divisor >= 10; divisor *= 10, digits++);

    if (negative) {
        digits++;
    }
    extra = 0;
    if (!adjleft) {
        /* print spaces */
        for ( ; digits < width; digits++ ) {
            extra ++; putc(pad);
        }
    }

    if (negative) {
        putc('-');
    } else if (minusspace) {
        putc(' ');
    }
    /* print digits */
    do {
        putc(((val/divisor) % 10) + '0');
    } while (divisor /= 10);

    if (adjleft) {
        /* print spaces */
        for ( ; digits < width; digits++ ) {
            extra ++; putc(pad);
        }
    }
    /* report number of chars printed */
    return digits + extra;
}

#ifdef CONFIG_MUNITS

DEFINE_SPINLOCK(printf_spin_lock);

static word_t print_lock_holder = -1UL;
static int print_lock_count = 0;

void SECTION(SEC_KDEBUG)
print_lock()
{
    if (print_lock_holder != get_current_context().raw) {
        printf_spin_lock.lock();
        print_lock_holder = get_current_context().raw;
        SMT_ASSERT(ALWAYS, print_lock_count == 0);
    }
    print_lock_count++;
}

void SECTION(SEC_KDEBUG)
print_unlock()
{
    SMT_ASSERT(ALWAYS, print_lock_count > 0);
    SMT_ASSERT(ALWAYS, printf_spin_lock.is_locked(true));

    print_lock_count--;
    if (print_lock_count == 0) {
        print_lock_holder = get_current_context().root_context().raw;
        printf_spin_lock.unlock();
    }
}

#endif

/**
 *      Does the real printf work
 *
 *      @param format_p         pointer to format string
 *      @param args             list of arguments, variable length
 *
 *      Prints the given arguments as specified by the format string.
 *      Implements a subset of the well-known printf plus some L4-specifics.
 *
 *      @returns the number of characters printed
 */
int SECTION(SEC_KDEBUG) do_printf(const char* format_p, va_list args)
{
    const char* format = format_p;
    int n = 0;
    int width = 8;
    int precision = 0;
    bool adjleft = false, nullpad = false, minusspace = false;
    int is_l = 0;

#define arg(x) va_arg(args, x)

#ifdef CONFIG_MUNITS
    print_lock();
#endif

    /* sanity check */
    if (format == NULL)
    {
        n = 0;
        goto end;
    }

    while (*format)
    {
        switch (*(format))
        {
        case '%':
            is_l = 0;
            width = precision = 0;
            adjleft = nullpad = false;
        reentry:
            switch (*(++format))
            {
                /* modifiers */
            case '.':
                for (format++; *format >= '0' && *format <= '9'; format++)
                    precision = precision * 10 + (*format) - '0';
                if (*format == 'w')
                {
                    // Set precision to printsize of a hex word
                    precision = sizeof (word_t) * 2;
                    format++;
                }
                format--;
                goto reentry;
            case '0':
                nullpad = (width == 0);
                /* fall through */
            case '1': 
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                width = width*10 + (*format)-'0';
                goto reentry;
            case ' ':
                minusspace = true;
                goto reentry;
            case 'w':
                // Set width to printsize of a hex word
                width = sizeof (word_t) * 2;
                goto reentry;
            case '-':
                adjleft = true;
                goto reentry;
            case 'l':
                is_l++;
                goto reentry;
            case 'c':
                putc(arg(int));
                n++;
                break;
            case 'm':   /* microseconds */
            {
                /*lint -e571 flint doesn't deal with va_arg very well*/
                n += print_hex64(arg(u64_t), width, precision,
                               adjleft, nullpad);
                break;
            }
            case 'd':
            {
                long val = arg(long);
                bool negative = false;
                if (val < 0)
                {
                    negative = true;
                    val = -val;
                }
                n += print_dec(val, width, adjleft, negative, minusspace);
                break;
            }
            case 'u':
                if (is_l > 1) {
                    n += print_dec64(arg(u64_t), width, adjleft);
                } else {
                    n += print_dec(arg(long), width, adjleft);
                }
                break;
            case 'p':
                precision = sizeof (word_t) * 2;
                /* fall through */
            case 'x':
                if (is_l > 1) {
                    n += print_hex64(arg(long long), width, precision,
                                   adjleft, nullpad);
                } else {
                    n += print_hex(arg(long), width, precision,
                                   adjleft, nullpad);
                }
                break;
            case 's':
            {
                char* s = arg(char*);
                if (s)
                    n += print_string(s, width, precision);
                else
                    n += print_string("(null)", width, precision);
            }
            break;

            /* L4-specific format codes. */
            case 't':
            case 'T':
                n += print_tcb(arg (word_t), width, precision, adjleft);
                break;
            case 'S':
                n += print_space (arg (word_t), width, precision, adjleft);
                break;
            case 'M':
                n += print_mutex(arg(word_t), width, precision, adjleft);
                break;

            case '%':
                putc('%');
                n++;
                format++;
                continue;
            default:
                n += print_string("?");
                break;
            };
            break;
        default:
            putc(*format);
            n++;
            break;
        }
        format++;
    }

end:
#ifdef CONFIG_MUNITS
    print_unlock();
#endif
    return n;
}

/**
 *      Flexible print function
 *
 *      @param format   string containing formatting and parameter type
 *                      information
 *      @param ...      variable list of parameters
 *
 *      @returns the number of characters printed
 */
extern "C" int SECTION(SEC_KDEBUG) printf(const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = do_printf(format, args);
    va_end(args);
    return i;
};

#endif
