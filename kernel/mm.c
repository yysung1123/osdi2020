#include <include/mm.h>
#include <include/memory.h>
#include <include/pgtable-hwdef.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/mmio.h>

static kernaddr_t nextfree; // virtual address of next byte of free memory
static page_t *page_free_list; // free list of physical pages
static page_t *pages;

kernaddr_t boot_alloc(uint32_t n) {
    kernaddr_t result;

    if (!nextfree) {
        extern char _KERNEL_END[];
        nextfree = ROUNDUP((kernaddr_t)_KERNEL_END, PAGE_SIZE);
    }

    if (n == 0) {
        return nextfree;
    } else {
        result = nextfree;
        nextfree += ROUNDUP(n, PAGE_SIZE);
    }

    return result;
}

void mem_init() {
    page_free_list = NULL;

    pl011_uart_printk_polling("%p\n", boot_alloc(0));

    pages = (page_t *)boot_alloc(sizeof(page_t) * NPAGES);
    memset(pages, 0, sizeof(page_t) * NPAGES);
    
    page_init();
}

void page_init() {
    extern char _KERNEL_START[];
    for (size_t i = 0; i < NPAGES; ++i) {
        if ((PADDR((kernaddr_t)_KERNEL_START) / PAGE_SIZE <= i && i < PADDR(nextfree) / PAGE_SIZE) ||
            (PADDR((kernaddr_t)PERIPHERAL_BASE) / PAGE_SIZE <= i && i <= PADDR((kernaddr_t)PERIPHERAL_END) / PAGE_SIZE)) {
            pages[i].pp_ref = 1;
        } else {
            pages[i].pp_ref = 0;
            pages[i].pp_link = page_free_list;
            page_free_list = &pages[i];
        }
    }
}

page_t* page_alloc(uint32_t alloc_flags) {
    page_t *pp = page_free_list;
    if (pp == NULL) return NULL;

    page_free_list = pp->pp_link;
    pp->pp_link = NULL;

    if (alloc_flags & ALLOC_ZERO) {
        memset((void *)page2kva(pp), 0, PAGE_SIZE);
    }

    return pp;
}

void page_free(page_t *pp) {
    if (pp->pp_ref != 0 || pp->pp_link) panic("page_free error");

    pp->pp_link = page_free_list;
    page_free_list = pp;
}

void page_decref(page_t *pp) {
    --pp->pp_ref;
    if (pp->pp_ref == 0) {
        page_free(pp);
    }
}

physaddr_t page2pa(page_t *pp) {
	return (physaddr_t)((uintptr_t)(pp - pages) << PAGE_SHIFT);
}
