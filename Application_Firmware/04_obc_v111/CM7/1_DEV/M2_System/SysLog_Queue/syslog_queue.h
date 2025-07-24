/*
 * syslog_queue.h
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#ifndef M2_SYSTEM_SYSLOG_QUEUE_SYSLOG_QUEUE_H_
#define M2_SYSTEM_SYSLOG_QUEUE_SYSLOG_QUEUE_H_

#include "SysLog/syslog.h"


void SysLog_Task(void *parameters);
void SysLog_Enqueue(syslog_level_t level, const char *msg);
void SysLogQueue_Init(void);

/*---------------------------
 * ON/OFF LOGLEVEL  (Only for QUEUE)
 *---------------------------*/
#ifndef QUEUE_INFOR_ENABLED
#define QUEUE_INFOR_ENABLED 1
#endif

#ifndef QUEUE_DEBUG_ENABLED
#define QUEUE_DEBUG_ENABLED 1
#endif

#ifndef QUEUE_NOTICE_ENABLED
#define QUEUE_NOTICE_ENABLED 1
#endif

#ifndef QUEUE_WARN_ENABLED
#define QUEUE_WARN_ENABLED 1
#endif

#ifndef QUEUE_ERROR_ENABLED
#define QUEUE_ERROR_ENABLED 1
#endif

#ifndef QUEUE_FATAL_ENABLED
#define QUEUE_FATAL_ENABLED 1
#endif

/*---------------------------
 * --> List Macro:
 *---------------------------*/

#if QUEUE_INFOR_ENABLED
#define LogQueue_INFO(msg)    SysLog_Enqueue(LOG_INFOR, msg)
#else
#define LogQueue_INFO(msg)    { __asm volatile("nop"); }
#endif

#if QUEUE_DEBUG_ENABLED
#define LogQueue_DEBUG(msg)   SysLog_Enqueue(LOG_DEBUG, msg)
#else
#define LogQueue_DEBUG(msg)   { __asm volatile("nop"); }
#endif

#if QUEUE_NOTICE_ENABLED
#define LogQueue_NOTICE(msg)  SysLog_Enqueue(LOG_NOTICE, msg)
#else
#define LogQueue_NOTICE(msg)  { __asm volatile("nop"); }
#endif

#if QUEUE_WARN_ENABLED
#define LogQueue_WARN(msg)    SysLog_Enqueue(LOG_WARN, msg)
#else
#define LogQueue_WARN(msg)    { __asm volatile("nop"); }
#endif

#if QUEUE_ERROR_ENABLED
#define LogQueue_ERROR(msg)   SysLog_Enqueue(LOG_ERROR, msg)
#else
#define LogQueue_ERROR(msg)   { __asm volatile("nop"); }
#endif

#if QUEUE_FATAL_ENABLED
#define LogQueue_FATAL(msg)   SysLog_Enqueue(LOG_FATAL, msg)
#else
#define LogQueue_FATAL(msg)   { __asm volatile("nop"); }
#endif

#endif /* M2_SYSTEM_SYSLOG_QUEUE_SYSLOG_QUEUE_H_ */
