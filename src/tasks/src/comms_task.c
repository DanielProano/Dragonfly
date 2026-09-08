#include "comms_task.h"
#include "control_task.h"
#include "uart.h"
#include "protocol.h"
#include "protocol_codec.h"
#include "crc.h"
#include "mutex.h"
#include <string.h>

#define COMMS_UART_ID UART_2
#define COMMS_BAUD    230400U
#define COMMS_FRAME_TIMEOUT_MS 50U
#define COMMS_HEADER_SIZE      4U

static Mutex comms_tx_mutex;
static uint8_t comms_sequence;

void comms_init(void) {
    compute_crc16_table();
    uart_init(COMMS_UART_ID, COMMS_BAUD);
    mutex_init(&comms_tx_mutex);
    comms_sequence = 0;
}

void comms_send(uint8_t msg_id, const void *payload, uint8_t payload_len) {
    FRAME frame;
    uint8_t buffer[sizeof(FRAME)];
    int encoded_len;

    if (payload_len > PAYLOAD_MAX_SIZE) {
        payload_len = PAYLOAD_MAX_SIZE;
    }

    mutex_lock(&comms_tx_mutex);

    frame.start_byte = PROTOCOL_START_BYTE;
    frame.version = PROTOCOL_VERSION;
    frame.message_id = msg_id;
    frame.sequence = comms_sequence++;
    frame.payload_len = payload_len;
    memcpy(frame.payload, payload, payload_len);

    encoded_len = protocol_frame_encode(buffer, sizeof(buffer), &frame);

    mutex_unlock(&comms_tx_mutex);

    if (encoded_len > 0) {
        uart_send_bytes(COMMS_UART_ID, buffer, (uint32_t) encoded_len);
    }
}

void comms_task(void) {
    uint8_t buffer[sizeof(FRAME)];

    for (;;) {
        uint8_t start_byte = uart_receive_byte(COMMS_UART_ID);

        if (start_byte != PROTOCOL_START_BYTE) {
            continue;
        }

        buffer[0] = start_byte;

        if (!uart_receive_bytes_timeout(COMMS_UART_ID, &buffer[1], COMMS_HEADER_SIZE, COMMS_FRAME_TIMEOUT_MS)) {
            continue;
        }

        uint8_t payload_len = buffer[4];

        if (payload_len > PAYLOAD_MAX_SIZE) {
            continue;
        }

        uint32_t remaining = (uint32_t) payload_len + sizeof(uint16_t);

        if (!uart_receive_bytes_timeout(COMMS_UART_ID, &buffer[5], remaining, COMMS_FRAME_TIMEOUT_MS)) {
            continue;
        }

        FRAME frame;

        if (protocol_frame_decode(&frame, buffer, 5 + remaining) < 0) {
            continue;
        }

        control_enqueue_command(&frame);
    }
}
