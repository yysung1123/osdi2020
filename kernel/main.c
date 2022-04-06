#include <include/uart.h>
#include <include/shell.h>

int main() {
    uart_init();
    do_shell();
}
