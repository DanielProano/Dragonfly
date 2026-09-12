#include "pid.h"

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_min, float output_max) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

void pid_reset(pid_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

float pid_update(pid_t *pid, float setpoint, float measured, float dt) {
    float error = setpoint - measured;
    float derivative = (dt > 0.0f) ? (error - pid->prev_error) / dt : 0.0f;
    float output;

    pid->integral += error * dt;
    output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);

    if (output > pid->output_max) {
        pid->integral -= error * dt;
        output = pid->output_max;
    } else if (output < pid->output_min) {
        pid->integral -= error * dt;
        output = pid->output_min;
    }

    pid->prev_error = error;

    return output;
}
