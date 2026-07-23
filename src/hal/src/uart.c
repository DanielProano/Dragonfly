#include "uart.h"
#include "stm32f401xc.h"
#include <stdio.h>

void uart_init(void) {
    /* Page 119 of Reference */

    /* Turn on GPIOA pin clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Page 123 of Reference Manual */

    /* Enable USART1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Page 158 in Reference Manual*/

    /* Clear GPIO A9 Pin */
    GPIOA->MODER &= ~(0x3 << 18);
    /* Set A9 to alternate function mode (USART1_TX) */
    GPIOA->MODER |= 0x2 << 18;

    /* Clear GPIO A10 Pin*/
    GPIOA->MODER &= ~(0x3 << 20);
    /* Set A10 to alternate function mode (USART1_RX) */
    GPIOA->MODER |= 0x2 << 20;

    /*  Configure alternate function registers */
    /*  Page 44 of STM32 datasheet demands we write
        AF07 into A9 & A10 to configure alternate
        function mapping */

    /* We need to configure pin 9 for A9, reset it*/
    GPIOA->AFR[1] &= ~(0xF << 4);

    /* See page 162 of Reference Manual */
    GPIOA->AFR[1] |= (15 << 4);

    /* A10 needs reset */
    GPIOA->AFR[1] &= ~(0xF << 8);

    /* See page 165 of Reference Manual, write AF7 into A10 */
    GPIOA->AFR[1] |= (15 << 8);

    /* Configure USART1 */

    /*  Baud rate (data transfer speed) is 
        configured to 0, so need to set it 
        manually */
    
    /*  Page 552 of Reference Manual
        Most common baud rate is 115,200, 
        Baud Rate = f_CK / ( 8 * (2 - OVER8) * USARTDIV)
        Default is 0 for control register, meaning 
        OVER8 is 0, meaning oversamping by 16
        = f_CK / ( 16 * USARTDIV) 
        Rearrange for USARTDIV:
        USARTDIV = f_CK / (Baud Rate * 16 )
        Substitute:
        USARTDIV = 84,000,000 / (115,200 * 16) = 45.572
        Now put into mantissa & fractional form:
        Mantissa needs to represent 45
        Need to get .572 into 1/16s form:
        ? / 16 = .572; ? = .572 * 16 = ~9 
        So we get (45 << 4) | 9 */
    USART1->BRR = ((45 << 4) | 9);

    /* Now enable Transmitter*/
    USART1->CR1 |= USART_CR1_TE;
    /* Enable Receiver */
    USART1->CR1 |= USART_CR1_RE;
    /* Enable USART */
    USART1->CR1 |= USART_CR1_UE;
}

void uart_send_byte(uint8_t byte) {
    /* check if data was sent to shift register yet */
    while (!(USART1->SR & USART_SR_TXE));

    /* Now write byte into data register */
    USART1->DR = byte;
}

uint8_t uart_receive_byte(void) {
    /* Wait for data register to be ready */
    while (!(USART1->SR &  USART_SR_RXNE));

    return (uint8_t) USART1->DR;
}