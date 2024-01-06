#pragma once

#include <include/types.h>
#include <include/list.h>
#include <include/spinlock_types.h>
#include <include/mm_types.h>

#define SLAB_PAGE_OFFSET 32

struct slab_page {
    struct list_head list; // 16 bytes
    page_t *page; // 8 bytes
    size_t nr_free; // 8 bytes
    char memory[4064];
    // memory: 4064 / (N + 16)
}; // 4096 bytes

struct slab {
    struct list_head page_list;
    size_t object_size;
    struct list_head free_list;
    spinlock_t lock;
};

#define __SLAB_INITIALIZER(slabname, size) \
    { .page_list = LIST_HEAD_INIT(slabname.page_list), \
      .object_size = size, \
      .free_list = LIST_HEAD_INIT(slabname.free_list) }

#define DEFINE_SLAB(slabname, size) \
    struct slab slabname = __SLAB_INITIALIZER(slabname, size)

void *slab_alloc(struct slab *);
void slab_free(struct slab *, void *);
void slab_init(struct slab *, size_t size);
