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
 * Authors: Ben Leslie, Alex Webster
 */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "memsection.h"
#include "objtable.h"

static struct memsection *root;

static inline uintptr_t
objtable_key(const struct memsection *memsection)
{
    return memsection->base;
}

static inline uintptr_t
objtable_end(const struct memsection *memsection)
{
    return memsection->end;
}

static int
objtable_node_compare(struct memsection * node, uintptr_t key)
{
    if (key < node->base)
    {
        return -1;
    }
    else if (key > node->end)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static struct memsection *
find_node(struct memsection *root, uintptr_t key, int *cmp_p, struct memsection **parent)
{
    int     cmp = 0;
    struct  memsection *second_last_root = root;
    struct  memsection *last_root = root;

    while (root != NULL)
    {

        second_last_root = last_root;
        last_root = root;

        cmp = objtable_node_compare(root, key);

        if (cmp < 0)
        {
            root = root->left;
        }
        else if (cmp > 0)
        {
            root = root->right;
        }
        else if (cmp == 0)
        {
            break;
        }
    }

    *cmp_p = cmp;

    if (parent)
    {
        *parent = second_last_root;
    }

    return last_root;
}

int
objtable_insert(struct memsection *memsection)
{
    int     cmp;
    struct  memsection *closest;

    if (root == NULL)
    {
        root = memsection;
    }
    else
    {
        closest = find_node(root, objtable_key(memsection), &cmp, NULL);

        if (cmp == 0)
        {
            ERROR_PRINT
            (
                 "Failed to insert object into objtable (ms_base: %" PRIxPTR ")\n",
                 objtable_key(memsection)
            );
            return -1;          /* Key clash */
        }
        else if (cmp < 0)
        {
            closest->left = memsection;
        }
        else
        {
            closest->right = memsection;
        }
    }

    return 0;
}

static struct memsection *
largest_node(struct memsection *root, struct memsection **parent)
{
    struct memsection *last_root = NULL;

    while (root->right != NULL)
    {
        last_root = root;
        root = root->right;
    }

    if (parent)
    {
        *parent = last_root;
    }
    return root;
}

int
objtable_delete(const struct memsection *memsection)
{
    int     cmp;
    struct  memsection *dead_node, *deads_parent;
    struct  memsection *replacement_node, *replacements_parent;

    if (root == NULL)
    {
        return 0;
    }

    dead_node = find_node(root, objtable_key(memsection), &cmp, &deads_parent);

    /* Found a match, need to delete */
    if (cmp == 0)
    {
        /* Two children... promotion becomes difficult!  */
        if (dead_node->left != NULL && dead_node->right != NULL)
        {
            /*
             * We'll take the largest on the left, could also take the smallest
             * on the right. Ideally we'd take whichever was the longest path,
             * but I don't want to calculate that... We should really use
             * something better than a binary tree.
             */
            replacement_node = largest_node(dead_node->left, &replacements_parent);

            /*
             * If the dead node only has one child on the left, no need to mess
             * around with replacements_parent, which happens to be the dead node!
             *
             * We know that the replacement node is always on the right of its own
             * parent, unless it is the direct left child of the dead node.
             */
            if (replacements_parent != NULL)
            {
                replacements_parent->right = replacement_node->left;
            }

            replacement_node->left  = replacements_parent;
            replacement_node->right = dead_node->right;
        }

        else if (dead_node->left != NULL)
        {
            replacement_node    = dead_node->left;
        }
        else if (dead_node->right != NULL)
        {
            replacement_node    = dead_node->right;
        }
        else
        {
            replacement_node    = NULL;
        }

        if (dead_node == deads_parent->left)
        {
            deads_parent->left = replacement_node;
        }
        else if (dead_node == deads_parent->right)
        {
            deads_parent->right = replacement_node;
        }

        /* If the dead node was root, update the root */
        if (dead_node == root)
        {
            root = replacement_node;
        }

        return 0;
    }

    return 0;
}

void
objtable_init(void)
{
    root = NULL;
}

struct memsection *
objtable_lookup(void * addr)
{
    int     cmp;
    struct  memsection * memsection;

    if (root == NULL)
    {
        return 0;
    }

    memsection = find_node(root, (uintptr_t)addr, &cmp, NULL);

    if (cmp == 0)
    {
        return memsection;
    }
    else
    {
        /*
         * XXX: for bootstrapping
         */
        struct memsection_node *ms, *first_ms = internal_memsections.first;

        for (ms = first_ms; ms->next != first_ms; ms = ms->next)
        {
            if ((uintptr_t)addr >= ms->data.base && (uintptr_t)addr <= ms->data.end)
            {
                return &ms->data;
            }
        }
    }

    return NULL;
}

