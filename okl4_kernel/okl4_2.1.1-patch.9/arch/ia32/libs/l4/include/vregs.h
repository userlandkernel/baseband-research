/*
 * Copyright (c) 2001-2003, Karlsruhe University
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

/** @file
 *  @brief UTCB (virtual register) layout for x86 (32-bit only).
 */

#ifndef __L4__X86__VREGS_H__
#define __L4__X86__VREGS_H__

/**
 *  Number of message registers.
 */
#define IPC_NUM_MR 64

/**
 *  Number of thread words copied by kernel.
 */
#define THREAD_WORDS_COPIED 4


/** UTCB structure layout.
 *  WARNING: Keep inline with the TCR locations.
 */
#define UTCB_STRUCT struct {                                            \
        word_t              padding0[32];           /* -256 .. -132 */  \
        word_t              platform_reserved[__L4_TCR_PLATFORM_NUM];   /* -128 .. -96 */   \
        word_t              syscall_args[3];        /* -92 .. -84 */    \
        word_t              tls_word;               /* -80  */          \
        /* Thread private words.                       -76          */  \
        word_t              thread_word[__L4_TCR_USER_NUM]; /* -76 .. -64 */ \
        spaceid_t           sender_space;           /* -60          */  \
        threadid_t          mutex_thread_handle;    /* -56          */  \
        word_t              notify_mask;            /* -52          */  \
        notify_bits_t       notify_bits;            /* -48          */  \
        acceptor_t          acceptor;               /* -44          */  \
        word_t              processor_no;           /* -40          */  \
        word_t              user_defined_handle;    /* -36          */  \
        u8_t                preempt_flags;          /* -32          */  \
        u8_t                cop_flags;                                  \
        u16_t               reserved0;                                  \
        word_t              error_code;             /* -28          */  \
        threadid_t          __unused[2];            /* -24 .. -20   */  \
        word_t              preempt_callback_ip;    /* -16          */  \
        word_t              preempted_ip;           /* -12          */  \
        word_t              reserved1[2];           /* -8 .. -4     */  \
        word_t              mr[IPC_NUM_MR];         /* 0 .. 252     */  \
    }


/*
 * Location of TCRs within UTCB.
 */

#define __L4_TCR_USER_NUM                       (4)
#define __L4_TCR_PLATFORM_NUM                   (9)

#define __L4_TCR_PREEMPTED_IP                   (-3)
#define __L4_TCR_PREEMPT_CALLBACK_IP            (-4)
#define __L4_TCR_ERROR_CODE                     (-7)
#define __L4_TCR_PREEMPT_FLAGS                  (-8)
#define __L4_TCR_COP_FLAGS                      (-8)
#define __L4_TCR_USER_DEFINED_HANDLE            (-9)
#define __L4_TCR_PROCESSOR_NO                   (-10)
#define __L4_TCR_ACCEPTOR                       (-11)
#define __L4_TCR_NOTIFY_BITS                    (-12)
#define __L4_TCR_NOTIFY_MASK                    (-13)
#define __L4_TCR_SENDER_SPACE                   (-15)
#define __L4_TCR_THREAD_WORD_0                  (-16)
#define __L4_TCR_THREAD_WORD_1                  (-17)
#define __L4_TCR_THREAD_WORD_2                  (-18)
#define __L4_TCR_THREAD_WORD_3                  (-19)
#define __L4_TCR_PLAT_TLS                       (-20)
#define __L4_TCR_SYSCALL_ARGS                   (-23)

#define __L4_TCR_PLATFORM_OFFSET                (-24)


/*
 * Note: this has to correspond to the value specified in the 
 * kernel arch/config.h.
 */
#define L4_UTCB ((9 << 3)/*idx9*/ | (0 << 2)/*!local*/ | 3/*rpl3*/)

/*
 * All virtual registers on x86 is located in a user Level Thread
 * Control Block (UTCB).
 */

#if !defined(__ASSEMBLER__) && !defined(ASSEMBLY)

#if (__GNUC__ >= 3) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96))
INLINE word_t *__L4_X86_Utcb(void) __attribute__ ((pure));
INLINE word_t * L4_GetUtcbBase(void) __attribute__ ((pure));
#endif


INLINE word_t *
__L4_X86_Utcb(void)
{
    word_t * utcb;

    __asm__ __volatile__(
             "   movl %%gs:0, %[retval]              \n"
             : [retval] "=r" (utcb)
        );

    return utcb;
}

INLINE L4_Word_t *
L4_GetUtcbBase(void)
{
    /*
     * want to return beginning of UTCB, not half way through
     */
    return __L4_X86_Utcb() - (0x100 >> 2);
}

#endif

    
#endif /* !__L4__X86__VREGS_H__ */
