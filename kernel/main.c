#include <include/uart.h>
#include <include/shell.h>
#include <include/fb.h>
#include <include/irq.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();

    irq_enable();

    do_shell();
}
