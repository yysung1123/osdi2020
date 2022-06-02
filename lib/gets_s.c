#include <include/types.h>
#include <include/stdio.h>

void gets_s(char *buf, size_t len) {
    size_t idx = 0;
    char c;

    do {
        c = getchar();
        buf[idx++] = c;
    } while (idx < len && c != '\n');
    buf[idx - 1] = 0;
}
