#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>
#include <include/task.h>
#include <include/sched.h>

int main() {
    pl011_uart_init();
    fb_init();
    fb_show_splash_image();
    task_init();

    // lab4 required 1-5
    irq_disable();
    privilege_task_create(&task1);
    privilege_task_create(&task2);
    privilege_task_create(&task3);

    while (1) {
        schedule();
    }
}
