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

#include <l4/thread.h>
#include <l4/mutex.h>
#include <l4/config.h>

#include <range_fl/range_fl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "mutex.h"

extern word_t kernel_max_mutexes;

/*
 * Mutexes may need to be allocated during the server startup process,
 * before much of the system is available. In particular, malloc() and
 * printf() require locks prior to mutex_init() being called.
 *
 * We dumbly allocate mutexes up until we are intialised properly later
 * in the boot sequence, and disallow releasing of mutexes until we are
 * initialised.
 */
static int static_mutexes_allocated = 0;
static int init_performed = 0;

/* The list of available mutexes. */
rfl_t free_mutexes;

/* Mutex support intialisation. */
void mutex_init(void)
{
    int result;

    /* Allocate a new range list. */
    free_mutexes = rfl_new();
    assert(free_mutexes != NULL);

    /* Add the list of available mutex IDs to the list. */
    result = rfl_insert_range(free_mutexes, static_mutexes_allocated,
            kernel_max_mutexes - 1);
    assert(result == 0);

    /* Keep note that we have finished initialising. */
    init_performed = 1;
}

/* Allocate a mutex id. */
int mutex_allocate(L4_MutexId_t * newid)
{
    int id;
    L4_Word_t success;

    /* Allocate a Mutex ID. */
    if (!init_performed) {
        /* Simple allocation of mutexes for system bootup. */
        assert(static_mutexes_allocated < kernel_max_mutexes);
        id = static_mutexes_allocated++;
    } else {
        /* Pooled allocation of mutexes. */
        id = rfl_alloc(free_mutexes);
    }

    /* Return an error if no mutex was available. */
    if (id < 0) {
        return ERR_MUTEX_NOMEM;
    }

    /* Create the mutex. */
    success = L4_CreateMutex(L4_MutexId(id));
    if (!success) {
        return ERR_MUTEX_INVAL;
    }

    /* Successfully created the mutex. */
    *newid = L4_MutexId(id);
    return 0;
}

int mutex_free(L4_MutexId_t id)
{
    L4_Word_t success;
    int error;

    /* Free the mutex using L4, and add the ID to the free list. */
    success = L4_DeleteMutex(id);
    if (!success) {
        if (L4_ErrorCode() == L4_ErrMutexBusy) {
            return ERR_MUTEX_BUSY;
        } else {
            return ERR_MUTEX_INVAL;
        }
    }

    /* Add it to the free list. */
    error = rfl_free(free_mutexes, L4_MutexNo(id));
    assert(!error);

    return 0;
}

