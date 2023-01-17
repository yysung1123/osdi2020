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
#include <include/assert.h>
#include <include/error.h>

static kernaddr_t nextfree; // virtual address of next byte of free memory
static page_t *page_free_list; // free list of physical pages
static page_t *pages;
size_t npages;

static struct mango_node *mango_nodes;
static struct mango_node *mango_node_free_list;
size_t n_mango_nodes;

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
    npages = 0;

    pl011_uart_printk_polling("%p\n", boot_alloc(0));

    pages = (page_t *)boot_alloc(sizeof(page_t) * NPAGES);
    mango_nodes = (struct mango_node *)boot_alloc(sizeof(struct mango_node) * N_MANGO_NODES);
    memset(pages, 0, sizeof(page_t) * NPAGES);
    
    page_init();
    mango_node_init();

    pgtable_test();
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
            ++npages;
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

    --npages;

    return pp;
}

void page_free(page_t *pp) {
    if (pp->pp_ref != 0 || pp->pp_link) panic("page_free error");

    pp->pp_link = page_free_list;
    page_free_list = pp;

    ++npages;
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
    assert(pp->pp_ref == 0);
    memset((void *)page2kva(pp), 0x55, PAGE_SIZE);
    pgprot_t prot = __pgprot(PTE_ATTR_NORMAL);
    assert(insert_page(mm, pp, va, prot) == 0);
    assert(pp->pp_ref == 1);

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
    assert(pp->pp_ref == 2);
    flush_tlb_all();
    assert(*(uint64_t *)va2 == 0xaaaaaaaaaaaaaaaa);
    pl011_uart_printk_polling("page sharing succeeded!\n");

    unmap_page(mm, va2);
    assert(pp->pp_ref == 1);
    assert(follow_pte(mm, va2, &pte) == -E_INVAL);
    assert(follow_pte(mm, va, &pte) == 0);
    pl011_uart_printk_polling("unmap_page() succeeded!\n");

    free_pgtables(mm);
    assert(pa2page(PADDR((kernaddr_t)mm->pgd))->pp_ref == 0);
    assert(pp->pp_ref == 0);
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
    pp->pp_ref++;

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
    pp->pp_ref++;

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
    pp->pp_ref++;

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
    pp->pp_ref++;
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
            pp->pp_ref++;
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
            pp->pp_ref++;
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
            pp->pp_ref++;
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
            pp->pp_ref++;
            pgd_t *dst = dst_mm->pgd + idx;
            *dst = __pgd(page2pa(pp) | (pgd_val(*src) & ~PTE_ADDR_MASK));
            copy_pud(pgd_pgtable(dst), pgd_pgtable(src));
        }
    }
}

void copy_mm(mm_struct *dst, mm_struct *src) {
    copy_pgd(dst, src);
}

void mango_node_init() {
    for (size_t idx = 0; idx < N_MANGO_NODES; ++idx) {
        mango_nodes[idx].alloc_link = mango_node_free_list;
        mango_node_free_list = &mango_nodes[idx];
    }

    n_mango_nodes = N_MANGO_NODES;
}

struct mango_node* mango_node_alloc() {
    struct mango_node *node = mango_node_free_list;
    if (node == NULL) return NULL;

    mango_node_free_list = node->alloc_link;
    node->alloc_link = NULL;

    --n_mango_nodes;

    return node;
}

void mango_node_free(struct mango_node *node) {
    if (node->alloc_link) panic("mango_node_free error");

    node->alloc_link = mango_node_free_list;
    mango_node_free_list = node;

    ++n_mango_nodes;
}
