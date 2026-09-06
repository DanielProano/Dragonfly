#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    UART_1,
    UART_2,
    UART_6,
    NUM_UARTS
} uart_id_t;

void uart_init(uart_id_t id, uint32_t baud);
void uart_send_byte(uart_id_t id, uint8_t byte);
uint8_t uart_receive_byte(uart_id_t id);
void uart_send_bytes(uart_id_t id, const uint8_t *buffer, uint32_t length);
void uart_receive_bytes(uart_id_t id, uint8_t *buffer, uint32_t length);
bool uart_receive_byte_timeout(uart_id_t id, uint8_t *byte, uint32_t timeout_ms);
bool uart_receive_bytes_timeout(uart_id_t id, uint8_t *buffer, uint32_t length, uint32_t timeout_ms);
uint32_t uart_rx_overrun_count(uart_id_t id);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART6_IRQHandler(void);

void DMA2_Stream2_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void DMA2_Stream6_IRQHandler(void);

#endif
