#ifndef PWM_H
#define PWM_H

#include "stdint.h"

typedef enum {
    PWM_CH1,
    PWM_CH2,
    PWM_CH3,
    PWM_CH4,
    NUM_PWMS
} PWM_channel_t;

void pwm_init(void);
void pwm_set_frequency(uint32_t hz);
void pwm_set_duty(PWM_channel_t channel, uint8_t percent);
void pwm_enable(PWM_channel_t channel);
void pwm_disable(PWM_channel_t channel);

#endif