#include <include/fault.h>
#include <include/exc.h>
#include <include/task.h>
#include <include/types.h>
#include <include/printk.h>

void page_fault_handler(struct TrapFrame *tf) {
    uint64_t FAR_EL1;
    __asm__ volatile("mrs %0, FAR_EL1"
                     : "=r"(FAR_EL1));
    pl011_uart_printk_polling("Page fault: 0x%016llx\n", FAR_EL1);

    do_exit();
}
