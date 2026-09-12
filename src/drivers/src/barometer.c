#include "barometer.h"
#include "i2c.h"
#include <math.h>

/* GY-BMP280-3.3 defaults SDO to GND -> address 0x76.
   If SDO is tied to VDD on your module, use 0x77 instead. */
#define BMP280_I2C_ADDR       0x76U

#define BMP280_I2C_WRITE      0U
#define BMP280_I2C_READ       1U

#define BMP280_REG_CALIB_START 0x88U
#define BMP280_REG_CHIP_ID     0xD0U
#define BMP280_REG_CONFIG      0xF5U
#define BMP280_REG_CTRL_MEAS   0xF4U
#define BMP280_REG_PRESS_MSB   0xF7U

#define BMP280_SEA_LEVEL_PA   101325.0f

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;

static bmp280_calib_t calib;

static uint16_t u16_le(const uint8_t *p) {
    return (uint16_t) (p[0] | (p[1] << 8));
}

static int16_t s16_le(const uint8_t *p) {
    return (int16_t) u16_le(p);
}

static bool bmp280_write_reg(uint8_t reg, uint8_t value) {
    bool ok = true;

    ok = ok && i2c_start();
    ok = ok && i2c_send_address(BMP280_I2C_ADDR, BMP280_I2C_WRITE);
    ok = ok && i2c_write_byte(reg);
    ok = ok && i2c_write_byte(value);
    i2c_stop();

    return ok;
}

static bool bmp280_read_regs(uint8_t reg, uint8_t *buffer, uint8_t length) {
    bool ok = true;

    ok = ok && i2c_start();
    ok = ok && i2c_send_address(BMP280_I2C_ADDR, BMP280_I2C_WRITE);
    ok = ok && i2c_write_byte(reg);

    ok = ok && i2c_start();
    ok = ok && i2c_send_address(BMP280_I2C_ADDR, BMP280_I2C_READ);

    for (uint8_t i = 0; ok && (i < length); i++) {
        bool ack = (i < (length - 1U));
        ok = i2c_read_byte(ack, &buffer[i]);
    }

    i2c_stop();

    return ok;
}

bool barometer_init(void) {
    uint8_t calib_raw[24];

    i2c_init();

    if (!bmp280_read_regs(BMP280_REG_CALIB_START, calib_raw, sizeof(calib_raw))) {
        return false;
    }

    calib.dig_T1 = u16_le(&calib_raw[0]);
    calib.dig_T2 = s16_le(&calib_raw[2]);
    calib.dig_T3 = s16_le(&calib_raw[4]);
    calib.dig_P1 = u16_le(&calib_raw[6]);
    calib.dig_P2 = s16_le(&calib_raw[8]);
    calib.dig_P3 = s16_le(&calib_raw[10]);
    calib.dig_P4 = s16_le(&calib_raw[12]);
    calib.dig_P5 = s16_le(&calib_raw[14]);
    calib.dig_P6 = s16_le(&calib_raw[16]);
    calib.dig_P7 = s16_le(&calib_raw[18]);
    calib.dig_P8 = s16_le(&calib_raw[20]);
    calib.dig_P9 = s16_le(&calib_raw[22]);

    /* ctrl_meas: temp oversample x1, pressure oversample x4, normal mode */
    if (!bmp280_write_reg(BMP280_REG_CTRL_MEAS, (0x1U << 5) | (0x3U << 2) | 0x3U)) {
        return false;
    }

    /* config: standby 0.5ms, IIR filter x4 */
    return bmp280_write_reg(BMP280_REG_CONFIG, (0x0U << 5) | (0x2U << 2));
}

bool barometer_poll(barometer *out) {
    uint8_t raw[6];
    double var1, var2, temperature, pressure;
    int32_t t_fine;
    int32_t adc_P;
    int32_t adc_T;

    if (!bmp280_read_regs(BMP280_REG_PRESS_MSB, raw, sizeof(raw))) {
        return false;
    }

    adc_P = ((int32_t) raw[0] << 12) | ((int32_t) raw[1] << 4) | (raw[2] >> 4);
    adc_T = ((int32_t) raw[3] << 12) | ((int32_t) raw[4] << 4) | (raw[5] >> 4);

    var1 = (((double) adc_T) / 16384.0 - ((double) calib.dig_T1) / 1024.0) * ((double) calib.dig_T2);
    var2 = ((((double) adc_T) / 131072.0 - ((double) calib.dig_T1) / 8192.0) *
            (((double) adc_T) / 131072.0 - ((double) calib.dig_T1) / 8192.0)) * ((double) calib.dig_T3);
    t_fine = (int32_t) (var1 + var2);
    temperature = (var1 + var2) / 5120.0;

    var1 = ((double) t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double) calib.dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double) calib.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double) calib.dig_P4) * 65536.0);
    var1 = (((double) calib.dig_P3) * var1 * var1 / 524288.0 + ((double) calib.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double) calib.dig_P1);

    if (var1 == 0.0) {
        return false;
    }

    pressure = 1048576.0 - (double) adc_P;
    pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double) calib.dig_P9) * pressure * pressure / 2147483648.0;
    var2 = pressure * ((double) calib.dig_P8) / 32768.0;
    pressure = pressure + (var1 + var2 + ((double) calib.dig_P7)) / 16.0;

    out->pressure_pascal = (float) pressure;
    out->temperature_celsius = (float) temperature;
    out->altitude_meters = 44330.0f * (1.0f - powf((float) pressure / BMP280_SEA_LEVEL_PA, 0.1903f));

    return true;
}
