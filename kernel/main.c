#include <include/uart.h>
#include <include/shell.h>

int main() {
    pl011_uart_init();
    do_shell();
}
