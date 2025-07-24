/*
 * usb_driver.c
 *
 *  Created on: Mar 1, 2025
 *      Author: CAO HIEU
 */

#include "cdc_driver.h"
#include "RingBuffer/ring_buffer.h"

/*************************************************
 *          Static RingBuffer Define             *
 *************************************************/
//RX
static RingBufElement cdcRxBuffer[CDC_RX_RING_BUFFER_SIZE];
static s_RingBufferType cdcRxRingBuffer;
//TX
static RingBufElement cdcTxBuffer[CDC_TX_RING_BUFFER_SIZE];
static s_RingBufferType cdcTxRingBuffer;

volatile uint8_t g_cdcRxReady = 0;
volatile uint8_t g_usbCheckEvt = 0;

/*************************************************
 *                 Function Define               *
 *************************************************/

void CDC_RingBuffer_Init(void)
{
	RingBuffer_Create(&cdcTxRingBuffer, 3, "CDC_TX", cdcTxBuffer, CDC_TX_RING_BUFFER_SIZE);
    RingBuffer_Create(&cdcRxRingBuffer, 4, "CDC_RX", cdcRxBuffer, CDC_RX_RING_BUFFER_SIZE);
}
/*************************************************/
static _Bool CDC_TX_RingBuffer_Put(uint8_t data)
{
    return RingBuffer_Put(&cdcTxRingBuffer, data);
}

static _Bool CDC_RX_RingBuffer_Put(uint8_t data)
{
    return RingBuffer_Put(&cdcRxRingBuffer, data);
}
/*************************************************/
uint32_t CDC_TX_RingBuffer_PutBuffer(const uint8_t* data, uint32_t len)
{
    uint32_t count = 0;
    for(uint32_t i = 0; i < len; i++)
    {
        if(CDC_TX_RingBuffer_Put(data[i]))
        {
            count++;
        }
    }
    return count;
}

uint32_t CDC_RX_RingBuffer_PutBuffer(uint8_t* data, uint32_t len)
{
    uint32_t count = 0;
    for(uint32_t i = 0; i < len; i++)
    {
        if(CDC_RX_RingBuffer_Put(data[i]))
        {
            count++;
        }
    }
    return count;
}
/*************************************************/
_Bool CDC_TX_RingBuffer_Get(uint8_t* data)
{
    return RingBuffer_Get(&cdcTxRingBuffer, data);
}

_Bool CDC_RX_RingBuffer_Get(uint8_t* data)
{
    return RingBuffer_Get(&cdcRxRingBuffer, data);
}
/*************************************************/

void CDC_ISR_RxNotify(void)
{
    g_cdcRxReady = 1;
}

void USB_Check_Notify(void)
{
    g_usbCheckEvt = 1;
}

uint8_t CDC_getRxReady(void)
{
    return g_cdcRxReady;
}

void CDC_setRxReady(uint8_t value)
{
    g_cdcRxReady = value;
}

uint8_t USB_checkUSB(void)
{
    return g_usbCheckEvt;
}

void USB_setCheckFlag(uint8_t value)
{
    g_usbCheckEvt = value;
}


/*************************************************
 *                    API Define                 *
 *************************************************/
_Bool CDCTxBuffer_IsDataAvailable(void)
{
    return RingBuffer_IsDataAvailable(&cdcTxRingBuffer);
}

_Bool CDC_SendString(const char *pStr, uint32_t len)
{
    if (len == 0 || pStr == NULL)
        return true;
    uint32_t written = CDC_TX_RingBuffer_PutBuffer((const uint8_t *)pStr, len);
    return (written == len) ? true : false;
}

_Bool CDC_SendChar(char c)
{
    return CDC_TX_RingBuffer_Put((uint8_t)c);
}
