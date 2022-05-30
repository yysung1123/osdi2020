#include <include/irq.h>
#include <include/timer.h>
#include <include/types.h>
#include <include/mmio.h>
#include <include/printk.h>

void irq_handler() {
    uint32_t stat;

    do {
        stat = mmio_read(CORE0_INTERRUPT_SOURCE);
        uint32_t irq = __builtin_ffs(stat) - 1;

        switch (irq) {
            case 1:
                core_timer_handler();
                break;
            case 11:
                local_timer_handler();
                break;
            default:
        }
    } while (stat);
}
