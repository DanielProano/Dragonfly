#ifndef ATTITUDE_H
#define ATTITUDE_H

#include <stdbool.h>

typedef struct {
    float quat_w;
    float quat_x;
    float quat_y;
    float quat_z;
    float gyro_roll_dps;
    float gyro_pitch_dps;
    float gyro_yaw_dps;
} attitude_state_t;

typedef struct {
    float roll;
    float pitch;
    float yaw;
} attitude_output_t;

void attitude_init(void);
void attitude_reset(void);
attitude_output_t attitude_update(
    bool angle_mode,
    float roll_setpoint,
    float pitch_setpoint,
    float yaw_rate_setpoint,
    const attitude_state_t *state,
    float dt
);

#endif
