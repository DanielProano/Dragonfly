#include "uart.h"
#include "stm32f401xc.h"
#include "queue.h"
#include "mutex.h"
#include "semaphore.h"
#include "scheduler.h"
#include "protocol.h"
#include <string.h>

#define UART_RX_QUEUE_DEPTH  (255)
#define UART_DMA_RX_BUF_SIZE (255)
#define UART_DMA_TX_BUF_SIZE (255)

typedef struct {
    USART_TypeDef *base;
    IRQn_Type irqn;

    DMA_Stream_TypeDef *dma_rx_stream;
    DMA_Stream_TypeDef *dma_tx_stream;
    uint32_t dma_tx_cr_bits;

    uint8_t dma_rx_buffer[UART_DMA_RX_BUF_SIZE];
    uint32_t dma_rx_read_index;
    uint8_t dma_tx_buffer[UART_DMA_TX_BUF_SIZE];

    uint8_t rx_buffer[UART_RX_QUEUE_DEPTH];
    Queue rx_queue;
    Mutex rx_mutex;
    Semaphore tx_dma_done;

    volatile uint32_t rx_overrun_count;
} uart_instance_t;

static uart_instance_t uart_instances[NUM_UARTS];

void uart_init(uart_id_t id, uint32_t baud) {
    uart_instance_t *instance = &uart_instances[id];
    uint32_t pclk_hz;
    DMA_Stream_TypeDef *dma_rx_stream;
    DMA_Stream_TypeDef *dma_tx_stream;
    IRQn_Type dma_rx_irqn;
    IRQn_Type dma_tx_irqn;
    uint32_t dma_channel_bits;

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

            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
            dma_rx_stream = DMA2_Stream2;
            dma_tx_stream = DMA2_Stream7;
            dma_rx_irqn = DMA2_Stream2_IRQn;
            dma_tx_irqn = DMA2_Stream7_IRQn;
            dma_channel_bits = DMA_SxCR_CHSEL_2;
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

            RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
            dma_rx_stream = DMA1_Stream5;
            dma_tx_stream = DMA1_Stream6;
            dma_rx_irqn = DMA1_Stream5_IRQn;
            dma_tx_irqn = DMA1_Stream6_IRQn;
            dma_channel_bits = DMA_SxCR_CHSEL_2;
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

            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
            dma_rx_stream = DMA2_Stream1;
            dma_tx_stream = DMA2_Stream6;
            dma_rx_irqn = DMA2_Stream1_IRQn;
            dma_tx_irqn = DMA2_Stream6_IRQn;
            dma_channel_bits = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0;
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

    instance->dma_rx_stream = dma_rx_stream;
    instance->dma_tx_stream = dma_tx_stream;
    instance->dma_rx_read_index = 0;

    dma_rx_stream->CR = 0;
    dma_rx_stream->PAR = (uint32_t) &instance->base->DR;
    dma_rx_stream->M0AR = (uint32_t) instance->dma_rx_buffer;
    dma_rx_stream->NDTR = UART_DMA_RX_BUF_SIZE;
    dma_rx_stream->FCR = 0;
    NVIC_EnableIRQ(dma_rx_irqn);
    dma_rx_stream->CR = dma_channel_bits | DMA_SxCR_CIRC | DMA_SxCR_MINC
        | DMA_SxCR_HTIE | DMA_SxCR_TCIE | DMA_SxCR_EN;

    dma_tx_stream->CR = 0;
    dma_tx_stream->PAR = (uint32_t) &instance->base->DR;
    dma_tx_stream->FCR = 0;
    instance->dma_tx_cr_bits = dma_channel_bits | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | DMA_SxCR_TCIE;
    NVIC_EnableIRQ(dma_tx_irqn);

    queue_init(&instance->rx_queue, instance->rx_buffer, sizeof(uint8_t), UART_RX_QUEUE_DEPTH);
    mutex_init(&instance->rx_mutex);
    semaphore_init(&instance->tx_dma_done, 1);
    semaphore_signal(&instance->tx_dma_done);
    instance->rx_overrun_count = 0;

    /* Now enable Transmitter*/
    instance->base->CR1 |= USART_CR1_TE;
    /* Enable Receiver */
    instance->base->CR1 |= USART_CR1_RE;
    /* Enable USART */
    instance->base->CR1 |= USART_CR1_UE;

    instance->base->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT | USART_CR3_EIE;
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

    if (length == 0) {
        return;
    }

    if (length > UART_DMA_TX_BUF_SIZE) {
        length = UART_DMA_TX_BUF_SIZE;
    }

    semaphore_wait(&instance->tx_dma_done);

    memcpy(instance->dma_tx_buffer, buffer, length);

    instance->dma_tx_stream->M0AR = (uint32_t) instance->dma_tx_buffer;
    instance->dma_tx_stream->NDTR = length;
    instance->dma_tx_stream->CR = instance->dma_tx_cr_bits | DMA_SxCR_EN;
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

static void uart_dma_rx_drain(uart_instance_t *instance) {
    uint32_t write_index = (UART_DMA_RX_BUF_SIZE - instance->dma_rx_stream->NDTR) % UART_DMA_RX_BUF_SIZE;

    while (instance->dma_rx_read_index != write_index) {
        uint8_t byte = instance->dma_rx_buffer[instance->dma_rx_read_index];
        instance->dma_rx_read_index = (instance->dma_rx_read_index + 1) % UART_DMA_RX_BUF_SIZE;

        if (!enqueue_isr(&instance->rx_queue, &byte)) {
            instance->rx_overrun_count += 1;
        }
    }
}

static void uart_dma_tx_complete(uart_instance_t *instance) {
    semaphore_signal(&instance->tx_dma_done);
}

static void uart_error_irq(uart_instance_t *instance) {
    uint32_t sr = instance->base->SR;

    if (sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE)) {
        (void) instance->base->DR;
        instance->rx_overrun_count += 1;
    }
}

void USART1_IRQHandler(void) {
    uart_error_irq(&uart_instances[UART_1]);
}

void USART2_IRQHandler(void) {
    uart_error_irq(&uart_instances[UART_2]);
}

void USART6_IRQHandler(void) {
    uart_error_irq(&uart_instances[UART_6]);
}

void DMA2_Stream2_IRQHandler(void) {
    DMA2->LIFCR = DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CFEIF2;
    uart_dma_rx_drain(&uart_instances[UART_1]);
}

void DMA2_Stream7_IRQHandler(void) {
    DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7;
    uart_dma_tx_complete(&uart_instances[UART_1]);
}

void DMA1_Stream5_IRQHandler(void) {
    DMA1->HIFCR = DMA_HIFCR_CTCIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CFEIF5;
    uart_dma_rx_drain(&uart_instances[UART_2]);
}

void DMA1_Stream6_IRQHandler(void) {
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6;
    uart_dma_tx_complete(&uart_instances[UART_2]);
}

void DMA2_Stream1_IRQHandler(void) {
    DMA2->LIFCR = DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1;
    uart_dma_rx_drain(&uart_instances[UART_6]);
}

void DMA2_Stream6_IRQHandler(void) {
    DMA2->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6;
    uart_dma_tx_complete(&uart_instances[UART_6]);
}
