/************************************************
 *  @file     : uart_dma_driver.h
 *  @date     : May 13, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M1_DRIVER4_UART_DMA_UART_DMA_DRIVER_H_
#define M1_DRIVER4_UART_DMA_UART_DMA_DRIVER_H_

#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_dma.h"
#include "RingBuffer/ring_buffer.h"
#include "Define/define.h"
#include <stdint.h>
#include "stdbool.h"

#define UART_DMA_DRIVER_COUNT 1

#define UART6_BUFFER_SIZE    			2048
#define UART6_DMA_RX_BUFFER_SIZE    	2048

#define TX_QUEUE_LENGTH 10


typedef struct {
    USART_TypeDef *uart;
    s_RingBufferType rx_buffer;
    s_RingBufferType tx_buffer;
    DMA_TypeDef *dma_rx_instance;
    uint32_t dma_rx_channel;
    uint8_t *dma_rx_buffer;
    uint16_t dma_rx_buffer_size;
    size_t old_dma_pos;   // DMA HISTORY
} UART_DMA_Driver_t;


Std_ReturnType UART_DMA_Driver_Init(void);
void UART_DMA_Driver_Flush(USART_TypeDef *uart);
uint16_t UART_DMA_Driver_TXNumFreeSlots(USART_TypeDef *uart);
uint16_t UART_DMA_Driver_RXNumFreeSlots(USART_TypeDef *uart);
UART_DMA_Driver_t* UART_DMA_Driver_Get(USART_TypeDef *uart);
/***************************************************
 *                     RX API                      *
 **************************************************/

void UART_DMA_Rx_Check(USART_TypeDef *uart);
int  UART_DMA_Driver_Read(USART_TypeDef *uart);
_Bool UART_DMA_Driver_IsDataAvailable(USART_TypeDef *uart);

/***************************************************
 *                     TX API                      *
 **************************************************/
void UART_Driver_Write(USART_TypeDef *uart, uint8_t data);
void UART_Driver_SendString(USART_TypeDef *uart, const char *str);
void UART_Driver_FlushTx(USART_TypeDef *uart);
void UART_Driver_TX_ISR(USART_TypeDef *uart);
void UART_Driver_Polling_Write(USART_TypeDef *uart, uint8_t data);
void UART_Driver_Polling_SendString(USART_TypeDef *uart, const char *str);

#endif /* M1_DRIVER4_UART_DMA_UART_DMA_DRIVER_H_ */
