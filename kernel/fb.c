#include <include/fb.h>
#include <include/mbox.h>
#include <include/types.h>
#include <include/printk.h>

static uint8_t *framebuffer;
static size_t framebuffer_len;
static int32_t pitch;

void fb_init() {
    // https://jsandler18.github.io/extra/prop-channel.html
    // configure the framebuffer
    uint32_t mailbox[30] __attribute__((aligned(16)));
    mailbox[0] = 30 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE; // request code
    // tags begin
    mailbox[2] = FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT; // tag identifier
    mailbox[3] = 2 * 4;
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = FRAMEBUFFER_WIDTH; // width
    mailbox[6] = FRAMEBUFFER_HEIGHT; // height
    mailbox[7] = FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT; // tag identifier
    mailbox[8] = 2 * 4;
    mailbox[9] = TAG_REQUEST_CODE;
    mailbox[10] = FRAMEBUFFER_WIDTH; // width
    mailbox[11] = FRAMEBUFFER_HEIGHT; // height
    mailbox[12] = FRAMEBUFFER_SET_DEPTH; // tag identifier
    mailbox[13] = 4;
    mailbox[14] = TAG_REQUEST_CODE;
    mailbox[15] = 32; // depth
    mailbox[16] = FRAMEBUFFER_SET_PXLORDR;
    mailbox[17] = 4;
    mailbox[18] = TAG_REQUEST_CODE;
    mailbox[19] = 1;
    mailbox[20] = FRAMEBUFFER_GET_PITCH;
    mailbox[21] = 4;
    mailbox[22] = TAG_REQUEST_CODE;
    mailbox[23] = 0;
    mailbox[24] = FRAMEBUFFER_ALLOCATE_BUFFER; // tag identifier
    mailbox[25] = 2 * 4;
    mailbox[26] = TAG_REQUEST_CODE;
    mailbox[27] = 16; // request: alignment, response: framebuffer base address
    mailbox[28] = 0; // response: framebuffer size
    // tags end
    mailbox[29] = END_TAG;

    mailbox_call((uintptr_t)mailbox, 8);

    pitch = mailbox[23];
    framebuffer = (uint8_t *)((physaddr_t)mailbox[27] & 0x3FFFFFFF);
    framebuffer_len = mailbox[28];
}

#ifdef WITH_BMP_SPLASH
void fb_show_splash_image() {
    if (framebuffer == NULL) return;

    uint8_t (*img)[FRAMEBUFFER_HEIGHT][pitch / 4][4] = (uint8_t (*)[FRAMEBUFFER_HEIGHT][pitch / 4][4])framebuffer;
    extern const uint8_t *splash_image_end;
    uint8_t (*splash_img)[480][640][3] = (uint8_t (*)[480][640][3])((physaddr_t)&splash_image_end - 640 * 480 * 3);

    for (int i = 0; i < 480; ++i) {
        for (int j = 0; j < 640; ++j) {
            for (int k = 0; k < 3; ++k) {
                (*img)[i][j][k] = (*splash_img)[(480 - 1) - i][j][k];
            }
        }
    }
}
#else
void fb_show_splash_image() {
    if (framebuffer == NULL) return;

    uint8_t (*img)[FRAMEBUFFER_HEIGHT][pitch / 4][4] = (uint8_t (*)[FRAMEBUFFER_HEIGHT][pitch / 4][4])framebuffer;

    for (int i = 0; i < FRAMEBUFFER_HEIGHT; ++i) {
        for (int j = 0; j < FRAMEBUFFER_WIDTH; ++j) {
            uint8_t value = (((i / 20) + (j / 20)) % 2) ? 0 : 0xff;
            (*img)[i][j][0] = (*img)[i][j][1] = (*img)[i][j][2] = value;
        }
    }
}
#endif
