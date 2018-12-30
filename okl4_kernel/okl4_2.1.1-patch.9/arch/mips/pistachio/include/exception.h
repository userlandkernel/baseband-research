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
 * Description:   Exception IPC message definitions.
 * Author:        Carl van Schaik
 */

#ifndef __ARCH__MIPS__EXCEPTION_H__
#define __ARCH__MIPS__EXCEPTION_H__

/*
 * Generic exception message format
 * All exceptions not handled by the kernel or other exception
 * message.
 */

#define EXCEPT_IPC_GEN_MR_IP            1
#define EXCEPT_IPC_GEN_MR_SP            2
#define EXCEPT_IPC_GEN_MR_FLAGS         3
#define EXCEPT_IPC_GEN_MR_EXCEPTNO      4
#define EXCEPT_IPC_GEN_MR_ERRORCODE     5
#define EXCEPT_IPC_GEN_MR_NUM           5

#if defined(L4_64BIT)
#define EXCEPTION_TAG                 (0xffffffffffffffb0UL) /* -5 */
#elif defined(L4_32BIT)
#define EXCEPTION_TAG                 (0xffffffb0UL) /* -5 */
#else
#error Must define L4_64BIT or L4_32BIT
#endif

#define EXCEPT_IPC_GEN_TAG              ((EXCEPTION_TAG << 16) | EXCEPT_IPC_GEN_MR_NUM)

/*
 * System call exception
 * System calls not handled by L4 are redirected via system call exception IPC
 */

#define EXCEPT_IPC_SYS_MR_IP            1
#define EXCEPT_IPC_SYS_MR_SP            2
#define EXCEPT_IPC_SYS_MR_FLAGS         3
#define EXCEPT_IPC_SYS_MR_V0            4
#define EXCEPT_IPC_SYS_MR_V1            5
#define EXCEPT_IPC_SYS_MR_A0            6
#define EXCEPT_IPC_SYS_MR_A1            7
#define EXCEPT_IPC_SYS_MR_A2            8
#define EXCEPT_IPC_SYS_MR_A3            9
#define EXCEPT_IPC_SYS_MR_A4            10
#define EXCEPT_IPC_SYS_MR_A5            11
#define EXCEPT_IPC_SYS_MR_A6            12
#define EXCEPT_IPC_SYS_MR_A7            13
#define EXCEPT_IPC_SYS_MR_NUM           13

#define EXCEPT_IPC_SYS_LABEL            (-5UL << 4)
#define EXCEPT_IPC_SYS_TAG              ((EXCEPT_IPC_SYS_LABEL << 16) | EXCEPT_IPC_SYS_MR_NUM)

#endif /* __ARCH__MIPS__EXCEPTION_H__ */
