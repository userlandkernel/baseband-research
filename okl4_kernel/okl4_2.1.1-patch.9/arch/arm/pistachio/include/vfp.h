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
 * Description:   ARM generic VFP Coprocessor Support
 */
#ifndef __ARCH_ARM_INCLUDE_VFP_H__
#define __ARCH_ARM_INCLUDE_VFP_H__

#define FPSID       c0
#define FPSCR       c1
#define FPEXC       c8

#define FPEXC_EXCEPT    (1UL<<31)
#define FPEXC_EN        (1UL<<30)

/* Don't use fmrx/fmxr etc instead use mrc/mcr since we don't compile
 * kernel with FPU support
 */

#if defined(_lint)
#define vfp_getsr(reg, ret) ret = 0
#else
#define vfp_getsr(reg, ret)     \
    __asm__ __volatile__ (                                  \
        "mrc    p10, 7, "_(ret)","STR(reg)", cr0, 0;"       \
        _OUTPUT(ret))
#endif

#if defined(_lint)
void __vfp_putsr(word_t val);
#define vfp_putsr(reg, val) __vfp_putsr(val)
#else
#define vfp_putsr(reg, val)                                 \
{                                                           \
    word_t v = (word_t)val;                                 \
    __asm__ __volatile__ (                                  \
        "mcr    p10, 7, "_(v)","STR(reg)", cr0, 0;"         \
    _INPUT(v));                                             \
};
#endif
class arm_vfp_t
{
public:
    static void init()
        {
        }

    static void enable()
        {
            vfp_putsr(FPEXC, FPEXC_EN);
        }

    static void disable()
        {
            /* Disable VFP System */
            vfp_putsr(FPEXC, 0);
        }

    static void save_state(addr_t fpu_state)
        {
#if !defined(CONFIG_VFP_BANKS)
            word_t tmp;
            word_t *save_region = (word_t*)fpu_state;

            __asm__ __volatile__ (
                //"   fstmiax     "_(fpu_state)", {d0-d15}  ;"
                "   stc     p11, cr0, ["_(fpu_state)"], {33}  ;"
                :: [fpu_state] "r" (fpu_state)
            );
            vfp_getsr(FPSCR, tmp); save_region[33] = tmp;
            vfp_getsr(FPEXC, tmp); save_region[34] = tmp;
            vfp_getsr(FPINST, tmp); save_region[35] = tmp;
            vfp_getsr(FPINST2, tmp); save_region[36] = tmp;
#else
#error unimpl
#endif
        }

    static void load_state(addr_t fpu_state)
        {
#if !defined(CONFIG_VFP_BANKS)
            word_t tmp;
            word_t *save_region = (word_t*)fpu_state;

            __asm__ __volatile__ (
                //"   fldmiax     "_(fpu_state)", {d0-d15}  ;"
                "   ldc     p11, cr0, ["_(fpu_state)"], {33}  ;"
                :: [fpu_state] "r" (fpu_state)
            );
            tmp = save_region[33]; vfp_putsr(FPSCR, tmp);
            tmp = save_region[34]; vfp_putsr(FPEXC, tmp | FPEXC_EN);
            tmp = save_region[35]; vfp_putsr(FPINST, tmp);
            tmp = save_region[36]; vfp_putsr(FPINST2, tmp);
#else
#error unimpl
#endif
        }

    static word_t get_state_size()
        {
#if !defined(CONFIG_VFP_BANKS)
            return ((33+4)*sizeof(word_t));
#else
#error unimpl
#endif
        }
};

#endif /*__ARCH_ARM_INCLUDE_VFP_H__*/
