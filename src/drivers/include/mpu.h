#ifndef MPU_H
#define MPU_H

#include "stdbool.h"
#include "stdint.h"

typedef struct {
    uint8_t shtp_lsb;
    uint8_t shtp_msb;
    uint8_t shtp_channel;
    uint8_t shtp_sequence_number;
    uint8_t feature_report_id;
    uint8_t report_id;
    uint8_t interval_msb;
    uint8_t interval_byte_5;
    uint8_t interval_byte_4;
    uint8_t interval_byte_3;
    uint8_t interval_byte_2;
    uint8_t interval_lsb;
    uint8_t reserved_1;
    uint8_t reserved_2;
    uint8_t reserved_3;
    uint8_t reserved_4;
    uint8_t reserved_5;
    uint8_t reserved_6;
    uint8_t reserved_7;
    uint8_t reserved_8;
    uint8_t reserved_9;
    uint8_t reserved_10;
} set_feature_report;

typedef struct {
    uint8_t report_id;
    uint8_t delta_lsb;
    uint8_t delta_byte1;
    uint8_t delta_byte2;
    uint8_t delta_msb;
} timebase_reference_report;

typedef struct {
    uint8_t report_id;
    uint8_t sequence_number;
    uint8_t status;
    uint8_t delay;
} sensor_report;

typedef struct {
    uint8_t axis_x_lsb;
    uint8_t axis_x_msb;
    uint8_t axis_y_lsb;
    uint8_t axis_y_msb;
    uint8_t axis_z_lsb;
    uint8_t axis_z_msb;
} accelerometer_input_report;

typedef struct {
    uint8_t quaternion_i_lsb;
    uint8_t quaternion_i_msb;
    uint8_t quaternion_j_lsb;
    uint8_t quaternion_j_msb;
    uint8_t quaternion_k_lsb;
    uint8_t quaternion_k_msb;
    uint8_t quaternion_real_lsb;
    uint8_t quaternion_real_msb;
    uint8_t quaternion_accuracy_lsb;
    uint8_t quanternion_accuracy_msb;
} rotation_vector_report;

void mpu_init(void);
void mpu_poll(void);
bool mpu_get_quaternion(float *w, float *x, float *y, float *z);
bool mpu_get_gyro(float *x, float *y, float *z);
bool mpu_get_acceleration(float *x, float *y, float *z);
bool mpu_get_magnitude(float *x, float *y, float *z);

#endif