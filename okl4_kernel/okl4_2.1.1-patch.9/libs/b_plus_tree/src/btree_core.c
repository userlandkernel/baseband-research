/*
 * Copyright (c) 1995-2004, University of New South Wales
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
 * Description: B+ tree main source file.
 * 
 * Authors: Daniel Potts <danielp@cse.unsw.edu.au> - ported to kenge 
 */

/*
 * btree.c: a b+tree index implementation, written 28/10/95 modified: 30/10/95 
 * (adding all the const) 1/11/95 BTModify 24/4/96 by Jerry to make it fit in
 * the Mungi kernel stuff 08/02/97 by Gernot adapted for OO1 use 22/02/97 by
 * Gernot cleaned up: - total separation of object and index structure -
 * interval support in index structure - user control over memory allocation. 
 */
/*
 * count is the number of keys (= number of children - 1) All key values in
 * child[j] are >= key[i], for j>i all key values in child[j] are < key[i], for 
 * j<=i. 
 */
#include BT_CONF_FILE
#include <stdint.h>
#include <stddef.h>

/* static declarations */
static int BTDelete(PagePool *pool, BTPage *const current, BTKey const key,
                    GBTObject *obj);
static int BTInsertPage(BTPage *const current, BTKeyCount const hi,
                        BTKey const kpromoted, BTPage *const ppromoted);
static int BTInsert(PagePool *pool, BTPage *const current, GBTObject const obj,
                    BTPage **const ppromoted, BTKey *const kpromoted,
                    GBTObject *ngb);
static int BTDeletePage(BTPage *const current, BTKeyCount idx);
static BTKey BTRedistribute(BTPage *const left, BTPage *const right);
static void BTCollapse(PagePool *pool, BTPage *const left, BTPage *const right);

#define min(x,y) (x>y?y:x)
#define max(x,y) (x>y?x:y)

int
EXPORT(BTSearch)(GBTree const btree, BTKey const key, GBTObject *obj)
{
    BTKeyCount lo, hi, mid;
    BTPage *current;

    if (!btree)
        return BT_INVALID;
    current = btree->root;
    if (!current)
        return BT_NOT_FOUND;

    for (;;) {                  /* use binary search to look thru the page */
        lo = 0;
        hi = current->count;
        while (lo < hi) {
            mid = (lo + hi) / 2;
            if (BTKeyEQ(current->key[mid], key)) {
                hi = mid + 1;
                break;
            } else if (BTKeyGT(current->key[mid], key))
                hi = mid;
            else
                lo = mid + 1;
        }
        if (current->isleaf) {
            break;
        }
        current = current->child[hi];
    }

    *obj = (GBTObject)(current->child[hi]);
    if (BTObjMatch(*obj, key)) {
        return BT_OK;
    } else {
        return BT_NOT_FOUND;
    }
}

int
EXPORT(BTIns)(GBTree const btree, GBTObject const obj, GBTObject *ngb)
{
    BTPage *ppromoted, *newroot;
    BTKey kpromoted;
    int retval;
    int i;

    if (btree == NULL) {        /* no tree */
        return BT_INVALID;
    }

    if (btree->root == NULL) {  /* tree empty */
        newroot = (BTPage *)EXPORT(AllocPage) (btree->pool);
        if (newroot == NULL)
            return BT_ALLOC_fail;

        for (i = 0; i < BT_MAXKEY; i++) {
            newroot->child[i] = NULL;
            newroot->key[i] = 0;
        }
        /*
         * Numbers of children is key+1, so ensure the last child is
         * initialised 
         */
        newroot->child[i] = NULL;

        newroot->count = 0;
        newroot->isleaf = 1;
        newroot->child[0] = (BTPage *)obj;
        btree->root = newroot;
        btree->depth = 1;
        *ngb = NULL;

        return BT_OK;
    }

    if ((retval = BTInsert(btree->pool, btree->root, obj,
                           &ppromoted, &kpromoted, ngb)) == BT_PROMOTION) {

        newroot = (BTPage *)EXPORT(AllocPage) (btree->pool);
        if (newroot == NULL)
            return BT_ALLOC_fail;

        for (i = 0; i < BT_MAXKEY; i++) {
            newroot->child[i] = NULL;
            newroot->key[i] = 0;
        }
        /*
         * Numbers of children is key+1, so ensure the last child is
         * initialised 
         */
        newroot->child[i] = NULL;

        newroot->count = 1;
        newroot->isleaf = 0;
        newroot->child[0] = btree->root;
        newroot->child[1] = ppromoted;
        newroot->key[0] = kpromoted;
        btree->root = newroot;
        btree->depth++;
        retval = BT_OK;
    }

    return retval;
}

int
EXPORT(BTDel)(GBTree const btree, BTKey const key, GBTObject *obj)
{
    BTPage *current;
    int retval;

    if (btree == NULL) {        /* no tree */
        return BT_INVALID;
    }

    if ((current = btree->root) == NULL) {      /* tree empty */
        return BT_NOT_FOUND;
    }

    if ((retval = BTDelete(btree->pool, current, key, obj)) == BT_LESS_THAN_MIN) {
        if (current->isleaf) {  /* only page */
            if (current->count < 0) {   /* B-tree now empty */
                EXPORT(FreePage) (btree->pool, current);
                btree->root = NULL;
                btree->depth = 0;
            }
        } else {
            if (current->count == 0) {
                btree->root = current->child[0];
                btree->depth--;
                EXPORT(FreePage) (btree->pool, current);
            }
        }
        retval = BT_OK;
    }
    return retval;
}

static void
GetLeastKey(BTPage *current, BTKey *const key)
{
    /* there must be a better way than this...  */
    while (!current->isleaf) {
        current = current->child[0];
    }
    *key = BTGetObjKey((GBTObject)(current->child[0]));
}

static int
BTDelete(PagePool *pool, BTPage *const current, BTKey const key, GBTObject *obj)
/*
 * delete key from b-tree with root pointed by current, returns: - BT_NOT_FOUND 
 * if there's no object with key - BT_LESS_THAN_MIN if deletion result in
 * current->count < BT_MINKEY [the caller should take care of collapsing or
 * redistributing keys] - BT_OK if key can be deleted successfully 
 */
{
    BTKeyCount lo, hi, mid;
    BTKeyCount med, sibling;
    int retval;

    if (!current)
        return BT_NOT_FOUND;

    /* use binary search to look thru the page */
    lo = 0;
    hi = current->count;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        if (BTKeyEQ(current->key[mid], key)) {
            hi = mid + 1;
            break;
        } else if (BTKeyGT(current->key[mid], key))
            hi = mid;
        else
            lo = mid + 1;
    }

    if (current->isleaf) {
#if 0                           // remove when confident
        if (!BTKeyEQ(BTGetObjKey((GBTObject)(current->child[hi])), key)) {
#else
        /* danielp: Was BTKeyEQ, but I think it should be BTObjMatch() */
        if (!BTObjMatch((GBTObject)(current->child[hi]), key)) {
#endif
            return BT_NOT_FOUND;
        }
        *obj = (GBTObject)(current->child[hi]);
        retval = BTDeletePage(current, hi - 1);
    } else {
        retval = BTDelete(pool, current->child[hi], key, obj);
        if (hi && (retval == BT_OK || retval == BT_LESS_THAN_MIN) &&
            BTKeyEQ(current->key[hi - 1], key)) {
            GetLeastKey(current->child[hi], &(current->key[hi - 1]));
        }
        if (retval == BT_LESS_THAN_MIN) {
            if (current->count) {
                /* try to redistribute first, identify fattest sibling */
                sibling = (!hi) ? 1 :
                    ((hi == current->count) ? hi - 1 :
                    ((current->child[hi - 1]->count >
                    current->child[hi + 1]->count) ? hi - 1 : hi + 1));
                med = (hi + sibling) / 2;
                if (current->child[sibling]->count > BT_MINKEY) {
                    current->key[med] =
                        BTRedistribute(current->child[min(hi, sibling)],
                                    current->child[max(hi, sibling)]);
                    retval = BT_OK;
                } else {
                    /* collapse the two pages, get the median value down */
                    BTCollapse(pool, current->child[min(hi, sibling)],
                            current->child[max(hi, sibling)]);
                    retval = BTDeletePage(current, med);
                }
            } else {
                retval = BT_OK;
            }
        }
    }                           /* if (current->isleaf) */
    return retval;
}

static int
BTInsertPage(BTPage *const current, BTKeyCount const hi,
             BTKey const kpromoted, BTPage *const ppromoted)
/*
 * insert kpromoted with child ppromoted at index hi in page current, ppromoted 
 * is kpromoted's right child 
 */
{
    BTKeyCount i;

    for (i = current->count - 1; i >= max(hi, 0); i--) {
        current->key[i + 1] = current->key[i];
        current->child[i + 2] = current->child[i + 1];
    }
    if (hi < 0) {
        current->key[0] = BTGetObjKey((GBTObject)(current->child[0]));
        current->child[1] = current->child[0];
        current->child[0] = ppromoted;
    } else {
        current->key[hi] = kpromoted;
        current->child[hi + 1] = ppromoted;
    }
    current->count++;
    return BT_OK;
}                               /* BTInsertage */

static int
BTInsert(PagePool *pool, BTPage *const current, GBTObject const obj,
         BTPage **const ppromoted, BTKey *const kpromoted, GBTObject *ngb)
/*
 * insert obj in the b+tree with b-tree index pointed by current, return:
 * BT_PROMOTION : if current splitted to two pages, ppromoted will point to
 * the right child of key kpromoted (while current will hold the left child)
 * BT_OK : if insert succesfull BT_DUPLICATE : if there has been another object 
 * with the same key in tree BT_ALLOC_fail : if there's not enough memory to
 * allocate a new page (the upper level index can be inconsistent with the
 * lower level) 
 */
{
    BTKeyCount lo, hi, mid;
    BTKeyCount i, j;
    BTPage *newpage;
    int retval;
    BTKey key, kmed;            /* median key */
    int in_old;

    if (current == NULL) {
        *kpromoted = BTGetObjKey(obj);
        *ppromoted = NULL;      /* this is the _initialization_ for all ptr */
        return BT_PROMOTION;
    }

    key = BTGetObjKey(obj);
    /* use binary search to look thru the page */
    lo = 0;
    hi = current->count;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        if (BTKeyEQ(current->key[mid], key)) {
            return BT_DUPLICATE;
        } else if (BTKeyGT(current->key[mid], key))
            hi = mid;
        else
            lo = mid + 1;
    }

#ifdef BT_HAVE_INTERVALS
    if (hi < current->count && BTObjMatch(obj, current->key[hi])) {
        return BT_OVERLAP;
    }
#endif /* BT_HAVE_INTERVALS */

    if (current->isleaf) {
#ifdef BT_HAVE_INTERVALS
        if (BTObjMatch((GBTObject)(current->child[hi]), key) ||
            (!hi && BTOverlaps(obj, (GBTObject)(current->child[0])))) {
            return BT_OVERLAP;
        }
#endif /* BT_HAVE_INTERVALS */
        *ppromoted = (BTPage *)obj;
        *kpromoted = BTGetObjKey(obj);
        retval = BT_PROMOTION;
        *ngb = (GBTObject)(current->child[hi]);
        if (!hi && BTKeyLT(key, BTGetObjKey((GBTObject)(current->child[0])))) {
            hi = -1;
        }
    } else {
        retval =
            BTInsert(pool, current->child[hi], obj, ppromoted, kpromoted, ngb);
    }
    if (retval == BT_PROMOTION) {
        if (current->count < BT_MAXKEY) {
            return BTInsertPage(current, hi, *kpromoted, *ppromoted);
        }
        /*
         * split the page BT_MINKEY keys on left (org) and BT_MAXKEY-BT_MINKEY
         * on right 
         */
        newpage = (BTPage *)EXPORT(AllocPage) (pool);
        if (newpage == NULL)
            return BT_ALLOC_fail;
        newpage->isleaf = current->isleaf;
        newpage->count = BT_MAXKEY - BT_MINKEY;
        current->count = BT_MINKEY;
        in_old = hi < current->count;
        if (in_old) {
            /* insert promoted key in the left (original) page */
            current->count--;
            kmed = current->key[current->count];
            current->key[current->count] = 0;
        } else {                /* insert promoted key in the right (new) page */
            newpage->count--;
            if (hi == current->count) {
                kmed = *kpromoted;
            } else {
                kmed = current->key[current->count];
                current->key[current->count] = 0;
            }
        }
        newpage->child[0] = current->child[current->count + 1];
        current->child[current->count + 1] = NULL;
        for (j = current->count + 1, i = 0; i < newpage->count; i++, j++) {
            newpage->key[i] = current->key[j];
            newpage->child[i + 1] = current->child[j + 1];
            current->key[j] = 0;
            current->child[j + 1] = NULL;
        }
        for (; i < BT_MAXKEY; i++) {
            newpage->key[i] = 0;
            newpage->child[i + 1] = NULL;
        }

        if (in_old) {
            retval = BTInsertPage(current, hi, *kpromoted, *ppromoted);
        } else {
            hi -= current->count + 1;
            retval = BTInsertPage(newpage, hi, *kpromoted, *ppromoted);
        }
        *kpromoted = kmed;
        *ppromoted = newpage;
        return BT_PROMOTION;
    }
    return retval;              /* can't just return ok, sth might go wrong */
}                               /* BTInsert */

static int
BTDeletePage(BTPage *const current, BTKeyCount idx)
/*
 * Delete key with index idx in page current, overwrites the right child ptr
 * current != NULL, idx < current->count, possible return values: BT_OK : if
 * the number of keys remaining within the page is >= BT_MINKEY
 * BT_LESS_THAN_MIN : if the page after deletion holds < BT_MINKEY keys 
 */
{
    BTKeyCount i;

    if (idx < 0) {
        current->child[0] = current->child[1];
        idx++;
    }
    for (i = idx + 1; i < current->count; i++) {
        current->key[i - 1] = current->key[i];
        current->child[i] = current->child[i + 1];
    }
    current->child[current->count] = NULL;
    current->count--;
    if (current->count >= 0) {
        current->key[current->count] = 0;
    }
    return (current->count < BT_MINKEY) ? BT_LESS_THAN_MIN : BT_OK;
}

static void
BTCollapse(PagePool *pool, BTPage *const left, BTPage *const right)
/* make left and right one page, copy to left, kmed is the median from parent */
{
    BTKeyCount i, j;
    BTKey kmed;

    GetLeastKey(right, &kmed);
    left->key[left->count] = kmed;
    left->child[left->count + 1] = right->child[0];
    for (i = 0, j = left->count + 1; i < right->count; i++, j++) {
        left->key[j] = right->key[i];
        left->child[j + 1] = right->child[i + 1];
    }
    left->count = left->count + right->count + 1;
    EXPORT(FreePage) (pool, right);
}                               /* BTCollapse */

static BTKey
BTRedistribute(BTPage *const left, BTPage *const right)
/*
 * redistribute from left to right page [kmed is the median value b/w left and
 * right], returns the new median 
 */
{
    BTKeyCount half, delta, i, j;
    BTKey kmedian, kmed;

    half = (left->count + right->count) >> 1;
    GetLeastKey(right, &kmed);
    if (left->count < half) {
        /* get (half - left->count)-1 keys from right to left */
        delta = (half - left->count);   /* distance travelled */
        left->key[left->count] = kmed;
        for (i = left->count + 1, j = 0; i < half; i++, j++) {
            left->key[i] = right->key[j];
            left->child[i] = right->child[j];
        }
        left->child[half] = right->child[j];
        kmedian = right->key[j];
        right->child[0] = right->child[j + 1];
        i = 0;
        while (++j < right->count) {
            right->key[i] = right->key[j];
            right->child[i + 1] = right->child[j + 1];
            right->key[j] = 0;
            right->child[j + 1] = NULL;
            i++;
        }
        right->count = right->count - delta;
        left->count = half;
    } else {
        /* move (left->count - half -1) keys from left to right */
        delta = left->count - half;     /* distance travelled */
        /* make space */
        right->child[right->count + delta] = right->child[right->count];
        for (j = right->count + delta - 1, i = right->count - 1; i >= 0;
             j--, i--) {
            right->key[j] = right->key[i];
            right->child[j] = right->child[i];
        }
        right->key[j] = kmed;
        right->child[j] = left->child[left->count];
        i = left->count - 1;
        j--;
        while (j >= 0) {
            right->key[j] = left->key[i];
            right->child[j] = left->child[i];
            left->key[i] = 0;
            left->child[i] = NULL;
            i--;
            j--;
        }
        kmedian = left->key[half];
        left->key[half] = 0;
        left->count = half;
        right->count = right->count + delta;
    }
    return kmedian;
}                               /* BTRedistribute */
