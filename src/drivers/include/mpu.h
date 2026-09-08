#ifndef MPU_H
#define MPU_H

#include <stdbool.h>

bool mpu_init(void);
void mpu_poll(void);
bool mpu_get_quaternion(float *w, float *x, float *y, float *z);
bool mpu_get_gyro(float *x, float *y, float *z);
bool mpu_get_acceleration(float *x, float *y, float *z);
bool mpu_get_magnitude(float *x, float *y, float *z);

#endif
