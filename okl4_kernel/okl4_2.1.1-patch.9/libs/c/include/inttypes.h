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

/*
 * Authors: Ben Leslie 
 */

#ifndef _INT_TYPES_
#define _INT_TYPES_

#include <stdint.h>

#include <arch/inttypes.h>

/* 7.8.1 Macros for format specifies */

/* 7.8.1.2 signed integers */
#define PRId8 __LENGTH_8_MOD "d"
#define PRId16 __LENGTH_16_MOD "d"
#define PRId32 __LENGTH_32_MOD "d"
#define PRId64 __LENGTH_64_MOD "d"

#define PRIi8 __LENGTH_8_MOD "i"
#define PRIi16 __LENGTH_16_MOD "i"
#define PRIi32 __LENGTH_32_MOD "i"
#define PRIi64 __LENGTH_64_MOD "i"

#define PRIdLEAST8 __LENGTH_LEAST8_MOD "d"
#define PRIdLEAST16 __LENGTH_LEAST16_MOD "d"
#define PRIdLEAST32 __LENGTH_LEAST32_MOD "d"
#define PRIdLEAST64 __LENGTH_LEAST64_MOD "d"

#define PRIiLEAST8 __LENGTH_LEAST8_MOD "i"
#define PRIiLEAST16 __LENGTH_LEAST16_MOD "i"
#define PRIiLEAST32 __LENGTH_LEAST32_MOD "i"
#define PRIiLEAST64 __LENGTH_LEAST64_MOD "i"

#define PRIdFAST8 __LENGTH_FAST8_MOD "d"
#define PRIdFAST16 __LENGTH_FAST16_MOD "d"
#define PRIdFAST32 __LENGTH_FAST32_MOD "d"
#define PRIdFAST64 __LENGTH_FAST64_MOD "d"

#define PRIiFAST8 __LENGTH_FAST8_MOD "i"
#define PRIiFAST16 __LENGTH_FAST16_MOD "i"
#define PRIiFAST32 __LENGTH_FAST32_MOD "i"
#define PRIiFAST64 __LENGTH_FAST64_MOD "i"

#define PRIdMAX __LENGTH_MAX_MOD "d"
#define PRIiMAX __LENGTH_MAX_MOD "i"

#define PRIdPTR __LENGTH_PTR_MOD "d"
#define PRIiPTR __LENGTH_PTR_MOD "i"

/* 7.8 __LENGTH_8_MOD.1.3 unsigned integers */

#define PRIo8 __LENGTH_8_MOD "o"
#define PRIo16 __LENGTH_16_MOD "o"
#define PRIo32 __LENGTH_32_MOD "o"
#define PRIo64 __LENGTH_64_MOD "o"

#define PRIu8 __LENGTH_8_MOD "u"
#define PRIu16 __LENGTH_16_MOD "u"
#define PRIu32 __LENGTH_32_MOD "u"
#define PRIu64 __LENGTH_64_MOD "u"

#define PRIx8 __LENGTH_8_MOD "x"
#define PRIx16 __LENGTH_16_MOD "x"
#define PRIx32 __LENGTH_32_MOD "x"
#define PRIx64 __LENGTH_64_MOD "x"

#define PRIX8 __LENGTH_8_MOD "X"
#define PRIX16 __LENGTH_16_MOD "X"
#define PRIX32 __LENGTH_32_MOD "X"
#define PRIX64 __LENGTH_64_MOD "X"

#define PRIoLEAST8 __LENGTH_LEAST8_MOD "o"
#define PRIoLEAST16 __LENGTH_LEAST16_MOD "o"
#define PRIoLEAST32 __LENGTH_LEAST32_MOD "o"
#define PRIoLEAST64 __LENGTH_LEAST64_MOD "o"

#define PRIuLEAST8 __LENGTH_LEAST8_MOD "u"
#define PRIuLEAST16 __LENGTH_LEAST16_MOD "u"
#define PRIuLEAST32 __LENGTH_LEAST32_MOD "u"
#define PRIuLEAST64 __LENGTH_LEAST64_MOD "u"

#define PRIxLEAST8 __LENGTH_LEAST8_MOD "x"
#define PRIxLEAST16 __LENGTH_LEAST16_MOD "x"
#define PRIxLEAST32 __LENGTH_LEAST32_MOD "x"
#define PRIxLEAST64 __LENGTH_LEAST64_MOD "x"

#define PRIXLEAST8 __LENGTH_LEAST8_MOD "X"
#define PRIXLEAST16 __LENGTH_LEAST16_MOD "X"
#define PRIXLEAST32 __LENGTH_LEAST32_MOD "X"
#define PRIXLEAST64 __LENGTH_LEAST64_MOD "X"

#define PRIoFAST8 __LENGTH_FAST8_MOD "o"
#define PRIoFAST16 __LENGTH_FAST16_MOD "o"
#define PRIoFAST32 __LENGTH_FAST32_MOD "o"
#define PRIoFAST64 __LENGTH_FAST64_MOD "o"

#define PRIuFAST8 __LENGTH_FAST8_MOD "u"
#define PRIuFAST16 __LENGTH_FAST16_MOD "u"
#define PRIuFAST32 __LENGTH_FAST32_MOD "u"
#define PRIuFAST64 __LENGTH_FAST64_MOD "u"

#define PRIxFAST8 __LENGTH_FAST8_MOD "x"
#define PRIxFAST16 __LENGTH_FAST16_MOD "x"
#define PRIxFAST32 __LENGTH_FAST32_MOD "x"
#define PRIxFAST64 __LENGTH_FAST64_MOD "x"

#define PRIXFAST8 __LENGTH_FAST8_MOD "X"
#define PRIXFAST16 __LENGTH_FAST16_MOD "X"
#define PRIXFAST32 __LENGTH_FAST32_MOD "X"
#define PRIXFAST64 __LENGTH_FAST64_MOD "X"

#define PRIoMAX __LENGTH_MAX_MOD "o"
#define PRIuMAX __LENGTH_MAX_MOD "u"
#define PRIxMAX __LENGTH_MAX_MOD "x"
#define PRIXMAX __LENGTH_MAX_MOD "X"

#define PRIoPTR __LENGTH_PTR_MOD "o"
#define PRIuPTR __LENGTH_PTR_MOD "u"
#define PRIxPTR __LENGTH_PTR_MOD "x"
#define PRIXPTR __LENGTH_PTR_MOD "X"

#endif
