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
 * Description:   Kernel Small Object Allocator
 */
#ifndef __SMALLALLOC_H__
#define __SMALLALLOC_H__

#include <kernel/bitmap.h>
#include <kernel/generic/lib.h>

#define TRACESA(x...)
//#define TRACESA TRACEF

class small_alloc_block_t;

class small_alloc_t
{
public:
    /* initialize the allocator (max = 0 means unlimited) */
    void init(kmem_group_t * const group, u32_t obj_size, u32_t max_objs = 0);

    /* allocate an object */
    void * allocate();

    /* free an object */
    void free(void * object);

    /* get the ID of the object allocated */
    word_t id(void * object);

private:
    small_alloc_block_t * allocate_block(kmem_group_t * const group);
    void free_block(kmem_group_t * const group, small_alloc_block_t *block);

    void * get_object(small_alloc_block_t * block, word_t index)
        {
            word_t offset = index * obj_size;
            return (void *)((word_t)block - header_offset + offset);
        }

    void * get_block_base(void * object)
        {
            return (void *)((word_t)object & ~(SMALL_OBJECT_BLOCKSIZE - 1UL));
        }

    void * get_block_base(small_alloc_block_t * block)
        {
            return (void *)((word_t)block - header_offset);
        }

    small_alloc_block_t * get_block(void * base)
        {
            return (small_alloc_block_t *)((word_t)base + header_offset);
        }

private:
    small_alloc_block_t * head;
    small_alloc_block_t * first_free;
    kmem_group_t * mem_group;

    u32_t num;
    u32_t max_objs;

    u16_t obj_size;
    u16_t objs_per_block;
    u16_t header_offset;
    
    spinlock_t lock;
};

class small_alloc_block_t
{
public:
    static word_t header_size(word_t obj_size)
        {
            return (2 + BITMAP_SIZE(SMALL_OBJECT_BLOCKSIZE/obj_size)) * sizeof(word_t);
        }

public:
    bitmap_t * get_bitmap(void)
        {
            return &bitmap[0];
        }
public:
    small_alloc_block_t * next;
    word_t id;
    bitmap_t bitmap[1];
};

INLINE word_t small_alloc_t::id (void * object)
{
    word_t id;
    void *base = get_block_base(object);

    small_alloc_block_t *block = get_block(base);
    id = ((word_t)object - (word_t)base) / obj_size;
    id += block->id;

    return id;
}

#endif
