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
 * Author: Ben Leslie
 * Created: Tue Nov 9 2004 
 */
#include <stdlib.h>
#include <string.h>
#include <binary_tree/binary_tree.h>

struct bin_tree *
binary_tree_new(void)
{
    struct bin_tree *new_tree;
    new_tree = malloc(sizeof(struct bin_tree));
    if (new_tree == NULL) {
        return new_tree;
    }
    new_tree->root = NULL;
    return new_tree;
}

static struct bin_tree_node *
find_node(struct bin_tree_node *root, char *key, int *cmp_p,
          struct bin_tree_node **parent)
{
    int cmp = 0;
    struct bin_tree_node *second_last_root = root;
    struct bin_tree_node *last_root = root;

    while (root != NULL) {
        second_last_root = last_root;
        last_root = root;

        cmp = strcmp(key, root->key);

        if (cmp == 0) {
            break;
        }
        if (cmp < 0) {
            root = root->left;
        } else {
            root = root->right;
        }
    }
    *cmp_p = cmp;
    if (parent) {
        *parent = second_last_root;
    }
    return last_root;
}

/*
 * Return the largest in this tree.. just go right 
 */
static struct bin_tree_node *
largest_node(struct bin_tree_node *root, struct bin_tree_node **parent)
{
    struct bin_tree_node *last_root = NULL;

    while (root->right != NULL) {
        last_root = root;
        root = root->right;
    }
    if (parent) {
        *parent = last_root;
    }
    return root;
}

int
binary_tree_insert(struct bin_tree *tree, char *key, void *data)
{
    struct bin_tree_node *closest;
    struct bin_tree_node *new_node;
    int cmp;

    new_node = malloc(sizeof(struct bin_tree_node));
    if (new_node == NULL)
        return -1;

    new_node->key = key;
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;

    if (tree->root == NULL) {
        tree->root = new_node;
    } else {
        closest = find_node(tree->root, key, &cmp, NULL);
        if (cmp == 0) {
            free(new_node);
            return -2;          /* Key clash */
        }
        if (cmp < 0) {
            closest->left = new_node;
        } else {
            closest->right = new_node;
        }
    }
    return 0;
}

void *
binary_tree_lookup(struct bin_tree *tree, char *key)
{
    int cmp;
    struct bin_tree_node *closest;

    if (tree->root == NULL)
        return 0;
    closest = find_node(tree->root, key, &cmp, NULL);
    if (cmp == 0) {
        return closest->data;
    }
    return 0;
}

int
binary_tree_remove(struct bin_tree *tree, char *key)
{
    int cmp;
    struct bin_tree_node *dead_node, *parent, *replacement_node;

    if (tree->root == NULL)
        return 0;
    dead_node = find_node(tree->root, key, &cmp, &parent);
    if (cmp == 0) {
        /* Need to delete */
        if (dead_node->left != NULL && dead_node->right != NULL) {
            /* Two children... promotion becomes difficult!  */
            /*
             * We'll take the largest on the left, could also take the smallest 
             * on the right. Ideally we'd take whichever was the longest path,
             * but I don't want to calculate that.. We should really use
             * something better than a binary tree. 
             */
            replacement_node = largest_node(dead_node->left, &parent);
            if (parent == NULL) {
                /*
                 * This happens if the largest on the left, is the first node
                 * of the left 
                 */
                parent = dead_node;
            }
            /* Inplace replace our dead-node with this new node */
            dead_node->key = replacement_node->key;
            dead_node->data = replacement_node->data;

            /*
             * Now we actually delete the repalced node, which must have at
             * most 1 child, which is easy to delete 
             */
            dead_node = replacement_node;
        }

        /*
         * Ok, by here we only have at most one child 
         */
        if (dead_node->left != NULL) {
            replacement_node = dead_node->left;
        } else if (dead_node->right != NULL) {
            replacement_node = dead_node->right;
        } else {
            /* Leaf node... easy!  */
            replacement_node = NULL;
        }
        /* Now slot in the replacement node, and kill the dead_node */
        if (tree->root == dead_node) {
            tree->root = replacement_node;
        } else {
            if (parent->left == dead_node)
                parent->left = replacement_node;
            else
                parent->right = replacement_node;
        }
        free(dead_node);
        return 0;
    } else {
        return -1;
    }
}
