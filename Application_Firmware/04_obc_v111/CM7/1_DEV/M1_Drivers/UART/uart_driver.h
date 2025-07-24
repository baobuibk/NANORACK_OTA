/*
 * uart.h
 *
 *  Created on: Feb 25, 2025
 *      Author: CAO HIEU
 */

#ifdef USE_NODMA_UART

#ifndef M1_DRIVERS_UART_UART_DRIVER_H_
#define M1_DRIVERS_UART_UART_DRIVER_H_

#include "stm32h7xx_ll_usart.h"
#include "ring_buffer.h"
#include "utils.h"
#include <stdint.h>

#define UART_DRIVER_COUNT    2

#define UART1_BUFFER_SIZE    128
#define UART6_BUFFER_SIZE    128

// Driver UART-Ringbuffer Struct
typedef struct {
    USART_TypeDef *uart;
    s_RingBufferType rx_buffer;  // Ringbuffer Rx
    s_RingBufferType tx_buffer;  // Ringbuffer Tx
} UART_Driver_t;

Std_ReturnType UART_Driver_Init(void);
void UART_Driver_ISR(USART_TypeDef *uart);
int  UART_Driver_Read(USART_TypeDef *uart);
void UART_Driver_Write(USART_TypeDef *uart, uint8_t data);
void UART_Driver_SendString(USART_TypeDef *uart, const char *str);
_Bool UART_Driver_IsDataAvailable(USART_TypeDef *uart);

// --- API: Flush Rx/Tx or Both ---
void UART_Driver_FlushRx(USART_TypeDef *uart);
void UART_Driver_FlushTx(USART_TypeDef *uart);
void UART_Driver_Flush(USART_TypeDef *uart);

#endif

#endif /* M1_DRIVERS_UART_UART_DRIVER_H_ */
