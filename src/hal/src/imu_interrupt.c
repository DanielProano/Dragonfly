#include "imu_interrupt.h"
#include "semaphore.h"
#include "stm32f401xc.h"

/* BNO08x INT pin -> PA0. Sensor drives it push-pull, active low, to
   signal a new report is ready to be read over I2C. */

static Semaphore imu_data_ready;

void imu_interrupt_init(void) {
    semaphore_init(&imu_data_ready, 1);

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIOA->MODER &= ~(0x3U << 0);
    GPIOA->PUPDR &= ~(0x3U << 0);
    GPIOA->PUPDR |= (0x1U << 0);

    /* SYSCFG_EXTICR1 bits [3:0] = 0000 routes EXTI line 0 to GPIOA. */
    SYSCFG->EXTICR[0] &= ~(0xFU << 0);

    EXTI->RTSR &= ~EXTI_RTSR_TR0;
    EXTI->FTSR |= EXTI_FTSR_TR0;
    EXTI->IMR |= EXTI_IMR_MR0;

    NVIC_EnableIRQ(EXTI0_IRQn);
}

void imu_interrupt_wait(void) {
    semaphore_wait(&imu_data_ready);
}

void EXTI0_IRQHandler(void) {
    EXTI->PR = EXTI_PR_PR0;
    semaphore_signal(&imu_data_ready);
}
