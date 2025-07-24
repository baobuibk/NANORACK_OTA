/*
 * usb_driver.c
 *
 *  Created on: Mar 1, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_USB_USB_DRIVER_C_
#define M1_DRIVERS_USB_USB_DRIVER_C_

#include "stdint.h"
#include "stdbool.h"

#define CDC_RX_RING_BUFFER_SIZE 2048
#define CDC_TX_RING_BUFFER_SIZE 2048
#define CDC_USB_MAX_PKT         2048

void CDC_RingBuffer_Init(void);

uint32_t CDC_TX_RingBuffer_PutBuffer(const uint8_t* data, uint32_t len);
uint32_t CDC_RX_RingBuffer_PutBuffer(uint8_t* data, uint32_t len);

_Bool CDC_TX_RingBuffer_Get(uint8_t* data);
_Bool CDC_RX_RingBuffer_Get(uint8_t* data);

void CDC_ISR_RxNotify(void);
void USB_Check_Notify(void);

_Bool CDC_SendString(const char *pStr, uint32_t len);
_Bool CDC_SendChar(char c);

_Bool CDCTxBuffer_IsDataAvailable(void);

uint8_t USB_checkUSB(void);
uint8_t CDC_getRxReady(void);
void CDC_setRxReady(uint8_t value);
void USB_setCheckFlag(uint8_t value);

#endif /* M1_DRIVERS_USB_USB_DRIVER_C_ */
