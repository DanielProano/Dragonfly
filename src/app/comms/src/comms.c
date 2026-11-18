#include <string.h>

#include "comms.h"
#include "mutex.h"
#include "protocol.h"
#include "protocol_codec.h"
#include "crc.h"
#include "uart.h"
#include "macros.h"

static Mutex comms_tx_mutex;
static uint8_t comms_sequence;
static bool g_initialized = false;

#define COMMS_UART_ID           UART_2
#define COMMS_BAUD              230400U
#define COMMS_FRAME_TIMEOUT_MS  50U

void comms_init(void) {
    compute_crc16_table();
    uart_init(COMMS_UART_ID, COMMS_BAUD);
    mutex_init(&comms_tx_mutex);
    comms_sequence = 0;
    g_initialized = true;
}

bool comms_send(uint8_t msg_id, const void *payload, uint8_t payload_len) {
    RETURN_IF_NULL(payload, false);
    RETURN_IF_EMPTY(payload_len, false);

    if (!g_initialized) {
        return false;
    }

    if (payload_len > PAYLOAD_MAX_SIZE) {
        return false;
    }

    if (mutex_lock(&comms_tx_mutex) != MUTEX_OK) {
        return false;
    }

    frame comms_frame;

    comms_frame.start_byte = PROTOCOL_START_BYTE;
    comms_frame.version = PROTOCOL_VERSION;
    comms_frame.message_id = msg_id;
    comms_frame.sequence = comms_sequence++;
    comms_frame.payload_len = payload_len;
    
    memcpy(comms_frame.payload, payload, payload_len);

    uint8_t buffer[sizeof(frame)];
    int encoded_len = protocol_frame_encode(buffer, sizeof(buffer), &comms_frame);

    if (encoded_len > 0) {
        uart_send_bytes(COMMS_UART_ID, buffer, (uint32_t) encoded_len);
    }

    if (mutex_unlock(&comms_tx_mutex) != MUTEX_OK) {
        return false;
    }

    return encoded_len > 0;
}

bool comms_receive(frame *out_frame) {
    RETURN_IF_NULL(out_frame, false);

    if (!g_initialized) {
        return false;
    }

    uint8_t buffer[sizeof(frame)];
    uint8_t start_byte = uart_receive_byte(COMMS_UART_ID);

    if (start_byte != PROTOCOL_START_BYTE) {
        return false;
    }

    buffer[0] = start_byte;

    if (!uart_receive_bytes_timeout(COMMS_UART_ID, &buffer[1], TOTAL_HEADER_BYTE_SIZE - 1, COMMS_FRAME_TIMEOUT_MS)) {
        return false;
    }

    uint8_t payload_len = buffer[TOTAL_HEADER_BYTE_SIZE - 1];

    if (payload_len > PAYLOAD_MAX_SIZE) {
        return false;
    }

    uint32_t remaining = (uint32_t) payload_len + sizeof(out_frame->crc);

    if (!uart_receive_bytes_timeout(COMMS_UART_ID, &buffer[TOTAL_HEADER_BYTE_SIZE], remaining, COMMS_FRAME_TIMEOUT_MS)) {
        return false;
    }

    return protocol_frame_decode(out_frame, buffer, TOTAL_HEADER_BYTE_SIZE + remaining) >= 0;
}
