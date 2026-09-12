#include "comms_protocol.h"
#include "comms_task.h"
#include "scheduler.h"
#include <string.h>

void send_heartbeat(uint8_t state, uint8_t mode, uint16_t error_flags) {
    heartbeat_payload payload;

    payload.timestamp = scheduler_get_tick_count();
    payload.error_flags = error_flags;
    payload.state = state;
    payload.mode = mode;

    comms_send(MSG_HEARTBEAT, &payload, (uint8_t) sizeof(payload));
}

void send_flight_state(uint8_t state) {
    flight_state_payload payload;

    payload.state = state;

    comms_send(MSG_FLIGHT_STATE, &payload, (uint8_t) sizeof(payload));
}

void send_ack(uint8_t ack_seq) {
    ack_payload payload;

    payload.ack_seq = ack_seq;

    comms_send(MSG_ACK, &payload, (uint8_t) sizeof(payload));
}

void send_nack(uint8_t nacked_seq, uint8_t error) {
    nack_payload payload;

    payload.nacked_seq = nacked_seq;
    payload.error = error;

    comms_send(MSG_NACK, &payload, (uint8_t) sizeof(payload));
}

void send_bootloader_stats(bootloader_stats_payload *payload) {
    comms_send(MSG_BOOTLOADER_STATS, payload, (uint8_t) sizeof(*payload));
}

void send_telem_imu(const imu *imu_data) {
    telem_imu_payload payload;

    payload.imu = *imu_data;

    comms_send(MSG_TELEM_IMU, &payload, (uint8_t) sizeof(payload));
}

void send_telem_barometer(const barometer *barometer_data) {
    telem_barometer_payload payload;

    payload.barometer = *barometer_data;

    comms_send(MSG_TELEM_BAROMETER, &payload, (uint8_t) sizeof(payload));
}

void send_telem_power(const power *power_data) {
    telem_power_payload payload;

    payload.power = *power_data;

    comms_send(MSG_TELEM_POWER, &payload, (uint8_t) sizeof(payload));
}

void send_log_string(const char *text) {
    log_string_payload payload;

    strncpy(payload.text, text, PAYLOAD_TEXT_SIZE - 1);
    payload.text[PAYLOAD_TEXT_SIZE - 1] = '\0';

    comms_send(MSG_LOG_STRING, &payload, (uint8_t) sizeof(payload));
}

void send_log_value(uint8_t key_id, float value) {
    log_value_payload payload;

    payload.key_id = key_id;
    payload.value = value;
    payload.timestamp = scheduler_get_tick_count();

    comms_send(MSG_LOG_VALUE, &payload, (uint8_t) sizeof(payload));
}

void send_esp32_oled_print(char *msg, size_t msg_len) {
    oled_payload payload;
    size_t copy_len = msg_len < (PAYLOAD_TEXT_SIZE - 1) ? msg_len : (PAYLOAD_TEXT_SIZE - 1);

    payload.cmd = OLED_PRINT;
    memcpy(payload.text, msg, copy_len);
    payload.text[copy_len] = '\0';

    comms_send(MSG_OLED, &payload, (uint8_t) sizeof(payload));
}

void send_esp32_oled_clear(void) {
    oled_payload payload;

    payload.cmd = OLED_CLEAR;
    payload.text[0] = '\0';

    comms_send(MSG_OLED, &payload, (uint8_t) sizeof(payload));
}
