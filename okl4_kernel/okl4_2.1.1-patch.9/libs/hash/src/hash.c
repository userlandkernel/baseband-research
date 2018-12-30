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
/*
 * Simple hash based on Mungi hash functions. Maps ints to void pointers.
 */
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <hash/hash.h>

struct hashtable *
hash_init(unsigned int size)
{
    struct hashtable *tablestruct;
    int counter;

    /*
     * Our hash function only works with power-of-2 bucket sizes for speed. 
     */
    assert((size & (size - 1)) == 0);

    tablestruct = malloc(sizeof(struct hashtable));
    assert(tablestruct);
    if (!tablestruct) {
        return NULL;
    }
    tablestruct->table = malloc(size * sizeof(struct hashentry *));
    if (!tablestruct->table) {
        return NULL;
    }
    for (counter = 0; counter < size; counter++) {
        tablestruct->table[counter] = NULL;
    }
    assert(tablestruct->table);
    tablestruct->size = size;
    tablestruct->spares = NULL;

    return tablestruct;
}

/*
 * Ref http://www.concentric.net/~Ttwang/tech/inthash.htm 
 */
uintptr_t
hash_hash(uintptr_t key)
{
#if (UINTPTR_MAX == UINT32_MAX) /*lint -e30 */
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
#elif (UINTPTR_MAX == UINT64_MAX)  /*lint -e30 The linter can't
                                    * grok UINTPTR_MAX */
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
#else
#error unsupported word size
#endif
    // printf ("new key is %d\n", key);
    return key;
}

void *
hash_lookup(struct hashtable *tablestruct, uintptr_t key)
{
    uintptr_t hash;
    struct hashentry *entry;

    hash = hash_hash(key) & (tablestruct->size - 1);
    for (entry = tablestruct->table[hash]; entry != NULL; entry = entry->next) {
        if (entry->key == key) {
            return entry->value;
        }
    }
    return NULL;
}

/*
 * Add the key to the hash table. Assumes the key is not already present. 
 */
int
hash_insert(struct hashtable *tablestruct, uintptr_t key, void *value)
{
    uintptr_t hash;
    struct hashentry *entry;

    hash = hash_hash(key) & (tablestruct->size - 1);
    // printf ("bucket is %d\n", hash);

    entry = malloc(sizeof(struct hashentry));
    if (!entry) {
        return -1;
    }
    entry->key = key;
    entry->value = value;
    entry->next = tablestruct->table[hash];

    tablestruct->table[hash] = entry;
    return 0;
}

/*
 * Removes the key from the hash table. Does not signal an error if the key was 
 * not present. 
 */
void
hash_remove(struct hashtable *tablestruct, uintptr_t key)
{
    uintptr_t hash;
    struct hashentry *entry, *tmpentry;

    hash = hash_hash(key) & (tablestruct->size - 1);
    entry = tablestruct->table[hash];
    /*
     * If this is the first entry then it needs special handling. 
     */
    if (entry && entry->key == key) {
        tmpentry = entry->next;
        free(entry);
        tablestruct->table[hash] = tmpentry;
    } else {
        while (entry) {
            if (entry->next && entry->next->key == key) {
                tmpentry = entry->next;
                entry->next = entry->next->next;
                free(tmpentry);
                break;
            }
            entry = entry->next;
        }
    }
}
