#ifndef MOTOR_MIX_H
#define MOTOR_MIX_H

#include <stdint.h>

#define NUM_MOTORS 4

typedef struct {
    uint8_t motors[NUM_MOTORS];
} motor_outputs_t;

motor_outputs_t motor_mix_compute(uint8_t throttle_percent, float roll, float pitch, float yaw);

#endif
