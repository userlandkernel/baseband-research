/*
 * Copyright (c) 2002, 2003, Karlsruhe University
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
 * Description:   Kernel debugger commands and command groups.
 */
#ifndef __CMD_H__
#define __CMD_H__

#if defined(CONFIG_DEBUG)

#include <kdb/macro_set.h>


/**
 * cmd_mode_t: Mode of operation for the kernel debugger.
 */
typedef enum {
    CMD_KEYMODE,
    CMD_LINEMODE
} cmd_mode_t;


/* From generic/entry.cc */
extern cmd_mode_t kdb_cmd_mode;



/*
 * Unprintable keystrokes.
 */

#define KEY_RETURN              '\r'
#define KEY_NEWLINE             '\n'
#define KEY_TAB                 '\t'
#define KEY_ESC                 0x1b
#define KEY_BS                  0x8

class cmd_t;
class cmd_group_t;



/**
 * cmd_ret_t: Value returned from every command.  NOQUIT means that
 * more commands should be executed from the command group, ABORT
 * means that one shoul back up to the previous command group, and
 * QUIT means that one should back up to the root and exit the kernel
 * debugger.
 */
typedef enum {
    CMD_NOQUIT,
    CMD_ABORT,
    CMD_QUIT
} cmd_ret_t;


/**
 * cmd_func_t: Function type for all kernel debugger commands.  The
 * function takes the current command group as an argument and returns
 * a cmd_ret_t value.
 */
typedef cmd_ret_t (*cmd_func_t)(cmd_group_t *);



/**
 * cmd_t: Descriptor for kernel debugger command.
 */
class cmd_t
{
public:
    char        key;
    char        *command;
    char        *description;
    cmd_func_t  function;
};


/**
 * cmd_group_t: Descriptor for kernel debugger command group.
 */
class cmd_group_t
{
public:
    macro_set_t        *cmd_set;
    cmd_group_t         *parent;
    char                *name;

    cmd_ret_t interact (cmd_group_t * myparent, char * myname);
    void reset (void) { cmd_set->reset (); }
    cmd_t * next (void) { return (cmd_t *) cmd_set->next (); }

private:
    cmd_t * interact_by_key (void);
    cmd_t * interact_by_command (void);
};

#if defined(CONFIG_KDB_CLI)

/**
 * DECLARE_CMD_GROUP: Declares a new command group.  Initializes group
 * with a help, abort and mode switch command.
 */
#define DECLARE_CMD_GROUP(group)                                        \
    DECLARE_SET(__kdb_group_##group);                                   \
    cmd_group_t group = { &__kdb_group_##group, NULL, NULL };           \
    DECLARE_CMD (cmd__help, group, '?', "help", "this help message");   \
    DECLARE_CMD (cmd__abort, group, KEY_BS, "up", "back up to previous menu"); \
    DECLARE_CMD (cmd__prior, group, KEY_ESC, "prior", "back to previous menu")

/**
 * DECLARE_CMD: Declares a new command in a command group and
 * initializes the command descriptor.
 */
#define DECLARE_CMD(func, group, key, cmd, desc)                        \
    static cmd_t __kdb_##group##_##func = { key, cmd, desc, &kdb_t::func };     \
    PUT_SET(__kdb_group_##group, __kdb_##group##_##func)

#define CMD_SEC         SEC_KDEBUG

#else /* !CONFIG_KDB_CLI */

/* FIXME: need to abstract this one! */
#define CMD_SEC         ".delete"

/* Disable Command Line Interface */
#define DECLARE_CMD_GROUP(group) \
    extern cmd_group_t group;
#define DECLARE_CMD(x...)

#endif /* !CONFIG_KDB_CLI */

#define CMD(func, param) \
cmd_ret_t SECTION(CMD_SEC) kdb_t::func(cmd_group_t *param)

#endif /* !CONFIG_DEBUG */

#endif /* !__CMD_H__ */
