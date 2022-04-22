#include <include/uart.h>
#include <include/shell.h>
#include <include/fb.h>

int main() {
    pl011_uart_init();
    fb_init();
    fb_show_splash_image();
    do_shell();
}
