#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>
#include <include/task.h>
#include <include/sched.h>
#include <include/timer.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();
    task_init();
    core_timer_init();

    // lab4 required 3-2
    privilege_task_create(&task1);
    privilege_task_create(&task2);
    privilege_task_create(&task3);

    irq_enable();

    while (1) {
        schedule();
    }
}
