#include <include/string.h>
#include <include/types.h>
#include <include/stdio.h>
#include <include/utils.h>
#include <include/syscall.h>
#include <include/syscall_test.h>

#define BSS_SIZE 10000000
int data[50] = {0, 1, 2};
int bss[BSS_SIZE] = {};

void prompt() {
    printf("# ");
}

void test_aa() {
    if (fork() == 0) {
        pid_t pid[3];
        pid[0] = fork();
        pid[1] = fork();
        pid[2] = fork();

        test(ATOMIC_ADD);

        if (!pid[2]) goto exit;
        wait();

        if (!pid[1]) goto exit;
        wait();

        if (!pid[0]) goto exit;
        wait();

exit:
        exit(0);
    } else {
        wait();
    }
}

void __attribute__((optimize("O0"))) delay(uint32_t d) {
    for (uint32_t i = 0; i < d; ++i) {}
}

void test_command1() { // test fork functionality
    int cnt = 0;
    if (fork() == 0) {
        int32_t pid1 = fork();
        int32_t pid2 = fork();
        while (cnt < 10) {
            printf("task id: %d, sp: 0x%llx cnt: %d\n", get_taskid(), &cnt, cnt++); // address should be the same across tasks, but the cnt should be increased indepndently
            delay(1000000);
        }
        if (!pid2) goto exit;
        wait();

        if (!pid1) goto exit;
        wait();

exit:
        exit(0); // all childs exit
    }

    wait();
}

void test_command2() { // test page fault
    if (fork() == 0) {
        int *a = 0x0; // a non-mapped address.
        printf("%d\n", *a); // trigger simple page fault, child will die here.
    } else {
        wait();
    }
}

void test_command3() { // test page reclaim.
     printf("Remaining page frames : %d\n", get_remain_page_num()); // get number of remaining page frames from kernel by system call.
}

void read_beyond_boundary() {
    if (fork() == 0) {
        int* ptr = mmap(NULL, 4096, PROT_READ, MAP_ANONYMOUS, (void *)-1, 0);
        printf("addr: %llx\n", ptr);
        printf("%d\n", ptr[1000]); // should be 0
        printf("%d\n", ptr[4097]); // should be seg fault
    } else {
        wait();
    }
}

void write_beyond_boundary() {
    if (fork() == 0) {
        int* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, (void *)-1, 0);
        printf("addr: %llx\n", ptr);
        ptr[1000] = 100;
        printf("%d\n", ptr[1000]); // should be 100
        ptr[4097] = 100;// should be seg fault
        printf("%d\n", ptr[4097]); // not reached
    } else {
        wait();
    }
}

void wrong_permission() {
    if (fork() == 0) {
        int* ptr = mmap(NULL, 4096, PROT_READ, MAP_ANONYMOUS, (void *)-1, 0);
        printf("addr: %llx\n", ptr);
        printf("%d\n", ptr[1000]); // should be 0
        for (int i = 0; i < 4096; ++i) {
            ptr[i] = i+1; // should be seg fault
        }
        for (int i = 0; i < 4096; ++i) { // not reached
            printf("%d\n", ptr[i]);
        }
    } else {
        wait();
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#pragma GCC optimize("O0")
int stack_overflow(int i) {
    int a[1024] = {0};
    printf("Recursive %d\n", i);
    stack_overflow(i + 1);
    return a[1023] + i;
}
#pragma GCC diagnostic pop

void test_stack_overflow() {
    if (fork() == 0) {
        stack_overflow(0);
    } else {
        wait();
    }
}

void mmaps() { // test multiple mmaps
    if (fork() == 0) {
        for (int i = 0; i < 40; ++i) {
            if (i < 20) {
                mmap(NULL, 4096, PROT_WRITE|PROT_READ, MAP_ANONYMOUS, (void *)-1, 0);
            } else if (i < 30){
                mmap(NULL, 4096, PROT_WRITE, MAP_ANONYMOUS, (void *)-1, 0);
            } else {
                mmap(NULL, 4096, PROT_WRITE|PROT_READ, MAP_ANONYMOUS, (void *)-1, 0);
            }
        }
        while (1); // hang to let shell see the mapped regions
    }
}

void mmap_unalign() {
    if (fork() == 0) {
        printf("0x%llx", mmap((void*)0x12345678, 0x1fff, PROT_WRITE|PROT_READ, MAP_ANONYMOUS, (void *)-1, 0)); // should be a page aligned address A and region should be A - A +0x2000
        while (1); // hang to let shell see the mapped regions
    }
}

void write_text() {
    if (fork() == 0) {
        int* pc;
        __asm__ volatile(
            "adr %0, ."
            :"=r"(pc)
        );
        *pc = 0; // seg fault
    } else {
        wait();
    }
}

void test_data() {
    if (fork() == 0) {
        for (int i = 0; i < 3; ++i) {
            if (data[i] != i) {
                printf("mismatch at data[%d]: %d\n", i, data[i]);
                exit(0);
            }
        }
        printf("pass test\n");
        exit(0);
    } else {
        wait();
    }
}

void test_bss() {
    if (fork() == 0) {
        for (int i = 0; i < BSS_SIZE; ++i) {
            if (bss[i]) {
                printf("bss[%d] should be 0 initialized: %d\n", i, bss[i]);
                exit(0);
            }
        }
        printf("pass test\n");
        exit(0);
    } else {
        wait();
    }
}

int main() {
    const size_t CMD_SIZE = 1024;
    char cmd[CMD_SIZE];
    puts("Welcome to NCTU OS");

    while (1) {
        prompt();
        gets_s(cmd, CMD_SIZE);
        if (!strcmp(cmd, "hello")) {
            puts("Hello World!");
        } else if (!strcmp(cmd, "help")) {
            puts("hello : print Hello World!\n"
                 "help : help\n"
                 "timestamp : get current timestamp\n"
                 "exc : issue svc #1 and print exception info\n"
                 "aa : test atomic_add\n"
                 "test1 : test fork functionality\n"
                 "test2 : test page fault\n"
                 "test3 : test page reclaim\n"
                 "rbb : read beyond boundary\n"
                 "wbb : write beyond boundary\n"
                 "wp : wrong permission\n"
                 "so : test stack overflow\n"
                 "mmaps : test multiple mmaps\n"
                 "mmap_unalign : test unalign mmap\n"
                 "wt : write text\n"
                 "data : test data section\n"
                 "bss : test bss section");
        } else if (!strcmp(cmd, "timestamp")) {
            struct Timestamp ts;
            get_timestamp(&ts);
            uint64_t integer_part = ts.counts / ts.freq;
            uint64_t decimal_part = (ts.counts * 1000000 / ts.freq) % 1000000;
            printf("[%lld.%06lld]\n", integer_part, decimal_part);
        } else if (!strcmp(cmd, "exc")) {
            __asm__ ("svc #1");
        } else if (!strcmp(cmd, "aa")) {
            test_aa();
        } else if (!strcmp(cmd, "test1")) {
            test_command1();
        } else if (!strcmp(cmd, "test2")) {
            test_command2();
        } else if (!strcmp(cmd, "test3")) {
            test_command3();
        } else if (!strcmp(cmd, "mango")) {
            printf("%d\n", get_remain_mango_node_num());
        } else if (!strcmp(cmd, "rbb")) {
            read_beyond_boundary();
        } else if (!strcmp(cmd, "wbb")) {
            write_beyond_boundary();
        } else if (!strcmp(cmd, "wp")) {
            wrong_permission();
        } else if (!strcmp(cmd, "so")) {
            test_stack_overflow();
        } else if (!strcmp(cmd, "mmaps")) {
            mmaps();
        } else if (!strcmp(cmd, "mmap_unalign")) {
            mmap_unalign();
        } else if (!strcmp(cmd, "wt")) {
            write_text();
        } else if (!strcmp(cmd, "data")) {
            test_data();
        } else if (!strcmp(cmd, "bss")) {
            test_bss();
        } else {
            printf("Err: command %s not found, try <help>\n", cmd);
        }
    }
}
