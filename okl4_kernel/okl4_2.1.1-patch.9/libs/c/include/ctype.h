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
 * Authors: Cristan Szmadja, Ben Leslie
 * Description: Implements character handling as per 7.4 
 * Status: Incomplete Ignores locale
 */

#ifndef _CTYPE_H_
#define _CTYPE_H_

static inline int
iscntrl(int c)
{
    return (0 <= c && c <= '\037') || c == '\177';
}

static inline int
isgraph(int c)
{
    return ' ' < c && c < '\177';
}

static inline int
isprint(int c)
{
    return ' ' <= c && c < '\177';
}

static inline int
isdigit(int c)
{
    return '0' <= c && c <= '9';
}

static inline int
islower(int c)
{
    return 'a' <= c && c <= 'z';
}

static inline int
isspace(int c)
{
    return c == ' ' || ('\011' <= c && c <= '\015');
}

static inline int
isblank(int c)
{
    return (c == ' ' || c == '\t');
}

static inline int
isupper(int c)
{
    return 'A' <= c && c <= 'Z';
}

static inline int
isalpha(int c)
{
    return isupper(c) || islower(c);
}

static inline int
isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

static inline int
ispunct(int c)
{
    return isgraph(c) && !isalnum(c);
}

static inline int
isxdigit(int c)
{
    return isdigit(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

static inline int
tolower(int c)
{
    return isupper(c) ? c + ('a' - 'A') : c;
}

static inline int
toupper(int c)
{
    return islower(c) ? c - ('a' - 'A') : c;
}

static inline int
isascii(int c)
{
    return 0 <= c && c <= '\177';
}

static inline int
toascii(int c)
{
    return c & 0177;
}

static inline int
digittoint(int c)
{
    return isdigit(c) ? c - '0' : 'A' <= c
        && c <= 'Z' ? c - 'A' : 'a' <= c && c <= 'z' ? c - 'a' : 99;
}

#endif /* !_CTYPE_H_ */

/*
 * XXX: Extra things needed by POSIX... need to find a correct #ifndef 
 */

/*
 * 
 * int isascii(int); int toascii(int);
 * 
 * #define _toupper(x) #define _tolower(x)
 * 
 * 
 */
