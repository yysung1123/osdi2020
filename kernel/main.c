#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>
#include <include/task.h>
#include <include/sched.h>
#include <include/timer.h>
#include <include/mm.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();
    task_init();
    core_timer_init();
    mem_init();

    // lab5 required 3
    privilege_task_create(&user_shell);

    irq_enable();

    idle();
}
