#ifndef COMMS_H
#define COMMS_H

#include <stdbool.h>
#include <stdint.h>

#include "protocol.h"

#define COMMS_UART_ID           UART_2
#define COMMS_BAUD              230400U
#define COMMS_FRAME_TIMEOUT_MS  50U
#define COMMS_HEADER_SIZE       4U

void comms_init(void);
bool comms_send(uint8_t id, const void *payload, uint8_t payload_len);
bool comms_receive(frame *out_frame);

#endif