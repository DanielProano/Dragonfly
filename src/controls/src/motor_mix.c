#include "motor_mix.h"

/* Standard quad-X mixing. Assumes motors[0..3] (which map straight to
   DSHOT_CH1..4 in dshot.c) are wired front-right, rear-right, rear-left,
   front-left, with (front-right, rear-left) spinning one direction and
   (rear-right, front-left) the other. This is a convention, not something
   the hardware enforces - verify it against actual wiring/prop direction
   on the bench (props off) before flight: command a small positive roll
   and confirm the right-side motors slow down, not speed up, etc. */

static uint8_t motor_mix_clamp(float value) {
    if (value < 0.0f) {
        return 0U;
    }

    if (value > 100.0f) {
        return 100U;
    }

    return (uint8_t) value;
}

motor_outputs_t motor_mix_compute(uint8_t throttle_percent, float roll, float pitch, float yaw) {
    motor_outputs_t outputs;
    float throttle = (float) throttle_percent;

    outputs.motors[0] = motor_mix_clamp(throttle - roll - pitch - yaw); /* front-right */
    outputs.motors[1] = motor_mix_clamp(throttle - roll + pitch + yaw); /* rear-right */
    outputs.motors[2] = motor_mix_clamp(throttle + roll + pitch - yaw); /* rear-left */
    outputs.motors[3] = motor_mix_clamp(throttle + roll - pitch + yaw); /* front-left */

    return outputs;
}
