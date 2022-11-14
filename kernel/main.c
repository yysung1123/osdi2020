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

    irq_enable();

    // lab4 elective 1
    privilege_task_create(&user_test);
    privilege_task_create_priority(&zombie_reaper, HIGH);

    idle();
}
