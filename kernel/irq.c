#include <include/irq.h>
#include <include/timer.h>
#include <include/types.h>
#include <include/mmio.h>
#include <include/printk.h>
#include <include/uart.h>

void irq_handler() {
    uint32_t stat;

    do {
        stat = mmio_read(CORE0_INTERRUPT_SOURCE);
        uint32_t irq = __builtin_ffs(stat) - 1;

        switch (irq) {
            case 1:
                core_timer_handler();
                break;
            case 8:
                peripheral_handler();
                break;
            case 11:
                local_timer_handler();
                break;
            default:
        }
    } while (stat);
}

void peripheral_handler() {
    uint32_t stat;

    do {
        stat = mmio_read(IRQ_PENDING_1);
        uint32_t irq = __builtin_ffs(stat) - 1;

        switch(irq) {
            default:
        }
    } while (stat);

    do {
        stat = mmio_read(IRQ_PENDING_2);
        uint32_t irq = __builtin_ffs(stat) - 1;

        switch(irq) {
            case 25:
                pl011_uart_intr();
                break;
            default:
        }
    } while (stat);
}

void irq_enable() {
    __asm__ volatile("msr daifclr, #2");
}

void irq_disable() {
    __asm__ volatile("msr daifset, #2");
}
