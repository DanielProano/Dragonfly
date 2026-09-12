#ifndef COMMS_PROTOCOL_H
#define COMMS_PROTOCOL_H

#include "protocol.h"
#include <stddef.h>
#include <stdint.h>

void send_heartbeat(uint8_t state, uint8_t mode, uint16_t error_flags);
void send_flight_state(uint8_t state);
void send_ack(uint8_t ack_seq);
void send_nack(uint8_t nacked_seq, uint8_t error);
void send_bootloader_stats(bootloader_stats_payload *paylaod);
void send_telem_imu(const imu *imu_data);
void send_telem_barometer(const barometer *barometer_data);
void send_telem_power(const power *power_data);
void send_log_string(const char *text);
void send_log_value(uint8_t key_id, float value);
void send_esp32_oled_print(char *msg, size_t msg_len);
void send_esp32_oled_clear(void);

#endif
