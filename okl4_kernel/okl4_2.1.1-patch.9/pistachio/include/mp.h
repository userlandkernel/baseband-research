/*
 * Copyright (c) 2006, National ICT Australia
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
 * Description:   Multi Processor support
 */
#ifndef __GENERIC__MP_H__
#define __GENERIC__MP_H__

#include <kernel/types.h>
#include <kernel/debug.h>

/*
 * This class is:
 *  - an interface description
 *  - a porting helper, the real mp_t can be derived from it
 *
 *   NOTE: If we are not building an MP kernel, then we instansiate a empty version, with
 *   no over-ridden members that will give us uniprocessor support by default
 */


static int generic_execution_units[1] = {1};

class generic_mp_t
{
public:

    /* Does what ever is necessary to find the amount of packages/cores/hwthreads. */
    void probe() {
        execution_units = generic_execution_units;
    }

    void print_info() {
        TRACE_INIT("domain pairs:");
        for(int i = 0; i < 1; i++)
            TRACE_INIT(" (%d, %d)", i, execution_units[i]);
        TRACE_INIT("\n");
    }

    /* These may be hardcoded for a particular platform or include specific
     * code to find the corerct values */
    inline unsigned int get_num_scheduler_domains()      { return 1; }

    inline void startup_context(cpu_context_t context)  { }
    inline void stop_context(cpu_context_t context)     { }
    inline void interrupt_context(cpu_context_t c) { UNIMPLEMENTED();}

protected:
    int * execution_units;
};


#if defined(CONFIG_MDOMAINS) || defined(CONFIG_MUNITS)
/* If we are a multi processing system of any variety, include the platform specific version to override the above
 * implementation
 *
 */
#include <kernel/arch/mp.h>

#define ON_CONFIG_MDOMAINS(x) do { x; } while(0)
#define ON_CONFIG_MUNITS(x) do { x; } while(0)

#else

/* Handle the single processor case */
inline cpu_context_t get_current_context(){
        return 0;
}

class mp_t : public generic_mp_t {};

#define ON_CONFIG_MDOMAINS(x)
#define ON_CONFIG_MUNITS(x)

#endif

inline mp_t* get_mp()
{
    extern mp_t mp;
    return &mp;
}

#endif /* !__GENERIC__MP_H__ */
