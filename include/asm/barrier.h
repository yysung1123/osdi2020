#pragma once

#define isb() __asm__ volatile("isb" : : : "memory")
#define dsb(opt) __asm__ volatile("dsb " #opt : : : "memory")
