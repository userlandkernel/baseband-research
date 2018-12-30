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
 * Description: L2 Cache control API class definitions
 */
#ifndef __GLUE__V4_ARM__L2CACHE_H__
#define __GLUE__V4_ARM__L2CACHE_H__

#include <kernel/debug.h>

#ifdef CONFIG_HAS_L2_CACHE
#include <kernel/plat/l2cc.h>

class arm_l2_cache_t
{
private:
    word_t  enabled;
    l210_aux_ctrl_t aux;
public:
    void cache_flush(void);
    void cache_flush_by_pa(addr_t);
    void cache_flush_by_pa_range(addr_t, word_t);
    void cache_flush_by_way(word_t);
    void cache_invalidate(void);
    void cache_invalidate_by_pa(addr_t);
    void cache_invalidate_by_way(word_t);
    void debug_read_line_tag(word_t, word_t, word_t *, word_t *);
    void cache_lockway_i(word_t);
    void cache_lockway_d(word_t);
#ifdef CONFIG_ARM_L2_INSTRUCTIONS
    #error "Unimplemented"
#else
    inline l210_aux_ctrl_t *read_aux(void)
    {
        this->aux.read();
        return &(this->aux);
    }

    inline word_t is_enabled(void) { return enabled; }

    inline void enable(void)
    {
        enabled = 0x1;
        L2CC_CONTROL = enabled;
    }

    inline void disable(void)
    {
        enabled = 0x0;
        L2CC_CONTROL = enabled;
    }

    inline void write_aux(word_t value)
    {
        /*
         * L2CC aux control register should only be written when L2CC is
         * disabled, otherwise could cause unexpected cache behaviour.
         */
        ASSERT(DEBUG, !enabled);
        if (!enabled) {
            this->aux.write(value);
        }
    }

    inline void init(void)
    {
        /* Aux control reg should only be changed when l2cc is disabled. */
        this->disable();
        this->aux.init();
        L2CC_AUX_CONTROL = aux.raw;
        this->cache_invalidate();
    }
#endif /* CONFIG_ARM_L2_INSTRUCTIONS */

    inline word_t get_associativity(void) { return (word_t)aux.reg.associativity; }
    inline word_t get_index_size_log2(void) { return (word_t)aux.reg.way_size + 8; }
    inline word_t get_way_log2(void) { return (word_t)aux.get_way_log2(); }
};

inline arm_l2_cache_t*
get_arm_l2cc(void)
{
    extern arm_l2_cache_t l2cc;
    return &l2cc;
}

#ifdef CONFIG_HAS_L2_EVTMON
class arm_l2_event_monitor_t
{
public:
    word_t counter_number;
public:
    void init(void);
    bool is_EMC_event_occured(word_t number);
    void clear_EMC_event(word_t number);
    bool read_EMC(word_t number, word_t * pvalue);
    word_t read_EMCC(word_t number);
    void set_EMCC(word_t number, word_t event, word_t trigger_method, word_t enable_irq);
    void enable(void);
    void disable(void);
};

inline arm_l2_event_monitor_t*
get_arm_l2evtmon(void)
{
    extern arm_l2_event_monitor_t l2evtmon;
    return &l2evtmon;
}
#endif

#endif // CONFIG_HAS_L2_CACHE

#endif /* __GLUE__V4_ARM__L2CC_H__ */
