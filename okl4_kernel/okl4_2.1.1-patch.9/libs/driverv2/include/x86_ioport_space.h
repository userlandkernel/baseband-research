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

#ifndef _DRIVER_x86_IOPORT_SPACE_T
#define _DRIVER_x86_IOPORT_SPACE_T

typedef uint16_t x86_ioport_space_t;

#if defined (__x86_64__) || defined(__i386__)

static inline uint32_t
x86_ioport_space_read_32(x86_ioport_space_t ignored, int offset)
{
    uint32_t tmp;
    __asm__ __volatile__("inl %w1, %0" : "=a"(tmp) : "d"(offset));
    return tmp;
}

static inline void
x86_ioport_space_write_32(x86_ioport_space_t ignored, int offset, uint32_t value)
{
    __asm__ __volatile__("outl %0, %w1" : : "a"(value), "d"(offset));
}

static inline uint16_t
x86_ioport_space_read_16(x86_ioport_space_t ignored, int offset)
{
    uint16_t tmp;
    __asm__ __volatile__("inw %w1, %0" : "=a"(tmp) : "d"(offset));
    return tmp;
}

static inline void
x86_ioport_space_write_16(x86_ioport_space_t ignored, int offset, uint16_t value)
{
    __asm__ __volatile__("outw %0, %w1" : : "a"(value), "d"(offset));
}

static inline uint8_t
x86_ioport_space_read_8(x86_ioport_space_t ignored, int offset)
{
    uint8_t tmp;
    __asm__ __volatile__("inb %w1, %0" : "=a"(tmp) : "d"(offset));
    return tmp;
}

static inline void
x86_ioport_space_write_8(x86_ioport_space_t ignored, int offset, uint8_t value)
{
    __asm__ __volatile__("outb %0, %w1" : : "a"(value), "d"(offset));
}

static inline void
x86_ioport_space_write_seq_32(x86_ioport_space_t ignored, int offset,
                               void *addr, uint32_t count)
{
    int i;
    uint32_t *mem_base = addr;

    for (i = 0; i < count; i++) {
        __asm__ __volatile__("outl %0, %w1" : : "a"(*((uint32_t *)(mem_base + i))), "d"(offset));
    }
}

static inline void
x86_ioport_space_read_seq_32(x86_ioport_space_t ignored, int offset,
                              void *addr, uint32_t count)
{
    int i;

    for(i = 0; i < count; i++){
        __asm__ __volatile__("inl %w1, %0" : "=a"(*((uint32_t *)addr + i)) : "d"(offset));
    }
}

static inline void
x86_ioport_space_write_seq_16(x86_ioport_space_t ignored, int offset,
                               void *addr, uint32_t count)
{
    int i;
    uint16_t *mem_base = addr;

    for (i = 0; i < count; i++) {
        __asm__ __volatile__("outw %0, %w1" : : "a"(*((uint16_t *)(mem_base + i))), "d"(offset));
    }
}

static inline void
x86_ioport_space_read_seq_16(x86_ioport_space_t ignored, int offset,
                              void *addr, uint32_t count)
{
    int i;

    for(i = 0; i < count; i++){
        __asm__ __volatile__("inw %w1, %0" : "=a"(*((uint16_t *)addr + i)) : "d"(offset));
    }
}
#endif /* _DRIVER_x86_IOPORT_SPACE_T */
#endif /* defined (__x86_64__) || defined(__i386__) */
