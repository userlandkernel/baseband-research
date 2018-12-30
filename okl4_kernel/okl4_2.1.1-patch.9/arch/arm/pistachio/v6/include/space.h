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
 * Description: ARMv6 specific space implementation.
 */

#ifndef __GLUE__V4_ARMV6__SPACE_H
#define __GLUE__V4_ARMV6__SPACE_H

#ifdef CONFIG_ENABLE_FASS
#error ARMv6 does not implement FASS
#endif

#include <kernel/asid.h>

typedef word_t arm_pid_t;

class space_t : public generic_space_t
{
public:
    enum rwx_e {
        read_only       = 0x1,
        read_write      = 0x3,
        read_execute    = 0x5,
        read_write_ex   = 0x7,
    };

    word_t space_control (word_t ctrl);

    asid_t *get_asid(void);
    arm_pid_t get_pid(void);
    void set_pid(arm_pid_t pid);

    bool add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
            rwx_e rwx, bool kernel, memattrib_e attrib);
    inline bool add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
            rwx_e rwx, bool kernel)
    {
        return add_mapping(vaddr, paddr, size, rwx, kernel, l4default);
    }

public:
    /* public for asmsyms */
    union {
        struct {
            BITFIELD3 (u16_t,
                    __zero      : 2,
                    pid : 7,
                    __fill1     : 7
                    );
            hw_asid_t asid;
        };
        word_t      pid_asid_raw;
    };
};

/**
 * ARMv6 specific part of space_control()
 */
INLINE word_t space_t::space_control (word_t ctrl) {
        set_pid(ctrl & PID_MASK);
        return 0;
}

INLINE arm_pid_t space_t::get_pid(void)
{
    return this->pid;
}

INLINE void space_t::set_pid(arm_pid_t new_pid)
{
    //printf("INSTALL PID = %d\n", new_pid);
    this->pid = new_pid;
}

INLINE asid_t* space_t::get_asid(void)
{
    return (asid_t*)&this->asid;
}

#endif /* __GLUE__V4_ARMV6__SPACE_H */
