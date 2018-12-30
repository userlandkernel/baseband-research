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

#ifndef __BOOTINFO__PRIV_H__
#define __BOOTINFO__PRIV_H__

#include <compat/c.h>
#include <stdio.h>

/* See SConstruct for iguana - enabled by DEBUG_TRACE option */

#ifdef  IG_DEBUG_PRINT
#define ERROR_PRINT (void) printf
#define DEBUG_PRINT (void) printf
#else
#define ERROR_PRINT(...)
#define DEBUG_PRINT(...)
#endif

#ifdef IG_VERBOSE
#define BOOTINFO_PRINT (void) printf
#else
#define BOOTINFO_PRINT(...)
#endif

#define BI_PAGER                (~0UL)

/* Operation Types. */
#define BI_OP_HEADER            1
#define BI_OP_END               2
#define BI_OP_NEW_PD            3
#define BI_OP_NEW_MS            4
#define BI_OP_ADD_VIRT_MEM      5
#define BI_OP_ADD_PHYS_MEM      6
#define BI_OP_NEW_THREAD        7
#define BI_OP_RUN_THREAD        8
#define BI_OP_MAP               9
#define BI_OP_ATTACH            10
#define BI_OP_GRANT             11
/*#define BI_OP_EXPORT            12*/ /* Retired */
#define BI_OP_ARGV              13
#define BI_OP_REGISTER_SERVER   14
#define BI_OP_REGISTER_CALLBACK 15
#define BI_OP_REGISTER_STACK    16
#define BI_OP_INIT_MEM          17
/*#define BI_OP_NEW_VM_POOL       18*/ /* Retired */
/*#define BI_OP_NEW_PHYS_POOL     19*/ /* Retired */
#define BI_OP_NEW_CAP           20
#define BI_OP_GRANT_CAP         21
#define BI_OP_OBJECT_EXPORT     22
#define BI_OP_STRUCT_EXPORT     23
#define BI_OP_REGISTER_ENV      24
#define BI_OP_NEW_POOL          25
#define BI_OP_GRANT_INTERRUPT   26
#define BI_OP_SECURITY_CONTROL  27
#define BI_OP_NEW_ZONE          28
#define BI_OP_ADD_ZONE_WINDOW   29
#define BI_OP_KERNEL_INFO       30

/*
 * Description of the new ElfWeaver output file.
 *
 * The file is a series of records described below, each describing an
 * operation.  These operations are read sequentially and are executed
 * as soon as they are read.  The first record must be an OP_HEADER
 * record and the last must be an OP_END record.
 *
 * Each record is of variable size.  The size field contains the
 * offset in bytes from the start of one record to the next.
 *
 * Certain operations create objects and these objects are assigned to
 * the supplied names.  These names are simple integers that must be
 * unique.  ElfWeaver is responsible for the allocation of these
 * names.  Execution aborts if two objects are created with the same
 * name or if a record refers to a name that has not been assigned to
 * earlier or refers to an unexpected type.
 *
 * The operations are:
 *
 * 1) OP_HEADER
 *
 * Describes the file.  The fields are:
 *
 * a) magic
 *
 * Always 0x1960021d
 *
 * b) version
 *
 * Version of the file format.  Currently 5.
 *
 * c) debug
 *
 * For future use and must be 0.
 *
 * 2) OP_END
 *
 * Marks the end of the file.  No further records will be read.
 *
 * There are no fields defined for this record.
 *
 * 3) OP_NEW_PD
 *
 * Create a new Protection Domain.  The fields are:
 *
 * a) owner
 *
 * The name of the owning PD.  The value of 0 refers to the PD of the
 * iguana server.
 *
 * 4) OP_NEW_MS
 *
 * Create a new memsection.  The fields are:
 *
 * a) owner
 *
 * The name of the owning protection domain
 *
 * b) base
 *
 * The base address of the memsection
 *
 * c) size
 *
 * Size of the memsection in bytes.
 *
 * d) attr
 *
 * Attributes for the memsection.  See
 * iguana_memsection_set_attributes() for a list of values.
 *
 * e) flags
 *
 * f) physpool
 *
 * The name of the memory pool from which to allocate physical memory.
 * 
 * f) virtpool
 *
 * The name of the memory pool from which to allocate virtual memory.
 * If set to BI_IGNORE, the virtual address comes from a zone.
 * 
 * f) zone
 *
 * The name of the zone from which to allocate virtual memory.  If set
 * to BI_IGNORE, then the virtual address comes from the virtpool.
 * 
 * 5) OP_ADD_VIRT_MEM
 *
 * Add a range of memory to the given VM pool or zone
 *
 * The fields are:
 *
 * a) pool
 *
 * The name of the memory pool or zone
 * 
 * b) base
 *
 * The base address of the range in bytes.
 *
 * c) end
 *
 * The end address of the range in bytes
 *              
 * 6) OP_ADD_PHYS_MEM
 *
 * Add a range of memory to the given physical pool
 *
 * The fields are:
 *
 * a) pool
 *
 * The name of the memory pool
 * 
 * b) base
 *
 * The base address of the range in bytes
 *
 * c) end
 *
 * The end address of the range in bytes.
 *              
 * 7) OP_NEW_THREAD
 *
 * Create a new inactive thread.
 *
 * The fields are:
 *
 * a) owner
 *
 * The name of the owning protection domain.
 *
 * b) ip
 *
 * The address of the first instruction
 *
 * c) priority
 *
 * The thread's priority.
 *
 * d) name
 *
 * The short string that is passed to L4_KDB_SetThreadName() to give
 * the debugger a human-readable identifier for the thread.
 *
 * 8) OP_RUN_THREAD
 *
 * Start running an inactive thread.  If this operation is made on
 * a running thread execution aborts.
 *
 * The running thread will find the thread's command line arguments 
 * (see OP_ARGV below) pushed onto the stack as well as a pointer to
 * the PD's object environment
 *
 * The fields are:
 *
 * a) name
 *
 * The name of the thread to run.
 *
 * 9) OP_MAP
 *
 * Map a virtual address to a physical address.
 *
 * The fields are:
 *
 * a) vaddr
 *
 * Virtual address.  This is an address, not a name.
 *
 * b) size
 *
 * Size in bytes of the memory region.
 *
 * c) paddr
 *
 * Corresponding physical address.  This is an address, not a
 * name.
 *
 * d) scrub
 *
 * A boolean value that indicates whether or not Iguana Server should
 * fill the mapped region with zeros.  This should be set to false if
 * mapping in device registers.
 *
 * 10) OP_ATTACH
 *
 * Attach a memsection to a protection domain.  This allows
 * memsections to be shared amongst protection domains.
 *
 * Memsections that are part of zones should not be attached directly
 * to PDs.
 *
 * The fields are:
 *
 * a) pd
 *
 * The name of the protection domain
 *
 * b) ms
 *
 * The name of the memsection.
 *
 * c) rights
 *
 * The read/write/execute permissions that the PD has on the
 * memsection.
 *
 * 11) OP_GRANT
 *
 * Grant a PD operational access over a memsection.  See also
 * OP_GRANT_CAP for more general access rights.
 *
 * The fields are:
 *
 * a) pd
 *
 * The name of the protection domain.
 *
 * b) obj
 *
 * The name of the memsection
 *
 * c) rights
 *
 * The read/write/execute rights that the PD has on the memsection.
 *
 * 12) Not Used
 *
 * Operation number 12 is not longer used.
 *
 * 13) OP_ARGV
 *
 * Declare a command line argument for a thread.  The order in argv
 * corresponds to the order of these operations (ie, the first op will
 * be argv[0], etc).
 *   
 * The fields are:
 *
 * a) thread
 *
 * The name of the thread to which the argument will be exported.
 *
 * b) arg
 *
 * String used as an argument.  The length of the string is
 * assumed to be the size of the record minus twice the size of a
 * word_t.
 *
 * 14) OP_REGISTER_SERVER
 *
 * Register a server thread for a memsection.
 *
 * The fields are:
 * 
 * a) thread
 *
 * The name of the thread being registered.
 *
 * b) ms 
 *
 * The name of the memsection to register the server for.
 *
 * 15) OP_REGISTER_CALLBACK
 *
 * Setup an async communications channel for messages.
 *
 * The fields are:
 *
 * a) pd
 *
 * The name of the containing protection domain.
 *
 * b) ms
 *
 * The name of the protection domain callback buffer.
 *
 * 16) OP_REGISTER_STACK
 *
 * Assign a memsection as the stack for a thread.
 *
 * The fields are:
 *
 * a) thread
 *
 * The name of the thread.
 * 
 * b) ms
 *
 * The name of the stack memsection.
 *
 * 17) OP_INIT_MEM
 *
 * Initialise the Iguana memory subsystem.
 *
 * Iguana requires a range of virtual and physical memory to store it
 * initial memory tracking data structures.  These memory ranges must
 * be paged aligned and at least one page in size.  These memory
 * regions will be added to Iguana's default virtual and memory pools.
 *
 * This operation create three (3) objects.  The first is the default
 * virtual memory pool, the second is the default physical pool and
 * the third is the direct memory pool.
 *
 * The fields are:
 *
 * a) virt_base
 *
 * The base address of a valid virtual memory range.
 * 
 * b) virt_end
 *
 * The end address of a valid virtual memory range.
 * 
 * c) phys_base
 * 
 * The base address of a valid physical memory range.
 * 
 * d) phys_end
 *
 * The end address of a valid virtual memory range.
 *
 * 18) Not Used
 *
 * Operation number 18 is not longer used.
 *
 * 19) Not Used
 *
 * Operation number 19 is not longer used.
 *
 * 20) OP_NEW_CAP
 *
 * Create a cap for an object
 *
 * The fields are:
 *
 * a) obj
 *
 * The name of the object
 *
 * b) rights
 *
 * The access rights of the CAP.  The currently supported rights are
 * found in security.h.
 *
 * 21) OP_GRANT_CAP
 *
 * Grant a PD a cap over an iguana object.
 *
 * The fields are:
 *
 * a) pd
 *
 * The name of the protection domain
 *
 * b) cap
 *
 * The cap to grant to the PD.  The cap contains a reference to the
 * object.
 *
 * 22) OP_OBJECT_EXPORT
 *
 * Export an object or an integer to a pd's object environment.
 *
 * The fields are:
 *
 * a) pd
 *
 * Name of the PD 
 *
 * b) obj
 *
 * Name of the object to export.
 *
 * c) type
 *
 * The type of the object
 *
 *  1) EXPORT_CONST          - The object is an int constant, not a name,
 *  2) EXPORT_OBJECT         - The name of an object of unknown type.
 *  3) EXPORT_BASE           - The object is a base address, not a name.
 *  4) EXPORT_THREAD_ID      - The object is a thread id, not a name.
 *  5) EXPORT_MEMSECTION_CAP - The object is the name of a memsection cap.
 *  6) EXPORT_THREAD_CAP     - The object is the name of a thread cap.
 *  7) EXPORT_VIRTPOOL_CAP   - The object is the name of a virtpool cap.
 *  8) EXPORT_PHYSPOOL_CAP   - The object is the name of a physpool cap.
 *  9) EXPORT_CLIST_CAP      - The object is the name of a clist cap.
 *  12) EXPORT_PD_CAP        - The object is the name of a pd cap.
 *  13) EXPORT_ZONE_CAP      - The object is the name of a zone cap.
 *
 * d) key
 *
 * The string key of the object in the environment.
 *
 * 23) OP_STRUCT_EXPORT
 *
 * Export two field structured data to a PD's object environment.
 *
 * The fields are:
 *
 * a) pd
 *
 *
 * The name of the PD.
 *
 * b) first
 *
 * The data for the first field of the structure.
 *
 * c) second
 *
 * The data for the second field of the structure.
 *
 * d) type
 *
 * The type of structured data.  Valid values are:
 *
 *  10) EXPORT_ELF_SEGMENT - ELF segment data.
 *  11) EXPORT_ELF_FILE    - ELF file data.
 *
 * e) key
 *
 * The string key of the object in the environment.
 *
 * 24) OP_REGISTER_ENV
 *
 * Assign a memsection as the object environment for a thread.
 *
 * The fields are:
 *
 * a) pd
 *
 * The of the pd.
 *
 * b) ms
 *
 * The name of the object environment memsection.
 *
 * 25) OP_NEW_POOL
 *
 * Create a new virtual or physical pool.
 *
 * The fields are:
 *
 * a) is_virtual
 *
 * A boolean value that indicated whether or not a virtual pool should
 * be created.
 *
 * 26) OP_GRANT_INTERRUPT
 * 
 * Record for granting an interrupt.
 * 
 * The fields are:
 *
 * a) handler
 *
 * The interrupt handler
 *
 * b) irq
 *
 * The interrupt number to be granted.
 *
 * 27) OP_SECURITY_CONTROL
 *
 * 28) NEW_ZONE
 *
 * Create a new Zone.
 *
 * The fields are:
 *
 * a) pool
 *
 * The name of the virtual pool from which the zone can obtain
 * additional memory.
 *
 * 29) OP_ADD_ZONE_WINDOW
 *
 * Record an active window for a zone.
 * 
 * The fields are:
 *
 * a) zone
 *
 * The name of the zone.
 *
 * b) base
 *
 * The base address of the window.  The base address must be 1M
 * aligned.
 *
 */

/*
 * Include the defintions of BI_MAGIC, BI_VERSION, bi_hdr_t and
 * bi_header_t.
 */
#include <bootinfo/bootinfo_header.h>

typedef struct {
    bi_hdr_t header;
} bi_end_t;

typedef struct {
    bi_hdr_t header;
    word_t owner;
} bi_new_pd_t;

typedef struct {
    bi_hdr_t header;
    word_t owner;
    word_t base;
    word_t size;
    word_t flags;
    word_t attr;
    word_t physpool;
    word_t virtpool;
    word_t zone;
} bi_new_ms_t;

typedef struct 
{
    bi_hdr_t header;
    word_t virt_base;
    word_t virt_end;
    word_t phys_base;
    word_t phys_end;
} bi_init_mem_t;

typedef struct {
    bi_hdr_t header;
    word_t pool;
    word_t base;
    word_t end;
} bi_add_virt_mem_t;

typedef struct {
    bi_hdr_t header;
    word_t pool;
    word_t base;
    word_t end;
} bi_add_phys_mem_t;

typedef struct {
    bi_hdr_t header;
    word_t owner;
    word_t ip;
    word_t user_main;
    word_t priority;
    PACKED(char name[1]);
} bi_new_thread_t;

typedef struct {
    bi_hdr_t header;
    word_t thread;
    word_t ms;
} bi_register_stack_t;

typedef struct {
    bi_hdr_t header;
    word_t name;
} bi_run_thread_t;

typedef struct {
    bi_hdr_t header;
    word_t vaddr;
    word_t size;
    word_t paddr;
    word_t scrub;
    word_t mode;
} bi_map_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t ms;
    word_t rights;
} bi_attach_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t obj;
    word_t rights;
} bi_grant_t;

typedef struct {
    bi_hdr_t header;
    word_t thread;
    PACKED(char arg[1]);
} bi_argv_t;

typedef struct {
    bi_hdr_t header;
    word_t thread;
    word_t ms;
} bi_register_server_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t ms;
} bi_register_callback_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t obj;
    word_t type;
} bi_security_control_t;

typedef struct {
    bi_hdr_t header;
    word_t obj;
    word_t rights;
} bi_new_cap_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t cap;
} bi_grant_cap_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t obj;
    word_t type;
    PACKED(char key[1]);
} bi_object_export_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t first;
    word_t second;
    word_t third;
    word_t fourth;
    word_t fifth;
    word_t sixth;
    word_t type;
    PACKED(char key[1]);
} bi_struct_export_t;

typedef struct {
    bi_hdr_t header;
    word_t pd;
    word_t ms;
} bi_register_env_t;

typedef struct {
    bi_hdr_t header;
    word_t is_virtual;
} bi_new_pool_t;

typedef struct {
    bi_hdr_t header;
    word_t pool;
} bi_new_zone_t;

typedef struct {
    bi_hdr_t header;
    word_t zone;
    word_t base;
} bi_add_zone_window_t;

typedef struct {
    bi_hdr_t header;
    word_t thread;
    word_t irq;
} bi_grant_interrupt_t;

typedef struct {
    bi_hdr_t header;
    word_t max_spaces;
    word_t max_mutexes;
    word_t max_root_caps;
} bi_kernel_info_t;


#endif /* !__BOOTINFO__PRIV_H__ */
