#include <include/mbox.h>
#include <include/mmio.h>
#include <include/types.h>

void mailbox_call(uintptr_t mbox_addr, int32_t ch) {
    uint32_t msg = (uint32_t)mbox_addr, res;
    // msg &= 0xFFFFFFF0;
    // The lower 4 bits of mbox_addr should be zero
    msg |= ch;
    do {
        while (mmio_read(MAILBOX_STATUS) == MAILBOX_FULL) {}
        mmio_write(MAILBOX_WRITE, msg);
        while (mmio_read(MAILBOX_STATUS) == MAILBOX_EMPTY) {}
        res = mmio_read(MAILBOX_READ);
    } while (msg != res);
}
