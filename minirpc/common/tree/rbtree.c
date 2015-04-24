
/*
** Copyright (C) 2012 AsiaInfo-linkage
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class rad black tree.
*/

#include "rbtree.h"
#include <stdlib.h>
#include <stdio.h>
/*
 * The red-black tree code is based on the algorithm described in
 * the "Introduction to Algorithms" by Cormen, Leiserson and Rivest.
 */

void
rbtree_insert( rbtree_t *tree,
    rbtree_node_t *node)
{
    rbtree_node_t  **root, *temp, *sentinel;

    /* a binary tree insert */

    root = (rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        rbt_black(node);
        *root = node;

        return;
    }

    tree->insert(*root, node, sentinel);

    /* re-balance tree */

    while (node != *root && rbt_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (rbt_is_red(temp)) {
                rbt_black(node->parent);
                rbt_black(temp);
                rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rbtree_left_rotate(root, sentinel, node);
                }

                rbt_black(node->parent);
                rbt_red(node->parent->parent);
                rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (rbt_is_red(temp)) {
                rbt_black(node->parent);
                rbt_black(temp);
                rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rbtree_right_rotate(root, sentinel, node);
                }

                rbt_black(node->parent);
                rbt_red(node->parent->parent);
                rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }
    tree->num_elements++;

    rbt_black(*root);
}


void
rbtree_insert_value(rbtree_node_t *temp, rbtree_node_t *node,
    rbtree_node_t *sentinel)
{
    rbtree_node_t  **p;

    for ( ;; ) {

        p = (node->key < temp->key) ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    rbt_red(node);
}


void
rbtree_insert_timer_value(rbtree_node_t *temp, rbtree_node_t *node,
    rbtree_node_t *sentinel)
{
    rbtree_node_t  **p;

    for ( ;; ) {

        /*
         * Timer values
         * 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         */

        /*  node->key < temp->key */

        p = ((rbtree_key_int_t) (node->key - temp->key) < 0)
            ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    rbt_red(node);
}


void
rbtree_delete(rbtree_t *tree,
    rbtree_node_t *node)
{
    unsigned int   red;
    rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = (rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = rbtree_min(node->right, sentinel);

        if (subst->left != sentinel) {
            temp = subst->left;
        } else {
            temp = subst->right;
        }
    }

    if (subst == *root) {
        *root = temp;
        rbt_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (rbt_is_red(w)) {
                rbt_black(w);
                rbt_red(temp->parent);
                rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (rbt_is_black(w->left) && rbt_is_black(w->right)) {
                rbt_red(w);
                temp = temp->parent;

            } else {
                if (rbt_is_black(w->right)) {
                    rbt_black(w->left);
                    rbt_red(w);
                    rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                rbt_copy_color(w, temp->parent);
                rbt_black(temp->parent);
                rbt_black(w->right);
                rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (rbt_is_red(w)) {
                rbt_black(w);
                rbt_red(temp->parent);
                rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (rbt_is_black(w->left) && rbt_is_black(w->right)) {
                rbt_red(w);
                temp = temp->parent;

            } else {
                if (rbt_is_black(w->left)) {
                    rbt_black(w->right);
                    rbt_red(w);
                    rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                rbt_copy_color(w, temp->parent);
                rbt_black(temp->parent);
                rbt_black(w->left);
                rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }
    tree->num_elements--;
    rbt_black(temp);
}

/* -----------------------------------------------------------
|   node           right
|   / \    ==>     / \
|  a  right     node  y
|       / \       / \
|       b  y     a   b    //left rotate
-----------------------------------------------------------*/
static inline void
rbtree_left_rotate(rbtree_node_t **root, rbtree_node_t *sentinel,
    rbtree_node_t *node)
{
    rbtree_node_t  *right;

    right = node->right;
    node->right = right->left;

    if (right->left != sentinel) {
        right->left->parent = node;
    }

    right->parent = node->parent;

    if (node == *root) {
        *root = right;

    } else if (node == node->parent->left) {
        node->parent->left = right;

    } else {
        node->parent->right = right;
    }

    right->left = node;
    node->parent = right;
}

/*-----------------------------------------------------------
|       node            left
|       / \             /  \
|    left  y   ==>    a    node
|   / \                   /  \
|  a   b                  b   y  //right rotate
-----------------------------------------------------------*/
static inline void
rbtree_right_rotate(rbtree_node_t **root, rbtree_node_t *sentinel,
    rbtree_node_t *node)
{
    rbtree_node_t  *left;

    left = node->left;
    node->left = left->right;

    if (left->right != sentinel) {
        left->right->parent = node;
    }

    left->parent = node->parent;

    if (node == *root) {
        *root = left;

    } else if (node == node->parent->right) {
        node->parent->right = left;

    } else {
        node->parent->left = left;
    }

    left->right = node;
    node->parent = left;
}

/*
 *  Walk the tree, Pre-order
 *
 *  We call ourselves recursively for each function, but that's OK,
 *  as the stack is only log(N) deep, which is ~12 entries deep.
 */
static int WalkNodePreOrder(rbtree_node_t *X,
                int (*callback)(rbtree_node_t *), rbtree_node_t *sentinel)
{
    int rcode;

    rcode = callback(X);
    if (rcode != 0) return rcode;

    if (X->left != sentinel) {
        rcode = WalkNodePreOrder(X->left, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    if (X->right != sentinel) {
        rcode = WalkNodePreOrder(X->right, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    return 0;       /* we know everything returned zero */
}

/*
 *  Inorder
 */
static int WalkNodeInOrder(rbtree_node_t *X,
               int (*callback)(rbtree_node_t *), rbtree_node_t *sentinel)
{
    int rcode;

    if (X->left != sentinel) {
        rcode = WalkNodeInOrder(X->left, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    rcode = callback(X);
    if (rcode != 0) return rcode;

    if (X->right != sentinel) {
        rcode = WalkNodeInOrder(X->right, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    return 0;       /* we know everything returned zero */
}


/*
 *  PostOrder
 */
static int WalkNodePostOrder(rbtree_node_t *X,
                 int (*callback)(rbtree_node_t *), rbtree_node_t *sentinel)
{
    int rcode;

    if (X->left != sentinel) {
        rcode = WalkNodeInOrder(X->left, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    if (X->right != sentinel) {
        rcode = WalkNodeInOrder(X->right, callback, sentinel);
        if (rcode != 0) return rcode;
    }

    rcode = callback(X);
    if (rcode != 0) return rcode;

    return 0;       /* we know everything returned zero */
}

/*
 *  Walk the entire tree.  The callback function CANNOT modify
 *  the tree.
 *
 *  The callback function should return 0 to continue walking.
 *  Any other value stops the walk, and is returned.
 */
int rbtree_walk(rbtree_t *tree, RBTREE_ORDER order,
        int (*callback)(rbtree_node_t *), rbtree_node_t *sentinel)
{
    if (tree->root == sentinel || callback == 0 ) return 0;

    switch (order) {
    case PreOrder:
        return WalkNodePreOrder(tree->root, callback, sentinel);
    case InOrder:
        return WalkNodeInOrder(tree->root, callback, sentinel);
    case PostOrder:
        return WalkNodePostOrder(tree->root, callback, sentinel);

    default:
        break;
    }

    return -1;
}

int rbtree_walk_callback(rbtree_node_t *node)
{
    printf("Key:(%d), data:(%d), color:(%d)\n",
            node->key,
            node->data,
            node->color);
    return 0;
}

int rbtree_num_elements(rbtree_t *tree)
{
    if (!tree) return 0;

    return tree->num_elements;
}
