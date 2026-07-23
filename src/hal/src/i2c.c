#include "i2c.h"
#include "stm32f401xc.h"

void i2c_init(void) {
    /* Enable Clock for B pins*/
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* Enable Clock for I2C1 */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~(0x3 << 12);
    GPIOB->MODER &= ~(0x3 << 14);

    GPIOB->MODER |= (0x2 << 12);
    GPIOB->MODER |= (0x2 << 14);

    /* Configure alternate functions */
    /* Page 162 of Reference Manual */

    GPIOB->AFR[0] &= ~(15 << 24);
    GPIOB->AFR[0] &= ~(15 << 28);

    /* Write AF4 into PB6 & PB7*/
    GPIOB->AFR[0] |= (0x4 << 24);
    GPIOB->AFR[0] |= (0x4 << 28);

    /* Enable Open-drain I2C*/
    GPIOB->OTYPER |= ((1 << 6) | (1 << 7));

    /* Reset */
    I2C1->CR2 &= ~(0x3F << 0);

    /* Configure I2C clock rate to APB, which is 42MHz */
    I2C1->CR2 |= 42;

    /* Page 503, configure clock wire*/

    /* Turn on standard mode */
    I2C1->CCR &= ~(1 << 15);

    /* https://controllerstech.com/stm32-i2c-configuration-using-registers/ */
    /* CCR = (T_r(SCL) + T_w(SCLH)) / T_PCLK1 */
    /* T_PCLK1 = 1 / I2C Clock rate */
    /* I2C clock rate = 42 MHz*/
    /* T_PCLK1 (ns) = 1 MHz / 42,000,000 ns = 0.0000000238 MHz*/
    /* = 23.8095 ns */
    /* Page 96 of Cortex M datasheet */
    /* T_r(SCL) = 1000 ns */
    /* T_w(SCLH) = 4000 ns */
    /* CCR = 5000 ns / 23.8095 ns = 210.00021*/
    I2C1->CCR = 210;

    /* Configure how long I2C must wait to read after release*/
    /* TRISE = T_r(RCL) / T_PCLK + 1*/
    /* = 1000 / 23.8095 + 1 = 43 */
    I2C1->TRISE = 43;

    /* Enable I2C_CR1 */
    I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_start(void) {
    /* https://controllerstech.com/stm32-i2c-configuration-using-registers/ */

    /* Acknowledge enable */
    I2C1->CR1 |= (1 << 10);

    /* Start generation */
    I2C1->CR1 |= (1 << 8);

    /* Wait for Start generation confirmation */
    while (!(I2C1->SR1 & (1 << 0)));
}

void i2c_write_byte(uint8_t byte) {
    /* Wait for TxE bit to indicate Data register is ready */
    while (!(I2C1->SR1 & (1 << 7)));

    /* Write byte */
    I2C1->DR = byte;

    /* Wait for end of send */
    while (!(I2C1->SR1 & (1 << 2)));
}

void i2c_write_address(uint8_t data, uint8_t read_or_write) {
    /* Shift address over, form complete message */
    /* LSB is used to indicate read or write */
    I2C1->DR = (data << 1) | (read_or_write & 0x1);

    /* Wait for ADDR flag to indicate success */
    while (!(I2C1->SR1 & (1 << 1)));

    /*  Clear ADDR flag by reading SR1 & then SR2, 
        ORing them is arbitrary */
    (void) (I2C1->SR1 | I2C1->SR2);
}

uint8_t i2c_read_byte(uint8_t ack_enable) {
    /* Enable acknowledgement if caller demands it*/
    if (!ack_enable) {
        I2C1->CR1 &= ~(1 << 10);
    }

    /* Wait for byte to arrive*/
    while (!(I2C1->SR1 & (1 << 6)));

    /* Return byte */
    return (uint8_t) I2C1->DR;
}

void i2c_stop(void) {
    I2C1->CR1 |= (1 << 9);
}