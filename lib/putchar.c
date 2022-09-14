#include <include/types.h>
#include <include/syscall.h>

int32_t putchar(int32_t c) {
    char buf[1] = {(char)c};
    for (size_t nwritten = 0; nwritten < 1;
         nwritten += uart_write(buf + nwritten, 1 - nwritten)) {}

    return (int32_t)buf[0];
}
