#pragma once

#include <include/types.h>

void kmalloc_init();
void* kmalloc(size_t);
void kfree(void *);
void kmalloc_test();
