#include <include/string.h>
#include <include/types.h>
#include <include/stdio.h>
#include <include/utils.h>
#include <include/syscall.h>

void prompt() {
    printf("# ");
}

void __attribute__((optimize("O0"))) delay(uint32_t d) {
    for (uint32_t i = 0; i < d; ++i) {}
}

void test_command1() { // test fork functionality
    int cnt = 0;
    if (fork() == 0) {
        fork();
        fork();
        while (cnt < 10) {
            printf("task id: %d, sp: 0x%llx cnt: %d\n", get_taskid(), &cnt, cnt++); // address should be the same across tasks, but the cnt should be increased indepndently
            delay(1000000);
        }
        exit(0); // all childs exit
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
                 "test1 : test fork functionality");
        } else if (!strcmp(cmd, "timestamp")) {
            struct Timestamp ts;
            get_timestamp(&ts);
            uint64_t integer_part = ts.counts / ts.freq;
            uint64_t decimal_part = (ts.counts * 1000000 / ts.freq) % 1000000;
            printf("[%lld.%06lld]\n", integer_part, decimal_part);
        } else if (!strcmp(cmd, "exc")) {
            __asm__ ("svc #1");
        } else if (!strcmp(cmd, "test1")) {
            test_command1();
        } else {
            printf("Err: command %s not found, try <help>\n", cmd);
        }
    }
}
