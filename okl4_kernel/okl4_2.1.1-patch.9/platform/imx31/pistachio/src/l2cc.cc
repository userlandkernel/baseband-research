/*
 * Copyright (c) 2006, National ICT Australia (NICTA)
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
 * Description:   imx31 L2 Cache Controlor implement
 */

#include <kernel/l4.h>
#include <kernel/arch/l2cache.h>

#ifdef CONFIG_HAS_L2_CACHE
#ifdef CONFIG_CP15_SUPPORT_L2CACHE
    #error "Unimplemented yet"
#else

void arm_l2_cache_t::cache_flush_by_pa(addr_t pa)
{
    L2CC_IVLD_CLN_BY_PA = (word_t)pa & ~((1UL << L2_LINE_SIZE_LOG2) - 1);
}

void arm_l2_cache_t::cache_flush_by_pa_range(addr_t pa, word_t sizelog2)
{
    if (sizelog2 <= L2_LINE_SIZE_LOG2)
        cache_flush_by_pa(pa);
    else if (sizelog2 >= (L2_LINE_SIZE_LOG2 + get_index_size_log2()))
        cache_flush();
    else
    {
        word_t index;
        for (index = 0; index < (1UL << (sizelog2 - L2_LINE_SIZE_LOG2)); index++)
            cache_flush_by_pa((addr_t)((word_t)pa + index * (1 << L2_LINE_SIZE_LOG2)));
    }
}

void arm_l2_cache_t::cache_flush_by_way(word_t way_bit)
{
    L2CC_IVLD_CLN_BY_WAY = way_bit;
    while (L2CC_IVLD_CLN_BY_WAY != 0);
}

void arm_l2_cache_t::cache_flush(void)
{
    word_t way_bit = (1UL << this->get_associativity()) - 1;
    cache_flush_by_way(way_bit);
}

void arm_l2_cache_t::cache_invalidate_by_pa(addr_t pa)
{
    L2CC_IVLD_BY_PA = (word_t)pa & ~((1UL << L2_LINE_SIZE_LOG2) - 1); 
}

void arm_l2_cache_t::cache_invalidate_by_way(word_t way_bit)
{
    L2CC_IVLD_BY_WAY = way_bit;
    while (L2CC_IVLD_BY_WAY != 0);
}

void arm_l2_cache_t::cache_invalidate(void)
{
    word_t way_bit = (1UL << this->get_associativity()) - 1;
    cache_invalidate_by_way(way_bit);
}

void arm_l2_cache_t::debug_read_line_tag(word_t way, word_t index, word_t *data, word_t *tag)
{
    word_t word_bit = 8 * L4_WORD_SIZE;
    word_t way_log2 = this->get_way_log2();
    word_t index_log2 = this->get_index_size_log2();
    word_t op_word = (way << (word_bit - way_log2)) | (index << (word_bit - index_log2) >> (word_bit - index_log2 - L2_LINE_SIZE_LOG2));
    word_t i;
    volatile word_t *line = L2CC_LINE_DATA_ARRAY;
    L2CC_DEBUG_CONTROL = 0x3; //Disable WB and linefile.
    L2CC_TEST_OP = op_word;
    /* Important!!!
     * Have to wait for a while until line register to be filled. */
    for (i = 0; i < 100; i++);
    *tag = L2CC_LINE_TAG;
    for (i = 0; i < ((1UL << L2_LINE_SIZE_LOG2)/L4_WORD_SIZE); i++)
        data[i] = line[i];
    L2CC_DEBUG_CONTROL = 0x0; //Enable WB and linefile.
}
void arm_l2_cache_t::cache_lockway_i(word_t way_bit)
{
    L2CC_LOCK_BY_WAY_INST = way_bit;
}
void arm_l2_cache_t::cache_lockway_d(word_t way_bit)
{
    L2CC_LOCK_BY_WAY_DATA = way_bit;
}
#endif

#ifdef CONFIG_HAS_L2_EVTMON
void arm_l2_event_monitor_t::init(void)
{
    counter_number = L2EVTMON_REGISTER_NUMBER;
    /*Use the last counter to detect L2 Cache Write Buffer abort */
    L2EVTMON_EMCC(counter_number - 1) = (L2EVTMON_EVT_BWABT << 2) | L2EVTMON_TRIGGER_METHOD_INCREAMENT | L2EVTMON_IRQ_ENABLED;
    L2EVTMON_EMMC = L2EVTMON_EMMC_INIT_VALUE;
}

void arm_l2_event_monitor_t::enable(void)
{
    volatile word_t old_value = L2EVTMON_EMMC;
    L2EVTMON_EMMC = old_value | L2EVTMON_EMMC_ENABLE;
}

void arm_l2_event_monitor_t::disable(void)
{
    volatile word_t old_value = L2EVTMON_EMMC;
    L2EVTMON_EMMC = old_value & ~L2EVTMON_EMMC_ENABLE;
}

bool arm_l2_event_monitor_t::is_EMC_event_occured(word_t number)
{
    if (number < counter_number)
        return (L2EVTMON_EMCS & (1UL << number));
    else
        return false;
}

void arm_l2_event_monitor_t::clear_EMC_event(word_t number)
{
    if (number < counter_number)
    {
        volatile word_t old_value = L2EVTMON_EMCS;
        L2EVTMON_EMCS = old_value | (1UL << number);
    }
}

bool arm_l2_event_monitor_t::read_EMC(word_t number, word_t * pvalue)
{
    if (number < counter_number)
    {
        *pvalue = (word_t)L2EVTMON_EMC(number);
        return true;
    }
    else
        return false;
}

word_t arm_l2_event_monitor_t::read_EMCC(word_t number)
{
    return L2EVTMON_EMCC(number);
}
void arm_l2_event_monitor_t::set_EMCC(word_t number, word_t event, word_t trigger_method, word_t enable_irq)
{
    /**
     * !!!Important: the last counter is used to detect
     * L2 Buffer Write abort, and can only set in the
     * init function, not allowed to set here.
     */
    if (number < (counter_number - 1))
    {
        L2EVTMON_EMCC(number) = event << 2 | trigger_method | enable_irq;
    }
}

arm_l2_event_monitor_t l2evtmon;
#endif

arm_l2_cache_t l2cc;
#endif

