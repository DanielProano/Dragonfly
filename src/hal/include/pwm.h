#ifndef PWM_H
#define PWM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PWM_CH1,
    PWM_CH2,
    PWM_CH3,
    PWM_CH4,
    NUM_PWMS
} pwm_channel_t;

void pwm_init(void);
bool pwm_set_frequency(uint32_t hz);
void pwm_set_pulse_us(pwm_channel_t channel, uint32_t us);
void pwm_set_duty(pwm_channel_t channel, uint8_t percent);
void pwm_enable(pwm_channel_t channel);
void pwm_disable(pwm_channel_t channel);

#endif
