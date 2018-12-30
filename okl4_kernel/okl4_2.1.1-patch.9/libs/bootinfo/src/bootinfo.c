/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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

#include <bootinfo/bootinfo.h>
#include "bootinfo_priv.h"

/** @todo FIXME: doxygen headers, split prototypes out - awiggins. */

static inline int
name_is_valid(bi_name_t name)
{
    return name != BI_NAME_INVALID;
}

static size_t
true_str_size(char * s, size_t n)
{
    char * end = s + n - 1;

    while (end >= s && *end == '\0') {
        end--;
    }

    return end < s ? 0 : end - s + 1;
}

/** @todo FIXME: Break this function down into smaller functions - awiggins. */
int
bootinfo_parse(void * buffer, const bi_callbacks_t * callbacks,
        void * user_data)
{
    char * this_rec = (char *)buffer;
    bi_user_data_t data;
    int ret = 1; /* Assume that we'll return an error. */

    data.total_pools   = 3;
    data.total_pds     = 0;
    data.total_mss     = 0;
    data.total_threads = 0;
    data.total_caps    = 0;
    data.total_zones   = 0;
    data.rec_num       = 0;
    data.user_data     = user_data;

    /* 1) Check for the header record. */
    if (((bi_hdr_t *)this_rec)->op != BI_OP_HEADER) {
        ERROR_PRINT("Boot Info header not found (found 0x%lx @ %p)\n",
                    (long) ((bi_hdr_t *)this_rec)->op, this_rec);
        return 1;
    }
    /*------------------------------------------------------------------------*/
    /*
     * First Pass: Run through the operations and calculate the
     * total number of objects that will be created.  This
     * information will be used to determine the size of the
     * object buffers.
     */

    BOOTINFO_PRINT("First pass: Calculating sizes and initialising memory.\n");

    while (1) {
        bi_hdr_t * current = (bi_hdr_t *)this_rec;

        switch (current->op) {
        case BI_OP_HEADER: {
            bi_header_t * rec = (bi_header_t *)current;

            BOOTINFO_PRINT("HEADER: stack_base: 0x%lx stack_end: 0x%lx, heap_base: 0x%lx, heap_end: 0x%lx\n",
                    (long)rec->stack_base, (long)rec->stack_end,
                    (long)rec->heap_base, (long)rec->heap_end);

            if (rec->magic != BI_MAGIC || rec->version != BI_VERSION) {
                ERROR_PRINT("Record %d: Bad bootinfo header found.\n",
                            data.rec_num);
                return 1;
            }
            break;
        }
        case BI_OP_END:
            goto end_first_pass;

        case BI_OP_INIT_MEM: {
            bi_init_mem_t * rec = (bi_init_mem_t *)current;

            BOOTINFO_PRINT("MEM_INIT (virt_base: 0x%lx, virt_end: 0x%lx, phys_base: 0x%lx, phys_end: 0x%lx)\n",
                           (long)rec->virt_base, (long)rec->virt_end,
                           (long)rec->phys_base, (long)rec->phys_end);
                
            if (rec->virt_base >= rec->virt_end ||
                    rec->phys_base >= rec->phys_end) {
                ERROR_PRINT("Record %d: Invalid address ranges: virt_base: 0x%lx, virt_end: 0x%lx, phys_base: 0x%lx, phys_end: 0x%lx\n",
                            data.rec_num, (long)rec->virt_base,
                            (long)rec->virt_end, (long)rec->phys_base,
                            (long)rec->phys_end);
                
                return 1;
            }
            
            /*
             * Initialise the memory subsystem.
             *
             * This is done in the first pass because the amount of
             * memory in the heap at startup is limited and there may
             * not be enough for all of the arrays we need to
             * malloc().  Initialising here will give more_core() the
             * ability to expand into new memsections.
             */
            if (callbacks->init_mem != NULL) {
                callbacks->init_mem((uintptr_t)rec->virt_base,
                        (uintptr_t)rec->virt_end, (uintptr_t)rec->phys_base,
                        (uintptr_t)rec->phys_end, &data);
            }

            break;
        }
        case BI_OP_NEW_PD:
            data.total_pds++;
            break;

        case BI_OP_NEW_MS:
            data.total_mss++;
            break;

        case BI_OP_NEW_THREAD:
            data.total_threads++;
            break;

        case BI_OP_NEW_CAP:
            data.total_caps++;
            break;

        case BI_OP_NEW_POOL:
            data.total_pools++;
            break;

        case BI_OP_NEW_ZONE:
            data.total_zones++;
            break;

            /* These ops don't allocate any bootinfo objects. */
        case BI_OP_ADD_VIRT_MEM:
        case BI_OP_ADD_PHYS_MEM:
        case BI_OP_REGISTER_STACK:
        case BI_OP_RUN_THREAD:
        case BI_OP_MAP:
        case BI_OP_ATTACH:
        case BI_OP_GRANT:
        case BI_OP_GRANT_CAP:
        case BI_OP_OBJECT_EXPORT:
        case BI_OP_STRUCT_EXPORT:
        case BI_OP_ARGV:
        case BI_OP_REGISTER_SERVER:
        case BI_OP_REGISTER_CALLBACK:
        case BI_OP_REGISTER_ENV:
        case BI_OP_GRANT_INTERRUPT:
        case BI_OP_SECURITY_CONTROL:
        case BI_OP_ADD_ZONE_WINDOW:
        case BI_OP_KERNEL_INFO:
            break;

        default:
            ERROR_PRINT("Record %d: Unknown opcode %ld\n",
                        data.rec_num, (long)current->op);
            return 1;
        }

        this_rec += current->size;
        data.rec_num++;
    }

end_first_pass:

    if (callbacks->init != NULL) {
        callbacks->init(&data);
    }
    
    BOOTINFO_PRINT("Second pass: Processing operation.\n");

    this_rec = (char *)buffer;
    data.rec_num = 0;

    while (1) {
        bi_hdr_t * current = (bi_hdr_t *)this_rec;

        BOOTINFO_PRINT("current is: %p\n", this_rec);

        switch (current->op) {
        case BI_OP_HEADER:
            break;

        case BI_OP_END:
            BOOTINFO_PRINT("END\n");
            goto success;

        case BI_OP_NEW_PD: {
            bi_new_pd_t * rec = (bi_new_pd_t *)current;

            BOOTINFO_PRINT("NEW PD (owner: %ld)\n", rec->owner);

            if (callbacks->new_pd != NULL) {
                bi_name_t name = callbacks->new_pd((bi_name_t)rec->owner,
                        &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created PD %d \n", name);
                }
                else {
                    goto quit;
                }
            }
            
            break;
        }
        case BI_OP_NEW_MS: {
            bi_new_ms_t * rec = (bi_new_ms_t *)current;

            BOOTINFO_PRINT
                ("NEW MS (owner: %ld, base: 0x%lx, size: 0x%lx, flags 0x%lx, attr: 0x%lx, physpool: 0x%lx, virtpool: 0x%lx, zone: %ld)\n",
                 (long)rec->owner, (long)rec->base, (long)rec->size,
                 (long)rec->flags, (long)rec->attr, (long)rec->physpool,
                 (long)rec->virtpool, (long)rec->zone);

            if (callbacks->new_ms != NULL) {
                bi_name_t name = callbacks->new_ms((bi_name_t)rec->owner,
                        (uintptr_t)rec->base, (uintptr_t)rec->size,
                        (uintptr_t)rec->flags, (uintptr_t)rec->attr,
                        (bi_name_t)rec->physpool, (bi_name_t)rec->virtpool,
                        (bi_name_t)rec->zone, &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created MS %d \n", name);
                }
                else {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_ADD_VIRT_MEM: {
            bi_add_virt_mem_t * rec = (bi_add_virt_mem_t *)current;

            BOOTINFO_PRINT("ADD VIRT MEM (pool: 0x%lx, base: 0x%lx, end: 0x%lx)\n",
                           (long)rec->pool, (long)rec->base, (long)rec->end);

            if (callbacks->add_virt_mem != NULL) {
                if (callbacks->add_virt_mem((bi_name_t)rec->pool,
                        (uintptr_t)rec->base, (uintptr_t)rec->end,
                        &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_ADD_PHYS_MEM: {
            bi_add_phys_mem_t * rec = (bi_add_phys_mem_t *)current;

            BOOTINFO_PRINT("ADD PHYS MEM (pool: 0x%lx, base: 0x%lx, end: 0x%lx)\n",
                           (long)rec->pool, (long)rec->base, (long)rec->end);

            if (callbacks->add_phys_mem != NULL) {
                if (callbacks->add_phys_mem((bi_name_t)rec->pool,
                                            (uintptr_t)rec->base,
                                            (uintptr_t)rec->end,
                                            &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_NEW_THREAD: {
            bi_new_thread_t * rec = (bi_new_thread_t *)current;
            size_t str_len = true_str_size((char *)rec->name,
                                           current->size - offsetof(bi_new_thread_t, name));

            BOOTINFO_PRINT("NEW THREAD (pd: %d, ip: 0x%lx, user_main: 0x%lx, pri: %d, name: \"%.*s\")\n",
                           (int)rec->owner, (long)rec->ip,
                           (long)rec->user_main, (int)rec->priority,
                           str_len, (char *) rec->name);

            if (callbacks->new_thread != NULL) {
                bi_name_t name = 
                    callbacks->new_thread((bi_name_t)rec->owner,
                                          (uintptr_t)rec->ip,
                                          (uintptr_t)rec->user_main,
                                          (int)rec->priority,
                                          (char *)rec->name, str_len,
                                          &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created Thread %d\n", name);
                }
                else {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_RUN_THREAD: {
            bi_run_thread_t * rec = (bi_run_thread_t *)current;

            BOOTINFO_PRINT("RUN THREAD (thread: %d)\n", (int)rec->name);

            if (callbacks->run_thread != NULL) {
                if (callbacks->run_thread((bi_name_t)rec->name,
                                          &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_MAP: {
            bi_map_t * rec = (bi_map_t *)current;

            BOOTINFO_PRINT("MAP (vaddr: 0x%lx, size %d, paddr: 0x%lx, scrub: %d, mode : 0x%lx)\n",
                           rec->vaddr, (int)rec->size, rec->paddr,
                           (int)rec->scrub, rec->mode);

            if (callbacks->map != NULL) {
                if (callbacks->map((uintptr_t)rec->vaddr, (uintptr_t)rec->size,
                        (uintptr_t)rec->paddr, (int)rec->scrub,
                        (unsigned)rec->mode, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_ATTACH: {
            bi_attach_t * rec = (bi_attach_t *)current;

            BOOTINFO_PRINT("ATTACH (pd: %d, ms: %d, rights: 0x%x)\n",
                           (int)rec->pd, (int)rec->ms, (int)rec->rights);

            if (callbacks->attach != NULL) {
                if (callbacks->attach((bi_name_t)rec->pd, (bi_name_t)rec->ms,
                        (int)rec->rights, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_GRANT: {
            bi_grant_t * rec = (bi_grant_t *)current;

            BOOTINFO_PRINT("GRANT (pd: %d, obj: %d, rights: 0x%x)\n",
                           (int)rec->pd, (int)rec->obj, (int)rec->rights);

            if (callbacks->grant != NULL) {
                if (callbacks->grant((bi_name_t)rec->pd, (bi_name_t)rec->obj,
                        (int)rec->rights, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_ARGV: {
            bi_argv_t * rec = (bi_argv_t *)current;
            size_t str_len = true_str_size((char *)rec->arg,
                                           current->size - offsetof(bi_argv_t, arg));

            BOOTINFO_PRINT("ARGV (thread: %d, arg: \"%.*s\"\n",
                           (int)rec->thread, str_len, (char *)rec->arg);

            if (callbacks->argv != NULL) {
                if (callbacks->argv((bi_name_t)rec->thread, (char *)rec->arg,
                        str_len, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_REGISTER_SERVER: {
            bi_register_server_t * rec = (bi_register_server_t *)current;

            BOOTINFO_PRINT("REGISTER SERVER (thread: %d, ms: %d)\n",
                           (int)rec->thread, (int)rec->ms);

            if (callbacks->register_server != NULL) {
                if (callbacks->register_server((bi_name_t)rec->thread,
                        (bi_name_t)rec->ms, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_REGISTER_CALLBACK: {
            bi_register_callback_t * rec = (bi_register_callback_t *)current;

            BOOTINFO_PRINT("REGISTER CALLBACK (pd: %d, ms: %d)\n",
                           (int)rec->pd, (int)rec->ms);

            if (callbacks->register_callback != NULL) {
                if (callbacks->register_callback((bi_name_t)rec->pd,
                        (bi_name_t)rec->ms, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_REGISTER_STACK: {
            bi_register_stack_t * rec = (bi_register_stack_t *)current;

            BOOTINFO_PRINT("REGISTER STACK (thead: %d, ms: %d)\n",
                           (int)rec->thread, (int)rec->ms);

            if (callbacks->register_stack != NULL) {
                if (callbacks->register_stack((bi_name_t)rec->thread,
                        (bi_name_t)rec->ms, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_INIT_MEM: {
            bi_init_mem_t * rec = (bi_init_mem_t *)current;

            if (callbacks->init_mem2 != NULL) {
                if (callbacks->init_mem2((uintptr_t)rec->virt_base,
                        (uintptr_t)rec->virt_end, (uintptr_t)rec->phys_base,
                        (uintptr_t)rec->phys_end, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_NEW_CAP: {
            bi_new_cap_t * rec = (bi_new_cap_t *)current;

            BOOTINFO_PRINT("NEW_CAP (obj: %d, rights: 0x%lx)\n",
                           (int)rec->obj, (long)rec->rights);

            if (callbacks->new_cap != NULL) {
                bi_name_t name = callbacks->new_cap((bi_name_t)rec->obj,
                        (bi_cap_rights_t)rec->rights, &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created CAP %d\n", name);
                }
                else {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_GRANT_CAP: {
            bi_grant_cap_t * rec = (bi_grant_cap_t *)current;

            BOOTINFO_PRINT("GRANT_CAP (pd: %d, cap: %d)\n",
                           (int)rec->pd, (int)rec->cap);

            if (callbacks->grant_cap != NULL) {
                if (callbacks->grant_cap((bi_name_t)rec->pd,
                        (bi_name_t)rec->cap, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_OBJECT_EXPORT: {
            bi_object_export_t * rec = (bi_object_export_t *)current;
            size_t str_len = true_str_size((char*)rec->key,
                                           current->size - offsetof(bi_object_export_t, key));

            BOOTINFO_PRINT
                ("OBJECT_EXPORT (pd: %d, key: \"%.*s\", obj: %d, type: %d)\n",
                 (int)rec->pd, (int) str_len, (char*) rec->key, (int)rec->obj,
                 (int)rec->type);

            if (callbacks->export_object != NULL) {
                if (callbacks->export_object((bi_name_t)rec->pd,
                        (bi_name_t)rec->obj, (bi_export_type_t)rec->type,
                        (char *)rec->key, str_len, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_STRUCT_EXPORT: {
            bi_struct_export_t * rec = (bi_struct_export_t *)current;
            size_t str_len = true_str_size((char *)rec->key,
                                           current->size - offsetof(bi_struct_export_t, key));

            BOOTINFO_PRINT
                ("STRUCT_EXPORT (pd: %d, key: \"%.*s\", first: 0x%lx, second: 0x%lx, third: 0x%lx, fourth: 0x%lx, fifth: 0x%lx, sixth: 0x%lx, type: %d)\n",
                 (int)rec->pd, (int) str_len, (char*)rec->key,
                 (long)rec->first, (long)rec->second, (long)rec->third,
                 (long)rec->fourth, (long)rec->fifth, (long)rec->sixth,
                 (int)rec->type);

            if (callbacks->export_struct != NULL) {
                if (callbacks->export_struct((bi_name_t)rec->pd,
                        (bi_export_type_t)rec->type, (char *)rec->key,
                        str_len, (uintptr_t)rec->first, (uintptr_t)rec->second,
                        (uintptr_t)rec->third, (uintptr_t)rec->fourth,
                        (uintptr_t)rec->fifth, (uintptr_t)rec->sixth,
                        &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_REGISTER_ENV: {
            bi_register_env_t * rec = (bi_register_env_t *)current;

            BOOTINFO_PRINT("REGISTER ENVIRONMENT (pd: %d, ms: %d)\n",
                           (int)rec->pd, (int)rec->ms);

            if (callbacks->register_env != NULL) {
                if (callbacks->register_env((bi_name_t)rec->pd,
                        (bi_name_t)rec->ms, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_NEW_POOL: {
            bi_new_pool_t * rec = (bi_new_pool_t *)current;

            BOOTINFO_PRINT("NEW POOL (is_virtual: %d)\n", (int)rec->is_virtual);

            if (callbacks->new_pool != NULL) {
                bi_name_t name = callbacks->new_pool((int) rec->is_virtual,
                        &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created %s Pool %d\n",
                                   rec->is_virtual ? "Virtual" : "Physical",
                                   name);
                }
                else {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_GRANT_INTERRUPT: {
            bi_grant_interrupt_t * rec = (bi_grant_interrupt_t *)current;

            BOOTINFO_PRINT("GRANT INTERRUPT (tid: %d, irq: %d)\n",
                           (int)rec->thread, (int)rec->irq);

            if (callbacks->grant_interrupt != NULL) {
                if (callbacks->grant_interrupt((bi_name_t)rec->thread,
                        (int)rec->irq, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_SECURITY_CONTROL: {
            bi_security_control_t * rec = (bi_security_control_t *)current;

            BOOTINFO_PRINT("SECURITY CONTROL (pd: %d, obj: %d, type: %d)\n",
                           (int)rec->pd, (int)rec->obj, (int)rec->type);

            if (callbacks->security_control != NULL) {
                if (callbacks->security_control((bi_name_t)rec->pd,
                        (bi_name_t)rec->obj,
                        (bi_security_control_type_t)rec->type, &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_NEW_ZONE: {
            bi_new_zone_t * rec = (bi_new_zone_t *)current;

            BOOTINFO_PRINT("NEW ZONE (pool: %d)\n", (int)rec->pool);

            if (callbacks->new_zone != NULL) {
                bi_name_t name = callbacks->new_zone((bi_name_t)rec->pool,
                        &data);

                if (name_is_valid(name)) {
                    BOOTINFO_PRINT("<--- Created Zone %d\n", name);
                }
                else {
                    goto quit;
                }
            }

            break;
        }
        case BI_OP_ADD_ZONE_WINDOW: {
            bi_add_zone_window_t * rec = (bi_add_zone_window_t *)current;

            BOOTINFO_PRINT("ADD ZONE WINDOW (zone: %d base: 0x%lx)\n",
                           (int) rec->zone, (long)rec->base);

            if (callbacks->add_zone_window != NULL) {
                if (callbacks->add_zone_window((bi_name_t) rec->zone,
                        (size_t)rec->base, &data) != 0) {
                    goto quit;
                }
            }
            break;
        }
        case BI_OP_KERNEL_INFO: {
            bi_kernel_info_t * rec = (bi_kernel_info_t *)current;
            
            BOOTINFO_PRINT("KERNEL INFO (spaces: %d, mutexes: %d, caps: %d)\n",
                           (int)rec->max_spaces, (int)rec->max_mutexes,
                           (int)rec->max_root_caps);

            if (callbacks->kernel_info != NULL) {
                if (callbacks->kernel_info((int)rec->max_spaces,
                        (int)rec->max_mutexes, (int)rec->max_root_caps,
                        &data) != 0) {
                    goto quit;
                }
            }

            break;
        }
        default:
            ERROR_PRINT("Record %d: Unknown opcode %ld\n",
                        data.rec_num, (long)current->op);

            goto quit;
        }
        
        this_rec += current->size;
        data.rec_num++;
    }

success:
    /* Success is at hand! */
    ret = 0;

quit:
    if (callbacks->cleanup != NULL) {
        if (callbacks->cleanup(&data) != 0) {
            ret = 1;
        }
    }

    return ret;
}
