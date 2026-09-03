#include "esc.h"
#include "pwm.h"

#define ESC_IDLE 1000U

void esc_init(void) {
    pwm_init();
}

void esc_arm(void) {
    for (pwm_channel_t ch = PWM_CH1; ch < NUM_PWMS; ch++) {
        pwm_enable(ch);
    }
}

void esc_disarm(void) {
    for (pwm_channel_t ch = PWM_CH1; ch < NUM_PWMS; ch++) {
        pwm_disable(ch);
    }
}

void esc_set_throttle(pwm_channel_t channel, uint8_t percent) {
    /* Normalized to */
    uint32_t normalized_us = 1000U + ((uint32_t) percent * 1000U) / 100U;
    pwm_set_pulse_us(channel, normalized_us);
}

void esc_stop_all(void) {
    for (pwm_channel_t channel = PWM_CH1; channel < NUM_PWMS; channel++) {
        pwm_set_pulse_us(channel, ESC_IDLE);
    }
}