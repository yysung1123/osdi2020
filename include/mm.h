#pragma once

#include <include/memory.h>
#include <include/types.h>

typedef struct page_t page_t;
struct page_t {
    page_t *pp_link;
    uint32_t pp_ref;
};

enum {
    ALLOC_ZERO = 1 << 0,
};

void mem_init();
kernaddr_t boot_alloc(uint32_t);
void page_init();
page_t* page_alloc(uint32_t);
void page_free(page_t *);
void page_decref(page_t *);
physaddr_t page2pa(page_t *);

static inline kernaddr_t page2kva(page_t *pp) {
	return KADDR(page2pa(pp));
}
