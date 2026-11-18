#ifndef CONTROL_TASK_H
#define CONTROL_TASK_H

#include "protocol.h"
#include <stdint.h>

void control_init(void);
void control_task(void);
void control_enqueue_command(const frame *frame);
uint8_t control_get_flight_state(void);
uint8_t control_get_flight_mode(void);

#endif
