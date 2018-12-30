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
 * Description:   Version 4 specific input functions
 */
#include <l4.h>
#include <debug.h>
#include <kdb/input.h>
#include <kdb/kdb.h>
#include <kdb/console.h>
#include <space.h>
#include <tcb.h>

#if defined(CONFIG_KDB_CONS)

#if defined(CONFIG_THREAD_NAMES)
static int SECTION (SEC_KDEBUG)
strncmp(const char * s1, const char * s2, int n)
{
    int i;
    int diff;

    for (i = 0; i < n; i++) {
        diff = ((unsigned char *) s1)[i] - ((unsigned char *) s2)[i];
        if (diff != 0 || s1[i] == '\0')
            return diff;
    }
    return 0;
}
#endif

static bool str2int(const char *str, int size, word_t *value, word_t base = 16)
{
    bool is_num = true;
    word_t val = 0;
    for (int i=0; i < size; i++) {
        word_t a = str[i] - '0';
        word_t b = str[i] - 'a';
        word_t c = str[i] - 'A';
        word_t digit = 0;

        val *= base;
        if (a < 10) {
            digit = a;
        } else if (b < 6) {
            digit = b + 10;
        } else if (c < 6) {
            digit = c + 10;
        } else {
            is_num = false;
            break;
        }
        if (digit >= base) {
            is_num = false;
        }
        val += digit;
    }
    *value = val;
    return is_num;
}

static int SECTION (SEC_KDEBUG) thread_match(const char * str, word_t size, tcb_t **tcb)
{
    int found = 0;
    word_t val;
    bool is_num = false;

    *tcb = NULL;

    /* Maybe it is an id (either TID or TCB) */
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        is_num = str2int(&str[2], size-2, &val);
    } else {
        is_num = str2int(str, size, &val);
    }

    if (is_num) {
        *tcb = get_tcb((addr_t)val);

        if (*tcb) {
            found = 1;
        } else {
            *tcb = get_root_clist()->lookup_ipc_cap_locked(threadid(val));
            if (*tcb) {
                if (((*tcb)->myself_global == threadid(val))) {
                    found = 1;
                }
                (*tcb)->unlock_read();
            }
        }
    } else {
#if defined(CONFIG_THREAD_NAMES)
        present_list_lock.lock();
        tcb_t* walk = global_present_list;

        do {
            /* Only compare first size characters */
            if (strncmp(str, walk->debug_name, size) == 0) {
                found = 1;
                *tcb = walk;
                break;
            }
            walk = walk->present_list.next;
        } while (walk != global_present_list);
        present_list_lock.unlock();
#endif
    }
    return found;
}

static int SECTION (SEC_KDEBUG) space_match(const char * str, word_t size, space_t **space)
{
    int found = 0;
    word_t val;
    bool is_num = false;

    *space = NULL;

    /* Maybe it is a space_id */
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        is_num = str2int(&str[2], size-2, &val);
    } else {
        /* read space_id as decimal */
        is_num = str2int(str, size, &val, 10);
    }

    if (is_num) {
        /* try lookup space_id */
        *space = get_space_list()->lookup_space(spaceid(val));

        if (*space == NULL) {
            /* not a space_id, is it a pointer? */
            *space = (space_t *)val;
        }
        found = 1;
    } else {
#if defined(CONFIG_SPACE_NAMES)
        spaces_list_lock.lock();
        space_t * walk = global_spaces_list;

        do {
            /* Only compare first size characters */
            if (strncmp(str, walk->debug_name, size) == 0) {
                found = 1;
                *space = walk;
                break;
            }

            walk = walk->get_spaces_list().next;
        } while (walk != global_spaces_list);

        spaces_list_lock.unlock();
#endif
    }
    return found;
}
/**
 * Prompt for an address space using PROMPT and return the space
 * pointer.  Input value can be a TCB address or a physical or virtual
 * space pointer.

 * @param prompt        prompt
 *
 * @return pointer to space
 */
space_t SECTION(SEC_KDEBUG) * get_space (const char * prompt)
{
    char buf[20];
    space_t *space = NULL;

    int cnt = 0;
    char c;
    /* Zero out string */
    for (int i=0; i < 20; i++) {
        buf[i] = '\0';
    }
    printf ("%s [current]: ", prompt);

    /* Get a string */
    while (1) {
        c = getc();
        if (c == KEY_BS) {
            if (cnt > 0) {
                putc(c);
                putc(' ');
                putc(c);
                buf[cnt--] = '\0';
            }
        } else if (c == KEY_RETURN) {
            break;
        } else {
            buf[cnt++] = c;
            putc(c);

            if (cnt == 20)
                break;
        }
    }

    if (cnt == 0) {
        /* Just hit enter -- current */
        printf ("current");
        space = kdb.kdb_current->get_space();
    } else {
        /* Check valid space address */
        if (!space_match(buf, cnt, &space))
            space = NULL;
    }

    printf ("\n");
    return space ? space : get_kernel_space();
}


/**
 * Prompt for a thread using PROMPT and return the tcb pointer.  Input
 * value can be a TCB address od a thread id.
 *
 * @param prompt        prompt
 *
 * @return pointer to tcb
 */
tcb_t SECTION (SEC_KDEBUG) * get_thread (const char * prompt)
{
    char buf[20];
    bool valid_id = 0;
    tcb_t *tcb;

    int cnt = 0;
    char c;
    /* Zero out string */
    for (int i=0; i < 20; i++) {
        buf[i] = '\0';
    }
    printf ("%s [current]: ", prompt);

    /* Get a string */
    while (1) {
        c = getc();
        if (c == KEY_BS) {
            if (cnt > 0) {
                putc(c);
                putc(' ');
                putc(c);
                buf[cnt--] = '\0';
            }
        } else if (c == KEY_RETURN) {
            break;
        } else {
            buf[cnt++] = c;
            putc(c);

            if (cnt == 20)
                break;
        }
    }

    if (cnt == 0) {
        /* Just hit enter -- current */
        printf ("current");
        tcb = kdb.kdb_current;
        valid_id = 1;
    } else {
        /* Check valid thread name */
        if (thread_match(buf, cnt, &tcb)) {
            valid_id = 1;
        }
    }

    if (valid_id == 0) {
        printf("\nNo thread named: %s\n", buf);
        return NULL;
    }

    printf ("\n");
    return tcb;
}

#else

tcb_t SECTION (SEC_KDEBUG) * get_thread (const char * prompt)
{
    return NULL;
}

space_t SECTION(SEC_KDEBUG) * get_space (const char * prompt)
{
    return NULL;
}

#endif
