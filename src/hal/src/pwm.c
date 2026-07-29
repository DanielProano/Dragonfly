#include "pwm.h"
#include"stm32f401xc.h"

/*  TIM3 drives the four ESC signal lines.
 *
 *  Channel mapping (AF2 = TIM3, page 162 of Reference Manual):
 *      PWM_CH1 -> TIM3_CH1 -> PB4
 *      PWM_CH2 -> TIM3_CH2 -> PB5
 *      PWM_CH3 -> TIM3_CH3 -> PB0
 *      PWM_CH4 -> TIM3_CH4 -> PB1
 *
 *  The channels are NOT in pin order: PB0/PB1 are channels 3 and 4.
 */

/*  APB1 runs at 42MHz (rcc_init divides SYSCLK by 2), but the APB timer
 *  clock doubler applies whenever the APB prescaler is not 1, so TIM3
 *  counts at 84MHz. */
#define TIM3_CLK_HZ         84000000U

/* 1MHz counter clock -> 1 tick = 1us, so CCRx is a pulse width in us */
#define PWM_TICK_HZ         1000000U

#define PWM_DEFAULT_HZ      400U
#define PWM_IDLE_US         1000U

#define TIM_OCM_PWM1        0x6U

/*  CCR1..CCR4 are contiguous in the peripheral, but index explicitly so
 *  the channel-to-register mapping stays readable. */
static volatile uint32_t *const pwm_ccr[NUM_PWMS] = {
    &TIM3->CCR1,
    &TIM3->CCR2,
    &TIM3->CCR3,
    &TIM3->CCR4,
};

void pwm_init(void) {
    /* Enable B GPIO Pins*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* Reset TIM3*/
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

    /*  Push-pull, no pull resistor: the timer drives the line both ways.
     *  High output speed keeps the edges clean if we later move to a
     *  faster protocol such as DShot. */
    GPIOB->OTYPER &= ~((1 << 0) | (1 << 1) | (1 << 4) | (1 << 5));

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0
        | GPIO_PUPDR_PUPD1
        | GPIO_PUPDR_PUPD4
        | GPIO_PUPDR_PUPD5
    );

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

    /*  Clear any leftover timer state. The counter stays stopped until
     *  every channel holds a safe idle pulse. */
    TIM3->CR1 = 0;
    TIM3->CCER = 0;
    TIM3->CCMR1 = 0;
    TIM3->CCMR2 = 0;

    /*  Upcounting, and buffer ARR so a frequency change cannot truncate
     *  the period already in flight. */
    TIM3->CR1 &= ~(TIM_CR1_DIR);
    TIM3->CR1 |= TIM_CR1_ARPE;

    /*  PWM mode 1 (OCxM = 110): output is active while CNT < CCRx.
     *  OCxPE buffers CCRx writes until the next update event, so changing
     *  throttle mid-period cannot emit a runt pulse. Page 305. */
    TIM3->CCMR1 |= (TIM_OCM_PWM1 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
    TIM3->CCMR1 |= (TIM_OCM_PWM1 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
    TIM3->CCMR2 |= (TIM_OCM_PWM1 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
    TIM3->CCMR2 |= (TIM_OCM_PWM1 << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;

    /* CCxP left at reset = active high, which is what an ESC expects */

    /* Set how long a tick is */
    /* On page 307 of reference manual: */
    /* https://deepbluembedded.com/stm32-pwm-example-timer-pwm-mode-tutorial/ */
    /* CK_CNT = f_(CK_PSC) + / (PSC[15:0] + 1) */
    /* 1 MHz = 84 / (x + 1)*/
    /* = 83 */
    /* Now set how many ticks minus 1 */
    /* ARR = period in ticks - 1 */
    pwm_set_frequency(PWM_DEFAULT_HZ);

    /*  Idle throttle on every channel before any output is enabled, so
     *  the first edge an ESC sees is a valid stop command. */
    for (PWM_channel_t ch = PWM_CH1; ch < NUM_PWMS; ch++) {
        pwm_set_pulse_us(ch, PWM_IDLE_US);
    }

    /* Load CCRx out of the preload registers */
    TIM3->EGR |= TIM_EGR_UG;

    /* Start counting. Outputs stay disconnected until pwm_enable */
    TIM3->CR1 |= TIM_CR1_CEN;
}

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

    TIM3->PSC = (TIM3_CLK_HZ / PWM_TICK_HZ) - 1U;
    TIM3->ARR = ticks - 1U;

    /*  PSC has no shadow register of its own and only reloads on an
     *  update event, so force one. */
    TIM3->EGR |= TIM_EGR_UG;
}

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
