#ifndef __UART_H__
#define __UART_H__

#include <stddef.h>
#include <stm32f446xx.h>

// Callback to be fired on receiving UART data
typedef void (*UART_RX_Handler)(char *str);

void configure_uart();
void start_uart();
void send_uart(char *data, size_t dataLength);
void receive_uart(UART_RX_Handler callback);

#endif
