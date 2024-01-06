#include <include/mango_tree.h>
#include <include/types.h>
#include <include/error.h>
#include <include/mm.h>
#include <include/printk.h>
#include <include/limits.h>
#include <include/pgtable-hwdef.h>
#include <include/kmalloc.h>

static inline void mango_pull_gap(struct mango_node *node) {
    uint64_t max = node->gap;

    if (node->left) {
        max = MAX(max, node->left->gap_max);
    }
    if (node->right) {
        max = MAX(max, node->right->gap_max);
    }
    node->gap_max = max;
}

void mango_split(struct mango_node *node, uint64_t key, struct mango_node **l, struct mango_node **r) {
    if (!node) {
        *l = *r = NULL;
        return;
    }

    if (key <= node->index) {
        *r = node;
        node->parent = NULL;
        mango_split(node->left, key, l, &(node->left));
        if (node->left) {
            node->left->parent = node;
        }
    } else {
        *l = node;
        node->parent = NULL;
        mango_split(node->right, key, &(node->right), r);
        if (node->right) {
            node->right->parent = node;
        }
    }

    mango_pull_gap(node);
}

struct mango_node* mango_merge(struct mango_node *l, struct mango_node *r) {
    if (l == NULL || r == NULL) {
        return l ? l : r;
    }

    if (l->pri > r->pri) {
        l->right = mango_merge(l->right, r);
        l->right->parent = l;
        mango_pull_gap(l);
        return l;
    } else {
        r->left = mango_merge(l, r->left);
        r->left->parent = r;
        mango_pull_gap(r);
        return r;
    }
}

static inline struct mango_node* mango_rightmost(struct mango_node *node) {
    if (node == NULL) return NULL;

    while (node->right) {
        node = node->right;
    }

    return node;
}

struct mango_node* mango_leftmost(struct mango_node *node) {
    if (node == NULL) return NULL;

    while (node->left) {
        node = node->left;
    }

    return node;
}


static inline void mango_update_gap(struct mango_node *node) {
    while (node) {
        mango_pull_gap(node);

        node = node->parent;
    }
}

void mango_node_print(struct mango_node *node) {
    pl011_uart_printk_polling("index: %lu, last: %lu pri: %lu, gap: %lu, gap_max: %lu\n",
        node->index, node->last, node->pri, node->gap, node->gap_max);
    pl011_uart_printk_polling("left: %p, right: %p\n", node->left, node->right);
}

void mango_tree_print(struct mango_node *node) {
    if (node->left) mango_tree_print(node->left);
    mango_node_print(node);
    if (node->right) mango_tree_print(node->right);
}

void mango_node_destroy(struct mango_node *node) {
    if (node == NULL) return;

    mango_node_destroy(node->left);
    mango_node_destroy(node->right);

    kfree(node);
}

struct mango_node* mango_next(struct mango_node *node) {
    if (node == NULL) return NULL;

    if (node->right) {
        return mango_leftmost(node->right);
    }

    while (node->parent != NULL && node->parent->right == node) {
        node = node->parent;
    }

    return node->parent;
}

int32_t mango_tree_rand(struct mango_tree *mt) {
    mt->state = (int32_t)(((mt->state * 1103515245ull) + 12345ull) & 0x7fffffff);
    return mt->state;
}

struct mango_node* mt_alloc_one(struct mango_tree *mt, uint64_t first, uint64_t last, void *entry) {
    struct mango_node *node = kmalloc(sizeof(struct mango_node));

    node->parent = node->left = node->right = NULL;
    node->gap = 0;
    node->gap_max = 0;
    node->index = first;
    node->last = last;
    node->entry = entry;
    node->pri = mt->mango_tree_rand(mt);

    return node;
}

struct mango_node* mt_find(struct mango_tree *mt, uint64_t index) {
    struct mango_node *node = mt->ma_root;

    if (!node) return NULL;

descend:
    if (node->index <= index && index <= node->last) {
        return node;
    } else if (node->left && node->index > index) {
        node = node->left;
        goto descend;
    } else if (node->right && node->last < index) {
        node = node->right;
        goto descend;
    }

    return NULL;
}

struct mango_tree mtree_init(int32_t seed, uint64_t min_val, uint64_t max_val) {
    struct mango_tree mt = {
        .state = seed,
        .min = min_val,
        .max = max_val,
        .mango_tree_rand = mango_tree_rand,
    };

    return mt;
}

int32_t mtree_check_range(struct mango_tree *mt, uint64_t first, uint64_t last) {
    if (first <= mt->min || last >= mt->max) return -E_INVAL;

    return 0;
}

int32_t mtree_insert_range(struct mango_tree *mt, uint64_t first, uint64_t last, void *entry) {
    if (mtree_check_range(mt, first, last)) return -E_INVAL;

    int32_t ret = 0;

    struct mango_node *l = NULL, *mid = NULL, *r = NULL;
    mango_split(mt->ma_root, first, &l, &r);
    mango_split(r, last + 1, &mid, &r);

    // check mid
    if (mid) {
        ret = -E_INVAL;
        goto merge;
    }

    // check range overlapping if the node to be inserted is not leftmost
    struct mango_node *prev = NULL;
    if (l) {
        prev = mango_rightmost(l);

        if (prev->last >= first) {
            ret = -E_INVAL;
            goto merge;
        }
    }

    // initialize mid
    mid = mt_alloc_one(mt, first, last, entry);
    if (mid == NULL) {
        ret = -E_NO_MEM;
        goto merge;
    }

    // update prev and mid gap
    if (prev) {
        uint64_t old_gap = prev->gap;
        prev->gap = (first - 1) - prev->last;
        mango_update_gap(prev);
        mid->gap = mid->gap_max = old_gap - (last - prev->last);
    } else {
        struct mango_node *next = mango_leftmost(r);
        if (next) {
            mid->gap = next->index - last - 1;
        } else {
            mid->gap = mt->max - last;
        }
    }

merge:
    // insert mid
    r = mango_merge(mid, r);

    mt->ma_root = mango_merge(l, r);

    return ret;
}

int32_t mtree_empty_area(struct mango_tree *mt, uint64_t min, uint64_t max, uint64_t size, uint64_t *start) {
    if (min > max) return -E_INVAL;
    if (min > mt->max - size + 1) return -E_INVAL;
    max = MIN(max, mt->max - size + 1);

    int32_t ret = 0;

    // l: [0, min), mid: [min, max), r: [max, inf)
    struct mango_node *l, *mid, *r;
    mango_split(mt->ma_root, min, &l, &r);
    mango_split(r, max, &mid, &r);

    struct mango_node *l_rm = mango_rightmost(l);
    // start = min
    if (l_rm && l_rm->last < min && l_rm->last + l_rm->gap >= min + size - 1) {
        *start = min;
        goto merge;
    }

    // start = min
    if (mid == NULL) {
        *start = min;
        goto merge;
    }

    // start in mid
    if (mid->gap_max >= size) {
        struct mango_node *node = mid;

        // since node->gap_max >= size, there's always an empty area >= size
        while (true) {
            if (node->left && node->left->gap_max >= size) {
                node = node->left;
            } else if (node->gap >= size) {
                break;
            } else if (node->right && node->right->gap_max >= size) {
                node = node->right;
            }
        }

        assert(node->gap >= size);
        *start = node->last + 1;
        goto merge;
    }

    ret = -E_INVAL;
merge:
    r = mango_merge(mid, r);
    mt->ma_root = mango_merge(l, r);

    return ret;
}

void mtree_destroy(struct mango_tree *mt) {
    mango_node_destroy(mt->ma_root);
}

void mango_test() {
    struct mango_tree mt_instance = mtree_init(0, 0, UULONG_MAX);
    struct mango_tree *mt = &mt_instance;
    struct mango_node *node;

    // test empty mtree mt_find
    assert(mt_find(mt, 0) == NULL);

    // test mtree_empty_area before inserting nodes
    for (uint64_t i = 0; i < 20; ++i) {
        uint64_t start;
        uint64_t first = i * PAGE_SIZE;
        assert(mtree_empty_area(mt, first, UULONG_MAX, PAGE_SIZE, &start) == 0);
        assert(start == first);
    }

    for (uint64_t i = 1; i < 20; i += 5) {
        uint64_t first = i * PAGE_SIZE;
        assert(mtree_insert_range(mt, first, first + PAGE_SIZE - 1, NULL) == 0);
    }

    for (uint64_t i = 0; i < 20; ++i) {
        uint64_t first = i * PAGE_SIZE;
        if ((node = mt_find(mt, first))) {
            assert(i % 5 == 1);
        } else {
            assert(i % 5 != 1);
        }

        // 1 byte offset
        if ((node = mt_find(mt, first + 1))) {
            assert(i % 5 == 1);
        } else {
            assert(i % 5 != 1);
        }
    }

    for (uint64_t i = 1; i < 20; i += 5) {
        uint64_t first = i * PAGE_SIZE;
        assert(mtree_insert_range(mt, first, first + 2 * PAGE_SIZE, NULL) != 0);
    }

    uint64_t start;
    for (size_t i = 0; i < 10; ++i) {
        mtree_empty_area(mt, PAGE_SIZE, 100 * PAGE_SIZE, 3 * PAGE_SIZE, &start);
        assert(mtree_insert_range(mt, start, start + 3 * PAGE_SIZE - 1, NULL) == 0);
    }

    mtree_destroy(mt);
}
