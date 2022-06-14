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

    // lab4 required 1 and required 2
    for (int i = 0; i < 4; ++i) {
        privilege_task_create(&foo);
    }

    idle();
}
