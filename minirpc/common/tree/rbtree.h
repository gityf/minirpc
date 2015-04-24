
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _RBTREE_H_INCLUDED_
#define _RBTREE_H_INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */
    typedef unsigned int  rbtree_key_t;
    typedef int   rbtree_key_int_t;


    typedef struct rbtree_node_s  rbtree_node_t;

    struct rbtree_node_s {
        rbtree_key_t       key;
        rbtree_node_t     *left;
        rbtree_node_t     *right;
        rbtree_node_t     *parent;
        unsigned char     color;
        unsigned char     data;
    };


    typedef struct rbtree_s  rbtree_t;

    typedef void (*rbtree_insert_pt) (rbtree_node_t *root,
        rbtree_node_t *node, rbtree_node_t *sentinel);

    struct rbtree_s {
        rbtree_node_t     *root;
        rbtree_node_t     *sentinel;
        rbtree_insert_pt   insert;
        int num_elements;
    };


    #define rbtree_init(tree, s, i)                                           \
        rbtree_sentinel_init(s);                                              \
        (tree)->root = s;                                                     \
        (tree)->sentinel = s;                                                 \
        (tree)->insert = i;                                                   \
        (tree)->num_elements = 0


    void rbtree_insert(rbtree_t *tree,
        rbtree_node_t *node);
    void rbtree_delete(rbtree_t *tree,
        rbtree_node_t *node);
    void rbtree_insert_value(rbtree_node_t *root, rbtree_node_t *node,
        rbtree_node_t *sentinel);
    void rbtree_insert_timer_value(rbtree_node_t *root,
        rbtree_node_t *node, rbtree_node_t *sentinel);


    #define rbt_red(node)               ((node)->color = 1)
    #define rbt_black(node)             ((node)->color = 0)
    #define rbt_is_red(node)            ((node)->color)
    #define rbt_is_black(node)          (!rbt_is_red(node))
    #define rbt_copy_color(n1, n2)      (n1->color = n2->color)

    /* a sentinel must be black */

    #define rbtree_sentinel_init(node)  rbt_black(node)


    static inline rbtree_node_t *
    rbtree_min(rbtree_node_t *node, rbtree_node_t *sentinel)
    {
        while (node->left != sentinel) {
            node = node->left;
        }

        return node;
    }
    static inline void rbtree_left_rotate(rbtree_node_t **root,
        rbtree_node_t *sentinel, rbtree_node_t *node);
    static inline void rbtree_right_rotate(rbtree_node_t **root,
        rbtree_node_t *sentinel, rbtree_node_t *node);

    /* callback order for walking  */
    typedef enum { PreOrder, InOrder, PostOrder } RBTREE_ORDER;

    /*
     *	The callback should be declared as:
     *	int callback(rbtree_node_t *node)
     *
     *
     *	It should return 0 if all is OK, and !0 for any error.
     *	The walking will stop on any error.
     */
    int rbtree_walk(rbtree_t *tree,
                    RBTREE_ORDER order,
                    int (*callback)(rbtree_node_t *),
                    rbtree_node_t *sentinel);
    /*
     *	The callback should be declared as:
     *	int ccallback(rbtree_node_t *node)
     *
     *
     *	It should return 0 if all is OK, and !0 for any error.
     *	The walking will stop on any error.
     */
    int rbtree_walk_callback(rbtree_node_t *node);

    int rbtree_num_elements(rbtree_t *tree);
#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* _RBTREE_H_INCLUDED_ */
