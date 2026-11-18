#ifndef PID_H
#define PID_H

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd, float output_min, float output_max);
void pid_reset(pid_t *pid);
float pid_update(pid_t *pid, float setpoint, float measured, float dt);

#endif
