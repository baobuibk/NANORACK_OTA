/*
 * uart_driver_dma.c
 *
 *  Created on: Feb 25, 2025
 *      Author: CAO HIEU
 */

#include "uart_driver_dma.h"
#include "stdatomic.h"

// Buffers USART1
static uint8_t uart1_dma_rx_buffer[UART1_DMA_RX_BUFFER_SIZE];
static RingBufElement uart1_rx_data[UART1_BUFFER_SIZE];
static RingBufElement uart1_tx_data[UART1_BUFFER_SIZE];

// Buffers USART2
static uint8_t uart2_dma_rx_buffer[UART2_DMA_RX_BUFFER_SIZE];
static RingBufElement uart2_rx_data[UART2_BUFFER_SIZE];
static RingBufElement uart2_tx_data[UART2_BUFFER_SIZE];

// Buffers UART7
static uint8_t uart7_dma_rx_buffer[UART7_DMA_RX_BUFFER_SIZE];
static RingBufElement uart7_rx_data[UART7_BUFFER_SIZE];
static RingBufElement uart7_tx_data[UART7_BUFFER_SIZE];

static UART_DMA_Driver_t uart_dma_drivers[UART_DMA_DRIVER_COUNT] = {
    // USART1
    {
        .uart = USART1,
        .rx_buffer = {0},
        .tx_buffer = {0},
        .rxSemaphore = NULL,
        .dma_rx_instance = DMA1,
        .dma_rx_channel = LL_DMA_STREAM_0,
        .dma_rx_buffer = uart1_dma_rx_buffer,
        .dma_rx_buffer_size = UART1_DMA_RX_BUFFER_SIZE,
		.old_dma_pos = 0
    },

    // USART2
    {
        .uart = USART2,
        .rx_buffer = {0},
		.tx_buffer = {0},
        .rxSemaphore = NULL,
        .dma_rx_instance = DMA1,
        .dma_rx_channel = LL_DMA_STREAM_1,
        .dma_rx_buffer = uart2_dma_rx_buffer,
        .dma_rx_buffer_size = UART2_DMA_RX_BUFFER_SIZE,
		.old_dma_pos = 0
    },

	// UART7
    {
        .uart = UART7,
        .rx_buffer = {0},
		.tx_buffer = {0},
        .rxSemaphore = NULL,
        .dma_rx_instance = DMA2,
        .dma_rx_channel = LL_DMA_STREAM_1,
        .dma_rx_buffer = uart7_dma_rx_buffer,
        .dma_rx_buffer_size = UART7_DMA_RX_BUFFER_SIZE,
		.old_dma_pos = 0
    }
};


/*************************************************
 *                  HELPER                       *
 *************************************************/
#ifndef SET
#define SET 						    1U
#endif
#ifndef RESET
#define RESET 						    0U
#endif

UART_DMA_Driver_t* UART_DMA_Driver_Get(USART_TypeDef *uart)
{
    for (int i = 0; i < UART_DMA_DRIVER_COUNT; i++) {
        if (uart_dma_drivers[i].uart == uart) {
            return &uart_dma_drivers[i];
        }
    }
    return NULL;
}

/*************************************************
 *                    Init                       *
 *************************************************/
Std_ReturnType UART_DMA_Driver_Init(void)
{
    // USART1 (index 0)
    for (int i = 0; i < UART_DMA_DRIVER_COUNT; i++) {
        LL_USART_Disable(uart_dma_drivers[i].uart);
        LL_DMA_DisableStream(uart_dma_drivers[i].dma_rx_instance, uart_dma_drivers[i].dma_rx_channel);
    }

    RingBuffer_Create(&uart_dma_drivers[0].rx_buffer, 1, "UART1_RX", uart1_rx_data, UART1_BUFFER_SIZE);
    RingBuffer_Create(&uart_dma_drivers[0].tx_buffer, 2, "UART1_TX", uart1_tx_data, UART1_BUFFER_SIZE);
    uart_dma_drivers[0].rxSemaphore = xSemaphoreCreateBinary();

    LL_DMA_SetPeriphAddress(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel,
                           LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetDataLength(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel,
                         uart_dma_drivers[0].dma_rx_buffer_size);
    LL_DMA_SetMemoryAddress(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel,
                            (uint32_t)uart_dma_drivers[0].dma_rx_buffer);
    LL_DMA_EnableIT_TC(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel);
    LL_DMA_EnableIT_HT(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel);
    LL_DMA_EnableIT_TE(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel);

    LL_USART_EnableDMAReq_RX(USART1);
    LL_DMA_EnableStream(uart_dma_drivers[0].dma_rx_instance, uart_dma_drivers[0].dma_rx_channel);

    LL_USART_EnableIT_IDLE(USART1);
//    LL_USART_EnableIT_RXNE(USART1);

    // USART2 (index 1)
    RingBuffer_Create(&uart_dma_drivers[1].rx_buffer, 5, "UART2_RX", uart2_rx_data, UART2_BUFFER_SIZE);
    RingBuffer_Create(&uart_dma_drivers[1].tx_buffer, 6, "UART2_TX", uart2_tx_data, UART2_BUFFER_SIZE);
    uart_dma_drivers[1].rxSemaphore = xSemaphoreCreateBinary();

    LL_DMA_SetPeriphAddress(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel,
                           LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetDataLength(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel,
                         uart_dma_drivers[1].dma_rx_buffer_size);
    LL_DMA_SetMemoryAddress(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel,
                            (uint32_t)uart_dma_drivers[1].dma_rx_buffer);
    LL_DMA_EnableIT_TC(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel);
    LL_DMA_EnableIT_HT(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel);
    LL_DMA_EnableIT_TE(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel);

    LL_USART_EnableDMAReq_RX(USART2);
    LL_DMA_EnableStream(uart_dma_drivers[1].dma_rx_instance, uart_dma_drivers[1].dma_rx_channel);

    LL_USART_EnableIT_IDLE(USART2);
//    LL_USART_EnableIT_RXNE(USART2);

    // UART7 (index 2)
    RingBuffer_Create(&uart_dma_drivers[2].rx_buffer, 11, "UART7_RX", uart7_rx_data, UART7_BUFFER_SIZE);
    RingBuffer_Create(&uart_dma_drivers[2].tx_buffer, 12, "UART7_TX", uart7_tx_data, UART7_BUFFER_SIZE);
    uart_dma_drivers[2].rxSemaphore = xSemaphoreCreateBinary();

    LL_DMA_SetPeriphAddress(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel,
                           LL_USART_DMA_GetRegAddr(UART7, LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetDataLength(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel,
                         uart_dma_drivers[2].dma_rx_buffer_size);
    LL_DMA_SetMemoryAddress(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel,
                            (uint32_t)uart_dma_drivers[2].dma_rx_buffer);
    LL_DMA_EnableIT_TC(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel);
    LL_DMA_EnableIT_HT(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel);
    LL_DMA_EnableIT_TE(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel);

    LL_USART_EnableDMAReq_RX(UART7);
    LL_DMA_EnableStream(uart_dma_drivers[2].dma_rx_instance, uart_dma_drivers[2].dma_rx_channel);

    LL_USART_EnableIT_IDLE(UART7);
//    LL_USART_EnableIT_RXNE(UART7);

    for (int i = 0; i < UART_DMA_DRIVER_COUNT; i++) {
        LL_USART_Enable(uart_dma_drivers[i].uart);
    }
    return E_OK;
}

/************************************************
 *                  NoDMA TX                    *
 ************************************************/
//void UART_Driver_Write(USART_TypeDef *uart, uint8_t data)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if (driver == NULL)
//        return;
//
//    uint32_t timeout = 500000;
//
//    while (!RingBuffer_Put(&driver->tx_buffer, data))
//    {
//        if (--timeout == 0)
//        {
//            return;
//        }
//    }
//
////    if (!LL_USART_IsEnabledIT_TXE(uart)) {
//        LL_USART_EnableIT_TXE(uart);
////    }
//}
void UART_Driver_Write(USART_TypeDef *uart, uint8_t data)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return;

    uint32_t timeout = 500000;

    while (!RingBuffer_Put(&driver->tx_buffer, data))
    {
        if (--timeout == 0)
        {
            return;
        }
    }

    if (!LL_USART_IsEnabledIT_TXE(uart)) {
        LL_USART_EnableIT_TXE(uart);
    }
}

void UART_Driver_SendString(USART_TypeDef *uart, const char *str)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL || str == NULL)
        return;


    while (*str)
        {
            UART_Driver_Write(uart, (uint8_t)(*str));
            str++;
        }
}

void UART_Driver_FlushTx(USART_TypeDef *uart)
{
	UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return;

    memset(driver->tx_buffer.buffer, 0, driver->tx_buffer.max_size);
    atomic_store_explicit(&driver->tx_buffer.head, 0U, memory_order_release);
    atomic_store_explicit(&driver->tx_buffer.tail, 0U, memory_order_release);
}

void UART_Driver_TX_ISR(USART_TypeDef *uart)
{
	UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return;

    if (driver->uart->ISR & USART_ISR_FE) {
        driver->uart->ICR = USART_ICR_FECF;  // Clear Framing Error Flag
    }

    if (driver->uart->ISR & USART_ISR_NE) {
        driver->uart->ICR = USART_ICR_NECF;  // Clear Noise Error Flag
    }

//    if (driver->uart->ISR & USART_ISR_EOBF) {
//        driver->uart->ICR = USART_ICR_EOBCF; // Clear End of Block Flag
//    }

    if (driver->uart->ISR & USART_ISR_CMF) {
        driver->uart->ICR = USART_ICR_CMCF;  // Clear Character Match Flag
    }

    if ((LL_USART_IsActiveFlag_TXE(uart) != RESET) &&
        (LL_USART_IsEnabledIT_TXE(uart) != RESET))
    {
        uint8_t tx_data;
        if (RingBuffer_Get(&driver->tx_buffer, &tx_data))
        {
            LL_USART_TransmitData8(uart, tx_data);
        }
        else
        {
            LL_USART_DisableIT_TXE(uart);
        }
    }
}

/************************************************
 *                    DMA RX                    *
 ************************************************/
void UART_DMA_Rx_Check(USART_TypeDef *uart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return;

    size_t old_pos = driver->old_dma_pos;
    size_t pos;

    uint16_t remaining = LL_DMA_GetDataLength(driver->dma_rx_instance, driver->dma_rx_channel);
    pos = driver->dma_rx_buffer_size - remaining;

//    char abc[60];
//    snprintf(abc, sizeof(abc), "o:%d, p: %d, r: %d, f0: %d , f1: %d\r\n", old_pos, pos, remaining, driver->dma_rx_buffer[0], driver->dma_rx_buffer[1]);
//    UART_Driver_Polling_SendString(USART1,  abc);
//    if (!LL_DMA_IsEnabledStream(driver->dma_rx_instance, driver->dma_rx_channel)) {
//        UART_Driver_Polling_SendString(USART1, "DMA stream disabled!\r\n");
//        LL_DMA_EnableStream(driver->dma_rx_instance, driver->dma_rx_channel);
//    }
//    if (driver->uart->ISR & USART_ISR_ORE) {
//        driver->uart->ICR = USART_ICR_ORECF;
//        UART_Driver_Polling_SendString(USART1, "Overrun Error detected!\r\n");
//    }

    if (pos != old_pos) {
        if (pos > old_pos) {
            for (size_t i = old_pos; i < pos; i++) {
                RingBuffer_Put(&driver->rx_buffer, driver->dma_rx_buffer[i]);
            }
        } else {
            for (size_t i = old_pos; i < driver->dma_rx_buffer_size; i++) {
                RingBuffer_Put(&driver->rx_buffer, driver->dma_rx_buffer[i]);
            }
            for (size_t i = 0; i < pos; i++) {
                RingBuffer_Put(&driver->rx_buffer, driver->dma_rx_buffer[i]);
            }
        }
        xSemaphoreGiveFromISR(driver->rxSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        driver->old_dma_pos = pos;
        if (driver->old_dma_pos == driver->dma_rx_buffer_size) {
            driver->old_dma_pos = 0;
        }
    }
}



int UART_DMA_Driver_Read(USART_TypeDef *uart)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if(driver == NULL)
        return -1;

    RingBufElement data;
    if(RingBuffer_Get(&driver->rx_buffer, &data)){
        return data;
    }
    return -1;
}

_Bool UART_DMA_Driver_IsDataAvailable(USART_TypeDef *uart)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if(driver == NULL)
        return false;

    return RingBuffer_IsDataAvailable(&driver->rx_buffer);
}

uint16_t UART_DMA_Driver_TXNumFreeSlots(USART_TypeDef *uart)
{
	UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return 0;

    return (uint16_t)RingBuffer_NumFreeSlots(&driver->tx_buffer);
}

uint16_t UART_DMA_Driver_RXNumFreeSlots(USART_TypeDef *uart)
{
	UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return 0;

    return (uint16_t)RingBuffer_NumFreeSlots(&driver->rx_buffer);
}


void UART_DMA_Driver_Flush(USART_TypeDef *uart)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if(driver == NULL)
        return;

    // Flush RX buffer
    memset(driver->rx_buffer.buffer, 0, driver->rx_buffer.max_size);
    atomic_store_explicit(&driver->rx_buffer.head, 0U, memory_order_release);
    atomic_store_explicit(&driver->rx_buffer.tail, 0U, memory_order_release);

    // Flush TX buffer
    memset(driver->tx_buffer.buffer, 0, driver->tx_buffer.max_size);
    atomic_store_explicit(&driver->tx_buffer.head, 0U, memory_order_release);
    atomic_store_explicit(&driver->tx_buffer.tail, 0U, memory_order_release);
}


/************************************************************************************************
 *                    						EXPAND FUNCTION                    					*
 ***********************************************************************************************/
void UART_Driver_Polling_Write(USART_TypeDef *uart, uint8_t data)
{
    while (!LL_USART_IsActiveFlag_TXE(uart))
    {
    }
    LL_USART_TransmitData8(uart, data);
    while (!LL_USART_IsActiveFlag_TC(uart))
    {
    }
}

void UART_Driver_Polling_SendString(USART_TypeDef *uart, const char *str)
{
    if (str == NULL)
        return;
    while (*str)
    {
    	UART_Driver_Polling_Write(uart, (uint8_t)(*str));
        str++;
    }
}
/*
 * @For Blocking Send Using DMA
 * @&For Non-Blocking Send Using DMA & Semaphores
 */

/************************************************
 *                  Blocking                    *
 ************************************************/

//void UART_DMA_Blocking_SendString(USART_TypeDef *uart, const char *str)
//{
//    if (uart == NULL || str == NULL)
//        return;
//
//    uint16_t len = strlen(str);
//    if (len == 0)
//        return;
//
//    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_1));
//    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)str);
//    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, len);
//    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
//    LL_USART_EnableDMAReq_TX(uart);
//}
//
//void UART_DMA_Blocking_SendByte(USART_TypeDef *uart, const uint8_t *byte)
//{
//    if (uart == NULL || byte == NULL)
//        return;
//
//    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_1));
//    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)byte);
//    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, 1);
//    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
//    LL_USART_EnableDMAReq_TX(uart);
//}

/************************************************
 *                    TX DMA                    *
 ************************************************/
//
//Std_ReturnType UART_DMA_Driver_SendString(USART_TypeDef *uart, const char *str)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if(driver == NULL || str == NULL)
//        return E_ERROR;
//
//    TX_Message_t msg;
//    msg.data = str;
//    msg.length = strlen(str);
//    if(msg.length == 0)
//        return E_ERROR;
//
//    if(xQueueSend(driver->txQueue, &msg, pdMS_TO_TICKS(50)) != pdPASS)
//    {
//        return E_ERROR;
//    }
//    return E_OK;
//}
//
//
//Std_ReturnType UART_DMA_Driver_SendByte(USART_TypeDef *uart, uint8_t byte)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if(driver == NULL)
//        return E_ERROR;
//
//    uint8_t *pByte = pvPortMalloc(sizeof(uint8_t));
//    if(pByte == NULL)
//        return E_ERROR;
//
//    *pByte = byte;
//    TX_Message_t msg;
//    msg.data = (const char *)pByte;
//    msg.length = 1;
//    msg.need_free = true;
//
//    if(xQueueSend(driver->txQueue, &msg, pdMS_TO_TICKS(50)) != pdPASS)
//    {
//        vPortFree(pByte);
//        return E_ERROR;
//    }
//    return E_OK;
//}
//
//void UART_DMA_Driver_FlushTX(USART_TypeDef *uart)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if(driver == NULL)
//        return;
//
//    TX_Message_t msg;
//    while(xQueueReceive(driver->txQueue, &msg, 0) == pdTRUE)
//    {
//        if(msg.need_free)
//        {
//            vPortFree((void *)msg.data);
//        }
//    }
//}
//
//void UART_DMA_TX_Task(void *pvParameters)
//{
//    UART_DMA_Driver_t *driver = (UART_DMA_Driver_t *) pvParameters;
//    TX_Message_t msg;
//
//    for(;;)
//    {
//        if(xQueueReceive(driver->txQueue, &msg, portMAX_DELAY) == pdTRUE)
//        {
//            xSemaphoreTake(driver->txSemaphore, 0);
//
//            LL_DMA_SetMemoryAddress(driver->dma_tx_instance, driver->dma_tx_channel, (uint32_t) msg.data);
//            LL_DMA_SetDataLength(driver->dma_tx_instance, driver->dma_tx_channel, msg.length);
//            LL_DMA_EnableStream(driver->dma_tx_instance, driver->dma_tx_channel);
//            LL_USART_EnableDMAReq_TX(driver->uart);
//            if(xSemaphoreTake(driver->txSemaphore, pdMS_TO_TICKS(100)) != pdTRUE)
//            {
//            }
//            if(msg.need_free)
//            {
//                vPortFree((void *)msg.data);
//            }
//        }
//    }
//}
//
//void TX_IRQHandler(USART_TypeDef *uart)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if(driver == NULL)
//        return;
//
//    if (LL_DMA_IsActiveFlag_TC1(driver->dma_tx_instance)) {
//        LL_DMA_ClearFlag_TC1(driver->dma_tx_instance);
//        LL_DMA_DisableStream(driver->dma_tx_instance, driver->dma_tx_channel);
//
//        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//        xSemaphoreGiveFromISR(driver->txSemaphore, &xHigherPriorityTaskWoken);
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }
//}

/*****************************************************
 *                    TEST RX DMA                    *
 *****************************************************/

//void UART_DMA_Driver_RX_ISR(USART_TypeDef *uart)
//{
//    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
//    if (driver == NULL)
//        return;
//
//    if (LL_USART_IsActiveFlag_IDLE(uart))
//    {
//    	LL_USART_ClearFlag_IDLE(USART1);
//
//        LL_DMA_DisableStream(driver->dma_rx_instance, driver->dma_rx_channel);
//
//
//        uint16_t remaining = LL_DMA_GetDataLength(driver->dma_rx_instance, driver->dma_rx_channel);
//        uint16_t bytes_received = driver->dma_rx_buffer_size - remaining;
//
//        // Copy DMA buffer to ring buffer
//        for (uint16_t i = 0; i < bytes_received; i++) {
////        	UART_DMA_Driver_SendByte(USART1, driver->dma_rx_buffer[i]);
//            RingBuffer_Put(&driver->rx_buffer, driver->dma_rx_buffer[i]);
//        }
//
//
//        LL_DMA_SetDataLength(driver->dma_rx_instance, driver->dma_rx_channel, driver->dma_rx_buffer_size);
//        LL_DMA_EnableStream(driver->dma_rx_instance, driver->dma_rx_channel);
//        UART_DMA_Driver_SendString(driver->uart, "Yo\r\n");
//
//        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//        xSemaphoreGiveFromISR(driver->rxSemaphore, &xHigherPriorityTaskWoken);
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }
//}
