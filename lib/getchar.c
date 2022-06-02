#include <include/types.h>
#include <include/syscall.h>
#include <include/stdio.h>

char getchar() {
    char buf[1];
    for (size_t nread = 0; nread < 1;
         nread += uart_read(buf + nread, 1 - nread)) {}

    char c = buf[0];
    if (c == '\r') c = '\n';
    putchar(c);
    return c;
}
