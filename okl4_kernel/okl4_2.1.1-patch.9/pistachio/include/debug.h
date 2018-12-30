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
 * Description:   Debug functions
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

/* FIXME: does this go here? */
#if defined(__APPLE_CC__)
#define SEC_KDEBUG "__KDEBUG, __kdebug"
#define SEC_KDEBUG_DATA "__KDEBUG, __kdebug.data"
#else
#define SEC_KDEBUG ".kdebug"
#define SEC_KDEBUG_DATA ".kdebug.data"
#endif

/* XXX? we use SECTION(SEC_INIT) below... */
#include <kernel/init.h>
#include <kernel/macros.h>

/*
 * Kernel debugger features:
 * These are indicated in kip->kdebug_config[0]
 */
#define KDB_FEATURE_CLI          (1UL << 0)
#define KDB_FEATURE_CONSOLE      (1UL << 1)
#define KDB_FEATURE_THREADNAMES  (1UL << 8)
#define KDB_FEATURE_SPACENAMES   (1UL << 9)
#define KDB_FEATURE_MUTEX_NAMES  (1UL << 10)

/**
 * Maximum Length of human-readable object names.
 */
#define MAX_DEBUG_NAME_LENGTH    16

/*
 * Escape codes for controlling text color, brightness, etc.
 */

#if defined(CONFIG_KDB_COLOR_VT)

#define TXT_CLRSCREEN           "\033[2J"
#define TXT_NORMAL              "\033[0m"
#define TXT_BRIGHT              "\033[1m"
#define TXT_REVERSED            "\033[7m"
#define TXT_FG_BLACK            "\033[30m"
#define TXT_FG_RED              "\033[31m"
#define TXT_FG_GREEN            "\033[32m"
#define TXT_FG_YELLOW           "\033[33m"
#define TXT_FG_BLUE             "\033[34m"
#define TXT_FG_MAGENTA          "\033[35m"
#define TXT_FG_CYAN             "\033[36m"
#define TXT_FG_WHITE            "\033[37m"
#define TXT_BG_BLACK            "\033[40m"
#define TXT_BG_RED              "\033[41m"
#define TXT_BG_GREEN            "\033[42m"
#define TXT_BG_YELLOW           "\033[43m"
#define TXT_BG_BLUE             "\033[44m"
#define TXT_BG_MAGENTA          "\033[45m"
#define TXT_BG_CYAN             "\033[46m"
#define TXT_BG_WHITE            "\033[47m"

#else

#define TXT_CLRSCREEN           ""
#define TXT_NORMAL              ""
#define TXT_BRIGHT              ""
#define TXT_REVERSED            ""
#define TXT_FG_BLACK            ""
#define TXT_FG_RED              ""
#define TXT_FG_GREEN            ""
#define TXT_FG_YELLOW           ""
#define TXT_FG_BLUE             ""
#define TXT_FG_MAGENTA          ""
#define TXT_FG_CYAN             ""
#define TXT_FG_WHITE            ""
#define TXT_BG_BLACK            ""
#define TXT_BG_RED              ""
#define TXT_BG_GREEN            ""
#define TXT_BG_YELLOW           ""
#define TXT_BG_BLUE             ""
#define TXT_BG_MAGENTA          ""
#define TXT_BG_CYAN             ""
#define TXT_BG_WHITE            ""

#endif

/*
 * Assert Levels
 */
#define REGRESSION      4
#define DEBUG           3
#define NORMAL          2
#define PRODUCTION      1
#define ALWAYS          0

#if defined(_lint)
/* Turn on all debug macros when linting */
#define CONFIG_DEBUG
#undef CONFIG_KDB_NO_ASSERTS
#endif

#if defined(CONFIG_DEBUG)

#include <kernel/arch/debug.h>

#if defined (CONFIG_MUNITS)
void print_lock();
void print_unlock();
#endif

# define UNIMPLEMENTED()                                        \
    do {                                                        \
        printf ("\nNot implemented: %s\n%s, line %d\n",         \
                __PRETTY_FUNCTION__, __FILE__, __LINE__);       \
        enter_kdebug ("unimplemented");                         \
        __panic();                                              \
    } while (false)

#define panic(x...)                                     \
do {                                                    \
    printf ("PANIC in %s, %s, line %d:\n===> ",         \
            __PRETTY_FUNCTION__, __FILE__, __LINE__);   \
    printf (x);                                         \
    __panic();                                          \
} while (false)

#if !defined(CONFIG_KDB_NO_ASSERTS)

#if defined(_lint)
#define ASSERT(level, x) do { /*lint -e506*/ if (! (x)) { __panic(); } /*lint +e506*/} while(false)
#else

/* This hack allows us to get __LINE__ as a string. The obvious
 * solution "#__LINE__" doesn't work, so we need to double-wrap
 * the call using these two macros. */
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define ASSERT(level, x)                                                \
    do {                                                                \
        if ((level <= CONFIG_ASSERT_LEVEL) && EXPECT_FALSE(! (x))) {    \
            __assert_failure("Assertion \"" #x "\" failed in file '"    \
                    __FILE__ "', line " TO_STRING(__LINE__));           \
        }                                                               \
    } while(false)
#endif

#define WARNING(fmt, args...)                                           \
    do {                                                                \
        printf ("WARNING: %s, line %d (fn=%p)\n===> " fmt,              \
                __FILE__, __LINE__, __return_address() , ## args);      \
        enter_kdebug ("warning");                                       \
    } while (false)

#define TRACEF(f, x...)                                         \
    do {                                                        \
        printf ("%s:%d: " f, __FUNCTION__, __LINE__ ,##x);      \
    } while(false)

#define TRACE(x...)    printf(x)

# else /* defined(CONFIG_KDB_NO_ASSERTS) */

#define ASSERT(level, x)        do { } while (false)
#define WARNING(fmt...)         do { } while (false)
#define ERROR(fmt...)           __panic();
#define TRACE(x...)             do { } while (false)
#define TRACEF(x...)            do { } while (false)

# endif

#if !defined(ASSEMBLY)

/* From kdb/generic/entry.cc */
void kdebug_entry (void *);

#endif /* ! ASSEMBLY */

#ifdef CONFIG_MUNITS
#define SMT_ASSERT ASSERT
#else
#define SMT_ASSERT(level, x)
#endif

#else /* !CONFIG_DEBUG */

/*
 * Turn off all config options.
 */

#undef CONFIG_KDB_CLI
#undef CONFIG_KDB_CONS
#undef CONFIG_KMEM_TRACE
#undef CONFIG_TRACEPOINTS
#undef CONFIG_THREAD_NAMES
#undef CONFIG_SPACE_NAMES
#undef CONFIG_MUTEX_NAMES

/*
 * Define all functions as empty.
 */

/* Functions that stop the kernel */
INLINE void NORETURN enter_kdebug(char *s) 
{
    do { } while (true);
}

# define UNIMPLEMENTED()        do { } while (true)
# define panic(x...)            do { } while (true)

# define ERROR(fmt, args...)    do { } while (true)
# define spin_forever(x...)     do { } while (true)

/* Functions that don't stop the kernel */
# define ASSERT(level, x)       do { } while (false)
# define SMT_ASSERT(level, x)   do { } while (false)
# define WARNING(fmt, args...)  do { } while (false)
# define TRACE(x...)            do { } while (false)
# define TRACEF(x...)           do { } while (false)
# define spin(x...)             do { } while (false)

#endif /* CONFIG_DEBUG */

# if !defined(ASSEMBLY)

extern "C" int printf (const char * format, ...);

# endif /* ! ASSEMBLY */

/* Kernel console support */
#if defined(CONFIG_KDB_CONS)

void init_console (void) SECTION (SEC_INIT);

#else /*!CONFIG_KDB_CONS*/

# define init_console()
# define printf(fmt, ...)       do { if (0) { printf(fmt, ## __VA_ARGS__); } } while (false)

# undef CONFIG_VERBOSE_INIT

#endif /*CONFIG_KDB_CONS*/

/* Disabled Command Line Interface if no console/output device */
#if !(defined(CONFIG_KDB_CONS) /* || KDB GDB? */)
# undef CONFIG_KDB_CLI
#endif

# define TID(x)         ((x).get_raw())


/*lint -function(abort,__panic) */
extern void NORETURN __panic(void);

/* Called to inform the user of an assertion failure.  */
void __assert_failure(const char *message);

#if !defined(ASSEMBLY)
typedef void (* kdebug_init_t)(void);
typedef void (* kdebug_entry_t)(void *);

/* KDEBUG entry points */
struct kdebug_handle_t {
    kdebug_init_t  kdebug_init;
    kdebug_entry_t kdebug_entry;
};

extern kdebug_handle_t kdebug_entries;
 
void init_hello();
#endif /* !ASSEMBLY */

/*
 * Verbose initialization.
 */

#if defined(CONFIG_VERBOSE_INIT)
# define TRACE_INIT(x...)       printf (x)
#else
#if defined(_lint)
# define TRACE_INIT(x...)   \
    do {                    \
        printf (##x);       \
    } while(false)
#else
# define TRACE_INIT(x...)
#endif
#endif
 
#define NULL_CHECK(val) ASSERT(DEBUG, (val) != NULL)

#endif /* !__DEBUG_H__ */
