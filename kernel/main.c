#include <include/uart.h>
#include <include/shell.h>

int main() {
    mini_uart_init();
    do_shell();
}
