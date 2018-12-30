/*
 * Copyright (c) 2002-2004, University of New South Wales
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
 * Description:   Resource bit definitions for mips
 * Author:        Carl van Schaik
 */ 

#ifndef __ARCH__MIPS__RESOURCES_H__
#define __ARCH__MIPS__RESOURCES_H__

#include <kernel/arch/context.h>

#define MIPS_RESOURCE_FPU   1

#define HAVE_RESOURCE_TYPE_E
enum resource_type_e {
    COPY_AREA   = 0,
    EXCEPTION   = 1,
    KIPC = 2,           /* Used for decisions in fastpath */
};
/* FIXME: This is a little bit hacky */
#if defined(_lint)
#if defined(CONFIG_IS_32BIT)
#define __mips_fpr 32
#else
#define __mips_fpr 64
#endif
#endif
#if (__mips_fpr == 64)
typedef u64_t   fpr_t;
#elif (__mips_fpr == 32)
typedef u32_t   fpr_t;
#else
#error __mips_fpr not defined
#endif

struct fpu_state_t
{
    fpr_t fpu_gprs[32];     /* 32 FPRs */
    fpr_t fpu_fpcsr;        /* FPU control/status register */
};

class thread_resources_t : public generic_thread_resources_t
{
public:
    void save(tcb_t * tcb);
    void load(tcb_t * tcb);
    void purge(tcb_t * tcb);
    void init(tcb_t * tcb);
    void free(tcb_t * tcb);

    void set_exception_ipc(tcb_t * tcb);
    void clear_exception_ipc(tcb_t * tcb);
    void set_kernel_ipc(tcb_t * tcb);
    void clear_kernel_ipc(tcb_t * tcb);

    bool control(tcb_t * tcb, tc_resources_t resources);

    bool mips_fpu_unavail_exception( tcb_t *tcb, mips_irq_context_t *context );

private:
    void spill_fpu( tcb_t *tcb );
    void restore_fpu( tcb_t *tcb );
    void deactivate_fpu( tcb_t *tcb );
    void activate_fpu( tcb_t *tcb );

private:
    struct fpu_state_t *fpu_state;
};


class processor_resources_t {
public:
    void init_cpu(void) { fp_lazy_tcb = NULL; }

public:
    tcb_t *get_fp_lazy_tcb() { return fp_lazy_tcb; }

    void set_fp_lazy_tcb( tcb_t *tcb ) { fp_lazy_tcb = tcb; }
    void clear_fp_lazy_tcb() { fp_lazy_tcb = NULL; }

private:
    tcb_t *fp_lazy_tcb;
};

INLINE processor_resources_t *get_resources(void)
{
    extern processor_resources_t processor_resources;
    return &processor_resources;
}

#endif /* !__ARCH__MIPS__RESOURCES_H__ */
