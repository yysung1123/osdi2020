#include <include/printk.h>
#include <include/uart.h>
#include <include/stdio.h>
#include <include/stdarg.h>

void pl011_uart_putch(int32_t ch, int32_t *cnt) {
    pl011_uart_write(ch);
    (*cnt)++;
}

int32_t pl011_uart_vprintk(const char *fmt, va_list ap) {
    int32_t cnt = 0;

    vprintfmt((void *)pl011_uart_putch, &cnt, fmt, ap);
    return cnt;
}

int32_t pl011_uart_printk(const char *fmt, ...) {
    va_list ap;
    int32_t cnt;

    va_start(ap, fmt);
    cnt = pl011_uart_vprintk(fmt, ap);
    va_end(ap);

    return cnt;
}
