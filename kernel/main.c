#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();

    irq_enable();

    // EL1 to EL0
    __asm__ volatile(
        "mov x0, xzr\n\t"
        "msr SPSR_EL1, x0\n\t" // EL0t
        "ldr x0, = shell_main\n\t"
        "msr ELR_EL1, x0\n\t"
        "ldr x0, = _el0_stack_top\n\t" // Set the stack pointer
        "msr SP_EL0, x0\n\t"
        "eret\n\t"
    );
}
