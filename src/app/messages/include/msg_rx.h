#ifndef COMMS_RX_H
#define COMMS_RX_H

#include "protocol.h"
#include <stddef.h>
#include <stdint.h>

heartbeat_payload *receive_heartbeat();
void receive_flight_state(uint8_t state);
void receive_ack(uint8_t ack_seq);
void receive_nack(uint8_t nacked_seq, uint8_t error);
void receive_bootloader_stats(bootloader_stats_payload *payload);
void receive_telem_imu(const imu *imu_data);
void receive_telem_barometer(const barometer *barometer_data);
void receive_telem_power(const power *power_data);
void receive_log_string(const char *text);
void receive_log_value(uint8_t key_id, float value);
void receive_esp32_oled_print(char *msg, size_t msg_len);
void receive_esp32_oled_clear(void);



#endif