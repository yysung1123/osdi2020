#include <include/assert.h>
#include <include/stdarg.h>
#include <include/printk.h>
#include <include/irq.h>

const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void _panic(const char *file, int line, const char *fmt,...) {
    va_list ap;

    if (panicstr)
        goto dead;
    panicstr = fmt;

    // Be extra sure that the machine is in as reasonable state
    irq_disable();

    va_start(ap, fmt);
    pl011_uart_printk_polling("kernel panic at %s:%d: ", file, line);
    pl011_uart_printk_polling(fmt, ap);
    pl011_uart_printk_polling("\n");
    va_end(ap);

dead:
    while(1);
}

/* like panic, but don't */
void _warn(const char *file, int line, const char *fmt,...) {
    va_list ap;

    va_start(ap, fmt);
    pl011_uart_printk_polling("kernel warning at %s:%d: ", file, line);
    pl011_uart_printk_polling(fmt, ap);
    pl011_uart_printk_polling("\n");
    va_end(ap);
}
