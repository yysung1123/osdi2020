#include <include/shell.h>
#include <include/mmio.h>
#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mbox.h>
#include <include/uart.h>

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reset(uint32_t tick){ // reboot after watchdog timer expire
  mmio_write(PM_RSTC, PM_PASSWORD | 0x20); // full reset
  mmio_write(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void prompt() {
    mini_uart_putchar('#');
    mini_uart_putchar(' ');
}

uint64_t read_frequency() {
    uint64_t res;
    // read frequency of core timer
    __asm__ __volatile("mrs %0, cntfrq_el0"
                       : "=r"(res));
    return res;
}

uint64_t read_counts() {
    uint64_t res;
    // read counts of core timer
    __asm__ __volatile("mrs %0, cntpct_el0"
                       : "=r"(res));
    return res;
}

void get_board_revision(){
    uint32_t mailbox[7] __attribute__((aligned(16)));
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call((uintptr_t)mailbox, 8);

    char outbuf[64];
    snprintf(outbuf, 64, "Board Revision: 0x%x", mailbox[5]); // it should be 0xa02082 for rpi3 b
    mini_uart_puts(outbuf);
}

void do_shell() {
    const size_t CMD_SIZE = 1024, OUTBUF_SIZE = 2048;
    char cmd[CMD_SIZE], outbuf[OUTBUF_SIZE];
    mini_uart_puts("Welcome to NCTU OS");
    get_board_revision();

    while (1) {
        prompt();
        mini_uart_gets_s(cmd, CMD_SIZE);
        if (!strcmp(cmd, "hello")) {
            mini_uart_puts("Hello World!");
        } else if (!strcmp(cmd, "help")) {
            mini_uart_puts("hello : print Hello World!\nhelp : help\nreboot : reboot rpi3\ntimestamp : get current timestamp");
        } else if (!strcmp(cmd, "reboot")) {
            mini_uart_puts("Reboot...");
            reset(0);
        } else if (!strcmp(cmd, "timestamp")) {
            uint64_t freq = read_frequency(), counts = read_counts();
            uint64_t integer_part = counts / freq;
            uint64_t decimal_part = (counts * 1000000 / freq) % 1000000;
            snprintf(outbuf, OUTBUF_SIZE, "[%d.%06d]", integer_part, decimal_part);
            mini_uart_puts(outbuf);
        } else {
            snprintf(outbuf, OUTBUF_SIZE, "Err: command %s not found, try <help>", cmd);
            mini_uart_puts(outbuf);
        }
    }
}
