/*
 * Copyright (c) 2002-2003, University of New South Wales
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
#include <arch/mips_cpu.h>

#include <tracebuffer.h>
#include <kdb/tracepoints.h>

/*
 * MIPS floating point resources do not have FPU resource flags.
 * The implementation depends on the per-thread flag in the thread's
 * status register. We only allow one thread to use the FPU per
 * CPU at a time.
 */

/*
 * Processor specific resources that are handled lazily
 * such as the floating point registers. This keeps a copy
 * of the resource owner for lazy replacement
 */
processor_resources_t processor_resources UNIT("cpulocal");

DECLARE_KMEM_GROUP (kmem_fpu);
DECLARE_TRACEPOINT (DISABLED_FPU);

void thread_resources_t::save (tcb_t * tcb)
{
}

void thread_resources_t::load (tcb_t * tcb)
{
}

void thread_resources_t::purge (tcb_t * tcb)
{
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    if (fp_tcb == tcb )
    {
        mips_cpu::enable_fpu();
        tcb->resources.spill_fpu( tcb );

        get_resources()->clear_fp_lazy_tcb();
    }
}

void thread_resources_t::free (tcb_t * tcb)
{
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    if (fp_tcb == tcb )
    {
        deactivate_fpu(tcb);
    }
}

void thread_resources_t::init (tcb_t * tcb)
{
    tcb->resource_bits.init ();

    this->fpu_state = NULL;
}

INLINE void thread_resources_t::deactivate_fpu( tcb_t *tcb )
{
    get_resources()->clear_fp_lazy_tcb();

    mips_irq_context_t * context = &tcb->arch.context;

    context->status &= ~ST_CU1;
}

INLINE void thread_resources_t::activate_fpu( tcb_t *tcb )
{
    get_resources()->set_fp_lazy_tcb( tcb );
    mips_irq_context_t * context = &tcb->arch.context;
    context->status |= ST_CU1;
}

void thread_resources_t::spill_fpu( tcb_t *tcb )
{
    // Spill the registers.
    /*lint -e529 start usage hidden by asm */
    fpr_t *start = this->fpu_state->fpu_gprs;
#if (__mips_fpr == 64)
    __asm__ __volatile__ (
            "sdc1  $f0,    0(%0) ;"
            "sdc1  $f1,    8(%0) ;"
            "sdc1  $f2,   16(%0) ;"
            "sdc1  $f3,   24(%0) ;"
            "sdc1  $f4,   32(%0) ;"
            "sdc1  $f5,   40(%0) ;"
            "sdc1  $f6,   48(%0) ;"
            "sdc1  $f7,   56(%0) ;"
            "sdc1  $f8,   64(%0) ;"
            "sdc1  $f9,   72(%0) ;"
            "sdc1  $f10,  80(%0) ;"
            "sdc1  $f11,  88(%0) ;"
            "sdc1  $f12,  96(%0) ;"
            "sdc1  $f13, 104(%0) ;"
            "sdc1  $f14, 112(%0) ;"
            "sdc1  $f15, 120(%0) ;"
            "sdc1  $f16, 128(%0) ;"
            "sdc1  $f17, 136(%0) ;"
            "sdc1  $f18, 144(%0) ;"
            "sdc1  $f19, 152(%0) ;"
            "sdc1  $f20, 160(%0) ;"
            "sdc1  $f21, 168(%0) ;"
            "sdc1  $f22, 176(%0) ;"
            "sdc1  $f23, 184(%0) ;"
            "sdc1  $f24, 192(%0) ;"
            "sdc1  $f25, 200(%0) ;"
            "sdc1  $f26, 208(%0) ;"
            "sdc1  $f27, 216(%0) ;"
            "sdc1  $f28, 224(%0) ;"
            "sdc1  $f29, 232(%0) ;"
            "sdc1  $f30, 240(%0) ;"
            "sdc1  $f31, 248(%0) ;"
            : /* ouputs */
            : /* inputs */
              "b" (start)
            );
#elif (__mips_fpr == 32)
    __asm__ __volatile__ (
            "swc1  $f0,    0(%0) ;"
            "swc1  $f1,    4(%0) ;"
            "swc1  $f2,    8(%0) ;"
            "swc1  $f3,   12(%0) ;"
            "swc1  $f4,   16(%0) ;"
            "swc1  $f5,   20(%0) ;"
            "swc1  $f6,   24(%0) ;"
            "swc1  $f7,   28(%0) ;"
            "swc1  $f8,   32(%0) ;"
            "swc1  $f9,   26(%0) ;"
            "swc1  $f10,  40(%0) ;"
            "swc1  $f11,  44(%0) ;"
            "swc1  $f12,  48(%0) ;"
            "swc1  $f13,  52(%0) ;"
            "swc1  $f14,  56(%0) ;"
            "swc1  $f15,  60(%0) ;"
            "swc1  $f16,  64(%0) ;"
            "swc1  $f17,  68(%0) ;"
            "swc1  $f18,  72(%0) ;"
            "swc1  $f19,  76(%0) ;"
            "swc1  $f20,  80(%0) ;"
            "swc1  $f21,  84(%0) ;"
            "swc1  $f22,  88(%0) ;"
            "swc1  $f23,  92(%0) ;"
            "swc1  $f24,  96(%0) ;"
            "swc1  $f25, 100(%0) ;"
            "swc1  $f26, 104(%0) ;"
            "swc1  $f27, 108(%0) ;"
            "swc1  $f28, 112(%0) ;"
            "swc1  $f29, 116(%0) ;"
            "swc1  $f30, 120(%0) ;"
            "swc1  $f31, 124(%0) ;"
            : /* ouputs */
            : /* inputs */
              "b" (start)
            );
#endif

    // Save the FPCSR
    __asm__ __volatile__ (
            "cfc1   %0, $31 ;"
            : /* outputs */
              "=r" (this->fpu_state->fpu_fpcsr)
            );

    this->deactivate_fpu( tcb );
}

INLINE void thread_resources_t::restore_fpu( tcb_t *tcb )
{
    this->activate_fpu( tcb );

    // Restore the registers.
    /*lint -e529 start usage hidden by asm */
    fpr_t *start = this->fpu_state->fpu_gprs;
#if (__mips_fpr == 64)
    __asm__ __volatile__ (
            "ldc1  $f0,    0(%0) ;"
            "ldc1  $f1,    8(%0) ;"
            "ldc1  $f2,   16(%0) ;"
            "ldc1  $f3,   24(%0) ;"
            "ldc1  $f4,   32(%0) ;"
            "ldc1  $f5,   40(%0) ;"
            "ldc1  $f6,   48(%0) ;"
            "ldc1  $f7,   56(%0) ;"
            "ldc1  $f8,   64(%0) ;"
            "ldc1  $f9,   72(%0) ;"
            "ldc1  $f10,  80(%0) ;"
            "ldc1  $f11,  88(%0) ;"
            "ldc1  $f12,  96(%0) ;"
            "ldc1  $f13, 104(%0) ;"
            "ldc1  $f14, 112(%0) ;"
            "ldc1  $f15, 120(%0) ;"
            "ldc1  $f16, 128(%0) ;"
            "ldc1  $f17, 136(%0) ;"
            "ldc1  $f18, 144(%0) ;"
            "ldc1  $f19, 152(%0) ;"
            "ldc1  $f20, 160(%0) ;"
            "ldc1  $f21, 168(%0) ;"
            "ldc1  $f22, 176(%0) ;"
            "ldc1  $f23, 184(%0) ;"
            "ldc1  $f24, 192(%0) ;"
            "ldc1  $f25, 200(%0) ;"
            "ldc1  $f26, 208(%0) ;"
            "ldc1  $f27, 216(%0) ;"
            "ldc1  $f28, 224(%0) ;"
            "ldc1  $f29, 232(%0) ;"
            "ldc1  $f30, 240(%0) ;"
            "ldc1  $f31, 248(%0) ;"
            : /* ouputs */
            : /* inputs */
              "b" (start)
            );
#elif (__mips_fpr == 32)
    __asm__ __volatile__ (
            "lwc1  $f0,    0(%0) ;"
            "lwc1  $f1,    4(%0) ;"
            "lwc1  $f2,    8(%0) ;"
            "lwc1  $f3,   12(%0) ;"
            "lwc1  $f4,   16(%0) ;"
            "lwc1  $f5,   20(%0) ;"
            "lwc1  $f6,   24(%0) ;"
            "lwc1  $f7,   28(%0) ;"
            "lwc1  $f8,   32(%0) ;"
            "lwc1  $f9,   26(%0) ;"
            "lwc1  $f10,  40(%0) ;"
            "lwc1  $f11,  44(%0) ;"
            "lwc1  $f12,  48(%0) ;"
            "lwc1  $f13,  52(%0) ;"
            "lwc1  $f14,  56(%0) ;"
            "lwc1  $f15,  60(%0) ;"
            "lwc1  $f16,  64(%0) ;"
            "lwc1  $f17,  68(%0) ;"
            "lwc1  $f18,  72(%0) ;"
            "lwc1  $f19,  76(%0) ;"
            "lwc1  $f20,  80(%0) ;"
            "lwc1  $f21,  84(%0) ;"
            "lwc1  $f22,  88(%0) ;"
            "lwc1  $f23,  92(%0) ;"
            "lwc1  $f24,  96(%0) ;"
            "lwc1  $f25, 100(%0) ;"
            "lwc1  $f26, 104(%0) ;"
            "lwc1  $f27, 108(%0) ;"
            "lwc1  $f28, 112(%0) ;"
            "lwc1  $f29, 116(%0) ;"
            "lwc1  $f30, 120(%0) ;"
            "lwc1  $f31, 124(%0) ;"
            : /* ouputs */
            : /* inputs */
              "b" (start)
            );
#endif

    // Restore the FPCSR
    __asm__ __volatile__ (
            "ctc1   %0, $31 ;"
            : /* outputs */
            : /* inputs */
              "r" (this->fpu_state->fpu_fpcsr)
            );
}

bool thread_resources_t::mips_fpu_unavail_exception( tcb_t *tcb, mips_irq_context_t *context )
{
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    TRACEPOINT (DISABLED_FPU,
                printf ("FPU disabled exception:  cur=%p  owner=%p\n",
                        tcb, fp_tcb));

    /* If thread has FPU resources rights */
    if ( fpu_state )
    {
        /* In our lazy floating point model, we should never see a floating point
         * exception if the current tcb already owns the floating point register
         * file.
         */
        ASSERT( DEBUG, fp_tcb != tcb );

        mips_cpu::enable_fpu();

        if( fp_tcb )
            fp_tcb->resources.spill_fpu( fp_tcb );

        this->restore_fpu( tcb );

        return true;
    }
    return false;
}

bool thread_resources_t::control(tcb_t * tcb, tc_resources_t resources)
{
    if (resources.bits.arch & MIPS_RESOURCE_FPU)
    {
        if (this->fpu_state == NULL)
        {
            this->fpu_state = (struct fpu_state_t*)kmem.alloc(kmem_fpu, sizeof(struct fpu_state_t), true);
            if (!this->fpu_state)
                return false;
        }
    }
    else
    {
        tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

        if (fp_tcb == tcb )
        {
            deactivate_fpu(tcb);
        }

        kmem.free(kmem_fpu, this->fpu_state, sizeof(struct fpu_state_t));
        this->fpu_state = NULL;
    }
    return true;
}

