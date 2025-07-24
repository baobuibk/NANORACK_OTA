#include "bscript_logger.h"
#include "uart_driver_dma.h"
#include "board.h"
#include "DateTime/date_time.h"
#include <stdio.h>

#define BSCRIPT_LOG_UART UART_USB

#ifdef LOG_WITHOUT_TIMESTAMP

void BScript_Log(const char* fmt, ...) {
    char log_buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);
    va_end(args);


    UART_Driver_SendString(BSCRIPT_LOG_UART, log_buf);
    UART_Driver_SendString(BSCRIPT_LOG_UART, "\r\n");

}

#else

void BScript_Log(const char* fmt, ...) {
    char log_buf[256];
    char time_prefix[32];

    uint32_t days;
    uint8_t hours, minutes, seconds;
    Utils_GetWorkingTime(&days, &hours, &minutes, &seconds);

    snprintf(time_prefix, sizeof(time_prefix), "[%lu-%02u:%02u:%02u] ", days, hours, minutes, seconds);

    va_list args;
    va_start(args, fmt);
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);
    va_end(args);

    UART_Driver_SendString(BSCRIPT_LOG_UART, time_prefix);
    UART_Driver_SendString(BSCRIPT_LOG_UART, log_buf);
    UART_Driver_SendString(BSCRIPT_LOG_UART, "\r\n");
}

#endif


//Example 
/*
//init:
BScript_LoggerSetFunc(uart_logger);
// or
BScript_LoggerSetFunc(freertos_logger);

void uart_logger(const char* fmt, va_list args) {
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    uart_send(buffer); 
}

void freertos_logger(const char* fmt, va_list args) {
    static SemaphoreHandle_t log_mutex = NULL;
    if (log_mutex == NULL) log_mutex = xSemaphoreCreateMutex();

    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    if (xSemaphoreTake(log_mutex, portMAX_DELAY)) {
        uart_send(buffer); 
        xSemaphoreGive(log_mutex);
    }
}
*/
