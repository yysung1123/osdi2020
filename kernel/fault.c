#include <include/fault.h>
#include <include/exc.h>
#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/mm.h>
#include <include/mango_tree.h>
#include <include/mm_types.h>
#include <include/pgtable-types.h>
#include <include/pgtable-hwdef.h>
#include <include/string.h>
#include <include/asm/tlbflush.h>

void page_fault_handler(struct TrapFrame *tf) {
    uint64_t FAR_EL1;
    __asm__ volatile("mrs %0, FAR_EL1"
                     : "=r"(FAR_EL1));

    task_t *cur = get_current();

    struct mango_node *node = mt_find(&cur->mm.mt, FAR_EL1);
    if (node == NULL) {
        pl011_uart_printk_polling("segmentation fault\n");

        do_exit();
        return;
    }

    struct vm_area_struct *vma = (struct vm_area_struct *)(node->entry);
    pgprot_t prot = vma->vm_page_prot;
    bool WnR = tf->esr_el1 & (1ull << 6);
    if ((pgprot_val(prot) & ~PD_USER & PD_RO) && WnR) {
        pl011_uart_printk_polling("segmentation fault\n");

        do_exit();
        return;
    }

    virtaddr_t va = PAGE_ALIGN_DOWN(FAR_EL1);
    pte_t *ptep;
    if (follow_pte(&cur->mm, va, &ptep) == 0) {
        // TODO: copy on write

        return;
    }

    page_t *pp = page_alloc(0);
    assert(pp);

    insert_page(&cur->mm, pp, va, prot);

    memset((void *)page2kva(pp), 0, PAGE_SIZE);

    tlbi_vmalle1is();
}
