/*
 * syslog.c
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#include "syslog.h"
#include "stdio.h"
#include "uart_driver_dma.h"
#include "utils.h"
#include "DateTime/date_time.h"
#include "Dmesg/dmesg.h"

static USART_TypeDef* syslog_uarts[SYSLOG_OUTPUT_UART_COUNT] = SYSLOG_OUTPUT_UARTS;

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

void Sys_Boardcast(bool status, syslog_level_t level, const char *msg)
{
    switch(level) {
        case LOG_INFOR:  if (!LOG_INFOR_ENABLED)  return; break;
        case LOG_DEBUG:  if (!LOG_DEBUG_ENABLED)  return; break;
        case LOG_NOTICE: if (!LOG_NOTICE_ENABLED) return; break;
        case LOG_WARN:   if (!LOG_WARN_ENABLED)   return; break;
        case LOG_ERROR:  if (!LOG_ERROR_ENABLED)  return; break;
        case LOG_FATAL:  if (!LOG_FATAL_ENABLED)  return; break;
        default:         return; // unknown level
    }

    char log_buffer[128];
    int offset;

    offset = 0;
    const char* status_str = status ? "[ ER ] " : "[ OK ] ";
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s", status_str);

    const char* level_str = syslog_level_to_str(level);
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s->[OBC-STM32] ", level_str);

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "\"%s\"\r\n", msg);

    for (uint32_t i = 0; log_buffer[i] != '\0'; i++)
    {
        while (!LL_USART_IsActiveFlag_TXE(UART_DEBUG));
        LL_USART_TransmitData8(UART_DEBUG, (uint8_t)log_buffer[i]);
    }
    while (!LL_USART_IsActiveFlag_TC(UART_DEBUG));

    offset = 0;
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s", status_str);

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s->[USB-STM32] ", level_str);

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "\"%s\"\r\n", msg);

    for (uint32_t i = 0; log_buffer[i] != '\0'; i++)
    {
        while (!LL_USART_IsActiveFlag_TXE(UART_USB));
        LL_USART_TransmitData8(UART_USB, (uint8_t)log_buffer[i]);
    }
    while (!LL_USART_IsActiveFlag_TC(UART_USB));
}

void Sys_Debugcast(bool status, syslog_level_t level, const char *msg)
{
    switch(level) {
        case LOG_INFOR:  if (!LOG_INFOR_ENABLED)  return; break;
        case LOG_DEBUG:  if (!LOG_DEBUG_ENABLED)  return; break;
        case LOG_NOTICE: if (!LOG_NOTICE_ENABLED) return; break;
        case LOG_WARN:   if (!LOG_WARN_ENABLED)   return; break;
        case LOG_ERROR:  if (!LOG_ERROR_ENABLED)  return; break;
        case LOG_FATAL:  if (!LOG_FATAL_ENABLED)  return; break;
        default:         return; // unknown level
    }

    char log_buffer[128];
    int offset;

    offset = 0;
    const char* status_str = status ? "[ ER ] " : "[ OK ] ";
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s", status_str);

    const char* level_str = syslog_level_to_str(level);
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "%s->[OBC-STM32] ", level_str);

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset, "\"%s\"\r\n", msg);

    for (uint32_t i = 0; log_buffer[i] != '\0'; i++)
    {
        while (!LL_USART_IsActiveFlag_TXE(UART_DEBUG));
        LL_USART_TransmitData8(UART_DEBUG, (uint8_t)log_buffer[i]);
    }
    while (!LL_USART_IsActiveFlag_TC(UART_DEBUG));
}

/*
 * Function syslog_log:
 * - If SYSLOG_USE_RTC is enabled, retrieves RTC time and formats it as: "20YY-MM-DD HH:MM:SS "
 * - If SYSLOG_USE_WORKING_TIME is enabled, retrieves working time and formats it as:
 *   "HH:MM:SS " (or "days+HH:MM:SS " if days > 0)
 * - Then, it appends the log level in the format "[LEVEL] " and the source (if configured) "[SYSLOG_SOURCE] "
 * - Finally, it appends the message content enclosed in quotes, followed by a newline.
 * Example output:
 *    2024-02-22 23:40:03 0:05:03 [NOTICE] [OBC-STM32] "Start up"
 */
void syslog_log(syslog_level_t level, const char *msg, int use_polling)
{
    char log_buffer[128];
    int offset = 0;

#if SYSLOG_USE_RTC
    s_DateTime rtc;
    Utils_GetRTC(&rtc);
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                "20%02d-%02d-%02d %02d:%02d:%02d ",
                rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
#endif

#if SYSLOG_USE_WORKING_TIME
    uint8_t hours = 0, minutes = 0, seconds = 0;
    Utils_GetWorkingTime(NULL, &hours, &minutes, &seconds);
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                    "%02u:%02u:%02u ",
                    hours, minutes, seconds);
#endif

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                "%s ", syslog_level_to_str(level));

#ifdef SYSLOG_SOURCE
    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                "[%s] ", SYSLOG_SOURCE);
#endif

    offset += snprintf(log_buffer + offset, sizeof(log_buffer) - offset,
                "\"%s", msg);

#ifdef DEBUG_USE_UART
        if (use_polling) {
            UART_Driver_Polling_SendString(syslog_uarts[0], log_buffer);
            UART_Driver_Polling_SendString(syslog_uarts[0], "\r\n");
        } else {
            Dmesg_SafeWrite(log_buffer);
        }
#endif
}
