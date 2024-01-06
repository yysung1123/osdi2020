#include <include/kmalloc.h>
#include <include/slab.h>
#include <include/types.h>
#include <include/assert.h>
#include <include/mm_types.h>
#include <include/mm.h>

#define NUM_SLAB_KMALLOC 9

const size_t slab_kmalloc_size[NUM_SLAB_KMALLOC] = {
    8, 16, 32, 64, 128, 256, 512, 1024, 2048
};

struct slab slab_kmalloc[NUM_SLAB_KMALLOC];

void kmalloc_init() {
    for (size_t i = 0; i < NUM_SLAB_KMALLOC; ++i) {
        slab_init(&slab_kmalloc[i], slab_kmalloc_size[i]);
    }
}

void* kmalloc(size_t size) {
    if (size > slab_kmalloc_size[NUM_SLAB_KMALLOC - 1]) {
        uint8_t order = 0;
        if ((size & (size - 1)) == 0) {
            order = 31 - __builtin_clz(size) - 12;
        }  else {
            order = 32 - __builtin_clz(size) - 12;
        }
        page_t *pp = buddy_alloc(order);
        return (void *)page2kva(pp);
    } else {
        for (size_t i = 0; i < NUM_SLAB_KMALLOC; ++i) {
            if (size <= slab_kmalloc_size[i]) {
                return slab_alloc(&slab_kmalloc[i]);
            }
        }
    }

    panic("should not execute here");
    return NULL;
}

void kfree(void *addr) {
    page_t *pp = pa2page(PADDR((kernaddr_t)addr));

    if (pp->slab_cache) {
        slab_free(pp->slab_cache, addr);
    } else {
        buddy_free(pp);
    }
}

void kmalloc_test() {
    const size_t test_cases[5] = {1 << 28, 1 << 12, 1 << 11, 2047, 2};
    void *ptr;
    for (int i = 0; i < 5; ++i) {
        ptr = kmalloc(test_cases[i]);
        for (size_t j = 0; j < test_cases[i]; ++j) {
            *((uint8_t *)(ptr) + j) = 0xff;
        }
        kfree(ptr);
    }

    const size_t arr1_len = 4096;
    const size_t arr1_elem_sz = 1;
    const size_t arr1_sz = arr1_elem_sz * arr1_len;
    uint8_t **arr1 = (uint8_t **)kmalloc(arr1_sz);
    for (size_t i = 0; i < arr1_len; ++i) {
        arr1[i] = (uint8_t *)kmalloc(arr1_elem_sz);
        *arr1[i] = 0xff;
    }
    for (size_t i = 0; i < arr1_len; ++i) {
        kfree(arr1[i]);
    }
    kfree(arr1);
}
