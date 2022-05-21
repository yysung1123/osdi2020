#include <include/printk.h>
#include <include/types.h>

void print_exception_info() {
    uint64_t ELR_EL1;
    uint32_t ESR_EL1;
    __asm__ volatile("mrs %0, ELR_EL1"
                     : "=r"(ELR_EL1));
    __asm__ volatile("mrs %0, ESR_EL1"
                     : "=r"(ESR_EL1));
    uint32_t EC = ESR_EL1 >> 26;
    uint32_t ISS = ESR_EL1 & 0x01ffffff;

    pl011_uart_printk("Exception return address: 0x%x\n", ELR_EL1);
    pl011_uart_printk("Exception class (EC): 0x%x\n", EC);
    pl011_uart_printk("Instruction specific syndrome (ISS): 0x%x\n", ISS);
}
