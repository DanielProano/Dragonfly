#include "fpu.h"
#include "rcc.h"
#include "nvic.h"
#include "scheduler.h"
#include "comms_task.h"
#include "control_task.h"

int main(void) {
    fpu_init();
    rcc_init();
    nvic_init();

    comms_init();
    control_init();

    task_create(comms_task, 2, "comms");
    task_create(control_task, 1, "control");

    scheduler_start();

    return 0;
}
