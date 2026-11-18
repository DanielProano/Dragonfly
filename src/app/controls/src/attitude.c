#include "attitude.h"
#include "pid.h"
#include <math.h>

#define RAD_TO_DEG_F 57.29577951308232f

/* Outer (angle) loop output is a rate setpoint fed to the inner loop. */
#define ANGLE_RATE_LIMIT_DPS 200.0f

/* Inner (rate) loop output feeds directly into motor_mix as a percent-scale
   correction against throttle. */
#define RATE_OUTPUT_LIMIT 50.0f

/* Placeholder gains only - never tuned against real hardware. Deliberately
   conservative (undershoot rather than oscillate) as a bench-test starting
   point, not a flight-ready tune. Retune before ever flying this. */
#define ANGLE_KP 4.0f

#define ROLL_RATE_KP 0.5f
#define ROLL_RATE_KI 0.3f
#define ROLL_RATE_KD 0.01f

#define PITCH_RATE_KP 0.5f
#define PITCH_RATE_KI 0.3f
#define PITCH_RATE_KD 0.01f

#define YAW_RATE_KP 0.8f
#define YAW_RATE_KI 0.3f
#define YAW_RATE_KD 0.0f

static pid_t roll_angle_pid;
static pid_t pitch_angle_pid;
static pid_t roll_rate_pid;
static pid_t pitch_rate_pid;
static pid_t yaw_rate_pid;

static float quat_to_roll_deg(const attitude_state_t *state) {
    float sinr_cosp = 2.0f * (state->quat_w * state->quat_x + state->quat_y * state->quat_z);
    float cosr_cosp = 1.0f - 2.0f * (state->quat_x * state->quat_x + state->quat_y * state->quat_y);

    return atan2f(sinr_cosp, cosr_cosp) * RAD_TO_DEG_F;
}

static float quat_to_pitch_deg(const attitude_state_t *state) {
    float sinp = 2.0f * (state->quat_w * state->quat_y - state->quat_z * state->quat_x);

    if (sinp > 1.0f) {
        sinp = 1.0f;
    } else if (sinp < -1.0f) {
        sinp = -1.0f;
    }

    return asinf(sinp) * RAD_TO_DEG_F;
}

void attitude_init(void) {
    pid_init(&roll_angle_pid, ANGLE_KP, 0.0f, 0.0f, -ANGLE_RATE_LIMIT_DPS, ANGLE_RATE_LIMIT_DPS);
    pid_init(&pitch_angle_pid, ANGLE_KP, 0.0f, 0.0f, -ANGLE_RATE_LIMIT_DPS, ANGLE_RATE_LIMIT_DPS);
    pid_init(&roll_rate_pid, ROLL_RATE_KP, ROLL_RATE_KI, ROLL_RATE_KD, -RATE_OUTPUT_LIMIT, RATE_OUTPUT_LIMIT);
    pid_init(&pitch_rate_pid, PITCH_RATE_KP, PITCH_RATE_KI, PITCH_RATE_KD, -RATE_OUTPUT_LIMIT, RATE_OUTPUT_LIMIT);
    pid_init(&yaw_rate_pid, YAW_RATE_KP, YAW_RATE_KI, YAW_RATE_KD, -RATE_OUTPUT_LIMIT, RATE_OUTPUT_LIMIT);
}

void attitude_reset(void) {
    pid_reset(&roll_angle_pid);
    pid_reset(&pitch_angle_pid);
    pid_reset(&roll_rate_pid);
    pid_reset(&pitch_rate_pid);
    pid_reset(&yaw_rate_pid);
}

attitude_output_t attitude_update(
    bool angle_mode,
    float roll_setpoint,
    float pitch_setpoint,
    float yaw_rate_setpoint,
    const attitude_state_t *state,
    float dt
) {
    attitude_output_t output;
    float roll_rate_setpoint;
    float pitch_rate_setpoint;

    if (angle_mode) {
        roll_rate_setpoint = pid_update(&roll_angle_pid, roll_setpoint, quat_to_roll_deg(state), dt);
        pitch_rate_setpoint = pid_update(&pitch_angle_pid, pitch_setpoint, quat_to_pitch_deg(state), dt);
    } else {
        roll_rate_setpoint = roll_setpoint;
        pitch_rate_setpoint = pitch_setpoint;
    }

    output.roll = pid_update(&roll_rate_pid, roll_rate_setpoint, state->gyro_roll_dps, dt);
    output.pitch = pid_update(&pitch_rate_pid, pitch_rate_setpoint, state->gyro_pitch_dps, dt);
    output.yaw = pid_update(&yaw_rate_pid, yaw_rate_setpoint, state->gyro_yaw_dps, dt);

    return output;
}
