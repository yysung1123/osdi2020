#pragma once

#include <include/types.h>

/* a range based treap implementation */

#define N_MANGO_NODES 4096

struct mango_tree {
    struct mango_node *ma_root;
    int32_t state;
    uint64_t min, max;
    int32_t (*mango_tree_rand)(struct mango_tree *);
};

struct mango_node {
    struct mango_node *parent;
    struct mango_node *left, *right;
    struct list_head alloc_link;
    uint64_t pri;
    uint64_t index, last;
    uint64_t gap;
    uint64_t gap_max;
    void *entry;
};

#define mt_for_each(__tree, __node) \
    for ((__node) = mango_leftmost((__tree)->ma_root); \
            (__node); (__node) = mango_next((__node)))

struct mango_node* mango_leftmost(struct mango_node *);
struct mango_node* mango_next(struct mango_node *);
struct mango_tree mtree_init(int32_t, uint64_t, uint64_t);
int32_t mtree_insert_range(struct mango_tree *, uint64_t, uint64_t, void *);
int32_t mtree_empty_area(struct mango_tree *, uint64_t, uint64_t, uint64_t, uint64_t *);
void mtree_destroy(struct mango_tree *);
struct mango_node* mt_find(struct mango_tree *, uint64_t);
