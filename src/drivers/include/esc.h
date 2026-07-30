#ifndef ESC_H
#define ESC_H

#include "pwm.h"

void esc_init(void);
void esc_arm(void);
void esc_disarm(void);
void esc_set_throttle(PWM_channel_t channel, uint8_t percent);
void esc_stop_all(void);

#endif