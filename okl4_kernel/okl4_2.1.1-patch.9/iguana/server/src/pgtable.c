/*
 * Copyright (c) 2005, National ICT Australia
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
 * Author: Alex Webster
 */

/*
 * XXX: This implements a simple multilevel page table, which we will
 * need until we can get libgpt up and running.
 */
#include <assert.h>
#include <l4/types.h>
#include "pgtable.h"
#include "slab_cache.h"

#if defined(L4_32BIT)
static unsigned int level[] = { 32, 28, 24, 20, 16, 12 };
static unsigned int mask[] = { 0, 0xf, 0xf, 0xf, 0xf, 0xf };

typedef union {
    uintptr_t raw;
    struct {
        uintptr_t is_valid:1;
    } invalid;
    struct {
        uintptr_t is_valid:1;
        uintptr_t is_subtree:1;
        uintptr_t ptr:30;
    } subtree;
    struct {
        uintptr_t is_valid:1;
        uintptr_t is_subtree:1;
        uintptr_t bits:6;
        uintptr_t phys:22;
    } leaf;
} pte_t;
#elif defined(L4_64BIT)
static unsigned int level[] = { 40, 36, 32, 28, 24, 20, 16, 12 };
static unsigned int mask[] = { 0, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf };

typedef union {
    uintptr_t raw;
    struct {
        uintptr_t is_valid:1;
    } invalid;
    struct {
        uintptr_t is_valid:1;
        uintptr_t is_subtree:1;
        uintptr_t ptr:62;
    } subtree;
    struct {
        uintptr_t is_valid:1;
        uintptr_t is_subtree:1;
        uintptr_t bits:6;
        uintptr_t phys:54;
    } leaf;
} pte_t;
#endif

static pte_t root;
static struct slab_cache pt_cache =
SLAB_CACHE_INITIALIZER((16 * sizeof(pte_t)), &pt_cache);

static inline unsigned int
get_bits(const pte_t *ent)
{
    return ent->leaf.bits;
}

static pte_t *
get_child(uintptr_t addr, const pte_t *ent, unsigned int d)
{
    return &((pte_t *)(ent->subtree.ptr << 2))[addr >> level[d] & mask[d]];
}

static pte_t *
get_fst_child(uintptr_t addr, const pte_t *ent, unsigned int d)
{
    pte_t *bas, *fst;

    bas = get_child(0UL, ent, d);
    fst = get_child(addr, ent, d);
    while (fst > bas && (fst - 1)->raw == fst->raw)
        fst--;
    return fst;
}

static pte_t *
get_lst_child(uintptr_t addr, unsigned int bits, pte_t *ent, unsigned int d)
{
    pte_t *end, *lst;

    end = get_child(~0UL, ent, d);
    lst = get_child(addr + (1UL << bits) - 1, ent, d);
    while (lst < end && (lst + 1)->raw == lst->raw)
        lst++;
    return lst;
}

static inline uintptr_t
get_phys(pte_t *ent)
{
    return ent->leaf.phys << 10;
}

static inline pte_t *
get_ptr(pte_t *ent)
{
    return (pte_t *)(ent->subtree.ptr << 2);
}

static inline int
is_leaf(pte_t *ent)
{
    return ent->leaf.is_valid && !ent->leaf.is_subtree;
}

static inline int
is_subtree(pte_t *ent)
{
    return ent->subtree.is_valid && ent->subtree.is_subtree;
}

static inline int
is_valid(pte_t *ent)
{
    return ent->invalid.is_valid;
}

static int
make_invalid(pte_t *ent)
{
    if (is_subtree(ent))
        slab_cache_free(&pt_cache, get_ptr(ent));
    ent->invalid.is_valid = 0;
    return 1;
}

static int
make_leaf(pte_t *ent, unsigned int bits, uintptr_t phys)
{
    ent->leaf.is_valid = 1;
    ent->leaf.is_subtree = 0;
    ent->leaf.bits = bits;
    ent->leaf.phys = phys >> 10;
    return 1;
}

static int
make_subtree(pte_t *ent, unsigned int d)
{
    void *mem = slab_cache_alloc(&pt_cache);

    if (mem == NULL)
        return 0;
    ent->subtree.is_valid = 1;
    ent->subtree.is_subtree = 1;
    ent->subtree.ptr = (uintptr_t)mem >> 2;
    return 1;
}

/*
 * r_insert: Return 0 on fail, 1 on success
 */
static int
r_insert(uintptr_t addr, unsigned int bits, uintptr_t phys, pte_t *ent,
         unsigned int d)
{
    pte_t *cur, *fst, *lst;
    int r;

    if (level[d - 1] <= bits) {
        if (is_subtree(ent)) {
            r = pt_remove(addr, level[d]);
            assert(r);
        }
        return make_leaf(ent, bits, phys);
    } else if (!is_subtree(ent)) {
        if (!make_subtree(ent, d)) {
            return 0;
        }
    }
    fst = get_child(addr, ent, d);
    lst = get_child(addr + (1UL << bits) - 1, ent, d);
    for (cur = fst; cur <= lst; cur++) {
        if (!r_insert(addr, bits, phys, cur, d + 1)) {
            return 0;
        }
    }
    return 1;
}

int
pt_insert(uintptr_t addr, unsigned int bits, uintptr_t phys)
{
    /* Fail if the object is too small. */
    if (bits < level[(sizeof(level) / sizeof(unsigned int)) - 1])
        return 0;
    /* Fail if the mapping is not page aligned. */
    if (addr % (1UL <<  bits))
        return 0;

    return r_insert(addr, bits, phys, &root, 1);
}

int
pt_lookup(uintptr_t addr, unsigned int *bits, uintptr_t *phys)
{
    pte_t *ent = &root;
    unsigned int d = 1;

    while (is_subtree(ent)) {
        ent = get_child(addr, ent, d);
        d++;
    }
    if (!is_leaf(ent))
        return 0;
    *bits = get_bits(ent), *phys = get_phys(ent);
    return 1;
}

static int
r_remove(uintptr_t addr, unsigned int bits, pte_t *ent, unsigned int d)
{
    pte_t *cur, *fst, *lst;
    int r;

    if (is_subtree(ent)) {
        fst = get_fst_child(addr, ent, d);
        lst = get_lst_child(addr, bits, ent, d);
        for (cur = fst; cur <= lst; cur++) {
            r = r_remove(addr, bits, cur, d + 1);
            assert(r);
        }
        fst = get_child(0UL, ent, d);
        lst = get_child(~0UL, ent, d);
        for (cur = fst; cur <= lst; cur++)
            if (is_valid(cur))
                return 1;
    }
    return make_invalid(ent);
}

int
pt_remove(uintptr_t addr, unsigned int bits)
{
    return r_remove(addr, bits, &root, 1);
}

#if defined(CONFIG_STATS)
void
pt_stats(size_t* pte_size)
{
    *pte_size = sizeof(pte_t);
}
#endif
