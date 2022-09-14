#include <include/types.h>
#include <include/string.h>
#include <include/syscall.h>
#include <include/stdio.h>

size_t puts(const char *buf) {
    size_t len = strlen(buf);

    for (size_t nwritten = 0; nwritten < len;
         nwritten += uart_write(buf + nwritten, len - nwritten)) {}

    putchar('\n');

    return len + 1;
}
