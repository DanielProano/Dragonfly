#include "fpu.h"
#include "rcc.h"
#include "nvic.h"
#include "scheduler.h"
#include "comms_task.h"
#include "control_task.h"
#include "crsf.h"
#include "health_task.h"
#include "imu_task.h"

int main(void) {
    fpu_init();
    rcc_init();
    nvic_init();

    comms_init();
    control_init();
    crsf_init();
    health_init();
    imu_init();

    task_create(imu_task, 4, "imu");
    task_create(crsf_task, 3, "crsf");
    task_create(comms_task, 2, "comms");
    task_create(control_task, 1, "control");
    task_create(health_task, 1, "health");

    scheduler_start();

    return 0;
}
