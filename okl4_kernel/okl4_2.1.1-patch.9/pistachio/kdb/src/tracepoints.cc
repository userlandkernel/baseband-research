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
 * Description:   Tracepoint related commands
 */
#include <l4.h>
#include <kdb/tracepoints.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>
#include <kdb/macro_set.h>
#if defined(CONFIG_ARCH_ARM)
#include <arch/globals.h>
#endif

#if defined(CONFIG_TRACEPOINTS)
/*lint -e665 */
GLOBAL_DEF(bool tracepoints_enabled = false)

/*
 * Linker set containing all tracepoints.
 */

DECLARE_SET (tracepoint_set);

tracepoint_list_t tp_list = { &tracepoint_set };

#if defined(CONFIG_KDB_CLI)

static void list_tp_choices (void);

/*
 * Command group for tracepoint commands.
 */

DECLARE_CMD_GROUP (tracepoints);

/**
 * cmd_tracepoints: enable/disable/list tracepoints
 */
DECLARE_CMD (cmd_tracepoints, root, 'r', "tracepoints",
             "enable/disable/list tracepoints");

CMD(cmd_tracepoints, cg)
{
    return tracepoints.interact (cg, "tracepoints");
}


static void print_header(void)
{
    printf("\nTracepoints: %s\n", GLOBAL(tracepoints_enabled) ? "enabled\n" : "disabled\n");

    printf (" Num  %28s Print  KDB  ", "Name");

#ifdef CONFIG_MDOMAINS
    for (int cpu = 0; cpu < CONFIG_NUM_DOMAINS; cpu++)
        printf(" CONTEXT[%d]  ", cpu);
    printf("\n");
#else
    printf("Counter\n");
#endif

}


/**
 * cmd_tp_list: list all tracepoints
 */
DECLARE_CMD (cmd_tp_list, tracepoints, 'l', "list", "list tracepoints");

CMD(cmd_tp_list, cg)
{
    tracepoint_t * tp;

    print_header();

    tp_list.reset ();
    for (int i = 0; (tp = tp_list.next ()) != NULL; i++)
    {
        printf ("%3d   %28s  %c      %c ",
                i+1, trace_names[tp->id], tp->enabled ? 'y' : 'n',
                tp->enter_kdb ? 'y' : 'n');
        for (int cpu = 0; cpu < CONFIG_NUM_DOMAINS; cpu++)
            printf("%8d ", tp->counter[cpu]);

        printf("\n");
    }

    return CMD_NOQUIT;
}


/**
 * cmd_tp_conf: configure a tracepoint
 */
DECLARE_CMD (cmd_tp_conf, tracepoints, 'c', "conf", "configure a tracepoint");

CMD(cmd_tp_conf, cg)
{
    for (;;)
    {
        word_t n = get_dec ("Tracepoint", 0, "list");
        if (n == 0)
            list_tp_choices ();
        else if (n == ABORT_MAGIC)
            return CMD_NOQUIT;
        else if (n <= tp_list.size ())
        {
            tracepoint_t * tp = tp_list.get (n - 1);
#ifdef CONFIG_MDOMAINS
            word_t cpu_mask = get_hex("Processor Mask", ~0UL, "all");
#else
            word_t cpu_mask = get_choice("Print", "y/n",
                    tp->enabled ? 'n' : 'y') == 'y' ? ~0UL : 0;
#endif
            tp->enabled = cpu_mask;
            tp->enter_kdb = get_choice ("Enter KDB", "y/n", 'n') == 'y' ?
                cpu_mask : 0;
            tp->reset_counter ();

            print_header();
            printf ("%3d   %28s  %c      %c ",
                    n, trace_names[tp->id], tp->enabled ? 'y' : 'n',
                    tp->enter_kdb ? 'y' : 'n');
            for (int cpu = 0; cpu < CONFIG_NUM_DOMAINS; cpu++)
                printf("%8d ", tp->counter[cpu]);
            printf("\n");
            return CMD_NOQUIT;
        }
    }

    NOTREACHED();
}


/**
 * cmd_tp_enable_all: configure all tracepoints
 */
DECLARE_CMD (cmd_tp_conf_all, tracepoints, 'C', "confall",
             "configure all tracepoints");

CMD(cmd_tp_conf_all, cg)
{
    tracepoint_t * tp;

#ifdef CONFIG_MDOMAINS
    word_t cpu_mask = get_hex("Processor Mask", ~0UL, "all");
#else
    word_t cpu_mask = get_choice("Print", "y/n", 'y') == 'y' ? ~0UL : 0;
#endif
    word_t enter_kdb = get_choice ("Enter KDB", "y/n", 'y') == 'y' ?
            cpu_mask : 0;

    tp_list.reset ();
    while ((tp = tp_list.next ()) != NULL)
    {
        tp->enabled = cpu_mask;
        tp->enter_kdb = enter_kdb;
        tp->reset_counter ();
    }

    return CMD_NOQUIT;
}


/**
 * cmd_tp_reset: reset all tracepoint counters
 */
DECLARE_CMD (cmd_tp_reset, tracepoints, 'R', "reset", "reset counters");

CMD(cmd_tp_reset, cg)
{
    tracepoint_t * tp;

    tp_list.reset ();
    while ((tp = tp_list.next ()) != NULL)
        tp->reset_counter();

    return CMD_NOQUIT;
}


/**
 * cmd_tp_enable: enable all tracepoints
 */
DECLARE_CMD (cmd_tp_enable, tracepoints, 'E', "enable", "enable tracepoints");

CMD(cmd_tp_enable, cg)
{
    GLOBAL(tracepoints_enabled) = true;

    return CMD_NOQUIT;
}


/**
 * cmd_tp_disable: disable all tracepoints
 */
DECLARE_CMD (cmd_tp_disable, tracepoints, 'D', "disable", "disable tracepoints");

CMD(cmd_tp_disable, cg)
{
    GLOBAL(tracepoints_enabled) = false;

    return CMD_NOQUIT;
}



static void SECTION(SEC_KDEBUG) list_tp_choices (void)
{
    word_t size = tp_list.size ();

    for (word_t i = 0; i <= size / 2; i++)
    {
        if (i == 0)
            printf ("%3d - %28s", 0, "List choices");
        else
            printf ("%3d - %28s", i, trace_names[tp_list.get (i - 1)->id]);

        if (i + (size / 2) < size)
            printf ("%3d - %s\n", i + (size / 2) + 1,
                    trace_names[tp_list.get (i + (size / 2))->id]);
    }
    printf ("\n");
}

#endif /* CONFIG_KDB_CONS */

#endif /* CONFIG_TRACEPONTS */

