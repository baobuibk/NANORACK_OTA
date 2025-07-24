/*
 * rtos.c
 *
 *  Created on: Feb 21, 2025
 *      Author: CAO HIEU
 */

#include "rtos.h"
#include "rtos_tasks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SysLog/syslog.h"
#include "stdio.h"

/*--------------------Star RTOS--------------*/
void OBC_RTOS_Start(void)
{
	OBC_RootGrowUp();
}

/*--------------------RTOS Task List--------------*/

/* Hook prototypes */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Stack Overflow -> Task %s", pcTaskName);
	SYSLOG_FATAL_POLL(buffer);
}

void vApplicationMallocFailedHook(void)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Malloc Hook Overflow");
	SYSLOG_FATAL_POLL(buffer);
}

