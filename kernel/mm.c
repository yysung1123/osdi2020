#include <include/mm.h>
#include <include/mm_types.h>
#include <include/memory.h>
#include <include/pgtable-hwdef.h>
#include <include/pgtable-types.h>
#include <include/pgtable.h>
#include <include/asm/tlbflush.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/string.h>
#include <include/mmio.h>
#include <include/list.h>
#include <include/spinlock.h>
#include <include/spinlock_types.h>
#include <include/atomic.h>
#include <include/assert.h>
#include <include/error.h>
#include <include/buddy.h>

static kernaddr_t nextfree; // virtual address of next byte of free memory
static page_t *pages;
size_t npages;
struct buddy_system buddy_system;

LIST_HEAD(mango_node_free_list);
static struct mango_node *mango_nodes;
spinlock_t mango_lock;
size_t n_mango_nodes;

LIST_HEAD(vma_free_list);
static struct vm_area_struct *vmas;
spinlock_t vma_lock;
size_t nvmas;

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
    mango_nodes = (struct mango_node *)boot_alloc(sizeof(struct mango_node) * N_MANGO_NODES);
    vmas = (struct vm_area_struct *)boot_alloc(sizeof(struct vm_area_struct) * NVMAS);
    memset(pages, 0, sizeof(page_t) * NPAGES);

    buddy_init();
    mango_node_init();
    vma_init();
}

static inline void add_to_free_list(page_t *pp, struct buddy_system *buddy_system, uint8_t order) {
    list_add(&pp->buddy_list, &buddy_system->free_area[order].free_list);
    buddy_system->free_area[order].nr_free++;
}

static inline void del_page_from_free_list(page_t *pp, struct buddy_system *buddy_system, uint8_t order) {
    list_del_init(&pp->buddy_list);
    buddy_system->free_area[order].nr_free--;
}

static inline uint8_t buddy_order(uint64_t addr) {
    assert(addr >= PAGE_SIZE);
    return __builtin_ctz(addr >> PAGE_SHIFT);
}

static inline page_t* buddy_find(page_t *pp, uint8_t order) {
    return pa2page(page2pa(pp) ^ (1ull << (order + PAGE_SHIFT)));
}

void buddy_init() {
    for (uint8_t order = 0; order < MAX_ORDER; ++order) {
        INIT_LIST_HEAD(&buddy_system.free_area[order].free_list);
    }

    for (size_t idx = 0; idx < NPAGES; ++idx) {
        INIT_LIST_HEAD(&pages[idx].buddy_list);
    }

    physaddr_t next_buddy_addr = PADDR(nextfree);
    const physaddr_t peripheral_base_phys = PADDR((kernaddr_t)PERIPHERAL_BASE);
    while (next_buddy_addr < peripheral_base_phys) {
        uint8_t order = MIN(MAX_ORDER - 1, MIN(31 - __builtin_clz(peripheral_base_phys - next_buddy_addr) - PAGE_SHIFT, buddy_order(next_buddy_addr)));
        page_t *pp = pa2page(next_buddy_addr);
        pp->private = (uint64_t)order;
        add_to_free_list(pp, &buddy_system, order);
        next_buddy_addr += 1ull << (order + PAGE_SHIFT);
    }
}

page_t* buddy_alloc(uint8_t order) {
    uint64_t flags = spin_lock_irqsave(&buddy_system.lock);
    page_t *ret = NULL;

    uint8_t order_found = order;
    for (; order_found < MAX_ORDER; ++order_found) {
        if (!list_empty(&buddy_system.free_area[order_found].free_list)) break;
    }
    if (order_found == MAX_ORDER) goto unlock;

    page_t *pp = list_first_entry(&buddy_system.free_area[order_found].free_list, page_t, buddy_list);
    while (order_found > order) {
        --order_found;
        page_t *buddy = buddy_find(pp, order_found);
        del_page_from_free_list(pp, &buddy_system, order_found + 1);
        pp->private = buddy->private = (uint64_t)order_found;
        add_to_free_list(pp, &buddy_system, order_found);
        add_to_free_list(buddy, &buddy_system, order_found);
    }
    del_page_from_free_list(pp, &buddy_system, order);
    pp->private = (uint64_t)order;
    ret = pp;

unlock:
    spin_unlock_irqrestore(&buddy_system.lock, flags);
    return ret;
}

void buddy_free(page_t *pp) {
    uint64_t flags = spin_lock_irqsave(&buddy_system.lock);

    uint8_t order = (uint8_t)pp->private;
    for (; order < MAX_ORDER - 1; ++order) {
        page_t *buddy = buddy_find(pp, order);
        if (!list_empty(&buddy->buddy_list) && (uint8_t)buddy->private == order) {
            del_page_from_free_list(buddy, &buddy_system, order);
        } else {
            break;
        }
    }

    pp->private = (uint8_t)order;
    add_to_free_list(pp, &buddy_system, order);

    spin_unlock_irqrestore(&buddy_system.lock, flags);
}

page_t* page_alloc(uint32_t alloc_flags) {
    page_t *pp = buddy_alloc(0);
    if (!pp) return NULL;

    if (alloc_flags & ALLOC_ZERO) {
        memset((void *)page2kva(pp), 0, PAGE_SIZE);
    }

    return pp;
}

void page_free(page_t *pp) {
    buddy_free(pp);
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

page_t* pa2page(physaddr_t pa) {
    if (PGNUM(pa) >= NPAGES) {
        panic("pa2page called with invalid pa");
    }
    return &pages[PGNUM(pa)];
}

void pgtable_test() {
    mm_struct mm_instance;
    mm_struct *mm = &mm_instance;
    mm->pgd = pgd_alloc();
    assert(mm->pgd);

    virtaddr_t va = 0x123456789000;
    pgd_t *pgd = pgd_offset(mm, va);
    assert((pgd - mm->pgd) == 36);
    pl011_uart_printk_polling("pgd_offset() succeeded!\n");

    assert(pgd_val(*(mm->pgd)) == 0);
    pud_t *pud = pud_alloc(mm, pgd, va);
    assert((pgd_val(*pgd) & ~PTE_ADDR_MASK) == PD_TABLE);
    pl011_uart_printk_polling("pud_alloc() succeeded!\n");

    assert(pud_val(*pud) == 0);
    pmd_t *pmd = pmd_alloc(mm, pud, va);
    assert((pud_val(*pud) & ~PTE_ADDR_MASK) == PD_TABLE);
    pl011_uart_printk_polling("pmd_alloc() succeeded!\n");

    assert(pmd_val(*pmd) == 0);
    pte_t *pte = pte_alloc(mm, pmd, va);
    assert((pmd_val(*pmd) & ~PTE_ADDR_MASK) == PD_TABLE);
    pl011_uart_printk_polling("pte_alloc() succeeded!\n");

    assert(walk_to_pte(mm, va) == pte);
    pl011_uart_printk_polling("walk_to_pte() succeeded!\n");

    page_t *pp = page_alloc(0);
    assert(pp);
    assert(atomic_read(&pp->pp_ref) == 0);
    memset((void *)page2kva(pp), 0x55, PAGE_SIZE);
    pgprot_t prot = __pgprot(PTE_ATTR_NORMAL);
    assert(insert_page(mm, pp, va, prot) == 0);
    assert(atomic_read(&pp->pp_ref) == 1);

    __asm__ volatile("msr TTBR0_EL1, %0"
                     :: "r"(mm->pgd));
    flush_tlb_all();

    assert(*(uint64_t *)va == 0x5555555555555555);
    pl011_uart_printk_polling("change TTBR0_EL1 succeeded!\n");

    memset((void *)va, 0xaa, PAGE_SIZE);
    assert(*(uint64_t *)page2kva(pp) == 0xaaaaaaaaaaaaaaaa);
    pl011_uart_printk_polling("write virtual address succeeded!\n");

    virtaddr_t va2 = 0x987654321000;
    assert(insert_page(mm, pp, va2, prot) == 0);
    assert(atomic_read(&pp->pp_ref) == 2);
    flush_tlb_all();
    assert(*(uint64_t *)va2 == 0xaaaaaaaaaaaaaaaa);
    pl011_uart_printk_polling("page sharing succeeded!\n");

    unmap_page(mm, va2);
    assert(atomic_read(&pp->pp_ref) == 1);
    assert(follow_pte(mm, va2, &pte) == -E_INVAL);
    assert(follow_pte(mm, va, &pte) == 0);
    pl011_uart_printk_polling("unmap_page() succeeded!\n");

    free_pgtables(mm);
    assert(atomic_read(&pa2page(PADDR((kernaddr_t)mm->pgd))->pp_ref) == 0);
    assert(atomic_read(&pp->pp_ref) == 0);
    assert(follow_pte(mm, va, &pte) == -E_INVAL);
    pl011_uart_printk_polling("free_pgtables() succeeded!\n");
}

int32_t __pud_alloc(mm_struct *mm, pgd_t *pgd, virtaddr_t address) {
    page_t *pp = page_alloc(ALLOC_ZERO);
    pud_t *new = (pud_t *)page2pa(pp);
    if (!new) {
        return -E_NO_MEM;
    }

    pgdval_t pgdval = PGD_TYPE_TABLE;
    *pgd = __pgd((pgdval_t)new | pgdval);
    atomic_add(1, &pp->pp_ref);

    return 0;
}

int32_t __pmd_alloc(mm_struct *mm, pud_t *pud, virtaddr_t address) {
    page_t *pp = page_alloc(ALLOC_ZERO);
    pud_t *new = (pud_t *)page2pa(pp);
    if (!new) {
        return -E_NO_MEM;
    }

    pudval_t pudval = PUD_TYPE_TABLE;
    *pud = __pud((pudval_t)new | pudval);
    atomic_add(1, &pp->pp_ref);

    return 0;
}

int32_t __pte_alloc(mm_struct *mm, pmd_t *pmd, virtaddr_t address) {
    page_t *pp = page_alloc(ALLOC_ZERO);
    pmd_t *new = (pmd_t *)page2pa(pp);
    if (!new) {
        return -E_NO_MEM;
    }

    pmdval_t pmdval = PMD_TYPE_TABLE;
    *pmd = __pmd((pmdval_t)new | pmdval);
    atomic_add(1, &pp->pp_ref);

    return 0;
}

pte_t* walk_to_pte(mm_struct *mm, virtaddr_t addr) {
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    pgd = pgd_offset(mm, addr);
    pud = pud_alloc(mm, pgd, addr);
    pmd = pmd_alloc(mm, pud, addr);
    pte = pte_alloc(mm, pmd, addr);

    return pte;
}

int32_t insert_page(mm_struct *mm, page_t *pp, virtaddr_t addr, pgprot_t prot) {
    pte_t *pte = walk_to_pte(mm, addr);
    if (!pte) return -E_NO_MEM;
    if (!pte_none(*pte)) return -E_BUSY;

    *pte = __pte((pteval_t)page2pa(pp) | pgprot_val(prot) | PD_TABLE);
    atomic_add(1, &pp->pp_ref);
    return 0;
}

int32_t follow_pte(mm_struct *mm, virtaddr_t address, pte_t **ptepp) {
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep;

    pgd = pgd_offset(mm, address);
    if (pgd_none(*pgd)) {
        goto out;
    }

    pud = pud_offset(pgd, address);
    if (pud_none(*pud)) {
        goto out;
    }

    pmd = pmd_offset(pud, address);
    if (pmd_none(*pmd)) {
        goto out;
    }

    ptep = pte_offset(pmd, address);
    if (pte_none(*ptep)) {
        goto out;
    }

    *ptepp = ptep;

    return 0;
out:
    return -E_INVAL;
}

void unmap_page(mm_struct *mm, virtaddr_t addr) {
    pte_t *pte;
    int32_t ret = follow_pte(mm, addr, &pte);
    if (ret) return;

    physaddr_t pa = __pte_to_phys(*pte);
    page_t *pp = pa2page(pa);

    page_decref(pp);
    *pte = __pte(0);
}

void free_pte(pte_t *pte_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pte_t); ++idx) {
        pte_t *pte = pte_base + idx;
        if (!pte_none(*pte)) {
            page_t *pp = pa2page(__pte_to_phys(*pte));
            page_decref(pp);
            *pte = __pte(0);
        }
    }

    page_t *pp = pa2page(PADDR((kernaddr_t)pte_base));
    page_decref(pp);
}

void free_pmd(pmd_t *pmd_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pmd_t); ++idx) {
        pmd_t *pmd = pmd_base + idx;
        if (!pmd_none(*pmd)) {
            free_pte(pmd_pgtable(pmd));
            *pmd = __pmd(0);
        }
    }

    page_t *pp = pa2page(PADDR((kernaddr_t)pmd_base));
    page_decref(pp);
}

void free_pud(pud_t *pud_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pud_t); ++idx) {
        pud_t *pud = pud_base + idx;
        if (!pud_none(*pud)) {
            free_pmd(pud_pgtable(pud));
            *pud = __pud(0);
        }
    }

    page_t *pp = pa2page(PADDR((kernaddr_t)pud_base));
    page_decref(pp);
}

void free_pgd(mm_struct *mm) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pgd_t); ++idx) {
        pgd_t *pgd = mm->pgd + idx;
        if (!pgd_none(*pgd)) {
            free_pud(pgd_pgtable(pgd));
            *pgd = __pgd(0);
        }
    }

    page_t *pp = pa2page(PADDR((kernaddr_t)mm->pgd));
    page_decref(pp);
}

void free_pgtables(mm_struct *mm) {
    free_pgd(mm);
}

void copy_pte(pte_t *dst_base, pte_t *src_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pte_t); ++idx) {
        pte_t *src = src_base + idx;
        if (!pte_none(*src)) {
            page_t *pp = page_alloc(ALLOC_ZERO);
            atomic_add(1, &pp->pp_ref);
            pte_t *dst = dst_base + idx;
            *dst = __pte(page2pa(pp) | (pte_val(*src) & ~PTE_ADDR_MASK));
            memcpy((void *)page2kva(pp), (void *)KADDR(__pte_to_phys(*src)), PAGE_SIZE);
        }
    }
}

void copy_pmd(pmd_t *dst_base, pmd_t *src_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pmd_t); ++idx) {
        pmd_t *src = src_base + idx;
        if (!pmd_none(*src)) {
            page_t *pp = page_alloc(ALLOC_ZERO);
            atomic_add(1, &pp->pp_ref);
            pmd_t *dst = dst_base + idx;
            *dst = __pmd(page2pa(pp) | (pmd_val(*src) & ~PTE_ADDR_MASK));
            copy_pte(pmd_pgtable(dst), pmd_pgtable(src));
        }
    }
}

void copy_pud(pud_t *dst_base, pud_t *src_base) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pud_t); ++idx) {
        pud_t *src = src_base + idx;
        if (!pud_none(*src)) {
            page_t *pp = page_alloc(ALLOC_ZERO);
            atomic_add(1, &pp->pp_ref);
            pud_t *dst = dst_base + idx;
            *dst = __pud(page2pa(pp) | (pud_val(*src) & ~PTE_ADDR_MASK));
            copy_pmd(pud_pgtable(dst), pud_pgtable(src));
        }
    }
}

void copy_pgd(mm_struct *dst_mm, mm_struct *src_mm) {
    for (size_t idx = 0; idx < PAGE_SIZE / sizeof(pgd_t); ++idx) {
        pgd_t *src = src_mm->pgd + idx;
        if (!pgd_none(*src)) {
            page_t *pp = page_alloc(ALLOC_ZERO);
            atomic_add(1, &pp->pp_ref);
            pgd_t *dst = dst_mm->pgd + idx;
            *dst = __pgd(page2pa(pp) | (pgd_val(*src) & ~PTE_ADDR_MASK));
            copy_pud(pgd_pgtable(dst), pgd_pgtable(src));
        }
    }
}

void mango_node_init() {
    for (size_t idx = 0; idx < N_MANGO_NODES; ++idx) {
        list_add(&mango_nodes[idx].alloc_link, &mango_node_free_list);
    }

    n_mango_nodes = N_MANGO_NODES;
}

struct mango_node* mango_node_alloc() {
    struct mango_node *node = NULL;
    spin_lock(&mango_lock);
    if (list_empty(&mango_node_free_list)) goto unlock;

    node = list_first_entry(&mango_node_free_list, struct mango_node, alloc_link);
    list_del_init(&node->alloc_link);

    --n_mango_nodes;

unlock:
    spin_unlock(&mango_lock);

    return node;
}

void mango_node_free(struct mango_node *node) {
    spin_lock(&mango_lock);
    if (!list_empty(&node->alloc_link)) panic("mango_node_free error");

    list_add(&node->alloc_link, &mango_node_free_list);
    ++n_mango_nodes;
    spin_unlock(&mango_lock);
}

void vma_init() {
    for (size_t idx = 0; idx < NVMAS; ++idx) {
        list_add(&vmas[idx].alloc_link, &vma_free_list);
    }

    nvmas = NVMAS;
}

struct vm_area_struct* vma_alloc() {
    struct vm_area_struct *vma = NULL;
    spin_lock(&vma_lock);
    if (list_empty(&vma_free_list)) goto unlock;

    vma = list_first_entry(&vma_free_list, struct vm_area_struct, alloc_link);
    list_del_init(&vma->alloc_link);

    --nvmas;

unlock:
    spin_unlock(&vma_lock);

    return vma;
}

void vma_free(struct vm_area_struct *vma) {
    spin_lock(&vma_lock);
    if (!list_empty(&vma->alloc_link)) panic("vma_free error");

    list_add(&vma->alloc_link, &vma_free_list);
    ++nvmas;
    spin_unlock(&vma_lock);
}
