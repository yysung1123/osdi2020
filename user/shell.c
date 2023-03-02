#include <include/string.h>
#include <include/types.h>
#include <include/stdio.h>
#include <include/utils.h>
#include <include/syscall.h>
#include <include/syscall_test.h>

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
                 "aa : test atomic_add");
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
        } else {
            printf("Err: command %s not found, try <help>\n", cmd);
        }
    }
}
