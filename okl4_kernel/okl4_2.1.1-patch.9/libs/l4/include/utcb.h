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

#ifndef __L4__UTCB_H__
#define __L4__UTCB_H__

#include <l4/types.h>
#include <l4/arch/vregs.h>

#define __L4_NUM_MRS IPC_NUM_MR

/** If we are compiling for user level provide missing types.
 *  WARNING: Make sure these match the size of the L4 types!
 */
typedef L4_SpaceId_t spaceid_t;
typedef L4_ThreadId_t threadid_t;
typedef word_t acceptor_t;
typedef word_t notify_bits_t;
typedef L4_Word8_t u8_t;
typedef L4_Word16_t u16_t;
typedef L4_Word32_t u32_t;

typedef UTCB_STRUCT utcb_t;


INLINE utcb_t *
utcb_base_get(void)
{
    return (utcb_t *)L4_GetUtcbBase();
}

// FIXME: Depricate the below accessor functions - AGW.

/**
 *  Thread Control Registers.
 */

INLINE word_t
__L4_TCR_MyGlobalId(void)
{
    return L4_myselfconst.raw;
}

INLINE word_t
__L4_TCR_ProcessorNo(void)
{
    return utcb_base_get()->processor_no;
}

INLINE word_t
__L4_TCR_UserDefinedHandle(void)
{
    return utcb_base_get()->user_defined_handle;
}

INLINE void
__L4_TCR_Set_UserDefinedHandle(word_t w)
{
    utcb_base_get()->user_defined_handle = w;
}

INLINE word_t
__L4_TCR_ErrorCode(void)
{
    return utcb_base_get()->error_code;
}

INLINE word_t
__L4_TCR_Acceptor(void)
{
    return utcb_base_get()->acceptor;
}

INLINE void
__L4_TCR_Set_Acceptor(word_t w)
{
    utcb_base_get()->acceptor = w;
}

INLINE word_t
__L4_TCR_NotifyMask(void)
{
    return utcb_base_get()->notify_mask;
}

INLINE word_t
__L4_TCR_NotifyBits(void)
{
    return utcb_base_get()->notify_bits;
}

INLINE void
__L4_TCR_Set_NotifyMask(word_t w)
{
    utcb_base_get()->notify_mask = w;
}

INLINE void
__L4_TCR_Set_NotifyBits(word_t w)
{
     utcb_base_get()->notify_bits = w;
}

INLINE void
L4_Set_CopFlag(word_t n)
{
    utcb_base_get()->cop_flags |= (1 << n);
}

INLINE void
L4_Clr_CopFlag(word_t n)
{
    utcb_base_get()->cop_flags &= ~(1 << n);
}

INLINE word_t
L4_PreemptedIP(void)
{
    return utcb_base_get()->preempted_ip;
}

INLINE void
L4_Set_PreemptCallbackIP(word_t ip)
{
    utcb_base_get()->preempt_callback_ip = ip;
}

// FIXME: Specify the shift amounts as a define - AGW.
INLINE L4_Bool_t
L4_EnablePreemptionCallback(void)
{
    L4_Bool_t old = (utcb_base_get()->preempt_flags >> 5) & 1;

    utcb_base_get()->preempt_flags |= (1 << 5);
    return old;
}

// FIXME: Specify the shift amounts as a define - AGW.
INLINE L4_Bool_t
L4_DisablePreemptionCallback(void)
{
    L4_Bool_t old = (utcb_base_get()->preempt_flags >> 5) & 1;

    utcb_base_get()->preempt_flags &= ~(1 << 5);
    return old;
}

INLINE word_t
__L4_TCR_SenderSpace(void)
{
    return utcb_base_get()->sender_space.raw;
}

INLINE word_t
__L4_TCR_ThreadWord(word_t n)
{
    return utcb_base_get()->thread_word[n];
}

INLINE void
__L4_TCR_Set_ThreadWord(word_t n, word_t w)
{
    utcb_base_get()->thread_word[n] = w;
}

INLINE word_t
__L4_TCR_ThreadLocalStorage(void)
{
    return utcb_base_get()->tls_word;
}

INLINE void
__L4_TCR_Set_ThreadLocalStorage(word_t tls)
{
    utcb_base_get()->tls_word = tls;
}

INLINE word_t
__L4_TCR_PlatformReserved(int i)
{
    return utcb_base_get()->platform_reserved[i];
}

INLINE void
__L4_TCR_Set_PlatformReserved(int i, word_t val)
{
    utcb_base_get()->platform_reserved[i] = val;
}

/**
 *  Message Registers.
 */

INLINE void
L4_StoreMR(int i, word_t * w)
{
    *w = utcb_base_get()->mr[i];
}

INLINE void
L4_LoadMR(int i, word_t w)
{
    utcb_base_get()->mr[i] = w;
}

INLINE word_t *
L4_MRStart(void)
{
    return &(utcb_base_get()->mr[0]);
}

INLINE void
L4_StoreMRs(int i, int k, word_t * w)
{
    word_t * mr = utcb_base_get()->mr + i;
    
    while (k-- > 0) {
        *w++ = *mr++;
    }
}

INLINE void
L4_LoadMRs(int i, int k, word_t * w)
{
    word_t * mr = utcb_base_get()->mr + i;
    
    while (k-- > 0) {
        *mr++ = *w++;
    }
}


#endif /* !__L4__UTCB_H__ */
