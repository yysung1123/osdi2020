#include <include/uart.h>
#include <include/fb.h>
#include <include/irq.h>
#include <include/task.h>

int main() {
    irq_disable();

    pl011_uart_init();
    fb_init();
    fb_show_splash_image();

    // lab4 required 1-3
    privilege_task_create(&task1);
    privilege_task_create(&task2);
    switch_to(get_task(0), get_task(1));
}
