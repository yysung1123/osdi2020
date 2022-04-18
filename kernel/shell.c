#include <include/shell.h>
#include <include/mmio.h>
#include <include/stdio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mbox.h>

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reset(uint32_t tick){ // reboot after watchdog timer expire
  mmio_write(PM_RSTC, PM_PASSWORD | 0x20); // full reset
  mmio_write(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

char getchar() {
    char c = mini_uart_read();
    if (c == '\r') c = '\n';
    mini_uart_write(c);

    return c;
}

void gets_s(char *buf, size_t len) {
    size_t idx = 0;
    char c;

    do {
        c = getchar();
        buf[idx++] = c;
    } while (idx < len && c != '\n');
    buf[idx - 1] = 0;
}

void putchar(char c) {
    mini_uart_write(c);
}

void puts(const char *buf) {
    ssize_t i = 0;
    while (buf[i]) {
        putchar(buf[i++]);
    }

    mini_uart_write('\n');
}

void prompt() {
    putchar('#');
    putchar(' ');
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
    puts(outbuf);
}

void do_shell() {
    const size_t CMD_SIZE = 1024, OUTBUF_SIZE = 2048;
    char cmd[CMD_SIZE], outbuf[OUTBUF_SIZE];
    puts("Welcome to NCTU OS");
    get_board_revision();

    while (1) {
        prompt();
        gets_s(cmd, CMD_SIZE);
        if (!strcmp(cmd, "hello")) {
            puts("Hello World!");
        } else if (!strcmp(cmd, "help")) {
            puts("hello : print Hello World!\nhelp : help\nreboot : reboot rpi3\ntimestamp : get current timestamp");
        } else if (!strcmp(cmd, "reboot")) {
            puts("Reboot...");
            reset(0);
        } else if (!strcmp(cmd, "timestamp")) {
            uint64_t freq = read_frequency(), counts = read_counts();
            uint64_t integer_part = counts / freq;
            uint64_t decimal_part = (counts * 1000000 / freq) % 1000000;
            snprintf(outbuf, OUTBUF_SIZE, "[%d.%06d]", integer_part, decimal_part);
            puts(outbuf);
        } else {
            snprintf(outbuf, OUTBUF_SIZE, "Err: command %s not found, try <help>", cmd);
            puts(outbuf);
        }
    }
}
