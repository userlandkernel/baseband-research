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

#include <bootinfo/bootinfo.h>
#include <l4test/bootinfo.h>
#include <stdio.h>

extern void * __okl4_bootinfo;

/* Information to get out of bootinfo */
word_t kernel_max_spaces;
word_t kernel_max_mutexes;
word_t kernel_phys_base;
word_t kernel_phys_end;

/* This is passed the biggest section of physical memory */
static int
bootinfo_init_mem(uintptr_t virt_base, uintptr_t virt_end,
        uintptr_t phys_base, uintptr_t phys_end,
        const bi_user_data_t * data)
{
    struct bi_block * b = (struct bi_block *)data->user_data;

    b->base = phys_base;
    b->end  = phys_end;
    return 0;
}

static int
bootinfo_kernel_info(int max_spaces, int max_mutexes, int max_root_caps,
        const bi_user_data_t * data)
{
    struct bi_block * b = (struct bi_block *)data->user_data;

    b->max_spaces = max_spaces;
    b->max_mutexes = max_mutexes;
    return 0;
}

int
init_boot_params(void)
{
    int result;
    struct bi_block block;
    bi_callbacks_t bi_callbacks = OKL4_BOOTINFO_CALLBACK_INIT;

    /* Setup out bootinfo callback functions. */
    bi_callbacks.init_mem = bootinfo_init_mem;
    bi_callbacks.kernel_info = bootinfo_kernel_info;

    result = bootinfo_parse(__okl4_bootinfo, &bi_callbacks, &block);
    /* If parse fails, return the error. */
    if (result) {
        return result;
    }

    kernel_max_spaces = block.max_spaces;
    kernel_max_mutexes = block.max_mutexes;
    kernel_phys_base = block.base;
    kernel_phys_end = block.end;

    printf("bootinfo: max space = %lu, max mutex = %lu, phys [0x%lx, 0x%lx)\n",
            kernel_max_spaces, kernel_max_mutexes, kernel_phys_base,
            kernel_phys_end);

    return result;
}
