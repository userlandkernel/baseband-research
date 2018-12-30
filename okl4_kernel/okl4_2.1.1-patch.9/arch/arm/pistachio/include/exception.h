/*
 * Copyright (c) 2004-2005, National ICT Australia (NICTA)
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
 * Description:
 */


#ifndef __GLUE__V4_ARM__EXCEPTION_H__
#define __GLUE__V4_ARM__EXCEPTION_H__

#define PC(x)           (x & (~1UL))

//#include <arch/types.h>

/*
 * Generic exception message format
 * All exceptions not handled by the kernel or other exception
 * messages.
 */

#define EXCEPT_IPC_GEN_MR_IP            1
#define EXCEPT_IPC_GEN_MR_SP            2
#define EXCEPT_IPC_GEN_MR_FLAGS         3
#define EXCEPT_IPC_GEN_MR_EXCEPTNO      4
#define EXCEPT_IPC_GEN_MR_ERRORCODE     5
#define EXCEPT_IPC_GEN_MR_NUM           5

#if defined(L4_32BIT)
#define EXCEPTION_TAG                   0xffffffb0UL
#elif defined(L4_64BIT)
#define EXCEPTION_TAG                   0xffffffffffffffb0UL
#else
#error Unknown wordsize
#endif

#define EXCEPT_IPC_GEN_TAG              ((EXCEPTION_TAG << 16) | EXCEPT_IPC_GEN_MR_NUM)

/* Extention for VFP exceptions */

#define EXCEPT_IPC_VFP_MR_FPINST        6
#define EXCEPT_IPC_VFP_MR_FPSCR         7
#define EXCEPT_IPC_VFP_MR_FPINST2       8
#define EXCEPT_IPC_VFP_MR_NUM           8

#define EXCEPT_IPC_VFP_TAG              ((EXCEPT_IPC_GEN_LABEL << 16) | EXCEPT_IPC_VFP_MR_NUM)

/*
 * System call exception
 */
#define EXCEPT_IPC_SYS_MR_R4            1
#define EXCEPT_IPC_SYS_MR_R5            2
#define EXCEPT_IPC_SYS_MR_R6            3
#define EXCEPT_IPC_SYS_MR_R7            4
#define EXCEPT_IPC_SYS_MR_R0            5
#define EXCEPT_IPC_SYS_MR_R1            6
#define EXCEPT_IPC_SYS_MR_R2            7
#define EXCEPT_IPC_SYS_MR_R3            8
#define EXCEPT_IPC_SYS_MR_PC            9
#define EXCEPT_IPC_SYS_MR_SP            10
#define EXCEPT_IPC_SYS_MR_LR            11
#define EXCEPT_IPC_SYS_MR_SYSCALL       12
#define EXCEPT_IPC_SYS_MR_FLAGS         13
#define EXCEPT_IPC_SYS_MR_NUM           13

#define EXCEPT_IPC_SYS_TAG              ((EXCEPTION_TAG << 16) | EXCEPT_IPC_SYS_MR_NUM)

#define EXCEPT_IPC_SYS_TAG_HI           (-5)
#define EXCEPT_IPC_SYS_TAG_LO           (EXCEPT_IPC_SYS_MR_NUM)

#endif /* __GLUE__V4_ARM__EXCEPTION_H__ */
