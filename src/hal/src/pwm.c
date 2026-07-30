#include "pwm.h"
#include"stm32f401xc.h"

#define TIM3_CLK_HZ         84000000U

/* 1MHz counter clock -> 1 tick = 1us, so CCRx is a pulse width in us */
#define PWM_TICK_HZ         1000000U

#define PWM_DEFAULT_HZ      400U
#define PWM_IDLE_US         1000U

#define TIM_OCM_PWM1        0x6U

static volatile uint32_t *const pwm_ccr[NUM_PWMS] = {
    &TIM3->CCR1,
    &TIM3->CCR2,
    &TIM3->CCR3,
    &TIM3->CCR4,
};

void pwm_init(void) {
    /* Enable B GPIO Pins*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* Enable TIM3*/
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* Config I/O Mode as alternate mode*/
    GPIOB->MODER &= ~(GPIO_MODER_MODE0);
    GPIOB->MODER &= ~(GPIO_MODER_MODE1);
    GPIOB->MODER &= ~(GPIO_MODER_MODE4);
    GPIOB->MODER &= ~(GPIO_MODER_MODE5);

    GPIOB->MODER |= (0x2 << GPIO_MODER_MODE0_Pos);
    GPIOB->MODER |= (0x2 << GPIO_MODER_MODE1_Pos);
    GPIOB->MODER |= (0x2 << GPIO_MODER_MODE4_Pos);
    GPIOB->MODER |= (0x2 << GPIO_MODER_MODE5_Pos);

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0
        | GPIO_PUPDR_PUPD1
        | GPIO_PUPDR_PUPD4
        | GPIO_PUPDR_PUPD5
    );

    /* Set GPIO B as high speed */
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0
        | GPIO_OSPEEDR_OSPEED1
        | GPIO_OSPEEDR_OSPEED4
        | GPIO_OSPEEDR_OSPEED5
    );

    GPIOB->OSPEEDR |= (0x2 << GPIO_OSPEEDR_OSPEED0_Pos);
    GPIOB->OSPEEDR |= (0x2 << GPIO_OSPEEDR_OSPEED1_Pos);
    GPIOB->OSPEEDR |= (0x2 << GPIO_OSPEEDR_OSPEED4_Pos);
    GPIOB->OSPEEDR |= (0x2 << GPIO_OSPEEDR_OSPEED5_Pos);

    /* Configure alternate functions */
    /* Page 162 of Reference Manual */
    GPIOB->AFR[0] &= ~(15 << 0);
    GPIOB->AFR[0] &= ~(15 << 4);
    GPIOB->AFR[0] &= ~(15 << 16);
    GPIOB->AFR[0] &= ~(15 << 20);

    GPIOB->AFR[0] |= GPIO_AFRL_AFRL0_1;
    GPIOB->AFR[0] |= GPIO_AFRL_AFRL1_1;
    GPIOB->AFR[0] |= GPIO_AFRL_AFRL4_1;
    GPIOB->AFR[0] |= GPIO_AFRL_AFRL5_1;

    /*  Enable PWM mode 1 (OCxM = 110) */
    /*  OCxPE buffers CCRx writes until the next update event */
    /*  Page 300 */
    TIM3->CCMR1 |= (TIM_OCM_PWM1 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM3->CCMR1 |= (TIM_OCM_PWM1 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
    TIM3->CCMR2 |= (TIM_OCM_PWM1 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
    TIM3->CCMR2 |= (TIM_OCM_PWM1 << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;

    pwm_set_frequency(PWM_DEFAULT_HZ);

    /*  Idle throttle */
    for (PWM_channel_t ch = PWM_CH1; ch < NUM_PWMS; ch++) {
        pwm_set_pulse_us(ch, PWM_IDLE_US);
    }

    /* force reload */
    TIM3->EGR |= TIM_EGR_UG;

    /* counter enable */
    TIM3->CR1 |= TIM_CR1_CEN;
}

/* pwm works by comparing CNT against CCR */
/* So pwn will auto-increment CNT & wrap it around */
/* A channel is active is CNT < CCR */
void pwm_set_frequency(uint32_t hz) {
    if (hz == 0) {
        return;
    }

    uint32_t ticks = PWM_TICK_HZ / hz;

    /*  ARR is 16 bit on TIM3, so the longest period a 1us tick can hold
     *  is 65536us. Anything slower would silently wrap. */
    if (ticks == 0 || ticks > 65536U) {
        return;
    }

    /* Set how long a tick is */
    /* On page 307 of reference manual: */
    /* https://deepbluembedded.com/stm32-pwm-example-timer-pwm-mode-tutorial/ */
    /* CK_CNT = f_(CK_PSC) + / (PSC[15:0] + 1) */
    /* 1 MHz = 84 / (x + 1)*/
    /* = 83 */
    TIM3->PSC = (TIM3_CLK_HZ / PWM_TICK_HZ) - 1U;

    /* Now set how many ticks minus 1 */
    /* ARR = period in ticks - 1 */
    TIM3->ARR = ticks - 1U;

    /*  force reload */
    TIM3->EGR |= TIM_EGR_UG;
}

/* Configure CCR for a channel */
void pwm_set_pulse_us(PWM_channel_t channel, uint32_t us) {
    if (channel >= NUM_PWMS) {
        return;
    }

    uint32_t period_us = TIM3->ARR + 1U;

    if (us > period_us) {
        us = period_us;
    }

    *pwm_ccr[channel] = us;
}

/* Percentage wise configure CCR for a channel */
void pwm_set_duty(PWM_channel_t channel, uint8_t percent) {
    if (channel >= NUM_PWMS) {
        return;
    }

    if (percent > 100) {
        percent = 100;
    }

    pwm_set_pulse_us(channel, ((TIM3->ARR + 1U) * percent) / 100U);
}

void pwm_enable(PWM_channel_t channel) {
    if (channel >= NUM_PWMS) {
        return;
    }

    /* Each channel owns a 4 bit field in CCER */
    TIM3->CCER |= (TIM_CCER_CC1E << (channel * 4U));
}

void pwm_disable(PWM_channel_t channel) {
    if (channel >= NUM_PWMS) {
        return;
    }

    TIM3->CCER &= ~(TIM_CCER_CC1E << (channel * 4U));
}
