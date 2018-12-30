/*
 * Copyright (c) 2002-2003, Karlsruhe University
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
 * Copyright (c) 2005, National ICT Australia
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
 * Description:   Tracepoint interface
 */
#ifndef __KDB__TRACEPOINTS_H__
#define __KDB__TRACEPOINTS_H__

#include <kernel/l4.h>
#include <kernel/debug.h>
#include <kernel/kdb/macro_set.h>
#include <kernel/traceids.h>
#include <kernel/tracebuffer.h>
#include <kernel/mp.h>

// avoid including api/smp.h for non-SMP case
#ifndef CONFIG_MDOMAINS
# define TP_CPU 0
#else
# include <smp.h>
# if defined(CONFIG_ARCH_IA64)
#   define TP_CPU 0
# else
#   define TP_CPU get_current_context().domain
# endif
#endif

GLOBAL_DEC(bool,tracepoints_enabled)

class tracepoint_t
{
public:
    word_t      id;
    word_t      enabled;
    word_t      enter_kdb;
    word_t      counter[CONFIG_NUM_DOMAINS];

public:
    void reset_counter ()
        { for (int cpu = 0; cpu < CONFIG_NUM_DOMAINS; counter[cpu++] = 0); }
};


#if defined(CONFIG_TRACEPOINTS)

#define EXTERN_TRACEPOINT(tp)                           \
    extern tracepoint_t __tracepoint_##tp

/*
 * Wrapper class for accessing tracepoint set.
 */

class tracepoint_list_t
{
public:
    macro_set_t        *tp_set;

    inline void reset (void)
        { tp_set->reset (); }

    inline tracepoint_t * next (void)
        { return (tracepoint_t *) tp_set->next (); }

    inline word_t size (void)
        { return tp_set->size (); }

    inline tracepoint_t * get (word_t n)
        { return (tracepoint_t *) tp_set->get (n); }
};

extern tracepoint_list_t tp_list;

/* FIXME: belongs in the kdebug section */
#define DECLARE_TRACEPOINT(tp)                                  \
    tracepoint_t __tracepoint_##tp = { ((word_t)(tp)), 0, 0, { 0, } };      \
    PUT_SET (tracepoint_set, __tracepoint_##tp)

#define TRACEPOINT(tp, code...)                                 \
do {                                                            \
    TBUF_RECORD_EVENT ((word_t)tp);                                     \
    if (EXPECT_FALSE(GLOBAL(tracepoints_enabled)))              \
    {                                                           \
        __tracepoint_##tp.counter[TP_CPU]++;                    \
        if (__tracepoint_##tp.enabled & (1UL << TP_CPU))        \
        {                                                       \
            {code;}                                             \
            if (__tracepoint_##tp.enter_kdb & (1UL << TP_CPU))  \
                enter_kdebug (#tp);                             \
        }                                                       \
    }                                                           \
} while (0)

#define TRACEPOINT_TB(tp, code, tbs, tba...)                    \
do {                                                            \
    TBUF_RECORD ((word_t)tp, tbs, tba);                                 \
    if (EXPECT_FALSE(GLOBAL(tracepoints_enabled)))              \
    {                                                           \
        __tracepoint_##tp.counter[TP_CPU]++;                    \
        if (__tracepoint_##tp.enabled & (1UL << TP_CPU))        \
        {                                                       \
            {code;}                                             \
            if (__tracepoint_##tp.enter_kdb & (1UL << TP_CPU))  \
                enter_kdebug (#tp);                             \
        }                                                       \
    }                                                           \
} while (0)

#define ENABLE_TRACEPOINT(tp, kdb)              \
do {                                            \
    __tracepoint_##tp.enabled = ~0UL;           \
    __tracepoint_##tp.enter_kdb = kdb;          \
} while (0)

#define TRACEPOINT_ENTERS_KDB(tp)               \
   (__tracepoint_##tp.enter_kdb)


#else /* !CONFIG_TRACEPOINTS */

#define EXTERN_TRACEPOINT(tp)

#define DECLARE_TRACEPOINT(tp)

#define TRACEPOINT(tp, code...)                

#if defined(CONFIG_TRACEBUFFER)
#define TRACEPOINT_TB(tp, code, tbs, tba...)    \
do {                                            \
    TBUF_RECORD ((word_t)tp, tbs, tba);         \
} while (0)
#else
#define TRACEPOINT_TB(tp, code, tbs, tba...) 
#endif

#define ENABLE_TRACEPOINT(tp, kdb)
#define TRACEPOINT_ENTERS_KDB(tp) (0)

#endif


#endif /* !__KDB__TRACEPOINTS_H__ */
