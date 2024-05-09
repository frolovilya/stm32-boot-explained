#include "uart.h"
#include <stdio.h>
#include <string.h>

#define RX_BUFFER_SIZE 100

static char rxBuffer[RX_BUFFER_SIZE];
static uint16_t rxBufferIndex = 0;

static UART_RX_Handler rxCallback;

#ifdef USE_USART3

#define USARTX USART3
#define USARTX_IRQn USART3_IRQn
#define USARTX_EN RCC_APB1ENR_USART3EN

/**
 * USART3 is using the following pins:
 * TX: PC10,
 * RX: PC11
 */
static void configure_uart_gpio() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // enable clock for GPIOC

  // enable alternate function (10) for GPIOC pins PC10 and PC11
  GPIOC->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1;
  // select alternate function (0111: AF7)
  GPIOC->AFR[1] |=
      GPIO_AFRH_AFSEL10_0 | GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2;
  GPIOC->AFR[1] |=
      GPIO_AFRH_AFSEL11_0 | GPIO_AFRH_AFSEL11_1 | GPIO_AFRH_AFSEL11_2;
  // set high speed (11) output
  GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11;
}

#else

#define USARTX USART2
#define USARTX_IRQn USART2_IRQn
#define USARTX_EN RCC_APB1ENR_USART2EN

/**
 * USART2 is connected to STLink USB on Nucleo boards.
 * Otherwise using the following pins:
 * TX: PA2
 * RX: PA3
 */
static void configure_uart_gpio() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable clock for GPIOA

  // enable alternate function (10) for GPIOA pins PA2 and PA3
  GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
  // select alternate function (0111: AF7)
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_2;
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2;
  // set high speed (11) output
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2 | GPIO_OSPEEDER_OSPEEDR3;
}

#endif // USE_USART3

/**
 * Configure UART peripheral
 */
void configure_uart() {
  RCC->APB1ENR |= USARTX_EN; // enable clock

  configure_uart_gpio();

  USARTX->CR1 |= USART_CR1_UE;     // enable USART
  USARTX->CR1 |= USART_CR1_RXNEIE; // enable RX interrupt
  USARTX->CR1 |= USART_CR1_IDLEIE; // enable IDLE interrupt
  USARTX->CR1 &= ~USART_CR1_M;     // 8-bit data word length
  USARTX->CR2 &= ~USART_CR2_STOP;  // 1 stop bit
  // baud rate = f_clock / (16 * USARTDIV)
  // USARTDIV = 42MHz / (16 * 115200) = 22,7864
  USARTX->BRR =
      0x16C; // Mantissa = 22 (0x16); Fraction ~= 16 * 0,7864 = 13 (0xD)

  NVIC_EnableIRQ(USARTX_IRQn);
}

/**
 * Start UART data transfer and receival
 */
void start_uart() {
  USARTX->CR1 |= USART_CR1_TE | USART_CR1_RE; // start transmitter and receiver
}

/**
 * Send UART message
 *
 * @param data data to send
 * @param dataLength data length
 */
void send_uart(char *data, size_t dataLength) {
  for (size_t i = 0; i < dataLength; i++) {
    USARTX->DR = data[i];
    // wait DR to be ready for next data
    while (!(USARTX->SR & USART_SR_TXE))
      ;
  }

  // wait for transfer to be completed
  while (!(USARTX->SR & USART_SR_TC))
    ;
}

/**
 * Register a callback to receive data via UART
 *
 * @param callback callback fired on data receival
 */
void receive_uart(UART_RX_Handler callback) { rxCallback = callback; }

// to support printf via UART
int __io_putchar(int ch) {
  send_uart((char *)&ch, 1);

  return ch;
}

void USART_Common_IRQHandler() {
  // fill buffer
  while (USARTX->SR & USART_SR_RXNE) {
    if (rxBufferIndex > RX_BUFFER_SIZE) {
      rxBufferIndex = 0;
    }
    uint32_t receivedData = USARTX->DR;
    char receivedChar = (char)receivedData;
    if (receivedChar != '\0') {
      rxBuffer[rxBufferIndex++] = receivedChar;

      // trigger callback on LF
      if (receivedChar == '\n') {
        char receivedString[rxBufferIndex];
        strncpy(receivedString, rxBuffer, rxBufferIndex - 1);
        receivedString[rxBufferIndex - 1] = '\0';

        rxCallback(receivedString);

        rxBufferIndex = 0;
      }
    }
  }
}

void USART2_IRQHandler() { USART_Common_IRQHandler(); }

void USART3_IRQHandler() { USART_Common_IRQHandler(); }
