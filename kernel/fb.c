#include <include/fb.h>
#include <include/mbox.h>
#include <include/types.h>
#include <include/printk.h>
#include <include/memory.h>

static uint8_t *framebuffer;
static size_t framebuffer_len;

void fb_init() {
    // https://jsandler18.github.io/extra/prop-channel.html
    // configure the framebuffer
    {
        uint32_t mailbox[17] __attribute__((aligned(16)));
        mailbox[0] = 17 * 4; // buffer size in bytes
        mailbox[1] = REQUEST_CODE; // request code
        // tags begin
        mailbox[2] = FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT; // tag identifier
        mailbox[3] = 2 * 4;
        mailbox[4] = TAG_REQUEST_CODE;
        mailbox[5] = 640; // width
        mailbox[6] = 480; // height
        mailbox[7] = FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT; // tag identifier
        mailbox[8] = 2 * 4;
        mailbox[9] = TAG_REQUEST_CODE;
        mailbox[10] = 640; // width
        mailbox[11] = 480; // height
        mailbox[12] = FRAMEBUFFER_SET_DEPTH; // tag identifier
        mailbox[13] = 4;
        mailbox[14] = TAG_REQUEST_CODE;
        mailbox[15] = 24; // depth
        // tags end
        mailbox[16] = END_TAG;

        mailbox_call((uintptr_t)mailbox, 8);
    }

    // allocate the framebuffer
    {
        uint32_t mailbox[8] __attribute__((aligned(16)));
        mailbox[0] = 8 * 4; // buffer size in bytes
        mailbox[1] = REQUEST_CODE; // request code
        // tags begin
        mailbox[2] = FRAMEBUFFER_ALLOCATE_BUFFER; // tag identifier
        mailbox[3] = 2 * 4;
        mailbox[4] = TAG_REQUEST_CODE;
        mailbox[5] = 16; // request: alignment, response: framebuffer base address
        mailbox[6] = 0; // response: framebuffer size
        // tags end
        mailbox[7] = END_TAG;

        mailbox_call((uintptr_t)mailbox, 8);

        framebuffer = (uint8_t *)KADDR((physaddr_t)mailbox[5]);
        framebuffer_len = mailbox[6];
    }

    // print init fb done information
    pl011_uart_printk_time_polling("Init Framebuffer done\n");
}

#ifdef WITH_BMP_SPLASH
void fb_show_splash_image() {
    uint8_t (*img)[480][640][3] = (uint8_t (*)[480][640][3])framebuffer;
    extern const uint8_t *splash_image_end;
    uint8_t (*splash_img)[480][640][3] = (uint8_t (*)[480][640][3])((kernaddr_t)&splash_image_end - 640 * 480 * 3);

    for (int i = 0; i < 480; ++i) {
        for (int j = 0; j < 640; ++j) {
            for (int k = 0; k < 3; ++k) {
                (*img)[i][j][k] = (*splash_img)[(480 - 1) - i][j][(3 - 1) - k];
            }
        }
    }
}
#else
void fb_show_splash_image() {
    uint8_t (*img)[480][640][3] = (uint8_t (*)[480][640][3])framebuffer;

    for (int i = 0; i < 480; ++i) {
        for (int j = 0; j < 640; ++j) {
            uint8_t value = (((i / 20) + (j / 20)) % 2) ? 0 : 0xff;
            (*img)[i][j][0] = (*img)[i][j][1] = (*img)[i][j][2] = value;
        }
    }
}
#endif
