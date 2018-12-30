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

#include <l4.h>
#include <debug.h>
#include <kmemory.h>
#include <smallalloc.h>

small_alloc_block_t*
small_alloc_t::allocate_block(kmem_group_t * const group)
{
    word_t length = objs_per_block;
    void * block_base = kmem.alloc(group, SMALL_OBJECT_BLOCKSIZE, true);

    if (!block_base)
        return NULL;

    small_alloc_block_t *block = get_block(block_base);

    TRACESA("Allocating object block %p\n", block);

    /* Allocate at front of the block list? */
    if (!head || head->id != 0)
    {
        block->next = head;
        head = block;
        if (max_objs && (length >= max_objs)) {
            length = max_objs;
        }
        bitmap_init(block->get_bitmap(), length, true);
        return block;
    }

    small_alloc_block_t * current = head;
    word_t next_id = current->id + objs_per_block;

    while (current->next)
    {
        current = current->next;

        /* If there is a gap in the block list, insert block */
        if (current->id != next_id) {
            block->next = current->next;
            current->next = block;

            block->id = next_id;
            bitmap_init(block->get_bitmap(), length, true);
            return block;
        }

        next_id += objs_per_block;
    }

    /* current is the last in the block list */
    block->id = next_id;
    ASSERT(DEBUG, !max_objs || block->id < max_objs);
    current->next = block;

    if (max_objs && (length + next_id >= max_objs)) {
        length = max_objs - next_id;
    }
    bitmap_init(block->get_bitmap(), length, true);
    return block;
}

void small_alloc_t::free_block(kmem_group_t * const group, small_alloc_block_t *block)
{
    TRACESA("Freeing object block %p\n", block);

    /* Deleting head of the block list? */
    if (head == block) {
        head = block->next;
    } else {
        small_alloc_block_t * current = head;
        small_alloc_block_t * next = current->next;

        while (next && next != block) {
            current = next;
            next = current->next;
        }

        current->next = block->next;
    }
    kmem.free(group, get_block_base(block), SMALL_OBJECT_BLOCKSIZE);
}

void * small_alloc_t::allocate()
{
    lock.lock();
    if (max_objs && (num >= max_objs)) {
        TRACESA("Already have max number of objects (%d)\n", max);
        lock.unlock();
        return NULL;
    }

    /* If no free objects, allocate a new block */
    if (EXPECT_FALSE(!first_free)) {
        first_free = allocate_block(mem_group);
        if (!first_free){
            lock.unlock();
            return NULL;
        }
    }

    int position = bitmap_findfirstset(first_free->get_bitmap(), objs_per_block);
    ASSERT(DEBUG, position != -1);

    TRACESA("Allocating object %d\n", position + first_free->id);

    void * object = get_object(first_free, position);
    memset(object, 0, obj_size);

    bitmap_clear(first_free->get_bitmap(), position);

    /* Update first_free if all objects in the current block are used */
    while (bitmap_findfirstset(first_free->get_bitmap(), objs_per_block) == -1)
    {
        first_free = first_free->next;
        if (!first_free)
            break;
    }

    num++;

    lock.unlock();
    return object;
}

void small_alloc_t::free(void * object)
{
    ASSERT(DEBUG, object);
    lock.lock();
    small_alloc_block_t *block;

    TRACESA("Freeing %d\n", id(object));
    block = get_block(get_block_base(object));

    /* mark this space as free */
    bitmap_set(block->get_bitmap(), id(object) - block->id);

    /* if none free, mark this block as first_free */
    if (!first_free || (block->id < first_free->id)) {
        first_free = block;
        num --;
        lock.unlock();
        return;
    }

    /* Try to free this block if now unused */
    if (bitmap_isallset(block->get_bitmap(), objs_per_block))
    {
        free_block(mem_group, block);

        if (first_free == block)
        {
            first_free = head;
            while (first_free && bitmap_findfirstset(first_free->get_bitmap(), objs_per_block) == -1)
                first_free = first_free->next;
        }
    }
    num--;
    lock.unlock();
}

void SECTION(SEC_INIT)
small_alloc_t::init(kmem_group_t * const group, u32_t object_size, u32_t max_objects)
{
    lock.init();
    head = NULL;
    first_free = NULL;
    num = 0;
    this->obj_size = object_size;
    this->max_objs = max_objects;
    this->mem_group = group;

    word_t header_size = small_alloc_block_t::header_size(obj_size);
    word_t extra_space = SMALL_OBJECT_BLOCKSIZE % obj_size;

    if (extra_space >= header_size) {
        objs_per_block = SMALL_OBJECT_BLOCKSIZE / obj_size;
    } else {
        /*
         * The remainder is not enough for header, we need to sacrifice a few
         * objects for header.
         */
        objs_per_block = (SMALL_OBJECT_BLOCKSIZE / obj_size) -
            (header_size - extra_space) / obj_size - 1;
    }
    /* Header is near the end of the block */
    this->header_offset = objs_per_block * obj_size;
}

