#include "control_task.h"
#include "comms_protocol.h"
#include "scheduler.h"
#include <string.h>

static uint8_t control_flight_state;
static uint8_t control_flight_mode;

void control_init(void) {
    control_flight_state = FLIGHT_DISARMED;
    control_flight_mode = FLIGHT_MANUAL;
}

void control_task(void) {
    for (;;) {
        task_delay(1000);
    }
}

void control_enqueue_command(const FRAME *frame) {
    send_ack(frame->sequence);

    char *msg = "STM32 Printed";
    send_esp32_oled_print(msg, strlen(msg));
}

uint8_t control_get_flight_state(void) {
    return control_flight_state;
}

uint8_t control_get_flight_mode(void) {
    return control_flight_mode;
}
