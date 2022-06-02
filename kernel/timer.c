#include <include/timer.h>
#include <include/types.h>
#include <include/printk.h>

void local_timer_init(){
    uint32_t flag = 0x30000000; // enable timer and interrupt.
    uint32_t reload = 25000000;
    mmio_write(LOCAL_TIMER_CONTROL_REG, flag | reload);
}

void local_timer_handler(){
    static uint32_t local_timer_jiffies = 0;

    mmio_write(LOCAL_TIMER_IRQ_CLR, 0xc0000000); // clear interrupt and reload.

    pl011_uart_printk_polling("Local timer interrupt, jiffies %d\n", ++local_timer_jiffies);
}

void core_timer_init() {
    // enable timer
    __asm__ volatile("msr cntp_ctl_el0, %0"
                     :: "r"(1));
    // set expired time
    __asm__ volatile("msr cntp_tval_el0, %0"
                     :: "r"(EXPIRE_PERIOD));
    // enable timer interrupt
    mmio_write(CORE0_TIMER_IRQ_CTRL, 2);
}

void core_timer_handler() {
    static uint32_t core_timer_jiffies = 0;

    __asm__ volatile("msr cntp_tval_el0, %0"
                     :: "r"(EXPIRE_PERIOD));

    pl011_uart_printk_polling("Core timer interrupt, jiffies %d\n", ++core_timer_jiffies);
}

int64_t do_init_timers() {
    local_timer_init();
    core_timer_init();

    return 0;
}
