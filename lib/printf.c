#include <include/types.h>
#include <include/stdio.h>
#include <include/syscall.h>

// Collect up to 256 characters into a buffer
// and perform ONE system call to print all of them,
// in order to make the lines output to the console atomic
// and prevent interrupts from causing context switches
// in the middle of a console output line and such.
struct printbuf {
    int idx; // current buffer index
    int cnt; // total bytes printed so far
    char buf[256];
};

static void putch(char ch, struct printbuf *b) {
    b->buf[b->idx++] = ch;
    if (b->idx == 256 - 1) {
        for (size_t nwritten = 0; nwritten < b->idx;
             nwritten += uart_write(b->buf + nwritten, b->idx - nwritten)) {}
        b->idx = 0;
    }
    b->cnt++;
}

size_t vprintf(const char *fmt, va_list ap) {
    struct printbuf b;

    b.idx = 0;
    b.cnt = 0;
    vprintfmt((void *)putch, &b, fmt, ap);
    for (size_t nwritten = 0; nwritten < b.idx;
         nwritten += uart_write(b.buf + nwritten, b.idx - nwritten)) {}

    return b.cnt;
}

size_t printf(const char *fmt, ...) {
    va_list ap;
    int cnt;

    va_start(ap, fmt);
    cnt = vprintf(fmt, ap);
    va_end(ap);

    return cnt;
}
