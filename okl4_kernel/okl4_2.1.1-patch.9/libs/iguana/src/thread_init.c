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

#include <stdio.h>
#include <iguana/env.h>
#include <iguana/object.h>
#include <iguana/pd.h>
#include <iguana/physpool.h>
#include <iguana/thread.h>
#include <iguana/virtpool.h>
#include <iguana/tls.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <mutex/mutex.h>

const envitem_t *__iguana_getenv(const char *name);

/*
 * Flag that non-main threads poll to see if the libraries have been
 * initialised.
 */
volatile uintptr_t __iguana_library_ready = 0;

/*
 * Initial work for additional threads created by the iguana server.
 *
 * These threads have to wait for the main thread to initialise the
 * libraries before they can safely jump to user code.  However,
 * because the libraries have not been initialised, there are very
 * limited things that this function can do.
 */
void __lib_iguana_thread_init(uintptr_t *obj_env)
{
    L4_ThreadId_t main_thread;
    const envitem_t *item;

    /*
     * Initialise the object environment.  This function needs to use
     * the object environment and the thread may have a higher
     * priority than the main thread.  Every thread in the PD will
     * pass in the same value, so this is safe to call multiple
     * times.
     */
    __lib_iguana_init_env(obj_env);

    /*
     * Find the main thread.  The name is generated automatically by
     * elfweaver for programs.
     */
    item = __iguana_getenv("MAIN");

    /* Give up now if there is no main thread. */
    if (item == NULL)
        return;

    /* Get the thread id of the main thread.
     *
     * This involves an iguana call.  If the magpie implmentation of
     * the call ever starts to rely on libc calls, this will break.
     */
    main_thread = thread_l4tid(env_thread(item));

    /* Wait for the libraries to be initialised and donate our
     * timeslices to the main thread to ensure that the initialisation
     * happens.
     */
    while (!__iguana_library_ready)
        L4_ThreadSwitch(main_thread);
}

/*
 * Flag that libc and other libraries and been initialised and that it
 * is safe for any other threads, created by iguana server, to run.
 */
void __lib_iguana_library_ready(void)
{
    __iguana_library_ready = 1;
}
