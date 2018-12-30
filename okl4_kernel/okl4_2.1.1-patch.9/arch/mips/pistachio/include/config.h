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
 * Copyright (c) 2005-2006, National ICT Australia (NICTA)
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
 * Description:   MIPS Config
 * Author:        Carl van Schaik
 */ 

#ifndef __ARCH__MIPS__CONFIG_H__
#define __ARCH__MIPS__CONFIG_H__

#include <kernel/l4.h>
#include <kernel/arch/page.h>

#if defined(CONFIG_IS_32BIT)
# define IF_MIPS32(...)         __VA_ARGS__
# define IF_MIPS64(...)
#elif defined(CONFIG_IS_64BIT)
# define IF_MIPS32(...)
# define IF_MIPS64(...)         __VA_ARGS__
#endif

/**
 * KTCB config
 */
#define KTCB_ALIGN      32

/**
 * Size of a UTCB in bytes
 */
#if defined(L4_32BIT)
#define UTCB_BITS       9
#else
#define UTCB_BITS       10
#endif
#define UTCB_SIZE       (1UL << UTCB_SIZE)
#define UTCB_MASK       (~((1 << UTCB_BITS) - 1))
#define UTCB_PGSIZE     pgent_t::size_4k

#define STACK_BITS      11
#define STACK_SIZE      (1<<STACK_BITS)
#define STACK_TOP       (STACK_SIZE-L4_WORD_SIZE)

/* Page size for small object allocator */
#define SMALL_OBJECT_BLOCKSIZE  (1UL << MIPS_PAGE_BITS)

/**
   attributes for system call functions
   @param x is the name of the system call lacking the leading sys_ .
   This makes it possible to place every system call in its own section
   if required. Default is empty.
 */
#define KIP_SYSCALL(x)          ((word_t) (x) - (word_t) &kip)

/*
 * Values for Kernel Interface Page (KIP).
 */
#if defined(L4_32BIT)
  #if defined(CONFIG_BIGENDIAN)
    #define KIP_API_FLAGS       {SHUFFLE2(1, 0)} /* 32-bit, big endian */
  #else
    #define KIP_API_FLAGS       {SHUFFLE2(0, 0)} /* 32-bit, little endian */
  #endif
#elif defined(L4_64BIT)
  #if defined(CONFIG_BIGENDIAN)
    #define KIP_API_FLAGS       {SHUFFLE2(1,1)} /* 64-bit, big endian */
  #else
    #define KIP_API_FLAGS       {SHUFFLE2(0,1)} /* 64-bit, little endian */
  #endif
#endif


/*
 * minimum size of UTCB area and number of UTCBs in this
 */
/* 8 byte aligned, 1KB size, 4KB area size */
#define KIP_UTCB_SIZE           MIPS_PAGE_BITS
#define KIP_UTCB_INFO           {SHUFFLE3(1, UTCB_BITS, KIP_UTCB_SIZE)}

/*
 * size of kernel interface page
 */
#define KIP_KIP_AREA    MIPS_PAGE_BITS  /* 4KB */

#ifdef L4_32BIT
#define KIP_ARCH_PAGEINFO {SHUFFLE2(2, HW_VALID_PGSIZES >> 10)}
#elif defined (L4_64BIT)
#define KIP_ARCH_PAGEINFO {SHUFFLE2(2, HW_VALID_PGSIZES >> 10)}
#endif


#ifdef L4_32BIT
extern word_t _memory_descriptors_offset[];
/* MIPS32 has bugs inserting 16-bit relocations, hardcode size here */
#define KIP_MEM_DESC_SIZE       ((word_t) 32)       // XXX
#define KIP_MEM_DESC_OFFSET     ((word_t) &_memory_descriptors_offset)
#endif

#define VALID_THREADNO_BITS     (CONFIG_ROOT_CAP_BITS)
#define VALID_THREADNO_MASK     ((1UL << VALID_THREADNO_BITS)-1)

/* Shared */

#if defined(L4_32BIT)

#define USER_AREA_START         AS_KUSEG_START
#define USER_AREA_END           AS_KUSEG_END

#define ROOT_UTCB_START         ((1UL << 31) - 0x2000000)
#define ROOT_KIP_START          ((1UL << 31) - 0x1000000)

#elif defined(L4_64BIT)

#define CPU_AREA_START          (AS_CKSSEG_START)
#define CPU_AREA_SIZE           (64*1024)
#define CPU_AREA_END            (CPU_AREA_START + CPU_AREA_SIZE)

#define USER_AREA_START         AS_XKUSEG_START
#define USER_AREA_END           AS_XKUSEG_END

#define ROOT_UTCB_START         (1UL << 32)
#define ROOT_KIP_START          (1UL << 33)

#endif

#define CACHE_LINE_SIZE         (CONFIG_MIPS_CACHE_LINE_SIZE)

/* Tracebuffer size */
#define TBUFF_SIZE              (CONFIG_TRACEBUF_PAGES * MIPS_PAGE_BITS)

/* Number of usec in a timer tick. NB CONFIG_CPU_CLOCK_SPEED in kHz!! */
#ifdef CONFIG_PLAT_SB1
# define TIMER_PERIOD           (CONFIG_CPU_CLOCK_SPEED*2)
#else
# define TIMER_PERIOD           (CONFIG_CPU_CLOCK_SPEED)
#endif
/* This works because:
 * 500 ints/sec,
 * CPU_CLOCK_SPEED/2 timer ticks/sec
 * PERIOD = (CPU_CLOCK_SPEED/2)/500
 *        = CPU_CLOCK_SPEED/1000
 *        = CONFIG_CPU_CLOCK (kHz)
 */

#define TIMER_TICK_LENGTH       (2000) /* usec */

#endif /* !__ARCH__MIPS__CONFIG_H__ */
