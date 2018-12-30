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
 * Description:   Interfaces for handling Kernel Mutexes. 
 */
#ifndef __L4__MUTEX_H__
#define __L4__MUTEX_H__

#include <stddef.h>
#include <l4/types.h>
#include <l4/arch/syscalls.h>

/*
 *  Derived Mutex functions
 */

/* Kernel-only mutex functions. */

/**
 * Lock the 'm' or block until it is aquired.
 *
 * @param m The mutex being aquired.
 *
 * @retval 1 Operation successful.
 * @retval 0 Operation failed, see error code.
 */
#if !defined(CONFIG_HYBRID_MUTEXES)
INLINE word_t
L4_Lock(L4_MutexId_t m)
{
    return L4_Mutex(m, L4_Mutex_Aquire | L4_Mutex_Blocking, NULL);
}

/**
 * Try to lock 'm' or return with error if it is unavailable.
 *
 * @param m The mutex being aquired.
 *
 * @retval 1 Operation successful.
 * @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_TryLock(L4_MutexId_t m)
{
    return L4_Mutex(m, L4_Mutex_Aquire | L4_Mutex_NonBlocking, NULL);
}

/**
 * Unlock previously locked 'm'. 
 *
 * @param m The mutex being aquired.
 *
 * @retval 1 Operation successful.
 * @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_Unlock(L4_MutexId_t m)
{
    return L4_Mutex(m, L4_Mutex_Release | L4_Mutex_NonBlocking, NULL);
}
#endif /* !CONFIG_HYBRID_MUTEXES */

/* Hybrid mutex functions */

/**
 *  Lock hybrid mutex 'm' stored at 'state_p' and block until it is aquired.
 *
 *  @param m The mutex being aquired.
 *  @param state_p The address of the hybrid mutex atomic word.
 *
 *  @retval 1 Operation successful.
 *  @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_HybridLock(L4_MutexId_t m, word_t * state_p)
{
    return L4_Mutex(m, L4_Mutex_Aquire | L4_Mutex_Blocking | L4_Mutex_Hybrid,
            state_p);
}

/**
 *  Unlock previously locked hybrid mutex 'm' stored at 'state_p'.
 *
 *  @param m The mutex being released.
 *  @param state_p The address of the hybrid mutex atomic word.
 *
 *  @retval 1 Operation successful.
 *  @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_HybridUnlock(L4_MutexId_t m, word_t * state_p)
{
    return L4_Mutex(m,
            L4_Mutex_Release | L4_Mutex_NonBlocking | L4_Mutex_Hybrid, state_p);
}

/*
 *  Derived Mutex Control functions.
 */

/**
 * Initialise a new mutex with the identifier 'm'.
 *
 * @param m The mutex being aquired.
 *
 * @retval 1 Operation successful.
 * @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_CreateMutex(L4_MutexId_t m)
{
    return L4_MutexControl(m, L4_MutexCtrl_Create);
}
              
/**
 * Tear down the mutex with the identifier 'm'.
 *
 * @param m The mutex being aquired.
 *
 * @retval 1 Operation successful.
 * @retval 0 Operation failed, see error code.
 */
INLINE word_t
L4_DeleteMutex(L4_MutexId_t m)
{
    return L4_MutexControl(m, L4_MutexCtrl_Delete);
}


#endif /* !__L4_MUTEX_H__ */
