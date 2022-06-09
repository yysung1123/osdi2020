#include <include/printk.h>
#include <include/uart.h>
#include <include/stdio.h>
#include <include/stdarg.h>
#include <include/types.h>
#include <include/utils.h>

void pl011_uart_putch(int32_t ch, int32_t *cnt) {
    pl011_uart_putchar(ch);
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

void pl011_uart_putch_polling(int32_t ch, int32_t *cnt) {
    pl011_uart_write_polling(ch);
    (*cnt)++;
}

int32_t pl011_uart_vprintk_polling(const char *fmt, va_list ap) {
    int32_t cnt = 0;

    vprintfmt((void *)pl011_uart_putch_polling, &cnt, fmt, ap);
    return cnt;
}

int32_t pl011_uart_printk_polling(const char *fmt, ...) {
    va_list ap;
    int32_t cnt;

    va_start(ap, fmt);
    cnt = pl011_uart_vprintk_polling(fmt, ap);
    va_end(ap);

    return cnt;
}

int32_t pl011_uart_printk_time(const char *fmt, ...) {
    struct Timestamp ts;
    do_get_timestamp(&ts);
    uint64_t integer_part = ts.counts / ts.freq;
    uint64_t decimal_part = (ts.counts * 1000000 / ts.freq) % 1000000;
    pl011_uart_printk("[%lld.%06lld] ", integer_part, decimal_part);

    va_list ap;
    int32_t cnt;

    va_start(ap, fmt);
    cnt = pl011_uart_vprintk(fmt, ap);
    va_end(ap);

    return cnt;
}

int32_t pl011_uart_printk_time_polling(const char *fmt, ...) {
    struct Timestamp ts;
    do_get_timestamp(&ts);
    uint64_t integer_part = ts.counts / ts.freq;
    uint64_t decimal_part = (ts.counts * 1000000 / ts.freq) % 1000000;
    pl011_uart_printk_polling("[%lld.%06lld] ", integer_part, decimal_part);

    va_list ap;
    int32_t cnt;

    va_start(ap, fmt);
    cnt = pl011_uart_vprintk_polling(fmt, ap);
    va_end(ap);

    return cnt;
}
