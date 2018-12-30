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
 * ldt.c
 *
 * Local segment descriptor for x86
 */

#include <l4.h>
#include <syscalls.h>
#include <tcb.h>
#include <generic/lib.h>
#include <arch/segdesc.h>
#include <arch/sysdesc.h>
#include <arch/ldt.h>
#include <arch/syscalls.h>

extern ia32_segdesc_t  gdt[GDT_SIZE];  // arch/src/init.cc 

void update_ldt(tcb_t *tcb)
{
    u16_t seg = IA32_LDT;

    gdt[SEG_IDX(IA32_LDT)].set_sys((word_t)&tcb->arch.ldt,
        sizeof(tcb->arch.ldt) - 1, 0, ia32_segdesc_t::ldt);

    __asm__ __volatile__ (
        "lldt %[lptr] \n"
        :
        : [lptr] "m" (seg)
    );
}

/*
 * set_ldt()
 *
 * Set an LDT entry on x86 
 *
 * EOK for success, otherwise appropriate failure code on fail.
 */
word_t set_ldt(tcb_t *tcb, word_t control, ia32_segdesc_t seg)
{
    word_t index = control & LDT_INDEX_MASK;
    bool set_gs = control & LDT_GS_MASK;
    bool set_fs = control & LDT_FS_MASK;
    bool remove = false;

    if (index >= USER_LDT_ENTRIES) {
        return EINVALID_PARAM;
    }

    remove = (seg.get_base() == 0 && seg.get_limit() == 0);

    if (remove) {
        memset(&tcb->arch.ldt[index], 0, sizeof(tcb->arch.ldt[index]));
    } else {
        if (!seg.user_check()) {
            return EINVALID_PARAM;
        }

        memcpy(&tcb->arch.ldt[index], &seg, sizeof(tcb->arch.ldt[index]));
    }

    if (remove) {
        /* We need to ensure all segment selectors that are
         * pointing to the removed entry are set to sane values
         * so the exit path doesn't crash when restoring them.
         */
        if (tcb->arch.context.ds == SEGDESC(index, 3, 1))
            tcb->arch.context.ds = IA32_UDS;
        if (tcb->arch.context.es == SEGDESC(index, 3, 1))
            tcb->arch.context.es = IA32_UDS;
        if (tcb->arch.context.fs == SEGDESC(index, 3, 1))
            tcb->arch.context.fs = IA32_UDS;
        if (tcb->arch.context.gs == SEGDESC(index, 3, 1))
            tcb->arch.context.gs = IA32_UTCB;
    } else {
        if (set_fs)
            tcb->arch.context.fs = SEGDESC(index, 3, 1);
        if (set_gs)
            tcb->arch.context.gs = SEGDESC(index, 3, 1);
    }

    return EOK;
}

