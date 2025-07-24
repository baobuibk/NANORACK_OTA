/*
 * syslog_queue.c
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "syslog_queue.h"
#include "uart_driver_dma.h"
#include "stdio.h"
#include "DateTime/date_time.h"
#include "Dmesg/dmesg.h"

#define SYSLOG_OUTPUT_BUFFER_SIZE 128
#define SYSLOG_MSG_MAX_LENGTH     64
#define SYSLOG_QUEUE_SLOT		  16

typedef struct {
    syslog_level_t level;
    char msg[SYSLOG_MSG_MAX_LENGTH];
} syslog_msg_t;

static QueueHandle_t syslogQueue = NULL;

//static USART_TypeDef* syslog_uarts[SYSLOG_OUTPUT_UART_COUNT] = SYSLOG_OUTPUT_UARTS;
//static const int syslog_uart_count = sizeof(syslog_uarts) / sizeof(syslog_uarts[0]);

static const char* syslog_level_to_str(syslog_level_t level)
{
    switch(level) {
        case LOG_INFOR:  return "[INFO]  ";
        case LOG_DEBUG:  return "[DEBUG] ";
        case LOG_NOTICE: return "[NOTICE]";
        case LOG_WARN:   return "[WARN]  ";
        case LOG_ERROR:  return "[ERROR] ";
        case LOG_FATAL:  return "[FATAL] ";
        default:         return "[UNK]   ";
    }
}

void SysLogQueue_Init(void)
{
    syslogQueue = xQueueCreate(SYSLOG_QUEUE_SLOT, sizeof(syslog_msg_t));
}

void SysLog_Enqueue(syslog_level_t level, const char *msg)
{
    syslog_msg_t logMsg;
    logMsg.level = level;
    strncpy(logMsg.msg, msg, SYSLOG_MSG_MAX_LENGTH - 1);
    logMsg.msg[SYSLOG_MSG_MAX_LENGTH - 1] = '\0';
    if(syslogQueue != NULL)
    {
        xQueueSend(syslogQueue, &logMsg, 0);
    }
}

void SysLog_Task(void *parameters)
{
    syslog_msg_t logMsg;
    char outputBuffer[SYSLOG_OUTPUT_BUFFER_SIZE];
    for(;;)
    {
        if(xQueueReceive(syslogQueue, &logMsg, portMAX_DELAY) == pdTRUE)
        {
            int offset = 0;
  #if SYSLOG_USE_RTC
            s_DateTime rtc;
            Utils_GetRTC(&rtc);
            offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                               "20%02d-%02d-%02d %02d:%02d:%02d ",
                               rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
  #endif

  #if SYSLOG_USE_WORKING_TIME
            uint32_t days = 0;
            uint8_t hours = 0, minutes = 0, seconds = 0;
            Utils_GetWorkingTime(&days, &hours, &minutes, &seconds);
            if (days > 0)
            {
                offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                                   "%lu+%02u:%02u:%02u ",
                                   days, hours, minutes, seconds);
            }
            else
            {
                offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                                   "%02u:%02u:%02u ",
                                   hours, minutes, seconds);
            }
  #endif

            offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                               "%s ", syslog_level_to_str(logMsg.level));

  #ifdef SYSLOG_SOURCE
            offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                               "[%s] ", SYSLOG_SOURCE);
  #endif

            offset += snprintf(outputBuffer + offset, SYSLOG_OUTPUT_BUFFER_SIZE - offset,
                               "\"%s", logMsg.msg);

  #ifdef DEBUG_USE_UART
            Dmesg_SafeWrite(outputBuffer);
  #endif
        }
    }
}
