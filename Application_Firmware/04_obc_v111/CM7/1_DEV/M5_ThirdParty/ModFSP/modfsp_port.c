#include "modfsp_port.h"

#include "uart_driver_dma.h"
#include "board.h"
#include "utils.h"

#ifndef NULL
#define NULL ((void *)0)
#endif


void MODFSP_SendByte(const uint8_t *byte)
{
    if (byte == NULL) return;
    UART_Driver_Write(UART_DEBUG, *byte);
}

int MODFSP_ReadByte(uint8_t *byte)
{
    if (byte == NULL) {
        return -1;
    }

    int data = UART_DMA_Driver_Read(UART_DEBUG);
    if (data >= 0) {
        *byte = data;
         return 0;
    }

    return -1;
}

uint16_t MODFSP_GetSpaceForTx(void)
{
    return UART_DMA_Driver_TXNumFreeSlots(UART_DEBUG);
}

uint32_t MODFSP_GetTick(void)
{
	return Utils_GetTick();
}


#if MODFSP_ENABLE_LOG

#include "stdarg.h"
#include "stdio.h"

void MODFSP_Log(const char *fmt, ...)
{
    char log_buf[MODFSP_LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);
    va_end(args);

#if MODFSP_LOG_METHOD == 1
    UART_Driver_SendString(UART_USB, "[MODFSP]: ");
    UART_Driver_SendString(UART_USB, log_buf);
    UART_Driver_SendString(UART_USB, "\r\n");
#elif MODFSP_LOG_METHOD == 2
    extern UART_HandleTypeDef MODFSP_LOG_UART_HANDLE;
    HAL_UART_Transmit(&MODFSP_LOG_UART_HANDLE, (uint8_t*)log_buf, strlen(log_buf), 100);
#endif
}

#endif // MODFSP_ENABLE_LOG
