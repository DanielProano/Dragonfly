#include "mpu.h"
#include "i2c.h"
#include <stdint.h>
#include <stddef.h>

/* BNO080/BNO085, SDA/SCA address select pin low -> 0x4A.
   If the address pin is tied high on your breakout, use 0x4B instead. */
#define BNO08X_I2C_ADDR         0x4AU

#define BNO08X_I2C_WRITE        0U
#define BNO08X_I2C_READ         1U

#define SHTP_CHANNEL_COMMAND    0U
#define SHTP_CHANNEL_EXECUTABLE 1U
#define SHTP_CHANNEL_CONTROL    2U
#define SHTP_CHANNEL_REPORTS    3U
#define SHTP_NUM_CHANNELS       6U

#define SHTP_REPORT_SET_FEATURE_COMMAND 0xFDU
#define SHTP_REPORT_BASE_TIMESTAMP      0xFBU

#define SENSOR_REPORTID_ACCELEROMETER   0x01U
#define SENSOR_REPORTID_GYROSCOPE       0x02U
#define SENSOR_REPORTID_MAGNETIC_FIELD  0x03U
#define SENSOR_REPORTID_ROTATION_VECTOR 0x05U

#define SENSOR_REPORT_LENGTH            10U
#define ROTATION_VECTOR_REPORT_LENGTH   14U

/* Q-point (fixed point fraction bits) for each report type, per the
   SH-2 reference manual's default sensor metadata. */
#define ACCEL_Q_POINT  8U
#define GYRO_Q_POINT   9U
#define MAG_Q_POINT    4U
#define QUAT_Q_POINT   14U

#define MPU_REPORT_INTERVAL_US  100000UL /* 100ms, matches health_task's poll rate */
#define MPU_RX_BUFFER_SIZE      64U

static uint8_t shtp_sequence_number[SHTP_NUM_CHANNELS];

static float accel[3];
static float gyro[3];
static float magnitude[3];
static float quaternion[4];

static bool accel_ready;
static bool gyro_ready;
static bool magnitude_ready;
static bool quaternion_ready;

static float q_to_float(int16_t fixed, uint8_t q_point) {
    return (float) fixed / (float) (1U << q_point);
}

static int16_t read_i16_le(const uint8_t *buffer) {
    return (int16_t) ((uint16_t) buffer[0] | ((uint16_t) buffer[1] << 8));
}

static void mpu_boot_delay(void) {
    /* Busy-wait so the sensor's power-on boot (~100-200ms) completes
       before the first I2C transaction. Runs before the scheduler
       starts, so task_delay() is not available here. */
    for (volatile uint32_t i = 0; i < 4000000U; i++) {
    }
}

static bool shtp_send_packet(uint8_t channel, const uint8_t *payload, uint8_t payload_length) {
    uint16_t total_length = (uint16_t) payload_length + 4U;
    bool ok = true;

    ok = ok && i2c_start();
    ok = ok && i2c_send_address(BNO08X_I2C_ADDR, BNO08X_I2C_WRITE);
    ok = ok && i2c_write_byte((uint8_t) (total_length & 0xFFU));
    ok = ok && i2c_write_byte((uint8_t) ((total_length >> 8) & 0x7FU));
    ok = ok && i2c_write_byte(channel);
    ok = ok && i2c_write_byte(shtp_sequence_number[channel]);

    for (uint8_t i = 0; ok && (i < payload_length); i++) {
        ok = i2c_write_byte(payload[i]);
    }

    i2c_stop();

    shtp_sequence_number[channel]++;

    return ok;
}

static bool shtp_receive_packet(uint8_t *channel_out, uint8_t *buffer, uint8_t buffer_size, uint8_t *length_out) {
    uint8_t header[4];
    uint16_t total_length;
    uint8_t payload_length;
    bool ok = true;

    ok = ok && i2c_start();
    ok = ok && i2c_send_address(BNO08X_I2C_ADDR, BNO08X_I2C_READ);

    for (uint8_t i = 0; ok && (i < 3U); i++) {
        ok = i2c_read_byte(true, &header[i]);
    }

    if (!ok) {
        i2c_stop();
        return false;
    }

    total_length = (uint16_t) header[0] | (((uint16_t) header[1] & 0x7FU) << 8);
    payload_length = (total_length > 4U) ? (uint8_t) (total_length - 4U) : 0U;
    if (payload_length > buffer_size) {
        payload_length = buffer_size;
    }

    ok = i2c_read_byte(payload_length > 0U, &header[3]);

    for (uint8_t i = 0; ok && (i < payload_length); i++) {
        bool ack = (i < (payload_length - 1U));
        ok = i2c_read_byte(ack, &buffer[i]);
    }

    i2c_stop();

    if (!ok) {
        return false;
    }

    *channel_out = header[2];
    *length_out = payload_length;
    return true;
}

static bool enable_feature(uint8_t feature_report_id) {
    uint8_t payload[17] = { 0 };

    payload[0] = SHTP_REPORT_SET_FEATURE_COMMAND;
    payload[1] = feature_report_id;
    payload[5] = (uint8_t) (MPU_REPORT_INTERVAL_US & 0xFFU);
    payload[6] = (uint8_t) ((MPU_REPORT_INTERVAL_US >> 8) & 0xFFU);
    payload[7] = (uint8_t) ((MPU_REPORT_INTERVAL_US >> 16) & 0xFFU);
    payload[8] = (uint8_t) ((MPU_REPORT_INTERVAL_US >> 24) & 0xFFU);

    return shtp_send_packet(SHTP_CHANNEL_CONTROL, payload, sizeof(payload));
}

static void parse_input_reports(const uint8_t *buffer, uint8_t length) {
    uint8_t offset = 0;

    while (offset < length) {
        uint8_t report_id = buffer[offset];

        if (report_id == SHTP_REPORT_BASE_TIMESTAMP) {
            offset += 5U;
            continue;
        }

        if (report_id == SENSOR_REPORTID_ROTATION_VECTOR) {
            if ((offset + ROTATION_VECTOR_REPORT_LENGTH) > length) {
                break;
            }

            quaternion[1] = q_to_float(read_i16_le(&buffer[offset + 4]), QUAT_Q_POINT);
            quaternion[2] = q_to_float(read_i16_le(&buffer[offset + 6]), QUAT_Q_POINT);
            quaternion[3] = q_to_float(read_i16_le(&buffer[offset + 8]), QUAT_Q_POINT);
            quaternion[0] = q_to_float(read_i16_le(&buffer[offset + 10]), QUAT_Q_POINT);
            quaternion_ready = true;

            offset += ROTATION_VECTOR_REPORT_LENGTH;
            continue;
        }

        if ((report_id == SENSOR_REPORTID_ACCELEROMETER) ||
            (report_id == SENSOR_REPORTID_GYROSCOPE) ||
            (report_id == SENSOR_REPORTID_MAGNETIC_FIELD)) {
            float x, y, z;

            if ((offset + SENSOR_REPORT_LENGTH) > length) {
                break;
            }

            switch (report_id) {
                case SENSOR_REPORTID_ACCELEROMETER:
                    x = q_to_float(read_i16_le(&buffer[offset + 4]), ACCEL_Q_POINT);
                    y = q_to_float(read_i16_le(&buffer[offset + 6]), ACCEL_Q_POINT);
                    z = q_to_float(read_i16_le(&buffer[offset + 8]), ACCEL_Q_POINT);
                    accel[0] = x;
                    accel[1] = y;
                    accel[2] = z;
                    accel_ready = true;
                    break;
                case SENSOR_REPORTID_GYROSCOPE:
                    x = q_to_float(read_i16_le(&buffer[offset + 4]), GYRO_Q_POINT);
                    y = q_to_float(read_i16_le(&buffer[offset + 6]), GYRO_Q_POINT);
                    z = q_to_float(read_i16_le(&buffer[offset + 8]), GYRO_Q_POINT);
                    gyro[0] = x;
                    gyro[1] = y;
                    gyro[2] = z;
                    gyro_ready = true;
                    break;
                default:
                    x = q_to_float(read_i16_le(&buffer[offset + 4]), MAG_Q_POINT);
                    y = q_to_float(read_i16_le(&buffer[offset + 6]), MAG_Q_POINT);
                    z = q_to_float(read_i16_le(&buffer[offset + 8]), MAG_Q_POINT);
                    magnitude[0] = x;
                    magnitude[1] = y;
                    magnitude[2] = z;
                    magnitude_ready = true;
                    break;
            }

            offset += SENSOR_REPORT_LENGTH;
            continue;
        }

        /* Unrecognized report: its length can't be determined safely,
           so the rest of this packet is discarded. */
        break;
    }
}

bool mpu_init(void) {
    uint8_t channel;
    uint8_t buffer[MPU_RX_BUFFER_SIZE];
    uint8_t length;
    bool ok = true;

    i2c_init();
    mpu_boot_delay();

    for (uint8_t ch = 0; ch < SHTP_NUM_CHANNELS; ch++) {
        shtp_sequence_number[ch] = 0;
    }

    /* Drain the advertisement/reset packets the sensor sends unsolicited
       on power-up so the first Set Feature command lands cleanly. */
    for (uint8_t i = 0; i < 4U; i++) {
        shtp_receive_packet(&channel, buffer, sizeof(buffer), &length);
    }

    ok = ok && enable_feature(SENSOR_REPORTID_ACCELEROMETER);
    ok = ok && enable_feature(SENSOR_REPORTID_GYROSCOPE);
    ok = ok && enable_feature(SENSOR_REPORTID_MAGNETIC_FIELD);
    ok = ok && enable_feature(SENSOR_REPORTID_ROTATION_VECTOR);

    return ok;
}

void mpu_poll(void) {
    uint8_t channel;
    uint8_t buffer[MPU_RX_BUFFER_SIZE];
    uint8_t length;

    if (!shtp_receive_packet(&channel, buffer, sizeof(buffer), &length)) {
        return;
    }

    if ((channel != SHTP_CHANNEL_REPORTS) || (length == 0U)) {
        return;
    }

    parse_input_reports(buffer, length);
}

bool mpu_get_quaternion(float *w, float *x, float *y, float *z) {
    if (!quaternion_ready) {
        return false;
    }

    *w = quaternion[0];
    *x = quaternion[1];
    *y = quaternion[2];
    *z = quaternion[3];
    return true;
}

bool mpu_get_gyro(float *x, float *y, float *z) {
    if (!gyro_ready) {
        return false;
    }

    *x = gyro[0];
    *y = gyro[1];
    *z = gyro[2];
    return true;
}

bool mpu_get_acceleration(float *x, float *y, float *z) {
    if (!accel_ready) {
        return false;
    }

    *x = accel[0];
    *y = accel[1];
    *z = accel[2];
    return true;
}

bool mpu_get_magnitude(float *x, float *y, float *z) {
    if (!magnitude_ready) {
        return false;
    }

    *x = magnitude[0];
    *y = magnitude[1];
    *z = magnitude[2];
    return true;
}
