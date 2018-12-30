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
 * Description: Show debugging information about kernel-level mutexes.
 */

#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/print.h>

#include <tcb.h>
#include <mutex.h>

#if defined(CONFIG_KDB_CONS)

DECLARE_CMD(cmd_show_mutexes, root, 'm', "showmutex",  "show created mutexes");

/**
 * Print the name of the given mutex-id in a human-readable format.
 */
int SECTION(SEC_KDEBUG)
print_mutex(word_t val, word_t width, word_t precision, bool adjleft)
{
#if defined(CONFIG_MUTEX_NAMES)
    mutex_t *mutex = mutexid_table.lookup(mutexid(val));
    if (mutex != NULL && mutex->debug_name[0] != '\0') {
        return print_string(mutex->debug_name, width, precision);
    }
#endif
    return print_dec(val, width);
}

/**
 * Dump information about the given mutex.
 */
void SECTION(SEC_KDEBUG) dump_mutex(mutex_t * mutex)
{
    int padding = 8;

    /* Show the mutex name. */
    printf("  ");
    padding -= printf("%M", mutex->id.get_number());
    while (padding-- > 0) {
        printf(" ");
    }

    /* Show the current holder, if any. */
    tcb_t * holder = mutex->get_holder();

    if (holder != NULL) {
        printf(": [%8t] ", holder);
    } else {
        printf(": [        ] ");
    }

    /* Show current threads waiting. */
    if (mutex->sync_point.get_blocked_head() != NULL) {
        tcb_t * first = mutex->sync_point.get_blocked_head();
        tcb_t * current = first;

        do {
            word_t id = current->get_global_id().get_raw();

            if (current->get_state().is_runnable()) {
                printf("%t ", id);
            } else {
                printf("(%t) ", id);
            }
            current = current->blocked_list.next;
        } while (current != first);
        printf("\n");
    }
    printf("\n");
}

CMD(cmd_show_mutexes, cg)
{
    if (!global_mutexes_list) {
        printf("No mutexes currently present in system.\n");
        return CMD_NOQUIT;
    }

    /* Iterate through each mutex and show its status. */
    printf("Mutexes in the system: (mutex name/holder/waiters)\n\n");
    mutex_t * first = global_mutexes_list;
    mutex_t * current = first;
    do {
        dump_mutex(current);
        current = current->present_list.next;
    } while (first != current);

    return CMD_NOQUIT;
}

DECLARE_CMD(cmd_show_dep_graph, root, 'G', "dependgraph",
            "show system sync-point dependency graph");

CMD(cmd_show_dep_graph, cg)
{
    /* Get the head of the list. */
    tcb_t * head = global_present_list;
    tcb_t * first = head;

    if (head == NULL) {
        printf("No threads in system.\n");
        return CMD_NOQUIT;
    }

    /* Print graph nodes. */
    printf("Nodes:\n\n");

    /* Print the threads in the system. */
    do {
        printf("  Thread [%p] (Thread '%t' with priority %d)\n",
                head, head, head->base_prio);
        head = head->present_list.next;
    } while (first != head);

    /* Print the IPC-endpoints in the system that have at least one waiter. */
    first = head;
    do {
        if (head->get_endpoint()->get_send_head() != NULL) {
            printf("  Syncpoint [%p] (End-point (send) of '%t')\n",
                    head->get_endpoint()->get_send_syncpoint(), head);
        }
        head = head->present_list.next;
    } while (first != head);

#if defined(CONFIG_SCHEDULE_INHERITANCE)

    first = head;
    do {
        if (head->get_endpoint()->get_recv_head() != NULL) {
            printf("  Syncpoint [%p] (End-point (recv) of '%t')\n",
                    head->get_endpoint()->get_recv_syncpoint(), head);
        }
        head = head->present_list.next;
    } while (first != head);


#endif

    /* Print mutexes that have at least one thread blocked on them
     * or are owned by a thread. */
    mutex_t * mutex = global_mutexes_list;
    if (mutex != NULL) {
        do {
            if (mutex->sync_point.get_blocked_head() != NULL
                    || mutex->sync_point.get_donatee() != NULL) {
                printf("  Syncpoint [%p] (Mutex '%M')\n",
                        &mutex->sync_point, mutex->id.get_number());
            }
            mutex = mutex->present_list.next;
        } while (mutex != global_mutexes_list);
    }
    printf("\n");

    /* Print edges. */
    printf("Edges:\n\n");

    /* Print who each thread is waiting for, and each endpoint. */
    first = head;
    do {
        if (head->get_waiting_for() != NULL) {
            printf("  [%p] -> [%p]\n", head, head->get_waiting_for());
        }
        if (head->get_endpoint()->get_send_head() != NULL) {
            printf("  [%p] -> [%p]\n",
                    head->get_endpoint()->get_send_syncpoint(), head);
        }
#if defined(CONFIG_SCHEDULE_INHERITANCE)
        if (head->get_endpoint()->get_recv_head() != NULL) {
            printf("  [%p] -> [%p]\n",
                    head->get_endpoint()->get_recv_syncpoint(), head);
        }
#endif
        head = head->present_list.next;
    } while (first != head);

    /* Print what each mutex is waiting for. */
    mutex = global_mutexes_list;
    if (mutex != NULL) {
        do {
            if (mutex->sync_point.get_blocked_head() != NULL
                    || mutex->sync_point.get_donatee() != NULL) {
                printf("  [%p] -> [%p]\n", mutex,
                        mutex->sync_point.get_donatee());
            }
            mutex = mutex->present_list.next;
        } while (mutex != global_mutexes_list);
    }
    printf("\n");

    return CMD_NOQUIT;
}

#endif /* CONFIG_KDB_CONS */

