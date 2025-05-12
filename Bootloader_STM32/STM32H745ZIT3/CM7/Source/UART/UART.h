/*
 * UART2.h
 *
 *  Created on: Nov 16, 2024
 *      Author: thanh
 */

#ifndef UART_H_
#define UART_H_

#include "RingBuffer.h"

// DEFINE RING BUFFER SIZE; MUST BE 2, 4, 8, 16, 32, 64 or 128
#define RBUFFER_SIZE 256

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART META STRUCT
typedef struct {
	volatile ringbuffer_t rb_rx;    // Rx ringbuffer
	volatile ringbuffer_t rb_tx;    // Tx ringbuffer
	volatile uint16_t usart_error;   // Holds error from RXDATAH
} usart_meta_t;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS
void USART1_init(void);
void USART1_send_char(char c);
void USART1_send_string(const char *str);
void USART1_send_array(const char *str, uint8_t len);
//void USART2_send_string_P(const char *chr);
uint8_t USART1_rx_count(void);
uint16_t USART1_read_char(void);
void USART1_close(void);
void USART1_SetBaudRate(uint32_t Baud);
volatile ringbuffer_t* uart_get_USART1_rx_buffer_address(void);

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//UART ISR
void USART1_IRQ();
void BLE_TX_Manager();

#endif /* UART_H_ */
