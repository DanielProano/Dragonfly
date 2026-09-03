#include "uart.h"
#include "stm32f401xc.h"
#include "queue.h"
#include "mutex.h"
#include "scheduler.h"

#define UART_TX_QUEUE_DEPTH 64u
#define UART_RX_QUEUE_DEPTH 64u

typedef struct {
    USART_TypeDef *base;
    IRQn_Type irqn;
    uint8_t tx_buffer[UART_TX_QUEUE_DEPTH];
    uint8_t rx_buffer[UART_RX_QUEUE_DEPTH];
    Queue tx_queue;
    Queue rx_queue;
    Mutex tx_mutex;
    Mutex rx_mutex;
    volatile uint32_t rx_overrun_count;
} uart_instance_t;

static uart_instance_t uart_instances[NUM_UARTS];

void uart_init(uart_id_t id, uint32_t baud) {
    uart_instance_t *instance = &uart_instances[id];
    uint32_t pclk_hz;

    /* Page 119 of Reference */

    /* Turn on GPIOA pin clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    switch (id) {
        case UART_1:
            /* Page 123 of Reference Manual */

            /* Enable USART1 clock */
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

            /* Page 158 in Reference Manual*/

            /* Clear GPIO A9 Pin */
            GPIOA->MODER &= ~(0x3U << 18);
            /* Set A9 to alternate function mode (USART1_TX) */
            GPIOA->MODER |= (0x2U << 18);

            /* Clear GPIO A10 Pin*/
            GPIOA->MODER &= ~(0x3U << 20);
            /* Set A10 to alternate function mode (USART1_RX) */
            GPIOA->MODER |= (0x2U << 20);

            /*  Configure alternate function registers */
            /*  Page 44 of STM32 datasheet demands we write
                AF07 into A9 & A10 to configure alternate
                function mapping */

            /* We need to configure pin 9 for A9, reset it*/
            GPIOA->AFR[1] &= ~(0xFU << 4);

            /* See page 162 of Reference Manual */
            GPIOA->AFR[1] |= (7U << 4);

            /* A10 needs reset */
            GPIOA->AFR[1] &= ~(0xFU << 8);

            /* See page 165 of Reference Manual, write AF7 into A10 */
            GPIOA->AFR[1] |= (7U << 8);

            instance->base = USART1;
            instance->irqn = USART1_IRQn;
            pclk_hz = 84000000U;
            break;

        case UART_2:
            RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

            GPIOA->MODER &= ~(0x3U << 4);
            GPIOA->MODER |= (0x2U << 4);
            GPIOA->MODER &= ~(0x3U << 6);
            GPIOA->MODER |= (0x2U << 6);

            GPIOA->AFR[0] &= ~(0xFU << 8);
            GPIOA->AFR[0] |= (7U << 8);
            GPIOA->AFR[0] &= ~(0xFU << 12);
            GPIOA->AFR[0] |= (7U << 12);

            instance->base = USART2;
            instance->irqn = USART2_IRQn;
            pclk_hz = 42000000U;
            break;

        case UART_6:
            RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

            GPIOA->MODER &= ~(0x3U << 22);
            GPIOA->MODER |= (0x2U << 22);
            GPIOA->MODER &= ~(0x3U << 24);
            GPIOA->MODER |= (0x2U << 24);

            GPIOA->AFR[1] &= ~(0xFU << 12);
            GPIOA->AFR[1] |= (8U << 12);
            GPIOA->AFR[1] &= ~(0xFU << 16);
            GPIOA->AFR[1] |= (8U << 16);

            instance->base = USART6;
            instance->irqn = USART6_IRQn;
            pclk_hz = 84000000U;
            break;

        default:
            return;
    }

    /*  Page 552 of Reference Manual
        Baud Rate = f_CK / ( 8 * (2 - OVER8) * USARTDIV)
        Default is 0 for control register, meaning
        OVER8 is 0, meaning oversamping by 16
        = f_CK / ( 16 * USARTDIV)
        Rearrange for USARTDIV:
        USARTDIV = f_CK / (Baud Rate * 16 )
        USARTDIV * 16 = f_CK / Baud Rate
        Put USARTDIV * 16 into mantissa & fractional form:
        mantissa = (USARTDIV * 16) / 16
        fraction = (USARTDIV * 16) % 16 */
    uint32_t usartdiv_x16 = (pclk_hz + (baud / 2U)) / baud;
    instance->base->BRR = ((usartdiv_x16 / 16U) << 4) | (usartdiv_x16 % 16U);

    /* Now enable Transmitter*/
    instance->base->CR1 |= USART_CR1_TE;
    /* Enable Receiver */
    instance->base->CR1 |= USART_CR1_RE;
    /* Enable USART */
    instance->base->CR1 |= USART_CR1_UE;

    queue_init(&instance->tx_queue, instance->tx_buffer, sizeof(uint8_t), UART_TX_QUEUE_DEPTH);
    queue_init(&instance->rx_queue, instance->rx_buffer, sizeof(uint8_t), UART_RX_QUEUE_DEPTH);
    mutex_init(&instance->tx_mutex);
    mutex_init(&instance->rx_mutex);
    instance->rx_overrun_count = 0;

    instance->base->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(instance->irqn);
}

void uart_send_byte(uart_id_t id, uint8_t byte) {
    uart_send_bytes(id, &byte, 1);
}

uint8_t uart_receive_byte(uart_id_t id) {
    uint8_t byte;
    uart_receive_bytes(id, &byte, 1);
    return byte;
}

void uart_send_bytes(uart_id_t id, const uint8_t *buffer, uint32_t length) {
    uart_instance_t *instance = &uart_instances[id];

    mutex_lock(&instance->tx_mutex);

    for (uint32_t i = 0; i < length; i++) {
        enqueue(&instance->tx_queue, (void *) &buffer[i]);
    }

    mutex_unlock(&instance->tx_mutex);

    instance->base->CR1 |= USART_CR1_TXEIE;
}

void uart_receive_bytes(uart_id_t id, uint8_t *buffer, uint32_t length) {
    uart_instance_t *instance = &uart_instances[id];

    mutex_lock(&instance->rx_mutex);

    for (uint32_t i = 0; i < length; i++) {
        dequeue(&instance->rx_queue, &buffer[i]);
    }

    mutex_unlock(&instance->rx_mutex);
}

bool uart_receive_byte_timeout(uart_id_t id, uint8_t *byte, uint32_t timeout_ms) {
    return uart_receive_bytes_timeout(id, byte, 1, timeout_ms);
}

bool uart_receive_bytes_timeout(uart_id_t id, uint8_t *buffer, uint32_t length, uint32_t timeout_ms) {
    uart_instance_t *instance = &uart_instances[id];
    uint32_t deadline = scheduler_get_tick_count() + timeout_ms;
    bool success = true;

    mutex_lock(&instance->rx_mutex);

    for (uint32_t i = 0; i < length; i++) {
        uint32_t now = scheduler_get_tick_count();
        uint32_t remaining = (deadline > now) ? (deadline - now) : 0;

        if (!dequeue_timeout(&instance->rx_queue, &buffer[i], remaining)) {
            success = false;
            break;
        }
    }

    mutex_unlock(&instance->rx_mutex);

    return success;
}

uint32_t uart_rx_overrun_count(uart_id_t id) {
    return uart_instances[id].rx_overrun_count;
}

static void uart_irq_common(uart_instance_t *instance) {
    uint32_t sr = instance->base->SR;

    if (sr & (USART_SR_RXNE | USART_SR_ORE | USART_SR_FE | USART_SR_NE)) {
        uint8_t byte = (uint8_t) instance->base->DR;

        if ((sr & USART_SR_RXNE) && !(sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE))) {
            if (!enqueue_isr(&instance->rx_queue, &byte)) {
                instance->rx_overrun_count += 1;
            }
        }
    }

    if (sr & USART_SR_TXE) {
        uint8_t byte;

        if (dequeue_isr(&instance->tx_queue, &byte)) {
            instance->base->DR = byte;
        } else {
            instance->base->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

void USART1_IRQHandler(void) {
    uart_irq_common(&uart_instances[UART_1]);
}

void USART2_IRQHandler(void) {
    uart_irq_common(&uart_instances[UART_2]);
}

void USART6_IRQHandler(void) {
    uart_irq_common(&uart_instances[UART_6]);
}
