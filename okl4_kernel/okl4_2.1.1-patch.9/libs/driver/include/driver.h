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
#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdint.h>
#include <stddef.h>

#include <queue/queue.h>

#include <driver/system/driver_env.h>

#define NULL_MEM_HANDLE ((mem_handle_t){NULL,0})

/*
 * A descriptor of an I/O request for the IDE driver.
 */
typedef struct {
    mem_handle_t handle;        /* where in memory to transfer to/from */
    uint64_t blockid;           /* Where on disc to transfer to/from */
    uint64_t block_count;       /* How many sectors to transfer */
    uintptr_t key;              /* Tag assigned by caller */
} block_t;

struct data {
    mem_handle_t data;
    size_t length;
    uintptr_t xferred;
};

struct frag {
    mem_handle_t vaddr;         /* payload location in virtual mem */
    mem_handle_t paddr;         /* payload location in DMA space */
    int len;                    /* payload length, -1 invalid */
    int size;                   /* allocated size of frag data */
    struct frag *next;          /* Chained for now, this is pointless when we
                                 * deal with arrays in drivers */
};

typedef struct packet {
    struct frag *header;        /* Fast header allocation, set NULL on creation 
                                 */
    struct frag *head;
    int length;                 /* Payload data length, includes header */
    int frag_count;             /* Frag count, does not include header */
    void *stack_ref;
    STAILQ_ENTRY(packet) chain;
} packet_t;

struct pci_id {
    uint16_t vendor;
    uint16_t device;
};

typedef void (*callback_t) (callback_data_t, int, struct data *, uintptr_t);
extern callback_t null_callback;

struct driver_ops;
struct block_ops;
struct network_ops;
struct character_ops;
struct timer_ops;
struct buffer_ops;
struct packet_ops;

enum driver_class { block_device, netif_device, character_device,
        timer_device_e, buffer_manager, packet_device };

struct driver {
    const char *name;
    enum driver_class class;
    union {
        struct driver_ops *d_ops;
        struct block_ops *b_ops;
        struct netif_ops *n_ops;
        struct character_ops *c_ops;
        struct timer_ops *t_ops;
        struct buffer_ops *bfr_ops;
        struct packet_ops *p_ops;
    } ops;
    void *priv;
};

struct driver_instance {
    struct driver *classp;
};

typedef enum direction {
    mem_to_bus,
    bus_to_mem,
    consistent,
} direction_t;

int get_pci_consistent_memory(dma_handle_t dma, size_t size,
                              mem_handle_t *handle, dmabuf_handle_t *busaddr);
int setup_dma(dma_handle_t dma, mem_handle_t handle, size_t size,
              dmabuf_handle_t *dmabuf, direction_t dir);
int teardown_dma(dma_handle_t dma, dmabuf_handle_t *dmabuf);

#endif /* _DRIVER_H_ */
