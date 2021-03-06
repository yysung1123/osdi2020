#pragma once

#include <include/types.h>

#define FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT 0x00048003
#define FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT 0x00048004
#define FRAMEBUFFER_SET_DEPTH 0x00048005
#define FRAMEBUFFER_ALLOCATE_BUFFER 0x00040001

void fb_init();
void fb_show_splash_image();
