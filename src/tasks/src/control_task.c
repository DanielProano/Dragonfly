#include "control_task.h"
#include "comms_protocol.h"
#include "scheduler.h"
#include "gpio.h"
#include "uart.h"
#include <string.h>

#define CONTROL_HEARTBEAT_INTERVAL_MS 500U

static uint8_t control_flight_state;
static uint8_t control_flight_mode;

void control_init(void) {
    control_flight_state = FLIGHT_DISARMED;
    control_flight_mode = FLIGHT_MANUAL;

    gpio_init_pc13();
}

void control_task(void) {
    for (;;) {
        if (!uart_any_byte_seen()) {
            gpio_toggle_pc13();
        }
        task_delay(CONTROL_HEARTBEAT_INTERVAL_MS);
    }
}

void control_enqueue_command(const FRAME *frame) {
    /* Temporary diagnostic: rapid-flicker the heartbeat LED so a
       command's arrival at the STM32 is visible independent of
       whether its response makes it back over the link. */
    for (int i = 0; i < 6; i++) {
        gpio_toggle_pc13();
        task_delay(50);
    }

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
