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

#include <iguana/object.h>
#if defined(CONFIG_SESSION)
#include <iguana/session.h>
#endif
#include <iguana/memsection.h>
#include <inttypes.h>
#include <util/trace.h>
#include <iguana/thread.h>

#include <stdlib.h>       /* For malloc -- maybe we use a slab allocator here */
#include <stdio.h>

#include <circular_buffer/cb.h>

#define PAGESIZE 4096           /* FIXME: Benjl */

object_t *
object_get_interface(objref_t obj)
{
    object_t *obj_interface = malloc(sizeof(object_t));
#if !defined(CONFIG_SESSION)
    thread_ref_t server;
    memsection_ref_t memsection;
#endif
    
    if (obj_interface == NULL)
        return NULL;

    obj_interface->obj = obj;

#if defined(CONFIG_SESSION)
    /*
     * FIXME: Default behaviour should really be dedicated clist 
     */
    obj_interface->session =
        session_create_full_share(obj, &obj_interface->server);
#else
    memsection = memsection_lookup(obj, &server);
    obj_interface->server = thread_l4tid(server);
#endif
    return obj_interface;
}

object_t *
object_get_async_interface(objref_t obj)
{
    object_t *obj_interface = object_get_interface(obj);
    objref_t call_buf, return_buf;

    memsection_create(PAGESIZE, &return_buf);

    call_buf = return_buf + PAGESIZE / 2;

    obj_interface->call_cb = cb_new_withmem((void *)call_buf, PAGESIZE / 2);
    obj_interface->return_cb = cb_new_withmem((void *)return_buf, PAGESIZE / 2);
#if defined(CONFIG_SESSION)
    session_add_async(obj_interface->session, call_buf, return_buf);
#endif
    return obj_interface;
}

void
object_print(object_t *instance)
{
    DEBUG_TRACE(1, "Instance: %p -- server: %lx objref: %" PRIxPTR "\n",
           instance, instance->server.raw, instance->obj);
}
