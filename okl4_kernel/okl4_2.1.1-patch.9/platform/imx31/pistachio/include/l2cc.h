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
 * Description:   i.MX31 Platform L2 Cache Controler
 */
#ifndef __PLATFORM__IMX31__L2CC_H_
#define __PLATFORM__IMX31__L2CC_H_

#include <kernel/l4.h>

#ifdef CONFIG_HAS_L2_CACHE
#include <kernel/plat/io.h>

#ifdef CONFIG_ARM_L2_INSTRUCTIONS
    #error "Unimplemented"
#else

#define L2CC_PHYS_BASE              IO_AREA3_PADDR
#define L2CC_VIRT_BASE              IO_AREA3_VADDR

#define L2EVTMON_PHYS_BASE          IO_AREA4_PADDR
#define L2EVTMON_VIRT_BASE          IO_AREA4_VADDR

#define L2CC_CACHE_ID               (*(volatile word_t *)(L2CC_VIRT_BASE + 0x000))
#define L2CC_CACHE_TYPE             (*(volatile word_t *)(L2CC_VIRT_BASE + 0x004))
#define L2CC_CONTROL                (*(volatile word_t *)(L2CC_VIRT_BASE + 0x100))
#define L2CC_AUX_CONTROL            (*(volatile word_t *)(L2CC_VIRT_BASE + 0x104))
#define L2CC_CACHE_SYNC             (*(volatile word_t *)(L2CC_VIRT_BASE + 0x730))
#define L2CC_IVLD_BY_WAY            (*(volatile word_t *)(L2CC_VIRT_BASE + 0x77C))
#define L2CC_IVLD_BY_PA             (*(volatile word_t *)(L2CC_VIRT_BASE + 0x770))
#define L2CC_CLN_BY_IDX_WAY         (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7B8))
#define L2CC_CLN_BY_WAY             (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7BC))
#define L2CC_CLN_BY_PA              (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7B0))
#define L2CC_IVLD_CLN_BY_IDX_WAY    (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7F8))
#define L2CC_IVLD_CLN_BY_WAY        (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7FC))
#define L2CC_IVLD_CLN_BY_PA         (*(volatile word_t *)(L2CC_VIRT_BASE + 0x7F0))
#define L2CC_LOCK_BY_WAY_DATA       (*(volatile word_t *)(L2CC_VIRT_BASE + 0x900))
#define L2CC_LOCK_BY_WAY_INST       (*(volatile word_t *)(L2CC_VIRT_BASE + 0x904))
#define L2CC_TEST_OP                (*(volatile word_t *)(L2CC_VIRT_BASE + 0xF00))
#define L2CC_LINE_DATA_ARRAY          (volatile word_t *)(L2CC_VIRT_BASE + 0xF10)
#define L2CC_LINE_TAG               (*(volatile word_t *)(L2CC_VIRT_BASE + 0xF30))
#define L2CC_DEBUG_CONTROL          (*(volatile word_t *)(L2CC_VIRT_BASE + 0xF40))

#define L2EVTMON_EMMC               (*(volatile word_t *)(L2EVTMON_VIRT_BASE + 0x00))
#define L2EVTMON_EMCS               (*(volatile word_t *)(L2EVTMON_VIRT_BASE + 0x04))
#define L2EVTMON_EMCC_BASE          (L2EVTMON_VIRT_BASE + 0x8)
#define L2EVTMON_EMCC(x)            (*(volatile word_t *)(L2EVTMON_EMCC_BASE + (x) * sizeof(word_t)))
#define L2EVTMON_EMC_BASE           (L2EVTMON_VIRT_BASE + 0x18)
#define L2EVTMON_EMC(x)             (*(volatile word_t *)(L2EVTMON_EMC_BASE + (x) * sizeof(word_t)))


/* i.MX31 has non-configurable l2 cache size of 128k, 8 ways */
//#define L2_WAY_LOG2                 3
//#define L2_INDEX_LOG2               9
//#define L2_TAG_LOG2                 18
#define RESET_VALUE                 0xE4020FFF
#ifdef CONFIG_HAS_L2_EVTMON
#define INIT_VALUE                  0x0013001B
#else
#define INIT_VALUE                  0x0003001B
#endif
#define L2_LINE_SIZE_LOG2           5
#define TAG_REG_VALID_SHIFT         13
#define TAG_REG_DIRTY_H_SHIFT       12
#define TAG_REG_DIRTY_L_SHIFT       11

#define L2EVTMON_TRIGGER_METHOD_OVERFLOW    0
#define L2EVTMON_TRIGGER_METHOD_INCREAMENT  2
#define L2EVTMON_IRQ_DISABLED               0
#define L2EVTMON_IRQ_ENABLED                1
#define L2EVTMON_REGISTER_NUMBER            4
#define L2EVTMON_EMMC_INIT_VALUE            0x00000F01
#define L2EVTMON_EMMC_COUNTER_RESET         0x00000F00
#define L2EVTMON_EMMC_ENABLE                0x00000001

#define L2EVTMON_EVT_BWABT                  0x01UL
#define L2EVTMON_EVT_CO                     0x02UL
#define L2EVTMON_EVT_DRHIT                  0x03UL
#define L2EVTMON_EVT_DRREQ                  0x04UL
#define L2EVTMON_EVT_DWHIT                  0x05UL
#define L2EVTMON_EVT_DWREQ                  0x06UL
#define L2EVTMON_EVT_DWTREQ                 0x07UL
#define L2EVTMON_EVT_IRHIT                  0x08UL
#define L2EVTMON_EVT_IRREQ                  0x09UL
#define L2EVTMON_EVT_WA                     0x0AUL
#define L2EVTMON_EVT_EMC3OFL                0x0BUL
#define L2EVTMON_EVT_EMC2OFL                0x0CUL
#define L2EVTMON_EVT_EMC1OFL                0x0DUL
#define L2EVTMON_EVT_EMC0OFL                0x0EUL
#define L2EVTMON_EVT_CLK                    0x0FUL

#define L2EVTMON_IRQ                        23
#endif /* !CONFIG_ARM_L2_INSTRUCTIONS */

class l210_aux_ctrl_t
{
public:
    union {
        word_t raw;
        struct {
            BITFIELD12 (word_t,
                    lat_data_read       : 3,
                    lat_data_write      : 3,
                    lat_tag             : 3,
                    lat_dirty           : 3,
                    wrap_disable        : 1,
                    associativity       : 4,
                    way_size            : 3,
                    evt_bus_enable      : 1,
                    parity_enable       : 1,
                    wa_override         : 1,
                    ex_abt_disable      : 1,
                    reserved            : 7
                    );
        } reg;
    };
    inline void reset(void) { raw = RESET_VALUE; }
    inline void init(void) { raw = INIT_VALUE; }
    inline word_t read(void) { raw = L2CC_AUX_CONTROL;  return raw; }
    /** 
     * i.MX31 has non-configurable l2 cache, can not change associativity
     * and way_size.
     */
    inline void write(word_t value)
    {
        value &= ~0xFE000UL;
        value |= 0x30000UL;
        raw = value;
        L2CC_AUX_CONTROL = value;
    }
    inline word_t get_way_log2(void) { return 3; }
};
#endif

#endif /* __PLATFORM__IMX31__CACHE_H_ */
