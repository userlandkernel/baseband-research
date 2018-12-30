/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   capability lists
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/print.h>
#include <kdb/tid_format.h>
#include <clist.h>
#include <tcb.h>

#if defined(CONFIG_KDB_CONS)

#if 0
int SECTION (SEC_KDEBUG)
print_cap (word_t val, word_t width, word_t precision, bool adjleft)
{
    int n = 0;
    spaceid_t sid;
    space_t * space;
    bool print_ptr = true;

    sid.set_raw (val);

    space = get_space_list()->lookup_space(sid);

    if (space == NULL)
        space = (space_t*)val;

    if (kdb_tid_format.X.human)
    {
#if defined(CONFIG_SPACE_NAMES)
        if (space->debug_name[0] != '\0') {
            n += print_string(space->debug_name, width, precision);
            print_ptr = false;
        }
#endif
    }
    if (print_ptr) {
        n += print_hex((word_t)space, width, precision, adjleft);
    }

    if (space) {
        val = space->get_space_id().get_spaceno();
        n += print_string("/");
        n += print_dec (val, 0);
    } else { 
        n += print_string("/-1");
    }
    
    return n;
}
#endif

DECLARE_CMD(cmd_list_clists, root, 'L', "listclists",  "list all capability lists");

CMD(cmd_list_clists, cg)
{
    word_t i = 0;
    clist_t *clist;

    printf("clist:  address, slots\n");
    while (get_clist_list()->is_valid(clistid(i))) {
        clist = get_clist_list()->lookup_clist(clistid(i));
        i++;

        if (!clist) {
            continue;
        }

        printf("%5d: %8p, %d\n", i-1, clist, clist->num_entries());
    }

    return CMD_NOQUIT;
}

DECLARE_CMD(cmd_show_clist, root, 'l', "showclist",  "dump clist contents");

CMD(cmd_show_clist, cg)
{
    word_t id = get_dec ("clistid", 0, "0");
    clist_t *clist;

    if (id == ABORT_MAGIC) {
        return CMD_NOQUIT;
    }

    if (!get_clist_list()->is_valid(clistid(id))) {
        printf("Invalid clist id %d\n", id);
        return CMD_NOQUIT;
    }

    clist = get_clist_list()->lookup_clist(clistid(id));
    if (!clist) {
        printf("Non existing clist %d\n", id);
        return CMD_NOQUIT;
    }

    printf("\nclist %8p: %d slots\n", clist, clist->num_entries());
    printf("index: type, tcb\n");

    bool null = false;
    for (word_t index = 0; index < clist->num_entries(); index++) {
        cap_t *cap = clist->lookup_cap(threadid_t::threadid(index, 1));
        if (cap) {
            printf("%5d: %s, %T\n", index,
                    cap->is_thread_cap() ? "T" :
                    cap->is_ipc_cap()    ? "I" : "?",
                    cap->get_tcb());
            null = false;
        } else {
            if (!null) {
                printf("...\n");
            }
            null = true;
        }
    }

    return CMD_NOQUIT;
}

#endif

