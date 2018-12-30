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
/*
 * Description:   address space lists
 */
#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/print.h>
#include <kdb/tid_format.h>
#include <space.h>
#include <tcb.h>

#if defined(CONFIG_KDB_CONS)

int SECTION (SEC_KDEBUG)
print_space (word_t val, word_t width, word_t precision, bool adjleft)
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
        n += printf("%d", val);
    } else { 
        n += print_string("/-1");
    }

    return n;
}

DECLARE_CMD(cmd_list_spaces, root, 'S', "listspaces",  "list all address spaces");

CMD(cmd_list_spaces, cg)
{
    spaces_list_lock.lock();
    space_t * walk = global_spaces_list;

    do {
        tcb_t * tcb_walk = walk->get_thread_list();

        printf("%S:", walk);
#ifdef CONFIG_MAX_NUM_ASIDS
        asid_t *asid = walk->get_asid();
        if (asid->is_valid())
            printf("(%3d)", (int)asid->get(walk));
        else
            printf("(###)");
#endif

        if (tcb_walk) {
            do {
#if !defined(CONFIG_MDOMAINS)
                printf(tcb_walk->ready_list.is_queued() ?
                        " %.wt" : " (%.wt)", tcb_walk);
#else
                printf(tcb_walk->ready_list.is_queued() ?
                        " %t:%d" : " (%t:%d)", tcb_walk,
                        tcb_walk->get_context().domain);
#endif
                tcb_walk = tcb_walk->thread_list.next;
            } while ((tcb_walk != walk->get_thread_list()));
        }

        printf("\n");
        walk = walk->get_spaces_list().next;
    } while (walk != global_spaces_list);

    spaces_list_lock.unlock();
    return CMD_NOQUIT;
}

DECLARE_CMD(cmd_show_space, root, 's', "showspace",  "show space info");

CMD(cmd_show_space, cg)
{
    space_t *space;

    space = get_space ("Space");

    if (space == NULL) {
        return CMD_NOQUIT;
    }

    printf("=== %s%sSPACE: %p == ID: %d == THREADS: %ld == CLIST: %p ===\n",
#ifdef CONFIG_SPACE_NAMES
            space->debug_name, " == ",
#else
            "","",
#endif
            space, space->get_space_id().get_raw(),
            space->get_thread_count(),
            space->get_clist()
          );
    printf("UTCB Area: %lx/%ld  ",
            space->get_utcb_area().get_address(),
            space->get_utcb_area().get_size_log2()
            );
    printf("PageTable: %p", space->pgent(0));
    printf("\n");
    printf("Permissions: %c       ", space->may_plat_control() ? 'P' : '-');
    printf("SwitchSpace ID: %ld", space->get_switch_space().get_raw());
    printf("\n");
    //printf("IPC Control Bitmap");
    //printf("\n");
#if defined(CONFIG_ARM_V5)
    word_t i, first;

    printf("ARM PID: %2d  ", space->get_pid());
    printf("     vspace: %2d   ", space->get_vspace());
    printf("\n");
    if (space->get_domain() == INVALID_DOMAIN) {
        printf("No active domain");
    } else {
        printf("Domain: %2d   ", space->get_domain());
        printf("Domain Mask: %lx  ", space->get_domain_mask());
    }
    printf("\n");
    printf("Sharing: ");
    first = 1;
    for (i = 0; i < CONFIG_MAX_SPACES; i++) {
        if (bitmap_isset(space->get_shared_spaces_bitmap(), i)) {
            bool manager = bitmap_isset(space->get_manager_spaces_bitmap(), i);
            space_t *tmp = get_space_list()->lookup_space(spaceid(i));
            if (manager) {
                printf("M/");
            }
            printf("%s%S", first ? first = 0, "" : ", ", tmp);
        }
    }
    printf("\n");
    printf("Shared By: ");
    first = 1;
    for (i = 0; i < CONFIG_MAX_SPACES; i++) {
        if (bitmap_isset(space->get_client_spaces_bitmap(), i)) {
            space_t *tmp = get_space_list()->lookup_space(spaceid(i));
            printf("%s%S", first ? first = 0, "" : ", ", tmp);
        }
    }
    printf("\n");
#endif

    return CMD_NOQUIT;
}

#endif
