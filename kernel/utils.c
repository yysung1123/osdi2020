#include <include/utils.h>
#include <include/types.h>

uint64_t timestamp_read_frequency() {
    uint64_t res;
    // read frequency of core timer
    __asm__ __volatile("mrs %0, cntfrq_el0"
                       : "=r"(res));
    return res;
}

uint64_t timestamp_read_counts() {
    uint64_t res;
    // read counts of core timer
    __asm__ __volatile("mrs %0, cntpct_el0"
                       : "=r"(res));
    return res;
}

int64_t do_get_timestamp(struct Timestamp *ts) {
    ts->freq = timestamp_read_frequency();
    ts->counts = timestamp_read_counts();

    return 0;
}
