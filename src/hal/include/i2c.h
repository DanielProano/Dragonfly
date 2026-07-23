#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(void);
void i2c_start(void);
void i2c_write_byte(uint8_t byte);
void i2c_send_address(uint8_t data, uint8_t read_or_write);
uint8_t i2c_read_byte(uint8_t ack_enable);
void i2c_stop(void);

#endif