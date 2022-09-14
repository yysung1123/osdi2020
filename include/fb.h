#pragma once

#include <include/types.h>

#define FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT 0x00048003
#define FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT 0x00048004
#define FRAMEBUFFER_SET_DEPTH 0x00048005
#define FRAMEBUFFER_SET_PXLORDR 0x00048006
#define FRAMEBUFFER_SET_VIRTOFF 0x00048009
#define FRAMEBUFFER_ALLOCATE_BUFFER 0x00040001
#define FRAMEBUFFER_GET_PITCH 0x00040008
#define FRAMEBUFFER_WIDTH 640
#define FRAMEBUFFER_HEIGHT 480

void fb_init();
void fb_show_splash_image();
