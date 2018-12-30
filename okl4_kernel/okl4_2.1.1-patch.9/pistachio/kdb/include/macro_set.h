/*
 * Copyright (c) 2006 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   Generic sets defined at link time
 */
#ifndef __KDB__MACRO_SET_H__
#define __KDB__MACRO_SET_H__

class macro_set_entry_t;

/**
 * macro_set_t: A macro set contains a number of set entries defined
 * at build time.  These entries can be iterated over using the reset()
 * and next() methods.
 */
class macro_set_t
{
public:
    macro_set_entry_t   **list;
    word_t              *entries;
    word_t              curidx;

    void print (void);
    void reset (void);
    addr_t next (void);
    word_t size (void);
    addr_t get (word_t n);
};


/**
 * macro_set_entry_t: Macro set entry types are opaque.  The entries
 * themselves are retrieved using the get_entry() method.
 */
class macro_set_entry_t
{
public:
    macro_set_t *set;
    addr_t              entry;

    inline macro_set_t * get_set (void) { return set; }
    inline addr_t get_entry (void) { return entry; }
};



/**
 * DECLARE_SET: Declares a new set and ensures that it is initialized
 * upon startup.
 *
 * The build system generates a C file from all the PUT_SET calls assocated
 * with this set which is linked in later.
 *
 * We need a CPP_ONLY declare flag so empty sets will work (eg. compile
 * them all out, or things like kdb_initfuncs which nobody has found
 * a use for yet :)
 */
#ifdef CPP_ONLY
#define DECLARE_SET(name) MACRO_SET name SYM DECLARE TES_ORCAM
#else
#define DECLARE_SET(name)                                               \
    extern macro_set_entry_t *__macro_set_##name##_array[];             \
    extern word_t __macro_set_##name##_count;                           \
    macro_set_t SECTION(SEC_KDEBUG_DATA) name = { __macro_set_##name##_array, \
                                                  &__macro_set_##name##_count, \
                                                  0 }
#endif

/**
 * PUT_SET: Puts an object into the given set.  The object can be of
 * any type.  It is up to the user to ensure correct use of types.
 *
 * The build system processes these macros to generate tables in a C file.
 */
#ifdef CPP_ONLY
#define PUT_SET(set, sym) MACRO_SET set SYM __setentry_##set##_##sym TES_ORCAM
#else
#define PUT_SET(set, sym)                               \
    extern macro_set_t set;                             \
    macro_set_entry_t __setentry_##set##_##sym          \
    SECTION(SEC_KDEBUG_DATA) = { &set, (addr_t)&sym }
#endif


#endif /* !__KDB__MACRO_SET_H__ */
