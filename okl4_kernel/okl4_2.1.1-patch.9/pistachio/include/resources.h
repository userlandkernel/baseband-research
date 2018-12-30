/*
 * Copyright (c) 2002, 2003, Karlsruhe University
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
 * Description:
 */
#ifndef __API__V4__RESOURCES_H__
#define __API__V4__RESOURCES_H__

#include <kernel/bitmask.h>


class tcb_t;

/*
 * ThreadControl resource bits encoding
 */
class tc_resources_t
{
public:
    union {
        word_t raw;
        struct {
            BITFIELD2(word_t,
                      arch  : BITS_WORD - 1,
                      valid : 1);
        } bits;
    };
};

/*
 * Generic thread resources inherited by threads
 */
class generic_thread_resources_t
{
public:
    void dump(tcb_t * tcb) { }
    void save(tcb_t * tcb) { }
    void load(tcb_t * tcb) { }
    void purge(tcb_t * tcb) { }
    void init(tcb_t * tcb) { }
    void free(tcb_t * tcb) { }
public:
    bool control(tcb_t * tcb, tc_resources_t resources)
        {
            //get_current_tcb()->set_error_code(EINVALID_PARAM);
            //return false;
            return true;
        }
};

#include <kernel/arch/resources.h>


#if !defined(HAVE_RESOURCE_TYPE_E)
typedef word_t  resource_bits_t;
#else


/**
 * Abstract class for handling resource bit settings.
 */
class resource_bits_t
{
    bitmask_t   resource_bits;

public:

    /**
     * Intialize resources (i.e., clear all resources).
     */
    inline void init (void)
        { resource_bits = 0; }

    /**
     * Clear all resources.
     */
    inline void clear (void)
        { resource_bits = 0; }

    /**
     * Add resource to resource bits.
     * @param t         type of resource
     * @return new resource bits
     */
    inline resource_bits_t operator += (resource_type_e t)
        {
            resource_bits += (int) t;
            return *this;
        }

    /**
     * Remove resource from resource bits.
     * @param t         type of resource
     * @return new resource bits
     */
    inline resource_bits_t operator -= (resource_type_e t)
        {
            resource_bits -= (int) t;
            return *this;
        }

    /**
     * Check if any resouces are registered.
     * @return true if any resources are registered, false otherwise
     */
    bool have_resources (void)
        {
            return (word_t) resource_bits != 0;
        }

    /**
     * Check if indicated resource is registered.
     * @param t         type of resource
     * @return true if resource is registered, false otherwise
     */
    bool have_resource (resource_type_e t)
        {
            return resource_bits.is_set ((int) t);
        }

    /**
     * Convert resource bits to a word (e.g., for printing).
     * @return the resource mask
     */
    inline operator word_t (void)
        {
            return (word_t) resource_bits;
        }
};

#endif

#endif /* !__API__V4__RESOURCES_H__ */
