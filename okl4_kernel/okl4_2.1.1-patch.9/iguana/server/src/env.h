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
#ifndef IGUANA_INCLUDE_OBJECT_H
#define IGUANA_INCLUDE_OBJECT_H

#include <iguana/types.h>
#include <assert.h>

typedef enum envtype {
    ENV_CONSTANT    = 1,
    ENV_MEMSECTION  = 2,
    ENV_THREAD      = 3,
    ENV_VIRTPOOL    = 4,
    ENV_PHYSPOOL    = 5,
    ENV_ELF_SEGMENT = 6,
    ENV_CLIST       = 7,
    ENV_ELF_FILE    = 8,
    ENV_ZONE        = 9,
    ENV_THREAD_ID   = 10,
    ENV_PD          = 11,
    ENV_OBJECT      = 12,
} envtype_t;

#define ELF_TYPE_PROGRAM 1
#define ELF_TYPE_EXTENSION 2

struct envitem {
    char *name; /* This is a const char* in libs/iguana. */
    envtype_t type;
    union {
        struct pair {
            uintptr_t a;
            uintptr_t b;
        } consts;
        cap_t object;
        cap_t pd;
        cap_t memsection;
        cap_t thread;
        cap_t virtpool;
        cap_t physpool;
        struct elf_segment {
            uintptr_t vaddr;
            uintptr_t flags;
            uintptr_t paddr;
            uintptr_t offset;
            uintptr_t filesz;
            uintptr_t memsz;
        } elf_segment;
        cap_t clist;
        struct elf_file {
            uintptr_t type;
            uintptr_t entry;
        } elf_file;
        cap_t zone;
        L4_ThreadId_t   l4_tid;
    } u;
};

extern const uintptr_t env_version;
#define ENV_VERSION env_version

struct environment {
    uintptr_t version;
    uintptr_t len;
    struct envitem item[1];
};

typedef struct envitem envitem_t;

extern struct environment *__okl4_environ;

const envitem_t *iguana_getenv(const char *name);

static inline envtype_t env_type(const envitem_t *item)
{
    assert(item);
    return item->type;
}

static inline uintptr_t env_elf_file_type(const envitem_t *item)
{
    assert(item);
    assert(item->type == ENV_ELF_FILE);
    return item->u.elf_file.type;
}

static inline uintptr_t env_elf_file_entry(const envitem_t *item)
{
    assert(item);
    assert(item->type == ENV_ELF_FILE);
    return item->u.elf_file.entry;
}

const envitem_t *env_get_next(const envitem_t* last);

#endif
