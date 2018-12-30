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
 * Description:   ARM kernel debugger interface
 */
#ifndef __L4__ARM__KDEBUG_H__
#define __L4__ARM__KDEBUG_H__

#include <l4/types.h>
#include <l4/arch/syscalls.h>

int L4_KDB_PrintChar_ASM(int c);
int L4_KDB_ReadChar_ASM(void);
int L4_KDB_ReadChar_Blocked_ASM(void);
void L4_KDB_SetObjectName_ASM(word_t type, word_t id, const char *name);

word_t L4_KDB_GetTick(void);
word_t L4_KDB_GetNumTracepoints(void);
word_t L4_KDB_GetTracepointCount(word_t);
void L4_KDB_GetTracepointNameIntroMRs(word_t);
void L4_KDB_TCCtrl(word_t);
word_t L4_KDB_PMN_Read(word_t);
void L4_KDB_PMN_Write(word_t, word_t);
word_t L4_KDB_PMN_Ofl_Read(word_t);
void L4_KDB_PMN_Ofl_Write(word_t, word_t);

#ifndef NDEBUG
void L4_KDB_Enter(char * s);
#endif


INLINE okl4_kdb_res_t
L4_KDB_TracepointCountStop(void)
{
    L4_KDB_TCCtrl(0);
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_TracepointCountReset(void)
{
    L4_KDB_TCCtrl(1);
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_PrintChar(char c)
{
    L4_KDB_PrintChar_ASM(c);
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_ReadChar(char *val)
{
    *val = (char)L4_KDB_ReadChar_ASM();
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_ReadChar_Blocked(char *val)
{
    *val = (char)L4_KDB_ReadChar_Blocked_ASM();
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_SetObjectName(word_t type, word_t id, const char *name)
{
    char buffer[L4_KDB_MAX_DEBUG_NAME_LENGTH];
    word_t i;

    /* Copy the name to our buffer. */
    for (i = 0; i < L4_KDB_MAX_DEBUG_NAME_LENGTH; i++) {
        buffer[i] = name[i];
        if (name[i] == '\0') {
            break;
        }
    }

    /* Perform the call. */
    L4_KDB_SetObjectName_ASM(type, id, buffer);
    return OKL4_KDB_RES_OK;
}

INLINE okl4_kdb_res_t
L4_KDB_GetTracepointName(word_t tp, char *dest)
{
    char *src = (char *)&(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET];

    L4_KDB_GetTracepointNameIntroMRs(tp);

    /*
     * Ewww! Another reimplementation of strcpy!
     */
    while (*src)
        *dest++ = *src++;
    *dest = '\0';

    return OKL4_KDB_RES_OK;
}


#endif /* !__L4__ARM__KDEBUG_H__ */
