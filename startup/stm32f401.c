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

    /* Peripheral Interrupts.
       IMPORTANT: array index N here (0-based from this comment) must
       equal the real hardware IRQn, per stm32f401xc.h's IRQn_Type enum.
       STM32F401 skips several IRQ numbers used for peripherals it
       doesn't have (CAN, USART3, UART4/5, TIM8, FSMC, ETH, OTG_HS, ...) -
       those slots must stay present as 0 placeholders or every vector
       after them silently points at the wrong handler. */
    WWDG_IRQHandler,                  /* 0 */
    PVD_IRQHandler,                   /* 1 */
    TAMP_STAMP_IRQHandler,            /* 2 */
    RTC_WKUP_IRQHandler,              /* 3 */
    FLASH_IRQHandler,                 /* 4 */
    RCC_IRQHandler,                   /* 5 */
    EXTI0_IRQHandler,                 /* 6 */
    EXTI1_IRQHandler,                 /* 7 */
    EXTI2_IRQHandler,                 /* 8 */
    EXTI3_IRQHandler,                 /* 9 */
    EXTI4_IRQHandler,                 /* 10 */
    DMA1_Stream0_IRQHandler,          /* 11 */
    DMA1_Stream1_IRQHandler,          /* 12 */
    DMA1_Stream2_IRQHandler,          /* 13 */
    DMA1_Stream3_IRQHandler,          /* 14 */
    DMA1_Stream4_IRQHandler,          /* 15 */
    DMA1_Stream5_IRQHandler,          /* 16 */
    DMA1_Stream6_IRQHandler,          /* 17 */
    ADC_IRQHandler,                   /* 18 */
    0,                                /* 19 - reserved (CAN1_TX) */
    0,                                /* 20 - reserved (CAN1_RX0) */
    0,                                /* 21 - reserved (CAN1_RX1) */
    0,                                /* 22 - reserved (CAN1_SCE) */
    EXTI9_5_IRQHandler,               /* 23 */
    TIM1_BRK_IRQHandler,              /* 24 */
    TIM1_UP_IRQHandler,               /* 25 */
    TIM1_TRG_COM_IRQHandler,          /* 26 */
    TIM1_CC_IRQHandler,               /* 27 */
    TIM2_IRQHandler,                  /* 28 */
    TIM3_IRQHandler,                  /* 29 */
    TIM4_IRQHandler,                  /* 30 */
    I2C1_EV_IRQHandler,               /* 31 */
    I2C1_ER_IRQHandler,               /* 32 */
    I2C2_EV_IRQHandler,               /* 33 */
    I2C2_ER_IRQHandler,               /* 34 */
    SPI1_IRQHandler,                  /* 35 */
    SPI2_IRQHandler,                  /* 36 */
    USART1_IRQHandler,                /* 37 */
    USART2_IRQHandler,                /* 38 */
    0,                                /* 39 - reserved (USART3) */
    EXTI15_10_IRQHandler,             /* 40 */
    RTC_Alarm_IRQHandler,             /* 41 */
    OTG_FS_WKUP_IRQHandler,           /* 42 */
    0,                                /* 43 - reserved (TIM8_BRK_TIM12) */
    0,                                /* 44 - reserved (TIM8_UP_TIM13) */
    0,                                /* 45 - reserved (TIM8_TRG_COM_TIM14) */
    0,                                /* 46 - reserved (TIM8_CC) */
    DMA1_Stream7_IRQHandler,          /* 47 */
    0,                                /* 48 - reserved (FSMC) */
    SDIO_IRQHandler,                  /* 49 */
    TIM5_IRQHandler,                  /* 50 */
    SPI3_IRQHandler,                  /* 51 */
    0,                                /* 52 - reserved (UART4) */
    0,                                /* 53 - reserved (UART5) */
    0,                                /* 54 - reserved (TIM6_DAC) */
    0,                                /* 55 - reserved (TIM7) */
    DMA2_Stream0_IRQHandler,          /* 56 */
    DMA2_Stream1_IRQHandler,          /* 57 */
    DMA2_Stream2_IRQHandler,          /* 58 */
    DMA2_Stream3_IRQHandler,          /* 59 */
    DMA2_Stream4_IRQHandler,          /* 60 */
    0,                                /* 61 - reserved (ETH) */
    0,                                /* 62 - reserved (ETH_WKUP) */
    0,                                /* 63 - reserved (CAN2_TX) */
    0,                                /* 64 - reserved (CAN2_RX0) */
    0,                                /* 65 - reserved (CAN2_RX1) */
    0,                                /* 66 - reserved (CAN2_SCE) */
    OTG_FS_IRQHandler,                /* 67 */
    DMA2_Stream5_IRQHandler,          /* 68 */
    DMA2_Stream6_IRQHandler,          /* 69 */
    DMA2_Stream7_IRQHandler,          /* 70 */
    USART6_IRQHandler,                /* 71 */
    I2C3_EV_IRQHandler,               /* 72 */
    I2C3_ER_IRQHandler,               /* 73 */
    0,                                /* 74 - reserved (OTG_HS_EP1_OUT) */
    0,                                /* 75 - reserved (OTG_HS_EP1_IN) */
    0,                                /* 76 - reserved (OTG_HS_WKUP) */
    0,                                /* 77 - reserved (OTG_HS) */
    0,                                /* 78 - reserved (DCMI) */
    0,                                /* 79 - reserved (CRYP) */
    0,                                /* 80 - reserved (HASH_RNG) */
    FPU_IRQHandler,                   /* 81 */
    0,                                /* 82 - reserved */
    0,                                /* 83 - reserved */
    SPI4_IRQHandler                   /* 84 */
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
