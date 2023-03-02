#include <include/exc.h>
#include <include/printk.h>
#include <include/types.h>
#include <include/syscall.h>
#include <include/irq.h>
#include <include/fault.h>

void print_exception_info(struct TrapFrame *tf) {
    uint64_t ELR_EL1 = tf->elr_el1;
    uint32_t ESR_EL1 = tf->esr_el1;
    uint32_t EC = ESR_EL1 >> 26;
    uint32_t ISS = ESR_EL1 & 0x01ffffff;

    pl011_uart_printk_polling("Exception return address: 0x%x\n", ELR_EL1);
    pl011_uart_printk_polling("Exception class (EC): 0x%x\n", EC);
    pl011_uart_printk_polling("Instruction specific syndrome (ISS): 0x%x\n", ISS);
}

void sync_handler(struct TrapFrame *tf) {
    uint32_t ESR_EL1 = tf->esr_el1;
    uint32_t EC = ESR_EL1 >> 26;

    switch (EC) {
        case 0b010101:
            irq_enable();
            svc_handler(tf);
            break;
        case 0b100000:
        case 0b100100:
        case 0b100101:
            page_fault_handler(tf);
            break;
        default:
            print_exception_info(tf);
    }
}

void svc_handler(struct TrapFrame *tf) {
    uint32_t ESR_EL1 = tf->esr_el1;
    uint32_t ISS = ESR_EL1 & 0xffff;

    switch (ISS) {
        case 0:
            syscall_handler(tf);
            break;
        default:
            print_exception_info(tf);
    }
}
