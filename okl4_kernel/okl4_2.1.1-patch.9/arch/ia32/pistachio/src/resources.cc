/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   thread resource management
 */
#include <l4.h>
#include <tcb.h>
#include <arch/fpu.h>
#include <arch/resource_functions.h>
#include <kdb/tracepoints.h>

//#define FPU_REENABLE
static tcb_t * fpu_owner UNIT("cpulocal");

DECLARE_KMEM_GROUP (kmem_resources);

#ifdef FPU_REENABLE
DECLARE_TRACEPOINT(IA32_FPU_REENABLE);
#endif

void thread_switch_helper (void)
{
    tcb_t * current = get_current_tcb();
    current->resources.load(current);
    ACTIVATE_CONTINUATION(current->cont);
}

void thread_resources_t::save(tcb_t * tcb)
{
    if (tcb->resource_bits.have_resource (FPU))
    {
        ia32_fpu_t::disable();
#ifndef FPU_REENABLE
        tcb->resource_bits -= FPU;
#endif
    }
}

void thread_resources_t::load(tcb_t * tcb)
{
#ifdef FPU_REENABLE
    if (tcb->resource_bits.have_resource (FPU))
    {
        ASSERT (DEBUG, fpu_owner == tcb);
        ASSERT (DEBUG, fpu_state != NULL);
        TRACEPOINT(IA32_FPU_REENABLE,
                   printf("strictly reenabling FPU for %t\n", tcb));
        ia32_fpu_t::enable();
    }
#endif
}

void thread_resources_t::purge(tcb_t * tcb)
{
    if (fpu_owner == tcb)
    {
        ia32_fpu_t::enable();
        ia32_fpu_t::save_state(this->fpu_state);
        fpu_owner = NULL;
        ia32_fpu_t::disable();
#ifdef FPU_REENABLE
        tcb->resource_bits -= FPU;
#endif
    }
}


void thread_resources_t::init(tcb_t * tcb)
{
    tcb->resource_bits.init ();
    fpu_state = NULL;
}


void thread_resources_t::free(tcb_t * tcb)
{
    ASSERT(DEBUG, tcb);
    if (fpu_state)
    {
        kmem.free(kmem_resources, fpu_state, ia32_fpu_t::get_state_size());
        fpu_state = NULL;

        if (fpu_owner == tcb)
        {
            fpu_owner = NULL;
            ia32_fpu_t::disable();
        }
    }
}

void thread_resources_t::ia32_no_math_exception(tcb_t * tcb)
{
    ASSERT(NORMAL, &tcb->resources == this);
    ia32_fpu_t::enable();

    // if the current thread owns the fpu already do a quick exit
    if (fpu_owner != tcb)
    {
        if (fpu_owner != NULL)
        {
            ia32_fpu_t::save_state(fpu_owner->resources.fpu_state);
#ifdef  FPU_REENABLE
            fpu_owner->resource_bits -= FPU;
#endif
        }
        fpu_owner = tcb;

        if (fpu_state == NULL)
        {
            fpu_state = kmem.alloc(kmem_resources, ia32_fpu_t::get_state_size(), true);
            ASSERT(ALWAYS, fpu_state);
            ia32_fpu_t::init();
        }
        else
            ia32_fpu_t::load_state(fpu_state);
    }

    tcb->resource_bits += FPU;
}

