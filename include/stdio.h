#pragma once

#include <include/stdarg.h>

// lib/printfmt.c
void printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);
int snprintf(char *str, int size, const char *fmt, ...);
int vsnprintf(char *str, int size, const char *fmt, va_list);

// lib/printf.c
size_t printf(const char *fmt, ...);
size_t vprintf(const char *fmt, va_list ap);

// lib/puts.c
size_t puts(const char *);

// lib/putchar.c
int32_t putchar(int32_t);

// lib/gets_s.c
void gets_s(char *, size_t);

// lib/getchar.c
char getchar();
