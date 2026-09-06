#ifndef COMMS_TASK_H
#define COMMS_TASK_H

#include <stdint.h>

void comms_init(void);
void comms_task(void);
void comms_send(uint8_t msg_id, const void *payload, uint8_t payload_len);

#endif
