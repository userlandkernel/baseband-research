/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description:   ARM init code support
 */
#ifndef __ARCH__ARM__INIT_H__
#define __ARCH__ARM__INIT_H__

#if defined(CONFIG_XIP)
#define KERNEL_RO_OFFSET        (VIRT_ADDR_ROM - *(physbase+1))
#else
#define KERNEL_RO_OFFSET        (VIRT_ADDR_RAM - *(physbase))
#endif
#define KERNEL_RW_OFFSET        (VIRT_ADDR_RAM - *(physbase))

/* Before VM is initialized we cannot use virtual addresses
 * of phys_addr_rom and phys_addr_ram.  Get the init code to 
 * pass a pointer to phys_addr_ram, phys_addr_rom is right 
 * above it
 */
template<typename T> INLINE T virt_to_phys_init(T x, word_t *p)
{
#if defined(CONFIG_XIP)
    if ((u32_t)x < VIRT_ADDR_RAM)
        return (T) ((u32_t) x - VIRT_ADDR_ROM + *(p+1));
    else
#endif
        return (T) ((u32_t) x - VIRT_ADDR_RAM + *p);
}

template<typename T> INLINE T phys_to_virt_init(T x, word_t *p)
{
#if defined(CONFIG_XIP)
    word_t pram = *p;
    word_t prom = *(p+1);

    if ( (pram > prom && (word_t)x < pram) || (pram < prom && (word_t)x >= prom))
        return (T) ((u32_t) x + VIRT_ADDR_ROM - *(p+1));
    else
#endif
        return (T) ((u32_t) x + VIRT_ADDR_RAM - *p);
}

#if defined(__GNUC__)
#define switch_to_virt()                                                \
    __asm__ __volatile__ (                                              \
        "add    sp, sp, %0              \n"                             \
        "add    r0, pc, %1              \n"                             \
        "mov    pc, r0                  \n"                             \
        :: "r" (KERNEL_RW_OFFSET), "r" (KERNEL_RO_OFFSET)               \
        : "memory"                                                      \
    )

#define switch_to_phys()                                                \
    __asm__ __volatile__ (                                              \
        "sub    sp, sp, %0              \n"                             \
        "sub    r0, pc, %1              \n"                             \
        "mov    pc, r0                  \n"                             \
        :: "r" (KERNEL_RW_OFFSET), "r" (KERNEL_RO_OFFSET)               \
        :"memory"                                                       \
    );

#define jump_to(address)                                                \
    do {                                                                \
        __asm__ __volatile__ (                                          \
            "mov        pc, %0          \n"                             \
            :: "r" (address)                                            \
        );                                                              \
    } while (1)

#elif defined(__RVCT_GNU__)

extern "C" void rvct_switch_to_virt(word_t *physbase);
extern "C" void rvct_switch_to_phys(word_t *physbase);
extern "C" void rvct_jump_to(word_t address);

#define switch_to_virt()        rvct_switch_to_virt(physbase)
#define switch_to_phys()        rvct_switch_to_phys(physbase)
#define jump_to(address)        rvct_jump_to(address)
#elif defined(_lint)
extern "C" void lint_switch_to_virt(word_t *physbase);
extern "C" void lint_switch_to_phys(word_t *physbase);
extern "C" void lint_jump_to(word_t address);

#define switch_to_virt()        lint_switch_to_virt(physbase)
#define switch_to_phys()        lint_switch_to_phys(physbase)
#define jump_to(address)        lint_jump_to(address)

#else
#error Unknown compiler
#endif


#endif /*__ARCH__ARM__INIT_H__*/
