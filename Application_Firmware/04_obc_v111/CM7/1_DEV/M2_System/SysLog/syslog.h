/*
 * syslog.h
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#ifndef M2_SYSTEM_SYSLOG_SYSLOG_H_
#define M2_SYSTEM_SYSLOG_SYSLOG_H_

#include "board.h"
#include "stdbool.h"

/*
 * Customizable configurations - SYSLOG & QUEUELOG
 */

#define DEBUG_USE_UART

// Enable/Disable RTC timestamp (DD-MM-YYYY HH:MM:SS)
#ifndef SYSLOG_USE_RTC
#define SYSLOG_USE_RTC 1
#endif

// Enable/Disable working time display (HH:MM:SS or days+HH:MM:SS)
#ifndef SYSLOG_USE_WORKING_TIME
#define SYSLOG_USE_WORKING_TIME 1
#endif

// Log source identifier (e.g., "OBC-STM32")
#ifndef SYSLOG_SOURCE
#define SYSLOG_SOURCE "OBC"
#endif

// Number of UART outputs (logs can be sent to multiple UARTs)
#ifndef SYSLOG_OUTPUT_UART_COUNT
#define SYSLOG_OUTPUT_UART_COUNT 1
#endif

// List of UART outputs, default is USART1 (can be modified)
#ifndef SYSLOG_OUTPUT_UARTS
#define SYSLOG_OUTPUT_UARTS {UART_DEBUG}
#endif

/*---------------------------
 * ON/OFF LOGLEVEL  (Queue not included here)
 *---------------------------*/
#ifndef LOG_INFOR_ENABLED
#define LOG_INFOR_ENABLED 1
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

#ifndef LOG_NOTICE_ENABLED
#define LOG_NOTICE_ENABLED 1
#endif

#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_ERROR_ENABLED
#define LOG_ERROR_ENABLED 1
#endif

#ifndef LOG_FATAL_ENABLED
#define LOG_FATAL_ENABLED 1
#endif

/**
 * @brief Log levels.
 */
typedef enum {
    LOG_INFOR = 0,
    LOG_DEBUG,
    LOG_NOTICE,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} syslog_level_t;

void syslog_log(syslog_level_t level, const char *msg, int use_polling);

void Sys_Boardcast(bool status, syslog_level_t level, const char *msg);

void Sys_Debugcast(bool status, syslog_level_t level, const char *msg);
/*---------------------------
 * --> List Macro:
 *---------------------------*/

#if LOG_INFOR_ENABLED
#define SYSLOG_INFO(msg)    syslog_log(LOG_INFOR, msg, 0)
#else
#define SYSLOG_INFO(msg)    { __asm volatile("nop"); }
#endif

#if LOG_DEBUG_ENABLED
#define SYSLOG_DEBUG(msg)   syslog_log(LOG_DEBUG, msg, 0)
#else
#define SYSLOG_DEBUG(msg)   { __asm volatile("nop"); }
#endif

#if LOG_NOTICE_ENABLED
#define SYSLOG_NOTICE(msg)  syslog_log(LOG_NOTICE, msg, 0)
#else
#define SYSLOG_NOTICE(msg)  { __asm volatile("nop"); }
#endif

#if LOG_WARN_ENABLED
#define SYSLOG_WARN(msg)    syslog_log(LOG_WARN, msg, 0)
#else
#define SYSLOG_WARN(msg)    { __asm volatile("nop"); }
#endif

#if LOG_ERROR_ENABLED
#define SYSLOG_ERROR(msg)   syslog_log(LOG_ERROR, msg, 0)
#else
#define SYSLOG_ERROR(msg)   { __asm volatile("nop"); }
#endif

#if LOG_FATAL_ENABLED
#define SYSLOG_FATAL(msg)   syslog_log(LOG_FATAL, msg, 0)
#else
#define SYSLOG_FATAL(msg)   { __asm volatile("nop"); }
#endif

// Macro Polling
#if LOG_INFOR_ENABLED
#define SYSLOG_INFO_POLL(msg)    syslog_log(LOG_INFOR, msg, 1)
#else
#define SYSLOG_INFO_POLL(msg)    { __asm volatile("nop"); }
#endif

#if LOG_DEBUG_ENABLED
#define SYSLOG_DEBUG_POLL(msg)   syslog_log(LOG_DEBUG, msg, 1)
#else
#define SYSLOG_DEBUG_POLL(msg)   { __asm volatile("nop"); }
#endif

#if LOG_NOTICE_ENABLED
#define SYSLOG_NOTICE_POLL(msg)  syslog_log(LOG_NOTICE, msg, 1)
#else
#define SYSLOG_NOTICE_POLL(msg)  { __asm volatile("nop"); }
#endif

#if LOG_WARN_ENABLED
#define SYSLOG_WARN_POLL(msg)    syslog_log(LOG_WARN, msg, 1)
#else
#define SYSLOG_WARN_POLL(msg)    { __asm volatile("nop"); }
#endif

#if LOG_ERROR_ENABLED
#define SYSLOG_ERROR_POLL(msg)   syslog_log(LOG_ERROR, msg, 1)
#else
#define SYSLOG_ERROR_POLL(msg)   { __asm volatile("nop"); }
#endif

#if LOG_FATAL_ENABLED
#define SYSLOG_FATAL_POLL(msg)   syslog_log(LOG_FATAL, msg, 1)
#else
#define SYSLOG_FATAL_POLL(msg)   { __asm volatile("nop"); }
#endif
#endif /* M2_SYSTEM_SYSLOG_SYSLOG_H_ */
