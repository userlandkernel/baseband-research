/*
 * Copyright (c) 2004, National ICT Australia
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
#ifndef _IG_UTIL_H_
#define _IG_UTIL_H_

#include <util/ansi.h>

#define INVALID_ADDR ((uintptr_t)(~0UL))

#define BASE_LOG2_PAGESIZE 12
#define BASE_PAGESIZE (1 << BASE_LOG2_PAGESIZE)
#define PAGEMASK  (BASE_PAGESIZE - 1)
#define PAGEOFFSETMASK (~PAGEMASK)

#define page_round_up(address) ((address + PAGEMASK) & PAGEOFFSETMASK)
#define round_up(address, size) ((((address) + ((size)-1)) & ~((size)-1)))
#define page_round_down(address) ((address) & PAGEOFFSETMASK)
#define round_down(address, size) ((address) & ~((size)-1))

#define ASSERT_PTR(ptr) assert(((void*) (ptr)) > ((void*) BASE_PAGESIZE))

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

#define MARK (void) printf(ANSI_BLUE "%s:%d %s: " ANSI_NORMAL, __FILE__, __LINE__, __func__)
#define MARK_RED (void) printf(ANSI_RED "%s:%d %s: " ANSI_NORMAL, __FILE__, __LINE__, __func__)

// See SConstruct for iguana - enabled by DEBUG_TRACE option
#ifdef  IG_DEBUG_PRINT
#define ERROR_PRINT MARK_RED; (void) printf
#define DEBUG_PRINT (void) printf
#else
#define ERROR_PRINT(...)
#define DEBUG_PRINT(...)
#endif

#define ERROR_PRINT_L4 ERROR_PRINT("Error: %lx\n", L4_ErrorCode())

// See SConstruct for iguana - enabled by VERBOSE option
#ifdef IG_VERBOSE
#define INIT_PRINT MARK; (void) printf
#define BOOTINFO_PRINT MARK; (void) printf
#define FPAGE_PRINT MARK; (void) printf
#define MEMSECTION_PRINT MARK; (void) printf
#define PD_PRINT MARK; (void) printf
#define SLAB_PRINT MARK; (void) printf
#define QUOTA_PRINT MARK; (void) printf
#define SECURITY_PRINT MARK; (void) printf
#define OBJTABLE_PRINT MARK; (void) printf
#define EXTENSION_PRINT MARK; (void) printf
#else
#define INIT_PRINT(...)
#define BOOTINFO_PRINT(...)
#define FPAGE_PRINT(...)
#define MEMSECTION_PRINT(...)
#define PD_PRINT(...)
#define SLAB_PRINT(...)
#define QUOTA_PRINT(...)
#define SECURITY_PRINT(...)
#define OBJTABLE_PRINT(...)
#define EXTENSION_PRINT(...)
#endif

#if defined(ARM_SHARED_DOMAINS)
#include <l4/types.h>

L4_Word_t l4_share_domain(L4_SpaceId_t client, L4_SpaceId_t share);
L4_Word_t l4_manage_domain(L4_SpaceId_t client, L4_SpaceId_t share);
void l4_unshare_domain(L4_SpaceId_t client, L4_SpaceId_t share);
L4_Word_t l4_map_window(L4_SpaceId_t client, L4_SpaceId_t share, L4_Fpage_t window);
L4_Word_t l4_unmap_window(L4_SpaceId_t client, L4_SpaceId_t share, L4_Fpage_t window);
#endif

#endif /* _IG_UTIL_H_ */
