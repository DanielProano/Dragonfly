#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>

void i2c_init(void);
void i2c_lock(void);
void i2c_unlock(void);
bool i2c_start(void);
bool i2c_write_byte(uint8_t byte);
bool i2c_send_address(uint8_t data, uint8_t read_or_write);
bool i2c_read_byte(uint8_t ack_enable, uint8_t *byte_out);
void i2c_stop(void);

#endif