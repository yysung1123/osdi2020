#include <include/mmap.h>
#include <include/types.h>
#include <include/task.h>
#include <include/mango_tree.h>
#include <include/pgtable-types.h>
#include <include/pgtable-hwdef.h>
#include <include/string.h>
#include <include/asm/tlbflush.h>
#include <include/memory.h>
#include <include/mm.h>
#include <include/kmalloc.h>

void* do_mmap(void *addr, size_t len, mmap_prot_t prot, mmap_flags_t flags, void *file_start, off_t file_offset) {
    task_t *cur = get_current();

    if (flags & MAP_FIXED) {
        if (addr == NULL || (virtaddr_t)addr & ((1ull << PAGE_SHIFT) - 1)) {
            return (void *)-1;
        }
    }

    if (addr == NULL) {
        uint64_t start;
        if (mtree_empty_area(&cur->mm.mt, cur->mm.mt.min, cur->mm.mt.max, PAGE_ALIGN(len), &start) != 0) {
            return (void *)-1;
        }

        addr = (void *)start;
    }

    pgprot_t attr = __pgprot(PTE_ATTR_NORMAL);
    if (prot & PROT_READ) {
        if (prot & PROT_WRITE) {
            pgprot_val(attr) |= PD_RW;
        } else {
            pgprot_val(attr) |= PD_RO;
        }
    }

    if (!(prot & PROT_EXEC)) {
        pgprot_val(attr) |= PD_NX;
    }

    virtaddr_t first = (virtaddr_t)addr;
    virtaddr_t last = (virtaddr_t)addr + len - 1;

    struct vm_area_struct *vma = kmalloc(sizeof(struct vm_area_struct));
    if (vma == NULL) {
        return (void *)-1;
    }

    vma->vm_start = first;
    vma->vm_end = last;
    vma->vm_mm = &cur->mm;
    vma->vm_page_prot = attr;

    if (mtree_insert_range(&cur->mm.mt, (uint64_t)addr, (uint64_t)(PAGE_ALIGN(last) - 1), vma) != 0) {
        return (void *)-1;
    }

    // region page mapping
    if (flags & MAP_POPULATE) {
        for (off_t off = 0; off < len; off += PAGE_SIZE) {
            page_t *pp = page_alloc(0);
            assert(pp);

            virtaddr_t va = first + off;
            insert_page(&cur->mm, pp, va, attr);

            memcpy((void *)page2kva(pp), file_start + file_offset + off, MIN(PAGE_SIZE, len - off));
        }

        tlbi_vmalle1is();
    }

    return addr;
}
