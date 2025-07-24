/************************************************
 *  @file     : uart_dma_driver.c
 *  @date     : May 13, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/
#include "uart_dma_driver.h"

// Buffers USART6
static uint8_t uart6_dma_rx_buffer[UART6_DMA_RX_BUFFER_SIZE];
static RingBufElement uart6_rx_data[UART6_BUFFER_SIZE];
static RingBufElement uart6_tx_data[UART6_BUFFER_SIZE];

static UART_DMA_Driver_t uart_dma_drivers[UART_DMA_DRIVER_COUNT] = {
    {
        .uart              = USART6,
        .rx_buffer         = {0},
        .tx_buffer         = {0},
        .dma_rx_instance   = DMA2,
        .dma_rx_channel    = LL_DMA_STREAM_0,
        .dma_rx_buffer     = uart6_dma_rx_buffer,
        .dma_rx_buffer_size= UART6_DMA_RX_BUFFER_SIZE,
        .old_dma_pos       = 0
    }
};

Std_ReturnType UART_DMA_Driver_Init(void)
{
    for (int i = 0; i < UART_DMA_DRIVER_COUNT; i++) {
        LL_USART_Disable(uart_dma_drivers[i].uart);
        LL_DMA_DisableStream(uart_dma_drivers[i].dma_rx_instance, uart_dma_drivers[i].dma_rx_channel);
    }
    /* Index 0 – USART6 ------------------------------------------------- */
    RingBuffer_Create(&uart_dma_drivers[0].rx_buffer, 1,
                      "UART6_RX", uart6_rx_data, UART6_BUFFER_SIZE);
    RingBuffer_Create(&uart_dma_drivers[0].tx_buffer, 2,
                      "UART6_TX", uart6_tx_data, UART6_BUFFER_SIZE);

    /* DMA circular RX --------------------------------------------------- */
    LL_DMA_SetPeriphAddress(uart_dma_drivers[0].dma_rx_instance,
                            uart_dma_drivers[0].dma_rx_channel,
                            LL_USART_DMA_GetRegAddr(USART6,
                                LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetMemoryAddress(uart_dma_drivers[0].dma_rx_instance,
                            uart_dma_drivers[0].dma_rx_channel,
                            (uint32_t)uart_dma_drivers[0].dma_rx_buffer);
    LL_DMA_SetDataLength (uart_dma_drivers[0].dma_rx_instance,
                          uart_dma_drivers[0].dma_rx_channel,
                          uart_dma_drivers[0].dma_rx_buffer_size);

    LL_DMA_EnableIT_TC(uart_dma_drivers[0].dma_rx_instance,
                       uart_dma_drivers[0].dma_rx_channel);
    LL_DMA_EnableIT_HT(uart_dma_drivers[0].dma_rx_instance,
                       uart_dma_drivers[0].dma_rx_channel);
    LL_DMA_EnableIT_TE(uart_dma_drivers[0].dma_rx_instance,
                       uart_dma_drivers[0].dma_rx_channel);

    LL_DMA_EnableStream(uart_dma_drivers[0].dma_rx_instance,
                        uart_dma_drivers[0].dma_rx_channel);

    LL_USART_EnableDMAReq_RX(USART6);
    LL_USART_EnableIT_IDLE(USART6);

    for (int i = 0; i < UART_DMA_DRIVER_COUNT; i++) {
        LL_USART_Enable(uart_dma_drivers[i].uart);
    }
    return E_OK;
}

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
void UART_DMA_Rx_Check(USART_TypeDef *uart)
{
    UART_DMA_Driver_t *driver = UART_DMA_Driver_Get(uart);
    if (driver == NULL)
        return;

    size_t old_pos = driver->old_dma_pos;
    size_t pos;

    uint16_t remaining = LL_DMA_GetDataLength(driver->dma_rx_instance,
                                              driver->dma_rx_channel);
    pos = driver->dma_rx_buffer_size - remaining;

    if (pos != old_pos)
    {
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
        driver->old_dma_pos = pos;
        if (driver->old_dma_pos == driver->dma_rx_buffer_size){
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
