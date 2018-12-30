/*
 * Copyright (c) 2004, Karlsruhe University
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
 * Description:   Kdebug stuff for V4 threads
 */
#include <l4.h>
#include <kdb/tid_format.h>
#include <thread.h>
#include <tcb.h>


#if defined(CONFIG_KDB_CONS)

/* From generic/print.cc */

int print_hex (const word_t val,
               int width,
               int precision,
               bool adjleft = false,
               bool nullpad = false);
int print_string (const char * s,
                  const int width = 0,
                  const int precision = 0);
int print_hex_sep (const word_t val,
                   const int bits,
                   const char *sep);
int print_dec (const word_t val,
               const int width = 0,
               const char pad = ' ');


static int SECTION (SEC_KDEBUG)
print_tid(threadid_t tid, word_t width, word_t precision, bool adjleft)
{
    /* If the TID has a human-readable name, use that. */
    if (kdb_tid_format.X.human) {
        if (tid.is_nilthread()) {
            return print_string("NIL_THRD", width, precision);
        }

        if (tid.is_anythread()) {
            return print_string("ANY_THRD", width, precision);
        }

        if (tid.is_waitnotify()) {
            return print_string("WAIT_NOTIFY", width, precision);
        }

#if defined(CONFIG_THREAD_NAMES)
        /* Are we the idle thread? */
        if (tid == get_idle_tcb()->get_global_id()) {
            return print_string("idle_thread", width, precision);
        }

        if (tid.is_threadhandle()) {
            tcb_t *tcb = lookup_tcb_by_handle_locked(tid.get_raw());
            if (tcb != NULL) {
                tcb->unlock_read();
                if (tcb->debug_name[0] != '\0') {
                    word_t n = print_string("R-", 2, 0);
                    n += print_string(tcb->debug_name, width, precision);
                    return n;
                }
            }
        }

        /* If the thread has a debugging name, print it out. */
        tcb_t * tcb = get_current_clist()->lookup_ipc_cap_locked(tid);
        if (tcb != NULL) {
            tcb->unlock_read();
            if (tcb->debug_name[0] != '\0') {
                return print_string(tcb->debug_name, width, precision);
            }
        }
#endif
    }

    /* We're dealing with something which does not have a name set. */
    word_t n = 0;

    if (tid.is_threadhandle()) {
        tcb_t *tcb = lookup_tcb_by_handle_locked(tid.get_raw());
        if (tcb != NULL) {
            n += print_string("R-", 2, 0);
            tid = tcb->get_global_id();
            tcb->unlock_read();
        }
    }

    bool f_both = (word_t)TID_FORMAT_VALUE_BOTH == kdb_tid_format.X.value;
    bool f_gid = (word_t)TID_FORMAT_VALUE_GID == kdb_tid_format.X.value;
    bool f_tcb = (word_t)TID_FORMAT_VALUE_TCB == kdb_tid_format.X.value;
    bool f_ver = (word_t)TID_FORMAT_VERSION_OFF != kdb_tid_format.X.version;

    if (f_gid || f_both)
    {
        // Getting a consistent output width with separators is pretty
        // much hopeless depending on the position of the separator,
        // additional hex characters become necessary

        if ((word_t)TID_FORMAT_VERSION_INLINE == kdb_tid_format.X.version)
        {
            // Do not separate version from threadno
            if (kdb_tid_format.X.sep != 0)
                // Insert a separator into threadno
                n += print_hex_sep (tid.get_raw (),
                                   kdb_tid_format.X.sep +
                                   L4_GLOBAL_VERSION_BITS, ".");
            else
                // No separator at all
                n += print_hex (tid.get_raw (), 0, sizeof (word_t) * 2);
        }
        else
        {
            if (kdb_tid_format.X.sep != 0)
                // Insert a separator into threadno
                n += print_hex_sep (tid.get_threadno (),
                                   kdb_tid_format.X.sep, ".");
            else
                // Print threadno without separator
                n += print_hex (tid.get_threadno (),
                               f_both || f_ver ? 0 : width,
                               0, adjleft);

            if (f_ver)
            {
                // Add a separator between threadno and version
                n += print_string ("v");
                width -= width > n ? n : 0;
                n += print_hex (tid.get_version (),
                                f_both ? 0 : width, 0, true);
            }
        }
    }

    if (f_both)
        n += print_string ("/");

    if (f_tcb || f_both) {
        /* Print a plain hexadecimal address. */
        n += print_hex((word_t)tid.get_raw(), 0, sizeof (word_t) * 2);
    }
    return n;
}

int SECTION (SEC_KDEBUG)
print_tcb(word_t val, word_t width, word_t precision, bool adjleft)
{
    tcb_t * tcb;
    threadid_t tid;

    /* Get TID value of 'val'. */
    tid.set_raw(val);

    /* Determine if the given val is a magic thread ID. */
    if (tid.is_nilthread()
            || tid.is_anythread()
            || tid.is_waitnotify()) {
        return print_tid(tid, width, precision, adjleft);
    }

    if (tid.is_threadhandle()) {
        tcb = lookup_tcb_by_handle_locked(tid.get_raw());
        if (tcb != NULL) {
            tcb->unlock_read();
            return print_tid(tid, width, precision, adjleft);
        }
    }

    /* Is the given value a thread's ID? */
    tcb = get_current_clist()->lookup_ipc_cap_locked(tid);
    if (tcb != NULL) {
        tcb->unlock_read();
        return print_tid(tid, width, precision, adjleft);
    }

    /* Is 'val' a pointer to 'idle_tcb'? */
    if (val == (word_t)get_idle_tcb()) {
        return print_tid(get_idle_tcb()->get_global_id(),
                width, precision, adjleft);
    }

    /* TCB may be a pointer to the middle of a tcb_t structure. Search through
     * all the TCBs in the system to see if it is. This is a slow operation, so
     * we avoid doing it until last. */
    tcb = get_tcb((addr_t)val);
    if (tcb != NULL) {
        return print_tid(tcb->get_global_id(), width, precision, adjleft);
    }

    /* Just use 'val' directly as a tid. */
    return print_tid(tid, width, precision, adjleft);
}

#endif
