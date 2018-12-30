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

/* These numbers MUST be multiple of 4 */
#define SYSCALL_ipc                 0x0
#define SYSCALL_thread_switch       0x4
#define SYSCALL_thread_control      0x8
#define SYSCALL_exchange_registers  0xc
#define SYSCALL_schedule            0x10
#define SYSCALL_map_control         0x14
#define SYSCALL_space_control       0x18
/* Unused syscall slot:             0x1c */
#define SYSCALL_cache_control       0x20
#define SYSCALL_security_control    0x24
#define SYSCALL_lipc                0x28
#define SYSCALL_platform_control    0x2c
#define SYSCALL_space_switch        0x30
#define SYSCALL_mutex               0x34
#define SYSCALL_mutex_control       0x38
#define SYSCALL_interrupt_control   0x3c
#define SYSCALL_cap_control         0x40
#define SYSCALL_memory_copy         0x44
#define SYSCALL_last                0x44

#define L4_TRAP_KPUTC               0xa0
#define L4_TRAP_KGETC               0xa4
#define L4_TRAP_KGETC_NB            0xa8
#define L4_TRAP_KDEBUG              0xac
#define L4_TRAP_GETUTCB             0xb0
/* L4_TRAP_KIP  was 0xb4 */
#define L4_TRAP_KSET_OBJECT_NAME    0xb8
#define L4_TRAP_GETCOUNTER          0xbc
#define L4_TRAP_GETNUMTPS           0xc0
#define L4_TRAP_GETTPNAME           0xc4
#define L4_TRAP_TCCTRL              0xc8

#define L4_TRAP_PMN_READ            0xcc
#define L4_TRAP_PMN_WRITE           0xd0
#define L4_TRAP_PMN_OFL_READ        0xd4
#define L4_TRAP_PMN_OFL_WRITE       0xd8

#define L4_TRAP_GETTICK             0xe0

#define SYSBASE                     0xffffff00
#define SWIBASE                     0x1400

#define SYSNUM(name)                (SYSBASE + SYSCALL_ ## name)
#define SWINUM(name)                (SWIBASE + SYSCALL_ ## name)
#define TRAPNUM(name)               (SYSBASE + name)
