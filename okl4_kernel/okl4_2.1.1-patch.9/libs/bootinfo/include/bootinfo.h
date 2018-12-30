/*
 * Copyright (c) 2006, National ICT Australia
 */
/*
 * Copyright (c) 2007, 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   The OKL4 common bootinfo interface
 */
#ifndef __BOOTINFO__BOOTINFO_H__
#define __BOOTINFO__BOOTINFO_H__

#include <stdint.h>
#include <stddef.h>
#include <l4/types.h>

#define BI_PAGER (~0UL)

/* Cap rights.  Sent as a bitmask. */
typedef enum 
{
    BI_CAP_EXECUTE = (1 << 0),
    BI_CAP_WRITE   = (1 << 1),
    BI_CAP_READ    = (1 << 2),
    BI_CAP_MASTER  = (BI_CAP_EXECUTE | BI_CAP_WRITE | BI_CAP_READ),
} bi_cap_rights_t;

/* Export types. */
typedef enum
{
    BI_EXPORT_CONST          = 1,
    BI_EXPORT_OBJECT         = 2,
    BI_EXPORT_BASE           = 3,
    BI_EXPORT_THREAD_ID      = 4,
    BI_EXPORT_MEMSECTION_CAP = 5,
    BI_EXPORT_THREAD_CAP     = 6,
    BI_EXPORT_VIRTPOOL_CAP   = 7,
    BI_EXPORT_PHYSPOOL_CAP   = 8,
    BI_EXPORT_CLIST_CAP      = 9,
    BI_EXPORT_ELF_SEGMENT    = 10,
    BI_EXPORT_ELF_FILE       = 11,
    BI_EXPORT_PD_CAP         = 12,
    BI_EXPORT_PHYSDEV_CAP    = 13,
    BI_EXPORT_VIRTDEV_CAP    = 14,
    BI_EXPORT_ZONE_CAP       = 15,
} bi_export_type_t;

/* Security Control types. */

typedef enum
{
    BI_SECURITY_PLATFORM  = 1, /* Platform Control */
    BI_SECURITY_SPACE     = 2, /* Space Switch */
    BI_SECURITY_INTERRUPT = 3, /* Interrupt Control */
    BI_SECURITY_MAPPING   = 4, /* User Mapping */
} bi_security_control_type_t;

/* Name of an object. */
typedef int bi_name_t;

#define BI_NAME_INVALID (-1)

typedef struct
{
    word_t total_pds;
    word_t total_mss;
    word_t total_threads;
    word_t total_caps;
    word_t total_pools;
    word_t total_zones;

    int rec_num; /* Number of the record being processed. */

    void * user_data;
} bi_user_data_t;

typedef struct
{
    int (* init)(const bi_user_data_t * data);
    int (* cleanup)(const bi_user_data_t * data);

    /**
     * @param virt_base Virtual base of largest block of memory
     * @param virt_end  Virtual end of largest block of memory
     * @param phys_base Physical base of largest block of memory
     * @param phys_end  Physical end of largest block of memory
     */
    int (* init_mem)(uintptr_t virt_base, uintptr_t virt_end,
            uintptr_t phys_base, uintptr_t phys_end,
            const bi_user_data_t * data);
    int (* init_mem2)(uintptr_t virt_base, uintptr_t virt_end,
            uintptr_t phys_base, uintptr_t phys_end,
            const bi_user_data_t * data);

    bi_name_t (* new_pd)(bi_name_t owner, const bi_user_data_t * data);
    bi_name_t (* new_ms)(bi_name_t owner, uintptr_t base, uintptr_t size,
            uintptr_t flags, uintptr_t attr, bi_name_t physpool,
            bi_name_t virtpool, bi_name_t zone, const bi_user_data_t * data);
    int (* add_virt_mem)(bi_name_t pool, uintptr_t base, uintptr_t end,
            const bi_user_data_t * data);
    int (* add_phys_mem)(bi_name_t pool, uintptr_t base, uintptr_t end,
            const bi_user_data_t * data);
    bi_name_t (* new_thread)(bi_name_t owner, uintptr_t ip, uintptr_t user_main,
            int priority, char * name, size_t name_len,
            const bi_user_data_t * data);
    int (* run_thread)(bi_name_t thread, const bi_user_data_t * data);
    int (* map)(uintptr_t vaddr, uintptr_t size, uintptr_t paddr,
            int scrub, unsigned mode, const bi_user_data_t * data);
    int (* attach)(bi_name_t pd, bi_name_t ms, int rights,
            const bi_user_data_t * data);
    int (* grant)(bi_name_t pd, bi_name_t obj, int rights,
            const bi_user_data_t * data);
    int (* argv)(bi_name_t thread, char * arg, size_t arg_len,
            const bi_user_data_t * data);
    int (* register_server)(bi_name_t thread, bi_name_t ms,
            const bi_user_data_t * data);
    int (* register_callback)(bi_name_t pd, bi_name_t ms,
            const bi_user_data_t * data);
    int (* register_stack)(bi_name_t thread, bi_name_t ms,
            const bi_user_data_t * data);
    bi_name_t (* new_cap)(bi_name_t obj, bi_cap_rights_t rights,
            const bi_user_data_t * data);
    int (* grant_cap)(bi_name_t pd, bi_name_t cap, const bi_user_data_t* data);
    int (* export_object)(bi_name_t pd, bi_name_t obj,
            bi_export_type_t export_type, char * key, size_t key_len,
            const bi_user_data_t * data);
    int (* export_struct)(bi_name_t pd, bi_export_type_t export_type,
            char * key, size_t key_len, uintptr_t first, intptr_t second,
            uintptr_t third, uintptr_t fourth, uintptr_t fifth, uintptr_t sixth,
            const bi_user_data_t * data);
    int (* register_env)(bi_name_t pd, bi_name_t ms,
            const bi_user_data_t * data);
    bi_name_t (*new_pool)(int is_virtual, const bi_user_data_t * data);
    int (* grant_interrupt)(bi_name_t thread, word_t interrupt,
            const bi_user_data_t * data);
    int (* security_control)(bi_name_t pd, bi_name_t obj,
            bi_security_control_type_t security_type,
            const bi_user_data_t * data);
    bi_name_t (* new_zone)(bi_name_t pool, const bi_user_data_t * data);
    int (* add_zone_window)(bi_name_t pool, size_t base,
            const bi_user_data_t * data);
    int (* kernel_info)(int max_spaces, int max_mutexes, int max_root_caps,
            const bi_user_data_t * data);
} bi_callbacks_t;

/** Initialiser for bi_callbacks_t. */
#define OKL4_BOOTINFO_CALLBACK_INIT {       \
    NULL, /* init */                        \
    NULL, /* cleanup */                     \
    NULL, /* init_mem */                    \
    NULL, /* init_mem2 */                   \
    NULL, /* new_pd */                      \
    NULL, /* new_ms */                      \
    NULL, /* add_virt_mem */                \
    NULL, /* add_phys_mem */                \
    NULL, /* new_thread */                  \
    NULL, /* run_thread */                  \
    NULL, /* map */                         \
    NULL, /* attach */                      \
    NULL, /* grant */                       \
    NULL, /* argv */                        \
    NULL, /* register_server */             \
    NULL, /* register_callback */           \
    NULL, /* register_stack */              \
    NULL, /* new_cap */                     \
    NULL, /* grant_cap */                   \
    NULL, /* export_object */               \
    NULL, /* export_struct */               \
    NULL, /* register_env */                \
    NULL, /* new_pool */                    \
    NULL, /* grant_interrupt */             \
    NULL, /* security_control */            \
    NULL, /* new_zone */                    \
    NULL, /* add_zone_window */             \
    NULL /* kernel_info */                  \
}


int bootinfo_parse(void * buffer, const bi_callbacks_t * callbacks,
        void * user_data);


#endif /* !__BOOTINFO__BOOTINFO_H__ */
