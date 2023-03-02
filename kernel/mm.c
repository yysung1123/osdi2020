#include <include/mm.h>
#include <include/memory.h>
#include <include/pgtable-hwdef.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/mmio.h>
#include <include/list.h>
#include <include/spinlock.h>
#include <include/spinlock_types.h>
#include <include/atomic.h>

static kernaddr_t nextfree; // virtual address of next byte of free memory
LIST_HEAD(page_free_list); // free list of physical pages
static page_t *pages;
spinlock_t page_lock;

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
            atomic_set(1, &pages[i].pp_ref);
        } else {
            atomic_set(0, &pages[i].pp_ref);
            list_add(&pages[i].pp_link, &page_free_list);
        }
    }
}

page_t* page_alloc(uint32_t alloc_flags) {
    page_t *pp = NULL;
    uint64_t flags = spin_lock_irqsave(&page_lock);
    if (list_empty(&page_free_list)) goto unlock;

    pp = list_first_entry(&page_free_list, page_t, pp_link);
    list_del_init(&pp->pp_link);

    if (alloc_flags & ALLOC_ZERO) {
        memset((void *)page2kva(pp), 0, PAGE_SIZE);
    }

unlock:
    spin_unlock_irqrestore(&page_lock, flags);
    return pp;
}

void page_free(page_t *pp) {
    uint64_t flags = spin_lock_irqsave(&page_lock);
    if (atomic_read(&pp->pp_ref) != 0 || !list_empty(&pp->pp_link)) panic("page_free error");

    list_add(&pp->pp_link, &page_free_list);
    spin_unlock_irqrestore(&page_lock, flags);
}

void page_decref(page_t *pp) {
    atomic_add(-1, &pp->pp_ref);
    if (atomic_read(&pp->pp_ref) == 0) {
        page_free(pp);
    }
}

physaddr_t page2pa(page_t *pp) {
	return (physaddr_t)((uintptr_t)(pp - pages) << PAGE_SHIFT);
}
