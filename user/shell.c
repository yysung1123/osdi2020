#include <include/shell.h>
#include <include/mmio.h>
#include <include/string.h>
#include <include/types.h>
#include <include/mbox.h>
#include <include/stdio.h>
#include <include/utils.h>
#include <include/syscall.h>

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reset(uint32_t tick){ // reboot after watchdog timer expire
    mmio_write(PM_RSTC, PM_PASSWORD | 0x20); // full reset
    mmio_write(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void prompt() {
    printf("# ");
}

void get_board_revision() {
    uint32_t mailbox[7] __attribute__((aligned(16)));
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call((uintptr_t)mailbox, 8);

    printf("Board Revision: 0x%x\n", mailbox[5]); // it should be 0xa02082 for rpi3 b
}

void get_vc_base_address() {
    uint32_t mailbox[8] __attribute__((aligned(16)));
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_VC_MEMORY; // tag identifier
    mailbox[3] = 2 * 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // response: base address
    mailbox[6] = 0; // response: size
    // tags end
    mailbox[7] = END_TAG;

    mailbox_call((uintptr_t)mailbox, 8);

    printf("VC base address: 0x%x\n", mailbox[5]);
}

uint32_t getuint32_be() {
    char buf[4];
    for (size_t nread = 0; nread < 4;
         nread += pl011_uart_read(buf + nread, 4 - nread)) {}

    uint32_t res = 0;
    for (int i = 0; i < 4; ++i) {
        res <<= 8;
        res += (uint8_t)buf[i];
    }

    return res;
}

uint32_t crc32(uint32_t crc, uint8_t *p, size_t len) {
    crc = ~crc;

    while (len >= sizeof(uint64_t)) {
        __asm__ volatile("crc32x %w[crc], %w[crc], %x[data]"
                         :[crc] "+r"(crc)
                         :[data] "r"(*(uint64_t *)p));
        p += sizeof(uint64_t);
        len -= sizeof(uint64_t);
    }

    if (len >= sizeof(uint32_t)) {
        __asm__ volatile("crc32w %w[crc], %w[crc], %w[data]"
                         :[crc] "+r"(crc)
                         :[data] "r"(*(uint32_t *)p));
        p += sizeof(uint32_t);
        len -= sizeof(uint32_t);
    }

    if (len >= sizeof(uint16_t)) {
        __asm__ volatile("crc32h %w[crc], %w[crc], %w[data]"
                         :[crc] "+r"(crc)
                         :[data] "r"(*(uint16_t *)p));
        p += sizeof(uint16_t);
        len -= sizeof(uint16_t);
    }

    if (len >= sizeof(uint8_t)) {
        __asm__ volatile("crc32b %w[crc], %w[crc], %w[data]"
                         :[crc] "+r"(crc)
                         :[data] "r"(*(uint8_t *)p));
        p += sizeof(uint8_t);
        len -= sizeof(uint8_t);
    }

    return ~crc;
}

__attribute__((noinline, section("bootstrap")))void bootstrap_func(
    register physaddr_t image_addr, register size_t image_size,
    register physaddr_t tmp_image_addr, register physaddr_t new_stack_top) {
    // move stack to new_stack_top
    __asm__ volatile("mov sp, %0"
                     :: "r"(new_stack_top));

    for (size_t i = 0; i < ROUNDUP(image_size, 8) / 8; ++i) {
        *(uint64_t *)(image_addr + i * 8) = *(uint64_t *)(tmp_image_addr + i * 8);
    }

    ((void (*)())image_addr)();
}

void loadimg() {
    /*
    Low Memory
    ----
    max(_KERNEL_END, new image end)
    ----
    tmp new image
    ----
    bootstrap()
    ----
    stack for bootstrap() (4k)
    ----
    High Memory
    */
    extern const void _KERNEL_END, __start_bootstrap, __stop_bootstrap;

    size_t image_size = getuint32_be();
    printf("Image size: %d\n", image_size);

    physaddr_t image_addr = getuint32_be();
    printf("Start address: 0x%016x\n", (uint32_t)image_addr);
    physaddr_t tmp_image_addr = MAX((physaddr_t)&_KERNEL_END, image_addr + image_size);

    uint32_t input_checksum = getuint32_be();
    for (size_t nread = 0; nread < image_size;
         nread += pl011_uart_read((uint8_t *)tmp_image_addr + nread, image_size - nread)) {}

    uint32_t checksum = crc32(0, (uint8_t *)tmp_image_addr, image_size);

    if (input_checksum != checksum) {
        printf("Checksum mismatch\nExpected: %x\nReceived: %x\n", input_checksum, checksum);
        return;
    }

    // move bootstrap to new memory space
    uint8_t *new_bootstrap_addr = (uint8_t *)ROUNDUP(tmp_image_addr + image_size, 8);
    size_t bootstrap_size = (size_t)(&__stop_bootstrap - &__start_bootstrap);
    for (size_t i = 0; i < bootstrap_size; ++i) {
        *(new_bootstrap_addr + i) = *(uint8_t *)(&__start_bootstrap + i);
    }

    // set new_stack_top to new bootstrap end + 4k;
    physaddr_t new_stack_top = (physaddr_t)(ROUNDUP(new_bootstrap_addr + bootstrap_size + 4096, 16));

    // jump to bootstrap
    ((void (*)(physaddr_t, size_t, physaddr_t, physaddr_t))new_bootstrap_addr)(image_addr, image_size, tmp_image_addr, new_stack_top);
}

void shell_main() {
    const size_t CMD_SIZE = 1024;
    char cmd[CMD_SIZE];
    extern const void _KERNEL_START;
    puts("Welcome to NCTU OS");
    printf("Kernel start address: 0x%016x\n", &_KERNEL_START);
    get_board_revision();
    get_vc_base_address();

    while (1) {
        prompt();
        gets_s(cmd, CMD_SIZE);
        if (!strcmp(cmd, "hello")) {
            puts("Hello World!");
        } else if (!strcmp(cmd, "help")) {
            puts("hello : print Hello World!\nhelp : help\n"
                 "reboot : reboot rpi3\ntimestamp : get current timestamp\n"
                 "loadimg: load the new kernel from UART\n"
                 "exc: issue svc #1 and print exception info\n"
                 "irq: enable timers");
        } else if (!strcmp(cmd, "reboot")) {
            puts("Reboot...");
            reset(0);
        } else if (!strcmp(cmd, "timestamp")) {
            struct Timestamp ts;
            get_timestamp(&ts);
            uint64_t integer_part = ts.counts / ts.freq;
            uint64_t decimal_part = (ts.counts * 1000000 / ts.freq) % 1000000;
            printf("[%lld.%06lld]\n", integer_part, decimal_part);
        } else if (!strcmp(cmd, "loadimg")) {
            loadimg();
        } else if (!strcmp(cmd, "exc")) {
            __asm__ ("svc #1");
        } else if (!strcmp(cmd, "irq")) {
            init_timers();
        } else {
            printf("Err: command %s not found, try <help>\n", cmd);
        }
    }
}
