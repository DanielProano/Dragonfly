/* Includes */
#include <stdint.h>
#include "fault_indicator.h"

/* Globals */

/*  stack grows downward, so is top instead of end */
extern uint32_t _stack_start;
extern uint32_t _stack_top;

extern uint32_t _data_start;
extern uint32_t _data_flash_start;
extern uint32_t _data_end;
extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _vector_start;
extern uint32_t _vector_end;
extern uint32_t _text_start;
extern uint32_t _text_end;

/* Function Prototypes */

extern int main();

/* System Exceptions */
void Default_Handler(void);
void Reset_Handler(void);
void NMI_Handler(void)                  __attribute__((weak));
void HardFault_Handler(void)            __attribute__((weak));
void MemManage_Handler(void)            __attribute__((weak));
void BusFault_Handler(void)             __attribute__((weak));
void UsageFault_Handler(void)           __attribute__((weak));
void SVC_Handler(void)                  __attribute__((weak));
void DebugMon_Handler(void)             __attribute__((weak));
void PendSV_Handler(void)               __attribute__((weak));
void SysTick_Handler(void)              __attribute__((weak));

/* Peripheral Interrupts */
void WWDG_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)               __attribute__ ((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void)        __attribute__ ((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void)          __attribute__ ((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)               __attribute__ ((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)             __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void ADC_IRQHandler(void)               __attribute__ ((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler(void)          __attribute__ ((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)            __attribute__ ((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)            __attribute__ ((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void)         __attribute__ ((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void)         __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler(void)       __attribute__ ((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void SDIO_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler(void)            __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler(void)      __attribute__ ((weak, alias("Default_Handler")));
void USART6_IRQHandler(void)            __attribute__ ((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void)           __attribute__ ((weak, alias("Default_Handler")));
void FPU_IRQHandler(void)               __attribute__ ((weak, alias("Default_Handler")));
void SPI4_IRQHandler(void)              __attribute__ ((weak, alias("Default_Handler")));

/* Interrupt Vector Table */

__attribute__ ((section(".isr_vector")))
void (* const vector_table[])(void) = {
    ((void (*)(void)) (&_stack_top)),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* Peripheral Interrupts */
    WWDG_IRQHandler,
    PVD_IRQHandler,
    TAMP_STAMP_IRQHandler,
    RTC_WKUP_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Stream0_IRQHandler,
    DMA1_Stream1_IRQHandler,
    DMA1_Stream2_IRQHandler,
    DMA1_Stream3_IRQHandler,
    DMA1_Stream4_IRQHandler,
    DMA1_Stream5_IRQHandler,
    DMA1_Stream6_IRQHandler,
    ADC_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_IRQHandler,
    TIM1_UP_IRQHandler,
    TIM1_TRG_COM_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    OTG_FS_WKUP_IRQHandler,
    DMA1_Stream7_IRQHandler,
    SDIO_IRQHandler,
    TIM5_IRQHandler,
    SPI3_IRQHandler,
    DMA2_Stream0_IRQHandler,
    DMA2_Stream1_IRQHandler,
    DMA2_Stream2_IRQHandler,
    DMA2_Stream3_IRQHandler,
    DMA2_Stream4_IRQHandler,
    OTG_FS_IRQHandler,
    DMA2_Stream5_IRQHandler,
    DMA2_Stream6_IRQHandler,
    DMA2_Stream7_IRQHandler,
    USART6_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    FPU_IRQHandler,
    SPI4_IRQHandler
};


/*  Function definitions */
void Default_Handler(void) {
    warning_light();
}

void Reset_Handler(void) {
    uint32_t *ptr_data_sram = &_data_start;
    uint32_t *ptr_data_flash = &_data_flash_start;
    uint32_t data_size_words = &_data_end - &_data_start;

    for (uint32_t i = 0; i < data_size_words; i++) {
        *ptr_data_sram++ = *ptr_data_flash++;
    }

    uint32_t bss_size_words = &_bss_end - &_bss_start;
    uint32_t *ptr_bss = &_bss_start;

    for (uint32_t i = 0; i < bss_size_words; i++) {
        *ptr_bss++ = 0;
    }

    main();

    warning_light();
}

void NMI_Handler(void) {
    warning_light();
}

void HardFault_Handler(void) {
    warning_light();
}

void MemManage_Handler(void) {
    warning_light();
}

void BusFault_Handler(void) {
    warning_light();
}

void UsageFault_Handler(void) {
    warning_light();
}

void SVC_Handler(void) {
    warning_light();
}

void DebugMon_Handler(void) {
    warning_light();
}

void PendSV_Handler(void) {
    warning_light();
}

void SysTick_Handler(void) {
    warning_light();
}
