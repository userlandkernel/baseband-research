/*
 * Copyright (c) 2001-2004, Karlsruhe University
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
 * Description:   IA32 helper functions to access special registers
 */
#ifndef __ARCH__IA32__CPU_H__
#define __ARCH__IA32__CPU_H__

#include <kernel/arch/ia32.h>

INLINE bool ia32_has_cpuid()
{
    /* Iff bit 21 in EFLAGS can be set the CPU supports the CPUID
     * instruction */
    word_t eflags;
    __asm__ (
        // Save EFLAGS to the stack
        "pushfl                 \n"
        // Set bit 21 in EFLAGS image on stack
        "orl     %1,(%%esp)     \n"
        // Restore EFLAGS from stack.
        "popfl                  \n"
        // If supported, this has set bit 21
        // Save EFLAGS on stack to see if bit 21 was set or not
        "pushfl                 \n"
        // Move EFLAGS image to register for inspection
        "pop     %0             \n"
        : "=a" (eflags)
        : "i" (IA32_EFL_ID)
        );
    return (eflags & IA32_EFL_ID);
}

INLINE void ia32_cpuid(word_t index,
                       word_t* eax, word_t* ebx, word_t* ecx, word_t* edx)
{
    __asm__ (
        "cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "a" (index)
        );
}


INLINE u32_t ia32_get_cpu_features()
{
    if (ia32_has_cpuid ())
    {
        u32_t features, dummy;
        ia32_cpuid(1, &dummy, &dummy, &dummy, &features);
        return features;
    } else {
        /* If there is no CPUID instruction we just fabricate the
         * appropriate feature word.  Currently we only support
         * i486DX+ and therefore assume the FPU to be present */
        return IA32_FEAT_FPU;
    }
}

INLINE u64_t ia32_rdpmc(const int ctrsel)
{
    u64_t __return;

    __asm__ __volatile__ (
        "rdpmc"
        : "=A"(__return)
        : "c"(ctrsel));

    return __return;
}

INLINE u64_t ia32_rdtsc(void)
{
    u64_t __return;

    __asm__ __volatile__ (
        "rdtsc"
        : "=A"(__return));

    return __return;
}

INLINE u64_t ia32_rdmsr(const u32_t reg)
{
    u64_t __return;

    __asm__ __volatile__ (
        "rdmsr"
        : "=A"(__return)
        : "c"(reg)
        );

    return __return;
}

INLINE void ia32_wrmsr(const u32_t reg, const u64_t val)
{
    __asm__ __volatile__ (
        "wrmsr"
        :
        : "A"(val), "c"(reg));
}

INLINE void ia32_settsc(const u64_t val)
{
    ia32_wrmsr(0x10, val);
}

INLINE void ia32_wbinvd()
{
    __asm__ __volatile__ ("wbinvd\n" : : : "memory");
}

INLINE int ia32_lsb (u32_t w) __attribute__ ((const));
INLINE int ia32_lsb (u32_t w)
{
    int bitnum;
    __asm__ __volatile__ ("bsf %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}

INLINE void ia32_cr0_set(const u32_t val)
{
    u32_t tmp;
    __asm__ __volatile__ ("mov  %%cr0, %0       \n"
                          "orl  %1, %0          \n"
                          "mov  %0, %%cr0       \n"
                          : "=r"(tmp)
                          : "ri"(val));
}

INLINE void ia32_cr0_mask(const u32_t val)
{
    u32_t tmp;
    __asm__ __volatile__ ("movl %%cr0, %0       \n"
                          "andl %1, %0          \n"
                          "movl %0, %%cr0       \n"
                          : "=r"(tmp)
                          : "ri"(~val));
}

INLINE void ia32_cr4_set(const u32_t val)
{
    u32_t tmp;
    __asm__ __volatile__ ("movl %%cr4, %0       \n"
                          "orl  %1, %0          \n"
                          "movl %0, %%cr4       \n"
                          : "=r"(tmp)
                          : "ri"(val));
}

INLINE void ia32_cr4_mask(const u32_t val)
{
    u32_t tmp;
    __asm__ __volatile__ ("movl %%cr4, %0       \n"
                          "andl %1, %0          \n"
                          "movl %0, %%cr4       \n"
                          : "=r"(tmp)
                          : "ri"(~val));
}

#endif /* !__ARCH__IA32__CPU_H__ */
