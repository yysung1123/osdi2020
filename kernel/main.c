#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>
#include <include/task.h>
#include <include/sched.h>
#include <include/timer.h>
#include <include/test_case.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();
    task_init();
    core_timer_init();

    // lab4 required 3 and required 4
    privilege_task_create(&user_test);
    privilege_task_create(&zombie_reaper);

    irq_enable();

    idle();
}
